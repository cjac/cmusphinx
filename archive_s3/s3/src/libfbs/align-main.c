/*
 * align-main.c -- Main driver routine for time alignment.
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
 * 19-Jun-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to handle the new libfeat interface.
 * 
 * 11-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added MLLR transformation for each utterance.
 * 
 * 06-Mar-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 *  
 * 16-Oct-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 *  
 * 14-Oct-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Removed explicit addition of SILENCE_WORD, START_WORD and
 * 		FINISH_WORD to the dictionary.
 * 
 * 18-Sep-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added optional start/end frame specification in control file, for
 * 		processing selected segments (utterances) from a large cepfile.
 * 		Control spec: cepfile [startframe endframe [uttid]].
 * 		(There are incompatibilities with ,CTL output directory specification.)
 * 
 * 13-Sep-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Normalized senone scores (subtracting the best) rather than density scores.
 * 		Bugfix: Absolute scores written to state score output file by removing
 * 		normalization factor.
 * 
 * 22-Jul-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added absolute (unnormalized) acoustic scores in log file.
 * 		Added Sphinx-II compatible output segmentation files.
 * 
 * 15-Jul-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
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
#include "gauden.h"
#include "senone.h"
#include "interp.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "align.h"
#include "mllr.h"
#include "agc.h"
#include "cmn.h"


static arg_def_t defn[] = {
    { "-logbase",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
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
      "Optional filler word (noise word) pronunciation dictionary input file" },
    { "-compwd",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "0",
      "Compound words in dictionary (not supported yet)" },
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
    { "-insentfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Input transcript file corresponding to control file" },
    { "-outsentfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output transcript file with exact pronunciation/transcription" },
    { "-stsegdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output directory for state segmentation files; optionally end with ,CTL" },
    { "-phsegdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output directory for phone segmentation files; optionally end with ,CTL" },
    { "-wdsegdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output directory for word segmentation files; optionally end with ,CTL" },
    { "-s2stsegdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output directory for Sphinx-II format state segmentation files; optionally end with ,CTL" },
    { "-logfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Log file (default stdout/stderr)" },
    
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

static FILE *outsentfp = NULL;

/* For profiling/timing */
static int32 tmr_utt;
static int32 tmr_gauden;
static int32 tmr_senone;
static int32 tmr_align;

static int32 tot_nfr;
static timing_t *tm_utt;


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

    arg = (char *) cmd_ln_access ("-agc");
    if ((strcmp (arg, "max") != 0) && (strcmp (arg, "none") != 0))
	E_FATAL("Unknown -agc argument: %s\n", arg);
    arg = (char *) cmd_ln_access ("-cmn");
    if ((strcmp (arg, "current") != 0) && (strcmp (arg, "none") != 0))
	E_FATAL("Unknown -cmn argument: %s\n", arg);
}


/*
 * Build a filename int buf as follows (without file extension):
 *     if dir ends with ,CTL and ctlspec does not begin with /, filename is dir/ctlspec
 *     if dir ends with ,CTL and ctlspec DOES begin with /, filename is ctlspec
 *     if dir does not end with ,CTL, filename is dir/uttid,
 * where ctlspec is the complete utterance spec in the input control file, and
 * uttid is the last component of ctlspec.
 */
static void build_output_uttfile (char *buf, char *dir, char *uttid, char *ctlspec)
{
    int32 k;
    
    k = strlen(dir);
    if ((k > 4) && (strcmp (dir+k-4, ",CTL") == 0)) {	/* HACK!! Hardwired ,CTL */
	if (ctlspec[0] != '/') {
	    strcpy (buf, dir);
	    buf[k-4] = '/';
	    strcpy (buf+k-3, ctlspec);
	} else
	    strcpy (buf, ctlspec);
    } else {
	strcpy (buf, dir);
	buf[k] = '/';
	strcpy (buf+k+1, uttid);
    }
}


/*
 * Write state segmentation in Sphinx-II format.  (Must be written in BIG-ENDIAN
 * format!)
 */
static void write_s2stseg (char *dir, align_stseg_t *stseg, char *uttid, char *ctlspec)
{
    char filename[1024];
    FILE *fp;
    align_stseg_t *tmp;
    int32 k;
    s3cipid_t ci[3];
    word_posn_t wpos;
    int16 s2_info;
    char buf[8];
    static int32 byterev = -1;	/* Whether to byte reverse output data */
    
    build_output_uttfile (filename, dir, uttid, ctlspec);
    strcat (filename, ".v8_seg");		/* .v8_seg for compatibility */
    E_INFO("Writing Sphinx-II format state segmentation to: %s\n", filename);
    if ((fp = fopen (filename, "wb")) == NULL) {
	E_ERROR("fopen(%s,wb) failed\n", filename);
	return;
    }

    if (byterev < 0) {
	/* Byte ordering of host machine unknown; first figure it out */
	k = (int32) BYTE_ORDER_MAGIC;
	if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	    goto write_error;

	fclose (fp);
	if ((fp = fopen (filename, "rb")) == NULL) {
	    E_ERROR ("fopen(%s,rb) failed\n", filename);
	    return;
	}
	if (fread (buf, 1, sizeof(int32), fp) != sizeof(int32)) {
	    E_ERROR ("fread(%s) failed\n", filename);
	    return;
	}
	fclose (fp);
	
	/* If buf[0] == lsB of BYTE_ORDER_MAGIC, we are little-endian.  Need to byterev */
	byterev = (buf[0] == (BYTE_ORDER_MAGIC & 0x000000ff)) ? 1 : 0;

	if ((fp = fopen (filename, "wb")) == NULL) {
	    E_ERROR("fopen(%s,wb) failed\n", filename);
	    return;
	}
    }
    
    /* Write #frames */
    for (k = 0, tmp = stseg; tmp; k++, tmp = tmp->next);
    if (byterev)
	SWAP_INT32(&k);
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write state info for each frame */
    for (; stseg; stseg = stseg->next) {
	mdef_phone_components (mdef, stseg->pid, ci, &(ci[1]), &(ci[2]), &wpos);

	s2_info = ci[0] * mdef->n_emit_state + stseg->state;
	if (stseg->start)
	    s2_info |= 0x8000;
	if (byterev)
	    SWAP_INT16(&s2_info);
	
	if (fwrite (&s2_info, sizeof(int16), 1, fp) != 1)
	    goto write_error;
    }
    
    fclose (fp);
    return;
    
write_error:
    E_ERROR("fwrite(%s) failed\n", filename);
    fclose (fp);
}


/* Write state segmentation output file */
static void write_stseg (char *dir, align_stseg_t *stseg, char *uttid, char *ctlspec)
{
    char filename[1024];
    FILE *fp;
    align_stseg_t *tmp;
    int32 i, k;
    s3cipid_t ci[3];
    uint8 pos;
    char *str;
    word_posn_t wpos;
    
    build_output_uttfile (filename, dir, uttid, ctlspec);
    strcat (filename, ".stseg");
    E_INFO("Writing state segmentation to: %s\n", filename);
    if ((fp = fopen (filename, "wb")) == NULL) {
	E_ERROR("fopen(%s,wb) failed\n", filename);
	return;
    }
    
    /* Write version no. */
    if (fwrite ("0.1\n", sizeof(char), 4, fp) != 4)
	goto write_error;

    /* Write CI phone names */
    for (k = 0; k < mdef->n_ciphone; k++) {
	str = mdef_ciphone_str (mdef, k);
	if (fwrite (str, sizeof(char), strlen(str), fp) != strlen(str))
	    goto write_error;
	if (fwrite (" ", sizeof(char), 1, fp) != 1)
	    goto write_error;
    }
    str = WPOS_NAME;
    if (fwrite (str, sizeof(char), strlen(str), fp) != strlen(str))
	goto write_error;

    /* Write format "description" */
    str = "\nCI.8 LC.8 RC.8 POS.3(HI)-ST.5(LO) SCR(32)\n";
    if (fwrite (str, sizeof(char), strlen(str), fp) != strlen(str))
	goto write_error;

    /* Write binary comment string */
    if (fwrite ("*end_comment*\n", sizeof(char), 14, fp) != 14)
	goto write_error;

    /* Write byte-ordering magic number */
    k = BYTE_ORDER_MAGIC;
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write #frames */
    for (k = 0, tmp = stseg; tmp; k++, tmp = tmp->next);
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write state segmentation for each frame */
    for (i = 0; stseg; i++, stseg = stseg->next) {
	mdef_phone_components (mdef, stseg->pid, ci, &(ci[1]), &(ci[2]), &wpos);
	assert ((wpos >= 0) && (wpos < 8));
	assert ((stseg->state >= 0) && (stseg->state < 32));
	
	if (fwrite (ci, sizeof(s3cipid_t), 3, fp) != 3)
	    goto write_error;
	pos = (wpos << 5) | (stseg->state & 0x001f);
	if (fwrite (&pos, sizeof(uint8), 1, fp) != 1)
	    goto write_error;

	k = stseg->score + senscale[i];
	if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	    goto write_error;
    }
    
    fclose (fp);
    return;
    
write_error:
    E_ERROR("fwrite(%s) failed\n", filename);
    fclose (fp);
}


/* Write phone segmentation output file */
static void write_phseg (char *dir, align_phseg_t *phseg, char *uttid, char *ctlspec)
{
    char str[1024];
    FILE *fp;
    int32 uttscr, f, scale;
    
    /* Attempt to write segmentation for this utt to a separate file */
    build_output_uttfile (str, dir, uttid, ctlspec);
    strcat (str, ".phseg");
    E_INFO("Writing phone segmentation to: %s\n", str);
    if ((fp = fopen (str, "w")) == NULL) {
	E_ERROR("fopen(%s,w) failed\n", str);
	fp = stdout;	/* Segmentations can be directed to stdout this way */
	E_INFO ("Phone segmentation (%s):\n", uttid);
	dir = NULL;	/* Flag to indicate fp shouldn't be closed at the end */
    }
    
    if (! dir)
	fprintf (fp, "PH:%s>", uttid);
    fprintf (fp, "\t%5s %5s %9s %s\n",
	     "SFrm", "EFrm", "SegAScr", "Phone");
    
    uttscr = 0;
    for (; phseg; phseg = phseg->next) {
	mdef_phone_str (mdef, phseg->pid, str);
	
	/* Account for senone score scaling in each frame */
	scale = 0;
	for (f = phseg->sf; f <= phseg->ef; f++)
	    scale += senscale[f];
	
	if (! dir)
	    fprintf (fp, "ph:%s>", uttid);
	fprintf (fp, "\t%5d %5d %9d %s\n",
		 phseg->sf, phseg->ef, phseg->score + scale, str);
	
	uttscr += (phseg->score + scale);
    }

    if (! dir)
	fprintf (fp, "PH:%s>", uttid);
    fprintf (fp, " Total score: %11d\n", uttscr);

    if (dir)
	fclose (fp);
    else
	fprintf (fp, "\n");
}


/* Write word segmentation output file */
static void write_wdseg (char *dir, align_wdseg_t *wdseg, char *uttid, char *ctlspec)
{
    char str[1024];
    FILE *fp;
    int32 uttscr, f, scale;
    
    /* Attempt to write segmentation for this utt to a separate file */
    build_output_uttfile (str, dir, uttid, ctlspec);
    strcat (str, ".wdseg");
    E_INFO("Writing word segmentation to: %s\n", str);
    if ((fp = fopen (str, "w")) == NULL) {
	E_ERROR("fopen(%s,w) failed\n", str);
	fp = stdout;	/* Segmentations can be directed to stdout this way */
	E_INFO ("Word segmentation (%s):\n", uttid);
	dir = NULL;	/* Flag to indicate fp shouldn't be closed at the end */
    }
    
    if (! dir)
	fprintf (fp, "WD:%s>", uttid);
    fprintf (fp, "\t%5s %5s %10s %s\n",
	     "SFrm", "EFrm", "SegAScr", "Word");
    
    uttscr = 0;
    for (; wdseg; wdseg = wdseg->next) {
	/* Account for senone score scaling in each frame */
	scale = 0;
	for (f = wdseg->sf; f <= wdseg->ef; f++)
	    scale += senscale[f];

	if (! dir)
	    fprintf (fp, "wd:%s>", uttid);
	fprintf (fp, "\t%5d %5d %10d %s\n",
		 wdseg->sf, wdseg->ef, wdseg->score + scale, dict_wordstr (wdseg->wid));

	uttscr += (wdseg->score + scale);
    }

    if (! dir)
	fprintf (fp, "WD:%s>", uttid);
    fprintf (fp, " Total score: %11d\n", uttscr);

    if (dir)
	fclose (fp);
    else
	fprintf (fp, "\n");
}


/* Write exact transcription (pronunciation and silence/noise words included) */
static void write_outsent (FILE *fp, align_wdseg_t *wdseg, char *uttid)
{
    for (; wdseg; wdseg = wdseg->next)
	fprintf (fp, "%s ", dict_wordstr (wdseg->wid));
    fprintf (fp, " (%s)\n", uttid);
    fflush (fp);
}


/*
 * Find Viterbi alignment.
 */
static void align_utt (char *sent,	/* In: Reference transcript */
		       int32 nfr,	/* In: #frames of input */
		       char *ctlspec,	/* In: Utt specifiction from control file */
		       char *uttid)	/* In: Utterance id, for logging and other use */
{
    static int32 w;
    static int32 topn;
    static gauden_dist_t ***dist;
    static int32 *senscr = NULL;
    static s3senid_t *sen_active;
    static int8 *mgau_active;
    static char *s2stsegdir;
    static char *stsegdir;
    static char *phsegdir;
    static char *wdsegdir;
    
    int32 i, s, sid, gid, n_sen_active, best;
    char *arg;
    align_stseg_t *stseg;
    align_phseg_t *phseg;
    align_wdseg_t *wdseg;
    float32 **fv;
    
    if (! senscr) {
	/* One-time allocation of necessary intermediate variables */

	/* Allocate space for top-N codeword density values in a codebook */
	w = feat_window_size (fcb);	/* #MFC vectors needed on either side of current
					   frame to compute one feature vector */
	topn = *((int32 *) cmd_ln_access("-topn"));
	if (topn > g->n_density) {
	    E_ERROR("-topn argument (%d) > #density codewords (%d); set to latter\n",
		   topn, g->n_density);
	    topn = g->n_density;
	}
	dist = (gauden_dist_t ***) ckd_calloc_3d (g->n_mgau, g->n_feat, topn,
						  sizeof(gauden_dist_t));
	
	/* Space for one frame of senone scores, and per frame active flags */
	senscr = (int32 *) ckd_calloc (sen->n_sen, sizeof(int32));
	sen_active = (s3senid_t *) ckd_calloc (sen->n_sen, sizeof(s3senid_t));
	mgau_active = (int8 *) ckd_calloc (g->n_mgau, sizeof(int8));

	/* Note various output directories */
	s2stsegdir = NULL;
	stsegdir = NULL;
	phsegdir = NULL;
	wdsegdir = NULL;
	if ((arg = (char *) cmd_ln_access ("-s2stsegdir")) != NULL)
	    s2stsegdir = (char *) ckd_salloc (arg);
	if ((arg = (char *) cmd_ln_access ("-stsegdir")) != NULL)
	    stsegdir = (char *) ckd_salloc (arg);
	if ((arg = (char *) cmd_ln_access ("-phsegdir")) != NULL)
	    phsegdir = (char *) ckd_salloc (arg);
	if ((arg = (char *) cmd_ln_access ("-wdsegdir")) != NULL)
	    wdsegdir = (char *) ckd_salloc (arg);
    }
    
    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return;
    }
    
    cyctimer_reset_all ();
    counter_reset_all ();
    
    timing_reset (tm_utt);
    timing_start (tm_utt);
    cyctimer_resume (tmr_utt);

    /* AGC and CMN */
    if (mfc) {
	arg = (char *) cmd_ln_access ("-cmn");
	if (strcmp (arg, "current") == 0)
	    norm_mean (mfc, nfr, feat_cepsize(fcb));
	arg = (char *) cmd_ln_access ("-agc");
	if (strcmp (arg, "max") == 0)
	    agc_max (mfc, nfr);
    }
    
    if (align_build_sent_hmm (sent) != 0) {
	align_destroy_sent_hmm ();
	cyctimer_pause (tmr_utt);

	E_ERROR("No sentence HMM; no alignment for %s\n", uttid);
	
	return;
    }
    
    align_start_utt (uttid);
    
    for (i = 0; i < nfr; i++) {
	cyctimer_resume (tmr_utt);
	
	/*
	 * Compute feature vector for current frame from input speech cepstra.
	 * A feature vector for frame f depends on input MFC vectors [f-w..f+w].  Hence
	 * the feature vector corresponding to the first w and last w input frames is
	 * undefined.  We define them by simply replicating the first and last true
	 * feature vectors (presumably silence regions).
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
	
	/*
	 * Evaluate gaussian density codebooks and senone scores for input codeword.
	 * Evaluate only active codebooks and senones.
	 */
	/* Obtain active senone flags */
	cyctimer_resume (tmr_senone);
	align_sen_active (sen_active, sen->n_sen);
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
	cyctimer_pause (tmr_senone);
	
	/* Flag all active mixture-gaussian codebooks */
	cyctimer_resume (tmr_gauden);
	for (gid = 0; gid < g->n_mgau; gid++)
	    mgau_active[gid] = 0;
	for (s = 0; s < n_sen_active; s++) {
	    sid = sen_active[s];
	    mgau_active[sen->mgau[sid]] = 1;
	}
	
	/* Compute topn gaussian density values (for active codebooks) */
	for (gid = 0; gid < g->n_mgau; gid++)
	    if (mgau_active[gid])
		gauden_dist (g, gid, topn, fv, dist[gid]);
	cyctimer_pause (tmr_gauden);
	
	/* Evaluate active senones */
	cyctimer_resume (tmr_senone);
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
	cyctimer_pause (tmr_senone);
	
	/* Step alignment one frame forward */
	cyctimer_resume (tmr_align);
	align_frame (senscr);
	cyctimer_pause (tmr_align);
	
	cyctimer_pause (tmr_utt);
    }
    timing_stop (tm_utt);

    printf ("\n");

    /* Wind up alignment for this utterance */
    if (align_end_utt (&stseg, &phseg, &wdseg) < 0)
	E_ERROR("Final state not reached; no alignment for %s\n\n", uttid);
    else {
	if (s2stsegdir)
	    write_s2stseg (s2stsegdir, stseg, uttid, ctlspec);
	if (stsegdir)
	    write_stseg (stsegdir, stseg, uttid, ctlspec);
	if (phsegdir)
	    write_phseg (phsegdir, phseg, uttid, ctlspec);
	if (wdsegdir)
	    write_wdseg (wdsegdir, wdseg, uttid, ctlspec);
	if (outsentfp)
	    write_outsent (outsentfp, wdseg, uttid);
    }
    
    align_destroy_sent_hmm ();
    
    cyctimer_print_all_norm (stdout, nfr*0.01, tmr_utt);
    counter_print_all (stdout);

    printf("EXECTIME: %5d frames, %7.2f sec CPU, %6.2f xRT; %7.2f sec elapsed, %6.2f xRT\n",
	   nfr,
	   tm_utt->t_cpu, tm_utt->t_cpu * 100.0 / nfr,
	   tm_utt->t_elapsed, tm_utt->t_elapsed * 100.0 / nfr);

    tot_nfr += nfr;
}


#define UPPER_CASE(c)   ((((c) >= 'a') && ((c) <= 'z')) ? (c-32) : c)
/* Case insensitive string compare */
static int32 id_cmp (char *str1, char *str2)
{
    char c1, c2;
    
    for (;;) {
        c1 = *(str1++);
        c1 = UPPER_CASE(c1);
        c2 = *(str2++);
        c2 = UPPER_CASE(c2);
        if (c1 != c2)
            return (c1-c2);
        if (c1 == '\0')
            return 0;
    }
}


/* Process utterances in the control file (-ctlfn argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp, *sentfp, *mllrctlfp;
    char *ctlfile, *cepdir, *cepext, *sentfile, *outsentfile, *mllrctlfile;
    char line[1024], cepfile[1024], ctlspec[1024];
    int32 ctloffset, ctlcount, sf, ef, nfr;
    char mllrfile[4096], prevmllr[4096], sent[16384];
    char uttid[1024];
    int32 i, k;
    
    ctlfile = (char *) cmd_ln_access("-ctlfn");
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    if ((mllrctlfile = (char *) cmd_ln_access("-mllrctlfn")) != NULL) {
	if ((mllrctlfp = fopen (mllrctlfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", mllrctlfile);
    } else
	mllrctlfp = NULL;
    prevmllr[0] = '\0';
    
    sentfile = (char *) cmd_ln_access("-insentfn");
    if ((sentfp = fopen (sentfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", sentfile);

    if ((outsentfile = (char *) cmd_ln_access("-outsentfn")) != NULL) {
	if ((outsentfp = fopen (outsentfile, "w")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", outsentfile);
    }
    
    E_INFO("Processing ctl file %s\n", ctlfile);
    
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
	fclose (sentfp);
	if (outsentfp)
	    fclose (outsentfp);
	
	return;
    }
    
    /* Skipping initial offset */
    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    if (fgets (sent, sizeof(sent), sentfp) == NULL) {
		E_ERROR("EOF(%s)\n", sentfile);
		ctlcount = 0;
		break;
	    }
	    if (mllrctlfp) {
		if (fscanf (mllrctlfp, "%s", mllrfile) != 1)
		    E_FATAL ("Unexpected EOF(%s)\n", mllrctlfile);
	    }
	    --ctloffset;
	}
    }

    /* Process the specified number of utterance or until end of control file */
    while ((ctlcount > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	printf ("\n");
	E_INFO("Utterance: %s", line);
	
	sf = 0;
	ef = (int32)0x7ffffff0;
	if ((k = sscanf (line, "%s %d %d %s", ctlspec, &sf, &ef, uttid)) <= 0)
	    continue;	    /* Empty line */

	if ((k == 2) || ( (k >= 3) && ((sf >= ef) || (sf < 0))) )
	    E_FATAL("Bad ctlfile line: %s\n", line);

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

	if (ctlspec[0] != '/')
	    sprintf (cepfile, "%s/%s.%s", cepdir, ctlspec, cepext);
	else
	    sprintf (cepfile, "%s.%s", ctlspec, cepext);
	
	/* Read utterance transcript */
	if (fgets (sent, sizeof(sent), sentfp) == NULL) {
	    E_ERROR("EOF(%s)\n", sentfile);
	    break;
	}
	/* Strip utterance id from the end of the transcript */
	for (k = strlen(sent) - 1;
	     (k > 0) && ((sent[k] == '\n') || (sent[k] == '\t') || (sent[k] == ' '));
	     --k);
	if ((k > 0) && (sent[k] == ')')) {
	    for (--k; (k >= 0) && (sent[k] != '('); --k);
	    if ((k >= 0) && (sent[k] == '(')) {
		sent[k] = '\0';

		/* Check that uttid in transcript and control file match */
		for (i = ++k;
		     sent[i] && (sent[i] != ')') &&
			 (sent[i] != '\n') && (sent[i] != '\t') && (sent[i] != ' ');
		     i++);
		sent[i] = '\0';
		if (id_cmp (sent+k, uttid) != 0)
		    E_ERROR("Uttid mismatch: ctlfile = \"%s\"; transcript = \"%s\"\n",
			   uttid, sent+k);
	    }
	}
	
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
	
	/* Read and process mfc/feature speech input file */
	if (fcb->compute_feat)
	    nfr = s2mfc_read (cepfile, sf, ef, mfc, S3_MAX_FRAMES);
	else
	    nfr = feat_readfile (fcb, cepfile, sf, ef, feat, S3_MAX_FRAMES);
	
	if (nfr <= 0)
	    E_ERROR("Utt %s: Input file read (%s) failed\n", uttid, cepfile);
	else {
	    E_INFO ("%s: %d input frames\n", uttid, nfr);
	    align_utt (sent, nfr, ctlspec, uttid);
	}
	
	--ctlcount;
    }
    printf ("\n");

    while (fgets(line, sizeof(line), ctlfp) != NULL) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    E_INFO("Skipping rest of control file beginning with:\n\t%s", line);
	    break;
	}
    }

    fclose (ctlfp);
    fclose (sentfp);
    if (outsentfp)
	fclose (outsentfp);
    if (mllrctlfp)
	fclose (mllrctlfp);
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

#if 0
    ckd_debug(100000);
#endif
    
    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);
    
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
	str = "s3align.arg";
	E_INFO("Looking for default argument file: %s\n", str);
    }
    if (str) {
	/* Build command line argument list from file */
	if ((argc = load_argfile (str, argv[0], &argv)) < 0) {
	    fprintf (stderr, "Usage:\n");
	    fprintf (stderr, "\t%s argument-list, or\n", argv[0]);
	    fprintf (stderr, "\t%s [argument-file] (default file: s3align.arg)\n\n",
		     argv[0]);
	    cmd_ln_print_definitions();
	    exit(1);
	}
    }
    
    cmdline_parse (argc, argv);
    
    if ((cmd_ln_access("-mdeffn") == NULL) ||
	(cmd_ln_access("-meanfn") == NULL) ||
	(cmd_ln_access("-varfn") == NULL)  ||
	(cmd_ln_access("-mixwfn") == NULL)  ||
	(cmd_ln_access("-tmatfn") == NULL) ||
	(cmd_ln_access("-dictfn") == NULL))
	E_FATAL("Missing -mdeffn, -meanfn, -varfn, -mixwfn, -tmatfn, or -dictfn argument\n");
    
    if ((cmd_ln_access("-ctlfn") == NULL) || (cmd_ln_access("-insentfn") == NULL))
	E_FATAL("Missing -ctlfn or -insentfn argument\n");

    if ((cmd_ln_access ("-s2stsegdir") == NULL) &&
	(cmd_ln_access ("-stsegdir") == NULL) &&
	(cmd_ln_access ("-phsegdir") == NULL) &&
	(cmd_ln_access ("-wdsegdir") == NULL) &&
	(cmd_ln_access ("-outsentfn") == NULL))
	E_FATAL("Missing output file/directory argument(s)\n");
    
    tm_utt = timing_new ();
    
    /*
     * Initialize log(S3-base).  All scores (probs...) computed in log domain to avoid
     * underflow.  At the same time, log base = 1.0001 (1+epsilon) to allow log values
     * to be maintained in int32 variables without significant loss of precision.
     */
    if (cmd_ln_access("-logbase") == NULL)
	logs3_init (1.0001);
    else {
	float32 logbase;
    
	logbase = *((float32 *) cmd_ln_access("-logbase"));
	if (logbase <= 1.0)
	    E_FATAL("Illegal log-base: %e; must be > 1.0\n", logbase);
	if (logbase > 1.1)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init ((float64) logbase);
    }

    /* Initialize feature stream type */
    fcb = feat_init ((char *) cmd_ln_access ("-feat"));
    
    /* Read in input databases */
    models_init ();
    
    senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    
    tmr_utt = cyctimer_new ("U");
    tmr_gauden = cyctimer_new ("G");
    tmr_senone = cyctimer_new ("S");
    tmr_align = cyctimer_new ("A");

    /* Initialize align module */
    align_init ();
    printf ("\n");
    
    tot_nfr = 0;
    
    process_ctlfile ();

    if (tot_nfr > 0) {
	printf ("\n");
	printf("TOTAL FRAMES:       %8d\n", tot_nfr);
	printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_cpu, tm_utt->t_tot_cpu/(tot_nfr*0.01));
	printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_elapsed, tm_utt->t_tot_elapsed/(tot_nfr*0.01));
    }

#if (! WIN32)
    system ("ps aguxwww | grep s3align");
#endif

    /* Hack!! To avoid hanging problem under Linux */
    if (logfp) {
	fclose (logfp);
	*stdout = orig_stdout;
	*stderr = orig_stderr;
    }
    
    exit(0);
}
