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
 * Revision 1.16  2006/02/24  04:38:04  arthchan2003
 * Merged Dave's change and my changes: started to use macros.  use Dave's change on -hyp and -hypseg. Used ctl_process.  Still need test.
 * 
 *
 * Revision 1.15  2006/02/07 20:51:33  dhdfu
 * Add -hyp and -hypseg arguments to allphone so we can calculate phoneme
 * error rate in a straightforward way.
 *
 * Revision 1.14  2006/02/02 22:56:07  dhdfu
 * Add ARPA language model support to allphone
 *
 * Revision 1.13.4.10  2005/09/26 02:28:26  arthchan2003
 * Changed -s3hmmdir to -hmm
 *
 * Revision 1.13.4.9  2005/09/11 02:54:19  arthchan2003
 * Remove s3_dag.c and s3_dag.h, all functions are now merged into dag.c and shared by decode_anytopo and dag.
 *
 * Revision 1.13.4.8  2005/08/03 20:01:32  arthchan2003
 * Added the -topn argument into acoustic_model_command_line_macro
 *
 * Revision 1.13.4.7  2005/08/03 18:55:03  dhdfu
 * Remove bogus initialization of ms_mgau's internals from here
 *
 * Revision 1.13.4.6  2005/08/02 21:42:33  arthchan2003
 * 1, Moved static variables from function level to the application level. 2, united all initialization of HMM using s3_am_init, 3 united all GMM computation using ms_cont_mgau_frame_eval.
 *
 * Revision 1.13.4.5  2005/07/27 23:23:39  arthchan2003
 * Removed process_ctl in allphone, dag, decode_anytopo and astar. They were duplicated with ctl_process and make Dave and my lives very miserable.  Now all application will provided their own utt_decode style function and will pass ctl_process.  In that way, the mechanism of reading would not be repeated. livepretend also follow the same mechanism now.  align is still not yet finished because it read yet another thing which has not been considered : transcription.
 *
 * Revision 1.13.4.4  2005/07/24 19:37:19  arthchan2003
 * Removed GAUDEN_EVAL_WINDOW, put it in srch.h now.
 *
 * Revision 1.13.4.3  2005/07/22 03:46:55  arthchan2003
 * 1, cleaned up the code, 2, fixed dox-doc. 3, use srch.c version of log_hypstr and log_hyp_detailed.
 *
 * Revision 1.13.4.2  2005/07/20 21:25:42  arthchan2003
 * Shared to code of Multi-stream GMM initialization in align/allphone and decode_anytopo.
 *
 * Revision 1.13.4.1  2005/07/18 23:21:23  arthchan2003
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
#include "ms_mgau.h"
#include "cb2mllr_io.h"
#include "srch.h"
#include "corpus.h"

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

    { "-lambda",
      ARG_STRING,
      NULL,
      "Interpolation weights (CD/CI senone) parameters input file" },
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

static kbcore_t *kbc;   /* a kbcore */
static ascr_t *ascr;    /* An acoustic score structure.  */

lm_t *lm;

static feat_t *fcb;		/* Feature type descriptor (Feature Control Block) */
static float32 ***feat = NULL;	/* Speech feature data */

static int32 *senscale;		/* ALL senone scores scaled by senscale[i] in frame i */

/* For profiling/timing */
static int32 tot_nfr;
static ptmr_t tm_utt;
static ptmr_t tm_gausen;
static ptmr_t tm_allphone;

/* File handles for match and hypseg files */
static FILE *matchfp, *matchsegfp;


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    int32 i;
    gauden_t* g;
    senone_t* sen;
    ms_mgau_model_t *msg;
    char str[10];
    int32 cisencnt;

    logs3_init ((float64) cmd_ln_float32("-logbase"),1,cmd_ln_int32("-log3table"));

    /* Initialize feature stream type */
    fcb = feat_init ( (char *) cmd_ln_access ("-feat"),
		      (char *) cmd_ln_access ("-cmn"),
		      (char *) cmd_ln_access ("-varnorm"),
		      (char *) cmd_ln_access ("-agc"),
		      1);

    kbc=New_kbcore();

    /** Temporarily used .s3cont. instead of .cont. when in s3.0 family of tool. 
	Then no need for changing the default command-line. 
     */

    if(strcmp(cmd_ln_str("-senmgau"),".cont.")==0){
      strcpy(str,".s3cont.");
    }else if(strcmp(cmd_ln_str("-senmgau"),".semi.")==0){
      strcpy(str,".semi.");
    }else if(strcmp(cmd_ln_str("-senmgau"),".s3cont.")==0){
      strcpy(str,".s3cont.");
    }

    s3_am_init(kbc,
	       cmd_ln_str("-hmm"),
	       cmd_ln_str("-mdef"),
	       cmd_ln_str("-mean"),
	       cmd_ln_str("-var"),
	       cmd_ln_float32("-varfloor"),
	       cmd_ln_str("-mixw"),
	       cmd_ln_float32("-mixwfloor"),
	       cmd_ln_str("-tmat"),
	       cmd_ln_float32("-tmatfloor"),
	       str, 
	       cmd_ln_str("-lambda"),
	       cmd_ln_int32("-topn")
	       );


    msg=kbcore_ms_mgau(kbc);
    assert(msg);    
    assert(msg->g);    
    assert(msg->s);

    g=ms_mgau_gauden(msg);
    sen=ms_mgau_senone(msg);

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

    for(cisencnt=0;cisencnt==kbc->mdef->cd2cisen[cisencnt];cisencnt++) ;


    /* Language model, if any. */
    if (cmd_ln_access("-lm")) {
	    if ((lm = lm_read(cmd_ln_str("-lm"),
			      cmd_ln_float32("-phonetpwt"),
			      cmd_ln_float32("-wip"),
			      cmd_ln_float32("-uw"))) == NULL)
		    E_FATAL("Failed to read language model from %s\n", cmd_ln_str("-lm"));
    }

    ascr=ascr_init(kbc->mdef->n_sen,
		   0, /* No composite senone */
		   mdef_n_sseq(kbc->mdef),
		   0, /* No composite senone sequence */
		   1, /* Phoneme lookahead window =1. Not enabled phoneme lookahead and CIGMMS at this moment */
		   cisencnt);
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
		 mdef_ciphone_str (kbc->mdef, phseg->ci));
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

/* FIX ME! Should only consider active senone from every frame in the search */
static void allphone_sen_active (int32 *senlist, int32 n_sen)
{
  int32 sen;
    
  for (sen = 0; sen < n_sen; sen++)
    senlist[sen] = 1;
}

/*
 * Find Viterbi allphone decoding.
 */
static void allphone_utt (int32 nfr, char *uttid)
{
    int32 i;
    phseg_t *phseg;
    int32 topn;
    int32 w;
    ms_mgau_model_t *msg;        /* Multi-stream multi mixture Gaussian */

    msg=kbcore_ms_mgau(kbc);
    topn=ms_mgau_topn(msg);
    w = feat_window_size (fcb);	/* #MFC vectors needed on either side of current
				   frame to compute one feature vector */

    ptmr_reset (&tm_utt);
    ptmr_reset (&tm_gausen);
    ptmr_reset (&tm_allphone);

    if (nfr <= (w<<1)) {
	E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid, (w<<1)+1, nfr);
	return;
    }
    ptmr_start (&tm_utt);

    allphone_start_utt (uttid);

#if 0
    /* Also see the old implementation at the bottom of the file */

#endif

#if 1
    for(i = 0 ; i < nfr ; i++){
      ptmr_start(&tm_gausen);
      allphone_sen_active(ascr->sen_active,ascr->n_sen);
      senscale[i]=ms_cont_mgau_frame_eval(ascr,
					  msg,
					  kbc->mdef,
					  feat[i]);
      ptmr_stop (&tm_gausen);

      ptmr_start (&tm_allphone);
      allphone_frame (ascr->senscr);
      if ((i%10) == 9) {
	printf ("."); fflush (stdout);
      }
      ptmr_stop (&tm_allphone);

    }

    printf ("\n");
#endif

  
  phseg = allphone_end_utt (uttid);
  write_phseg ((char *) cmd_ln_access ("-phsegdir"), uttid, phseg);
  /* Log recognition output to the standard match and matchseg files */
  if (matchfp)
    log_hypstr (matchfp, phseg, uttid);
  for (h = phseg; h; h = h->next) {
    ascr += h->score;
    lscr += h->tscore;
  }
  if (matchsegfp)
    log_hypseg (uttid, matchsegfp, phseg, nfr, scl);
  
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

static void utt_allphone(void *data, utt_res_t *ur, int32 sf, int32 ef, char *uttid)
{
  int32 nfr;
  char *cepdir, *cepext;

	/* FIX ME: merging is incomplete for this part. */
  ctlfile = (char *) cmd_ln_access("-ctl");
  if ((ctlfp = fopen (ctlfile, "r")) == NULL)
      E_FATAL("fopen(%s,r) failed\n", ctlfile);
  if ((matchfile = (char *) cmd_ln_access("-hyp")) == NULL) {
    matchfp = NULL;
  } else {
    if ((matchfp = fopen (matchfile, "w")) == NULL)
      E_ERROR("fopen(%s,w) failed\n", matchfile);
  }
  if ((matchsegfile = (char *) cmd_ln_access("-hypseg")) == NULL) {
    matchsegfp = NULL;
  } else {
    if ((matchsegfp = fopen (matchsegfile, "w")) == NULL)
      E_ERROR("fopen(%s,w) failed\n", matchsegfile);
  }
  
  if ((mllrctlfile = (char *) cmd_ln_access("-mllrctl")) != NULL) {
    if ((mllrctlfp = fopen (mllrctlfile, "r")) == NULL)
      E_FATAL("fopen(%s,r) failed\n", mllrctlfile);
  } else
    mllrctlfp = NULL;
  prevmllr[0] = '\0';
  
  if (cmd_ln_access("-mllr") != NULL) {
    model_set_mllr(cmd_ln_access("-mllr"), cmd_ln_access("-cb2mllr"));
    strcpy(prevmllr, cmd_ln_access("-mllr"));
  }

  cepdir=cmd_ln_str("-cepdir");
  cepext=cmd_ln_str("-cepext");

  nfr = feat_s2mfc2feat(fcb, ur->uttfile, cepdir, cepext, sf, ef, feat, S3_MAX_FRAMES);

  assert(kbc->ms_mgau);
  if(ur->regmatname) model_set_mllr(kbc->ms_mgau,ur->regmatname, ur->cb2mllrname,fcb,kbc->mdef);
  
  if (nfr <= 0){
    if (cepdir != NULL) {
      E_ERROR("Utt %s: Input file read (%s) with dir (%s) and extension (%s) failed \n", 
	      uttid, ur->uttfile, cepdir, cepext);
    } else {
      E_ERROR("Utt %s: Input file read (%s) with extension (%s) failed \n", uttid, ur->uttfile, cepext);
    }
  }
  else {
    E_INFO ("%s: %d input frames\n", uttid, nfr);
    allphone_utt (nfr, uttid);
  }

}

int
main (int32 argc, char *argv[])
{
  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);
  unlimit ();

  /* Read in input databases */
  models_init ();
  
  /* Senone scaling factor in each frame */
  senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
  feat = feat_array_alloc (fcb, S3_MAX_FRAMES);
    
  /* Initialize allphone decoder module */
  allphone_init (kbc->mdef, kbc->tmat);
  printf ("\n");
  
  assert(kbc->ms_mgau);
  if (cmd_ln_access("-mllr") != NULL) 
    model_set_mllr(kbc->ms_mgau,cmd_ln_access("-mllr"), cmd_ln_access("-cb2mllr"),fcb,kbc->mdef);

  tot_nfr = 0;

  if (cmd_ln_str ("-ctl")) {
    /* When -ctlfile is speicified, corpus.c will look at -ctl_mllr to get
       the corresponding  MLLR for the utterance */
    ctl_process (cmd_ln_str("-ctl"),
		 NULL,
		 cmd_ln_str("-ctl_mllr"),
		 cmd_ln_int32("-ctloffset"),
		 cmd_ln_int32("-ctlcount"),
		 utt_allphone, 
		 NULL);
  } else {
      /* Is error checking good enough?" */
      E_FATAL(" -ctl are not specified.\n");
      
  }

  
  if (tot_nfr > 0) {
    printf ("\n");
    printf("TOTAL FRAMES:       %8d\n", tot_nfr);
    printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	   tm_utt.t_tot_cpu, tm_utt.t_tot_cpu/(tot_nfr*0.01));
    printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	   tm_utt.t_tot_elapsed, tm_utt.t_tot_elapsed/(tot_nfr*0.01));
  }

  if(ascr){
    ascr_free(ascr);
  }

  
#if (! WIN32)
  system ("ps aguxwww | grep s3allphone");
#endif

  cmd_ln_appl_exit();
    
  return 0;
}

