/*
 * main.c -- Main tree decoder driver module.
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
 * 02-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include <libmain/cmn.h>
#include <libmain/agc.h>
#include <libmain/hyp.h>
#include "kb.h"
#include "lextree.h"


static arg_t arglist[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-feat",
      ARG_STRING,
      "s3_1x39",
      "Feature type: s3_1x39 / s2_4x / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
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
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
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
      "0.02",
      "Default non-silence filler word probability" },
    { "-lw",
      ARG_FLOAT32,
      "9.5",
      "Language weight" },
    { "-wip",
      ARG_FLOAT32,
      "0.2",
      "Word insertion penalty" },
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
      "Mixture gaussian variance floor (applied to -var file)" },
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixw",
      REQARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to -mixw file)" },
    { "-mgaubeam",
      ARG_FLOAT32,
      "1e-3",
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" },
    { "-tmat",
      REQARG_STRING,
      NULL,
      "HMM state transition matrix input file" },
    { "-tmatfloor",
      ARG_FLOAT32,
      "0.0001",
      "HMM state transition probability floor (applied to -tmat file)" },
    { "-ctl",
      REQARG_STRING,
      NULL,
      "Control file listing utterances to be processed" },
    { "-ctloffset",
      ARG_INT32,
      "0",
      "No. of utterances at the beginning of -ctlfn file to be skipped" },
    { "-ctlcount",
      ARG_INT32,
      "1000000000",	/* A big number to approximate the default: "until EOF" */
      "No. of utterances to be processed (after -ctloffset)" },
    { "-cepdir",
      ARG_STRING,
      ".",
      "Directory of mfc files (for utterances in -ctl file)" },
    { "-cepext",
      ARG_STRING,
      "mfc",
      "File extension to be appended to utterances listed in -ctlfn file" },
    { "-agc",
      ARG_STRING,
      "max",
      "Automatic Gain Control.  max: C0 -= max(C0) in current utt; none: no AGC" },
    { "-cmn",
      ARG_STRING,
      "current",
      "Cepstral Mean Normalization.  current: C[1..n-1] -= mean(C[1..n-1]) in current utt; none: no CMN" },
    { "-beam",
      ARG_FLOAT64,
      "1e-70",
      "Main pruning beam" },
    { "-wordbeam",
      ARG_FLOAT64,
      "1e-30",
      "Default cross-word pruning beam" },
    { "-wordmax",
      ARG_INT32,
      "1000",
      "Max word exit candidates to expand (limits whatever passes wordbeam)" },
    { "-flatdepth",
      ARG_INT32,
      "100000000",
      "Tree depth at which it is flattened" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


static void decode_utt (void *data, char *uttfile, int32 sf, int32 ef, char *uttid)
{
    kb_t *kb;
    acoustic_t *am;
    int32 featwin, nfr, min_utt_frames, n_vithist;
    char cepfile[4096], latfile[4096];
    vithist_t *finalhist;
    int32 i, f;
    glist_t hyplist;
    FILE *latfp;

    printf ("\n");
    fflush (stdout);
    E_INFO("Utterance %s\n", uttid);
    
    kb = (kb_t *)data;
    am = kb->am;
    featwin = feat_window_size(am->fcb);
    
    /* Build complete cepfile name and read cepstrum data; check for min length */
    ctl_infile (cepfile, cmd_ln_str("-cepdir"), cmd_ln_str("-cepext"), uttfile);

    if ((nfr = s2mfc_read (cepfile, sf, ef, featwin, am->mfc, S3_MAX_FRAMES)) < 0) {
	E_ERROR("%s: MFC read failed\n", uttid);
	return;
    }
    E_INFO("%s: %d frames\n", uttid, nfr-(featwin<<1));
    
    ptmr_reset (kb->tm);
    ptmr_reset (kb->tm_search);
    ptmr_start (kb->tm);
    
    min_utt_frames = (featwin<<1) + 1;
    if (nfr < min_utt_frames) {
	E_ERROR("%s: Utterance shorter than %d frames; ignored\n",
		uttid, min_utt_frames, nfr);
	return;
    }
    
    /* CMN/AGC */
    if (strcmp (cmd_ln_str("-cmn"), "current") == 0)
	cmn (am->mfc, nfr, feat_cepsize(am->fcb));
    if (strcmp (cmd_ln_str("-agc"), "max") == 0)
	agc_max (am->mfc, nfr);
    
    /* Process utterance */
    lextree_vit_start (kb, uttid);
    for (i = featwin, f = 0; i < nfr-featwin; i++, f++) {
	am->senscale[f] = acoustic_eval (am, i);
	
	ptmr_start (kb->tm_search);
	
	lextree_vit_frame (kb, f, uttid);
	printf (" %d,%d,%d", f, glist_count (kb->vithist[f]), glist_count (kb->lextree_active));
	fflush (stdout);
	
	ptmr_stop (kb->tm_search);
    }
    printf ("\n");
    finalhist = lextree_vit_end (kb, f, uttid);
    
    hyplist = vithist_backtrace (finalhist, kb->am->senscale);
    hyp_log (stdout, hyplist, _dict_wordstr, (void *)kb->dict);
    hyp_myfree (hyplist);
    printf ("\n");
    
    /* Log the entire Viterbi word lattice */
    sprintf (latfile, "%s.lat", uttid);
    if ((latfp = fopen(latfile, "w")) == NULL) {
	E_ERROR("fopen(%s,w) failed; using stdout\n", latfile);
	latfp = stdout;
    }
    n_vithist = vithist_log (latfp, kb->vithist, f, _dict_wordstr, (void *)kb->dict);
    if (latfp != stdout)
	fclose (latfp);
    else {
	printf ("\n");
	fflush (stdout);
    }
    
    ptmr_stop (kb->tm);
    if (f > 0) {
	printf("TMR(%s): %5d frames; %.1fs CPU, %.2f xRT; %.1fs CPU(search), %.2f xRT; %.1fs Elapsed, %.2f xRT\n",
	       uttid, f,
	       kb->tm->t_cpu, kb->tm->t_cpu * 100.0 / f,
	       kb->tm_search->t_cpu, kb->tm_search->t_cpu * 100.0 / f,
	       kb->tm->t_elapsed, kb->tm->t_elapsed * 100.0 / f);
	printf("CTR(%s): %5d frames; %d Sen (%.1f/fr); %d HMM (%.1f/fr); %d Words (%.1f/fr)\n",
	       uttid, f,
	       kb->n_sen_eval, ((float64)kb->n_sen_eval) / f,
	       kb->n_hmm_eval, ((float64)kb->n_hmm_eval) / f,
	       n_vithist, ((float64) n_vithist) / f);
    }
    
    /* Cleanup */
    glist_free (kb->lextree_active);
    kb->lextree_active = NULL;
    for (; f >= -1; --f) {	/* I.e., including dummy START_WORD node at frame -1 */
	glist_myfree (kb->vithist[f], sizeof(vithist_t));
	kb->vithist[f] = NULL;
    }
    
    lm_cache_reset (kb->lm);
}


main (int32 argc, char *argv[])
{
    kb_t kb;
    kbcore_t *kbcore;
    bitvec_t active;
    int32 w;
    
    cmd_ln_parse (arglist, argc, argv);
    unlimit();
    
    kbcore = kbcore_init (cmd_ln_float32("-logbase"),
			  cmd_ln_str("-feat"),
			  cmd_ln_str("-mdef"),
			  cmd_ln_str("-dict"),
			  cmd_ln_str("-fdict"),
			  cmd_ln_str("-compsep"),
			  cmd_ln_str("-lm"),
			  cmd_ln_str("-fillpen"),
			  cmd_ln_float32("-silprob"),
			  cmd_ln_float32("-fillprob"),
			  cmd_ln_float32("-lw"),
			  cmd_ln_float32("-wip"),
			  cmd_ln_str("-mean"),
			  cmd_ln_str("-var"),
			  cmd_ln_float32("-varfloor"),
			  cmd_ln_str("-senmgau"),
			  cmd_ln_str("-mixw"),
			  cmd_ln_float32("-mixwfloor"),
			  cmd_ln_str("-tmat"),
			  cmd_ln_float32("-tmatfloor"));
    
    /* Here's the perfect candidate for inheritance */
    kb.mdef = kbcore->mdef;
    kb.dict = kbcore->dict;
    kb.lm = kbcore->lm;
    kb.fillpen = kbcore->fillpen;
    kb.tmat = kbcore->tmat;
    kb.dict2lmwid = kbcore->dict2lmwid;
    
    if ((kb.am = acoustic_init (kbcore->fcb, kbcore->gau, kbcore->sen, cmd_ln_float32("-mgaubeam"),
				S3_MAX_FRAMES)) == NULL) {
	E_FATAL("Acoustic models initialization failed\n");
    }
    kb.beam = logs3 (cmd_ln_float64("-beam"));
    kb.wordbeam = logs3 (cmd_ln_float64("-wordbeam"));
    kb.wordmax = cmd_ln_int32("-wordmax");
    
    /* Mark the active words and build lextree */
    active = bitvec_alloc (dict_size (kb.dict));
    bitvec_clear_all (active, dict_size(kb.dict));
    for (w = 0; w < dict_size(kb.dict); w++) {
	if (IS_LMWID(kb.dict2lmwid[w]) || dict_filler_word (kb.dict, w))
	    bitvec_set (active, w);
    }
    kb.lextree_root = lextree_build (kb.dict, kb.mdef, active, cmd_ln_int32("-flatdepth"));
    
    kb.vithist = (glist_t *) ckd_calloc (S3_MAX_FRAMES+2, sizeof(glist_t));
    kb.vithist++;	/* Allow for dummy frame -1 for start word */
    kb.lextree_active = NULL;
    kb.wd_last_sf = (int32 *) ckd_calloc (dict_size(kb.dict), sizeof(int32));
    
    kb.tm = (ptmr_t *) ckd_calloc (1, sizeof(ptmr_t));
    kb.tm_search = (ptmr_t *) ckd_calloc (1, sizeof(ptmr_t));
    
    ctl_process (cmd_ln_str("-ctl"), cmd_ln_int32("-ctloffset"), cmd_ln_int32("-ctlcount"),
		 decode_utt, &kb);
    
    exit(0);
}
