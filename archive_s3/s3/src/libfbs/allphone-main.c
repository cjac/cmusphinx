/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * allphone-main.c -- Main driver routine for allphone Viterbi decoding.
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
 * 02-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added allphone lattice output.
 * 
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 * 
 * 15-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started
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
#include "allphone.h"
#include "agc.h"
#include "cmn.h"


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
    { "-varnorm",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "no",
      "Variance Normalization Flag" },
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
    { "-ceplen",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "13",
      "Length of input feature vector" },
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
    { "-topn",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "4",
      "No. of top scoring densities computed in each mixture gaussian codebook" },
    { "-beam",
      CMD_LN_FLOAT64,
      CMD_LN_NO_VALIDATION,
      "1e-64",
      "Main pruning beam applied during search" },
    { "-phlatbeam",
      CMD_LN_FLOAT64,
      CMD_LN_NO_VALIDATION,
      "1e-20",
      "Pruning beam for writing phone lattice" },
    { "-phonetpfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Phone transition probabilities inputfile (default: flat probs)" },
    { "-phonetpfloor",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.00001",
      "Floor for phone transition probabilities" },
    { "-phonetpwt",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "3.0",
      "Weight (exponent) applied to phone transition probabilities" },
    { "-inspen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.05",
      "Phone insertion penalty (applied above phone transition probabilities)" },
    { "-phsegdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output directory for phone segmentation files; optionally end with ,CTL" },
    { "-phlatdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Output directory for phone lattice files" },
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

static int32 n_feat, *featlen;	/* #features, vector-length/feature */
static int32 cepsize;		/* Input mfc vector size (C0..C<cepsize-1>) */

static int32 *senscale;		/* ALL senone scores scaled by senscale[i] in frame i */

/* For profiling/timing */
static int32 tot_nfr;
static timing_t *tm_utt;
static timing_t *tm_gausen;
static timing_t *tm_allphone;


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    float32 varfloor, mixwfloor, tpfloor;
    int32 i;
    char *arg;
    
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdeffn"));

    /* Codebooks */
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


/* Write phone segmentation output file */
static void write_phseg (char *dir, char *uttid, phseg_t *phseg)
{
    char str[1024];
    FILE *fp;
    int32 uttscr, f, scale;
    
    /* Attempt to write segmentation for this utt to a separate file */
    if (dir) {
	sprintf (str, "%s/%s.allp", dir, uttid);
	E_INFO("Writing phone segmentation to: %s\n", str);
	if ((fp = fopen (str, "w")) == NULL) {
	    E_ERROR("fopen(%s,w) failed\n", str);
	    dir = NULL;	/* Flag to indicate fp shouldn't be closed at the end */
	}
    }
    
    if (! dir) {
	fp = stdout;	/* Segmentations can be directed to stdout this way */
	E_INFO ("Phone segmentation (%s):\n", uttid);
	fprintf (fp, "PH:%s>", uttid);
    }
    
    fprintf (fp, "\t%5s %5s %9s %s\n",
	     "SFrm", "EFrm", "SegAScr", "Phone");
    
    uttscr = 0;
    for (; phseg; phseg = phseg->next) {
	/* Account for senone score scaling in each frame */
	scale = 0;
	for (f = phseg->sf; f <= phseg->ef; f++)
	    scale += senscale[f];
	
	if (! dir)
	    fprintf (fp, "ph:%s>", uttid);
	fprintf (fp, "\t%5d %5d %9d %s\n",
		 phseg->sf, phseg->ef, phseg->score + scale,
		 mdef_ciphone_str (mdef, phseg->ci));
	
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


#define GAUDEN_EVAL_WINDOW	8

/* Lists of senones sharing each mixture Gaussian codebook */
typedef struct mgau2sen_s {
    s3senid_t sen;		/* Senone shared by this mixture Gaussian */
    struct mgau2sen_s *next;	/* Next entry in list for this mixture Gaussian */
} mgau2sen_t;


/*
 * Find Viterbi allphone decoding.
 */
static void allphone_utt (float32 **mfc, int32 nfr, char *uttid)
{
    static float32 **feat = NULL;
    static int32 w;
    static int32 topn;
    static gauden_dist_t **dist;	/* Density values for one mgau in one frame */
    static int32 **senscr;		/* Senone scores for window of frames */
    static mgau2sen_t **mgau2sen;	/* Senones sharing mixture Gaussian codebooks */

    int32 i, j, k, s, gid, best;
    char *arg;
    phseg_t *phseg;
    mgau2sen_t *m2s;

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
	    E_ERROR("-topn argument (%d) > #density codewords (%d); set to latter\n",
		   topn, g->n_density);
	    topn = g->n_density;
	}
	dist = (gauden_dist_t **) ckd_calloc_2d (n_feat, topn, sizeof(gauden_dist_t));
	
	/* Space for one frame of senone scores, and per frame active flags */
	senscr = (int32 **) ckd_calloc_2d (GAUDEN_EVAL_WINDOW, sen->n_sen, sizeof(int32));
	
	/* Initialize mapping from mixture Gaussian to senones */
	mgau2sen = (mgau2sen_t **) ckd_calloc (g->n_mgau, sizeof(mgau2sen_t *));
	for (s = 0; s < sen->n_sen; s++) {
	    m2s = (mgau2sen_t *) listelem_alloc (sizeof(mgau2sen_t));
	    m2s->sen = s;
	    m2s->next = mgau2sen[sen->mgau[s]];
	    mgau2sen[sen->mgau[s]] = m2s;
	}
    }

    timing_reset (tm_utt);
    timing_reset (tm_gausen);
    timing_reset (tm_allphone);
    counter_reset_all ();

    timing_start (tm_utt);
    
    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return;
    }
    
    /* AGC and CMN */
    arg = (char *) cmd_ln_access ("-cmn");
    if (strcmp (arg, "current") == 0)
	norm_mean (mfc, nfr, cepsize);
    arg = (char *) cmd_ln_access ("-agc");
    if (strcmp (arg, "max") == 0)
	agc_max (mfc, nfr);
    
    allphone_start_utt (uttid);

    /*
     * A feature vector for frame f depends on input MFC vectors [f-w..f+w].  Hence
     * the feature vector corresponding to the first w and last w input frames is
     * undefined.  We define them by simply replicating the first and last true
     * feature vectors (presumably silence regions).
     */
    for (j = 0; j < nfr; j += GAUDEN_EVAL_WINDOW) {
	/* Compute Gaussian densities and senone scores for window of frames */
	timing_start (tm_gausen);
	for (gid = 0; gid < g->n_mgau; gid++) {
	    for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
		/* Compute feature vector for current frame from input speech cepstra */
		if (i < w)
		    feat_cep2feat (mfc+w, feat);
		else if (i >= nfr-w)
		    feat_cep2feat (mfc+(nfr-w-1), feat);
		else
		    feat_cep2feat (mfc+i, feat);
		
		/* Evaluate mixture Gaussian densities */
		gauden_dist (g, gid, topn, feat, dist);
		
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
	
	/* Find best phone scores for each frame in window */
	for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
	    /* Interpolate senones for each frame in window */
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

	/* Step search one frame forward */
	timing_start (tm_allphone);
	for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
	    allphone_frame (senscr[k]);
	    if ((i%10) == 9) {
		printf ("."); fflush (stdout);
	    }
	}
	timing_stop (tm_allphone);
    }
    printf ("\n");
    
    phseg = allphone_end_utt (uttid);
    write_phseg ((char *) cmd_ln_access ("-phsegdir"), uttid, phseg);
    
    timing_stop (tm_utt);
    
    printf ("%s: TMR:[frm %5d]", uttid, nfr);
    printf ("[el %6.2fx]", tm_utt->t_elapsed * 100.0 / nfr);
    printf ("[cpu %6.2fx]", tm_utt->t_cpu * 100.0 / nfr);
    if (tm_utt->t_cpu > 0.0) {
	printf ("[gau+sen %6.2fx %2d%%]", tm_gausen->t_cpu * 100.0 / nfr,
		(int32) ((tm_gausen->t_cpu * 100.0) / tm_utt->t_cpu));
	printf ("[srch %6.2fx %2d%%]", tm_allphone->t_cpu * 100.0 / nfr,
		(int32) ((tm_allphone->t_cpu * 100.0) / tm_utt->t_cpu));
    }
    printf ("\n");
    fflush (stdout);
}


/* Process utterances in the control file (-ctlfn argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp;
    char *ctlfile, *cepdir, *cepext;
    char line[1024], cepfile[1024], ctlspec[1024];
/* CHANGE BY BHIKSHA: ADDED veclen AS A VARIABLE, 6 JAN 98 */
    int32 ctloffset, ctlcount, veclen, sf, ef, nfr;
/* END OF CHANGES BY BHIKSHA */
    char uttid[1024];
    int32 i, k;
    float32 **mfc;
    
    ctlfile = (char *) cmd_ln_access("-ctlfn");
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    cepdir = (char *) cmd_ln_access("-cepdir");
    cepext = (char *) cmd_ln_access("-cepext");
    assert ((cepdir != NULL) && (cepext != NULL));
/* BHIKSHA: ADDING VECLEN TO ALLOW VECTORS OF DIFFERENT SIZES */
    veclen = *((int32 *) cmd_ln_access("-ceplen"));
/* END CHANGES, 6 JAN 1998, BHIKSHA */
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    if (! cmd_ln_access("-ctlcount"))
	ctlcount = 0x7fffffff;	/* All entries processed if no count specified */
    else
	ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));
    if (ctlcount == 0) {
	E_INFO("-ctlcount argument = 0!!\n");
	fclose (ctlfp);
	return;
    }
    
    /* Skipping initial offset */
    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0)
	    --ctloffset;
    }

    /* Process the specified number of utterance or until end of control file */
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

	if (ctlspec[0] != '/')
	    sprintf (cepfile, "%s/%s.%s", cepdir, ctlspec, cepext);
	else
	    sprintf (cepfile, "%s.%s", ctlspec, cepext);
	
	/* Read and process mfc file */
/* CHANGE BY BHIKSHA; PASSING VECLEN TO s2mfc_read(), 6 JAN 98 */
	/* Read mfc file */
	if ((nfr = s2mfc_read (cepfile, sf, ef, &mfc, veclen)) <= 0) 
	    E_ERROR("Utt %s: MFC file read (%s) failed\n", uttid, cepfile);
/* END CHANGES BY BHIKSHA */
	else {
	    E_INFO ("%d mfc frames\n", nfr);
	    allphone_utt (mfc, nfr, uttid);
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
	str = "s3allphone.arg";
	E_INFO("Default argument file: %s\n", str);
    }
    if (str) {
	/* Build command line argument list from file */
	if ((argc = load_argfile (str, argv[0], &argv)) < 0) {
	    fprintf (stderr, "Usage:\n");
	    fprintf (stderr, "\t%s argument-list, or\n", argv[0]);
	    fprintf (stderr, "\t%s [argument-file] (default file: s3allphone.arg)\n\n",
		     argv[0]);
	    cmd_ln_print_definitions();
	    exit(1);
	}
    }
    
    cmdline_parse (argc, argv);

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
	if (logbase > 1.1)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init ((float64) logbase);
    }

    /* Initialize feature stream type */
    feat_init ((char *) cmd_ln_access ("-feat"));
/* BHIKSHA: PASS CEPSIZE TO FEAT_CEPSIZE, 6 Jan 98 */
    cepsize = *((int32 *) cmd_ln_access("-ceplen"));
    cepsize = feat_cepsize (cepsize);
/* END CHANGES BY BHIKSHA */
    
    /* Read in input databases */
    models_init ();
    
    /* Senone scaling factor in each frame */
    senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    
    /* Allocate profiling timers and counters */
    tm_utt = timing_new ();
    tm_gausen = timing_new ();
    tm_allphone = timing_new ();

    /* Initialize allphone decoder module */
    allphone_init ();
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
    system ("ps aguxwww | grep s3allphone");
#endif

    /* Hack!! To avoid hanging problem under Linux */
    if (logfp) {
	fclose (logfp);
	*stdout = orig_stdout;
	*stderr = orig_stderr;
    }
    
    exit(0);
}
