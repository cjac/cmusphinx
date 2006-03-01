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
 * lm_3g.c -- Darpa Trigram LM module (adapted to Sphinx 3)
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
 * Revision 1.3  2006/03/01  20:05:09  arthchan2003
 * Pretty format the lm dumping and make numbers in 4 decimals only.
 * 
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert lm to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.4  2006/01/16 19:58:25  arthchan2003
 * Small change to make function public.
 *
 * Revision 1.1.2.3  2005/11/17 06:21:05  arthchan2003
 * 1, Change all lm_write_arpa_* functions' s3wid_t to s3lmwid_t.  2, Make the writing code to be changed more easily.
 *
 * Revision 1.1.2.2  2005/07/28 20:04:20  dhdfu
 * Make LM writing not crash for bigram LM
 *
 * Revision 1.1.2.1  2005/07/13 01:39:55  arthchan2003
 * Added lm_3g.c, which take cares of reading and writing of LM into the lm_t (3-gram specific lm structure.)
 *
 */

#include "lm.h"
#include "bio.h"
#include "logs3.h"
#include "wid.h"

#define MAX_SORTED_ENTRIES	65534
#define FIRST_BG(m,u)		((m)->ug[u].firstbg)
#define TSEG_BASE(m,b)		((m)->tg_segbase[(b)>>LOG2_BG_SEG_SZ])
#define FIRST_TG(m,b)		(TSEG_BASE((m),(b))+((m)->bg[b].firsttg))

static int32 wstr2wid (lm_t *model, /**< an LM */
		       char *w      /**< a pre-allocated word string */
		       )
{
    int32 val;
    
    if (hash_lookup (model->HT, w, &val) != 0)
	return NO_WORD;
    return ((int32) val);
}

/*
 * Initialize sorted list with the 0-th entry = MIN_PROB_F, which may be needed
 * to replace spurious values in the Darpa LM file.
 */
static void init_sorted_list (sorted_list_t *l /**< a sorted list */
			      )
{
    l->list =
	(sorted_entry_t *) ckd_calloc (MAX_SORTED_ENTRIES, sizeof (sorted_entry_t));
    l->list[0].val.f = MIN_PROB_F;
    l->list[0].lower = 0;
    l->list[0].higher = 0;
    l->free = 1;
}

/**
 * Free a sorted list 
 */
static void free_sorted_list (sorted_list_t *l /**< a sorted list */
			      )
{
    free (l->list);
}

static lmlog_t *vals_in_sorted_list (sorted_list_t *l)
{
    lmlog_t *vals;
    int32 i;
    
    vals = (lmlog_t *) ckd_calloc (l->free, sizeof (lmlog_t));
    for (i = 0; i < l->free; i++)
	vals[i].f = l->list[i].val.f;
    return (vals);
}

static int32 sorted_id (sorted_list_t *l, float *val)
{
    int32 i = 0;
    
    for (;;) {
	if (*val == l->list[i].val.f)
	    return (i);
	if (*val < l->list[i].val.f) {
	    if (l->list[i].lower == 0) {
		if (l->free >= MAX_SORTED_ENTRIES)
		    E_INFO("sorted list overflow\n");
		l->list[i].lower = l->free;
		(l->free)++;
		i = l->list[i].lower;
		l->list[i].val.f = *val;
		return (i);
	    } else
		i = l->list[i].lower;
	} else {
	    if (l->list[i].higher == 0) {
		if (l->free >= MAX_SORTED_ENTRIES)
		    E_INFO("sorted list overflow\n");
		l->list[i].higher = l->free;
		(l->free)++;
		i = l->list[i].higher;
		l->list[i].val.f = *val;
		return (i);
	    } else
		i = l->list[i].higher;
	}
    }
}

/**
 * Read and return #unigrams, #bigrams, #trigrams as stated in input file.
 */
static void ReadNgramCounts (FILE *fp,
			     int32 *n_ug,  /**< Out: number of unigram read */
			     int32 *n_bg,  /**< Out: number of bigram read */
			     int32 *n_tg   /**< Out: number of trigram read */
			     )
{
  char string[256];
  int32 ngram, ngram_cnt;
  
  /* skip file until past the '\data\' marker */


  do{
    fgets (string, sizeof (string), fp);
  }while ( (strcmp (string, "\\data\\\n") != 0) && (! feof (fp)) );

  
  if (strcmp (string, "\\data\\\n") != 0)
    E_FATAL("No \\data\\ mark in LM file\n");

	
  *n_ug = *n_bg = *n_tg = 0;
  while (fgets (string, sizeof (string), fp) != NULL) {
    if (sscanf (string, "ngram %d=%d", &ngram, &ngram_cnt) != 2)
      break;

    switch (ngram) {
    case 1: *n_ug = ngram_cnt;
      break;
    case 2: *n_bg = ngram_cnt;
      break;
    case 3: *n_tg = ngram_cnt;
      break;
    default:
      E_FATAL("Unknown ngram (%d)\n", ngram);
      break;
    }
  }
    
  /* Position file to just after the unigrams header '\1-grams:\' */
  while ( (strcmp (string, "\\1-grams:\n") != 0) && (! feof (fp)) )
    fgets (string, sizeof (string), fp);
    
  /* Check counts;  NOTE: #trigrams *CAN* be 0 */
  if ((*n_ug <= 0) || (*n_bg <= 0) || (*n_tg < 0))
    E_FATAL("Bad or missing ngram count\n");

}

ug_t *NewUnigramTable (int32 n_ug)
{
    ug_t *table;
    int32 i;

    table = (ug_t *) ckd_calloc (n_ug, sizeof (ug_t));
    for (i = 0; i < n_ug; i++) {
	table[i].dictwid = NO_WORD;
	table[i].prob.f = -99.0;
	table[i].bowt.f = -99.0;
    }
    return table;
}

static lm_t * NewModel (int32 n_ug, int32 n_bg, int32 n_tg)
{
    lm_t *model;

    model = (lm_t *) ckd_calloc (1, sizeof (lm_t));

    /*
     * Allocate one extra unigram and bigram entry: sentinels to terminate
     * followers (bigrams and trigrams, respectively) of previous entry.
     */
    model->ug	= NewUnigramTable (n_ug+1);
    model->bg	= (bg_t *) ckd_calloc (n_bg+1, sizeof (bg_t));
    if (n_tg > 0)
      model->tg	= (tg_t *) ckd_calloc (n_tg+1, sizeof (tg_t));

    if (n_tg > 0) {
	model->tg_segbase = (int32 *) ckd_calloc ((n_bg+1)/BG_SEG_SZ+1, sizeof (int32));
#if 0
	E_INFO("%8d = tseg_base entries allocated\n",
	       (n_bg+1)/BG_SEG_SZ+1);
#endif
    }

    /*This part will not be compiled */
    model->max_ug = model->n_ug = n_ug;
    model->n_bg = n_bg;
    model->n_tg = n_tg;

    model->HT=hash_new(model->n_ug,HASH_CASE_YES);
    model->log_bg_seg_sz = LOG2_BG_SEG_SZ;	/* Default */

    return model;
}

/*
 * Read in the unigrams from given file into the LM structure model.  On
 * entry to this procedure, the file pointer is positioned just after the
 * header line '\1-grams:'.
 */
static void ReadUnigrams (FILE *fp, 
			  lm_t *model /**< An LM where unigram will be filled in */
			  )
{
    char string[256];
    char name[128];
    int32 wcnt,i;
    float p1, bo_wt;
    s3lmwid_t startwid, endwid;
    
    E_INFO ("Reading unigrams\n");

    wcnt = 0;
    while ((fgets (string, sizeof(string), fp) != NULL) &&
	   (strcmp (string, "\\2-grams:\n") != 0))
    {
	if (sscanf (string, "%f %s %f", &p1, name, &bo_wt) != 3) {
	    if (string[0] != '\n')
		E_WARN ("Format error; unigram ignored:%s",string);
	    continue;
	}
	
	if (wcnt >= model->n_ug)
	    E_FATAL("Too many unigrams\n");

	/* Associate name with word id */
	/* This is again not local */
	model->wordstr[wcnt] = (char *) ckd_salloc (name);
	hash_enter (model->HT, model->wordstr[wcnt], wcnt);
	model->ug[wcnt].prob.f = p1;
	model->ug[wcnt].bowt.f = bo_wt;

	model->ug[wcnt].dictwid = wcnt;
	wcnt++;
    }

    if (model->n_ug != wcnt) {
	E_WARN ("lm_t.n_ug(%d) != #unigrams read(%d)\n",
		model->n_ug, wcnt);
	model->n_ug = wcnt;
    }

    startwid = endwid = BAD_S3LMWID;

    for(i=0;i< model->n_ug ;i++){
      if (strcmp (model->wordstr[i], S3_START_WORD) == 0)
	startwid = i;
      else if (strcmp (model->wordstr[i], S3_FINISH_WORD) == 0)
	endwid = i;
    }

    /* Force ugprob(<s>) = MIN_PROB_F */
    if (IS_S3LMWID(startwid)) {
	model->ug[startwid].prob.f = MIN_PROB_F;
	model->startlwid = startwid;
    }
    
    /* Force bowt(</s>) = MIN_PROB_F */
    if (IS_S3LMWID(endwid)) {
	model->ug[endwid].bowt.f = MIN_PROB_F;
	model->finishlwid = endwid;
    }

}

/*
 * Read bigrams from given file into given model structure.  File may be arpabo
 * or arpabo-id format, depending on idfmt = 0 or 1.
 */
static void ReadBigrams (FILE *fp, lm_t *model, int32 idfmt)
{
    char string[1024], word1[256], word2[256];
    int32 w1, w2, prev_w1, bgcount, p;
    bg_t *bgptr;
    float p2, bo_wt;
    int32 n_fld, n;
    
    E_INFO ("Reading bigrams\n");
    
    bgcount = 0;
    bgptr = model->bg;
    prev_w1 = -1;
    n_fld = (model->n_tg > 0) ? 4 : 3;

    bo_wt = 0.0;
    while (fgets (string, sizeof(string), fp) != NULL) {
	if (! idfmt)
	    n = sscanf (string, "%f %s %s %f", &p2, word1, word2, &bo_wt);
	else
	    n = sscanf (string, "%f %d %d %f", &p2, &w1, &w2, &bo_wt);
	if (n < n_fld) {
	    if (string[0] != '\n')
		break;
	    continue;
	}

	if (! idfmt) {
	    if ((w1 = wstr2wid (model, word1)) == NO_WORD)
		E_FATAL("Unknown word: %s\n",word1);
	    if ((w2 = wstr2wid (model, word2)) == NO_WORD)
		E_FATAL("Unknown word: %s\n",word2);
	} else {
	    if ((w1 >= model->n_ug) || (w2 >= model->n_ug) || (w1 < 0) || (w2 < 0))
	      E_FATAL("Bad bigram: %s", string);
	}
	
	/* HACK!! to quantize probs to 4 decimal digits */
	p = p2*10000;
	p2 = p*0.0001;
	p = bo_wt*10000;
	bo_wt = p*0.0001;

	if (bgcount >= model->n_bg)
	  E_INFO("Too many bigrams\n");
	
	bgptr->wid = w2;
	bgptr->probid = sorted_id (&(model->sorted_prob2), &p2);
	if (model->n_tg > 0)
	    bgptr->bowtid = sorted_id (&(model->sorted_bowt2), &bo_wt);

	if (w1 != prev_w1) {
	    if (w1 < prev_w1)
	      E_INFO("Bigrams not in unigram order\n");
	    
	    for (prev_w1++; prev_w1 <= w1; prev_w1++)
		model->ug[prev_w1].firstbg = bgcount;
	    prev_w1 = w1;
	}
	
	bgcount++;
	bgptr++;

	if ((bgcount & 0x0000ffff) == 0) {
	    E_INFO_NOFN ("Processing bigram .\n");
	}
    }
    if ((strcmp (string, "\\end\\\n") != 0) && (strcmp (string, "\\3-grams:\n") != 0))
      E_FATAL("Bad bigram: %s\n", string);
    
    for (prev_w1++; prev_w1 <= model->n_ug; prev_w1++)
	model->ug[prev_w1].firstbg = bgcount;
}
 
/*
 * Very similar to ReadBigrams.
 */
static void ReadTrigrams (FILE *fp, lm_t *model, int32 idfmt)
{
    char string[1024], word1[256], word2[256], word3[256];
    int32 i, n, w1, w2, w3, prev_w1, prev_w2, tgcount, prev_bg, bg, endbg, p;
    int32 seg, prev_seg, prev_seg_lastbg;
    tg_t *tgptr;
    bg_t *bgptr;
    float p3;
    
    E_INFO ("Reading trigrams\n");
    
    tgcount = 0;
    tgptr = model->tg;
    prev_w1 = -1;
    prev_w2 = -1;
    prev_bg = -1;
    prev_seg = -1;

    while (fgets (string, sizeof(string), fp) != NULL) {
	if (! idfmt)
	    n = sscanf (string, "%f %s %s %s", &p3, word1, word2, word3);
	else
	    n = sscanf (string, "%f %d %d %d", &p3, &w1, &w2, &w3);
	if (n != 4) {
	    if (string[0] != '\n')
		break;
	    continue;
	}

	if (! idfmt) {
	    if ((w1 = wstr2wid (model, word1)) == NO_WORD)
		E_FATAL("Unknown word: %s\n",word1);
	    if ((w2 = wstr2wid (model, word2)) == NO_WORD)
		E_FATAL("Unknown word: %s\n",word2);
	    if ((w3 = wstr2wid (model, word3)) == NO_WORD)
		E_FATAL("Unknown word: %s\n",word3);
	} else {
	    if ((w1 >= model->n_ug) || (w2 >= model->n_ug) || (w3 >= model->n_ug) ||
		    (w1 < 0) || (w2 < 0) || (w3 < 0))
	      E_FATAL("Bad trigram: %s\n",string);
	}
	
	/* HACK!! to quantize probs to 4 decimal digits */
	p = p3*10000;
	p3 = p*0.0001;

	if (tgcount >= model->n_tg)
	    E_INFO("Too many trigrams\n");
	
	tgptr->wid = w3;
	tgptr->probid = sorted_id (&model->sorted_prob3, &p3);

	if ((w1 != prev_w1) || (w2 != prev_w2)) {
	    /* Trigram for a new bigram; update tg info for all previous bigrams */
	    if ((w1 < prev_w1) || ((w1 == prev_w1) && (w2 < prev_w2)))
	      E_INFO("Trigrams not in bigram order\n");
		      
	    
	    bg = (w1 != prev_w1) ? model->ug[w1].firstbg : prev_bg+1;
	    endbg = model->ug[w1+1].firstbg;
	    bgptr = model->bg + bg;
	    for (; (bg < endbg) && (bgptr->wid != w2); bg++, bgptr++);
	    if (bg >= endbg)
	      E_FATAL("Missing bigram for trigram: %s",string);

	    /* bg = bigram entry index for <w1,w2>.  Update tseg_base */
	    seg = bg >> LOG2_BG_SEG_SZ;
	    for (i = prev_seg+1; i <= seg; i++)
		model->tg_segbase[i] = tgcount;

	    /* Update trigrams pointers for all bigrams until bg */
	    if (prev_seg < seg) {
		int32 tgoff = 0;

		if (prev_seg >= 0) {
		    tgoff = tgcount - model->tg_segbase[prev_seg];
		    if (tgoff > 65535)
		      E_FATAL( "Offset from tseg_base > 65535\n");
		}
		
		prev_seg_lastbg = ((prev_seg+1) << LOG2_BG_SEG_SZ) - 1;
		bgptr = model->bg + prev_bg;
		for (++prev_bg, ++bgptr; prev_bg <= prev_seg_lastbg; prev_bg++, bgptr++)
		    bgptr->firsttg = tgoff;
		
		for (; prev_bg <= bg; prev_bg++, bgptr++)
		    bgptr->firsttg = 0;
	    } else {
		int32 tgoff;

		tgoff = tgcount - model->tg_segbase[prev_seg];
		if (tgoff > 65535)
		  E_FATAL("Offset from tseg_base > 65535\n");
		
		bgptr = model->bg + prev_bg;
		for (++prev_bg, ++bgptr; prev_bg <= bg; prev_bg++, bgptr++)
		    bgptr->firsttg = tgoff;
	    }
	    
	    prev_w1 = w1;
	    prev_w2 = w2;
	    prev_bg = bg;
	    prev_seg = seg;
	}
	
	tgcount++;
	tgptr++;

	if ((tgcount & 0x0000ffff) == 0) {
	    E_INFO_NOFN ("Processing trigram .\n");
	}
    }
    if (strcmp (string, "\\end\\\n") != 0)
	E_FATAL("Bad trigram: %s\n", string);
    
    for (prev_bg++; prev_bg <= model->n_bg; prev_bg++) {
	if ((prev_bg & (BG_SEG_SZ-1)) == 0)
	    model->tg_segbase[prev_bg >> LOG2_BG_SEG_SZ] = tgcount;
	if ((tgcount - model->tg_segbase[prev_bg >> LOG2_BG_SEG_SZ]) > 65535)
	    E_FATAL("Offset from tseg_base > 65535\n");
	model->bg[prev_bg].firsttg =
	    tgcount - model->tg_segbase[prev_bg >> LOG2_BG_SEG_SZ];
    }
}


lm_t *lm_read_txt(const char *filename,  /**< The file name*/
		  int32 lminmemory
		  )
{
  lm_t *model;
  FILE *fp = NULL;
  int32 usingPipe = FALSE;
  int32 n_unigram;
  int32 n_bigram;
  int32 n_trigram;
  int32 idfmt=0;

  E_INFO ("Reading LM file %s (name \"%s\")\n",filename);


  fp=fopen_comp(filename,"r",&usingPipe);
  if(fp==NULL){
    E_FATAL("failed to read filename for LM\n");
  }


  ReadNgramCounts (fp, &n_unigram, &n_bigram, &n_trigram);


  E_INFO ("ngrams 1=%d, 2=%d, 3=%d\n",n_unigram, n_bigram, n_trigram); 
  /* HACK! This should be something provided by the dictionary What is dict_size? */
  model=NewModel(n_unigram,n_bigram,n_trigram);
  model->n_ng=1;

  assert(lminmemory==1);
  model->isLM_IN_MEMORY = lminmemory;


  if(model->n_bg>0){
    model->n_ng=2;
    model->membg = (membg_t *) ckd_calloc (model->n_ug, sizeof(membg_t));
  }
  if(model->n_tg>0){
    model->n_ng=3;
    model->tginfo = (tginfo_t **) ckd_calloc (model->n_ug, sizeof(tginfo_t *));
  }
  
  
  /* Have to put it somewhere in lm as a kind of buffer */
  model->wordstr = (char **) ckd_calloc (n_unigram, sizeof (char *));

  /* control the lm dumping mechanism */
  
  ReadUnigrams (fp, model);
  E_INFO("%8d = #unigrams created\n", model->n_ug);
	
  init_sorted_list (&(model->sorted_prob2));
  if (model->n_tg > 0)
    init_sorted_list (&(model->sorted_bowt2));

  ReadBigrams (fp, model, idfmt);
	
  model->n_bg = FIRST_BG(model,model->n_ug);
  model->n_bgprob = model->sorted_prob2.free;
  model->bgprob  = vals_in_sorted_list (&(model->sorted_prob2));
  free_sorted_list (&(model->sorted_prob2));

  E_INFO("\n%8d = #bigrams created\n", model->n_bg);
  E_INFO("%8d = #prob2 entries\n", model->n_bgprob);

  if (model->n_tg > 0) {
    /* Create trigram bo-wts array */
    model->n_tgbowt = model->sorted_bowt2.free;
    model->tgbowt = vals_in_sorted_list (&(model->sorted_bowt2));
    free_sorted_list (&(model->sorted_bowt2));
    E_INFO("%8d = #bo_wt2 entries\n", model->n_tgbowt);
    
    init_sorted_list (&(model->sorted_prob3));
    
    ReadTrigrams (fp, model, idfmt);
    
    model->n_tg = FIRST_TG(model,model->n_bg);
    model->n_tgprob = model->sorted_prob3.free;
    model->tgprob  = vals_in_sorted_list (&(model->sorted_prob3));
    E_INFO("\n%8d = #trigrams created\n", model->n_tg);
    E_INFO("%8d = #prob3 entries\n", model->n_tgprob);
    
    free_sorted_list (&model->sorted_prob3);
  }

  return model;
}


static char const *txtheader[] = {
  "#############################################################################",
  "#Copyright (c) 1999-2004 Carnegie Mellon University.  All rights reserved",
  "#############################################################################",
  "#=============================================================================",
  "#===============  This file was produced by the CMU Sphinx 3.X  ==============",
  "#=============================================================================",
  "#############################################################################",
  "This file is in the ARPA-standard format introduced by Doug Paul.",
  "",
  "p(wd3|wd1,wd2)= if(trigram exists)           p_3(wd1,wd2,wd3)",
  "                else if(bigram w1,w2 exists) bo_wt_2(w1,w2)*p(wd3|wd2)",
  "                else                         p(wd3|w2)",
  "",
  "p(wd2|wd1)= if(bigram exists) p_2(wd1,wd2)",
  "            else              bo_wt_1(wd1)*p_1(wd2)",
  "",
  "All probs and back-off weights (bo_wt) are given in log10 form.",
  "\\",
  "Data formats:",
  "",
  "Beginning of data mark: \\data\\",
  NULL
};

/**
   Write an arpa LM header (Sphinx 3.x -specific)
 */
static void lm_write_arpa_header(lm_t *lmp, /**< The LM pointer */
				 FILE *fp   /**< The file pointer */
				 )
{
  /* Print header */
  int32 i,j;
  for (i = 0; txtheader[i] != NULL; i++) 
    fprintf (fp,"%s\n",txtheader[i]);

  for (i=1;i<=lmp->n_ng;i++) {
    fprintf(fp,"ngram %d=nr            # number of %d-grams\n",i,i);
  }
  fprintf(fp,"\n");
  for (i=1;i<=lmp->n_ng;i++) {
    fprintf(fp,"\\%d-grams:\n",i);
    fprintf(fp,"p_%d     ",i);
    for (j=1;j<=i;j++) {
      fprintf(fp,"wd_%d ",j);
    }
    if (i == lmp->n_ng) {
      fprintf(fp,"\n");
    }
    else {
      fprintf(fp,"bo_wt_%d\n",i);
    }
  } 

  fprintf(fp,"\n");
  fprintf(fp,"end of data mark: \\end\\\n");
  fprintf(fp,"\n");
}

/**
   Write the n-gram counts for ARPA file format. 
 */
static void lm_write_arpa_count(lm_t *lmp, /**< The LM pointer */
				FILE* fp   /**< The file pointer */
				)
{
  fprintf(fp,"\\data\\\n");

  fprintf(fp,"ngram %d=%d\n",1,lmp->n_ug);
  if(lmp->n_bg)
    fprintf(fp,"ngram %d=%d\n",2,lmp->n_bg);
  if(lmp->n_tg)
    fprintf(fp,"ngram %d=%d\n",3,lmp->n_tg);

  fprintf(fp,"\n");
 
}

/**
   Write unigram
 */

static void lm_write_arpa_unigram(lm_t *lmp, /**< The LM pointer */
				  FILE* fp   /**< The file pointer */
				  )
{
  int32 i;
  fprintf(fp,"\\1-grams:\n");
  for(i=0;i<lmp->n_ug;i++){
    fprintf(fp,"%.4f ",lmp->ug[i].prob.f);
    fprintf(fp,"%s",lmp->wordstr[i]);
    fprintf(fp," ");
    fprintf(fp,"%.4f\n",lmp->ug[i].bowt.f);
  }
  fprintf(fp,"\n");
}

/**
   Write bigram in arpa format
 */

static void lm_write_arpa_bigram(lm_t *lmp,
				 FILE* fp
				 )
{
  int32 i,j;
  int32 n,b;
  s3lmwid_t lw1, lw2;
  
  /*  s3wid_t w1;*/
  uint16 probid;
  uint16 bowtid;
  fprintf(fp,"\\2-grams:\n");

  for(i=0;i<=lmp->n_ug-1;i++){
    b=lmp->ug[i].firstbg;
    n=lmp->ug[i+1].firstbg;
    
    for(j=b;j<n;j++){
      lw1=i;
      lw2=lmp->bg[j].wid;
      probid=lmp->bg[j].probid;
      bowtid=lmp->bg[j].bowtid;

      fprintf(fp,"%.4f ",lmp->bgprob[probid].f);
      fprintf(fp,"%s",lmp->wordstr[lw1]);
      fprintf(fp," ");
      fprintf(fp,"%s",lmp->wordstr[lw2]);

      if (lmp->tgbowt){
	fprintf(fp," ");
	fprintf(fp,"%.4f\n",lmp->tgbowt[bowtid].f);
      }
      else
	fprintf(fp,"\n");

      /*
	if (lmp->tgbowt)
	fprintf(fp,"%.4f %s %s %.4f\n",lmp->bgprob[probid].f,
	lmp->wordstr[lw1],lmp->wordstr[lw2],lmp->tgbowt[bowtid].f);
	else
	fprintf(fp,"%.4f %s %s\n",lmp->bgprob[probid].f,
		lmp->wordstr[lw1],lmp->wordstr[lw2]);
      */
    }
  }

  fprintf(fp,"\n");
}

/**
   Write trigram in arpa format
 */

static void lm_write_arpa_trigram(lm_t *lmp, /**< The pointer of LM */
				  FILE* fp   /**< A FILE pointer */
				  )
{
  int32 i,j,k;
  int32 b_bg,n_bg;
  int32 b_tg,n_tg;
  s3lmwid_t lw1,lw2,lw3;
  uint32 probid;

  fprintf(fp,"\\3-grams:\n");
  for(i=0;i<=lmp->n_ug-1;i++){
    b_bg=lmp->ug[i].firstbg;
    n_bg=lmp->ug[i+1].firstbg;

    for(j=b_bg;j<= n_bg-1 ; j++){

      b_tg=lmp->tg_segbase[j >> lmp->log_bg_seg_sz] + lmp->bg[j].firsttg;
      n_tg=lmp->tg_segbase[(j+1) >> lmp->log_bg_seg_sz] + lmp->bg[j+1].firsttg;

      for(k=b_tg;k<n_tg;k++){
	lw1=i;
	lw2=lmp->bg[j].wid;
	lw3=lmp->tg[k].wid;
	probid=lmp->tg[k].probid;
	fprintf(fp,"%.4f ",lmp->tgprob[probid].f);
	fprintf(fp,"%s",lmp->wordstr[lw1]);
	fprintf(fp," ");
	fprintf(fp,"%s",lmp->wordstr[lw2]);
	fprintf(fp," ");
	fprintf(fp,"%s",lmp->wordstr[lw3]);
	fprintf(fp,"\n");

	/*
	  fprintf(fp,"%.4f %s %s %s\n",lmp->tgprob[probid].f,lmp->wordstr[lw1],lmp->wordstr[lw2],lmp->wordstr[lw3]);
	*/
      }
    }
  }

}

/**
   Write the end for an arpa file format
 */
static void lm_write_arpa_end(lm_t *lmp, /**< The LM pointer */
			      FILE *fp   /**< The FILE Pointer */
			      )
{
  fprintf(fp,"\\end\\\n");
}

/**
 * Write an LM with ARPA file format 
 */
int32 lm_write_arpa_text(lm_t *lmp,
			 const char* outputfn
			 )
{
  FILE *fp;

  E_INFO ("Dumping LM to %s\n", outputfn);
  if ((fp = fopen (outputfn, "w")) == NULL) {
    E_ERROR ("Cannot create file %s\n",outputfn);
    return LM_FAIL;
  }

  lm_write_arpa_header(lmp,fp);
  lm_write_arpa_count(lmp,fp);
  lm_write_arpa_unigram(lmp,fp);
  if(lmp->n_bg>0)
    lm_write_arpa_bigram(lmp,fp);
  if(lmp->n_tg>0)
    lm_write_arpa_trigram(lmp,fp);
  lm_write_arpa_end(lmp,fp);

  fclose(fp);
  return LM_SUCCESS;
}


