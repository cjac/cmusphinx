/*
 * align-main.c -- Time alignment driver.
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
 * 23-Mar-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added checks between dimensions of various KB components.
 * 		Added data (KB) argument to align_utt().
 * 
 * 22-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added NULL validation and duplicate-resolution arguments to corpus_load()
 *		call, following a change to that function definition.
 * 
 * 11-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libfeat/libfeat.h>
#include <libmisc/libmisc.h>
#include <libmain/cmn.h>
#include <libmain/agc.h>

#include "align.h"
#include "wdnet.h"


static arg_t arglist[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-feat",
      ARG_STRING,
      "s3_1x39",
      "Feature type: s3_1x39 / s2_4x / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
    { "-mdeffn",
      REQARG_STRING,
      NULL,
      "Model definition input file" },
    { "-dictfn",
      REQARG_STRING,
      NULL,
      "Pronunciation dictionary input file" },
    { "-fdictfn",
      REQARG_STRING,
      NULL,
      "Filler word pronunciation dictionary input file" },
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
    { "-meanfn",
      REQARG_STRING,
      NULL,
      "Mixture gaussian means input file" },
    { "-varfn",
      REQARG_STRING,
      NULL,
      "Mixture gaussian variances input file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to -var file)" },
    { "-senmgaufn",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixwfn",
      REQARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to -mixw file)" },
    { "-tmatfn",
      REQARG_STRING,
      NULL,
      "HMM state transition matrix input file" },
    { "-tmatfloor",
      ARG_FLOAT32,
      "0.0001",
      "HMM state transition probability floor (applied to -tmat file)" },
    { "-beam",
      ARG_FLOAT64,
      "1e-64",
      "Main pruning beam" },
    { "-mgaubeam",
      ARG_FLOAT64,
      "1e-3",
      "Beam selecting highest scoring components within each mixture Gaussian [0(widest)..1(narrowest)]" },
    { "-ctlfn",
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
    { "-insentfn",
      REQARG_STRING,
      NULL,
      "Input transcript file corresponding to utterances in the control file" },
    { "-noalt",
      ARG_INT32,
      "0",
      "Do NOT insert alternative pronunciations or compound words if TRUE" },
    { "-nosil",
      ARG_INT32,
      "0",
      "Do NOT insert optional silence between transcript words if TRUE" },
    { "-nofiller",
      ARG_INT32,
      "0",
      "Do NOT insert optional filler (non-silence) words between transcript words if TRUE" },
    { "-wdsegdir",
      ARG_STRING,
      NULL,
      "Output directory for writing word segmentation files; use ,CTL suffix for following control file directory structure" },
    { "-wdsegext",
      ARG_STRING,
      "wdseg",
      "File extension for word-segmentation output files" },
    { "-s2stsegdir",
      ARG_STRING,
      NULL,
      "Output directory for writing Sphinx-II format state segmentation files; use ,CTL suffix for following control file directory structure" },
    { "-s2stsegext",
      ARG_STRING,
      "v8_seg",
      "File extension for Sphinx-II format state segmentation output files" },
    { "-outsentfn",
      ARG_STRING,
      NULL,
      "Output transcript file" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


static ptmr_t *tm;
static int32 tot_frames;

#define MAX_TRANS_WORDS		4096


/*
 * Align a single utterance.  Called by the generic driver ctl_process that processes
 * a control file.
 */
static void align_utt (void *data, char *uttfile, int32 sf, int32 ef, char *uttid)
{
    char cepfile[4096], outfile[4096];
    char *str;
    char *trans, *wd[MAX_TRANS_WORDS];	/* Transcript */
    int32 nwd;
    int32 noalt, nosil, nofiller;
    glist_t wnet;		/* The word net */
    wnode_t *wstart, *wend;	/* Dummy start and end nodes in word net */
    int32 featwin;
    int32 nfr;
    int32 beam;
    int32 f, i;
    glist_t stseg, wdseg;	/* Alignment result: state/word segmentation */
    kb_t *kb;
    acoustic_t *am;
    int32 min_utt_frames;
    
    printf ("\n");
    fflush (stdout);
    E_INFO("Utterance: %s\n", uttid);
    
    kb = (kb_t *) data;
    am = kb->am;
    
    featwin = feat_window_size(am->fcb);
    f = nfr = 0;
    wnet = NULL;
    kb->pnet = NULL;
    
    ptmr_reset (tm);
    ptmr_start (tm);
    
    /* Lookup transcript; create private copy and break it up into words */
    trans = NULL;
    if ((str = corpus_lookup (kb->insent, uttid)) == NULL) {
	E_ERROR("%s: Missing or bad transcript\n", uttid);
	goto cleanup;
    }
    trans = ckd_salloc (str);
    if ((nwd = str2words (trans, wd, MAX_TRANS_WORDS)) < 0) {
	E_ERROR("%s: Transcript too long; increase MAX_TRANS_WORDS\n", uttid);
	goto cleanup;
    }
    
    noalt = *((int32 *) cmd_ln_access ("-noalt"));
    nosil = *((int32 *) cmd_ln_access ("-nosil"));
    nofiller = *((int32 *) cmd_ln_access ("-nofiller"));

    /* Build word net */
    wnet = wdnet_build (kb->dict, wd, nwd, noalt, nosil, nofiller, &wstart, &wend);
    if (! wnet) {
	E_ERROR("%s: No word net\n", uttid);
	goto cleanup;
    }
    
    /* Build phone net */
    kb->pnet = wnet2pnet (kb->mdef, kb->dict, wnet, wstart, wend,
			  &(kb->pstart), &(kb->pend));
    if (! kb->pnet) {
	E_ERROR("%s: No phone net\n", uttid);
	goto cleanup;
    }
    
    /* Build complete cepfile name and read cepstrum data */
    ctl_infile (cepfile, (char *) cmd_ln_access ("-cepdir"),
		(char *) cmd_ln_access ("-cepext"), uttfile);

    if ((nfr = s2mfc_read (cepfile, sf, ef, featwin, am->mfc, S3_MAX_FRAMES)) < 0) {
	E_ERROR("%s: MFC read failed\n", uttid);
	goto cleanup;
    }
    E_INFO("%s: %d frames\n", uttid, nfr-(featwin<<1));

    min_utt_frames = (featwin<<1) + 10;
    if (nfr < min_utt_frames) {
	E_ERROR("%s: Utterance shorter than %d frames; ignored\n",
		uttid, min_utt_frames, nfr);
	goto cleanup;
    }
    
    /* CMN/AGC */
    str = (char *) cmd_ln_access ("-cmn");
    if (strcmp (str, "current") == 0)
	cmn (am->mfc, nfr, feat_cepsize(am->fcb));
    str = (char *) cmd_ln_access ("-agc");
    if (strcmp (str, "max") == 0)
	agc_max (am->mfc, nfr);
    
    beam = logs3 (*((float64 *) cmd_ln_access ("-beam")));
    
    /* Start search */
    align_start (kb, uttid);
    
    /* Process each frame; f = running frame no. */
    for (i = featwin, f = 0; i < nfr-featwin; i++, f++) {
#if 0
	if (glist_chkdup (kb->pactive))
	    E_FATAL("%s: frame %d: Duplicate entry in active pnode list\n", uttid, f);
#endif
	kb->am->senscale[f] = acoustic_eval (kb->am, i);
	
	if (align_frame (kb, beam, f, uttid) < 0)
	    break;
    }
    
    /* Obtain recognition result: state segmentation */
    stseg = align_end (kb, f, uttid);
    if (stseg) {
	/* Output alignment as indicated; Sphinx-II format state segmentation (v8_seg) */
	if ((str = (char *) cmd_ln_access ("-s2stsegdir")) != NULL) {
	    ctl_outfile (outfile, str, (char *) cmd_ln_access ("-s2stsegext"),
			 uttfile, uttid);
	    align_write_s2stseg (kb, stseg, outfile, uttid);
	}
	
	/* Get word segmentation */
	wdseg = align_st2wdseg (kb, stseg);
	assert (wdseg);
	
	if ((str = (char *) cmd_ln_access ("-wdsegdir")) != NULL) {
	    ctl_outfile (outfile, str, (char *) cmd_ln_access ("-wdsegext"),
			 uttfile, uttid);
	    align_write_wdseg (kb, wdseg, outfile, uttid);
	}
	
	/* Output word transcript */
	if (kb->outsentfp)
	    align_write_outsent (kb, wdseg, kb->outsentfp, uttid);
	
	align_hypfree (wdseg);
	align_hypfree (stseg);
    } else
	E_ERROR("%s: Alignment failed\n", uttid);
    
cleanup:
    if (kb->pnet)
	pnet_free (kb->pnet);
    if (wnet)
	wdnet_free (wnet);
    if (trans)
	ckd_free (trans);

    ptmr_stop (tm);
    f = nfr-(featwin<<1);
    
    if (f > 0) {
	printf ("TMR(%s): %5d frames; %.1fs CPU, %.2f xRT; %.1fs Elapsed, %.2f xRT\n",
		uttid, f, tm->t_cpu, tm->t_cpu*100.0/f,
		tm->t_elapsed, tm->t_elapsed*100.0/f);
	tot_frames += f;
    } else {
	printf ("TMR(%s): %5d frames; %.1fs CPU, %.2f xRT; %.1fs Elapsed, %.2f xRT\n",
		uttid, 0, 0.0, 0.0, 0.0, 0.0);
    }
    if (am->tot_mgau_eval > 0)
	E_INFO("Active components/mgau/frame = %.1f\n",
	       am->tot_dist_valid/am->tot_mgau_eval);
    am->tot_mgau_eval = 0;
    am->tot_dist_valid = 0.0;
}


static void models_init (kb_t *kb)
{
    feat_t *fcb;
    gauden_t *gau;
    senone_t *sen;
    char *str;
    int32 i;
    float64 mbeam;
    
    logs3_init ( *((float32 *) cmd_ln_access("-logbase")) );
    
    fcb = feat_init ((char *) cmd_ln_access ("-feat"));
    
    str = (char *) cmd_ln_access ("-cmn");
    if ((strcmp (str, "current") != 0) && (strcmp (str, "none") != 0))
	E_FATAL("Unknown -cmn argument(%s); must be current or none\n");
    str = (char *) cmd_ln_access ("-agc");
    if ((strcmp (str, "max") != 0) && (strcmp (str, "none") != 0))
	E_FATAL("Unknown -agc argument(%s); must be max or none\n");
    
    kb->mdef = mdef_init ((char *) cmd_ln_access ("-mdeffn"));
    
    str = (char *) cmd_ln_access ("-compsep");
    if ((str[0] != '\0') && (str[1] != '\0'))
	E_FATAL("Compound word separator(%s) must be a single character\n", str);
    kb->dict = dict_init (kb->mdef,
			 (char *) cmd_ln_access ("-dictfn"),
			 (char *) cmd_ln_access ("-fdictfn"),
			 str[0]);
    
    gau = gauden_init ((char *) cmd_ln_access ("-meanfn"),
		       (char *) cmd_ln_access ("-varfn"),
		       *((float32 *) cmd_ln_access ("-varfloor")),
		       TRUE);
    /* Verify Gaussian density feature type with front end */
    if (feat_n_stream(fcb) != gauden_n_stream(gau))
	E_FATAL("#Feature streams mismatch: feat(%d), gauden(%d)\n",
		feat_n_stream(fcb), gauden_n_stream(gau));
    for (i = 0; i < feat_n_stream(fcb); i++) {
	if (feat_stream_len(fcb, i) != gauden_stream_len(gau, i))
	    E_FATAL("Feature streamlen[%d] mismatch: feat(%d), gauden(%d)\n", i,
		    feat_stream_len(fcb, i), gauden_stream_len(gau, i));
    }
    
    sen = senone_init ((char *) cmd_ln_access ("-mixwfn"),
		       (char *) cmd_ln_access ("-senmgaufn"),
		       *((float32 *) cmd_ln_access ("-mixwfloor")));

    /* Verify senone parameters against gauden parameters */
    if (senone_n_stream(sen) != feat_n_stream(fcb))
	E_FATAL("#Feature mismatch: feat(%d), senone(%d)\n",
		feat_n_stream(fcb), senone_n_stream(sen));
    if (senone_n_mgau(sen) != gauden_n_mgau(gau))
	E_FATAL("#Mixture gaussians mismatch: senone(%d), gauden(%d)\n",
		senone_n_mgau(sen), gauden_n_mgau(gau));

    /* Verify senone parameters against model definition parameters */
    if (kb->mdef->n_sen != senone_n_sen(sen))
	E_FATAL("#Senones mismatch: Model definition(%d) senone(%d)\n",
		kb->mdef->n_sen, senone_n_sen(sen));
    
    kb->tmat = tmat_init ((char *) cmd_ln_access ("-tmatfn"),
			 *((float32 *) cmd_ln_access ("-tmatfloor")));
    /* Verify transition matrices parameters against model definition parameters */
    if (kb->mdef->n_tmat != kb->tmat->n_tmat)
	E_FATAL("Model definition has %d tmat; but #tmat= %d\n",
		kb->mdef->n_tmat, kb->tmat->n_tmat);
    if (kb->mdef->n_emit_state != kb->tmat->n_state)
	E_FATAL("#Emitting states in model definition = %d, #states in tmat = %d\n",
		kb->mdef->n_emit_state, kb->tmat->n_state);

    mbeam = *((float64 *) cmd_ln_access ("-mgaubeam"));
    if ((kb->am = acoustic_init (fcb, gau, sen, mbeam, S3_MAX_FRAMES)) == NULL)
	E_FATAL("Acoustic models initialization failed\n");
    
    kb->insent = corpus_load_tailid ((char *) cmd_ln_access ("-insentfn"), NULL, NULL);
    if (! kb->insent)
	E_FATAL("Corpus-load(%s) failed\n", (char *) cmd_ln_access ("-insentfn"));

    if ((str = (char *) cmd_ln_access ("-outsentfn")) != NULL) {
	if ((kb->outsentfp = fopen(str, "w")) == NULL)
	    E_FATAL("fopen(%s,w) failed\n", str);
    } else
	kb->outsentfp = NULL;
}


main (int32 argc, char *argv[])
{
    kb_t kb;
    
#if (! WIN32)
    {
	char buf[1024];
	
	gethostname (buf, 1024);
	buf[1023] = '\0';
	E_INFO ("%s: Executing on: %s\n", argv[0], buf);
    }
#endif
    E_INFO("%s: COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);
    
    cmd_ln_parse (arglist, argc, argv);
    unlimit ();
    
    models_init (&kb);
    
    /* Create process-timing object */
    tm = (ptmr_t *) ckd_calloc (1, sizeof(ptmr_t));
    tot_frames = 0;
    
    ctl_process ((char *) cmd_ln_access ("-ctlfn"),
		 *((int32 *) cmd_ln_access ("-ctloffset")),
		 *((int32 *) cmd_ln_access ("-ctlcount")),
		 align_utt, &kb);
    
    if (kb.outsentfp)
	fclose (kb.outsentfp);
    
    printf ("\n");
    if (tot_frames > 0) {
	printf ("EXEC SUMMARY: %d frames, %.2fs CPU, %.2f xRT; %.2fs ELAPSED, %.2f xRT\n",
		tot_frames,
		tm->t_tot_cpu, tm->t_tot_cpu*100.0/tot_frames,
		tm->t_tot_elapsed, tm->t_tot_elapsed*100.0/tot_frames);
    } else
	printf ("EXEC SUMMARY: 0 frames\n");
    fflush (stdout);
    
    exit(0);
}
