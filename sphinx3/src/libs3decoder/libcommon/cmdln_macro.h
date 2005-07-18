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
 * cmdln_macro.h -- Corpus-file related misc functions.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1.2.1  2005/07/18  18:56:20  arthchan2003
 * A centralized macro definition file that contains command-line information that shared by different applications.
 * 
 *
 *
 */

#include <fe.h>

/* Note by ARCHAN at 20050717 
   
   The following are macros that used by individual modules and
   routines. Since we want in one application, there is only one
   command-line arguments. (The concept of singleton in Pattern).  If
   we used variables, then every application that call this file and
   its implementation will need got every command-line from every
   application. That is not very nice. 

   Using macro will temporarily solve this problem and I hope this
   will only last for less than one minor version time.  A better way,
   as what we have already done in Sphinx 4, is to ask the libraries
   to register its command-line argument as well as configuration
   arguments.  This will give user a better time of configuration and
   us a better architecture. 

 */

#define gmm_command_line_macro() \
    { "-mean",\
      REQARG_STRING,\
      NULL,\
      "Mixture gaussian means input file" },\
    { "-var",\
      REQARG_STRING,\
      NULL,\
      "Mixture gaussian variances input file" },\
    { "-varfloor",\
      ARG_FLOAT32,\
      "0.0001",\
      "Mixture gaussian variance floor (applied to data from -var file)" },\
    { "-mixw",\
      REQARG_STRING,\
      NULL,\
      "Senone mixture weights input file" },\
    { "-mixwfloor",\
      ARG_FLOAT32,\
      "0.0000001",\
      "Senone mixture weights floor (applied to data from -mixw file)" },


#define acoustic_model_command_line_macro() \
    gmm_command_line_macro() \
    { "-mdef", \
      REQARG_STRING,\
      NULL,\
      "Model definition input file" },\
    { "-tmat",\
      REQARG_STRING,\
      NULL,\
      "HMM state transition matrix input file" },\
    { "-tmatfloor",\
      ARG_FLOAT32,\
      "0.0001",\
      "HMM state transition probability floor (applied to -tmat file)" },\
    { "-senmgau",\
      ARG_STRING,\
      ".cont.",\
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" }, 

#define language_model_command_line_macro() \
    { "-lminmemory", \
      ARG_INT32, \
      "0", \
      "Load language model into memory (default: use disk cache for lm"}, \
    { "-lm", \
      ARG_STRING, \
      NULL, \
      "Word trigram language model input file" }, \
    { "-lmctlfn", \
      ARG_STRING, \
      NULL, \
      "Specify a set of language model\n"}, \
    { "-lmdumpdir", \
      ARG_STRING, \
      NULL, \
      "The directory for dumping the DMP file. "}, \
    { "-lmname", \
      ARG_STRING, \
      NULL, \
      "Name of language model in -lmctlfn to use for all utterances" }, 

#define log_table_command_line_macro() \
    { "-logbase", \
      ARG_FLOAT32, \
      "1.0003", \
      "Base in which all log-likelihoods calculated" }, \
    { "-log3table", \
      ARG_INT32, \
      "1", \
      "Determines whether to use the logs3 table or to compute the values at run time."}, 

#define phoneme_lookahead_command_line_macro() \
    { "-pheurtype", \
      ARG_INT32, \
      "0", \
      "0 = bypass, 1= sum of max, 2 = sum of avg, 3 = sum of 1st senones only" }, \
    { "-pl_window", \
      ARG_INT32, \
      "1", \
      "Window size (actually window size-1) of phoneme look-ahead." }, \
    { "-pl_beam", \
      ARG_FLOAT64, \
      "1.0e-80", \
      "Beam for phoneme look-ahead. [1 (narrowest)..10000000(very wide)]" }, 

#define histogram_pruning_command_line_macro() \
    { "-maxwpf", \
      ARG_INT32, \
      "20", \
      "Max no. of distinct word exits to maintain at each frame" }, \
    { "-maxhistpf", \
      ARG_INT32, \
      "100", \
      "Max no. of histories to maintain at each frame" }, \
    { "-hmmhistbinsize", \
      ARG_INT32, \
      "5000", \
      "Performance histogram: #frames vs #HMMs active; #HMMs/bin in this histogram" }, \
    { "-maxhmmpf", \
      ARG_INT32, \
      "20000", \
      "Max no. of active HMMs to maintain at each frame; approx." }, 

#define dictionary_command_line_macro() \
    { "-dict", \
      ARG_STRING, \
      NULL, \
      "Main pronunciation dictionary (lexicon) input file" }, \
    { "-fdict", \
      ARG_STRING, \
      NULL, \
      "Silence and filler (noise) word pronunciation dictionary input file" },

#define fast_GMM_computation_command_line_macro() \
    { "-subvq", \
      ARG_STRING, \
      NULL, \
      "Sub-vector quantized form of acoustic model" }, \
    { "-subvqbeam", \
      ARG_FLOAT64, \
      "3.0e-3", \
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" }, \
    { "-gs", \
      ARG_STRING, \
      NULL, \
      "Gaussian Selection Mapping." }, \
    { "-ds", \
      ARG_INT32, \
      "1", \
      "Ratio of Down-sampling the frame computation." }, \
    { "-cond_ds", \
      ARG_INT32, \
      "0", \
      "Conditional Down-sampling, override normal down sampling. require specify a gaussian selection map" }, \
    { "-dist_ds", \
      ARG_INT32, \
      "0", \
      "Distance-based Down-sampling, override normal down sampling." }, \
    { "-gs4gs", \
      ARG_INT32, \
      "1", \
      "A flag that specified whether the input GS map will be used for Gaussian Selection. If it is disabled, the map will only provide information to other modules." }, \
    { "-svq4svq", \
      ARG_INT32, \
      "0", \
      "A flag that specified whether the input SVQ will be used as approximate scores of the Gaussians" }, \
    { "-ci_pbeam", \
      ARG_FLOAT64, \
      "1e-80", /*default is huge , so nearly every cd phone will be computed */ \
      "CI phone beam for CI-based GMM Selection. [0(widest) .. 1(narrowest)]"}, \
    { "-tighten_factor", /* Use in "Down-sampling", this will tightened the beam \
			  */ \
      ARG_FLOAT64, \
      "0.5", \
      "From 0 to 1, it tightens the beam width when the frame is dropped"}, \
    { "-maxcdsenpf", \
      ARG_INT32, \
      "100000", \
      "Max no. of distinct CD senone will be computed. " }, \
    { "-vqeval", \
      ARG_INT32, \
      "3", \
      "A value added which  used only part of the cepstral vector to do the estimation"}, 


#if 0
    { "-feat",
      ARG_STRING,
      "s2_4x",
      "Feature stream:\n\t\t\t\ts2_4x: Sphinx-II type 4 streams, 12cep, 24dcep, 3pow, 12ddcep\n\t\t\t\ts3_1x39: Single stream, 12cep+12dcep+3pow+12ddcep\n\t\t\t\t1s_12c_12d_3p_12dd: Single stream, 12cep+12dcep+3pow+12ddcep\n\t\t\t\t1s_c: Single stream, given input vector only\n\t\t\t\t1s_c_d: Feature + Deltas only\n\t\t\t\t1s_c_dd: Feature + Double deltas only\n\t\t\t\t1s_c_d_dd: Feature + Deltas + Double deltas\n\t\t\t\t1s_c_wd_dd: Feature cep+windowed delcep+deldel \n\t\t\t1s_c_d_ld_dd: Feature + delta + longter delta + doubledelta" },

    { "-feat",	/* Captures the computation for converting input to feature vector */
      ARG_STRING,
      "1s_c_d_dd",
      "Feature stream: s2_4x / s3_1x39 / cep_dcep[,%d] / cep[,%d] / %d,%d,...,%d" },
#endif

#define cepstral_to_feature_command_line_macro() \
   { "-feat", \
      ARG_STRING, \
      "1s_c_d_dd", \
      "Feature type: Must be s3_1x39 / 1s_c_d_dd/ s2_4x ."}, \
    { "-cmn", \
      ARG_STRING, \
      "current", \
      "Cepstral mean normalization scheme (default: Cep -= mean-over-current-sentence(Cep))" }, \
    { "-varnorm", \
      ARG_STRING, \
      "no", \
      "Variance normalize each utterance (yes/no; only applicable if CMN is also performed)" }, \
    { "-agc", \
      ARG_STRING, \
      "max", \
      "Automatic gain control for c0 ('max' or 'none'); (max: c0 -= max-over-current-sentence(c0))" }, 

#define waveform_to_cepstral_command_line_macro() \
    { "-maxcepvecs", \
      ARG_INT32, \
      "256", \
      "Maximum number of cepstral vectors that can be obtained from a single sample buffer" }, \
    { "-samprate", \
      ARG_FLOAT32, \
      DEFAULT_SAMPLING_RATE, \
      "Sampling rate (only 8K and 16K currently supported)" }, \
    { "-nfilt", \
      ARG_INT32, \
      DEFAULT_NUM_FILTERS, \
      "Number of mel filters" }, \
    { "-lowerf", \
      ARG_FLOAT32, \
      DEFAULT_LOWER_FILT_FREQ, \
      "Lower edge of filters" }, \
    { "-upperf", \
      ARG_FLOAT32, \
      DEFAULT_UPPER_FILT_FREQ, \
      "Upper edge of filters" }, \
    { "-alpha", \
      ARG_FLOAT32, \
      DEFAULT_PRE_EMPHASIS_ALPHA, \
      "alpha for pre-emphasis window"}, \
    { "-frate", \
      ARG_INT32, \
      DEFAULT_FRAME_RATE, \
      "frame rate"}, \
    { "-nfft", \
      ARG_INT32, \
      DEFAULT_FFT_SIZE, \
      "no. pts for FFT" }, \
    { "-wlen", \
      ARG_FLOAT32, \
      DEFAULT_WINDOW_LENGTH, \
      "window length"}, \
    { "-doublebw", \
      ARG_INT32, \
      "0", \
      "whether mel filter triangle will have double the bandwidth, 0 is false"}, \
    { "-input_endian", \
      ARG_STRING, \
      "little", \
      "the input data byte order, big or little"}, \
    { "-ncep", \
      ARG_INT32, \
      DEFAULT_NUM_CEPSTRA, \
      "Number of cepstrums" }, \
    { "-fbtype", \
      ARG_STRING, \
      "mel_scale", \
      "FB Type of mel_scale or log_linear" }, \
    { "-dither", \
      ARG_INT32, \
      "0", \
      "Add 1/2-bit noise" }, \
    { "-seed", \
      ARG_INT32, \
      "-1", \
      "The seed for the random generator"}, \
    { "-blocksize", \
      ARG_INT32, \
      DEFAULT_BLOCKSIZE, \
      "Block size, used to limit the number of samples used at a time when reading very large audio files" }, 

#define speaker_adaptation_command_line_macro() \
    { "-mllr", \
      ARG_STRING, \
      NULL, \
      "MLLR transfomation matrix to be applied to mixture gaussian means"}, \
    { "-cb2mllr", \
      ARG_STRING, \
      ".1cls.", \
      "Senone to MLLR transformation matrix mapping file (or .1cls.)" },


#define common_filler_properties_command_line_macro() \
    { "-fillpen", \
      ARG_STRING, \
      NULL, \
      "Filler word probabilities input file (used in place of -silpen and -noisepen)" }, \
    { "-silprob", \
      ARG_FLOAT32, \
      "0.1", \
      "Default silence word probability" }, \
    { "-fillprob", \
      ARG_FLOAT32, \
      "0.1", \
      "Default non-silence filler word probability" }, \
    { "-lw", \
      ARG_FLOAT32, \
      "9.5", \
      "Language weight" }, \
    { "-wip", \
      ARG_FLOAT32, \
      "0.7", \
      "Word insertion penalty" }, \
    { "-uw", \
      ARG_FLOAT32, \
      "0.7", \
      "Unigram weight" }, 

#define common_s3x_beam_properties_command_line_macro() \
    { "-beam", \
      ARG_FLOAT64, \
      "1.0e-55", \
      "Beam selecting active HMMs (relative to best) in each frame [0(widest)..1(narrowest)]" }, \
    { "-pbeam", \
      ARG_FLOAT64, \
      "1.0e-50", \
      "Beam selecting HMMs transitioning to successors in each frame [0(widest)..1(narrowest)]" }, \
    { "-wbeam", \
      ARG_FLOAT64, \
      "1.0e-35", \
      "Beam selecting word-final HMMs exiting in each frame [0(widest)..1(narrowest)]" }, \
    { "-wend_beam", \
      ARG_FLOAT64, \
      "1.0e-80", \
      "Beam selecting word-final HMMs exiting in each frame [0(widest) .. 1(narrowest)]" }, \
    { "-ptranskip", \
      ARG_INT32, \
      "0", \
      "Use wbeam for phone transitions every so many frames (if >= 1)" }, 

#define common_application_properties_command_line_macro() \
    { "-logfn", \
      ARG_STRING, \
      NULL, \
      "Log file (default stdout/stderr)" },

#define control_file_handling_command_line_macro() \
    { "-ctl", \
      ARG_STRING, \
      NULL, \
      "Control file listing utterances to be processed" }, \
    { "-ctloffset", \
      ARG_INT32, \
      "0", \
      "No. of utterances at the beginning of -ctl file to be skipped" }, \
    { "-ctlcount", \
      ARG_INT32, \
      "1000000000",	/* A big number to approximate the default: "until EOF" */ \
      "No. of utterances to be processed (after skipping -ctloffset entries)" }, \

#define hypothesis_file_handling_command_line_macro() \
    { "-hyp", \
      ARG_STRING, \
      NULL, \
      "Recognition result file, with only words" }, \
    { "-hypseg", \
      ARG_STRING, \
      NULL, \
      "Recognition result file, with word segmentations and scores" },

#define cepstral_input_handling_command_line_macro() \
    { "-cepdir", \
      ARG_STRING, \
      NULL, \
      "Input cepstrum files directory (prefixed to filespecs in control file)" }, \
    { "-cepext", \
      ARG_STRING, \
      ".mfc", \
      "Input cepstrum files extension (prefixed to filespecs in control file)" }, 

#define output_lattice_handling_command_line_macro() \
    { "-outlatdir", \
      ARG_STRING, \
      NULL, \
      "Directory in which to dump word lattices" }, \
    { "-outlatoldfmt", \
      ARG_INT32, \
      "1", \
      "Whether to dump lattices in old format" }, \
    { "-latext", \
      ARG_STRING, \
      "lat.gz", \
      "Filename extension for lattice files (gzip compressed, by default)" }, 


/* decode-specific, that includes mode 4 and mode 5'
						   share between decode/livepretend/livedecode
						 */
#define decode_specific_command_line_macro() \
    { "-op_mode", \
      ARG_INT32, \
      "4", \
      "Operation Mode. Mode 4: TST search, Mode 5: WST search, Mode 1369: Debug "}, \
    { "-bptbldir", \
      ARG_STRING, \
      NULL, \
      "Directory in which to dump word Viterbi back pointer table (for debugging)" }, \
    { "-hmmdump", \
      ARG_INT32, \
      "0", \
      "Whether to dump active HMM details to stderr (for debugging)" }, \
    { "-lextreedump", \
      ARG_INT32, \
      "0", \
      "Whether to dump the lextree structure to stderr (for debugging)" }, \
    { "-bghist", \
      ARG_INT32, \
      "0", \
      "Bigram-mode: If TRUE only one BP entry/frame; else one per LM state" }, \
    { "-lmrescore", \
      ARG_INT32, \
      "1", \
      "Whether LM is used to rescore the history at every frame. If 0, only acoustic score will be considered as path score. "}, \
    { "-treeugprob", \
      ARG_INT32, \
      "1", \
      "If TRUE (non-0), Use unigram probs in lextree" }, 

#define dag_handling_command_line_macro() \
    { "-min_endfr", \
      ARG_INT32, \
      "3", \
      "Nodes ignored during search if they persist for fewer than so many end frames" }, \
    { "-dagfudge", \
      ARG_INT32, \
      "2", \
      "(0..2); 1 or 2: add edge if endframe == startframe; 2: if start == end-1" }, \
    { "-maxedge", \
      ARG_INT32, \
      "2000000", \
      "Max DAG edges allowed in utterance; aborted if exceeded; controls memory usage" }, \
    { "-maxlmop", \
      ARG_INT32, \
      "100000000", \
      "Max LMops in utterance after which it is aborted; controls CPU use (see maxlpf)" }, \
    { "-maxlpf", \
      ARG_INT32, \
      "40000", \
      "Max LMops/frame after which utterance aborted; controls CPU use (see maxlmop)" }, 

#define search_specific_command_line_macro() \
    {"-bt_wsil", \
      ARG_INT32, \
      "1", \
     "Specified whether silence will be used to be the last word for backtracking. "}, \
    {"-backtrace", \
     ARG_INT32, \
      "1", \
      "Whether detailed backtrace information (word segmentation/scores) shown in log" }, 

/* mode TST or mode 4*/
#define search_modeTST_specific_command_line_macro() \
    { "-Nlextree", \
      ARG_INT32, \
      "3", \
      "No. of lextrees to be instantiated; entries into them staggered in time" }, \
    { "-epl", \
      ARG_INT32, \
      "3", \
      "Entries Per Lextree; #successive entries into one lextree before lextree-entries shifted to the next" }, 

/* mode WST or mode 5*/
#define search_modeWST_specific_command_line_macro() \
    { "-Nstalextree", \
      ARG_INT32, \
      "25", \
      "No. of lextrees to be instantiated statically; " }, 

#define partial_hypothesis_command_line_macro() \
    { "-maxhyplen", \
      ARG_INT32, \
      "1000", \
      "Maximum number of words in a partial hypothesis (for block decoding)" }, \
    { "-phypdump", \
      ARG_INT32, \
      "1", \
      "dump parital hypothesis on the screen"},

#define control_lm_file_command_line_macro() \
    { "-ctl_lm", \
      ARG_STRING, \
      NULL, \
      "Control file that list the corresponding LMs" },

#define control_mllr_file_command_line_macro() \
    { "-ctl_mllr", \
      ARG_STRING, \
      NULL, \
      "Control file that list the corresponding MLLR matrix for an utterance"},

#define control_lm_mllr_file_command_line_macro() \
    control_lm_file_command_line_macro() \
    control_mllr_file_command_line_macro() 
