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

void kb_init (kb_t *kb)
{
    kbcore_t *kbcore;
    mdef_t *mdef;
    dict_t *dict;
    dict2pid_t *d2p;
    lm_t *lm;
    s3cipid_t sil, ci;
    s3wid_t w;
    int32 i, n, n_lc;
    wordprob_t *wp;
    s3cipid_t *lc;
    bitvec_t lc_active;
    char *str;
    
    kb->kbcore = kbcore_init (cmd_ln_float32 ("-logbase"),
			      "s3_1x39",    /* Hack!! Hardwired constant 
						for -feat argument */
			      cmd_ln_str("-cmn"),
			      cmd_ln_str("-varnorm"),
			      cmd_ln_str("-agc"),
			      cmd_ln_str("-mdef"),
			      cmd_ln_str("-dict"),
			      cmd_ln_str("-fdict"),
			      "",	/* Hack!! Hardwired constant 
						for -compsep argument */
			      cmd_ln_str("-lm"),
			      cmd_ln_str("-fillpen"),
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
			      cmd_ln_str("-tmat"),
			      cmd_ln_float32("-tmatfloor"));
    
    kbcore = kb->kbcore;
    
    mdef = kbcore_mdef(kbcore);
    dict = kbcore_dict(kbcore);
    lm = kbcore_lm(kbcore);
    d2p = kbcore_dict2pid(kbcore);
    
    if (NOT_S3WID(dict_startwid(dict)) || NOT_S3WID(dict_finishwid(dict)))
	E_FATAL("%s or %s not in dictionary\n", S3_START_WORD, S3_FINISH_WORD);
    if (NOT_S3LMWID(lm_startwid(lm)) || NOT_S3LMWID(lm_finishwid(lm)))
	E_FATAL("%s or %s not in LM\n", S3_START_WORD, S3_FINISH_WORD);
    
    /* Check that HMM topology restrictions are not violated */
    if (tmat_chk_1skip (kbcore->tmat) < 0)
	E_FATAL("Tmat contains arcs skipping more than 1 state\n");
    
    /*
     * Unlink <s> and </s> between dictionary and LM, to prevent their 
     * recognition.  They are merely dummy words (anchors) at the beginning 
     * and end of each utterance.
     */
    lm_lmwid2dictwid(lm, lm_startwid(lm)) = BAD_S3WID;
    lm_lmwid2dictwid(lm, lm_finishwid(lm)) = BAD_S3WID;
    for (w = dict_startwid(dict); IS_S3WID(w); w = dict_nextalt(dict, w))
	kbcore->dict2lmwid[w] = BAD_S3LMWID;
    for (w = dict_finishwid(dict); IS_S3WID(w); w = dict_nextalt(dict, w))
	kbcore->dict2lmwid[w] = BAD_S3LMWID;
    
    sil = mdef_silphone (kbcore_mdef (kbcore));
    if (NOT_S3CIPID(sil))
	E_FATAL("Silence phone '%s' not in mdef\n", S3_SILENCE_CIPHONE);
    
    E_INFO("Building lextrees\n");
    
    kb->sen_active = (int32 *) ckd_calloc (mdef_n_sen(mdef), sizeof(int32));
    kb->ssid_active = (int32 *) ckd_calloc (mdef_n_sseq(mdef), sizeof(int32));
    kb->comssid_active = (int32 *) ckd_calloc (dict2pid_n_comsseq(d2p), 
							sizeof(int32));
    /* Build active word list */
    wp = (wordprob_t *) ckd_calloc (dict_size(dict), sizeof(wordprob_t));
    n = lm_ug_wordprob (lm, MAX_NEG_INT32, wp);
    if (n < 1)
	E_FATAL("%d active words\n", n);
    n = wid_wordprob2alt (dict, wp, n);	   /* Add alternative pronunciations */
    
    /* Retain or remove unigram probs from lextree, depending on option */
    if (cmd_ln_int32("-treeugprob") == 0) {
	for (i = 0; i < n; i++)
	    wp[i].prob = -1;    	/* Flatten all initial probabilities */
    }
    
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
    
    /* Create the desired no. of unigram lextrees */
    kb->n_lextree = cmd_ln_int32 ("-Nlextree");
    if (kb->n_lextree < 1) {
	E_ERROR("No. of ugtrees specified: %d; will instantiate 1 ugtree\n", 
								kb->n_lextree);
	kb->n_lextree = 1;
    }
    kb->ugtree = (lextree_t **) ckd_calloc (kb->n_lextree, sizeof(lextree_t *));
    for (i = 0; i < kb->n_lextree; i++) {
	kb->ugtree[i] = lextree_build (kbcore, wp, n, lc);
	lextree_type (kb->ugtree[i]) = 0;
    }
    bitvec_free (lc_active);
    ckd_free ((void *) lc);
    
    /* Create filler lextrees */
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
    
    E_INFO("Lextrees(%d), %d nodes(ug), %d nodes(filler)\n",
	   kb->n_lextree, lextree_n_node(kb->ugtree[0]), 
			lextree_n_node(kb->fillertree[0]));
    
    if (cmd_ln_int32("-lextreedump")) {
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
    
    if ((kb->feat = feat_array_alloc(kbcore_fcb(kbcore),S3_MAX_FRAMES)) == NULL)
	E_FATAL("feat_array_alloc() failed\n");
    
    kb->vithist = vithist_init(kbcore, kb->beam->word, cmd_ln_int32("-bghist"));
    
    ptmr_init (&(kb->tm_sen));
    ptmr_init (&(kb->tm_srch));
    kb->tot_fr = 0;
    kb->tot_sen_eval = 0.0;
    kb->tot_gau_eval = 0.0;
    kb->tot_hmm_eval = 0.0;
    kb->tot_wd_exit = 0.0;
    
    kb->hmm_hist_binsize = cmd_ln_int32("-hmmhistbinsize");
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

