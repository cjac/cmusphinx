/* ====================================================================
* Copyright (c) 1999-2001 Carnegie Mellon University.	All rights
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
/*************************************************
* CMU ARPA Speech Project
*
* Copyright (c) 2000 Carnegie Mellon University.
* ALL RIGHTS RESERVED.
*************************************************
*
* May 14, 2004
*   Created by Yitao Sun (yitao@cs.cmu.edu) based on the live.c created by
*   Rita Singh.  This version is meant to expose features with a simpler and
*   more explicit API.
*
* Jun 10, 2004
*   Modified by Yitao Sun.  Added argument parsing.
*/

#include "live2.h"

#define LD_STATE_IDLE			0
#define LD_STATE_STARTED		1
#define LD_STATE_ENDED			2

static arg_t arg_def[] = {
    { "-logbase",
		ARG_FLOAT32,
		"1.0003",
		"Base in which all log-likelihoods calculated" },
#if 1
		/* Commented out; must be s3_1x39 */
    { "-feat",
	ARG_STRING,
	NULL,
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
    { "-gs4gs",
	ARG_INT32,
	"1",
	"A flag that specified whether the input GS map will be used for Gaussian Selection. If it is disabled, the map will only provide information to other modules." },
    { "-svq4svq",
	ARG_INT32,
	"0",
	"A flag that specified whether the input SVQ will be used as approximate scores of the Gaussians" },
    { "-ci_pbeam",
	ARG_FLOAT32,
	"1e-80", /*default is huge , so nearly every cd phone will be computed */
	"CI phone beam for CI-based GMM Selection. Good number should be [0(widest) .. 1(narrowest)]"},
    { "-wend_beam",
	ARG_FLOAT32,
	"1.0e-80",
	"Beam selecting word-final HMMs exiting in each frame [0(widest) .. 1(narrowest)]"},
    { "-pl_window",
	ARG_INT32,
	"1",
	"Window size (actually window size-1) of phoneme look-ahead." },
    { "-pl_beam",
	ARG_FLOAT32,
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
	REQARG_STRING,
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
    { "-cepdir",
	ARG_STRING,
	NULL,
	"Input cepstrum files directory (prefixed to filespecs in control file)" },
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
    { "-lmdumpdir",
	ARG_STRING,
	NULL,
	"The directory for dumping the DMP file. "},
		{ "-lmctlfn",
		ARG_STRING,
		NULL,
		"Control file for language model\n"},
		
    { NULL, ARG_INT32, NULL, NULL }
};

int
ld_utt_proc_raw_impl(live_decoder_t *decoder,
					 int16 *samples,
					 int32 num_samples,
					 int32 begin_utt,
					 int32 end_utt);

int
ld_utt_record_hyps(live_decoder_t *decoder);

int
ld_utt_free_hyps(live_decoder_t *decoder);

int
ld_init(live_decoder_t *decoder, int argc, char **argv)
{
	param_t fe_param;
	
	cmd_ln_parse(arg_def, argc, argv);
	unlimit();
	
    /* some decoder parameter capturing
	* !!! NOTE - HARDCODED FOR NOW.  REPLACE WITH PARSE_ARG() ASAP !!!!
	*/
	memset(decoder, 0, sizeof(live_decoder_t));
	kb_init(&decoder->kb);
	decoder->max_wpf = cmd_ln_int32 ("-maxwpf");;
	decoder->max_histpf = cmd_ln_int32 ("-maxhistpf");
	decoder->max_hmmpf = cmd_ln_int32 ("-maxhmmpf");
	decoder->phones_skip = cmd_ln_int32 ("-ptranskip");
	decoder->hmm_log = cmd_ln_int32("-hmmdump") ? stderr : NULL;
	
	decoder->kbcore = decoder->kb.kbcore;
	decoder->kb.uttid = decoder->uttid;
	decoder->hypsegs = 0;
	decoder->num_hypsegs = 0;
	decoder->hypstr_len = 0;
	decoder->hypstr[0] = '\0';
	decoder->features =
		feat_array_alloc(kbcore_fcb(decoder->kbcore), LIVEBUFBLOCKSIZE);
	decoder->ld_state = LD_STATE_IDLE;
	
	/* some front-end parameter capturing
	* !!! NOTE - HARDCODED FOR NOW.  REPLACE WITH PARSE_ARG() ASAP !!!!
	*/
	memset(&fe_param, 0, sizeof(param_t));
	fe_param.SAMPLING_RATE = (float32)cmd_ln_int32 ("-samprate");
	fe_param.LOWER_FILT_FREQ = cmd_ln_float32("-lowerf");
	fe_param.UPPER_FILT_FREQ = cmd_ln_float32("-upperf");
	fe_param.NUM_FILTERS = cmd_ln_int32("-nfilt");
	fe_param.FRAME_RATE = cmd_ln_int32("-frate");
	fe_param.PRE_EMPHASIS_ALPHA = cmd_ln_float32("-alpha");
	fe_param.FFT_SIZE = cmd_ln_int32("-nfft");
	fe_param.WINDOW_LENGTH = cmd_ln_float32("-wlen");
	
	decoder->fe = fe_init(&fe_param);
	if (!decoder->fe) {
		E_WARN("Front end initialization fe_init() failed\n");
		return -1;
	}
	
	return 0;
}

int
ld_finish(live_decoder_t *decoder)
{
	cmd_ln_free();
	ckd_free(decoder->kb.uttid);
	kb_free(&decoder->kb);
	
	ld_utt_free_hyps(decoder);
	
	/* consult the implementation of feat_array_alloc() for the following two
	* lines
	*/
	ckd_free((void *)decoder->features);
	ckd_free_2d((void **)decoder->features);
	
	decoder->ld_state = LD_STATE_IDLE;
	
	return 0;
}

int
ld_utt_begin(live_decoder_t *decoder, char *uttid)
{
	ld_utt_free_hyps(decoder);
	
	fe_start_utt(decoder->fe);
	utt_begin(&decoder->kb);
	decoder->frame_num = 0;
	decoder->kb.nfr = 0;
	decoder->kb.utt_hmm_eval = 0;
	decoder->kb.utt_sen_eval = 0;
	decoder->kb.utt_gau_eval = 0;
	decoder->ld_state = LD_STATE_STARTED;
	
	return 0;
}

int
ld_utt_end(live_decoder_t *decoder)
{
	ld_utt_proc_raw_impl(decoder, 0, 0, decoder->frame_num == 0, 1);
	decoder->kb.tot_fr += decoder->kb.nfr;
	utt_end(&decoder->kb);
	decoder->ld_state = LD_STATE_ENDED;
	
	return 0;
}

int
ld_utt_proc_raw(live_decoder_t *decoder, int16 *samples, int32 num_samples)
{
	return ld_utt_proc_raw_impl(decoder,
		samples,
		num_samples,
		decoder->frame_num == 0,
		0);
}

int
ld_utt_hypstr(live_decoder_t *decoder, char **hypstr)
{
	if (decoder->frame_num != decoder->hyps_frame_num ||
		strncmp(decoder->uttid, decoder->hyps_uttid, MAX_UTTID_LEN) != 0) {
		ld_utt_free_hyps(decoder);
		ld_utt_record_hyps(decoder);
	}
	
	*hypstr = decoder->hypstr;
	return decoder->hypstr_len;
}

int
ld_utt_hypseg(live_decoder_t *decoder, hyp_t ***hypsegs)
{
	if (decoder->frame_num != decoder->hyps_frame_num ||
		strncmp(decoder->uttid, decoder->hyps_uttid, MAX_UTTID_LEN) != 0) {
		ld_utt_free_hyps(decoder);
		ld_utt_record_hyps(decoder);
	}
	
	*hypsegs = decoder->hypsegs;
	return decoder->num_hypsegs;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

int
ld_utt_proc_raw_impl(live_decoder_t *decoder,
					 int16 *samples,
					 int32 num_samples,
					 int32 begin_utt,
					 int32 end_utt)
{
	float32 dummy_frame[MAX_CEP_LEN];
	float32 **frames = 0;
	int32 num_frames = 0;
	int32 num_features = 0;
	
	num_frames = fe_process_utt(decoder->fe, samples, num_samples, &frames);
	
	if (end_utt) {
		fe_end_utt(decoder->fe, dummy_frame);
	}
	
	if (num_frames > 0) {
		num_features = feat_s2mfc2feat_block(kbcore_fcb(decoder->kbcore),
			frames,
			num_frames,
			begin_utt,
			end_utt,
			decoder->features);
	}
	
	if (num_features > 0) {
		utt_decode_block(decoder->features, 
			num_features, 
			&decoder->frame_num, 
			&decoder->kb, 
			decoder->max_wpf, 
			decoder->max_histpf, 
			decoder->max_hmmpf, 
			decoder->phones_skip, 
			decoder->hmm_log);
	}
	
	if (num_frames > 0) {
		ckd_free_2d((void **)frames);
	}
	
	return 0;
}

int ld_utt_record_hyps(live_decoder_t *decoder)
{
	int32	id;
	int32	i = 0;
	glist_t hyp_list;
	gnode_t *node;
	dict_t *dict;
	char *hypstr_ptr = decoder->hypstr;
	char *dictstr;
	int32 dictstr_len;
	
	/* record the current frame-number and uttid so later on we can check
	* whether this result is the latest.
	*/
	strncpy(decoder->hyps_uttid, decoder->uttid, MAX_UTTID_LEN);
	decoder->hyps_frame_num = decoder->frame_num;
	
	dict = kbcore_dict (decoder->kbcore);
	id = decoder->ld_state == LD_STATE_ENDED ?
		vithist_utt_end(decoder->kb.vithist, decoder->kbcore) :
    vithist_partialutt_end(decoder->kb.vithist, decoder->kbcore);
	
	if (id >= 0) {
		hyp_list = vithist_backtrace(decoder->kb.vithist, id);
		decoder->num_hypsegs = glist_count(hyp_list);
		decoder->hypsegs = (hyp_t **)ckd_calloc(decoder->num_hypsegs,
			sizeof(hyp_t *));
		
		for (node = hyp_list; node; node = gnode_next(node), i++) {
			decoder->hypsegs[i] = (hyp_t *)gnode_ptr(node);
			dictstr = dict_wordstr(dict, decoder->hypsegs[i]->id);
			dictstr_len = strlen(dictstr);
			
			if (decoder->hypstr_len < MAX_HYPSTR_LEN) {
				if (decoder->hypstr_len + dictstr_len >= MAX_HYPSTR_LEN) {
					dictstr_len = MAX_HYPSTR_LEN - decoder->hypstr_len;
				}
				
				decoder->hypstr_len += dictstr_len;
				strncpy(hypstr_ptr, dictstr, dictstr_len);
				hypstr_ptr += dictstr_len;
			}
		}
		
		glist_free(hyp_list);
		*hypstr_ptr = '\0';
	}
	
	return 0;
}


int
ld_utt_free_hyps(live_decoder_t *decoder)
{
	int i = 0;
	
	if (decoder->hypsegs) {
		for (; i < decoder->num_hypsegs; i++) {
			if (decoder->hypsegs[i]) {
				ckd_free(decoder->hypsegs[i]);
			}
		}
		ckd_free(decoder->hypsegs);
		decoder->num_hypsegs = 0;
	}
	
	decoder->hypstr[0] = '\0';
	decoder->hypstr_len = 0;
	
	return 0;
}
