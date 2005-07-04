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
/***********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.13.4.1  2005/07/04  07:14:15  arthchan2003
 * Added -lminsearch option.
 * 
 * Revision 1.13  2005/06/22 02:53:12  arthchan2003
 * Add flags \-lmrescore (for rescoring paths in the search), \-op_mode (operation mode), \-Nstalextree (number of lexical tree used in mode 5), \-bt_wsil (back track with silence)
 *
 * Revision 1.10  2005/06/19 19:41:20  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.9  2005/06/17 23:44:39  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.8  2005/06/16 04:59:08  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.7  2005/05/27 01:15:44  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.6  2005/05/11 06:10:38  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.5  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 15-Jun-2004  Yitao Sun (yitao@cs.cmu.edu) at Carnegie Mellon University.
 * Created.
 */

/*
----------------------------
revision 1.3
date: 2004/07/27 21:53:05;  author: yitao;  state: Exp;  lines: +1 -1

weird.  i only changed kb.c to fix a small bug.  why so many changes?
----------------------------
revision 1.2
date: 2004/07/21 04:17:01;  author: yitao;  state: Exp;  lines: +8 -0

fixed conflicts when Arthur merged Ziad's fe_process_frame() code
----------------------------
revision 1.1
date: 2004/07/12 20:56:00;  author: yitao;  state: Exp;

moved these files from src/programs to src/libs3decoder so they could be included in t
he library.
=============================================================================
*/

#include "live_decode_args.h"

arg_t arg_def[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0003",
      "Base in which all log-likelihoods calculated" },
#if 1
    /* Commented out; must be s3_1x39 */
    { "-feat",
      ARG_STRING,
      "1s_c_d_dd",
      "Feature type: Must be s3_1x39 / s2_4x / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
#endif
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
      "Conditional Down-sampling, override normal down sampling." },
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
      "CI phone beam for CI-based GMM Selection. Good number should be [0(widest) .. 1(narrowest)]"},
    { "-tighten_factor", /* Use in "Down-sampling", this will tightened the beam
			  */
      ARG_FLOAT64,
      "0.5",
      "From 0 to 1, it tightens the beam width when the frame is dropped"},
    { "-maxcdsenpf",
      ARG_INT32,
      "100000",
      "Max no. of distinct CD senone will be computed. " },
    { "-wend_beam",
      ARG_FLOAT64,
      "1.0e-80",
      "Beam selecting word-final HMMs exiting in each frame [0(widest) .. 1(narrowest)]"},
    { "-pl_window",
      ARG_INT32,
      "1",
      "Window size (actually window size-1) of phoneme look-ahead." },
    { "-pheurtype",
      ARG_INT32,
      "0",
      "0 = bypass, 1= sum of max, 2 = sum of avg, 3 = sum of 1st senones only" },
    { "-pl_beam",
      ARG_FLOAT64,
      "1.0e-80",
      "Beam for phoneme look-ahead. [0(widest) .. 1(narrowest)]" },
    { "-ctl",
      ARG_STRING,
      NULL,
      "Control file listing utterances to be processed" },
    { "-ctl_lm",
      ARG_STRING,
      NULL,
      "Control file that list the corresponding LMs" },
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
    { "-lminmemory",
      ARG_INT32,
      "0",
      "Load language model into memory (default: use disk cache for lm"},
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    { "-vqeval",
      ARG_INT32,
      "3",
      "How many vectors should be analyzed by VQ when building the shortlist. It speeds up the decoder, but at a cost."},
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
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
#if 0
    /* Commented out; not supported */
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
#endif
    { "-lm",
      ARG_STRING,
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
    { "-mllr",
      ARG_STRING,
      NULL,
      "MLLR transfomation matrix to be applied to mixture gaussian means"},
    { "-cb2mllr",
      ARG_STRING,
      ".1cls.",
      "Senone to MLLR transformation matrix mapping file (or .1cls.)" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to data from -var file)" },
    { "-mixw",
      REQARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to data from -mixw file)" },
    { "-subvq",
      ARG_STRING,
      NULL,
      "Sub-vector quantized form of acoustic model" },
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
    { "-Nstalextree",
      ARG_INT32,
      "25",
      "No. of lextrees to be instantiated statically; " },
    { "-subvqbeam",
      ARG_FLOAT64,
      "3.0e-3",
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" },
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

#if 0 /*ARCHAN: As mentioned by Evandro, the fact that there is both -cepdir and rawdir is very confusing. Removed*/
    { "-cepdir",
      ARG_STRING,
      NULL,
      "Input cepstrum files directory (prefixed to filespecs in control file)" },
#endif
    { "-rawext",
      ARG_STRING,
      ".raw",
      "Input raw files extension"},
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
    { "-lmrescore",
      ARG_INT32,
      "1",
      "Whether LM is used to rescore the history at every frame. If 0, only acoustic score will be considered as path score. "},
    { "-lminsearch",
      ARG_INT32,
      "0",
      "Whether LM is used in search. Depends on the mode of the search, it could be ignored(e.g. mode LUCKY_WHEEL), or mutally exclusive to -lmrescore (e.g. mode WST)"},
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
    { "-treeugprob",
      ARG_INT32,
      "1",
      "If TRUE (non-0), Use unigram probs in lextree" },
    { "-maxhyplen",
      ARG_INT32,
      "1000",
      "Maximum number of words in a partial hypothesis (for block decoding)" },
    { "-maxcepvecs",
      ARG_INT32,
      "256",
      "Maximum number of cepstral vectors that can be obtained from a single sample buffer" },
    { "-samprate",
      ARG_INT32,
      "8000",
      "Sampling rate (only 8K and 16K currently supported)" },
    { "-nfilt",
      ARG_INT32,
      "31",
      "Number of mel filters" },
    { "-lowerf",
      ARG_FLOAT32,
      "200",
      "Lower edge of filters" },
    { "-upperf",
      ARG_FLOAT32,
      "3500",
      "Upper edge of filters" },
    { "-alpha",
      ARG_FLOAT32,
      "0.97",
      "alpha for pre-emphasis window"},
    { "-frate",
      ARG_INT32,
      "100",
      "frame rate"},
    { "-nfft",
      ARG_INT32,
      "256",
      "no. pts for FFT" },
    { "-wlen",
      ARG_FLOAT32,
      "0.0256",
      "window length"},
    { "-doublebw",
      ARG_INT32,
      "0",
      "whether mel filter triangle will have double the bandwidth, 0 is false"},
    { "-machine_endian",
      ARG_INT32,
#if defined(WORDS_BIGENDIAN) 
      "1",
#else
      "0",
#endif
      "the machine's endian, 0 is little, 1 is big endian"},
    { "-input_endian",
      ARG_INT32,
      "0",
      "the input data byte order, 0 is little, 1 is big endian"},
    { "-lmdumpdir",
      ARG_STRING,
      NULL,
      "The directory for dumping the DMP file. "},
    { "-lmctlfn",
      ARG_STRING,
      NULL,
      "Control file for language model\n"},
    { "-lmname",
      ARG_STRING,
      NULL,
      "Name of language model in -lmctlfn to use for all utterances" },
    { "-ncep",
      ARG_INT32,
      "13",
      "Number of cepstrums" },
    { "-fbtype",
      ARG_STRING,
      "mel_scale",
      "FB Type of mel_scale or log_linear" },
    { "-phypdump",
      ARG_INT32,
      "1",
      "dump parital hypothesis on the screen"},
    { "-op_mode",
      ARG_INT32,
      "4",
      "Operation Mode. Mode 4: TST search, Mode 5: WST search, Mode 1369: Debug "},
    {"-bt_wsil",
      ARG_INT32,
      "1",
     "Specified whether silence will be used to be the last word for backtracking. "},
    {"-backtrace",
     ARG_INT32,
      "1",
      "Whether detailed backtrace information (word segmentation/scores) shown in log" },
    { NULL, ARG_INT32, NULL, NULL }
};

