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
 *  May 14, 2004
 *    Created by Yitao Sun (yitao@cs.cmu.edu) based on the live.c created by
 *    Rita Singh.  This version is meant to expose features with a simpler and
 *    more explicit API.
 *
 */

#include "live2.h"

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
ld_utt_record_hyps(live_decoder_t *decoder);

int
ld_utt_free_hyps(live_decoder_t *decoder);

int
ld_init(live_decoder_t *decoder, int argc, char **argv)
{
  param_t fe_param;

  unlimit();

    /* some decoder parameter capturing
   * !!! NOTE - HARDCODED FOR NOW.  REPLACE WITH PARSE_ARG() ASAP !!!!
   */
  memset(decoder, 0, sizeof(live_decoder_t));
  kb_init(&decoder->kb);
  decoder->max_wpf = 20;
  decoder->max_histpf = 100;
  decoder->max_hmmpf = 20000;
  decoder->phones_skip = 0;
  decoder->hmm_log = 0;

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
  fe_param.SAMPLING_RATE = 16000.0f;
  fe_param.LOWER_FILT_FREQ = 200.0f;
  fe_param.UPPER_FILT_FREQ = 3500.0f;
  fe_param.NUM_FILTERS = 31;
  fe_param.FRAME_RATE = 100;
  fe_param.PRE_EMPHASIS_ALPHA = 0.97f;
  fe_param.FFT_SIZE = 256;
  fe_param.WINDOW_LENGTH = 0.0256f;
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
