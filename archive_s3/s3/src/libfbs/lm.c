/*
 * lm.c -- Disk-based backoff word trigram LM module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 22-Sep-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check for validity of START_ and FINISH_WORD in lm_read_dump().
 * 
 * 31-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added compound words handling option.  Compound words are broken up
 * 		into component words for computing LM probabilities.
 * 
 * 23-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.log_bg_seg_sz and lm_t.bg_seg_sz.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_rawscore().
 *
 * 03-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added unistd.h include.
 *
 * 26-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Fixed bug in lm_bg_score (pointed out by purify) that touched uninitialized
 * 		variable bg when (n == 0).
 *
 * 02-Oct-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (_SUN4)
#include <unistd.h>
#endif
#include <assert.h>

#include <libutil/libutil.h>

#include "s3types.h"
#include "lm.h"
#include "logs3.h"
#include "s3.h"

#if (! NO_DICT)
#include "dict.h"
#endif


#define MAX_UG		(65534)

#define SWAP_UINT16(x)	x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define SWAP_UINT32(x)	x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
    			      (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )


static lmset_t *lmset = NULL;		/* List of LMs loaded */
static int32 n_lm_alloc = 0;		/* Space allocated in lmset; used or not */
static int32 n_lm = 0;			/* #LMs; subset of n_lm_alloc */
static lm_t *cur_lm;			/* The currently active lm */

static s3lmwid_t *dict2lmwid = NULL;	/* Dictionary to LM word-id map */
static int32 dictsize;			/* #words in dictionary */

static char *darpa_hdr = "Darpa Trigram LM";
#define MIN_PROB_F	((float32)-99.0)

#if (! NO_DICT)
static dict_t *dict = NULL;
#endif


#if (NO_DICT || _LM_TEST_)
#include <math.h>

static int32 p_to_s3log(float64 wip)
{
    int32 p;
    
    p = (log(wip)/9.9995e-5);
    return p;
}

static int32 logp_to_s3log (float64 log10p)
{
    int32 p;
    
    p = log10p * (2.30258509 / 9.9995e-5);
    return p;
}
#endif


static int32 lmname_to_id (char *name)
{
    int32 i;
    
    for (i = 0; (i < n_lm) && (strcmp (lmset[i].name, name) != 0); i++);
    return ((i < n_lm) ? i : -1);
}


int32 lm_delete (char *name)
{
    int32 i;
    lm_t *lm;
    tginfo_t *tginfo, *next_tginfo;
    
    if ((i = lmname_to_id (name)) < 0)
	return (-1);
    
    lm = lmset[i].lm;
    if (cur_lm == lm)
	cur_lm = NULL;
    
    if (lm->fp)
	fclose (lm->fp);
    
    free (lm->ug);

    if (lm->n_bg > 0) {
	if (lm->bg)		/* Memory-based; free all bg */
	    free (lm->bg);
	else {		/* Disk-based; free in-memory bg */
	    for (i = 0; i < lm->n_ug; i++)
		if (lm->membg[i].bg)
		    free (lm->membg[i].bg);
	    free (lm->membg);
	}

	free (lm->bgprob);
    }
    
    if (lm->n_tg > 0) {
	if (lm->tg)		/* Memory-based; free all tg */
	    free (lm->tg);
	for (i = 0; i < lm->n_ug; i++) {	/* Free cached tg access info */
	    for (tginfo = lm->tginfo[i]; tginfo; tginfo = next_tginfo) {
		next_tginfo = tginfo->next;
		if ((! lm->tg) && tginfo->tg)	/* Disk-based; free in-memory tg */
		    free (tginfo->tg);
		free (tginfo);
	    }
	}
	free (lm->tginfo);

	free (lm->tgprob);
	free (lm->tgbowt);
	free (lm->tg_segbase);
    }
    
    for (i = 0; i < lm->n_ug; i++)
	free (lm->wordstr[i]);
    free (lm->wordstr);
    
    free (lm);
    free (lmset[i].name);
    
    for (; i < n_lm-1; i++)
	lmset[i] = lmset[i+1];
    --n_lm;
    
    E_INFO("LM(\"%s\") deleted\n", name);
    
    return (0);
}


/*
 * Return unscaled, raw LM score for the given weighted score.
 */
int32 lm_rawscore (int32 score, float64 lwf)
{
    lm_t *lm;
    
    lm = cur_lm;

    if (lwf != 1.0)
	score /= lwf;
    score -= lm->wip;
    score /= lm->lw;
    
    return score;
}


/*
 * Apply LW, UW, WIP.  Convert values from from prob or log10(prob) to logS3(prob).
 */
static void lm_set_param (lm_t *lm, float64 lw, float64 uw, float64 wip)
{
    int32 i, loguw, loguw_, p1, p2, loguniform;

    lm->lw = (float32) lw;
    lm->uw = (float32) uw;
    lm->wip = logs3 (wip);
    
    /* Interpolate unigram probs with uniform PDF, with weight uw */
    loguw = logs3 (uw);
    loguw_ = logs3 (1.0 - uw);
    loguniform = logs3 (1.0/(lm->n_ug-1));
    for (i = 0; i < lm->n_ug; i++) {
	if (strcmp (lm->wordstr[i], START_WORD) == 0)
	    lm->ug[i].prob.l = log10_to_logs3 (lm->ug[i].prob.f) * lw + lm->wip;
	else {
	    p1 = log10_to_logs3 (lm->ug[i].prob.f) + loguw;
	    p2 = loguniform + loguw_;
	    p1 = logs3_add (p1, p2);
	    lm->ug[i].prob.l = p1 * lw + lm->wip;
	}
	
	lm->ug[i].bowt.l = log10_to_logs3 (lm->ug[i].bowt.f) * lw;
    }

    for (i = 0; i < lm->n_bgprob; i++)
	lm->bgprob[i].l = log10_to_logs3 (lm->bgprob[i].f) * lw + lm->wip;

    if (lm->n_tg > 0) {
	for (i = 0; i < lm->n_tgprob; i++)
	    lm->tgprob[i].l = log10_to_logs3 (lm->tgprob[i].f) * lw + lm->wip;
	for (i = 0; i < lm->n_tgbowt; i++)
	    lm->tgbowt[i].l = log10_to_logs3 (lm->tgbowt[i].f) * lw;
    }
}


static int32 lm_add (lm_t *lm, char *name, float64 lw, float64 uw, float64 wip)
{
    if (lmname_to_id (name) >= 0)
	lm_delete (name);
    
    if (n_lm == n_lm_alloc) {
	if (n_lm_alloc > 0)
	    lmset = (lmset_t *) ckd_realloc (lmset, (n_lm+15)*sizeof(lmset_t));
	else
	    lmset = (lmset_t *) ckd_calloc (15, sizeof(lmset_t));
	n_lm_alloc += 15;
    }
    lmset[n_lm].lm = lm;
    lmset[n_lm].name = (char *) ckd_salloc (name);
    
    lm_set_param (lm, lw, uw, wip);
    
    n_lm++;
    
    E_INFO("LM(\"%s\") added\n", name);

    return 0;
}


static int32 lm_fread_int32 (lm_t *lm)
{
    int32 val;
    
    if (fread (&val, sizeof(int32), 1, lm->fp) != 1)
	E_FATAL("fread failed\n");
    if (lm->byteswap)
	SWAP_UINT32(val);
    return (val);
}


/*
 * Read LM dump (<lmname>.DMP) file and make it the current LM.
 * Same interface as lm_read except that the filename refers to a .DMP file.
 */
static int32 lm_read_dump (char *file, char *name)
{
    lm_t *lm;
    int32 i, j, k, vn;
    char str[1024];
    char *tmp_word_str;
    int32 notindict;
    float64 lw, uw, wip;

    lw = *((float32 *) cmd_ln_access("-langwt"));
    uw = *((float32 *) cmd_ln_access("-ugwt"));
    wip = *((float32 *) cmd_ln_access("-inspen"));
    
    lm = (lm_t *) ckd_calloc (1, sizeof(lm_t));
    cur_lm = lm;
    
    if ((lm->fp = fopen (file, "rb")) == NULL)
	E_FATAL("fopen(%s,rb) failed\n", file);
    
    /* Standard header string-size; set byteswap flag based on this */
    if (fread (&k, sizeof(int32), 1, lm->fp) != 1)
	E_FATAL("fread(%s) failed\n", file);
    if (k == strlen(darpa_hdr)+1)
	lm->byteswap = 0;
    else {
	SWAP_UINT32(k);
	if (k == strlen(darpa_hdr)+1)
	    lm->byteswap = 1;
	else {
	    SWAP_UINT32(k);
	    E_FATAL("Bad magic number: %d(%08x), not an LM dumpfile??\n", k, k);
	}
    }

    /* Read and verify standard header string */
    if (fread (str, sizeof (char), k, lm->fp) != k)
	E_FATAL("fread(%s) failed\n", file);
    if (strncmp (str, darpa_hdr, k) != 0)
	E_FATAL("Bad header\n");

    /* Original LM filename string size and string */
    k = lm_fread_int32 (lm);
    if ((k < 1) || (k > 1024))
	E_FATAL("Bad original filename size: %d\n", k);
    if (fread (str, sizeof (char), k, lm->fp) != k)
	E_FATAL("fread(%s) failed\n", file);

    /* Version#.  If present (must be <= 0); otherwise it's actually the unigram count */
    vn = lm_fread_int32 (lm);
    if (vn <= 0) {
	/* Read and skip orginal file timestamp; (later compare timestamps) */
	k = lm_fread_int32 (lm);

	/* Read and skip format description */
	for (;;) {
	    if ((k = lm_fread_int32 (lm)) == 0)
		break;
	    if (fread (str, sizeof(char), k, lm->fp) != k)
		E_FATAL("fread(%s) failed\n", file);
	}

	/* Read log_bg_seg_sz */
	if (vn <= -2) {
	    k = lm_fread_int32 (lm);
	    if ((k < 1) || (k > 15))
		E_FATAL("log2(bg_seg_sz) outside range 1..15\n", k);
	    lm->log_bg_seg_sz = k;
	} else
	    lm->log_bg_seg_sz = LOG2_BG_SEG_SZ;	/* Default */
	
	/* Read #ug */
	lm->n_ug = lm_fread_int32 (lm);
    } else {
	/* No version number, actually a unigram count */
	lm->n_ug = vn;
	lm->log_bg_seg_sz = LOG2_BG_SEG_SZ;	/* Default */
    }
    if ((lm->n_ug <= 0) || (lm->n_ug > MAX_UG))
	E_FATAL("Bad #unigrams: %d\n", lm->n_ug);

    lm->bg_seg_sz = 1 << lm->log_bg_seg_sz;
    
    /* #bigrams */
    lm->n_bg = lm_fread_int32 (lm);
    if (lm->n_bg < 0)
	E_FATAL("Bad #bigrams: %d\n", lm->n_bg);

    /* #trigrams */
    lm->n_tg = lm_fread_int32 (lm);
    if (lm->n_tg < 0)
	E_FATAL("Bad #trigrams: %d\n", lm->n_tg);

    /* Read unigrams; remember sentinel ug at the end! */
    lm->ug = (ug_t *) ckd_calloc (lm->n_ug+1, sizeof(ug_t));
    if (fread (lm->ug, sizeof(ug_t), lm->n_ug+1, lm->fp) != lm->n_ug+1)
	E_FATAL("fread(%s) failed\n", file);
    if (lm->byteswap)
	for (i = 0; i <= lm->n_ug; i++) {
	    SWAP_UINT32(lm->ug[i].dictwid);
	    SWAP_UINT32(lm->ug[i].prob.l);
	    SWAP_UINT32(lm->ug[i].bowt.l);
	    SWAP_UINT32(lm->ug[i].firstbg);
	}
    E_INFO("%8d unigrams\n", lm->n_ug);

    /* Space for in-memory bigrams/trigrams info; FOR NOW, DISK-BASED LM OPTION ONLY!! */
    lm->bg = NULL;
    lm->tg = NULL;

    /* Skip bigrams; remember sentinel at the end */
    if (lm->n_bg > 0) {
	lm->bgoff = ftell (lm->fp);
	fseek (lm->fp, (lm->n_bg+1) * sizeof(bg_t), SEEK_CUR);
	E_INFO("%8d bigrams [on disk]\n", lm->n_bg);

	lm->membg = (membg_t *) ckd_calloc (lm->n_ug, sizeof(membg_t));
    }
    
    /* Skip trigrams */
    if (lm->n_tg > 0) {
	lm->tgoff = ftell (lm->fp);
	fseek (lm->fp, lm->n_tg * sizeof(tg_t), SEEK_CUR);
	E_INFO("%8d trigrams [on disk]\n", lm->n_tg);

	lm->tginfo = (tginfo_t **) ckd_calloc (lm->n_ug, sizeof(tginfo_t *));
    }
    
    if (lm->n_bg > 0) {
	/* Bigram probs table size */
	lm->n_bgprob = lm_fread_int32 (lm);
	if ((lm->n_bgprob <= 0) || (lm->n_bgprob > 65536))
	    E_FATAL("Bad bigram prob table size: %d\n", lm->n_bgprob);
    
	/* Allocate and read bigram probs table */
	lm->bgprob = (lmlog_t *) ckd_calloc (lm->n_bgprob, sizeof (lmlog_t));
	if (fread (lm->bgprob, sizeof (lmlog_t), lm->n_bgprob, lm->fp) != lm->n_bgprob)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < lm->n_bgprob; i++)
		SWAP_UINT32(lm->bgprob[i].l);
	}

	E_INFO("%8d bigram prob entries\n", lm->n_bgprob);
    }

    if (lm->n_tg > 0) {
	/* Trigram bowt table size */
	lm->n_tgbowt = lm_fread_int32 (lm);
	if ((lm->n_tgbowt <= 0) || (lm->n_tgbowt > 65536))
	    E_FATAL("Bad trigram bowt table size: %d\n", lm->n_tgbowt);
	
	/* Allocate and read trigram bowt table */
	lm->tgbowt = (lmlog_t *) ckd_calloc (lm->n_tgbowt, sizeof (lmlog_t));
	if (fread (lm->tgbowt, sizeof (lmlog_t), lm->n_tgbowt, lm->fp) != lm->n_tgbowt)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < lm->n_tgbowt; i++)
		SWAP_UINT32(lm->tgbowt[i].l);
	}
	E_INFO("%8d trigram bowt entries\n", lm->n_tgbowt);

	/* Trigram prob table size */
	lm->n_tgprob = lm_fread_int32 (lm);
	if ((lm->n_tgprob <= 0) || (lm->n_tgprob > 65536))
	    E_FATAL("Bad trigram bowt table size: %d\n", lm->n_tgprob);
	
	/* Allocate and read trigram bowt table */
	lm->tgprob = (lmlog_t *) ckd_calloc (lm->n_tgprob, sizeof (lmlog_t));
	if (fread (lm->tgprob, sizeof (lmlog_t), lm->n_tgprob, lm->fp) != lm->n_tgprob)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < lm->n_tgprob; i++)
		SWAP_UINT32(lm->tgprob[i].l);
	}
	E_INFO("%8d trigram prob entries\n", lm->n_tgprob);

	/* Trigram seg table size */
	k = lm_fread_int32 (lm);
	if (k != (lm->n_bg+1)/lm->bg_seg_sz+1)
	    E_FATAL("Bad trigram seg table size: %d\n", k);
	
	/* Allocate and read trigram seg table */
	lm->tg_segbase = (int32 *) ckd_calloc (k, sizeof(int32));
	if (fread (lm->tg_segbase, sizeof(int32), k, lm->fp) != k)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < k; i++)
		SWAP_UINT32(lm->tg_segbase[i]);
	}
	E_INFO("%8d trigram segtable entries (%d segsize)\n", k, lm->bg_seg_sz);
    }

    /* Read word string names */
    k = lm_fread_int32 (lm);
    if (k <= 0)
	E_FATAL("Bad wordstrings size: %d\n", k);
    
    tmp_word_str = (char *) ckd_calloc (k, sizeof (char));
    if (fread (tmp_word_str, sizeof(char), k, lm->fp) != k)
	E_FATAL("fread(%s) failed\n", file);

    /* First make sure string just read contains ucount words (PARANOIA!!) */
    for (i = 0, j = 0; i < k; i++)
	if (tmp_word_str[i] == '\0')
	    j++;
    if (j != lm->n_ug)
	E_FATAL("Bad #words: %d\n", j);

    /* Break up string just read into words */
    lm->startwid = lm->endwid = -1;
    lm->wordstr = (char **) ckd_calloc (lm->n_ug, sizeof(char *));
    j = 0;
    notindict = 0;
    for (i = 0; i < lm->n_ug; i++) {
#if (! NO_DICT)
	lm->ug[i].dictwid = dict_wordid (tmp_word_str+j);
#else
	lm->ug[i].dictwid = i;
#endif
	if (IS_WID(lm->ug[i].dictwid))
	    dict2lmwid[lm->ug[i].dictwid] = i;
	else
	    notindict++;
	
	if (strcmp (tmp_word_str+j, START_WORD) == 0)
	    lm->startwid = i;
	else if (strcmp (tmp_word_str+j, FINISH_WORD) == 0)
	    lm->endwid = i;

	lm->wordstr[i] = (char *) ckd_salloc (tmp_word_str+j);
	
	j += strlen(tmp_word_str+j) + 1;
    }
    free (tmp_word_str);
    E_INFO("%8d word strings\n", i);
    if (notindict > 0)
	E_INFO("%d LM words not in dict; ignored\n", notindict);
    
    /* Force ugprob(<s>) = MIN_PROB_F */
    if (lm->startwid >= 0)
	lm->ug[lm->startwid].prob.f = MIN_PROB_F;
    else
	E_ERROR("%s not in LM\n", START_WORD);
    
    /* Force bowt(</s>) = MIN_PROB_F */
    if (lm->endwid >= 0)
	lm->ug[lm->endwid].bowt.f = MIN_PROB_F;
    else
	E_ERROR("%s not in LM\n", FINISH_WORD);
    
    lm_add (lm, name, lw, uw, wip);

    return (0);
}


int32 lm_read (char *file, char *name)
{
    int32 i;
    
    E_INFO ("Reading %s, name \"%s\"\n", file, name);

    if (! dict2lmwid) {
	/* One-time allocation of dictwid->LMwid map */
#if (! NO_DICT)
	dictsize = dict_size ();
	dict = dict_getdict();
#else
	dictsize = MAX_UG;
#endif
	dict2lmwid = (s3lmwid_t *) ckd_calloc (dictsize, sizeof(s3lmwid_t));
    }

    for (i = 0; i < dictsize; i++)
	dict2lmwid[i] = BAD_LMWID;
    
    /* For now, only dump files can be read */
    return (lm_read_dump (file, name));
}


/*
 * Free stale bigram and trigram info, those not used since last reset.
 */
void lm_cache_reset ( void )
{
    int32 i, n_bgfree, n_tgfree;
    lm_t *lm;
    tginfo_t *tginfo, *next_tginfo, *prev_tginfo;
    
    lm = cur_lm;
    n_bgfree = n_tgfree = 0;
    
    if ((lm->n_bg > 0) && (! lm->bg)) {	/* Disk-based; free "stale" bigrams */
	for (i = 0; i < lm->n_ug; i++) {
	    if (lm->membg[i].bg && (! lm->membg[i].used)) {
		lm->n_bg_inmem -= lm->ug[i+1].firstbg - lm->ug[i].firstbg;

		free (lm->membg[i].bg);
		lm->membg[i].bg = NULL;
		n_bgfree++;
	    }

	    lm->membg[i].used = 0;
	}
    }
    
    if (lm->n_tg > 0) {
	for (i = 0; i < lm->n_ug; i++) {
	    prev_tginfo = NULL;
	    for (tginfo = lm->tginfo[i]; tginfo; tginfo = next_tginfo) {
		next_tginfo = tginfo->next;
		
		if (! tginfo->used) {
		    if ((! lm->tg) && tginfo->tg) {
			lm->n_tg_inmem -= tginfo->n_tg;
			free (tginfo->tg);
			n_tgfree++;
		    }
		    
		    free (tginfo);
		    if (prev_tginfo)
			prev_tginfo->next = next_tginfo;
		    else
			lm->tginfo[i] = next_tginfo;
		} else {
		    tginfo->used = 0;
		    prev_tginfo = tginfo;
		}
	    }
	}
    }

    if ((n_tgfree > 0) || (n_bgfree > 0)) {
	E_INFO("%d tg frees, %d in mem; %d bg frees, %d in mem\n",
	       n_tgfree, lm->n_tg_inmem, n_bgfree, lm->n_bg_inmem);
    }
}


void lm_cache_stats_dump ( void )
{
    lm_t *lm;
    
    lm = cur_lm;
    
    E_INFO("%8d tg(), %d bo; %d fills, %d in mem (%.1f%%)\n",
	   lm->n_tg_score, lm->n_tg_bo, lm->n_tg_fill, lm->n_tg_inmem,
	   (lm->n_tg_inmem*100.0)/(lm->n_tg+1));
    E_INFO("%8d bg(), %d bo; %d fills, %d in mem (%.1f%%)\n",
	   lm->n_bg_score, lm->n_bg_bo, lm->n_bg_fill, lm->n_bg_inmem,
	   (lm->n_bg_inmem*100.0)/(lm->n_bg+1));
    
    lm->n_tg_fill = 0;
    lm->n_tg_score = 0;
    lm->n_tg_bo = 0;
    lm->n_bg_fill = 0;
    lm->n_bg_score = 0;
    lm->n_bg_bo = 0;
}


int32 lm_ug_score (s3wid_t wid)
{
    s3lmwid_t lwid;
    
    assert (IS_WID(wid) && (wid < dictsize));
    
    lwid = dict2lmwid[wid];
#if (! NO_DICT)
    if (NOT_LMWID(lwid))
	E_FATAL("%s not in LM\n", dict_wordstr(wid));
#endif
    return (cur_lm->ug[lwid].prob.l);
}


int32 lm_uglist (ug_t **ugptr)
{
    *ugptr = cur_lm->ug;
    return (cur_lm->n_ug);
}


/*
 * Load bigrams for the given unigram (LMWID) lw1 from disk into memory
 */
static void load_bg (lm_t *lm, s3lmwid_t lw1)
{
    int32 i, n, b;
    bg_t *bg;
    
    b = lm->ug[lw1].firstbg;		/* Absolute first bg index for ug lw1 */
    n = lm->ug[lw1+1].firstbg - b;	/* Not including guard/sentinel */
    
    bg = lm->membg[lw1].bg = (bg_t *) ckd_calloc (n+1, sizeof(bg_t));
    
    if (fseek (lm->fp, lm->bgoff + b*sizeof(bg_t), SEEK_SET) < 0)
	E_FATAL_SYSTEM ("fseek failed\n");
    
    /* Need to read n+1 because obtaining tg count for one bg also depends on next bg */
    if (fread (bg, sizeof(bg_t), n+1, lm->fp) != n+1)
	E_FATAL("fread failed\n");
    if (lm->byteswap) {
	for (i = 0; i <= n; i++) {
	    SWAP_UINT16(bg[i].wid);
	    SWAP_UINT16(bg[i].probid);
	    SWAP_UINT16(bg[i].bowtid);
	    SWAP_UINT16(bg[i].firsttg);
	}
    }
    
    lm->n_bg_fill++;
    lm->n_bg_inmem += n;
}


#define BINARY_SEARCH_THRESH	16

/* Locate a specific bigram within a bigram list */
static int32 find_bg (bg_t *bg, int32 n, s3lmwid_t w)
{
    int32 i, b, e;
    
    /* Binary search until segment size < threshold */
    b = 0;
    e = n;
    while (e-b > BINARY_SEARCH_THRESH) {
	i = (b+e)>>1;
	if (bg[i].wid < w)
	    b = i+1;
	else if (bg[i].wid > w)
	    e = i;
	else
	    return i;
    }

    /* Linear search within narrowed segment */
    for (i = b; (i < e) && (bg[i].wid != w); i++);
    return ((i < e) ? i : -1);
}


int32 lm_bglist (s3wid_t w1, bg_t **bgptr, int32 *bowt)
{
    int32 n;
    lm_t *lm;
    s3lmwid_t lw1;

    lm = cur_lm;
    
    lw1 = dict2lmwid[w1];
    assert (IS_LMWID(lw1) && (lw1 < lm->n_ug));

    n = (lm->n_bg > 0) ? lm->ug[lw1+1].firstbg - lm->ug[lw1].firstbg : 0;
    
    if (n > 0) {
	if (! lm->membg[lw1].bg)
	    load_bg (lm, lw1);
	lm->membg[lw1].used = 1;

	*bgptr = lm->membg[lw1].bg;
	*bowt = lm->ug[lw1].bowt.l;
    } else {
	*bgptr = NULL;
	*bowt = 0;
    }
    
    return (n);
}


/* w1, w2 are dictionary (base-)word ids */
static int32 lm_bg_score_nocomp (s3wid_t w1, s3wid_t w2)
{
    s3lmwid_t lw1, lw2;
    int32 i, n, score;
    lm_t *lm;
    bg_t *bg;
    
    lm = cur_lm;

#if (! NO_DICT)
    if ((lm->n_bg == 0) || (NOT_WID(w1)))
	return (lm_ug_score (w2));
#else
    if ((lm->n_bg == 0) || (NOT_LMWID(w1)))
	return (lm_ug_score (w2));
#endif

    lm->n_bg_score++;
    
    assert (IS_WID(w1) && (w1 < dictsize));
    assert (IS_WID(w2) && (w2 < dictsize));
    
    lw1 = dict2lmwid[w1];
    lw2 = dict2lmwid[w2];
#if (! NO_DICT)
    if (NOT_LMWID(lw1))
	E_FATAL("%s not in LM\n", dict_wordstr(w1));
    if (NOT_LMWID(lw2))
	E_FATAL("%s not in LM\n", dict_wordstr(w2));
#endif
    
    n = lm->ug[lw1+1].firstbg - lm->ug[lw1].firstbg;
    
    if (n > 0) {
	if (! lm->membg[lw1].bg)
	    load_bg (lm, lw1);
	lm->membg[lw1].used = 1;
	bg = lm->membg[lw1].bg;

	i = find_bg (bg, n, lw2);
    } else
	i = -1;
    
    if (i >= 0)
	score = lm->bgprob[bg[i].probid].l;
    else {
	lm->n_bg_bo++;
	score = lm->ug[lw1].bowt.l + lm->ug[lw2].prob.l;
    }

#if 0
    printf ("      %5d %5d -> %8d\n", w1, w2, score);
#endif

    return (score);
}


static void load_tg (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2)
{
    int32 i, n, b, t;
    bg_t *bg;
    tg_t *tg;
    tginfo_t *tginfo;
    
    /* First allocate space for tg information for bg lw1,lw2 */
    tginfo = (tginfo_t *) ckd_malloc (sizeof(tginfo_t));
    tginfo->w1 = lw1;
    tginfo->tg = NULL;
    tginfo->next = lm->tginfo[lw2];
    lm->tginfo[lw2] = tginfo;
    
    /* Locate bigram lw1,lw2 */

    b = lm->ug[lw1].firstbg;
    n = lm->ug[lw1+1].firstbg - b;
    
    /* Make sure bigrams for lw1, if any, loaded into memory */
    if (n > 0) {
	if (! lm->membg[lw1].bg)
	    load_bg (lm, lw1);
	lm->membg[lw1].used = 1;
	bg = lm->membg[lw1].bg;
    }

    /* At this point, n = #bigrams for lw1 */
    if ((n > 0) && ((i = find_bg (bg, n, lw2)) >= 0)) {
	tginfo->bowt = lm->tgbowt[bg[i].bowtid].l;
	
	/* Find t = Absolute first trigram index for bigram lw1,lw2 */
	b += i;			/* b = Absolute index of bigram lw1,lw2 on disk */
	t = lm->tg_segbase[b >> lm->log_bg_seg_sz];
	t += bg[i].firsttg;
	
	/* Find #tg for bigram w1,w2 */
	n = lm->tg_segbase[(b+1) >> lm->log_bg_seg_sz];
	n += bg[i+1].firsttg;
	n -= t;
	tginfo->n_tg = n;
    } else {			/* No bigram w1,w2 */
	tginfo->bowt = 0;
	n = tginfo->n_tg = 0;
    }
    
    /* At this point, n = #trigrams for lw1,lw2.  Read them in */
    if (n > 0) {
	tg = tginfo->tg = (tg_t *) ckd_calloc (n, sizeof(tg_t));
	if (fseek (lm->fp, lm->tgoff + t*sizeof(tg_t), SEEK_SET) < 0)
	    E_FATAL_SYSTEM("fseek failed\n");

	if (fread (tg, sizeof(tg_t), n, lm->fp) != n)
	    E_FATAL("fread(tg, %d at %d) failed\n", n, lm->tgoff);
	if (lm->byteswap) {
	    for (i = 0; i < n; i++) {
		SWAP_UINT16(tg[i].wid);
		SWAP_UINT16(tg[i].probid);
	    }
	}
    }
    
    lm->n_tg_fill++;
    lm->n_tg_inmem += n;
}


/* Similar to find_bg */
static int32 find_tg (tg_t *tg, int32 n, s3lmwid_t w)
{
    int32 i, b, e;
    
    b = 0;
    e = n;
    while (e-b > BINARY_SEARCH_THRESH) {
	i = (b+e)>>1;
	if (tg[i].wid < w)
	    b = i+1;
	else if (tg[i].wid > w)
	    e = i;
	else
	    return i;
    }
    
    for (i = b; (i < e) && (tg[i].wid != w); i++);
    return ((i < e) ? i : -1);
}


int32 lm_tglist (s3wid_t w1, s3wid_t w2, tg_t **tgptr, int32 *bowt)
{
    tginfo_t *tginfo, *prev_tginfo;
    lm_t *lm;
    s3lmwid_t lw1, lw2;

    lm = cur_lm;
    
    if (lm->n_tg <= 0) {
	*tgptr = NULL;
	*bowt = 0;
	return 0;
    }
    
    lw1 = dict2lmwid[w1];
    lw2 = dict2lmwid[w2];
    assert (IS_LMWID(lw1) && (lw1 < lm->n_ug));
    assert (IS_LMWID(lw2) && (lw2 < lm->n_ug));

    prev_tginfo = NULL;
    for (tginfo = lm->tginfo[lw2]; tginfo; tginfo = tginfo->next) {
	if (tginfo->w1 == lw1)
	    break;
	prev_tginfo = tginfo;
    }
    
    if (! tginfo) {
    	load_tg (lm, lw1, lw2);
	tginfo = lm->tginfo[lw2];
    } else if (prev_tginfo) {
	prev_tginfo->next = tginfo->next;
	tginfo->next = lm->tginfo[lw2];
	lm->tginfo[lw2] = tginfo;
    }
    tginfo->used = 1;

    *tgptr = tginfo->tg;
    *bowt = tginfo->bowt;

    return (tginfo->n_tg);
}


/* w1, w2, w3 are dictionary wids */
static int32 lm_tg_score_nocomp (s3wid_t w1, s3wid_t w2, s3wid_t w3)
{
    s3lmwid_t lw1, lw2, lw3;
    int32 i, n, score;
    lm_t *lm;
    tg_t *tg;
    tginfo_t *tginfo, *prev_tginfo;
    
    lm = cur_lm;
    
#if (! NO_DICT)
    if ((lm->n_tg == 0) || (NOT_WID(w1)))
	return (lm_bg_score_nocomp (w2, w3));
#else
    if ((lm->n_tg == 0) || (NOT_LMWID(w1)))
	return (lm_bg_score_nocomp (w2, w3));
#endif

    lm->n_tg_score++;

    assert (IS_WID(w1) && (w1 < dictsize));
    assert (IS_WID(w2) && (w2 < dictsize));
    assert (IS_WID(w3) && (w3 < dictsize));
    
    lw1 = dict2lmwid[w1];
    lw2 = dict2lmwid[w2];
    lw3 = dict2lmwid[w3];
#if (! NO_DICT)
    if (NOT_LMWID(lw1))
	E_FATAL("%s not in LM\n", dict_wordstr(w1));
    if (NOT_LMWID(lw2))
	E_FATAL("%s not in LM\n", dict_wordstr(w2));
    if (NOT_LMWID(lw3))
	E_FATAL("%s not in LM\n", dict_wordstr(w3));
#endif
    
    prev_tginfo = NULL;
    for (tginfo = lm->tginfo[lw2]; tginfo; tginfo = tginfo->next) {
	if (tginfo->w1 == lw1)
	    break;
	prev_tginfo = tginfo;
    }
    
    if (! tginfo) {
    	load_tg (lm, lw1, lw2);
	tginfo = lm->tginfo[lw2];
    } else if (prev_tginfo) {
	prev_tginfo->next = tginfo->next;
	tginfo->next = lm->tginfo[lw2];
	lm->tginfo[lw2] = tginfo;
    }

    tginfo->used = 1;
    
    /* Trigrams for w1,w2 now in memory; look for w1,w2,w3 */
    n = tginfo->n_tg;
    tg = tginfo->tg;
    if ((i = find_tg (tg, n, lw3)) >= 0)
	score = lm->tgprob[tg[i].probid].l;
    else {
	lm->n_tg_bo++;
	score = tginfo->bowt + lm_bg_score_nocomp (w2, w3);
    }

#if 0
    printf ("%5d %5d %5d -> %8d\n", w1, w2, w3, score);
#endif

    return (score);
}


int32 lm_bg_score (s3wid_t w1, s3wid_t w2)
{
    lm_t *lm;
    s3wid_t w[32];	/* HACK!! Assuming compound words are no longer than this */
    int32 i, n;
    int32 lscr;
    
    lm = cur_lm;
    
#if (NO_DICT)
    return (lm_bg_score_nocomp (w1, w2));
#else
    assert (dict->word[w2].n_comp < 30);	/* Ensure we don't overflow w[32] */
    
    /* Obtain the 2-word history; check if w1 is a compound word */
    if (IS_WID(w1)) {
	n = dict->word[w1].n_comp;
	if (n > 0) {
	    assert (n > 1);
	    w[1] = dict->word[w1].comp[n-1].wid;
	    w[0] = dict->word[w1].comp[n-2].wid;
	} else {
	    w[1] = w1;
	    w[0] = BAD_WID;
	}
    } else {
	w[1] = BAD_WID;
	w[0] = BAD_WID;
    }
    
    /* Break up w2 into component words if it's a compound word */
    n = dict->word[w2].n_comp;
    if (n > 0) {
	for (i = 0; i < n; i++)
	    w[i+2] = dict->word[w2].comp[i].wid;
    } else {
	w[2] = w2;
	n = 1;
    }
    
    /* Find LM score for w3 */
    lscr = 0;
    for (i = 0; i < n; i++)
	lscr += lm_tg_score_nocomp (w[i], w[i+1], w[i+2]);
    
    return lscr;
#endif
}


/*
 * With compound word handling:  Find the score for transition to w3 (which may be a
 * compound word), given the history w1,w2, either or both of which may also be compound
 * words.
 */
int32 lm_tg_score (s3wid_t w1, s3wid_t w2, s3wid_t w3)
{
    lm_t *lm;
    s3wid_t w[32];	/* HACK!! Assuming compound words are no longer than this */
    int32 i, n;
    int32 lscr;
    
    lm = cur_lm;
    
#if (NO_DICT)
    return (lm_tg_score_nocomp (w1, w2, w3));
#else
    assert (dict->word[w3].n_comp < 30);	/* Ensure we don't overflow w[32] */
    
    if (NOT_WID(w2)) {
	assert (NOT_WID(w1));
	return lm_bg_score (w2, w3);
    }
    
    /* Obtain the 2-word history; first check if w2 is a compound word, then w1 */
    n = dict->word[w2].n_comp;
    if (n > 0) {
	assert (n > 1);
	w[1] = dict->word[w2].comp[n-1].wid;
	w[0] = dict->word[w2].comp[n-2].wid;
    } else {
	w[1] = w2;
	if (IS_WID(w1)) {
	    n = dict->word[w1].n_comp;
	    if (n > 0) {
		assert (n > 1);
		w[0] = dict->word[w1].comp[n-1].wid;
	    } else
		w[0] = w1;
	} else
	    w[0] = BAD_WID;
    }

    /* Break up w3 into component words if it's a compound word */
    n = dict->word[w3].n_comp;
    if (n > 0) {
	for (i = 0; i < n; i++)
	    w[i+2] = dict->word[w3].comp[i].wid;
    } else {
	w[2] = w3;
	n = 1;
    }
    
    /* Find LM score for w3 */
    lscr = 0;
    for (i = 0; i < n; i++)
	lscr += lm_tg_score_nocomp (w[i], w[i+1], w[i+2]);
    
    return lscr;
#endif
}


s3lmwid_t lm_lmwid (s3wid_t w)
{
    assert (IS_WID(w));
    return (dict2lmwid[w]);
}


s3wid_t lm_dictwid (s3lmwid_t w)
{
    assert (IS_LMWID(w) && (w < cur_lm->n_ug));
    return (cur_lm->ug[w].dictwid);
}


int32 lm_tgprob (tg_t *tg)
{
    return (cur_lm->tgprob[tg->probid].l);
}


int32 lm_bgprob (bg_t *bg)
{
    return (cur_lm->bgprob[bg->probid].l);
}


lm_t *lm_current ( void )
{
    return cur_lm;
}


#if (NO_DICT)
int32 lm_wid (char *word)
{
    lm_t *lm;
    int32 i;
    
    lm = cur_lm;
    for (i = 0; i < lm->n_ug; i++)
	if (strcmp (lm->wordstr[i], word) == 0)
	    return i;
    return -1;
}


static int32 sentence_lmscore (char *line)
{
    int32 wid[3];
    char *_lp;
    int32 linelen;
    char word[1024];
    int32 k, score, tgscr;
    lm_t *model;
    
    model = cur_lm;
    
    wid[0] = wid[1] = BAD_LMWID;
    wid[2] = lm_wid ("<s>");
    score = 0;
    
    _lp = line;
    while (sscanf (_lp, "%s%n", word, &linelen) == 1) {
	_lp += linelen;
	
	k = strlen (word) - 1;
	if (word[k] == ')') {
	    for (--k; (k > 0) && (word[k] != '('); --k);
	    if (k > 0)
		word[k] = '\0';
	}
	
	wid[0] = wid[1];
	wid[1] = wid[2];
	if ((wid[2] = lm_wid (word)) < 0) {
	    if ((wid[2] = lm_wid ("<UNK>")) < 0) {
		fprintf (stderr, "%s(%d): Unknown word: %s, skipping sentence\n",
			 __FILE__, __LINE__, word);
		return (0);
	    } else
		fprintf (stderr, "%s(%d): Unknown word: %s, using <UNK>\n",
			 __FILE__, __LINE__, word);
	}
	
	tgscr = lm_tg_score (wid[0], wid[1], wid[2]);
#if 0
	printf ("\t[%s,%s,%s] = %d\n",
		NOT_LMWID(wid[0]) ? "--" : model->wordstr[wid[0]],
		NOT_LMWID(wid[1]) ? "--" : model->wordstr[wid[1]],
		NOT_LMWID(wid[2]) ? "--" : model->wordstr[wid[2]],
		tgscr);
#endif
	score += tgscr;
    }

    return (score);
}


main (int32 argc, char *argv[])
{
    char line[4096];
    int32 score, k;
    
    if (argc < 2)
	E_FATAL("Usage: %s <LMdumpfile>\n", argv[0]);

    lm_read (argv[1], "", 9.5, 0.5, 0.65);

    for (;;) {
	printf ("> ");
	if (fgets (line, sizeof(line), stdin) == NULL)
	    break;
	
	score = sentence_lmscore (line);

	k = strlen(line);
	if (line[k-1] == '\n')
	    line[k-1] = '\0';
	printf ("LMScr(%s) = %d\n", line, score);
    }
}
#endif
