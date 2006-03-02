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
 * lm.c -- Disk-based backoff word trigram LM module.
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
 * Revision 1.19  2006/03/02  22:11:56  arthchan2003
 * Fixed dox-doc.
 * 
 * Revision 1.18  2006/03/01 20:03:55  arthchan2003
 * Do encoding conversion when the encodings are different. This will avoid a lot of weird characters.
 *
 * Revision 1.17  2006/02/24 13:38:08  arthchan2003
 * Added lm_read, it is a simple version of lm_read_advance.
 *
 * Revision 1.16  2006/02/23 04:16:29  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH:
 * Splited the original lm.c into five parts,
 * a, lm.c - a controller of other subroutines.
 * b, lm_3g.c - implement TXT-based lm operations
 * c, lm_3g_dmp.c - implement DMP-based lm operations
 * d, lm_attfsm.c - implement FSM-based lm operations
 * e, lmset.c - implement sets of lm.
 *
 *
 * Revision 1.14.4.9  2006/01/16 19:56:37  arthchan2003
 * 1, lm_rawscore doesn't need a language weight, 2, Support dumping the LM in FST format.  This code used Yannick Esteve's and LIUM code.
 *
 * Revision 1.14.4.8  2005/11/17 06:18:49  arthchan2003
 * Added a string encoding conversion routine in lm.c. Currently it only works for converting hex to its value.
 *
 * Revision 1.14.4.7  2005/10/17 04:49:13  arthchan2003
 * Free resource of lm_t and lmset_t correctly.
 *
 * Revision 1.14.4.6  2005/09/07 23:30:26  arthchan2003
 * Changed error message for LM dump.
 *
 * Revision 1.14.4.5  2005/08/02 21:10:18  arthchan2003
 * Added function declaration for lm_read_dump.
 *
 * Revision 1.14.4.4  2005/07/17 05:24:23  arthchan2003
 * (Incomplete) Added lm_arbitrary.[ch], an arbitrary n-gram data structure.  Far from completed. Don't expect too much.
 *
 * Revision 1.14.4.3  2005/07/13 01:44:17  arthchan2003
 * 1, Moved text formatted LM code into lm_3g.c, 2 Changed lm_read such that it will work with both TXT file format and DMP file format. 3,  Added function lm_write to handle lm writing.
 *
 * Revision 1.14.4.2  2005/07/05 21:31:25  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.15  2005/07/05 13:12:37  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.14.4.1  2005/07/03 22:58:56  arthchan2003
 * tginfo and membg 's memory were not deallocated at all. This change fixed it.
 *
 * Revision 1.14  2005/06/21 22:24:02  arthchan2003
 * Log. In this change, I introduced a new interface for lm ,which is
 * call lmset_t. lmset_t wraps up multiple lm, n_lm, n_alloclm into the
 * same structure and handle LM initialization (lm_init) switching,
 * (lmset_curlm_widx), delete LM (lmset_delete_lm).  The internal
 * structure is called lmarray and is an array of pointers of lm.  The
 * current lm is always maintained and pointed by a pointer called cur_lm
 * . This substantially clarify the structure of the code.  At this
 * check-in, not every core function of lmset is completed.
 * e.g. lmset_add_lm because that required testing of several LM reading
 * routines and could be quite time-consuming.
 *
 * Log. Another notable change is the fact dict2lmwid map is started to
 * be part of the LM. The reason of this is clearly described inside the
 * code. Don't want to repeat here.
 *
 * Log. The new interface has been already used broadly in both Sphinx
 * 3.0 and sphinx 3.x family of tools.
 *
 * Revision 1.4  2005/06/18 03:22:28  archan
 * Add lmset_init. A wrapper function of various LM initialization and initialize an lmset It is now used in decode, livepretend, dag and astar.
 *
 * Revision 1.3  2005/06/17 23:44:40  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.2  2005/05/10 21:21:53  archan
 * Three functionalities added but not tested. Code on 1) addition/deletion of LM in mode 4. 2) reading text-based LM 3) Converting txt-based LM to dmp-based LM.
 *
 * Revision 1.1  2005/05/04 06:08:07  archan
 * Refactor all lm routines except fillpen.c into ./libs3decoder/liblm/ . This will be equivalent to ./lib/liblm in future.
 *
 * Revision 1.6  2005/05/04 04:02:24  archan
 * Implementation of lm addition, deletion in (mode 4) time-switching tree implementation of search.  Not yet tested. Just want to keep up my own momentum.
 *
 * Revision 1.5  2005/04/20 03:37:59  archan
 * LM code changes: functions are added to set, add and delete LM from the lmset, change the legacy lmset data structure to contain n_lm and n_alloc_lm.
 *
 * Revision 1.4  2005/03/30 16:28:34  archan
 * delete test-full.log alog
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Adding lm_free() to free allocated memory
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Removed language weight application to wip. To maintain
 *		comparability between s3decode and current decoder. Does
 *		not affect decoding performance.
 *
 * 23-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: Applied language weight to word insertion penalty.
 * 
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.access_type; made lm_wid externally visible.
 * 
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.log_bg_seg_sz and lm_t.bg_seg_sz.
 * 
 * 13-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Creating from original S3 version.
 */


#include "lm.h"
#include "bio.h"
#include "logs3.h"
#include "wid.h"

/*ARCHAN, 20041112: NOP, NO STATIC VARIABLES! */

extern lm_t *lm_read_txt(const char *filename,
			 const int lminmemory /**< Whether using in memory LM */
			 );

extern lm_t *lm_read_dump (const char *file, /**< The file name*/
			   int lminmemory /**< Whether using in memory LM */
			   );


int32 lm3g_dump (char const *file,  /**< the file name */
		 lm_t *model,       /**< the langauge model for output */
		 char const *lmfile,  /**< the */
		 int32 mtime  /**< LM file modification date */
		 );

/**
   Writer of lm in ARPA text format
 */
int32 lm_write_arpa_text(lm_t *lmp,/**< the pointer of the language model */
			 const char* outputfn, /**< the output file name */
			 const char* inputenc, /**< The input encoding method */
			 const char* outputenc /**< The output encoding method */
			 );

/**
   Writer of lm in FST format
 */

int32 lm_write_att_fsm(lm_t *lm,  /**< the languauge model pointer */
		      const char *filename /**< output file name */
		      );


int32 lm_get_classid (lm_t *model, char *name)
{
    int32 i;
    
    if (! model->lmclass)
	return BAD_LMCLASSID;
    
    for (i = 0; i < model->n_lmclass; i++) {
	if (strcmp (lmclass_getname(model->lmclass[i]), name) == 0)
	    return (i + LM_CLASSID_BASE);
    }
    return BAD_LMCLASSID;
}


#if 0 /* ARCHAN 20050715: Make it permanently removed */
int32 lm_delete (lm_t *lm,lmset_t *lmset)
{
#if 0 /* ARCHAN 20050419: This part of the code is obsolete */
    int32 i;
    tginfo_t *tginfo, *next_tginfo;
    
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
#endif    
    
    E_INFO("Warning, lm_delete is currently empty, no memory is deleted\n");

    return (0);
}
#endif



/* Apply unigram weight; should be part of LM creation, but... */
static void lm_uw (lm_t *lm, float64 uw)
{
    int32 i, loguw, loguw_, loguniform, p1, p2;

    /* Interpolate unigram probs with uniform PDF, with weight uw */
    loguw = logs3 (uw);
    loguw_ = logs3 (1.0 - uw);
    loguniform = logs3 (1.0/(lm->n_ug-1));	/* Skipping S3_START_WORD */
    
    for (i = 0; i < lm->n_ug; i++) {
	if (strcmp (lm->wordstr[i], S3_START_WORD) != 0) {
	    p1 = lm->ug[i].prob.l + loguw;
	    p2 = loguniform + loguw_;
	    lm->ug[i].prob.l = logs3_add (p1, p2);
	}
    }
}


static void lm2logs3 (lm_t *lm, float64 uw)
{
    int32 i;

    for (i = 0; i < lm->n_ug; i++) {
	lm->ug[i].prob.l = log10_to_logs3 (lm->ug[i].prob.f);
	lm->ug[i].bowt.l = log10_to_logs3 (lm->ug[i].bowt.f);
    }
    
    lm_uw (lm, uw);
    
    for (i = 0; i < lm->n_bgprob; i++)
	lm->bgprob[i].l = log10_to_logs3 (lm->bgprob[i].f);

    if (lm->n_tg > 0) {
	for (i = 0; i < lm->n_tgprob; i++)
	    lm->tgprob[i].l = log10_to_logs3 (lm->tgprob[i].f);
	for (i = 0; i < lm->n_tgbowt; i++)
	    lm->tgbowt[i].l = log10_to_logs3 (lm->tgbowt[i].f);
    }
}


void lm_set_param (lm_t *lm, float64 lw, float64 wip)
{
    int32 i, iwip;
    float64 f;
    
    if (lw <= 0.0)
	E_FATAL("lw = %e\n", lw);
    if (wip <= 0.0)
	E_FATAL("wip = %e\n", wip);
#if 0 /* No lang weight on wip */
    iwip = logs3(wip) * lw; 
#endif
    iwip = logs3(wip);
    
    f = lw / lm->lw;
    
    for (i = 0; i < lm->n_ug; i++) {
	lm->ug[i].prob.l = (int32)((lm->ug[i].prob.l - lm->wip) * f) + iwip;
	lm->ug[i].bowt.l = (int32)(lm->ug[i].bowt.l * f);
    }

    for (i = 0; i < lm->n_bgprob; i++)
	lm->bgprob[i].l = (int32)((lm->bgprob[i].l - lm->wip) * f) + iwip;

    if (lm->n_tg > 0) {
	for (i = 0; i < lm->n_tgprob; i++)
	    lm->tgprob[i].l = (int32)((lm->tgprob[i].l - lm->wip) * f) + iwip;
	for (i = 0; i < lm->n_tgbowt; i++)
	    lm->tgbowt[i].l = (int32)(lm->tgbowt[i].l * f);
    }

    lm->lw = (float32) lw;
    lm->wip = iwip;
}



lm_t * lm_read(const char *file, const char *lmname)
{
  return lm_read_advance(file,
			 lmname,
			 cmd_ln_float32("-lw"),
			 cmd_ln_float32("-wip"),
			 cmd_ln_float32("-uw"),
			 0,
			 NULL,
			 1);
}

lm_t *lm_read_advance (const char *file, const char *lmname, float64 lw, float64 wip, float64 uw, int32 ndict, char* fmt, int32 applyWeight)
{
    int32 i, u;
    lm_t *lm;
      
    if (! file)
	E_FATAL("No LM file\n");
    if (lw <= 0.0)
	E_FATAL("lw = %e\n", lw);
    if (wip <= 0.0)
	E_FATAL("wip = %e\n", wip);
    if ((uw < 0.0) || (uw > 1.0))
	E_FATAL("uw = %e\n", uw);

    /* HACK: At this part, one should check whether the LM name is being used already */
    
    E_INFO ("LM read('%s', lw= %.2f, wip= %.2f, uw= %.2f)\n", file, lw, wip, uw);
    E_INFO ("Reading LM file %s (LM name \"%s\")\n",file,lmname);

    /* First it will try to decide whether the file a .DMP file */
    /* ARCHAN: We should provide function pointer implementation at here. */
    if(fmt==NULL){ /**Automatically decide the LM format */
      lm = lm_read_dump (file,(int32) cmd_ln_int32("-lminmemory"));
      if(lm == NULL){
	E_INFO("In lm_read, LM is not a DMP file. Trying reading it as a txt file\n");
	
	/* HACK !!! */
	if(cmd_ln_int32("-lminmemory")==0){
	  E_INFO("LM is not a dump file, so it is assumed to be a text file. However, disk-based LM is not working for -lminmemory=0 at this point (i.e. LM has to be loaded into the memory). Forced exit.\n");
	  return NULL;
	}
	lm = lm_read_txt (file,cmd_ln_int32("-lminmemory"));
	if(lm ==NULL){
	  E_INFO("In lm_read, LM is also not in text format.\n");
	  return NULL;
	}
      }
    }else if (!strcmp(fmt,"TXT")){
      lm=lm_read_txt (file,cmd_ln_int32("-lminmemory"));
      if(lm ==NULL){
	E_INFO("In lm_read, a text format reader is called, but lm cannot be read, Diagnosis: LM is in wrong format or not enough memory.\n");
	return NULL;
      }
    }else if (!strcmp(fmt,"DMP")){
      lm=lm_read_dump (file,cmd_ln_int32("-lminmemory"));
      if(lm ==NULL){
	E_INFO("In lm_read, a DMP format reader is called, but lm cannot be read, Diagnosis: LM is corrupted or not enough memory.\n");
	return NULL;
      }
    }else{
      E_INFO("Unknown format (%s) is specified\n",fmt);
      return NULL;
    }


    lm->name = ckd_salloc(lmname);
    lm->inputenc = IND_BADENCODING;
    lm->outputenc = IND_BADENCODING;

    /* Initialize the fast trigram cache, with all entries invalid */
    lm->tgcache = (lm_tgcache_entry_t *) ckd_calloc(LM_TGCACHE_SIZE, sizeof(lm_tgcache_entry_t));
    for (i = 0; i < LM_TGCACHE_SIZE; i++)
	lm->tgcache[i].lwid[0] = BAD_S3LMWID;

    if(applyWeight){
      lm2logs3 (lm, uw);	/* Applying unigram weight; convert to logs3 values */
    
      /* Apply the new lw and wip values */
      lm->lw = 1.0;	/* The initial settings for lw and wip */
      lm->wip = 0;	/* logs3(1.0) */
      lm_set_param (lm, lw, wip);
    }


    assert(lm);
    /* Set the size of dictionary */
    lm->dict_size=ndict;
    /*    E_INFO("lm->dict %d\n",lm->dict_size);*/
    for (u = 0; u < lm->n_ug; u++)
	lm->ug[u].dictwid = BAD_S3WID;
    
    
    return lm;
}

/*
  This convert every string in the lm from lmp->inputenc to
  lm->outputenc.  This function assumes the caller has checked the
  encoding schemes appropriateness. 

  (Caution!) At 20051115, the method is specific and only support hex
  to value conversion.  The code also hasn't considered that output
  encoding requires a longer length of string than the input encoding.
 */
static void lm_convert_encoding(lm_t *lmp)
{
  int i;

  E_INFO("Encoding Conversion\n");
  for(i=0;i<lmp->n_ug;i++){
#if 0
    E_INFO("%s\n",lmp->wordstr[i]);
#endif

    if(ishex(lmp->wordstr[i])){
       hextocode(lmp->wordstr[i]);
    }

#if 0
    E_INFO("%s\n",lmp->wordstr[i]);
#endif
  }
}

int32 lm_write_advance(lm_t * lmp, const char* outputfn,const char* filename,char *fmt, char* inputenc, char* outputenc)
{
  /* This might be duplicated with the caller checking but was done for extra safety. */

  assert(encoding_resolve(inputenc,outputenc));

  lmp->inputenc=encoding_str2ind(inputenc);
  lmp->outputenc=encoding_str2ind(outputenc);

  if(lmp->inputenc!=lmp->outputenc){
    E_INFO("Did I come here?\n");
    lm_convert_encoding(lmp);
  }

  if (!strcmp(fmt,"TXT")){
    return lm_write_arpa_text(lmp,outputfn,inputenc,outputenc);
  }else if (!strcmp(fmt,"DMP")){
    /* set mtime to be zero because sphinx3 has no mechanism to check
       whether the file is generated earlier (at least for now.)*/
    return lm3g_dump(outputfn,lmp,filename,0);
  }else if (!strcmp(fmt,"FST")){
    E_WARN("Invoke un-tested ATT-FSM writer\n");
    return lm_write_att_fsm(lmp,outputfn);
  }else{
    E_INFO("Unknown format (%s) is specified\n",fmt);
    return LM_FAIL; 
  }
}

int32 lm_write(lm_t * lmp, const char* outputfn,const char* filename,char *fmt)
{
  return lm_write_advance(lmp,outputfn,filename,fmt,"iso8859-1","iso8859-1");
}


/*
 * Free stale bigram and trigram info, those not used since last reset.
 */
void lm_cache_reset (lm_t *lm)
{
    int32 i, n_bgfree, n_tgfree;
    tginfo_t *tginfo, *next_tginfo, *prev_tginfo;

    n_bgfree = n_tgfree = 0;

    /* ARCHAN: RAH only short-circult this function only */
    if (lm->isLM_IN_MEMORY)		/* RAH We are going to short circuit this if we are running with the lm in memory */
    return;

  
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


void lm_cache_stats_dump (lm_t *lm)
{
    E_INFO("%9d tg(), %9d tgcache, %8d bo; %5d fills, %8d in mem (%.1f%%)\n",
	   lm->n_tg_score, lm->n_tgcache_hit, lm->n_tg_bo, lm->n_tg_fill, lm->n_tg_inmem,
	   (lm->n_tg_inmem*100.0)/(lm->n_tg+1));
    E_INFO("%8d bg(), %8d bo; %5d fills, %8d in mem (%.1f%%)\n",
	   lm->n_bg_score, lm->n_bg_bo, lm->n_bg_fill, lm->n_bg_inmem,
	   (lm->n_bg_inmem*100.0)/(lm->n_bg+1));
    
    lm->n_tgcache_hit = 0;
    lm->n_tg_fill = 0;
    lm->n_tg_score = 0;
    lm->n_tg_bo = 0;
    lm->n_bg_fill = 0;
    lm->n_bg_score = 0;
    lm->n_bg_bo = 0;
}


int32 lm_ug_score (lm_t *lm, s3lmwid_t lwid, s3wid_t wid)
{
    if (NOT_S3LMWID(lwid) || (lwid >= lm->n_ug))
	E_FATAL("Bad argument (%d) to lm_ug_score\n", lwid);

    lm->access_type = 1;
    
    if(lm->inclass_ugscore)
      return (lm->ug[lwid].prob.l +lm->inclass_ugscore[wid]);
    else
      return (lm->ug[lwid].prob.l );
}

int32 lm_ug_exists(lm_t* lm, s3lmwid_t lwid)
{
  if(NOT_S3LMWID(lwid) || (lwid >=lm->n_ug))
    return 0;
  else
    return 1;
}


int32 lm_uglist (lm_t *lm, ug_t **ugptr)
{
    *ugptr = lm->ug;
    return (lm->n_ug);
}


/* This create a mapping from either the unigram or words in a class*/
int32 lm_ug_wordprob (lm_t *lm, dict_t *dict,int32 th, wordprob_t *wp)
{
    int32 i, j, n, p;
    s3wid_t w,dictid;
    lmclass_t lmclass;
    lmclass_word_t lm_cw;
    n = lm->n_ug;
    
    for (i = 0, j = 0; i < n; i++) {
	w = lm->ug[i].dictwid;
	if (IS_S3WID(w)) { /*Is w>0? Then it can be either wid or class id*/
	  if (w <  LM_CLASSID_BASE){ /*It is just a word*/
	    if ((p = lm->ug[i].prob.l) >= th) {
		wp[j].wid = w;
		wp[j].prob = p;
		j++;
	    }
	  }else{ /* It is a class */
	    lmclass=LM_CLASSID_TO_CLASS(lm,w); /* Get the class*/
	    lm_cw=lmclass_firstword(lmclass);
	    while(lmclass_isword(lm_cw)){
	      dictid =lmclass_getwid(lm_cw); 

	      /*E_INFO("Lookup dict_id using dict_basewid %d\n",dictid);*/
	      if(IS_S3WID(dictid)){
		if(dictid !=dict_basewid(dict,dictid)){
		  dictid=dict_basewid(dict,dictid);
		}
		if((p=lm->ug[i].prob.l+lm->inclass_ugscore[dictid])>=th){
		  wp[j].wid=dictid;
		  wp[j].prob=lm->ug[i].prob.l;
		  j++;
		}
	      }else{
		  E_INFO("Word %s cannot be found \n", lmclass_getword(lm_cw));
	      }

	      lm_cw= lmclass_nextword (lmclass,lm_cw);
	      
	    }
	  }
	}
    }
    
    return j;
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


    
  if (lm->isLM_IN_MEMORY)		/* RAH, if LM_IN_MEMORY, then we don't need to go get it. */
    bg = lm->membg[lw1].bg = &lm->bg[b];
  else {
    bg = lm->membg[lw1].bg = (bg_t *) ckd_calloc (n+1, sizeof(bg_t));
    
    if (fseek (lm->fp, lm->bgoff + b*sizeof(bg_t), SEEK_SET) < 0)
	E_FATAL_SYSTEM ("fseek failed\n");
    
    /* Need to read n+1 because obtaining tg count for one bg also depends on next bg */
    if (fread (bg, sizeof(bg_t), n+1, lm->fp) != (size_t)(n+1))
	E_FATAL("fread failed\n");
    if (lm->byteswap) {
	for (i = 0; i <= n; i++) {
	    SWAP_INT16(&(bg[i].wid));
	    SWAP_INT16(&(bg[i].probid));
	    SWAP_INT16(&(bg[i].bowtid));
	    SWAP_INT16(&(bg[i].firsttg));
	}
    }
  }
    lm->n_bg_fill++;
    lm->n_bg_inmem += n;
}


#define BINARY_SEARCH_THRESH	16

/* Locate a specific bigram within a bigram list */
int32 find_bg (bg_t *bg, int32 n, s3lmwid_t w)
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


int32 lm_bglist (lm_t *lm, s3lmwid_t w1, bg_t **bgptr, int32 *bowt)
{
    int32 n;

    if (NOT_S3LMWID(w1) || (w1 >= lm->n_ug))
	E_FATAL("Bad w1 argument (%d) to lm_bglist\n", w1);

    n = (lm->n_bg > 0) ? lm->ug[w1+1].firstbg - lm->ug[w1].firstbg : 0;
    
    if (n > 0) {
	if (! lm->membg[w1].bg)
	    load_bg (lm, w1);
	lm->membg[w1].used = 1;

	*bgptr = lm->membg[w1].bg;
	*bowt = lm->ug[w1].bowt.l;
    } else {
	*bgptr = NULL;
	*bowt = 0;
    }
    
    return (n);
}


#if 0 /*Obsolete, not used */
int32 lm_bg_wordprob (lm_t *lm, s3lmwid_t lwid, int32 th, wordprob_t *wp, int32 *bowt)
{
    bg_t *bgptr;
    int32 i, j, n, ugprob, bgprob;
    s3wid_t w;
    
    n = lm_bglist (lm, lwid, &bgptr, bowt);
    ugprob = lm_ug_score (lm, lwid);
    /* Convert bglist to wordprob */
    for (i = 0, j = 0; i < n; i++, bgptr++) {
	w = lm->ug[bgptr->wid].dictwid;
	if (IS_S3WID (w)) {
	    bgprob = LM_BGPROB(lm, bgptr);
	    
	    if (ugprob + bgprob >= th) {	/* ABSOLUTE prob (count) >= min thresh */
		wp[j].wid = w;
		wp[j].prob = bgprob;
		j++;
	    }
	}
    }
    
    return j;
}
#endif

/*
 *  This function look-ups the bigram score of p(lw2|lw1)
 *  The information for lw2 and w2 are repeated because the legacy 
 *  implementation(since s3.2) of vithist used only LM wid rather 
 *  than dictionary wid.  
 */

int32 lm_bg_score (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2, s3wid_t w2)
{
    int32 i, n, score;
    bg_t *bg=0;

    if ((lm->n_bg == 0) || (NOT_S3LMWID(lw1)))
	return (lm_ug_score (lm, lw2, w2));

    lm->n_bg_score++;

    if (NOT_S3LMWID(lw2) || (lw2 >= lm->n_ug))
	E_FATAL("Bad lw2 argument (%d) to lm_bg_score\n", lw2);
    
    n = lm->ug[lw1+1].firstbg - lm->ug[lw1].firstbg;
    
    if (n > 0) {
	if (! lm->membg[lw1].bg)
	    load_bg (lm, lw1);
	lm->membg[lw1].used = 1;
	bg = lm->membg[lw1].bg;

	i = find_bg (bg, n, lw2);
    } else
	i = -1;
    
    if (i >= 0) {
	score = lm->bgprob[bg[i].probid].l;
	if(lm->inclass_ugscore){ /*Only add within class prob if class information exists.
				  Is actually ok to just add the score because if the word
				  is not within-class. The returning scores will be 0. I just
				  love to safe-guard it :-). 
				 */
	  score += lm->inclass_ugscore[w2];
	}

	lm->access_type = 2;
    } else {
	lm->n_bg_bo++;
	lm->access_type = 1;
	score = lm->ug[lw1].bowt.l + lm->ug[lw2].prob.l;
    }

#if 0
    printf ("      %5d %5d -> %8d\n", lw1, lw2, score);
#endif

    return (score);
}

int32 lm_bg_exists (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2)
{
    int32 i, n, score;
    bg_t *bg=0;

    if ((lm->n_bg == 0) || (NOT_S3LMWID(lw1)))
      return 0 ;

    if (NOT_S3LMWID(lw2) || (lw2 >= lm->n_ug))
      return 0;
    
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
      return 1;
    else 
      return 0;


    return (score);
}


static void load_tg (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2)
{
    int32 i, n, b;
    int32 t = -1; /* Let's make sure that if t isn't initialized after the
		   * "if" statement below, it makes things go bad */
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

    /* "t" has not been assigned any meanigful value, so if you use it
     * beyond this point, make sure it's been properly assigned.
     */   
//	assert (t != -1);

    /* At this point, n = #trigrams for lw1,lw2.  Read them in */

    if (lm->isLM_IN_MEMORY) {
		/* RAH, already have this in memory */
      if (n > 0){
	assert(t != -1);
	tg = tginfo->tg = &lm->tg[t];
      }
    } else {
    if (n > 0) {
	tg = tginfo->tg = (tg_t *) ckd_calloc (n, sizeof(tg_t));
	if (fseek (lm->fp, lm->tgoff + t*sizeof(tg_t), SEEK_SET) < 0)
	    E_FATAL_SYSTEM("fseek failed\n");

	if (fread (tg, sizeof(tg_t), n, lm->fp) != (size_t)n)
	    E_FATAL("fread(tg, %d at %d) failed\n", n, lm->tgoff);
	if (lm->byteswap) {
	    for (i = 0; i < n; i++) {
		SWAP_INT16(&(tg[i].wid));
		SWAP_INT16(&(tg[i].probid));
	    }
	}
    }
    }
    lm->n_tg_fill++;
    lm->n_tg_inmem += n;
}


/* Similar to find_bg */
int32 find_tg (tg_t *tg, int32 n, s3lmwid_t w)
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


int32 lm_tglist (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2, tg_t **tgptr, int32 *bowt)
{
    tginfo_t *tginfo, *prev_tginfo;

    if (lm->n_tg <= 0) {
	*tgptr = NULL;
	*bowt = 0;
	return 0;
    }
    
    if (NOT_S3LMWID(lw1) || (lw1 >= lm->n_ug))
	E_FATAL("Bad lw1 argument (%d) to lm_tglist\n", lw1);
    if (NOT_S3LMWID(lw2) || (lw2 >= lm->n_ug))
	E_FATAL("Bad lw2 argument (%d) to lm_tglist\n", lw2);

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

/*
 *  This function look-ups the trigram score of p(lw3|lw2,lw1)
 *  and compute the in-class ug probability of w3.
 *  The information for lw3 and w3 are repeated because the legacy 
 *  implementation(since s3.2) of vithist used only LM wid rather 
 *  than dictionary wid.  
 *  
 */

int32 lm_tg_score (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2, s3lmwid_t lw3, s3wid_t w3)
{
    int32 i, h, n, score;
    tg_t *tg;
    tginfo_t *tginfo, *prev_tginfo;
    


    if ((lm->n_tg == 0) || (NOT_S3LMWID(lw1)))
	return (lm_bg_score (lm, lw2, lw3, w3));

    lm->n_tg_score++;

    if (NOT_S3LMWID(lw1) || (lw1 >= lm->n_ug))
	E_FATAL("Bad lw1 argument (%d) to lm_tg_score\n", lw1);
    if (NOT_S3LMWID(lw2) || (lw2 >= lm->n_ug))
	E_FATAL("Bad lw2 argument (%d) to lm_tg_score\n", lw2);
    if (NOT_S3LMWID(lw3) || (lw3 >= lm->n_ug))
	E_FATAL("Bad lw3 argument (%d) to lm_tg_score\n", lw3);

    /* Lookup tgcache first; compute hash(lw1, lw2, lw3) */
    h = ((lw1 & 0x000003ff) << 21) + ((lw2 & 0x000003ff) << 11) + (lw3 & 0x000007ff);
    h %= LM_TGCACHE_SIZE;

    if ((lm->tgcache[h].lwid[0] == lw1) &&
	(lm->tgcache[h].lwid[1] == lw2) &&
	(lm->tgcache[h].lwid[2] == lw3)) {

	lm->n_tgcache_hit++;
	return lm->tgcache[h].lscr;
    }
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

    assert(tginfo);
    if ((i = find_tg (tg, n, lw3)) >= 0) {
	score = lm->tgprob[tg[i].probid].l ;
	if(lm->inclass_ugscore){ /*Only add within class prob if class information exists.
				  Is actually ok to just add the score because if the word
				  is not within-class. The returning scores will be 0. 
				 */
	  score += lm->inclass_ugscore[w3];
	}
	lm->access_type = 3;
    } else {
	lm->n_tg_bo++;
	score = tginfo->bowt + lm_bg_score(lm, lw2, lw3, w3);
    }
#if 0
    printf ("%5d %5d %5d -> %8d\n", lw1, lw2, lw3, score);
#endif
    
    lm->tgcache[h].lwid[0] = lw1;
    lm->tgcache[h].lwid[1] = lw2;
    lm->tgcache[h].lwid[2] = lw3;
    lm->tgcache[h].lscr = score;
    
    return (score);
}

int32 lm_tg_exists (lm_t *lm, s3lmwid_t lw1, s3lmwid_t lw2, s3lmwid_t lw3)
{
    int32 i,n;
    tg_t *tg;
    tginfo_t *tginfo, *prev_tginfo;

    if ((lm->n_tg == 0) || (NOT_S3LMWID(lw1)))
      return 0;

    if (NOT_S3LMWID(lw1) || (lw1 >= lm->n_ug))
      return 0;
    if (NOT_S3LMWID(lw2) || (lw2 >= lm->n_ug))
      return 0;
    if (NOT_S3LMWID(lw3) || (lw3 >= lm->n_ug))
      return 0;

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

    assert(tginfo);
    if ((i = find_tg (tg, n, lw3)) >= 0) 
      return 1;
    else 
      return 0;
}


s3lmwid_t lm_wid (lm_t *lm, char *word)
{
    int32 i;
    
    for (i = 0; i < lm->n_ug; i++)
	if (strcmp (lm->wordstr[i], word) == 0)
	    return ((s3lmwid_t) i);
    
    return BAD_S3LMWID;
}

void lm_free (lm_t *lm)
{
  int i;
  tginfo_t *tginfo;

  if(lm->fp)
    fclose(lm->fp);

  ckd_free ((void *) lm->ug);  

  for (i=0;i<lm->n_ug;i++) 
    ckd_free ((void *) lm->wordstr[i]);	/*  */
  ckd_free ((void *) lm->wordstr);

  if(lm->n_bg >0){
    if (lm->bg){		/* Memory-based; free all bg */
      ckd_free (lm->bg);
      if(lm->membg){
	ckd_free(lm->membg);
      }
    }
    else {		/* Disk-based; free in-memory bg */
      for (i = 0; i < lm->n_ug; i++)
	if (lm->membg[i].bg)
	  ckd_free (lm->membg[i].bg);
      ckd_free (lm->membg);
    }
    
    ckd_free (lm->bgprob);
  }

  if(lm->n_tg>0){
    if(lm->tg)
      ckd_free((void*)lm->tg);

    for(i=0;i<lm->n_ug;i++){
      if(lm->tginfo[i]!=NULL){
	/* Free the whole linked list of tginfo. */
	while (lm->tginfo[i]){
	  tginfo=lm->tginfo[i];
	  lm->tginfo[i]=tginfo->next;
	  ckd_free((void*) tginfo);
	}
      }
    }
    ckd_free ((void *) lm->tginfo);

    ckd_free ((void *) lm->tgcache);
    ckd_free ((void *) lm->tg_segbase);
    ckd_free ((void *) lm->tgprob);
    ckd_free ((void *) lm->tgbowt);

    if(lm->dict2lmwid){
      ckd_free(lm->dict2lmwid);
    }
    
    if(lm->HT){
      hash_free(lm->HT);
    }
  }


  ckd_free ((void *) lm);

  
}


int32 lm_rawscore (lm_t *lm, int32 score)
{

    score -= lm->wip;
    score /= (int32)lm->lw;
    
    return score;
}



#if (_LM_TEST_)
static int32 sentence_lmscore (lm_t *lm, char *line)
{
    char *word[1024];
    s3lmwid_t w[1024];
    int32 nwd, score, tgscr;
    int32 i, j;
    
    if ((nwd = str2words (line, word, 1020)) < 0)
	E_FATAL("Increase word[] and w[] arrays size\n");
    
    w[0] = BAD_S3LMWID;
    w[1] = lm_wid (lm, S3_START_WORD);
    if (NOT_S3LMWID(w[1]))
	E_FATAL("Unknown word: %s\n", S3_START_WORD);
    
    for (i = 0; i < nwd; i++) {
	w[i+2] = lm_wid (lm, word[i]);
	if (NOT_S3LMWID(w[i+2])) {
	    E_ERROR("Unknown word: %s\n", word[i]);
	    return 0;
	}
    }

    w[i+2] = lm_wid (lm, S3_FINISH_WORD);
    if (NOT_S3LMWID(w[i+2]))
	E_FATAL("Unknown word: %s\n", S3_FINISH_WORD);
    
    score = 0;
    for (i = 0, j = 2; i <= nwd; i++, j++) {
	tgscr = lm_tg_score (lm, w[j-2], w[j-1], w[j]);
	score += tgscr;
	printf ("\t%10d %s\n", tgscr, lm->wordstr[w[j]]);
    }
    
    return (score);
}


main (int32 argc, char *argv[])
{
    char line[4096];
    int32 score, k;
    lm_t *lm;
    
    if (argc < 2)
	E_FATAL("Usage: %s <LMdumpfile>\n", argv[0]);

    logs3_init (1.0001, 1, 1);
    lm = lm_read (argv[1], 9.5, 0.2);

    if (1) {			/* Short cut this so we can test for memory leaks */
      for (;;) {
	printf ("> ");
	if (fgets (line, sizeof(line), stdin) == NULL)
	    break;
	
	score = sentence_lmscore (lm, line);

	k = strlen(line);
	if (line[k-1] == '\n')
	    line[k-1] = '\0';
	printf ("LMScr(%s) = %d\n", line, score);
      }
    } /*  */
    lm_free(lm);
    exit (0);
}
#endif


