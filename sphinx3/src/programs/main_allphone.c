/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * 19-Jun-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to handle the new libfeat interface.
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

#include <libutil/libutil.h>
#include "feat.h"

#include "s3types.h"
#include "logs3.h"
#include "ms_gauden.h"
#include "ms_senone.h"

#ifdef INTERP
#include "interp.h"
#endif

#include "tmat.h"
#include "mdef.h"
#include "s3_allphone.h"
#include "agc.h"
#include "cmn.h"


static arg_t defn[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-mdef", 
      ARG_STRING,
      NULL,
      "Model definition input file: triphone -> senones/tmat tying" },
    { "-tmat",
      ARG_STRING,
      NULL,
      "Transition matrix input file" },
    { "-mean",
      ARG_STRING,
      NULL,
      "Mixture gaussian codebooks mean parameters input file" },
    { "-var",
      ARG_STRING,
      NULL,
      "Mixture gaussian codebooks variance parameters input file" },
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixw",
      ARG_STRING,
      NULL,
      "Senone mixture weights parameters input file" },
#ifdef INTERP
    { "-lambda",
      ARG_STRING,
      NULL,
      "Interpolation weights (CD/CI senone) parameters input file" },
#endif
    { "-tpfloor",
      ARG_FLOAT32,
      "0.0001",
      "Triphone state transition probability floor applied to -tmat file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Codebook variance floor applied to -var file" },
    { "-mwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Codebook mixture weight floor applied to -mixw file" },
    { "-agc",
      ARG_STRING,
      "max",
      "AGC.  max: C0 -= max(C0) in current utt; none: no AGC" },
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    { "-cmn",
      ARG_STRING,
      "current",
      "Cepstral mean norm.  current: C[1..n-1] -= mean(C[1..n-1]) in current utt; none: no CMN" },
    { "-varnorm",
      ARG_STRING,
      "no",
      "Variance normalize each utterance (yes/no; only applicable if CMN is also performed)" },
    { "-feat",	/* Captures the computation for converting input to feature vector */
      ARG_STRING,
      "1s_c_d_dd",
      "Feature stream: s2_4x / s3_1x39 / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
    { "-ctl",
      ARG_STRING,
      NULL,
      "Input control file listing utterances to be decoded" },
    { "-ctloffset",
      ARG_INT32,
      "0",
      "No. of utterances at the beginning of -ctl file to be skipped" },
    { "-ctlcount",
      ARG_INT32,
      NULL,
      "No. of utterances in -ctl file to be processed (after -ctloffset).  Default: Until EOF" },
    { "-cepdir",
      ARG_STRING,
      ".",
      "Directory for utterances in -ctl file (if relative paths specified)." },
    { "-cepext",
      ARG_STRING,
      "mfc",
      "File extension appended to utterances listed in -ctl file" },
    { "-topn",
      ARG_INT32,
      "4",
      "No. of top scoring densities computed in each mixture gaussian codebook" },
    { "-beam",
      ARG_FLOAT64,
      "1e-64",
      "Main pruning beam applied during search" },
    { "-phlatbeam",
      ARG_FLOAT64,
      "1e-20",
      "Pruning beam for writing phone lattice" },
    { "-phonetp",
      ARG_STRING,
      NULL,
      "Phone transition probabilities inputfile (default: flat probs)" },
    { "-phonetpfloor",
      ARG_FLOAT32,
      "0.00001",
      "Floor for phone transition probabilities" },
    { "-phonetpwt",
      ARG_FLOAT32,
      "3.0",
      "Weight (exponent) applied to phone transition probabilities" },
    { "-inspen",
      ARG_FLOAT32,
      "0.05",
      "Phone insertion penalty (applied above phone transition probabilities)" },
    { "-phsegdir",
      ARG_STRING,
      NULL,
      "Output directory for phone segmentation files; optionally end with ,CTL" },
    { "-phlatdir",
      ARG_STRING,
      NULL,
      "Output directory for phone lattice files" },
    { "-logfn",
      ARG_STRING,
      NULL,
      "Log file (default stdout/stderr)" },
    
    { NULL, ARG_INT32, NULL, NULL }
};



/*  The definition of mdef and tmat can be found in s3_allphone.c
*/

mdef_t *mdef;
tmat_t *tmat;

static gauden_t *g;		/* Gaussian density codebooks */
static senone_t *sen;		/* Senones */
#ifdef INTERP
static interp_t *interp;	/* CD/CI interpolation */
#endif


static feat_t *fcb;		/* Feature type descriptor (Feature Control Block) */
static float32 ***feat = NULL;	/* Speech feature data */

static int32 *senscale;		/* ALL senone scores scaled by senscale[i] in frame i */

/* For profiling/timing */
static int32 tot_nfr;
static ptmr_t tm_utt;
static ptmr_t tm_gausen;
static ptmr_t tm_allphone;


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    float32 varfloor, mixwfloor, tpfloor;
    int32 i;
    char *arg;
    
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdef"));

    /* Codebooks */
    varfloor = *((float32 *) cmd_ln_access("-varfloor"));
    g = gauden_init ((char *) cmd_ln_access("-mean"),
		     (char *) cmd_ln_access("-var"),
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
    sen = senone_init ((char *) cmd_ln_access("-mixw"),
		       (char *) cmd_ln_access("-senmgau"),
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

#ifdef INTERP
    /* CD/CI senone interpolation weights file, if present */
    if ((arg = (char *) cmd_ln_access ("-lambda")) != NULL) {
	interp = interp_init (arg);

	/* Verify interpolation weights size with senones */
	if (interp->n_sen != sen->n_sen)
	    E_FATAL("Interpolation file has %d weights; but #senone= %d\n",
		    interp->n_sen, sen->n_sen);
    } else
	interp = NULL;
#endif

    /* Transition matrices */
    tpfloor = *((float32 *) cmd_ln_access("-tpfloor"));
    tmat = tmat_init ((char *) cmd_ln_access("-tmat"), tpfloor);

    /* Verify transition matrices parameters against model definition parameters */
    if (mdef->n_tmat != tmat->n_tmat)
	E_FATAL("Model definition has %d tmat; but #tmat= %d\n",
		mdef->n_tmat, tmat->n_tmat);
    if (mdef->n_emit_state != tmat->n_state)
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
    FILE *fp = (FILE *)0;
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
	fflush(fp);
    }
    
    fprintf (fp, "\t%5s %5s %9s %s\n",
	     "SFrm", "EFrm", "SegAScr", "Phone");
    fflush(fp);

    uttscr = 0;
    for (; phseg; phseg = phseg->next) {
	/* Account for senone score scaling in each frame */
	scale = 0;
	for (f = phseg->sf; f <= phseg->ef; f++)
	    scale += senscale[f];
	
	if (! dir){
	    fprintf (fp, "ph:%s>", uttid);
	    fflush(fp);
	}
	fprintf (fp, "\t%5d %5d %9d %s\n",
		 phseg->sf, phseg->ef, phseg->score + scale,
		 mdef_ciphone_str (mdef, phseg->ci));
	fflush(fp);
	uttscr += (phseg->score + scale);
    }

    if (! dir){
	fprintf (fp, "PH:%s>", uttid);
	fflush(fp);
    }
    fprintf (fp, " Total score: %11d\n", uttscr);
    fflush(fp);
    if (dir)
	fclose (fp);
    else{
	fprintf (fp, "\n");
	fflush(fp);
    }
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
static void allphone_utt (int32 nfr, char *uttid)
{
    static int32 w;
    static int32 topn;
    static gauden_dist_t **dist;	/* Density values for one mgau in one frame */
    static int32 **senscr = NULL;	/* Senone scores for window of frames */
    static mgau2sen_t **mgau2sen;	/* Senones sharing mixture Gaussian codebooks */

    int32 i, j, k, s, gid, best;
    phseg_t *phseg;
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

    ptmr_reset (&tm_utt);
    ptmr_reset (&tm_gausen);
    ptmr_reset (&tm_allphone);

    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return;
    }
    ptmr_start (&tm_utt);

    allphone_start_utt (uttid);

    for (j = 0; j < nfr; j += GAUDEN_EVAL_WINDOW) {
	/* Compute Gaussian densities and senone scores for window of frames */
	ptmr_start (&tm_gausen);
	for (gid = 0; gid < g->n_mgau; gid++) {
	    for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
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
	
	/* Find best phone scores for each frame in window */
	for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
#ifdef INTERP
	  /* Interpolate senones for each frame in window */
	  if (interp)
	      interp_all (interp, senscr[k], mdef->cd2cisen, mdef->n_ci_sen);
#endif

	  /* Normalize senone scores */
	  best = (int32)0x80000000;
	  for (s = 0; s < sen->n_sen; s++)
	      if (best < senscr[k][s])
		  best = senscr[k][s];
	  for (s = 0; s < sen->n_sen; s++)
	      senscr[k][s] -= best;
	  senscale[i] = best;
      }
      ptmr_stop (&tm_gausen);

      /* Step search one frame forward */
      ptmr_start (&tm_allphone);
      for (i = j, k = 0; (k < GAUDEN_EVAL_WINDOW) && (i < nfr); i++, k++) {
	  allphone_frame (senscr[k]);
	  if ((i%10) == 9) {
	      printf ("."); fflush (stdout);
	  }
      }
      ptmr_stop (&tm_allphone);
  }
  printf ("\n");
  
  phseg = allphone_end_utt (uttid);
  write_phseg ((char *) cmd_ln_access ("-phsegdir"), uttid, phseg);
  
  ptmr_stop (&tm_utt);
  
  printf ("%s: TMR:[frm %5d]", uttid, nfr);
  printf ("[el %6.2fx]", tm_utt.t_elapsed * 100.0 / nfr);
  printf ("[cpu %6.2fx]", tm_utt.t_cpu * 100.0 / nfr);
  if (tm_utt.t_cpu > 0.0) {
      printf ("[gau+sen %6.2fx %2d%%]", tm_gausen.t_cpu * 100.0 / nfr,
	      (int32) ((tm_gausen.t_cpu * 100.0) / tm_utt.t_cpu));
      printf ("[srch %6.2fx %2d%%]", tm_allphone.t_cpu * 100.0 / nfr,
	      (int32) ((tm_allphone.t_cpu * 100.0) / tm_utt.t_cpu));
  }
  printf ("\n");
  fflush (stdout);
}


/* Process utterances in the control file (-ctl argument) */
static void process_ctlfile ( void )
{
  FILE *ctlfp;
  char *ctlfile, *cepdir, *cepext;
  char line[1024], cepfile[1024], ctlspec[1024];
  int32 ctloffset, ctlcount, sf, ef, nfr;
  char uttid[1024];
  int32 k,i;
  
  ctlfile = (char *) cmd_ln_access("-ctl");
  if ((ctlfp = fopen (ctlfile, "r")) == NULL)
      E_FATAL("fopen(%s,r) failed\n", ctlfile);
  
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
      if (! feat) 
	  feat = feat_array_alloc (fcb, S3_MAX_FRAMES);

      nfr = feat_s2mfc2feat(fcb, ctlspec, cepdir, sf, ef, feat, S3_MAX_FRAMES);

      if (nfr <= 0)
	  E_ERROR("Utt %s: Input file read (%s) failed\n", uttid, cepfile);
      else {
	  E_INFO ("%s: %d input frames\n", uttid, nfr);
	  allphone_utt (nfr, uttid);
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

int
main (int32 argc, char *argv[])
{
    /*  kb_t kb;
      ptmr_t tm;*/

  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);
  unlimit ();
    
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
    fcb = feat_init ( (char *) cmd_ln_access ("-feat"),
		      (char *) cmd_ln_access ("-cmn"),
		      (char *) cmd_ln_access ("-varnorm"),
		      (char *) cmd_ln_access ("-agc"));
    
    /* Read in input databases */
    models_init ();
    
    /* Senone scaling factor in each frame */
    senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    
    /* Initialize allphone decoder module */
    allphone_init (mdef, tmat);
    printf ("\n");
    
    tot_nfr = 0;
    
    process_ctlfile ();

    if (tot_nfr > 0) {
	printf ("\n");
	printf("TOTAL FRAMES:       %8d\n", tot_nfr);
	printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	       tm_utt.t_tot_cpu, tm_utt.t_tot_cpu/(tot_nfr*0.01));
	printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	       tm_utt.t_tot_elapsed, tm_utt.t_tot_elapsed/(tot_nfr*0.01));
    }

#if (! WIN32)
    system ("ps aguxwww | grep s3allphone");
#endif

    cmd_ln_appl_exit();
    
    return 0;
}
