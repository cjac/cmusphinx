/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * decode.c --  
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
 */

/** \file analysis.c
 * \brief main function for analyse the performance of individual parts of the decoder
 */
#include "utt.h"
#include "kb.h"
#include "logs3.h"		/* RAH, added to resolve log3_free */
#include "corpus.h"
#include "gmm_compute.h"

static arg_t arg[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0003",
      "Base in which all log-likelihoods calculated" },
    { "-feat",
      ARG_STRING,
      "1s_c_d_dd",
      "Feature type: Must be s3_1x39 / 1s_c_d_dd/ s2_4x ."},
    /*Uncomment the hardwiring feature part, allow use to choose the type of feature */
    /*By ARCHAN Jan 19, 2004 , other features are legitimate but not tested.*/
    { "-lminmemory",
      ARG_INT32,
      "0",
      "Load language model into memory (default: use disk cache for lm"},
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
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
    { "-lm",
      ARG_STRING,
      NULL,
      "Word trigram language model input file" },
    { "-lmctlfn",
      ARG_STRING,
      NULL,
      "Control file for language model\n"},
    { "-lmdumpdir",
      ARG_STRING,
      NULL,
      "The directory for dumping the DMP file. "},
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
#if 1
    /* 20040504: ARCHAN */
    /* Re-start this part of the code */
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
    { "-subvq",
      ARG_STRING,
      NULL,
      "Sub-vector quantized form of acoustic model" },
    { "-subvqbeam",
      ARG_FLOAT64,
      "3.0e-3",
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" },
    { "-gs",
      ARG_STRING,
      NULL,
      "Gaussian Selection Mapping." },
    { "-ds",
      ARG_INT32,
      "1",
      "Ratio of Down-sampling the frame computation." },
    { "-cond_ds",
      ARG_INT32,
      "0",
      "Conditional Down-sampling, override normal down sampling. require specify a gaussian selection map" },

    { "-dist_ds",
      ARG_INT32,
      "0",
      "Distance-based Down-sampling, override normal down sampling." },
    { "-gs4gs",
      ARG_INT32,
      "1",
      "A flag that specified whether the input GS map will be used for Gaussian Selection. If it is disabled, the map will only provide information to other modules." },
    { "-svq4svq",
      ARG_INT32,
      "0",
      "A flag that specified whether the input SVQ will be used as approximate scores of the Gaussians" },
    { "-ci_pbeam",
      ARG_FLOAT64,
      "1e-80", /*default is huge , so nearly every cd phone will be computed */
      "CI phone beam for CI-based GMM Selection. [0(widest) .. 1(narrowest)]"},
    { "-tighten_factor", /* Use in "Down-sampling", this will tightened the beam
			  */
      ARG_FLOAT64,
      "0.5",
      "From 0 to 1, it tightens the beam width when the frame is dropped"},
    { "-maxcdsenpf",
      ARG_INT32,
      "100000",
      "Max no. of distinct CD senone will be computed. " },
    { "-pheurtype",
      ARG_INT32,
      "0",
      "0 = bypass, 1= sum of max, 2 = sum of avg, 3 = sum of 1st senones only" },
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
    { "-wend_beam",
      ARG_FLOAT64,
      "1.0e-80",
      "Beam selecting word-final HMMs exiting in each frame [0(widest) .. 1(narrowest)]" },
    { "-pl_window",
      ARG_INT32,
      "1",
      "Window size (actually window size-1) of phoneme look-ahead." },
    { "-pl_beam",
      ARG_FLOAT64,
      "1.0e-80",
      "Beam for phoneme look-ahead. [1 (narrowest)..10000000(very wide)]" },
    { "-ctl",
      ARG_STRING,
      NULL,
      "Control file listing utterances to be processed" },
    { "-ctl_lm",
      ARG_STRING,
      NULL,
      "Control file that list the corresponding LM for an utterance" },
    { "-ctl_mllr",
      ARG_STRING,
      NULL,
      "Control file that list the corresponding MLLR matrix for an utterance"},
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
    { "-cepext",
      ARG_STRING,
      ".mfc",
      "Input cepstrum files extension (prefixed to filespecs in control file)" },
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
    { "-hyp",
      ARG_STRING,
      NULL,
      "Recognition result file, with only words" },
    { "-hypseg",
      ARG_STRING,
      NULL,
      "Recognition result file, with word segmentations and scores" },
    { "-vqeval",
      ARG_INT32,
      "3",
      "A value added which  used only part of the cepstral vector to do the estimation"},
    { "-treeugprob",
      ARG_INT32,
      "1",
      "If TRUE (non-0), Use unigram probs in lextree" },
    { "-logfn",
      ARG_STRING,
      NULL,
      "Log file (default stdout/stderr)" },
    { NULL, ARG_INT32, NULL, NULL }
};

/*Beware of globals! */

stats_t cd_st;
stats_t ci_st;
stats_t *sen_st;
stats_t *cur_sen_st;

stats_t *sen_Nbest_st;
stats_t *cur_sen_Nbest_st;


int32 main (int32 argc, char *argv[])
{
    kb_t kb;
    ptmr_t tm;
    int i;
    
    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc,argv,"default.arg",arg);
    char str[100];

    unlimit ();
    
    kb_init (&kb);

    init_stat(&cd_st,"CD Senone");
    init_stat(&ci_st,"CI Senone");
  
    sen_st=ckd_calloc(kb.kbcore->mdef->n_ci_sen,sizeof(stats_t));
    cur_sen_st=ckd_calloc(kb.kbcore->mdef->n_ci_sen,sizeof(stats_t));

    sen_Nbest_st=
      ckd_calloc((int32) (kb.kbcore->mdef->n_sen - kb.kbcore->mdef->n_ci_sen)/NBEST_STEP,sizeof(stats_t));
    cur_sen_Nbest_st=
      ckd_calloc((int32) (kb.kbcore->mdef->n_sen - kb.kbcore->mdef->n_ci_sen)/NBEST_STEP,sizeof(stats_t));

    for(i=0;i<kb.kbcore->mdef->n_ci_sen;i++){
      sprintf(str,"Senone %d",i);
      init_stat(&sen_st[i],str);
      sprintf(str,"Cur Senone %d",i);
      init_stat(&cur_sen_st[i],str);
    }

    for(i=0 ; i< (int32) ((float)(kb.kbcore->mdef->n_sen - kb.kbcore->mdef->n_ci_sen)/ (float)NBEST_STEP) ; i++){
      sprintf(str," %d -Best Senone",i*NBEST_STEP);
      init_stat(&sen_Nbest_st[i],str);
      sprintf(str," %d -Cur Best Senone",i*NBEST_STEP);
      init_stat(&cur_sen_Nbest_st[i],str);
    }

    if(cmd_ln_str("-ctl")){
      tm = ctl_process (cmd_ln_str("-ctl"),
			cmd_ln_str("-ctl_mllr"),
			cmd_ln_int32("-ctloffset"),
			cmd_ln_int32("-ctlcount"),
			gmm_compute, &kb);
    }else if (cmd_ln_str("-utt")){
      E_FATAL("-utt is disabled for goodat this moment.\n");
    }else{
      E_FATAL("Both -utt and -ctl are not specified. \n");
    }
    
    print_stat(&cd_st);
    print_stat(&ci_st);

    for(i=0 ; i< (int32) ((float)(kb.kbcore->mdef->n_sen - kb.kbcore->mdef->n_ci_sen)/ (float)NBEST_STEP) ; i++){
      print_stat(&sen_Nbest_st[i]);
    }

    for(i =0 ; i <kb.kbcore->mdef->n_ci_sen ;i++)
      print_stat(&sen_st[i]);



    ckd_free(sen_st);
    ckd_free(cur_sen_st);
    ckd_free(sen_Nbest_st);
    ckd_free(cur_sen_Nbest_st);

    cmd_ln_appl_exit();


    exit(0);
}

