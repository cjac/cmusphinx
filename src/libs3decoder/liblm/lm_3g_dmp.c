/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

/*
 * lm_3g_dmp.c -- DMP format LM manipulation.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.4  2006/03/03  00:42:36  egouvea
 * In bio.h, definition of REVERSE_SWAP_... depends on WORDS_BIGENDIAN,
 * since __BIG_ENDIAN__ isn't defined.
 * 
 * In lm_3g_dmp.c, swap bigram and trigram values if needed.
 * 
 * In lm_convert regresssion test, allow for tolerance (< 0.0002) when
 * comparing the results.
 * 
 * Revision 1.3  2006/03/02 00:35:08  arthchan2003
 * Merged the logic in share/lm3g2dmp to here.  It will take care the situation when log_bg_seg_sz is different. (Must be an old format Ravi played with in the past). This will match the reading code also generalize the old sphinx 2's logic a little bit.
 *
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert lm to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.1  2005/07/17 05:23:25  arthchan2003
 * added lm_3g_dmp.c and lmset.c, split it out from lm.c to avoid overcrowding situation in it.
 *
 *
 *
 */

#include <lm.h>
#include <s3types.h>
#include <bio.h>

const char *darpa_hdr = "Darpa Trigram LM";

static void fwrite_int32 (FILE *fp, int32 val)
{
    REVERSE_SENSE_SWAP_INT32(val);
    fwrite (&val, sizeof(int32), 1, fp);
}

static void fwrite_ug (FILE *fp, ug_t *ug)
{
    ug_t tmp_ug = *ug;
    
    REVERSE_SENSE_SWAP_INT32(tmp_ug.dictwid);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.prob.l);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.bowt.l);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.firstbg);
    fwrite (&tmp_ug, sizeof(ug_t), 1, fp);
}

static void fwrite_bg (FILE *fp, bg_t *bg)
{
    bg_t tmp_bg = *bg;
    
    REVERSE_SENSE_SWAP_INT16(tmp_bg.wid);
    REVERSE_SENSE_SWAP_INT16(tmp_bg.probid);
    REVERSE_SENSE_SWAP_INT16(tmp_bg.bowtid);
    REVERSE_SENSE_SWAP_INT16(tmp_bg.firsttg);
    fwrite (&tmp_bg, sizeof(bg_t), 1, fp);
}

static void fwrite_tg (FILE *fp, tg_t *tg)
{
    tg_t tmp_tg = *tg;
    
    REVERSE_SENSE_SWAP_INT16(tmp_tg.wid);
    REVERSE_SENSE_SWAP_INT16(tmp_tg.probid);
    fwrite (&tmp_tg, sizeof(tg_t), 1, fp);
}

static char const *fmtdesc[] = {
    "BEGIN FILE FORMAT DESCRIPTION",
    "Header string length (int32) and string (including trailing 0)",
    "Original LM filename string-length (int32) and filename (including trailing 0)",
    "(int32) version number (present iff value <= 0)",
    "(int32) original LM file modification timestamp (iff version# present)",
    "(int32) string-length and string (including trailing 0) (iff version# present)",
    "... previous entry continued any number of times (iff version# present)",
    "(int32) 0 (terminating sequence of strings) (iff version# present)",
    "(int32) log_bg_seg_sz (present iff different from default value of LOG2_BG_SEG_SZ)",
    "(int32) lm_t.ucount (must be > 0)",
    "(int32) lm_t.bcount",
    "(int32) lm_t.tcount",
    "lm_t.ucount+1 unigrams (including sentinel)",
    "lm_t.bcount+1 bigrams (including sentinel)",
    "lm_t.tcount trigrams (present iff lm_t.tcount > 0)",
    "(int32) lm_t.n_prob2",
    "(int32) lm_t.prob2[]",
    "(int32) lm_t.n_bo_wt2 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.bo_wt2[] (present iff lm_t.tcount > 0)",
    "(int32) lm_t.n_prob3 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.prob3[] (present iff lm_t.tcount > 0)",
    "(int32) (lm_t.bcount+1)/BG_SEG_SZ+1 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.tseg_base[] (present iff lm_t.tcount > 0)",
    "(int32) Sum(all word string-lengths, including trailing 0 for each)",
    "All word strings (including trailing 0 for each)",
    "END FILE FORMAT DESCRIPTION",
    NULL,
};

int32 lm3g_dump (char const *file,  /**< the file name */
		 lm_t *model,       /**< the langauge model for output */
		 char const *lmfile,  /**< the */
		 int32 mtime  /**< LM file modification date */
		 )
{
    int32 i, k;
    FILE *fp;

    E_INFO ("Dumping LM to %s\n", file);
    if ((fp = fopen (file, "wb")) == NULL) {
	E_ERROR ("Cannot create file %s\n",file);
	return LM_FAIL;
    }

    k = strlen(darpa_hdr)+1;
    fwrite_int32 (fp, k);
    fwrite (darpa_hdr, sizeof(char), k, fp);

    k = strlen(lmfile)+1;
    fwrite_int32 (fp, k);
    fwrite (lmfile, sizeof(char), k, fp);

    /* Write version# and LM file modification date */
    if (model->log_bg_seg_sz != LOG2_BG_SEG_SZ){	/* Hack!! */
      E_WARN("log_bg_seg_sz is different from default");
      fwrite_int32 (fp, -2);	/* version # */
    }else
      fwrite_int32 (fp, -1);	/* version # */

    fwrite_int32 (fp, mtime);
    
    /* Write file format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	k = strlen(fmtdesc[i])+1;
	fwrite_int32 (fp, k);
	fwrite (fmtdesc[i], sizeof(char), k, fp);
    }
    fwrite_int32 (fp, 0);

    /* HACK!! Write only if different from previous version */
    if (model->log_bg_seg_sz != LOG2_BG_SEG_SZ)
	fwrite_int32 (fp, model->log_bg_seg_sz);
    
    fwrite_int32 (fp, model->n_ug);
    fwrite_int32 (fp, model->n_bg);
    fwrite_int32 (fp, model->n_tg);

    for (i = 0; i <= model->n_ug; i++)
	fwrite_ug (fp, &(model->ug[i]));
    for (i = 0; i <= model->n_bg; i++)
	fwrite_bg (fp, &(model->bg[i]));
    for (i = 0; i < model->n_tg; i++)
	fwrite_tg (fp, &(model->tg[i]));
    
    fwrite_int32 (fp, model->n_bgprob);
    for (i = 0; i < model->n_bgprob; i++)
	fwrite_int32 (fp, model->bgprob[i].l);
    
    if (model->n_tg > 0) {
	fwrite_int32 (fp, model->n_tgbowt);
	for (i = 0; i < model->n_tgbowt; i++)
	    fwrite_int32 (fp, model->tgbowt[i].l);
	fwrite_int32 (fp, model->n_tgprob);
	for (i = 0; i < model->n_tgprob; i++)
	    fwrite_int32 (fp, model->tgprob[i].l);
	
	k = (model->n_bg+1)/BG_SEG_SZ + 1;
	fwrite_int32 (fp, k);
	for (i = 0; i < k; i++)
	    fwrite_int32 (fp, model->tg_segbase[i]);
    }
    
    k = 0;
    for (i = 0; i < model->n_ug; i++)
	k += strlen(model->wordstr[i])+1;
    fwrite_int32 (fp, k);
    for (i = 0; i < model->n_ug; i++)
	fwrite (model->wordstr[i], sizeof(char), strlen(model->wordstr[i])+1, fp);

    fclose (fp);
    return LM_SUCCESS;
}


static int32 lm_fread_int32 (lm_t *lm)
{
    int32 val;
    
    if (fread (&val, sizeof(int32), 1, lm->fp) != 1)
	E_FATAL("fread failed\n");
    if (lm->byteswap)
	SWAP_INT32(&val);
    return (val);
}


/**
 * Read LM dump (<lmname>.DMP) file and make it the current LM.
 * Same interface as lm_read except that the filename refers to a .DMP file.
 */
lm_t *lm_read_dump (const char *file, /**< The file name*/
		    int lminmemory /**< Whether using in memory LM */
		    )
{
    lm_t *lm;
    int32 i, j, k, vn;
    char str[1024];
    char *tmp_word_str;
    s3lmwid_t startwid, endwid;

    lm = (lm_t *) ckd_calloc (1, sizeof(lm_t));
    
    lm->isLM_IN_MEMORY = lminmemory;
    lm->n_ng=1;

    if ((lm->fp = fopen (file, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file);
    
    /* Standard header string-size; set byteswap flag based on this */
    if (fread (&k, sizeof(int32), 1, lm->fp) != 1)
	E_FATAL("fread(%s) failed\n", file);
    if ((size_t)k == strlen(darpa_hdr)+1)
	lm->byteswap = 0;
    else {
	SWAP_INT32(&k);
	if ((size_t)k == strlen(darpa_hdr)+1)
	    lm->byteswap = 1;
	else {
	  SWAP_INT32(&k);
	  E_INFO("Bad magic number: %d(%08x), not an LM dumpfile??\n", k, k);
	  return NULL;
	}
    }

    /* Read and verify standard header string */
    if (fread (str, sizeof (char), k, lm->fp) != (size_t)k)
	E_FATAL("fread(%s) failed\n", file);
    if (strncmp (str, darpa_hdr, k) != 0)
	E_FATAL("Bad header\n");

    /* Original LM filename string size and string */
    k = lm_fread_int32 (lm);
    if ((k < 1) || (k > 1024))
	E_FATAL("Bad original filename size: %d\n", k);
    if (fread (str, sizeof (char), k, lm->fp) != (size_t)k)
	E_FATAL("fread(%s) failed\n", file);

    /* Version#.  If present (must be <= 0); otherwise it's actually the unigram count */
    vn = lm_fread_int32 (lm);
    if (vn <= 0) {

	/* Read and skip orginal file timestamp; 
	   ARCHAN: Unlike the sphinx2's code, currently, the timestamp
	   is not compared in Sphinx 3. 
	 */
	k = lm_fread_int32 (lm);

	/* Read and skip format description */
	for (;;) {
	    if ((k = lm_fread_int32 (lm)) == 0)
		break;
	    if (fread (str, sizeof(char), k, lm->fp) != (size_t)k)
		E_FATAL("fread(%s) failed\n", file);
	}

	/* Read log_bg_seg_sz if present */
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
	/* oldest dump file version has no version# or any of the above */
	if (vn > lm->n_ug)
	    E_FATAL("LM.ucount(%d) out of range [1..%d]\n", vn, lm->n_ug);

	/* No version number, actually a unigram count */
	lm->n_ug = vn;
	lm->log_bg_seg_sz = LOG2_BG_SEG_SZ;	/* Default */
    }
    if ((lm->n_ug <= 0) || (lm->n_ug >= MAX_S3LMWID))
	E_FATAL("Bad #ug: %d (must be >0, <%d\n", lm->n_ug, MAX_S3LMWID);

    lm->bg_seg_sz = 1 << lm->log_bg_seg_sz;

    /* #bigrams */
    lm->n_bg = lm_fread_int32 (lm);
    if (lm->n_bg < 0)
	E_FATAL("Bad #bigrams: %d\n", lm->n_bg);

    /* #trigrams */
    lm->n_tg = lm_fread_int32 (lm);
    if (lm->n_tg < 0)
	E_FATAL("Bad #trigrams: %d\n", lm->n_tg);

    /* Read ug; remember sentinel ug at the end! */
    lm->ug = (ug_t *) ckd_calloc (lm->n_ug+1, sizeof(ug_t));
    if (fread (lm->ug, sizeof(ug_t), lm->n_ug+1, lm->fp) != (size_t)(lm->n_ug+1))
	E_FATAL("fread(%s) failed\n", file);
    if (lm->byteswap)
	for (i = 0; i <= lm->n_ug; i++) {
	    SWAP_INT32(&(lm->ug[i].prob.l));
	    SWAP_INT32(&(lm->ug[i].bowt.l));
	    SWAP_INT32(&(lm->ug[i].firstbg));
	}
    E_INFO("%8d ug\n", lm->n_ug);

    if(lm->n_bg> 0)
      lm->n_ng=2;

    if(lm->n_tg> 0)
      lm->n_ng=3;

    /* RAH, 5.1.01 - Let's try reading the whole damn thing in here   */
    if (lm->isLM_IN_MEMORY) {
      lm->bg = (bg_t *) ckd_calloc (lm->n_bg+1,sizeof(bg_t));
      lm->tg = (tg_t *) ckd_calloc (lm->n_tg+1,sizeof(tg_t));

      if (lm->n_bg > 0) {       /* Read bigrams; remember sentinel at the end */
	lm->bgoff = ftell (lm->fp);
	fread (lm->bg, lm->n_bg+1,sizeof(bg_t),lm->fp);
	if (lm->byteswap)
	  for (i = 0; i <= lm->n_bg; i++) {
	    SWAP_INT16(&(lm->bg[i].wid));
	    SWAP_INT16(&(lm->bg[i].probid));
	    SWAP_INT16(&(lm->bg[i].bowtid));
	    SWAP_INT16(&(lm->bg[i].firsttg));
	  }
	E_INFO("Read %8d bigrams [in memory]\n", lm->n_bg);
      }
      
      if (lm->n_tg > 0) {       /* Read trigrams */
	lm->tgoff = ftell (lm->fp);
	fread (lm->tg,lm->n_tg,sizeof(tg_t),lm->fp);
	if (lm->byteswap)
	  for (i = 0; i <= lm->n_tg; i++) {
	    SWAP_INT16(&(lm->tg[i].wid));
	    SWAP_INT16(&(lm->tg[i].probid));
	  }
	E_INFO("Read %8d trigrams [in memory]\n", lm->n_tg);
      }
    } else {
      lm->bg = NULL;
      lm->tg = NULL;
      
      /* Skip bigrams; remember sentinel at the end */
      if (lm->n_bg > 0) {
	lm->bgoff = ftell (lm->fp);
	fseek (lm->fp, (lm->n_bg+1) * sizeof(bg_t), SEEK_CUR);
	E_INFO("%8d bigrams [on disk]\n", lm->n_bg);
      }
      
      /* Skip trigrams */
      if (lm->n_tg > 0) {
	lm->tgoff = ftell (lm->fp);
	fseek (lm->fp, lm->n_tg * sizeof(tg_t), SEEK_CUR);
	E_INFO("%8d trigrams [on disk]\n", lm->n_tg);	
      }
    }

    if(lm->n_bg>0)
      lm->membg = (membg_t *) ckd_calloc (lm->n_ug, sizeof(membg_t));
    if(lm->n_tg>0)
      lm->tginfo = (tginfo_t **) ckd_calloc (lm->n_ug, sizeof(tginfo_t *));

    if (lm->n_bg > 0) {
	/* Bigram probs table size */
	lm->n_bgprob = lm_fread_int32 (lm);
	if ((lm->n_bgprob <= 0) || (lm->n_bgprob > 65536))
	    E_FATAL("Bad bigram prob table size: %d\n", lm->n_bgprob);
	
	/* Allocate and read bigram probs table */
	lm->bgprob = (lmlog_t *) ckd_calloc (lm->n_bgprob, sizeof (lmlog_t));
	if (fread(lm->bgprob, sizeof(lmlog_t), lm->n_bgprob, lm->fp) !=
	    (size_t)lm->n_bgprob)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < lm->n_bgprob; i++)
		SWAP_INT32(&(lm->bgprob[i].l));
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
	if (fread (lm->tgbowt, sizeof (lmlog_t), lm->n_tgbowt, lm->fp) !=
	    (size_t)lm->n_tgbowt)
	  E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	  for (i = 0; i < lm->n_tgbowt; i++)
	    SWAP_INT32(&(lm->tgbowt[i].l));
	}
	E_INFO("%8d trigram bowt entries\n", lm->n_tgbowt);

	/* Trigram prob table size */
	lm->n_tgprob = lm_fread_int32 (lm);
	if ((lm->n_tgprob <= 0) || (lm->n_tgprob > 65536))
	  E_FATAL("Bad trigram bowt table size: %d\n", lm->n_tgprob);
	
	/* Allocate and read trigram bowt table */
	lm->tgprob = (lmlog_t *) ckd_calloc (lm->n_tgprob, sizeof (lmlog_t));
	if (fread (lm->tgprob, sizeof (lmlog_t), lm->n_tgprob, lm->fp) !=
	    (size_t)lm->n_tgprob)
	  E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	  for (i = 0; i < lm->n_tgprob; i++)
	    SWAP_INT32(&(lm->tgprob[i].l));
	}
	E_INFO("%8d trigram prob entries\n", lm->n_tgprob);

	/* Trigram seg table size */
	k = lm_fread_int32 (lm);
	if (k != (lm->n_bg+1)/lm->bg_seg_sz+1)
	    E_FATAL("Bad trigram seg table size: %d\n", k);
	
	/* Allocate and read trigram seg table */
	lm->tg_segbase = (int32 *) ckd_calloc (k, sizeof(int32));
	if (fread (lm->tg_segbase, sizeof(int32), k, lm->fp) != (size_t)k)
	    E_FATAL("fread(%s) failed\n", file);
	if (lm->byteswap) {
	    for (i = 0; i < k; i++)
		SWAP_INT32(&(lm->tg_segbase[i]));
	}
	E_INFO("%8d trigram segtable entries (%d segsize)\n", k, lm->bg_seg_sz);
    }

    /* Read word string names */
    k = lm_fread_int32 (lm);
    if (k <= 0)
	E_FATAL("Bad wordstrings size: %d\n", k);
    
    tmp_word_str = (char *) ckd_calloc (k, sizeof (char));
    if (fread (tmp_word_str, sizeof(char), k, lm->fp) != (size_t)k)
	E_FATAL("fread(%s) failed\n", file);

    /* First make sure string just read contains n_ug words (PARANOIA!!) */
    for (i = 0, j = 0; i < k; i++)
	if (tmp_word_str[i] == '\0')
	    j++;
    if (j != lm->n_ug)
	E_FATAL("Bad #words: %d\n", j);

    /* Break up string just read into words */
    startwid = endwid = BAD_S3LMWID;
    lm->wordstr = (char **) ckd_calloc (lm->n_ug, sizeof(char *));
    j = 0;
    for (i = 0; i < lm->n_ug; i++) {
	if (strcmp (tmp_word_str+j, S3_START_WORD) == 0)
	    startwid = i;
	else if (strcmp (tmp_word_str+j, S3_FINISH_WORD) == 0)
	    endwid = i;

	lm->wordstr[i] = (char *) ckd_salloc (tmp_word_str+j);
	
	j += strlen(tmp_word_str+j) + 1;
    }
    free (tmp_word_str);
    E_INFO("%8d word strings\n", i);
    
    /* Force ugprob(<s>) = MIN_PROB_F */
    if (IS_S3LMWID(startwid)) {
	lm->ug[startwid].prob.f = MIN_PROB_F;
	lm->startlwid = startwid;
    }
    
    /* Force bowt(</s>) = MIN_PROB_F */
    if (IS_S3LMWID(endwid)) {
	lm->ug[endwid].bowt.f = MIN_PROB_F;
	lm->finishlwid = endwid;
    }



    return lm;
}
