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
 * main_allphone.c -- Main driver routine for allphone Viterbi decoding.
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
 * Revision 1.13.4.1  2005/07/18  23:21:23  arthchan2003
 * Tied command-line arguments with marcos
 * 
 * Revision 1.13  2005/06/22 05:37:45  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init
 *
 * Revision 1.8  2005/06/19 04:51:48  archan
 * Add multi-class MLLR support for align, decode_anytopo as well as allphone.
 *
 * Revision 1.7  2005/06/17 23:46:06  archan
 * Sphinx3 to s3.generic 1, Remove bogus log messages in align and allphone, 2, Unified the logbase value from 1.0001 to 1.0003
 *
 * Revision 1.6  2005/06/03 06:12:57  archan
 * 1, Simplify and unify all call of logs3_init, move warning when logbase > 1.1 into logs3.h.  2, Change arguments to require arguments in align and astar.
 *
 * Revision 1.5  2005/05/27 01:15:45  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.4  2005/04/21 23:50:27  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.2  2005/03/30 00:43:41  archan
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

#include <s3types.h>

#include "feat.h"
#include "logs3.h"
#include "ms_mllr.h"
#include "ms_gauden.h"
#include "ms_senone.h"
#include "cb2mllr_io.h"


#ifdef INTERP
#include "interp.h"
#endif

#include "tmat.h"
#include "mdef.h"
#include "s3_allphone.h"
#include "agc.h"
#include "cmn.h"
#include "cmdln_macro.h"

/** \file main_allphone.c
 * \brief  Main driver routine for allphone Viterbi decoding
 */
static arg_t defn[] = {
  cepstral_to_feature_command_line_macro()
  log_table_command_line_macro()
  acoustic_model_command_line_macro()
  speaker_adaptation_command_line_macro()
  common_application_properties_command_line_macro()
  control_file_handling_command_line_macro()
  hypothesis_file_handling_command_line_macro()
  control_mllr_file_command_line_macro()
  cepstral_input_handling_command_line_macro()

#ifdef INTERP
    { "-lambda",
      ARG_STRING,
      NULL,
      "Interpolation weights (CD/CI senone) parameters input file" },
#endif
    { "-topn",
      ARG_INT32,
      "4",
      "No. of top scoring densities computed in each mixture gaussian codebook" },
    { "-beam",
      ARG_FLOAT64,
      "1e-64",
      "Main pruning beam applied during search" },
    { "-wip",
      ARG_FLOAT32,
      "0.05",
      "Phone insertion penalty (applied above phone transition probabilities)" },

  /* allphone-specific arguments */
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
    { "-phsegdir",
      ARG_STRING,
      NULL,
      "Output directory for phone segmentation files; optionally end with ,CTL" },
    { "-phlatbeam",
      ARG_FLOAT64,
      "1e-20",
      "Pruning beam for writing phone lattice" },
    { "-phlatdir",
      ARG_STRING,
      NULL,
      "Output directory for phone lattice files" },
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
    mdef = mdef_init ((char *) cmd_ln_access("-mdef"),1);

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
    mixwfloor = *((float32 *) cmd_ln_access("-mixwfloor"));
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
    tpfloor = *((float32 *) cmd_ln_access("-tmatfloor"));
    tmat = tmat_init ((char *) cmd_ln_access("-tmat"), tpfloor,1);

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


static int32 model_set_mllr(const char *mllrfile, const char *cb2mllrfile)
{
    float32 ****A, ***B;
    int32 *cb2mllr;
    int32 gid, sid, nclass;
    uint8 *mgau_xform;
		
    gauden_mean_reload (g, (char *) cmd_ln_access("-mean"));
		
    if (ms_mllr_read_regmat (mllrfile, &A, &B,
			     fcb->stream_len, feat_n_stream(fcb),
			     &nclass) < 0)
	E_FATAL("ms_mllr_read_regmat failed\n");

    if (cb2mllrfile && strcmp(cb2mllrfile, ".1cls.") != 0) {
	int32 ncb, nmllr;

	cb2mllr_read(cb2mllrfile,
		     &cb2mllr,
		     &ncb, &nmllr);
	if (nmllr != nclass)
	    E_FATAL("Number of classes in cb2mllr does not match mllr (%d != %d)\n",
		    ncb, nclass);
	if (ncb != sen->n_sen)
	    E_FATAL("Number of senones in cb2mllr does not match mdef (%d != %d)\n",
		    ncb, sen->n_sen);
    }
    else
	cb2mllr = NULL;

		
    mgau_xform = (uint8 *) ckd_calloc (g->n_mgau, sizeof(uint8));

    /* Transform each non-CI mixture Gaussian */
    for (sid = 0; sid < sen->n_sen; sid++) {
	int32 class = 0;

	if (cb2mllr)
	    class = cb2mllr[sid];
	if (class == -1)
	    continue;

	if (mdef->cd2cisen[sid] != sid) {	/* Otherwise it's a CI senone */
	    gid = sen->mgau[sid];
	    if (! mgau_xform[gid]) {
		ms_mllr_norm_mgau (g->mean[gid], g->n_density, A, B,
				   fcb->stream_len, feat_n_stream(fcb),
				   class);
		mgau_xform[gid] = 1;
	    }
	}
    }

    ckd_free (mgau_xform);
		
    ms_mllr_free_regmat (A, B, feat_n_stream(fcb));
    ckd_free(cb2mllr);

    return S3_SUCCESS;
}


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
  FILE *ctlfp, *mllrctlfp;
  char *ctlfile, *cepdir, *cepext, *mllrctlfile;
  char line[1024], ctlspec[1024];
  int32 ctloffset, ctlcount, sf, ef, nfr;

  char mllrfile[4096], cb2mllrfile[4096], prevmllr[4096]; 
  char uttid[1024];
  int32 k,i;
  
  ctlfile = (char *) cmd_ln_access("-ctl");
  if ((ctlfp = fopen (ctlfile, "r")) == NULL)
      E_FATAL("fopen(%s,r) failed\n", ctlfile);
  
  if ((mllrctlfile = (char *) cmd_ln_access("-ctl_mllr")) != NULL) {
    if ((mllrctlfp = fopen (mllrctlfile, "r")) == NULL)
      E_FATAL("fopen(%s,r) failed\n", mllrctlfile);
  } else
    mllrctlfp = NULL;
  prevmllr[0] = '\0';
  
  if (cmd_ln_access("-mllr") != NULL) {
    model_set_mllr(cmd_ln_access("-mllr"), cmd_ln_access("-cb2mllr"));
    strcpy(prevmllr, cmd_ln_access("-mllr"));
  }

  E_INFO("Processing ctl file %s\n", ctlfile);
  
  cepdir = (char *) cmd_ln_access("-cepdir");
  cepext = (char *) cmd_ln_access("-cepext");
  assert (cepext != NULL);
  
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

      if (mllrctlfp) {
	int32 tmp1, tmp2;
	
	if ((k = fscanf (mllrctlfp, "%s %d %d %s", mllrfile,
			 &tmp1, &tmp2, cb2mllrfile)) <= 0)
	  E_FATAL ("Unexpected EOF(%s)\n", mllrctlfile);
	if (!(k == 1) || (k == 4))
	  E_FATAL ("Expected MLLR file or MLLR, two ints, and cb2mllr (%s)\n",
		   mllrctlfile);
	if (k == 1)
	  strcpy(cb2mllrfile, ".1cls.");
	
	if (strcmp (prevmllr, mllrfile) != 0) {
	  model_set_mllr(mllrfile, cb2mllrfile);
	  strcpy (prevmllr, mllrfile);
	}
      }

      if (! feat) 
	  feat = feat_array_alloc (fcb, S3_MAX_FRAMES);

      nfr = feat_s2mfc2feat(fcb, ctlspec, cepdir, cepext, sf, ef, feat, S3_MAX_FRAMES);

      if (nfr <= 0){
	if (cepdir != NULL) {
	  E_ERROR("Utt %s: Input file read (%s) with dir (%s) and extension (%s) failed \n", 
		  uttid, ctlspec, cepdir, cepext);
	} else {
	  E_ERROR("Utt %s: Input file read (%s) with extension (%s) failed \n", uttid, ctlspec, cepext);
	}
      }
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

    if (mllrctlfp)
	fclose (mllrctlfp);
}

int
main (int32 argc, char *argv[])
{
    /*  kb_t kb;
      ptmr_t tm;*/

  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);
  unlimit ();
    
  logs3_init ((float64) cmd_ln_float32("-logbase"),1,cmd_ln_int32("-log3table"));

  /* Initialize feature stream type */
  fcb = feat_init ( (char *) cmd_ln_access ("-feat"),
		    (char *) cmd_ln_access ("-cmn"),
		    (char *) cmd_ln_access ("-varnorm"),
		    (char *) cmd_ln_access ("-agc"),
		    1);
    
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
