/* ====================================================================
 * Copyrightgot (c) 1999-2001 Carnegie Mellon University.  All rights
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
/************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 ************************************************
 * 
 * HISTORY
 * 
 * 30-Dec-2000	Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Moved kb_*() routines into kb.c to make them independent of
 *		main() during compilation
 *
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to allow runtime choice between 3-state and 5-state HMM
 * 		topologies (instead of compile-time selection).
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxwpf.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "kb.h"
#include "logs3.h"		/* RAH, added to resolve log3_free */

/*ARCHAN, to allow backward compatibility -lm, -lmctlfn coexists. This makes the current implmentation more complicated than necessary. */
void kb_init (kb_t *kb)
{
    kbcore_t *kbcore;
    mdef_t *mdef;
    dict_t *dict;
    dict2pid_t *d2p;
    lm_t *lm;
    lmset_t *lmset;
    s3cipid_t sil, ci;
    s3wid_t w;
    int32 i, n, n_lc;
    wordprob_t *wp;
    s3cipid_t *lc;
    bitvec_t lc_active;
    char *str;
    int32 cisencnt;
    int32 j;
    
    /* Initialize the kb structure to zero, just in case */
    memset(kb, 0, sizeof(*kb));
    kb->kbcore = NULL;

    kb->kbcore = kbcore_init (cmd_ln_float32 ("-logbase"),
			      cmd_ln_str("-feat"),
			      cmd_ln_str("-cmn"),
			      cmd_ln_str("-varnorm"),
			      cmd_ln_str("-agc"),
			      cmd_ln_str("-mdef"),
			      cmd_ln_str("-dict"),
			      cmd_ln_str("-fdict"),
			      "",	/* Hack!! Hardwired constant 
						for -compsep argument */
			      cmd_ln_str("-lm"),
			      cmd_ln_str("-lmctlfn"),
			      cmd_ln_str("-lmdumpdir"),
			      cmd_ln_str("-fillpen"),
			      cmd_ln_str("-senmgau"),
			      cmd_ln_float32("-silprob"),
			      cmd_ln_float32("-fillprob"),
			      cmd_ln_float32("-lw"),
			      cmd_ln_float32("-wip"),
			      cmd_ln_float32("-uw"),
			      cmd_ln_str("-mean"),
			      cmd_ln_str("-var"),
			      cmd_ln_float32("-varfloor"),
			      cmd_ln_str("-mixw"),
			      cmd_ln_float32("-mixwfloor"),
			      cmd_ln_str("-subvq"),
			      cmd_ln_str("-gs"),
			      cmd_ln_str("-tmat"),
			      cmd_ln_float32("-tmatfloor"));
    if(kb->kbcore==NULL){
      E_FATAL("Initialization of kb failed\n");
    }

    kbcore = kb->kbcore;
    
    mdef = kbcore_mdef(kbcore);
    dict = kbcore_dict(kbcore);
    lm = kbcore_lm(kbcore);
    lmset=kbcore_lmset(kbcore);
    d2p = kbcore_dict2pid(kbcore);
    
    if (NOT_S3WID(dict_startwid(dict)) || NOT_S3WID(dict_finishwid(dict)))
	E_FATAL("%s or %s not in dictionary\n", S3_START_WORD, S3_FINISH_WORD);

    if(lmset){
      for(i=0;i<kbcore_nlm(kbcore);i++){
	if (NOT_S3LMWID(lm_startwid(lmset[i].lm)) || NOT_S3LMWID(lm_finishwid(lmset[i].lm)))
	E_FATAL("%s or %s not in LM %s\n", S3_START_WORD, S3_FINISH_WORD,lmset[i].name);
      }
    }else if(lm){
      if (NOT_S3LMWID(lm_startwid(lm)) || NOT_S3LMWID(lm_finishwid(lm)))
	E_FATAL("%s or %s not in LM\n", S3_START_WORD, S3_FINISH_WORD);
    }

    
    /* Check that HMM topology restrictions are not violated */
    if (tmat_chk_1skip (kbcore->tmat) < 0)
	E_FATAL("Tmat contains arcs skipping more than 1 state\n");
    
    /*
     * Unlink <s> and </s> between dictionary and LM, to prevent their 
     * recognition.  They are merely dummy words (anchors) at the beginning 
     * and end of each utterance.
     */
    if(lmset){
      for(i=0;i<kbcore_nlm(kbcore);i++){
	lm_lmwid2dictwid(lmset[i].lm, lm_startwid(lmset[i].lm)) = BAD_S3WID;
	lm_lmwid2dictwid(lmset[i].lm, lm_finishwid(lmset[i].lm)) = BAD_S3WID;

	for (w = dict_startwid(dict); IS_S3WID(w); w = dict_nextalt(dict, w))
	  lmset[i].lm->dict2lmwid[w] = BAD_S3LMWID;
	for (w = dict_finishwid(dict); IS_S3WID(w); w = dict_nextalt(dict, w))
	  lmset[i].lm->dict2lmwid[w] = BAD_S3LMWID;

      }
    }else if(lm){ /* No LM is set at this point*/
      lm_lmwid2dictwid(lm, lm_startwid(lm)) = BAD_S3WID;
      lm_lmwid2dictwid(lm, lm_finishwid(lm)) = BAD_S3WID;
      for (w = dict_startwid(dict); IS_S3WID(w); w = dict_nextalt(dict, w))
	kbcore->dict2lmwid[w] = BAD_S3LMWID;
      for (w = dict_finishwid(dict); IS_S3WID(w); w = dict_nextalt(dict, w))
	kbcore->dict2lmwid[w] = BAD_S3LMWID;

    }
    sil = mdef_silphone (kbcore_mdef (kbcore));
    if (NOT_S3CIPID(sil))
	E_FATAL("Silence phone '%s' not in mdef\n", S3_SILENCE_CIPHONE);
    
    
    kb->sen_active = (int32 *) ckd_calloc (mdef_n_sen(mdef), sizeof(int32));
    kb->rec_sen_active = (int32 *) ckd_calloc (mdef_n_sen(mdef), sizeof(int32));
    kb->ssid_active = (int32 *) ckd_calloc (mdef_n_sseq(mdef), sizeof(int32));
    kb->comssid_active = (int32 *) ckd_calloc (dict2pid_n_comsseq(d2p), sizeof(int32));
    
    /* Build set of all possible left contexts */
    lc = (s3cipid_t *) ckd_calloc (mdef_n_ciphone(mdef) + 1, sizeof(s3cipid_t));
    lc_active = bitvec_alloc (mdef_n_ciphone (mdef));
    for (w = 0; w < dict_size (dict); w++) {
	ci = dict_pron (dict, w, dict_pronlen(dict, w) - 1);
	if (! mdef_is_fillerphone (mdef, (int)ci))
	    bitvec_set (lc_active, ci);
    }
    ci = mdef_silphone(mdef);
    bitvec_set (lc_active, ci);
    for (ci = 0, n_lc = 0; ci < mdef_n_ciphone(mdef); ci++) {
	if (bitvec_is_set (lc_active, ci))
	    lc[n_lc++] = ci;
    }
    lc[n_lc] = BAD_S3CIPID;

    E_INFO("Building lextrees\n");
    /* Get the number of lexical tree*/
    kb->n_lextree = cmd_ln_int32 ("-Nlextree");
    if (kb->n_lextree < 1) {
	E_ERROR("No. of ugtrees specified: %d; will instantiate 1 ugtree\n", 
								kb->n_lextree);
	kb->n_lextree = 1;
    }

    /* ARCHAN: This code was rearranged in s3.4 implementation of dynamic LM */
    /* Build active word list */
    wp = (wordprob_t *) ckd_calloc (dict_size(dict), sizeof(wordprob_t));


    if(lmset){
      kb->ugtreeMulti = (lextree_t **) ckd_calloc (kbcore_nlm(kbcore)*kb->n_lextree, sizeof(lextree_t *));
      /* Just allocate pointers*/
      kb->ugtree = (lextree_t **) ckd_calloc (kb->n_lextree, sizeof(lextree_t *));

      for(i=0;i<kbcore_nlm(kbcore);i++){
	E_INFO("Creating Unigram Table for lm %d name %s\n",i,lmset[i].name);
	n=0;
	for(j=0;j<dict_size(dict);j++){ /*try to be very careful again */
	  wp[j].wid=-1;
	  wp[j].prob=-1;
	}
	n = lm_ug_wordprob (lmset[i].lm, dict,MAX_NEG_INT32, wp);
	E_INFO("Size of word table after unigram + words in class: %d.\n",n);
	if (n < 1)
	  E_FATAL("%d active words in %s\n", n,lmset[i].name);
	n = wid_wordprob2alt(dict,wp,n);
	E_INFO("Size of word table after adding alternative prons: %d.\n",n);
	if (cmd_ln_int32("-treeugprob") == 0) {
	  for (i = 0; i < n; i++)
	    wp[i].prob = -1;    	/* Flatten all initial probabilities */
	}

	for (j = 0; j < kb->n_lextree; j++) {
	  kb->ugtreeMulti[i*kb->n_lextree+j] = lextree_build (kbcore, wp, n, lc);
	  lextree_type (kb->ugtreeMulti[i*kb->n_lextree+j]) = 0;
	  E_INFO("Lextrees (%d) for lm %d, its name is %s, it has %d nodes(ug)\n",
		 j, i, lmset[i].name,lextree_n_node(kb->ugtreeMulti[i*kb->n_lextree+j]));
	}
      }

    }else if (lm){
      E_INFO("Creating Unigram Table\n");
      n=0;
      n = lm_ug_wordprob (lm, dict,MAX_NEG_INT32, wp);
      E_INFO("Size of word table after unigram + words in class: %d\n",n);
      if (n < 1)
	E_FATAL("%d active words\n", n);
      n = wid_wordprob2alt (dict, wp, n);	   /* Add alternative pronunciations */
      
      /* Retain or remove unigram probs from lextree, depending on option */
      if (cmd_ln_int32("-treeugprob") == 0) {
	for (i = 0; i < n; i++)
	  wp[i].prob = -1;    	/* Flatten all initial probabilities */
      }
      
      /* Create the desired no. of unigram lextrees */
      kb->ugtree = (lextree_t **) ckd_calloc (kb->n_lextree, sizeof(lextree_t *));
      for (i = 0; i < kb->n_lextree; i++) {
	kb->ugtree[i] = lextree_build (kbcore, wp, n, lc);
	lextree_type (kb->ugtree[i]) = 0;
      }
      E_INFO("Lextrees(%d), %d nodes(ug)\n",
	     kb->n_lextree, lextree_n_node(kb->ugtree[0]));
    }



    /* Create filler lextrees */
    /* ARCHAN : only one filler tree is supposed to be build even for dynamic LMs */
    n = 0;
    for (i = dict_filler_start(dict); i <= dict_filler_end(dict); i++) {
	if (dict_filler_word(dict, i)) {
	    wp[n].wid = i;
	    wp[n].prob = fillpen (kbcore->fillpen, i);
	    n++;
	}
    }


    kb->fillertree = (lextree_t **)ckd_calloc(kb->n_lextree,sizeof(lextree_t*));
    for (i = 0; i < kb->n_lextree; i++) {
	kb->fillertree[i] = lextree_build (kbcore, wp, n, NULL);
	lextree_type (kb->fillertree[i]) = -1;
    }
    ckd_free ((void *) wp);
    ckd_free ((void *) lc);
    bitvec_free (lc_active);


    E_INFO("Lextrees(%d), %d nodes(filler)\n",
	     kb->n_lextree, 
	     lextree_n_node(kb->fillertree[0]));
    

    if (cmd_ln_int32("-lextreedump")) {
      if(lmset){
	E_FATAL("Currently, doesn't support -lextreedump for multiple-LMs\n");
      }
      for (i = 0; i < kb->n_lextree; i++) {
	fprintf (stderr, "UGTREE %d\n", i);
	lextree_dump (kb->ugtree[i], dict, stderr);
      }
      for (i = 0; i < kb->n_lextree; i++) {
	fprintf (stderr, "FILLERTREE %d\n", i);
	lextree_dump (kb->fillertree[i], dict, stderr);
      }
      fflush (stderr);
    }
    
    kb->ascr = ascr_init (mgau_n_mgau(kbcore_mgau(kbcore)), 
				kbcore->dict2pid->n_comstate);
    kb->beam = beam_init (cmd_ln_float64("-subvqbeam"),
			  cmd_ln_float64("-beam"),
			  cmd_ln_float64("-pbeam"),
			  cmd_ln_float64("-wbeam"));
    E_INFO("Beam= %d, PBeam= %d, WBeam= %d, SVQBeam= %d\n",
	   kb->beam->hmm, kb->beam->ptrans, kb->beam->word, kb->beam->subvq);
    
    /*Sections of optimization related parameters*/
    kb->ds_ratio=cmd_ln_int32("-ds");
    E_INFO("Down Sampling Ratio = %d\n",kb->ds_ratio);
    
    kb->rec_bstcid=-1;
    kb->skip_count=0;
    
    kb->cond_ds=cmd_ln_int32("-cond_ds");
    E_INFO("Conditional Down Sampling Parameter = %d\n",kb->cond_ds);
    
    if(kb->cond_ds>0&&kb->kbcore->gs==NULL) E_FATAL("Conditional Down Sampling require the use of Gaussian Selection map\n");

    kb->gs4gs=cmd_ln_int32("-gs4gs");
    E_INFO("GS map would be used for Gaussian Selection? = %d\n",kb->gs4gs);

    kb->svq4svq=cmd_ln_int32("-svq4svq");
    E_INFO("SVQ would be used as Gaussian Score? = %d\n",kb->svq4svq);

    kb->ci_pbeam=-1*logs3(cmd_ln_float64("-ci_pbeam"));
    E_INFO("CI phone beam to prune the number of parent CI phones in CI-base GMM Selection = %d\n",kb->ci_pbeam);
    if(kb->ci_pbeam>10000000){
      E_INFO("Virtually no CI phone beam is applied now. (ci_pbeam>1000000)\n");
    }
    
    kb->wend_beam=-1*logs3(cmd_ln_float64("-wend_beam"));
    E_INFO("Word-end pruning beam: %d\n",kb->wend_beam);

    kb->pl_window=cmd_ln_int32("-pl_window");
    E_INFO("Phoneme look-ahead window size = %d\n",kb->pl_window);

	kb->pl_window_start=0;

    kb->pl_beam=logs3(cmd_ln_float64("-pl_beam"));
    E_INFO("Phoneme look-ahead beam = %d\n",kb->pl_beam);

    for(cisencnt=0;cisencnt==mdef->cd2cisen[cisencnt];cisencnt++) 
      ;

    kb->cache_ci_senscr=(int32**)ckd_calloc_2d(kb->pl_window,cisencnt,sizeof(int32));
    kb->cache_best_list=(int32*)ckd_calloc(kb->pl_window,sizeof(int32));
    kb->phn_heur_list=(int32*)ckd_calloc(mdef_n_ciphone (mdef),sizeof(int32));
  

    if ((kb->feat = feat_array_alloc(kbcore_fcb(kbcore),S3_MAX_FRAMES)) == NULL)
	E_FATAL("feat_array_alloc() failed\n");
    
    kb->vithist = vithist_init(kbcore, kb->beam->word, cmd_ln_int32("-bghist"));
    
    ptmr_init (&(kb->tm_sen));
    ptmr_init (&(kb->tm_srch));
    ptmr_init (&(kb->tm_ovrhd));
    kb->tot_fr = 0;
    kb->tot_sen_eval = 0.0;
    kb->tot_gau_eval = 0.0;
    kb->tot_hmm_eval = 0.0;
    kb->tot_wd_exit = 0.0;
    
    kb->hmm_hist_binsize = cmd_ln_int32("-hmmhistbinsize");

    if(lmset)
      n = ((kb->ugtreeMulti[0]->n_node) + (kb->fillertree[0]->n_node)) * kb->n_lextree;
    else
      n = ((kb->ugtree[0]->n_node) + (kb->fillertree[0]->n_node)) * kb->n_lextree;

    n /= kb->hmm_hist_binsize;
    kb->hmm_hist_bins = n+1;
    kb->hmm_hist = (int32 *) ckd_calloc (n+1, sizeof(int32));	/* Really no need for +1 */
    
    /* Open hypseg file if specified */
    str = cmd_ln_str("-hypseg");
    kb->matchsegfp = NULL;
    if (str) {
#ifdef WIN32
	if ((kb->matchsegfp = fopen(str, "wt")) == NULL)
#else
	if ((kb->matchsegfp = fopen(str, "w")) == NULL)
#endif
	    E_ERROR("fopen(%s,w) failed; use FWDXCT: from std logfile\n", str);
    }

    str = cmd_ln_str("-hyp");
    kb->matchfp = NULL;
    if (str) {
#ifdef WIN32
	if ((kb->matchfp = fopen(str, "wt")) == NULL)
#else
	if ((kb->matchfp = fopen(str, "w")) == NULL)
#endif
	    E_ERROR("fopen(%s,w) failed; use FWDXCT: from std logfile\n", str);
    }
    
    /* Setting for MLLR matrix */
    kb->prevmllrfn=(char*)ckd_calloc(1024,sizeof(char));
    kb->prevmllrfn[0]='\0';
    
}

void kb_setlm(char* lmname,kb_t* kb)
{
  lmset_t* lms;
  kbcore_t* kbc=NULL;
  int i = 0;
  int j;
  int n;
  /*  s3wid_t dictid;*/

  kbc=kb->kbcore;
  lms=kbc->lmset;
  kbc->lm=NULL;
  for(j=0;j<kb->n_lextree;j++){
    kb->ugtree[j]=NULL;
  }

  if(lms!=NULL || cmd_ln_str("-lmctlfn")){
    for(i=0;i<kbc->n_lm;i++){
      if(!strcmp(lmname,lms[i].name)){
	/* Point the current lm to a particular lm */
	kbc->lm=lms[i].lm;

	for(j=0;j<kb->n_lextree;j++){
	  kb->ugtree[j]=kb->ugtreeMulti[i*kb->n_lextree+j];
	}

	break;
      }
    }
    if(kbc->lm==NULL){
      E_ERROR("LM name %s cannot be found in the preloaded lm ctl file (spefied by -lmctlfile)! Fall back to use base LM model\n",lmname);
      kbc->lm=lms[0].lm;
      /* Also, don't forget to reset 'i', since if the code reaches
       * this point, i has a value of 'n_lm + 1'
       */
      i = 0;
      for(j=0;j<kb->n_lextree;j++){
	kb->ugtree[j]=kb->ugtreeMulti[j];
      }
    }
  }
  /*  Just to make sure we're not trying to point beyond the limit of
   *  array lms.
   */
  assert (i < kbc->n_lm);


  if((kb->vithist->lms2vh_root=
     (vh_lms2vh_t**)ckd_realloc(kb->vithist->lms2vh_root,
				lm_n_ug(kbc->lm)*sizeof(vh_lms2vh_t *)
				))==NULL) 
    {
      E_FATAL("failed to allocate memory for vithist\n");
    }


  n = ((kb->ugtree[0]->n_node) + (kb->fillertree[0]->n_node)) * kb->n_lextree;
  n /= kb->hmm_hist_binsize;
  kb->hmm_hist_bins = n+1;
  kb->hmm_hist = (int32 *) ckd_realloc (kb->hmm_hist,(n+1)*sizeof(int32));	/* Really no need for +1 */

  E_INFO("Current LM name %s\n",lms[i].name);
  E_INFO("LM ug size %d\n",kbc->lm->n_ug);
  E_INFO("LM bg size %d\n",kbc->lm->n_bg);
  E_INFO("LM tg size %d\n",kbc->lm->n_tg);
  E_INFO("HMM history bin size %d\n", n+1);

  for(j=0;j<kb->n_lextree;j++){
    E_INFO("Lextrees(%d), %d nodes(ug)\n",
	   kb->n_lextree, lextree_n_node(kb->ugtree[j]));
  }

  /*  for (n = 0; n < dict_size(kbcore_dict(kbc)); n++){
    E_INFO("Index %d, map %d word %s\n",n,kbc->lm->dict2lmwid[n],dict_wordstr(kbcore_dict(kbc),n));
    }*/
}

void kb_setmllr(char* mllrname,kb_t* kb)
{
  int32 veclen;
  E_INFO("Using MLLR matrix %s\n", mllrname);
  
  if(strcmp(kb->prevmllrfn,mllrname)!=0){ /* If there is a change of mllr file name */
    /* Reread the gaussian mean from the file again */
    E_INFO("Reloading mean\n");
    mgau_mean_reload(kbcore_mgau(kb->kbcore),cmd_ln_str("-mean"));

    /* Read in the mllr matrix */

#if MLLR_DEBUG
    /*This generates huge amount of information */
    /*    mgau_dump(kbcore_mgau(kb->kbcore),1);*/
#endif

    mllr_read_regmat(mllrname,
		     &(kb->regA),
		     &(kb->regB),
		     mgau_veclen(kbcore_mgau(kb->kbcore)));




    /* Transform all the mean vectors */

    mllr_norm_mgau(kbcore_mgau(kb->kbcore),kb->regA,kb->regB,kbcore_mdef(kb->kbcore));

#if MLLR_DEBUG
    /*#if 1*/
    mllr_dump(kb->regA,kb->regB,mgau_veclen(kbcore_mgau(kb->kbcore)));
    /*This generates huge amount of information */
    /*mgau_dump(kbcore_mgau(kb->kbcore),1);*/
#endif 


    /* allocate memory for the prevmllrfn if it is too short*/
    if(strlen(mllrname)*sizeof(char) > 1024){
      kb->prevmllrfn=(char*)ckd_calloc(strlen(mllrname), sizeof(char));
    }

    strcpy(kb->prevmllrfn,mllrname);
  }else{
    /* No need to change anything for now */
  }
}


/*
 * Make the next_active information within all lextrees be the current one, after blowing
 * away the latter; in preparation for moving on to the next frame.
 */
void kb_lextree_active_swap (kb_t *kb)
{
    int32 i;
    
    for (i = 0; i < kb->n_lextree; i++) {
	lextree_active_swap (kb->ugtree[i]);
	lextree_active_swap (kb->fillertree[i]);
    }
}

void
kb_freehyps(kb_t *kb)
{
  int i = 0;

  if (kb->hyp_segs) {
    for (; i < kb->hyp_seglen; i++) {
      if (kb->hyp_segs[i]) {
	ckd_free(kb->hyp_segs[i]);
      }
    }
    ckd_free(kb->hyp_segs);
	kb->hyp_segs = 0;
    kb->hyp_seglen = 0;
  }
	
  if (kb->hyp_str) {
    ckd_free(kb->hyp_str);
	kb->hyp_str = 0;
    kb->hyp_strlen = 0;
  }
}

/* RAH 4.15.01 Lots of memory is allocated, but never freed, this function will clean up.
 * First pass will get the low hanging fruit.*/
void kb_free (kb_t *kb)
{
  vithist_t *vithist = kb->vithist;

  if (kb->sen_active)
    ckd_free ((void *)kb->sen_active);
  if (kb->ssid_active) 
    ckd_free ((void *)kb->ssid_active);
  if (kb->comssid_active)
    ckd_free ((void *)kb->comssid_active);
  if (kb->fillertree) 
    ckd_free ((void *)kb->fillertree);
  if (kb->hmm_hist) 
    ckd_free ((void *)kb->hmm_hist);
  

  /* vithist */
  if (vithist) {
    ckd_free ((void *) vithist->entry);
    ckd_free ((void *) vithist->frame_start);
    ckd_free ((void *) vithist->bestscore);
    ckd_free ((void *) vithist->bestvh);
    ckd_free ((void *) vithist->lms2vh_root);    
    ckd_free ((void *) kb->vithist);
  }


  kbcore_free (kb->kbcore);

  if (kb->feat) {
    ckd_free ((void *)kb->feat[0][0]);
    ckd_free_2d ((void **)kb->feat);
  }

  if (kb->cache_ci_senscr) {
    ckd_free_2d ((void **)kb->cache_ci_senscr);
  }
  if( kb->cache_best_list) {
    ckd_free((void*) kb->cache_best_list);
  }
  if(kb->phn_heur_list) {
    ckd_free((void*) kb->phn_heur_list);
  }

  if (kb->matchsegfp) fclose(kb->matchsegfp);
  if (kb->matchfp) fclose(kb->matchfp);

  if(kb->prevmllrfn) ckd_free((char*) kb->prevmllrfn);
  if(kb->regA && kb->regB) mllr_free_regmat(kb->regA, kb->regB);
  kb_freehyps(kb);
}
