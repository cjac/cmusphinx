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

#include "libutil/libutil.h"
#include "live2.h"
#include "args.h"

#define LD_STATE_IDLE			0
#define LD_STATE_STARTED		1
#define LD_STATE_ENDED			2

int
ld_utt_proc_raw_impl(live_decoder_t *decoder,
		     int16 *samples,
		     int32 num_samples,
		     int32 begin_utt,
		     int32 end_utt);

int
ld_init_with_args(live_decoder_t *decoder, int argc, char **argv)
{
  int rv = 0;

  cmd_ln_parse(arg_def, argc, argv);
  rv = ld_init(decoder);
  decoder->internal_cmd_ln = 0;
  return rv;
}

int
ld_init(live_decoder_t *decoder)
{
  param_t fe_param;
	
  unlimit();
  decoder->internal_cmd_ln = 1;
	
  /*
   * decoder parameter capturing
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
	
  /*
   * front-end parameter capturing
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
  if (decoder->internal_cmd_ln) {
    cmd_ln_free();
  }
  kb_free(&decoder->kb);
	
  /*
   * consult the implementation of feat_array_alloc() for the following two
   * lines
   */
  ckd_free((void *)**decoder->features);
  ckd_free_2d((void **)decoder->features);
	
  decoder->ld_state = LD_STATE_IDLE;
	
  return 0;
}

int
ld_utt_begin(live_decoder_t *decoder, char *uttid)
{
  kb_freehyps(&decoder->kb);
	
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
ld_utt_proc_frame(live_decoder_t *decoder, 
		 float32 **frames,
		 int32 num_frames)
{
  int32 num_features = 0;
	
  if (num_frames > 0) {
    num_features = feat_s2mfc2feat_block(kbcore_fcb(decoder->kbcore),
					 frames,
					 num_frames,
					 decoder->frame_num == 0,
					 0,
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
	
  return 0;
}

int
ld_utt_proc_feat(live_decoder_t *decoder, 
		 float32 ***features,
		 int32 num_features)
{
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
	
  return 0;
}

int
ld_utt_hyps(live_decoder_t *decoder, char **hyp_str, hyp_t ***hyp_segs)
{
  int32	id;
  int32	i = 0;
  glist_t hyp_list;
  gnode_t *node;
  hyp_t *hyp;
  dict_t *dict;
  char *hyp_strptr = NULL;
  kb_t *kb = &decoder->kb;

  if (decoder->ld_state == LD_STATE_ENDED) {
    if (hyp_segs) {
      *hyp_segs = kb->hyp_segs;
    }
    if (hyp_str) {
      *hyp_str = kb->hyp_str;
    }
    return 0;
  }
  else {
    kb_freehyps(kb);
  }

  dict = kbcore_dict (decoder->kbcore);
  id = vithist_partialutt_end(kb->vithist, decoder->kbcore);
  if (id >= 0) {
    hyp_list = vithist_backtrace(kb->vithist, id);

    /* record the segment length and the overall string length */
    for (node = hyp_list; node; node = gnode_next(node)) {
      hyp = (hyp_t *)gnode_ptr(node);
      if (hyp_segs) {
	kb->hyp_seglen++;
      }
      if (hyp_str) {
	if (!dict_filler_word(dict, hyp->id) && 
	    hyp->id != dict_finishwid(dict)) {
	  kb->hyp_strlen +=
	    strlen(dict_wordstr(dict, dict_basewid(dict, hyp->id))) + 1;
	}
      }
    }

    /* allocate array to hold the segments and/or decoded string */
    if (hyp_segs) {
      kb->hyp_segs = (hyp_t **)ckd_calloc(kb->hyp_seglen, sizeof(hyp_t *));
    }
    if (hyp_str) {
      kb->hyp_str = (char *)ckd_calloc(kb->hyp_strlen+1, sizeof(char));
    }
		
    /* iterate thru to fill in the array of segments and/or decoded string */
    i = 0;
    if (hyp_str) {
      hyp_strptr = kb->hyp_str;
    }
    for (node = hyp_list; node; node = gnode_next(node), i++) {
      hyp = (hyp_t *)gnode_ptr(node);
      if (hyp_segs) {
	kb->hyp_segs[i] = hyp;
      }
      if (hyp_str) {
	strcat(hyp_strptr, dict_wordstr(dict, dict_basewid(dict, hyp->id)));
	hyp_strptr += strlen(hyp_strptr);
	strcat(hyp_strptr, " ");
	hyp_strptr += 1;
      }
    }
    glist_free(hyp_list);

    if (hyp_str) {
      kb->hyp_str[kb->hyp_strlen - 1] = '\0';
    }
  }

  if (hyp_segs) {
    *hyp_segs = kb->hyp_segs;
  }

  if (hyp_str) {
    *hyp_str = kb->hyp_str;
  }

  return 0;
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
	
  if (frames) {
    ckd_free_2d((void **)frames);
  }
	
  return 0;
}

