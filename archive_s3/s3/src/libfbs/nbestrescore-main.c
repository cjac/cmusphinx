/*
 * nbestrescore-main.c -- Main driver routine for Alpha (forward) rescoring of
 * 	N-best lists.
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
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 *  
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started, based on align-main.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3.h>
#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>

#include "s3types.h"
#include "logs3.h"
#include "misc.h"
#include "gauden.h"
#include "senone.h"
#include "interp.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "search.h"
#include "nbestrescore.h"


static arg_def_t defn[] = {
    { "-logbase",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "1.0001",
      "Base in which all log values calculated" },
    { "-mdeffn", 
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Model definition input file: triphone -> senones/tmat tying" },
    { "-tmatfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Transition matrix input file" },
    { "-meanfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Mixture gaussian codebooks mean parameters input file" },
    { "-varfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Mixture gaussian codebooks variance parameters input file" },
    { "-senmgaufn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixwfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Senone mixture weights parameters input file" },
    { "-lambdafn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Interpolation weights (CD/CI senone) parameters input file" },
    { "-tpfloor",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.0001",
      "Triphone state transition probability floor applied to -tmatfn file" },
    { "-varfloor",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.0001",
      "Codebook variance floor applied to -varfn file" },
    { "-mwfloor",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.0000001",
      "Codebook mixture weight floor applied to -mixwfn file" },
    { "-agc",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "max",
      "AGC.  max: C0 -= max(C0) in current utt; none: no AGC" },
    { "-cmn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "current",
      "Cepstral mean norm.  current: C[1..n-1] -= mean(C[1..n-1]) in current utt; none: no CMN" },
    { "-feat",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "s2_4x",
      "Feature stream:\n\t\t\t\ts2_4x: Sphinx-II type 4 streams, 12cep, 24dcep, 3pow, 12ddcep\n\t\t\t\ts3_1x39: Single stream, 12cep+12dcep+3pow+12ddcep" },
    { "-dictfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdictfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Optional filler word (noise word) pronunciation dictionary input file" },
    { "-ctlfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Input control file listing utterances to be decoded" },
    { "-ctloffset",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "0",
      "No. of utterances at the beginning of -ctlfn file to be skipped" },
    { "-ctlcount",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "2147483647",	/* Max +ve int32 */
      "No. of utterances in -ctlfn file to be processed (after -ctloffset) or until EOF" },
    { "-cepdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      ".",
      "Directory for utterances in -ctlfn file (if relative paths specified)." },
    { "-cepext",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "mfc",
      "File extension appended to utterances listed in -ctlfn file" },
    { "-topn",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "4",
      "No. of top scoring densities computed in each mixture gaussian codebook" },
    { "-beam",
      CMD_LN_FLOAT64,
      CMD_LN_NO_VALIDATION,
      "1e-64",
      "Main pruning beam applied to triphones in forward search" },
    { "-nbestdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      ".",
      "Input Nbest files directory" },
    { "-alphadir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      ".",
      "Output directory for writing alpha scores" },
    
    { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
};


static int32 cmdline_parse (int argc, char *argv[])
{
    int32 i;
    
    E_INFO("Parsing command line:\n");
    for (i = 0; i < argc; i++) {
	if (argv[i][0] == '-')
	    printf ("\\\n\t");
	printf ("%s ", argv[i]);
    }
    printf ("\n\n");
    fflush (stdout);

    cmd_ln_parse (argc, argv);

    if (cmd_ln_validate() == FALSE) {
	E_FATAL("Unable to validate command line arguments\n");
    }
    
    E_INFO("Configuration in effect:\n");
    cmd_ln_print_configuration();
    printf ("\n");
    
    return 0;
}


static mdef_t *mdef;		/* Model definition */
static gauden_t *g;		/* Gaussian density codebooks */
static senone_t *sen;		/* Senones */
static interp_t *interp;	/* CD/CI interpolation */
static tmat_t *tmat;		/* HMM transition matrices */

static int32 n_feat, *featlen;	/* #features, vector-length/feature */
static int32 cepsize;		/* Input mfc vector size (C0..C<cepsize-1>) */

static int32 *senscale;		/* ALL senone scores scaled by senscale[i] in frame i */

static s3wid_t silwid, startwid, finishwid;

/* For profiling/timing */
static int32 tot_nfr;
static timing_t *tm_utt;
static timing_t *tm_gausen;
static timing_t *tm_alpha;


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    float32 varfloor, mixwfloor, tpfloor;
    int32 i, s;
    s3cipid_t ci;
    s3wid_t w;
    char *arg;
    dict_t *dict;
    
    /* HMM model definition */
    if (cmd_ln_access("-mdeffn") == NULL)
	E_FATAL("Missing -mdeffn argument\n");
    mdef = mdef_init ((char *) cmd_ln_access("-mdeffn"));

    /* Dictionary */
    if (cmd_ln_access("-dictfn") == NULL)
	E_FATAL("Missing -dictfn argument\n");
    dict = dict_init ((char *) cmd_ln_access("-dictfn"),
		      (char *) cmd_ln_access("-fdictfn"));

    /* HACK!! Make sure SILENCE_WORD, START_WORD and FINISH_WORD are in dictionary */
    silwid = dict_wordid (SILENCE_WORD);
    startwid = dict_wordid (START_WORD);
    finishwid = dict_wordid (FINISH_WORD);
    if (NOT_WID(silwid) || NOT_WID(startwid) || NOT_WID(finishwid)) {
	E_FATAL("%s, %s, or %s missing from dictionary\n",
		SILENCE_WORD, START_WORD, FINISH_WORD);
    }
    if ((dict->filler_start > dict->filler_end) || (! dict_filler_word (silwid)))
	E_FATAL("%s must occur (only) in filler dictionary\n", SILENCE_WORD);
    /* No check that alternative pronunciations for filler words are in filler range!! */

    /* Codebooks */
    if ((cmd_ln_access("-meanfn") == NULL) || (cmd_ln_access("-varfn") == NULL))
	E_FATAL("Missing -meanfn or -varfn argument\n");
    varfloor = *((float32 *) cmd_ln_access("-varfloor"));
    g = gauden_init ((char *) cmd_ln_access("-meanfn"),
		     (char *) cmd_ln_access("-varfn"),
		     varfloor);

    /* Verify codebook feature dimensions against libfeat */
    n_feat = feat_featsize (&featlen);
    if (n_feat != g->n_feat)
	E_FATAL("#feature mismatch: s2= %d, mean/var= %d\n", n_feat, g->n_feat);
    for (i = 0; i < n_feat; i++)
	if (featlen[i] != g->featlen[i])
	    E_FATAL("featlen[%d] mismatch: s2= %d, mean/var= %d\n", i,
		    featlen[i], g->featlen[i]);

    /* Senone mixture weights */
    if (cmd_ln_access("-mixwfn") == NULL)
	E_FATAL("Missing -mixwfn argument\n");
    mixwfloor = *((float32 *) cmd_ln_access("-mwfloor"));
    sen = senone_init ((char *) cmd_ln_access("-mixwfn"),
		       (char *) cmd_ln_access("-senmgaufn"),
		       mixwfloor);
    
    /* Verify senone parameters against gauden parameters */
    if (sen->n_feat != g->n_feat)
	E_FATAL("#Feature mismatch: gauden= %d, senone= %d\n", g->n_feat, sen->n_feat);
    if (sen->n_cw != g->n_density)
	E_FATAL("#Densities mismatch: gauden= %d, senone= %d\n", g->n_density, sen->n_cw);
    if (sen->n_gauden > g->n_mgau)
	E_FATAL("Senones need more codebooks (%d) than present (%d)\n",
		sen->n_gauden, g->n_mgau);
    if (sen->n_gauden < g->n_mgau)
	E_ERROR("Senones use fewer codebooks (%d) than present (%d)\n",
		sen->n_gauden, g->n_mgau);

    /* Verify senone parameters against model definition parameters */
    if (mdef->n_sen != sen->n_sen)
	E_FATAL("Model definition has %d senones; but #senone= %d\n",
		mdef->n_sen, sen->n_sen);

    /* CD/CI senone interpolation weights file, if present */
    if ((arg = (char *) cmd_ln_access ("-lambdafn")) != NULL) {
	interp = interp_init (arg);

	/* Verify interpolation weights size with senones */
	if (interp->n_sen != sen->n_sen)
	    E_FATAL("Interpolation file has %d weights; but #senone= %d\n",
		    interp->n_sen, sen->n_sen);
    } else
	interp = NULL;

    /* Transition matrices */
    if (cmd_ln_access("-tmatfn") == NULL)
	E_FATAL("Missing -tmatfn argument\n");
    tpfloor = *((float32 *) cmd_ln_access("-tpfloor"));
    tmat = tmat_init ((char *) cmd_ln_access("-tmatfn"), tpfloor);

    /* Verify transition matrices parameters against model definition parameters */
    if (mdef->n_tmat != tmat->n_tmat)
	E_FATAL("Model definition has %d tmat; but #tmat= %d\n",
		mdef->n_tmat, tmat->n_tmat);
    if (mdef->n_emit_state != tmat->n_state-1)
	E_FATAL("#Emitting states in model definition = %d, #states in tmat = %d\n",
		mdef->n_emit_state, tmat->n_state);

    arg = (char *) cmd_ln_access ("-agc");
    if ((strcmp (arg, "max") != 0) && (strcmp (arg, "none") != 0))
	E_FATAL("Unknown -agc argument: %s\n", arg);
    arg = (char *) cmd_ln_access ("-cmn");
    if ((strcmp (arg, "current") != 0) && (strcmp (arg, "none") != 0))
	E_FATAL("Unknown -cmn argument: %s\n", arg);
}


/*
 * Rescore Nbest hypotheses using alpha (forward) algorithm.
 */
static void nbestrescore_utt (hyp_t **hyplist, /* In: Nbest hypotheses to rescore */
			      int32 nhyp,	/* In: #hyp in hyplist */
			      float32 **mfc,	/* In: MFC cepstra for input utterance */
			      int32 nfr,	/* In: #frames of input */
			      char *uttid)	/* In: Utterance id */
{
    static float32 **feat = NULL;
    static int32 w;
    static int32 topn;
    static gauden_dist_t ***dist;
    static int32 *senscr;
    static s3senid_t *sen_active;
    static int8 *mgau_active;
    
    int32 i, s, sid, gid, n_sen_active, best;
    char *arg;

    if (! feat) {
	/* One-time allocation of necessary intermediate variables */

	/* Allocate space for a feature vector */
	feat = (float32 **) ckd_calloc (n_feat, sizeof(float32 *));
	for (i = 0; i < n_feat; i++)
	    feat[i] = (float32 *) ckd_calloc (featlen[i], sizeof(float32));
	
	/* Allocate space for top-N codeword density values in a codebook */
	w = feat_window_size ();	/* #MFC vectors needed on either side of current
					   frame to compute one feature vector */
	topn = *((int32 *) cmd_ln_access("-topn"));
	if (topn > g->n_density) {
	    E_WARN("-topn argument (%d) > #density codewords (%d); set to latter\n",
		   topn, g->n_density);
	    topn = g->n_density;
	}
	dist = (gauden_dist_t ***) ckd_calloc_3d (g->n_mgau, n_feat, topn,
						  sizeof(gauden_dist_t));
	
	/* Space for one frame of senone scores, and per frame active flags */
	senscr = (int32 *) ckd_calloc (sen->n_sen, sizeof(int32));
	sen_active = (s3senid_t *) ckd_calloc (sen->n_sen, sizeof(s3senid_t));
	mgau_active = (int8 *) ckd_calloc (g->n_mgau, sizeof(int8));

	/* Initialize space for senone scale factor per frame */
	senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    }
    
    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return;
    }
    
    timing_reset (tm_utt);
    timing_start (tm_utt);

    /* AGC and CMN */
    arg = (char *) cmd_ln_access ("-cmn");
    if (strcmp (arg, "current") == 0)
	norm_mean (mfc, nfr, cepsize);
    arg = (char *) cmd_ln_access ("-agc");
    if (strcmp (arg, "max") == 0)
	agc_max (mfc, nfr);
    
    alpha_start_utt (hyplist, nhyp, uttid);
    
    /*
     * A feature vector for frame f depends on input MFC vectors [f-w..f+w].  Hence
     * the feature vector corresponding to the first w and last w input frames is
     * undefined.  We define them by simply replicating the first and last true
     * feature vectors (presumably silence regions).
     */
    for (i = 0; i < nfr; i++) {
	/* Compute feature vector for current frame from input speech cepstra */
	if (i < w)
	    feat_cep2feat (mfc+w, feat);
	else if (i >= nfr-w)
	    feat_cep2feat (mfc+(nfr-w-1), feat);
	else
	    feat_cep2feat (mfc+i, feat);

	/*
	 * Evaluate gaussian density codebooks and senone scores for input codeword.
	 * Evaluate only active codebooks and senones.
	 */
	/* Obtain active senone flags */
	alpha_sen_active (sen_active, sen->n_sen);
	/* Flag all CI senones to active if interpolating */
	if (interp) {
	    for (s = 0; s < mdef->n_ci_sen; s++)
		sen_active[s] = 1;
	}
	/* Turn active flags into list (for faster access) */
	n_sen_active = 0;
	for (s = 0; s < mdef->n_sen; s++) {
	    if (sen_active[s])
		sen_active[n_sen_active++] = s;
	}
	
	/* Flag all active mixture-gaussian codebooks */
	for (gid = 0; gid < g->n_mgau; gid++)
	    mgau_active[gid] = 0;
	for (s = 0; s < n_sen_active; s++) {
	    sid = sen_active[s];
	    mgau_active[sen->mgau[sid]] = 1;
	}
	
	/* Compute topn gaussian density values (for active codebooks) */
	for (gid = 0; gid < g->n_mgau; gid++)
	    if (mgau_active[gid])
		gauden_dist (g, gid, topn, feat, dist[gid]);
	
	/* Evaluate active senones */
	best = (int32) 0x80000000;
	for (s = 0; s < n_sen_active; s++) {
	    sid = sen_active[s];
	    senscr[sid] = senone_eval (sen, sid, dist[sen->mgau[sid]], topn);
	    if (best < senscr[sid])
		best = senscr[sid];
	}
	if (interp) {
	    for (s = 0; s < n_sen_active; s++) {
		if ((sid = sen_active[s]) >= mdef->n_ci_sen)
		    interp_cd_ci (interp, senscr, sid, mdef->cd2cisen[sid]);
	    }
	}
	
	/* Normalize senone scores (interpolation above can only lower best score) */
	for (s = 0; s < n_sen_active; s++) {
	    sid = sen_active[s];
	    senscr[sid] -= best;
	}
	senscale[i] = best;
	
	/* Step rescoring one frame forward */
	alpha_frame (uttid, senscr);
    }

    /* Wind up this utterance */
    arg = (char *) cmd_ln_access ("-alphadir");
    alpha_end_utt (arg, uttid);

    timing_stop (tm_utt);

    printf ("%s: TMR:[frm %5d]", uttid, nfr);
    if (nfr > 0) {
	printf ("[el %6.2fx]", tm_utt->t_elapsed * 100.0 / nfr);
	printf ("[cpu %6.2fx]", tm_utt->t_cpu * 100.0 / nfr);
    }
    printf ("\n");
    fflush (stdout);
}


/* Process utterances in the control file (-ctlfn argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp;
    char *ctlfile, *cepdir, *cepext, *nbestdir;
    char cepfile[1024], ctlspec[1024], uttid[1024];
    int32 ctloffset, ctlcount, sf, ef, nfr, nhyp;
    float32 **mfc;
    hyp_t **hyplist;
    
    ctlfile = (char *) cmd_ln_access("-ctlfn");
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));

    ctlfp = ctlfile_open (ctlfile);

    /* Skip initial offset, if any, from control file */
    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n", ctloffset);
    for (; ctloffset > 0; --ctloffset) {
	if (ctlfile_next (ctlfp, ctlspec, &sf, &ef, uttid) < 0)
	    E_FATAL("Premature EOF(%s)\n", ctlfile);
    }

    cepdir = (char *) cmd_ln_access("-cepdir");
    cepext = (char *) cmd_ln_access("-cepext");

    nbestdir = (char *) cmd_ln_access("-nbestdir");

    /* Process the specified number of utterance or until end of control file */
    while ((ctlcount > 0) && (ctlfile_next (ctlfp, ctlspec, &sf, &ef, uttid) == 0)) {
	printf ("\n");
	E_INFO("Utterance: %s(%s)\n", ctlspec, uttid);

	/* Read and process mfc file */
	if (ctlspec[0] != '/')
	    sprintf (cepfile, "%s/%s.%s", cepdir, ctlspec, cepext);
	else
	    sprintf (cepfile, "%s.%s", ctlspec, cepext);
	if ((nfr = s2mfc_read (cepfile, sf, ef, &mfc)) <= 0)
	    E_ERROR("Utt %s: MFC file read (%s) failed\n", uttid, cepfile);
	
	/* Read Nbest file; hyplist only contains word strings, not ids */
	if ((nhyp = nbestfile_load (nbestdir, uttid, &hyplist)) <= 0)
	    E_ERROR("Nbest load failed; cannot rescore %s\n", uttid);

	if ((nfr > 0) && (nhyp > 0)) {
	    E_INFO("%s: Rescoring %d hyps; %d frames\n", uttid, nhyp, nfr);
	    nbestrescore_utt (hyplist, nhyp, mfc, nfr, uttid);
	    tot_nfr += nfr;

	    nbestlist_free (hyplist, nhyp);
	}
	
	--ctlcount;
    }
    printf ("\n");

    if (ctlfile_next(ctlfp, ctlspec, &sf, &ef, uttid) == 0)
	E_INFO("Skipping rest of control file from:\n\t%s(%s)\n", ctlspec, uttid);
    ctlfile_close (ctlfp);
}


main (int32 argc, char *argv[])
{
    char *str;
    float32 logbase;
    
    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);
    
    /* Digest command line argument definitions */
    cmd_ln_define (defn);
    if ((argc == 2) && (strcmp (argv[1], "help") == 0)) {
	cmd_ln_print_definitions();
	exit(0);
    }

    /* Look for default or specified arguments file */
    str = NULL;
    if ((argc == 2) && (argv[1][0] != '-'))
	str = argv[1];
    else if (argc == 1) {
	str = "s3nbestrescore.arg";
	E_INFO("Looking for default argument file: %s\n", str);
    }
    if (str) {
	/* Build command line argument list from file */
	if ((argc = argfile_load (str, argv[0], &argv)) < 0) {
	    fprintf (stderr, "Usage:\n");
	    fprintf (stderr, "\t%s argument-list, or\n", argv[0]);
	    fprintf (stderr, "\t%s [argument-file] (default file: s3nbestrescore.arg)\n\n",
		     argv[0]);
	    cmd_ln_print_definitions();
	    exit(1);
	}
    }
    
    cmdline_parse (argc, argv);
    
    /*
     * Initialize log(S3-base).  All scores (probs...) computed in log domain to avoid
     * underflow.  At the same time, log base ~= 1.0001 (1+epsilon) to allow log values
     * to be maintained in int32 variables without significant loss of precision.
     */
    logbase = *((float32 *) cmd_ln_access("-logbase"));
    logs3_init ((float64) logbase);

    /* Initialize feature stream type */
    feat_init ((char *) cmd_ln_access ("-feat"));
    cepsize = feat_cepsize ();
    
    /* Read in input databases */
    models_init ();
    
    /* Initialize nbest rescoring module */
    alpha_init ();
    
    /* Initialize performance timing variables */
    tm_utt = timing_new ();
    tm_gausen = timing_new ();
    tm_alpha = timing_new ();
    tot_nfr = 0;
    
    /* Process control file */
    printf ("\n");
    process_ctlfile ();
    printf ("\n");

    /* Print final summary stats */
    printf("TOTAL FRAMES:       %8d\n", tot_nfr);
    if (tot_nfr > 0) {
	printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_cpu, tm_utt->t_tot_cpu/(tot_nfr*0.01));
	printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_elapsed, tm_utt->t_tot_elapsed/(tot_nfr*0.01));
    }
#if (! WIN32)
    system ("ps aguxwww | grep s3nbestrescore");
#endif
    
    exit(0);
}
