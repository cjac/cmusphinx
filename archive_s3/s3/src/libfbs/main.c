/*
 * main.c -- Main S3 decoder driver.
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
 * $Log$
 * Revision 1.1  2000/04/24  09:39:41  lenzo
 * s3 import.
 * 
 * 
 * 16-Jun-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to handle the new libfeat interface.
 * 
 * 31-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added compound words handling option.  Compound words are broken up
 * 		into component words for computing LM probabilities.
 * 		Added utterance boundary LM context option.  An utterance need not
 * 		begin with <s> and end with </s>.  Instead, one can use the context
 * 		specified in a given file.
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 * 
 * 02-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Restricted MLLR transformation to CD mixture densities only.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the meaning of -matchsegfn and, correspondingly, log_hypseg().
 * 
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -min_endfr and -dagfudge arguments.
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added BSTXCT: reporting since that became available from dag_search.
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ,NODES suffix to -outlatdir argument for dumping only words to
 * 		lattice output files.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added fillpen_init() and removed explicit addition of SILENCE_WORD,
 * 		START_WORD and FINISH_WORD to the dictionary.
 * 
 * 04-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added unlimit() call to remove malloc restrictions.
 *  
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added separate language weight (-bestpathlw) for bestpath DAG search.
 * 		Added -mllrctlfn flag and handling.
 * 
 * 21-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -bptblsize argument.
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added optional start/end frame specification in control file, for
 * 		processing selected segments (utterances) from a large cepfile.
 * 		Control spec: cepfile [startframe endframe [uttid]].
 * 
 * 13-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: added senscale to bestscr before writing best score file.
 * 		(Otherwise, the scaled scores are meaningless.)
 * 		Added ,EXACT suffix option to -matchfn argument, and correspondingly
 * 		added "exact" argument to log_hypstr().  (But running bestpath search
 * 		will still cause <sil> and filler words to be removed from matchfile.)
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed structure of gauden/senone computation:
 * 		    from: foreach (gauden) {eval gauden}; foreach (senone) {senone}
 * 		    to:   foreach (gauden) {eval gauden;  foreach (senone in gauden) {...}}
 * 		reducing memory space for results of gauden, specially in block mode.
 * 		Normalized senone scores (subtracting the best) rather than density scores.
 * 		Changed active senone list to flags.
 * 
 * 09-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Block-mode gauden computation for improving cache performance: changed
 *		    from: foreach(frame) {foreach(gauden)...}
 * 		    to:   foreach(gauden) {foreach(frame)...}
 * 		within a block of frames.  Must evaluate all gauden, not just active ones.
 * 		But even so the resulting caching performance is better.
 * 
 * 29-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed argument -inlatext to -latext.
 * 		Added check to ensure input and output lattice directories are different.
 * 		Added reporting of hostname.
 * 
 * 23-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed profiling to use timing_ functions, available on all platforms.
 * 		Added write_bestscore() for writing best statescore in each frame,
 * 		for helping determine desirable beamwidth.
 * 
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added DAG search option and consolidated logging and reporting.
 * 		Added backtrace option.
 * 
 * 22-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added absolute (unnormalized) acoustic scores in log file.
 * 		Added uttid in log file with each word segmentation.
 * 		Compute only active codebooks and senones if multiple codebooks present.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added specification of input word lattices to limit search (-inlatdir
 * 		argument...).  Added computation of active senone and gauden codebook
 * 		lists when such a lattice is provided, to minimize computation.
 * 		Added -cmn and -agc flags.
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added call to feat_init(); added cepsize variable and initialization;
 * 		Changed argument to norm_mean from featlen[0]+1 to cepsize.
 * 
 * 13-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed call to senone_eval to senone_eval_all optimized for the
 * 		semi-continuous case.
 * 		Completed handling multiple mixture-gaussian codebooks.
 * 
 * 01-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>

#include "s3types.h"
#include "logs3.h"
#include "gauden.h"
#include "senone.h"
#include "interp.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "mllr.h"
#include "cmn.h"
#include "agc.h"


static mdef_t *mdef;		/* Model definition */
static gauden_t *g;		/* Gaussian density codebooks */
static senone_t *sen;		/* Senones */
static interp_t *interp;	/* CD/CI interpolation */
static tmat_t *tmat;		/* HMM transition matrices */

static feat_t *fcb;		/* Feature type descriptor (Feature Control Block) */
static float32 ***feat = NULL;	/* Speech feature data */
static float32 **mfc = NULL;	/* MFC data for entire utterance */

static s3wid_t startwid, finishwid, silwid;

static int32 *senscale;		/* ALL senone scores scaled by senscale[i] in frame i */
static int32 *bestscr;		/* Best statescore in each frame */

/* For profiling/timing */
static timing_t *tm_utt;	/* Entire utterance */
static timing_t *tm_fwdvit;	/* Entire forward Viterbi */
static timing_t *tm_bstpth;	/* Bestpath search */
static timing_t *tm_gausen;	/* Gaussian density computation within fwdvit */
static timing_t *tm_fwdsrch;	/* Forward search computation within fwdvit */
static int32 ctr_nfrm;		/* #Frames in utterance */
static int32 ctr_nsen;		/* #Senones evaluated in utterance */

static int32 tot_nfr;

static char *inlatdir;
static char *outlatdir;
static int32 outlat_onlynodes;
static FILE *matchfp, *matchsegfp;
static int32 matchexact;


/*
 * Command line arguments.
 */
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
    { "-feat",	/* Captures the computation for converting input to feature vector */
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "s2_4x",
      "Feature stream: s2_4x / s3_1x39 / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
    { "-dictfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdictfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Silence and filler (noise) word pronunciation dictionary input file" },
    { "-compwd",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "0",
      "Whether compound words should be broken up internally into component words" },
    { "-lmfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Language model input file (precompiled .DMP file)" },
    { "-langwt",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "9.5",
      "Language weight: empirical exponent applied to LM probabilty" },
    { "-ugwt",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.7",
      "LM unigram weight: unigram probs interpolated with uniform distribution with this weight" },
    { "-lmcontextfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Language model context file (parallel to -ctlfn argument)" },
    { "-bestpath",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "0",
      "Whether to run bestpath DAG search after forward Viterbi pass" },
    { "-min_endfr",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "3",
      "Nodes ignored during search if they persist for fewer than so many end frames" },
    { "-dagfudge",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "2",
      "Adjacency fudge (#frames) between nodes in DAG (0..2)" },
    { "-bestpathlw",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Language weight for bestpath DAG search (default: same as -langwt)" },
    { "-inspen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.65",
      "Word insertion penalty" },
    { "-silpen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.1",
      "Language model 'probability' of silence word" },
    { "-noisepen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.05",
      "Language model 'probability' of each non-silence filler word" },
    { "-fillpenfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Filler word probabilities input file (used in place of -silpen and -noisepen)" },
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
      CMD_LN_NO_DEFAULT,
      "No. of utterances in -ctlfn file to be processed (after -ctloffset).  Default: Until EOF" },
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
    { "-mllrctlfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Input control file listing MLLR input data; parallel to -ctlfn argument file" },
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
    { "-nwbeam",
      CMD_LN_FLOAT64,
      CMD_LN_NO_VALIDATION,
      "1e-27",
      "Pruning beam applied in forward search upon word exit" },
    { "-phonepen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "1.0",
      "Penalty applied for each phone transition" },
    { "-tracewhmm",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Word whose active HMMs are to be traced (for debugging/diagnosis/analysis)" },
    { "-hmmdumpsf",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Starting frame for dumping all active HMMs (for debugging/diagnosis/analysis)" },
    { "-worddumpsf",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Starting frame for dumping all active words (for debugging/diagnosis/analysis)" },
    { "-inlatdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-inlatwin",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "50",
      "Input word-lattice words starting within +/- <this argument> of current frame considered during search" },
    { "-outlatdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Directory for writing word lattices (one file/utterance); optional ,NODES suffix to write only the nodes" },
    { "-latext",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "lat.gz",
      "Word-lattice filename extension (.gz or .Z extension for compression)" },
    { "-bestscoredir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Directory for writing best score/frame (used to set beamwidth; one file/utterance)" },
    { "-matchfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Recognition result output file (pre-1995 NIST format) (optional ,EXACT suffix)" },
    { "-matchsegfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Exact recognition result file with word segmentations and scores" },
    { "-logfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Log file (default stdout/stderr)" },
    { "-backtrace",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "1",
      "Whether detailed backtrace information (word segmentation/scores) shown in log" },
    { "-bptblsize",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "32767",
      "Number of BPtable entries to allocate initially (grown as necessary)" },
    { "-bptbldump",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "0",
      "Whether BPTable should be dumped to log output (for debugging)" },
    
    { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
};


/* Hacks to avoid hanging problem under Linux */
static FILE orig_stdout, orig_stderr;
static FILE *logfp;

static int32 cmdline_parse (int argc, char *argv[])
{
    int32 i;
    char *logfile;
    
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

    logfp = NULL;
    if ((logfile = (char *)cmd_ln_access("-logfn")) != NULL) {
	if ((logfp = fopen(logfile, "w")) == NULL) {
	    E_ERROR("fopen(%s,w) failed; logging to stdout/stderr\n");
	} else {
	    orig_stdout = *stdout;	/* Hack!! To avoid hanging problem under Linux */
	    orig_stderr = *stderr;	/* Hack!! To avoid hanging problem under Linux */
	    *stdout = *logfp;
	    *stderr = *logfp;
	    
	    E_INFO("Command line:\n");
	    for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-')
		    printf ("\\\n\t");
		printf ("%s ", argv[i]);
	    }
	    printf ("\n\n");
	    fflush (stdout);
	}
    }
    
    E_INFO("Configuration in effect:\n");
    cmd_ln_print_configuration();
    printf ("\n");
    
    return 0;
}


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    float32 varfloor, mixwfloor, tpfloor;
    int32 i;
    dict_t *dict;
    char *arg;
    
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdeffn"));

    /* Dictionary */
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
    varfloor = *((float32 *) cmd_ln_access("-varfloor"));
    g = gauden_init ((char *) cmd_ln_access("-meanfn"),
		     (char *) cmd_ln_access("-varfn"),
		     varfloor);

    /* Verify codebook feature dimensions against libfeat */
    if (feat_n_stream(fcb) != g->n_feat) {
	E_FATAL("#feature mismatch: feat= %d, mean/var= %d\n",
		feat_n_stream(fcb), g->n_feat);
    }
    for (i = 0; i < feat_n_stream(fcb); i++) {
	if (feat_stream_len(fcb,i) != g->featlen[i]) {
	    E_FATAL("featlen[%d] mismatch: feat= %d, mean/var= %d\n", i,
		    feat_stream_len(fcb, i), g->featlen[i]);
	}
    }
    
    /* Senone mixture weights */
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
    tpfloor = *((float32 *) cmd_ln_access("-tpfloor"));
    tmat = tmat_init ((char *) cmd_ln_access("-tmatfn"), tpfloor);

    /* Verify transition matrices parameters against model definition parameters */
    if (mdef->n_tmat != tmat->n_tmat)
	E_FATAL("Model definition has %d tmat; but #tmat= %d\n",
		mdef->n_tmat, tmat->n_tmat);
    if (mdef->n_emit_state != tmat->n_state-1)
	E_FATAL("#Emitting states in model definition = %d, #states in tmat = %d\n",
		mdef->n_emit_state, tmat->n_state);

    /* LM */
    lm_read ((char *) cmd_ln_access("-lmfn"), "");

    /* Filler penalties */
    fillpen_init ((char *) cmd_ln_access("-fillpenfn"),
		  dict->filler_start, dict->filler_end);

    arg = (char *) cmd_ln_access ("-agc");
    if ((strcmp (arg, "max") != 0) && (strcmp (arg, "none") != 0))
	E_FATAL("Unknown -agc argument: %s\n", arg);
    arg = (char *) cmd_ln_access ("-cmn");
    if ((strcmp (arg, "current") != 0) && (strcmp (arg, "none") != 0))
	E_FATAL("Unknown -cmn argument: %s\n", arg);
}


/*
 * Write exact hypothesis.  Format
 *   <id> S <scl> T <scr> A <ascr> L <lscr> {<sf> <wascr> <wlscr> <word>}... <ef>
 * where:
 *   scl = acoustic score scaling for entire utterance
 *   scr = ascr + (lscr*lw+N*wip), where N = #words excluding <s>
 *   ascr = scaled acoustic score for entire utterance
 *   lscr = LM score (without lw or wip) for entire utterance
 *   sf = start frame for word
 *   wascr = scaled acoustic score for word
 *   wlscr = LM score (without lw or wip) for word
 *   ef = end frame for utterance.
 */
static void log_hypseg (char *uttid,
			FILE *fp,	/* Out: output file */
			hyp_t *hypptr,	/* In: Hypothesis */
			int32 nfrm,	/* In: #frames in utterance */
			int32 scl,	/* In: Acoustic scaling for entire utt */
			float64 lwf)	/* In: LM score scale-factor (in dagsearch) */
{
    hyp_t *h;
    int32 ascr, lscr, tscr;
    
    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
	ascr += h->ascr;
	if (dict_basewid(h->wid) != startwid) {
	    lscr += lm_rawscore (h->lscr, lwf);
	} else {
	    assert (h->lscr == 0);
	}
	tscr += h->ascr + h->lscr;
    }

    fprintf (fp, "%s S %d T %d A %d L %d", uttid, scl, tscr, ascr, lscr);
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, " (null)\n");
    else {
	for (h = hypptr; h; h = h->next) {
	    lscr = (dict_basewid(h->wid) != startwid) ? lm_rawscore (h->lscr, lwf) : 0;
	    fprintf (fp, " %d %d %d %s", h->sf, h->ascr, lscr, dict_wordstr (h->wid));
	}
	fprintf (fp, " %d\n", nfrm);
    }
    
    fflush (fp);
}


/* Write hypothesis in old (pre-Nov95) NIST format */
static void log_hypstr (FILE *fp, hyp_t *hypptr, char *uttid, int32 exact, int32 scr)
{
    hyp_t *h;
    s3wid_t w;
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, "(null)");
    
    for (h = hypptr; h; h = h->next) {
	w = h->wid;
	if (! exact) {
	    w = dict_basewid (w);
	    if ((w != startwid) && (w != finishwid) && (! dict_filler_word (w)))
		fprintf (fp, "%s ", dict_wordstr(w));
	} else
	    fprintf (fp, "%s ", dict_wordstr(w));
    }
    if (scr != 0)
	fprintf (fp, " (%s %d)\n", uttid, scr);
    else
	fprintf (fp, " (%s)\n", uttid);
    fflush (fp);
}


/* Log hypothesis in detail with word segmentations, acoustic and LM scores  */
static void log_hyp_detailed (FILE *fp, hyp_t *hypptr, char *uttid, char *LBL, char *lbl)
{
    hyp_t *h;
    int32 f, scale, ascr, lscr;

    ascr = 0;
    lscr = 0;
    
    fprintf (fp, "%s:%s> %20s %5s %5s %11s %10s %8s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr", "LScr", "Conf");
    
    for (h = hypptr; h; h = h->next) {
	scale = 0;
	for (f = h->sf; f <= h->ef; f++)
	    scale += senscale[f];
	
	fprintf (fp, "%s:%s> %20s %5d %5d %11d %10d %8.4f\n", lbl, uttid,
		 h->word, h->sf, h->ef, h->ascr + scale, h->lscr, h->conf);

	ascr += h->ascr + scale;
	lscr += h->lscr;
    }

    fprintf (fp, "%s:%s> %20s %5s %5s %11d %10d\n", LBL, uttid,
	     "TOTAL", "", "", ascr, lscr);
}


static void write_bestscore (char *dir, char *uttid, int32 *score, int32 nfr)
{
    char filename[1024];
    FILE *fp;
    int32 k;
    
    sprintf (filename, "%s/%s.bscr", dir, uttid);
    E_INFO("Writing bestscore file: %s\n", filename);
    if ((fp = fopen (filename, "wb")) == NULL) {
	E_ERROR("fopen(%s,wb) failed\n", filename);
	return;
    }
    
    /* Write version no. */
    if (fwrite ("0.1\n", sizeof(char), 4, fp) != 4)
	goto write_error;

    /* Write binary comment string */
    if (fwrite ("*end_comment*\n", sizeof(char), 14, fp) != 14)
	goto write_error;

    /* Write byte-ordering magic number */
    k = BYTE_ORDER_MAGIC;
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write #frames */
    k = nfr;
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write bestscore/frame */
    if (fwrite (score, sizeof(int32), nfr, fp) != nfr)
	goto write_error;

    fclose (fp);
    return;
    
write_error:
    E_ERROR("fwrite(%s) failed\n", filename);
    fclose (fp);
}


#define GAUDEN_EVAL_WINDOW	8

/* Lists of senones sharing each mixture Gaussian codebook */
typedef struct mgau2sen_s {
    s3senid_t sen;		/* Senone shared by this mixture Gaussian */
    struct mgau2sen_s *next;	/* Next entry in list for this mixture Gaussian */
} mgau2sen_t;

/*
 * Forward Viterbi decode.
 * Return value: recognition hypothesis with detailed segmentation and score info.
 */
static hyp_t *fwdvit (int32 nfr,	/* In: #frames of input */
		      char *uttid)	/* In: Utterance id, for logging and other use */
{
    static int32 w;
    static int32 topn;
    static int32 **senscr = NULL;	/* Senone scores for window of frames */
    static gauden_dist_t **dist;	/* Density values for one mgau in one frame */
    static int8 *sen_active;		/* [s] TRUE iff s active in current frame */
    static int8 *mgau_active;		/* [m] TRUE iff m active in current frame */
    static mgau2sen_t **mgau2sen;	/* Senones sharing mixture Gaussian codebooks */

    int32 i, j, k, s, gid, n_sen_active, best;
    hyp_t *hyp;
    char *arg;
    mgau2sen_t *m2s;
    float32 **fv;
    
    if (! senscr) {
	/* One-time allocation of necessary intermediate variables */
	
	/* Allocate space for top-N codeword density values in a codebook */
	w = feat_window_size (fcb);	/* #MFC vectors needed on either side of current
					   frame to compute one feature vector */
	topn = *((int32 *) cmd_ln_access("-topn"));
	if (topn > g->n_density) {
	    E_WARN("-topn argument (%d) > #density codewords (%d); set to latter\n",
		   topn, g->n_density);
	    topn = g->n_density;
	}
	dist = (gauden_dist_t **) ckd_calloc_2d (g->n_feat, topn, sizeof(gauden_dist_t));

	/*
	 * If search limited to given word lattice, or if many codebooks, only active
	 * senones computed in each frame.   Allocate space for list of active senones,
	 * and active codebook flags.
	 */
	if (inlatdir) {
	    E_INFO("Computing only active codebooks and senones each frame\n");
	    sen_active = (int8 *) ckd_calloc (sen->n_sen, sizeof(int8));
	    mgau_active = (int8 *) ckd_calloc (g->n_mgau, sizeof(int8));
	
	    /* Space for senone scores (one frame) */
	    senscr = (int32 **) ckd_calloc_2d (1, sen->n_sen, sizeof(int32));
	} else {
	    E_INFO("Computing all codebooks and senones each frame\n");
	    sen_active = NULL;
	    mgau_active = NULL;
	
	    /* Space for senone scores (window of frames) */
	    senscr = (int32 **) ckd_calloc_2d (GAUDEN_EVAL_WINDOW, sen->n_sen,
					       sizeof(int32));
	}
	
	/* Initialize mapping from mixture Gaussian to senones */
	mgau2sen = (mgau2sen_t **) ckd_calloc (g->n_mgau, sizeof(mgau2sen_t *));
	for (s = 0; s < sen->n_sen; s++) {
	    m2s = (mgau2sen_t *) listelem_alloc (sizeof(mgau2sen_t));
	    m2s->sen = s;
	    m2s->next = mgau2sen[sen->mgau[s]];
	    mgau2sen[sen->mgau[s]] = m2s;
	}
    }
    
    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return NULL;
    }
    
    timing_reset (tm_gausen);
    timing_reset (tm_fwdsrch);

    /* AGC and CMN */
    if (mfc) {
	arg = (char *) cmd_ln_access ("-cmn");
	if (strcmp (arg, "current") == 0)
	    norm_mean (mfc, nfr, feat_cepsize(fcb));
	arg = (char *) cmd_ln_access ("-agc");
	if (strcmp (arg, "max") == 0)
	    agc_max (mfc, nfr);
    }
    
    fwd_start_utt (uttid);

    if (sen_active) {
	for (i = 0; i < nfr; i++) {
	    timing_start (tm_gausen);

	    /*
	     * Compute feature vector for current frame from input speech cepstra.
	     * A feature vector for frame f depends on input MFC vectors [f-w..f+w].
	     * Hence the feature vector corresponding to the first w and last w input
	     * frames is undefined.  We define them by simply replicating the first and
	     * last true feature vectors (presumably silence regions).
	     */
	    if (mfc) {
		if (i < w)
		    fcb->compute_feat (fcb, mfc+w, feat[0]);
		else if (i >= nfr-w)
		    fcb->compute_feat (fcb, mfc+(nfr-w-1), feat[0]);
		else
		    fcb->compute_feat (fcb, mfc+i, feat[0]);
		fv = feat[0];
	    } else
		fv = feat[i];
	    
	    /* Obtain list of active senones */
	    fwd_sen_active (sen_active, sen->n_sen);
	    
	    /* Flag all active mixture-gaussian codebooks */
	    for (gid = 0; gid < g->n_mgau; gid++)
		mgau_active[gid] = 0;
	    n_sen_active = 0;
	    for (s = 0; s < sen->n_sen; s++) {
		if (sen_active[s]) {
		    mgau_active[sen->mgau[s]] = 1;
		    n_sen_active++;
		}
	    }
	    /* Add in CI senones and codebooks if interpolating with CI */
	    if (interp) {
		for (s = 0; s < mdef->n_ci_sen; s++) {
		    mgau_active[s] = 1;
		    if (! sen_active[s]) {
			sen_active[s] = 1;
			n_sen_active++;
		    }
		}
	    }
	    counter_increment (ctr_nsen, n_sen_active);
	    
	    /* Compute topn gaussian density and senones values (for active codebooks) */
	    best = (int32) 0x80000000;
	    for (gid = 0; gid < g->n_mgau; gid++) {
		if (mgau_active[gid]) {
		    gauden_dist (g, gid, topn, fv, dist);
		    for (m2s = mgau2sen[gid]; m2s; m2s = m2s->next) {
			s = m2s->sen;
			if (sen_active[s]) {
			    senscr[0][s] = senone_eval (sen, s, dist, topn);
			    if (best < senscr[0][s])
				best = senscr[0][s];
			}
		    }
		}
	    }

	    /* Interpolate CI and CD senones if indicated */
	    if (interp) {
		for (s = mdef->n_ci_sen; s < sen->n_sen; s++) {
		    if (sen_active[s])
			interp_cd_ci (interp, senscr[0], s, mdef->cd2cisen[s]);
		}
	    }

	    /* Normalize senone scores (interpolation above can only lower best score) */
	    for (s = 0; s < sen->n_sen; s++) {
		if (sen_active[s])
		    senscr[0][s] -= best;
	    }
	    senscale[i] = best;
	    timing_stop (tm_gausen);

	    /* Step HMMs one frame forward */
	    timing_start (tm_fwdsrch);
	    bestscr[i] = fwd_frame (senscr[0]);
	    timing_stop (tm_fwdsrch);
	    
	    if ((i%10) == 9) {
		printf ("."); fflush (stdout);
	    }
	}
    } else {
	/* Work in groups of GAUDEN_EVAL_WINDOW frames (blocking to improve cache perf) */
	for (j = 0; j < nfr; j += GAUDEN_EVAL_WINDOW) {
	    /* Compute Gaussian densities and senone scores for window of frames */
	    timing_start (tm_gausen);
	    for (gid = 0; gid < g->n_mgau; gid++) {
		for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		    /* Feature vector for current frame from input speech cepstra */
		    if (mfc) {
			if (i < w)
			    fcb->compute_feat (fcb, mfc+w, feat[0]);
			else if (i >= nfr-w)
			    fcb->compute_feat (fcb, mfc+(nfr-w-1), feat[0]);
			else
			    fcb->compute_feat (fcb, mfc+i, feat[0]);
			fv = feat[0];
		    } else
			fv = feat[i];
		    
		    /* Evaluate mixture Gaussian densities */
		    gauden_dist (g, gid, topn, fv, dist);
		    
		    /* Compute senone scores */
		    if (g->n_mgau > 1) {
			for (m2s = mgau2sen[gid]; m2s; m2s = m2s->next) {
			    s = m2s->sen;
			    senscr[k][s] = senone_eval (sen, s, dist, topn);
			}
		    } else {
			/* Semi-continuous special case; single shared codebook */
			senone_eval_all (sen, dist, topn, senscr[k]);
		    }
		}
	    }
	    
	    /* Interpolate senones and normalize */
	    for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		counter_increment (ctr_nsen, sen->n_sen);

		if (interp)
		    interp_all (interp, senscr[k], mdef->cd2cisen, mdef->n_ci_sen);

		/* Normalize senone scores */
		best = (int32)0x80000000;
		for (s = 0; s < sen->n_sen; s++)
		    if (best < senscr[k][s])
			best = senscr[k][s];
		for (s = 0; s < sen->n_sen; s++)
		    senscr[k][s] -= best;
		senscale[i] = best;
	    }
	    timing_stop (tm_gausen);
		
	    /* Step HMMs one frame forward */
	    timing_start (tm_fwdsrch);
	    for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		bestscr[i] = fwd_frame (senscr[k]);
		if ((i%10) == 9) {
		    printf ("."); fflush (stdout);
		}
	    }
	    timing_stop (tm_fwdsrch);
	}
    }
    printf ("\n");

    hyp = fwd_end_utt ();

    /* Add in senscale into bestscr, turning them into absolute scores */
    k = 0;
    for (i = 0; i < nfr; i++) {
	k += senscale[i];
	bestscr[i] += k;
    }

    counter_increment (ctr_nfrm, nfr);

    return hyp;
}


/* Decode the given mfc file and write result to matchfp and matchsegfp */
static void decode_utt (int32 nfr, char *uttid)
{
    char *bscrdir;
    hyp_t *hyp, *h;
    int32 i, bp, ascr, lscr, scl;
    float32 *f32arg;
    float64 lwf;

    timing_reset (tm_utt);
    timing_reset (tm_fwdvit);
    timing_reset (tm_bstpth);
    counter_reset_all ();

    timing_start (tm_utt);
    
    /* Viterbi and bestpath decode */
    timing_start (tm_fwdvit);
    hyp = fwdvit (nfr, uttid);
    timing_stop (tm_fwdvit);
    
    bp = *((int32 *) cmd_ln_access("-bestpath"));

    scl = 0;
    lwf = 1.0;
    if (hyp != NULL) {
	if ( *((int32 *) cmd_ln_access("-backtrace")) )
	    log_hyp_detailed (stdout, hyp, uttid, "FV", "fv");

	/* Total acoustic score scaling */
	for (i = 0; i < nfr; i++)
	    scl += senscale[i];

	/* Total scaled acoustic score and LM score */
	ascr = lscr = 0;
	for (h = hyp; h; h = h->next) {
	    ascr += h->ascr;
	    lscr += h->lscr;
	}

	/* Print sanitized recognition */
	printf ("FWDVIT: ");
	log_hypstr (stdout, hyp, uttid, matchexact, ascr + lscr);

	printf ("FWDXCT: ");
	log_hypseg (uttid, stdout, hyp, nfr, scl, lwf);
	lm_cache_stats_dump ();

	/* Check if need to dump bestscore/frame */
	if ((bscrdir = (char *) cmd_ln_access ("-bestscoredir")) != NULL)
	    write_bestscore (bscrdir, uttid, bestscr, nfr);

	/* Check if need to dump or search DAG */
	if ((outlatdir || bp) && (dag_build () == 0)) {
	    if (outlatdir)
		dag_dump (outlatdir, outlat_onlynodes, uttid);
	    
	    /* Perform bestpath DAG search if specified */
	    if (bp) {
		timing_start (tm_bstpth);
		h = dag_search (uttid);
		timing_stop (tm_bstpth);
		
		if (h) {
		    hyp = h;

		    f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
		    lwf = f32arg ?
			((*f32arg) / *((float32 *) cmd_ln_access ("-langwt"))) :
			1.0;
		} else
		    E_ERROR("%s: Bestpath search failed; using Viterbi result\n", uttid);
		
		if ( *((int32 *) cmd_ln_access("-backtrace")) )
		    log_hyp_detailed (stdout, hyp, uttid, "BP", "bp");
		
		/* Total scaled acoustic score and LM score */
		ascr = lscr = 0;
		for (h = hyp; h; h = h->next) {
		    ascr += h->ascr;
		    lscr += h->lscr;
		}
		
		printf ("BSTPTH: ");
		log_hypstr (stdout, hyp, uttid, matchexact, ascr + lscr);
		
		printf ("BSTXCT: ");
		log_hypseg (uttid, stdout, hyp, nfr, scl, lwf);
	    }
	    
	    dag_destroy ();
	}
	
	lm_cache_stats_dump ();
	lm_cache_reset ();
    } else {
	E_ERROR ("%s: Viterbi search failed\n", uttid);
	hyp = NULL;
    }
    
    /* Log recognition output to the standard match and matchseg files */
    if (matchfp)
	log_hypstr (matchfp, hyp, uttid, matchexact, 0);
    if (matchsegfp)
	log_hypseg (uttid, matchsegfp, hyp, nfr, scl, lwf);

    timing_stop (tm_utt);
    
    printf ("%s: ", uttid);
    counter_print_all (stdout);

    printf ("%s: TMR: %5d Frm", uttid, nfr);
    if (nfr > 0) {
	printf (" %6.2f xEl", tm_utt->t_elapsed * 100.0 / nfr);
	printf (" %6.2f xCPU", tm_utt->t_cpu * 100.0 / nfr);

	if (tm_utt->t_cpu > 0.0) {
	    printf (" [fwd %6.2fx %3d%%]", tm_fwdvit->t_cpu * 100.0 / nfr,
		    (int32) ((tm_fwdvit->t_cpu * 100.0) / tm_utt->t_cpu));
	    printf ("[gau+sen %6.2fx %2d%%]", tm_gausen->t_cpu * 100.0 / nfr,
		    (int32) ((tm_gausen->t_cpu * 100.0) / tm_utt->t_cpu));
	    printf ("[srch %6.2fx %2d%%]", tm_fwdsrch->t_cpu * 100.0 / nfr,
		    (int32) ((tm_fwdsrch->t_cpu * 100.0) / tm_utt->t_cpu));
	    
	    fwd_timing_dump (tm_utt->t_cpu);
	    
	    if (bp)
		printf ("[bp %6.2fx %2d%%]", tm_bstpth->t_cpu * 100.0 / nfr,
			(int32) ((tm_bstpth->t_cpu * 100.0) / tm_utt->t_cpu));
	}
    }
    printf ("\n");
    fflush (stdout);
    
    tot_nfr += nfr;
}


/* Process utterances in the control file (-ctlfn argument) */
static int32 process_ctlfile ( void )
{
    FILE *ctlfp, *mllrctlfp;
    char *ctlfile, *cepdir, *cepext, *mllrctlfile;
    char *matchfile, *matchsegfile;
    char line[1024], ctlspec[1024], cepfile[1024], uttid[1024];
    char mllrfile[1024], prevmllr[1024];
    int32 ctloffset, ctlcount, sf, ef, nfr;
    int32 err_status;
    int32 i, k;
    
    err_status = 0;
    
    if ((ctlfile = (char *) cmd_ln_access("-ctlfn")) == NULL)
	E_FATAL("No -ctlfn argument\n");
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    if ((mllrctlfile = (char *) cmd_ln_access("-mllrctlfn")) != NULL) {
	if ((mllrctlfp = fopen (mllrctlfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", mllrctlfile);
    } else
	mllrctlfp = NULL;
    prevmllr[0] = '\0';
    
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    if ((matchfile = (char *) cmd_ln_access("-matchfn")) == NULL) {
	matchfp = NULL;
    } else {
	/* Look for ,EXACT suffix, for retaining fillers/pronunciation specs in output */
	k = strlen (matchfile);
	if ((k > 6) && (strcmp (matchfile+(k-6), ",EXACT") == 0)) {
	    matchexact = 1;
	    matchfile[k-6] = '\0';
	} else
	    matchexact = 0;

	if ((matchfp = fopen (matchfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchfile);
    }
    
    if ((matchsegfile = (char *) cmd_ln_access("-matchsegfn")) == NULL) {
	E_WARN("No -matchsegfn argument\n");
	matchsegfp = NULL;
    } else {
	if ((matchsegfp = fopen (matchsegfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchsegfile);
    }
    
    cepdir = (char *) cmd_ln_access("-cepdir");
    cepext = (char *) cmd_ln_access("-cepext");
    assert ((cepdir != NULL) && (cepext != NULL));
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    if (! cmd_ln_access("-ctlcount"))
	ctlcount = 0x7fffffff;	/* All entries processed if no count specified */
    else
	ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));
    if (ctlcount == 0) {
	E_INFO("-ctlcount argument = 0!!\n");
	fclose (ctlfp);
	return err_status;
    }
    
    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    if (mllrctlfp) {
		if (fscanf (mllrctlfp, "%s", mllrfile) != 1)
		    E_FATAL ("Unexpected EOF(%s)\n", mllrctlfile);
	    }
	    --ctloffset;
	}
    }
    
    while ((ctlcount > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	printf ("\n");
	E_INFO("Utterance: %s", line);

	sf = 0;
	ef = (int32)0x7ffffff0;
	if ((k = sscanf (line, "%s %d %d %s", ctlspec, &sf, &ef, uttid)) <= 0)
	    continue;	    /* Empty line */

	if ((k == 2) || ( (k >= 3) && ((sf >= ef) || (sf < 0))) ) {
	    E_ERROR("Error in ctlfile spec; skipped\n");
	    /* What happens to ctlcount??? */
	    continue;
	}
	if (k < 4) {
	    /* Create utt-id from mfc-filename (and sf/ef if specified) */
	    for (i = strlen(ctlspec)-1; (i >= 0) && (ctlspec[i] != '/'); --i);
	    if (k == 3)
		sprintf (uttid, "%s_%d_%d", ctlspec+i+1, sf, ef);
	    else
		strcpy (uttid, ctlspec+i+1);
	}

	if (mllrctlfp) {
	    if (fscanf (mllrctlfp, "%s", mllrfile) != 1)
		E_FATAL ("Unexpected EOF(%s)\n", mllrctlfile);
	    
	    if (strcmp (prevmllr, mllrfile) != 0) {
		float32 ***A, **B;
		int32 gid, sid;
		uint8 *mgau_xform;
		
		gauden_mean_reload (g, (char *) cmd_ln_access("-meanfn"));
		
		if (mllr_read_regmat (mllrfile, &A, &B,
				      fcb->stream_len, feat_n_stream(fcb)) < 0)
		    E_FATAL("mllr_read_regmat failed\n");
		
		mgau_xform = (uint8 *) ckd_calloc (g->n_mgau, sizeof(uint8));

		/* Transform each non-CI mixture Gaussian */
		for (sid = 0; sid < sen->n_sen; sid++) {
		    if (mdef->cd2cisen[sid] != sid) {	/* Otherwise it's a CI senone */
			gid = sen->mgau[sid];
			if (! mgau_xform[gid]) {
			    mllr_norm_mgau (g->mean[gid], g->n_density, A, B,
					    fcb->stream_len, feat_n_stream(fcb));
			    mgau_xform[gid] = 1;
			}
		    }
		}

		ckd_free (mgau_xform);
		
		mllr_free_regmat (A, B, fcb->stream_len, feat_n_stream(fcb));

		strcpy (prevmllr, mllrfile);
	    }
	}
	
	/* Form full cepfile name */
	if (ctlspec[0] != '/')
	    sprintf (cepfile, "%s/%s.%s", cepdir, ctlspec, cepext);
	else
	    sprintf (cepfile, "%s.%s", ctlspec, cepext);
	
	if (! feat) {
	    /* One time allocation of space for MFC data/feature vector */
	    if (fcb->compute_feat) {
		mfc = (float32 **) ckd_calloc_2d (S3_MAX_FRAMES, feat_cepsize(fcb),
						  sizeof(float32));
		feat = feat_array_alloc (fcb, 1);
	    } else {
		/* Speech input is directly feature vectors; no MFC input */
		feat = feat_array_alloc (fcb, S3_MAX_FRAMES);
	    }
	}
	
	/* Read MFC/feature speech input file */
	if (fcb->compute_feat)
	    nfr = s2mfc_read (cepfile, sf, ef, mfc, S3_MAX_FRAMES);
	else
	    nfr = feat_readfile (fcb, cepfile, sf, ef, feat, S3_MAX_FRAMES);
	
	if (nfr <= 0) {
	    E_ERROR("Utt %s: Input file read (%s) failed\n", uttid, cepfile);
	    
	    /* Log NULL recognition output to the standard match and matchseg files */
	    if (matchfp)
		log_hypstr (matchfp, NULL, uttid, matchexact, 0);
	    if (matchsegfp)
		log_hypseg (uttid, matchsegfp, NULL, 0, 0, 1.0);
	    
	    err_status = 1;
	} else {
	    E_INFO ("%s: %d input frames\n", uttid, nfr);
	    decode_utt (nfr, uttid);
	}
	
#if 0
	linklist_stats ();
#endif
	--ctlcount;
    }
    printf ("\n");

    while (fgets(line, sizeof(line), ctlfp) != NULL) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    E_INFO("Skipping rest of control file beginning with:\n\t%s", line);
	    break;
	}
    }
    
    if (matchfp)
	fclose (matchfp);
    if (matchsegfp)
	fclose (matchsegfp);

    fclose (ctlfp);
    if (mllrctlfp)
	fclose (mllrctlfp);
    
    return (err_status);
}


static int32 load_argfile (char *file, char *pgm, char ***argvout)
{
    FILE *fp;
    char line[1024], word[1024], *lp, **argv;
    int32 len, n;

    E_INFO("Reading arguments from %s\n", file);

    if ((fp = fopen (file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return -1;
    }

    /* Count #arguments */
    n = 1;	/* Including pgm */
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    n++;
	}
    }
    
    /* Allocate space for arguments */
    argv = (char **) ckd_calloc (n+1, sizeof(char *));
    
    /* Create argv list */
    rewind (fp);
    argv[0] = pgm;
    n = 1;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    argv[n] = ckd_salloc (word);
	    n++;
	}
    }
    argv[n] = NULL;
    *argvout = argv;

    fclose (fp);

    return n;
}


main (int32 argc, char *argv[])
{
    char *str;
    int32 err_status;
    
#if 0
    ckd_debug(100000);
#endif
    
    /* Digest command line argument definitions */
    cmd_ln_define (defn);

    if ((argc == 2) && (strcmp (argv[1], "help") == 0)) {
	cmd_ln_print_definitions();
	exit(1); 
    }

    /* Look for default or specified arguments file */
    str = NULL;
    if ((argc == 2) && (argv[1][0] != '-'))
	str = argv[1];
    else if (argc == 1) {
	str = "s3decode.arg";
	E_INFO("Default argument file: %s\n", str);
    }
    if (str) {
	/* Build command line argument list from file */
	if ((argc = load_argfile (str, argv[0], &argv)) < 0) {
	    fprintf (stderr, "Usage:\n");
	    fprintf (stderr, "\t%s argument-list, or\n", argv[0]);
	    fprintf (stderr, "\t%s [argument-file] (default file: s3decode.arg)\n\n",
		     argv[0]);
	    cmd_ln_print_definitions();
	    exit(1);
	}
    }
    
    cmdline_parse (argc, argv);

    /* Remove memory allocation restrictions */
    unlimit ();
    
#if (! WIN32)
    {
	char buf[1024];
	
	gethostname (buf, 1024);
	buf[1023] = '\0';
	E_INFO ("Executing on: %s\n", buf);
    }
#endif

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);
    
    if ((cmd_ln_access("-mdeffn") == NULL) ||
	(cmd_ln_access("-meanfn") == NULL) ||
	(cmd_ln_access("-varfn") == NULL)  ||
	(cmd_ln_access("-mixwfn") == NULL)  ||
	(cmd_ln_access("-tmatfn") == NULL))
	E_FATAL("Missing -mdeffn, -meanfn, -varfn, -mixwfn, or -tmatfn argument\n");

    if ((cmd_ln_access("-dictfn") == NULL) ||
	(cmd_ln_access("-lmfn") == NULL))
	E_FATAL("Missing -dictfn or -lmfn argument\n");
    
    inlatdir = (char *) cmd_ln_access ("-inlatdir");
    outlatdir = (char *) cmd_ln_access ("-outlatdir");
    if (outlatdir) {
	int32 k;
	
	k = strlen(outlatdir);
	if ((k > 6) && (strcmp (outlatdir+(k-6), ",NODES") == 0)) {
	    outlat_onlynodes = 1;
	    outlatdir[k-6] = '\0';
	} else
	    outlat_onlynodes = 0;
    }

    if (inlatdir && outlatdir && (strcmp (inlatdir, outlatdir) == 0))
	E_FATAL("Input and output lattice directories are the same\n");

    /*
     * Initialize log(S3-base).  All scores (probs...) computed in log domain to avoid
     * underflow.  At the same time, log base = 1.0001 (1+epsilon) to allow log values
     * to be maintained in int32 variables without significant loss of precision.
     */
    {
	float32 logbase;
    
	logbase = *((float32 *) cmd_ln_access("-logbase"));
	if (logbase <= 1.0)
	    E_FATAL("Illegal log-base: %e; must be > 1.0\n", logbase);
	if (logbase > 1.01)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init ((float64) logbase);
    }

    /* Initialize feature stream type */
    fcb = feat_init ((char *) cmd_ln_access ("-feat"));
    
    /* Read in input databases */
    models_init ();
    
    /* Senone scaling factor in each frame */
    senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));

    /* Best statescore in each frame */
    bestscr = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    
    /* Allocate profiling timers and counters */
    tm_utt = timing_new ();
    tm_fwdvit = timing_new ();
    tm_bstpth = timing_new ();
    tm_gausen = timing_new ();
    tm_fwdsrch = timing_new ();
    ctr_nfrm = counter_new ("frm");
    ctr_nsen = counter_new ("sen");

    /* Initialize forward Viterbi search module */
    fwd_init ();
    printf ("\n");
    
    tot_nfr = 0;
    
    err_status = process_ctlfile ();

    if (tot_nfr > 0) {
	printf ("\n");
	printf("TOTAL FRAMES:       %8d\n", tot_nfr);
	printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_cpu, tm_utt->t_tot_cpu/(tot_nfr*0.01));
	printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_elapsed, tm_utt->t_tot_elapsed/(tot_nfr*0.01));
	fflush (stdout);
    }

#if (! WIN32)
    system ("ps aguxwww | grep s3decode");
#endif

    /* Hack!! To avoid hanging problem under Linux */
    if (logfp) {
	fclose (logfp);
	*stdout = orig_stdout;
	*stderr = orig_stderr;
    }

    exit(err_status);
}
