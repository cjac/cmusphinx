/*
 * lm.c -- Word trigram LM module.
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
 * 07-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define MIN_PROB_F	((float32)-99.0)


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
static lm_t *lm_read_dump (char *file, float64 lw, float64 uw, float64 wip)
{
    lm_t *lm;
    int32 i, j, k, vn;
    char str[1024];
    char *tmp_word_str;
    int32 notindict;
    
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

	/* Read #ug */
	lm->n_ug = lm_fread_int32 (lm);
    } else {
	/* No version number, actually a unigram count */
	lm->n_ug = vn;
    }
    if ((lm->n_ug <= 0) || (lm->n_ug > MAX_UG))
	E_FATAL("Bad #unigrams: %d\n", lm->n_ug);

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
	if (k != (lm->n_bg+1)/BG_SEG_SZ+1)
	    E_FATAL("Bad trigram seg table size: %d\n", k);
	
	/* Allocate and read trigram seg table */
	lm->tg_segbase = (int32 *) ckd_calloc (k, sizeof(int32));
	if (fread (lm->tg_segbase, sizeof(int32), k, lm->fp) != k)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < k; i++)
		SWAP_UINT32(lm->tg_segbase[i]);
	}
	E_INFO("%8d trigram segtable entries\n", k);
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
	    lm->dict2lmwid[lm->ug[i].dictwid] = i;
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
    
    /* Force bowt(</s>) = MIN_PROB_F */
    lm->ug[lm->endwid].bowt.f = MIN_PROB_F;
    
    /* Force ugprob(<s>) = MIN_PROB_F */
    lm->ug[lm->startwid].prob.f = MIN_PROB_F;

    return (0);
}


int32 lm_read (char *file, float64 lw, float64 uw, float64 wip)
{
    int32 i, dictsize;
    lm_t *lm;
    
    E_INFO ("Reading %s, lw %.3f, uw %.3f, wip %.3f\n",
	    file, lw, uw, wip);

#if (! NO_DICT)
    dictsize = dict_size ();
#else
    dictsize = MAX_UG;
#endif
    lm->dict2lmwid = (s3lmwid_t *) ckd_calloc (dictsize, sizeof(s3lmwid_t));

    for (i = 0; i < dictsize; i++)
	lm->dict2lmwid[i] = BAD_LMWID;
    
    /* For now, only dump files can be read */
    lm = lm_read_dump (file, lw, uw, wip);
    lm_set_param (lm, lw, uw, wip);
    
    return lm;
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
