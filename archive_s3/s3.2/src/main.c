/*
 * main.c -- 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to allow run-time choice between 3-state and 5-state HMM
 * 		topologies (instead of compile-time selection).
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxwpf.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "kb.h"
#include "corpus.h"


static arg_t arg[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0003",
      "Base in which all log-likelihoods calculated" },
#if 0
    /* Commented out; must be s3_1x39 */
    { "-feat",
      ARG_STRING,
      NULL,
      "Feature type: Must be s3_1x39 / s2_4x / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
#endif
    { "-cmn",
      ARG_STRING,
      "current",
      "Cepstral mean normalization scheme (default: Cep -= mean-over-current-sentence(Cep))" },
    { "-varnorm",
      ARG_STRING,
      "no",
      "Variance normalize each utterance (yes/no; only applicable if CMN is also performed)" },
    { "-agc",
      ARG_STRING,
      "max",
      "Automatic gain control for c0 ('max' or 'none'); (max: c0 -= max-over-current-sentence(c0))" },
    { "-mdef",
      REQARG_STRING,
      NULL,
      "Model definition input file" },
    { "-dict",
      REQARG_STRING,
      NULL,
      "Pronunciation dictionary input file" },
    { "-fdict",
      REQARG_STRING,
      NULL,
      "Filler word pronunciation dictionary input file" },
#if 0
    /* Commented out; not supported */
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
#endif
    { "-lm",
      REQARG_STRING,
      NULL,
      "Word trigram language model input file" },
    { "-fillpen",
      ARG_STRING,
      NULL,
      "Filler word probabilities input file" },
    { "-silprob",
      ARG_FLOAT32,
      "0.1",
      "Default silence word probability" },
    { "-fillprob",
      ARG_FLOAT32,
      "0.1",
      "Default non-silence filler word probability" },
    { "-lw",
      ARG_FLOAT32,
      "8.5",
      "Language weight" },
    { "-wip",
      ARG_FLOAT32,
      "0.7",
      "Word insertion penalty" },
    { "-uw",
      ARG_FLOAT32,
      "0.7",
      "Unigram weight" },
    { "-mean",
      REQARG_STRING,
      NULL,
      "Mixture gaussian means input file" },
    { "-var",
      REQARG_STRING,
      NULL,
      "Mixture gaussian variances input file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to data from -var file)" },
#if 0
    /* Commented out; must be .cont. */
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
#endif
    { "-mixw",
      REQARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to data from -mixw file)" },
    { "-subvq",
      ARG_STRING,
      NULL,
      "Sub-vector quantized form of acoustic model" },
    { "-tmat",
      REQARG_STRING,
      NULL,
      "HMM state transition matrix input file" },
    { "-tmatfloor",
      ARG_FLOAT32,
      "0.0001",
      "HMM state transition probability floor (applied to -tmat file)" },
    { "-Nlextree",
      ARG_INT32,
      "3",
      "No. of lextrees to be instantiated; entries into them staggered in time" },
    { "-epl",
      ARG_INT32,
      "3",
      "Entries Per Lextree; #successive entries into one lextree before lextree-entries shifted to the next" },
    { "-subvqbeam",
      ARG_FLOAT64,
      "3.0e-3",
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" },
    { "-beam",
      ARG_FLOAT64,
      "1.0e-55",
      "Beam selecting active HMMs (relative to best) in each frame [0(widest)..1(narrowest)]" },
    { "-pbeam",
      ARG_FLOAT64,
      "1.0e-50",
      "Beam selecting HMMs transitioning to successors in each frame [0(widest)..1(narrowest)]" },
    { "-wbeam",
      ARG_FLOAT64,
      "1.0e-35",
      "Beam selecting word-final HMMs exiting in each frame [0(widest)..1(narrowest)]" },
    { "-ctl",
      ARG_STRING,
      NULL,
      "Control file listing utterances to be processed" },
    { "-utt",
      ARG_STRING,
      NULL,
      "Utterance file to be processed (-ctlcount argument times)" },
    { "-ctloffset",
      ARG_INT32,
      "0",
      "No. of utterances at the beginning of -ctl file to be skipped" },
    { "-ctlcount",
      ARG_INT32,
      "1000000000",	/* A big number to approximate the default: "until EOF" */
      "No. of utterances to be processed (after skipping -ctloffset entries)" },
    { "-cepdir",
      ARG_STRING,
      NULL,
      "Input cepstrum files directory (prefixed to filespecs in control file)" },
    { "-bptbldir",
      ARG_STRING,
      NULL,
      "Directory in which to dump word Viterbi back pointer table (for debugging)" },
    { "-outlatdir",
      ARG_STRING,
      NULL,
      "Directory in which to dump word lattices" },
    { "-outlatoldfmt",
      ARG_INT32,
      "1",
      "Whether to dump lattices in old format" },
    { "-latext",
      ARG_STRING,
      "lat.gz",
      "Filename extension for lattice files (gzip compressed, by default)" },
    { "-hmmdump",
      ARG_INT32,
      "0",
      "Whether to dump active HMM details to stderr (for debugging)" },
    { "-lextreedump",
      ARG_INT32,
      "0",
      "Whether to dump the lextree structure to stderr (for debugging)" },
    { "-maxwpf",
      ARG_INT32,
      "20",
      "Max no. of distinct word exits to maintain at each frame" },
    { "-maxhistpf",
      ARG_INT32,
      "100",
      "Max no. of histories to maintain at each frame" },
    { "-bghist",
      ARG_INT32,
      "0",
      "Bigram-mode: If TRUE only one BP entry/frame; else one per LM state" },
    { "-maxhmmpf",
      ARG_INT32,
      "20000",
      "Max no. of active HMMs to maintain at each frame; approx." },
    { "-hmmhistbinsize",
      ARG_INT32,
      "5000",
      "Performance histogram: #frames vs #HMMs active; #HMMs/bin in this histogram" },
    { "-ptranskip",
      ARG_INT32,
      "0",
      "Use wbeam for phone transitions every so many frames (if >= 1)" },
    { "-hypseg",
      ARG_STRING,
      NULL,
      "Recognition result file, with word segmentations and scores" },
    { "-treeugprob",
      ARG_INT32,
      "1",
      "If TRUE (non-0), Use unigram probs in lextree" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


static void kb_init (kb_t *kb)
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
			      "s3_1x39",	/* Hack!! Hardwired constant for -feat argument */
			      cmd_ln_str("-cmn"),
			      cmd_ln_str("-varnorm"),
			      cmd_ln_str("-agc"),
			      cmd_ln_str("-mdef"),
			      cmd_ln_str("-dict"),
			      cmd_ln_str("-fdict"),
			      "",	/* Hack!! Hardwired constant for -compsep argument */
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
     * Unlink <s> and </s> between dictionary and LM, to prevent their recognition.  They
     * are merely dummy words (anchors) at the beginning and end of each utterance.
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
    kb->comssid_active = (int32 *) ckd_calloc (dict2pid_n_comsseq(d2p), sizeof(int32));
    
    /* Build active word list */
    wp = (wordprob_t *) ckd_calloc (dict_size(dict), sizeof(wordprob_t));
    n = lm_ug_wordprob (lm, MAX_NEG_INT32, wp);
    if (n < 1)
	E_FATAL("%d active words\n", n);
    n = wid_wordprob2alt (dict, wp, n);		/* Add alternative pronunciations */
    
    /* Retain or remove unigram probs from lextree, depending on option */
    if (cmd_ln_int32("-treeugprob") == 0) {
	for (i = 0; i < n; i++)
	    wp[i].prob = -1;	/* Flatten all initial probabilities to a constant */
    }
    
    /* Build set of all possible left contexts */
    lc = (s3cipid_t *) ckd_calloc (mdef_n_ciphone (mdef) + 1, sizeof(s3cipid_t));
    lc_active = bitvec_alloc (mdef_n_ciphone (mdef));
    for (w = 0; w < dict_size (dict); w++) {
	ci = dict_pron (dict, w, dict_pronlen(dict, w) - 1);
	if (! mdef_is_fillerphone (mdef, ci))
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
	E_ERROR("No. of ugtrees specified: %d; will instantiate 1 ugtree\n", kb->n_lextree);
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
    kb->fillertree = (lextree_t **) ckd_calloc (kb->n_lextree, sizeof(lextree_t *));
    for (i = 0; i < kb->n_lextree; i++) {
	kb->fillertree[i] = lextree_build (kbcore, wp, n, NULL);
	lextree_type (kb->fillertree[i]) = -1;
    }
    ckd_free ((void *) wp);
    
    E_INFO("Lextrees(%d), %d nodes(ug), %d nodes(filler)\n",
	   kb->n_lextree, lextree_n_node(kb->ugtree[0]), lextree_n_node(kb->fillertree[0]));
    
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
    
    kb->ascr = ascr_init (mgau_n_mgau(kbcore_mgau(kbcore)), kbcore->dict2pid->n_comstate);
    kb->beam = beam_init (cmd_ln_float64("-subvqbeam"),
			  cmd_ln_float64("-beam"),
			  cmd_ln_float64("-pbeam"),
			  cmd_ln_float64("-wbeam"));
    E_INFO("Beam= %d, PBeam= %d, WBeam= %d, SVQBeam= %d\n",
	   kb->beam->hmm, kb->beam->ptrans, kb->beam->word, kb->beam->subvq);
    
    if ((kb->feat = feat_array_alloc (kbcore_fcb(kbcore), S3_MAX_FRAMES)) == NULL)
	E_FATAL("feat_array_alloc() failed\n");
    
    kb->vithist = vithist_init (kbcore, kb->beam->word, cmd_ln_int32 ("-bghist"));
    
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
	if ((kb->matchsegfp = fopen(str, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed; use FWDXCT: from std logfile\n", str);
    }
}


/*
 * Make the next_active information within all lextrees be the current one, after blowing
 * away the latter; in preparation for moving on to the next frame.
 */
static void kb_lextree_active_swap (kb_t *kb)
{
    int32 i;
    
    for (i = 0; i < kb->n_lextree; i++) {
	lextree_active_swap (kb->ugtree[i]);
	lextree_active_swap (kb->fillertree[i]);
    }
}


/*
 * Begin search at bigrams of <s>, backing off to unigrams; and fillers.  Update
 * kb->lextree_next_active with the list of active lextrees.
 */
static void utt_begin (kb_t *kb)
{
    kbcore_t *kbc;
    int32 n, pred;
    
    kbc = kb->kbcore;
    
    /* Insert initial <s> into vithist structure */
    pred = vithist_utt_begin (kb->vithist, kbc);
    assert (pred == 0);	/* Vithist entry ID for <s> */
    
    /* Enter into unigram lextree[0] */
    n = lextree_n_next_active(kb->ugtree[0]);
    assert (n == 0);
    lextree_enter (kb->ugtree[0], mdef_silphone(kbc->mdef), -1, 0, pred, kb->beam->hmm);
    
    /* Enter into filler lextree */
    n = lextree_n_next_active(kb->fillertree[0]);
    assert (n == 0);
    lextree_enter (kb->fillertree[0], BAD_S3CIPID, -1, 0, pred, kb->beam->hmm);
    
    kb->n_lextrans = 1;
    
    kb_lextree_active_swap (kb);
}


static void matchseg_write (FILE *fp, kb_t *kb, glist_t hyp, char *hdr)
{
    gnode_t *gn;
    hyp_t *h;
    int32 ascr, lscr;
    dict_t *dict;
    
    ascr = 0;
    lscr = 0;
    
    for (gn = hyp; gn; gn = gnode_next(gn)) {
	h = (hyp_t *) gnode_ptr (gn);
	ascr += h->ascr;
	lscr += h->lscr;
    }
    
    dict = kbcore_dict(kb->kbcore);
    
    fprintf (fp, "%s%s S 0 T %d A %d L %d", (hdr ? hdr : ""), kb->uttid, ascr+lscr, ascr, lscr);
    for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
	h = (hyp_t *) gnode_ptr (gn);
	fprintf (fp, " %d %d %d %s", h->sf, h->ascr, h->lscr, dict_wordstr (dict, h->id));
    }
    fprintf (fp, " %d\n", kb->nfr);
    fflush (fp);
}


static void utt_end (kb_t *kb)
{
    int32 id, ascr, lscr;
    glist_t hyp;
    gnode_t *gn;
    hyp_t *h;
    FILE *fp, *latfp;
    dict_t *dict;
    int32 i;
    
    fp = stdout;
    dict = kbcore_dict (kb->kbcore);
    
    if ((id = vithist_utt_end (kb->vithist, kb->kbcore)) >= 0) {
	if (cmd_ln_str("-bptbldir")) {
	    char file[8192];
	    
	    sprintf (file, "%s/%s.bpt", cmd_ln_str ("-bptbldir"), kb->uttid);
	    if ((latfp = fopen (file, "w")) == NULL) {
		E_ERROR("fopen(%s,w) failed; using stdout\n", file);
		latfp = stdout;
	    }
	    
	    vithist_dump (kb->vithist, -1, kb->kbcore, latfp);
	    if (latfp != stdout)
		fclose (latfp);
	}
	
	hyp = vithist_backtrace (kb->vithist, id);
	
	/* Detailed backtrace */
	fprintf (fp, "\nBacktrace(%s)\n", kb->uttid);
	fprintf (fp, "%6s %5s %5s %11s %8s %4s\n",
		 "LatID", "SFrm", "EFrm", "AScr", "LScr", "Type");
	
	ascr = 0;
	lscr = 0;
	
	for (gn = hyp; gn; gn = gnode_next(gn)) {
	    h = (hyp_t *) gnode_ptr (gn);
	    fprintf (fp, "%6d %5d %5d %11d %8d %4d %s\n",
		     h->vhid, h->sf, h->ef, h->ascr, h->lscr, h->type,
		     dict_wordstr(dict, h->id));
	    
	    ascr += h->ascr;
	    lscr += h->lscr;
	}
	fprintf (fp, "       %5d %5d %11d %8d (Total)\n", 0, kb->nfr, ascr, lscr);
	
	/* Match */
	fprintf (fp, "\nFWDVIT: ");
	for (gn = hyp; gn; gn = gnode_next(gn)) {
	    h = (hyp_t *) gnode_ptr (gn);
	    if ((! dict_filler_word(dict, h->id)) && (h->id != dict_finishwid(dict)))
		fprintf (fp, "%s ", dict_wordstr (dict, dict_basewid(dict, h->id)));
	}
	fprintf (fp, " (%s)\n\n", kb->uttid);
	
	/* Matchseg */
	if (kb->matchsegfp)
	    matchseg_write (kb->matchsegfp, kb, hyp, NULL);
	matchseg_write (fp, kb, hyp, "FWDXCT: ");
	fprintf (fp, "\n");
	
	if (cmd_ln_str ("-outlatdir")) {
	    char str[16384];
	    int32 ispipe;
	    float64 logbase;
	    
	    sprintf (str, "%s/%s.%s",
		     cmd_ln_str("-outlatdir"), kb->uttid, cmd_ln_str("-latext"));
	    E_INFO("Writing lattice file: %s\n", str);
	    
	    if ((latfp = fopen_comp (str, "w", &ispipe)) == NULL)
		E_ERROR("fopen_comp (%s,w) failed\n", str);
	    else {
		/* Write header info */
		getcwd (str, sizeof(str));
		fprintf (latfp, "# getcwd: %s\n", str);
		
		/* Print logbase first!!  Other programs look for it early in the DAG */
		logbase = cmd_ln_float32 ("-logbase");
		fprintf (latfp, "# -logbase %e\n", logbase);
		
		fprintf (latfp, "# -dict %s\n", cmd_ln_str ("-dict"));
		if (cmd_ln_str ("-fdict"))
		    fprintf (latfp, "# -fdict %s\n", cmd_ln_str ("-fdict"));
		fprintf (latfp, "# -lm %s\n", cmd_ln_str ("-lm"));
		fprintf (latfp, "# -mdef %s\n", cmd_ln_str ("-mdef"));
		fprintf (latfp, "# -mean %s\n", cmd_ln_str ("-mean"));
		fprintf (latfp, "# -var %s\n", cmd_ln_str ("-var"));
		fprintf (latfp, "# -mixw %s\n", cmd_ln_str ("-mixw"));
		fprintf (latfp, "# -tmat %s\n", cmd_ln_str ("-tmat"));
		fprintf (latfp, "#\n");
		
		fprintf (latfp, "Frames %d\n", kb->nfr);
		fprintf (latfp, "#\n");
		
		vithist_dag_write (kb->vithist, hyp, dict, cmd_ln_int32("-outlatoldfmt"), latfp);
		fclose_comp (latfp, ispipe);
	    }
	}
	
	/* Free hyplist */
	for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
	    h = (hyp_t *) gnode_ptr (gn);
	    ckd_free ((void *) h);
	}
	glist_free (hyp);
    } else
	E_ERROR("%s: No recognition\n\n", kb->uttid);
    
    E_INFO("%4d frm;  %4d sen, %5d gau/fr, %4.1f CPU %4.1f Clk;  %5d hmm, %3d wd/fr, %4.1f CPU %4.1f Clk (%s)\n",
	   kb->nfr,
	   (kb->utt_sen_eval + (kb->nfr >> 1)) / kb->nfr,
	   (kb->utt_gau_eval + (kb->nfr >> 1)) / kb->nfr,
	   kb->tm_sen.t_cpu * 100.0 / kb->nfr, kb->tm_sen.t_elapsed * 100.0 / kb->nfr,
	   (kb->utt_hmm_eval + (kb->nfr >> 1)) / kb->nfr,
	   (vithist_n_entry(kb->vithist) + (kb->nfr >> 1)) / kb->nfr,
	   kb->tm_srch.t_cpu * 100.0 / kb->nfr, kb->tm_srch.t_elapsed * 100.0 / kb->nfr,
	   kb->uttid);
    
    {
	int32 j, k;
	
	for (j = kb->hmm_hist_bins-1; (j >= 0) && (kb->hmm_hist[j] == 0); --j);
	E_INFO("HMMHist[0..%d](%s):", j, kb->uttid);
	for (i = 0, k = 0; i <= j; i++) {
	    k += kb->hmm_hist[i];
	    fprintf (stderr, " %d(%d)", kb->hmm_hist[i], (k*100)/kb->nfr);
	}
	fprintf (stderr, "\n");
	fflush (stderr);
    }
    
    kb->tot_sen_eval += kb->utt_sen_eval;
    kb->tot_gau_eval += kb->utt_gau_eval;
    kb->tot_hmm_eval += kb->utt_hmm_eval;
    kb->tot_wd_exit += vithist_n_entry(kb->vithist);
    
    ptmr_reset (&(kb->tm_sen));
    ptmr_reset (&(kb->tm_srch));
    
    system ("ps aguxwww | grep dec");
    
    for (i = 0; i < kb->n_lextree; i++) {
	lextree_utt_end (kb->ugtree[i], kb->kbcore);
	lextree_utt_end (kb->fillertree[i], kb->kbcore);
    }
    
    vithist_utt_reset (kb->vithist);
    
    lm_cache_stats_dump (kbcore_lm(kb->kbcore));
    lm_cache_reset (kbcore_lm(kb->kbcore));
}


static void utt_word_trans (kb_t *kb, int32 cf)
{
    int32 k, th;
    vithist_t *vh;
    vithist_entry_t *ve;
    int32 vhid, le, n_ci, score;
    static int32 *bs = NULL, *bv = NULL, epl;
    s3wid_t wid;
    int32 p;
    dict_t *dict;
    mdef_t *mdef;
    
    vh = kb->vithist;
    th = kb->bestscore + kb->beam->hmm;	/* Pruning threshold */
    
    if (vh->bestvh[cf] < 0)
	return;

    dict = kbcore_dict(kb->kbcore);
    mdef = kbcore_mdef(kb->kbcore);
    n_ci = mdef_n_ciphone(mdef);
    
    /* Initialize best exit for each distinct word-final CIphone to NONE */
    if (! bs) {
	bs = (int32 *) ckd_calloc (n_ci, sizeof(int32));
	bv = (int32 *) ckd_calloc (n_ci, sizeof(int32));
	epl = cmd_ln_int32 ("-epl");
    }
    for (p = 0; p < n_ci; p++) {
	bs[p] = MAX_NEG_INT32;
	bv[p] = -1;
    }
    
    /* Find best word exit in this frame for each distinct word-final CI phone */
    vhid = vithist_first_entry (vh, cf);
    le = vithist_n_entry (vh) - 1;
    for (; vhid <= le; vhid++) {
	ve = vithist_id2entry (vh, vhid);
	if (! vithist_entry_valid(ve))
	    continue;
	
	wid = vithist_entry_wid (ve);
	p = dict_last_phone (dict, wid);
	if (mdef_is_fillerphone(mdef, p))
	    p = mdef_silphone(mdef);
	
	score = vithist_entry_score (ve);
	if (score > bs[p]) {
	    bs[p] = score;
	    bv[p] = vhid;
	}
    }
    
    /* Find lextree instance to be entered */
    k = kb->n_lextrans++;
    k = (k % (kb->n_lextree * epl)) / epl;
    
    /* Transition to unigram lextrees */
    for (p = 0; p < n_ci; p++) {
	if (bv[p] >= 0)
	    lextree_enter (kb->ugtree[k], p, cf, bs[p], bv[p], th);
    }
    
    /* Transition to filler lextrees */
    lextree_enter (kb->fillertree[k], BAD_S3CIPID, cf, vh->bestscore[cf], vh->bestvh[cf], th);
}


/* Invoked by ctl_process in libmisc/corpus.c */
static void utt_decode (void *data, char *uttfile, int32 sf, int32 ef, char *uttid)
{
    kb_t *kb;
    kbcore_t *kbcore;
    mdef_t *mdef;
    dict_t *dict;
    dict2pid_t *d2p;
    mgau_model_t *mgau;
    subvq_t *svq;
    lextree_t *lextree;
    int32 besthmmscr, bestwordscr, th, pth, wth, maxwpf, maxhistpf, maxhmmpf, ptranskip;
    int32 i, j, f;
    int32 n_hmm_eval, frm_nhmm, hb, pb, wb;
    FILE *hmmdumpfp;
    
    E_INFO("Processing: %s\n", uttid);
    
    kb = (kb_t *) data;
    kbcore = kb->kbcore;
    mdef = kbcore_mdef (kbcore);
    dict = kbcore_dict (kbcore);
    d2p = kbcore_dict2pid (kbcore);
    mgau = kbcore_mgau (kbcore);
    svq = kbcore_svq (kbcore);
    
    kb->uttid = uttid;
    
    hmmdumpfp = cmd_ln_int32("-hmmdump") ? stderr : NULL;
    maxwpf = cmd_ln_int32 ("-maxwpf");
    maxhistpf = cmd_ln_int32 ("-maxhistpf");
    maxhmmpf = cmd_ln_int32 ("-maxhmmpf");
    ptranskip = cmd_ln_int32 ("-ptranskip");
    
    /* Read mfc file and build feature vectors for entire utterance */
    kb->nfr = feat_s2mfc2feat(kbcore_fcb(kbcore), uttfile, cmd_ln_str("-cepdir"), sf, ef,
			      kb->feat, S3_MAX_FRAMES);
    E_INFO("%s: %d frames\n", kb->uttid, kb->nfr);
    
    for (i = 0; i < kb->hmm_hist_bins; i++)
	kb->hmm_hist[i] = 0;
    
    utt_begin (kb);
    
    n_hmm_eval = 0;
    kb->utt_sen_eval = 0;
    kb->utt_gau_eval = 0;
    
    for (f = 0; f < kb->nfr; f++) {
	/* Acoustic (senone scores) evaluation */
	ptmr_start (&(kb->tm_sen));

	/* Find active senones and composite senones, from active lextree nodes */
	if (kb->sen_active) {
	    memset (kb->ssid_active, 0, mdef_n_sseq(mdef) * sizeof(int32));
	    memset (kb->comssid_active, 0, dict2pid_n_comsseq(d2p) * sizeof(int32));
	    /* Find active senone-sequence IDs (including composite ones) */
	    for (i = 0; i < (kb->n_lextree <<1); i++) {
		lextree = (i < kb->n_lextree) ? kb->ugtree[i] :
		    kb->fillertree[i - kb->n_lextree];
		lextree_ssid_active (lextree, kb->ssid_active, kb->comssid_active);
	    }
	    
	    /* Find active senones from active senone-sequences */
	    memset (kb->sen_active, 0, mdef_n_sen(mdef) * sizeof(int32));
	    mdef_sseq2sen_active (mdef, kb->ssid_active, kb->sen_active);
	    
	    /* Add in senones needed for active composite senone-sequences */
	    dict2pid_comsseq2sen_active (d2p, mdef, kb->comssid_active, kb->sen_active);
	}
	
	/* Evaluate senone acoustic scores for the active senones */
	subvq_frame_eval (svq, mgau, kb->beam->subvq, kb->feat[f][0], kb->sen_active,
			  kb->ascr->sen);
	kb->utt_sen_eval += mgau_frm_sen_eval(mgau);
	kb->utt_gau_eval += mgau_frm_gau_eval(mgau);
	
	/* Evaluate composite senone scores from senone scores */
	dict2pid_comsenscr (kbcore_dict2pid(kbcore), kb->ascr->sen, kb->ascr->comsen);
	ptmr_stop (&(kb->tm_sen));
	
	/* Search */
	ptmr_start (&(kb->tm_srch));
	
	/* Evaluate active HMMs in each lextree; note best HMM state score */
	besthmmscr = MAX_NEG_INT32;
	bestwordscr = MAX_NEG_INT32;
	frm_nhmm = 0;
	for (i = 0; i < (kb->n_lextree <<1); i++) {
	    lextree = (i < kb->n_lextree) ? kb->ugtree[i] : kb->fillertree[i - kb->n_lextree];
	    
	    if (hmmdumpfp != NULL)
		fprintf (hmmdumpfp, "Fr %d Lextree %d #HMM %d\n", f, i, lextree->n_active);
	    
	    lextree_hmm_eval (lextree, kbcore, kb->ascr, f, hmmdumpfp);
	    
	    if (besthmmscr < lextree->best)
		besthmmscr = lextree->best;
	    if (bestwordscr < lextree->wbest)
		bestwordscr = lextree->wbest;
	    
	    n_hmm_eval += lextree->n_active;
	    frm_nhmm += lextree->n_active;
	}
	if (besthmmscr > 0) {
	    E_ERROR("***ERROR*** Fr %d, best HMM score > 0 (%d); int32 wraparound?\n",
		    f, besthmmscr);
	}
	
	kb->hmm_hist[frm_nhmm / kb->hmm_hist_binsize]++;
	
	/* Set pruning threshold depending on whether number of active HMMs within limit */
	if (frm_nhmm > (maxhmmpf + (maxhmmpf >> 1))) {
	    int32 *bin, nbin, bw;
	    
	    /* Use histogram pruning */
	    nbin = 1000;
	    bw = -(kb->beam->hmm) / nbin;
	    bin = (int32 *) ckd_calloc (nbin, sizeof(int32));
	    
	    for (i = 0; i < (kb->n_lextree <<1); i++) {
		lextree = (i < kb->n_lextree) ?
		    kb->ugtree[i] : kb->fillertree[i - kb->n_lextree];
		
		lextree_hmm_histbin (lextree, besthmmscr, bin, nbin, bw);
	    }
	    
	    for (i = 0, j = 0; (i < nbin) && (j < maxhmmpf); i++, j += bin[i]);
	    ckd_free ((void *) bin);
	    
	    /* Determine hmm, phone, word beams */
	    hb = -(i * bw);
	    pb = (hb > kb->beam->ptrans) ? hb : kb->beam->ptrans;
	    wb = (hb > kb->beam->word) ? hb : kb->beam->word;
#if 0
	    E_INFO("Fr %5d, #hmm= %6d, #bin= %d, #hmm= %6d, beam= %8d, pbeam= %8d, wbeam= %8d\n",
		   f, frm_nhmm, i, j, hb, pb, wb);
#endif
	} else {
	    hb = kb->beam->hmm;
	    pb = kb->beam->ptrans;
	    wb = kb->beam->word;
	}
	
	kb->bestscore = besthmmscr;
	kb->bestwordscore = bestwordscr;
	th = kb->bestscore + hb;		/* HMM survival threshold */
	pth = kb->bestscore + pb;		/* Cross-HMM transition threshold */
	wth = kb->bestwordscore + wb;		/* Word exit threshold */
	
	/*
	 * For each lextree, determine if the active HMMs remain active for next
	 * frame, propagate scores across HMM boundaries, and note word exits.
	 */
	for (i = 0; i < (kb->n_lextree <<1); i++) {
	    lextree = (i < kb->n_lextree) ? kb->ugtree[i] : kb->fillertree[i - kb->n_lextree];
	    
	    /* Hack!! Use a narrow phone transition beam (wth) every so many frames */
	    if ((ptranskip < 1) || ((f % ptranskip) != 0))
		lextree_hmm_propagate (lextree, kbcore, kb->vithist, f, th, pth, wth);
	    else
		lextree_hmm_propagate (lextree, kbcore, kb->vithist, f, th, wth, wth);
	}
	
	/* Limit vithist entries created this frame to specified max */
	vithist_prune (kb->vithist, dict, f, maxwpf, maxhistpf, wb);
	
	/* Cross-word transitions */
	utt_word_trans (kb, f);
	
	/* Wind up this frame */
	vithist_frame_windup (kb->vithist, f, NULL, kbcore);
	
	kb_lextree_active_swap (kb);
	
	ptmr_stop (&(kb->tm_srch));
	
	if ((f % 100) == 0) {
	    fprintf (stderr, ".");
	    fflush (stderr);
	}
    }
    
    kb->utt_hmm_eval = n_hmm_eval;
    
    utt_end (kb);
    kb->tot_fr += kb->nfr;
    
    fprintf (stdout, "\n");
}


int32 main (int32 argc, char *argv[])
{
    kb_t kb;
    ptmr_t tm;
    
#if (! WIN32)
    {
	char host[4096], path[16384];
	
	gethostname (host, 1024);
	host[1023] = '\0';
	getcwd (path, sizeof(path));
	
	E_INFO ("Host: '%s'\n", host);
	E_INFO ("Directory: '%s'\n", path);
    }
#endif
    
    E_INFO("Compiled on %s, at %s\n\n", __DATE__, __TIME__);
    
    cmd_ln_parse (arg, argc, argv);
    unlimit ();
    
    kb_init (&kb);
    
    fprintf (stdout, "\n");
    
    if (cmd_ln_str ("-ctl")) {
	tm = ctl_process (cmd_ln_str("-ctl"),
			  cmd_ln_int32("-ctloffset"),
			  cmd_ln_int32("-ctlcount"),
			  utt_decode, &kb);
    } else if (cmd_ln_str ("-utt")) {
	tm = ctl_process_utt (cmd_ln_str("-utt"), cmd_ln_int32("-ctlcount"), utt_decode, &kb);
    }
    
    if (kb.matchsegfp)
	fclose (kb.matchsegfp);
    
    if (kb.tot_fr == 0)
	kb.tot_fr = 1;	/* Just to avoid /0 */
    
    E_INFO("SUMMARY:  %d fr;  %d sen, %d gau/fr, %.1f xCPU;  %d hmm/fr, %d wd/fr, %.1f xCPU;  tot: %.1f xCPU, %.1f xClk\n",
	   kb.tot_fr,
	   (int32)(kb.tot_sen_eval / kb.tot_fr),
	   (int32)(kb.tot_gau_eval / kb.tot_fr),
	   kb.tm_sen.t_tot_cpu * 100.0 / kb.tot_fr,
	   (int32)(kb.tot_hmm_eval / kb.tot_fr),
	   (int32)(kb.tot_wd_exit / kb.tot_fr),
	   kb.tm_srch.t_tot_cpu * 100.0 / kb.tot_fr,
	   tm.t_tot_cpu * 100.0 / kb.tot_fr,
	   tm.t_tot_elapsed * 100.0 / kb.tot_fr);
    
    exit(0);
}
