/* ====================================================================
* Copyright (c) 1999-2004 Carnegie Mellon University.	All rights
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

/* OLD LOGS before renaming to live_decode_API.h
----------------------------
revision 1.8
date: 2004/09/03 18:17:11;  author: yitao;  state: Exp;  lines: +15 -11

changed ld_process_frames to ld_process_ceps
----------------------------
revision 1.7
date: 2004/08/25 20:44:31;  author: yitao;  state: Exp;  lines: +65 -41


1.  added code to record uttid in live-decode
2.  added more code to flesh out remote-decode.  not compiling yet.
----------------------------
revision 1.6
date: 2004/08/24 18:05:50;  author: yitao;  state: Exp;  lines: +2 -2

fixed compilation bug in function ld_utt_free_hyps().
----------------------------
revision 1.5
date: 2004/08/23 20:41:36;  author: yitao;  state: Exp;  lines: +7 -14

basic implementation for remote-decode API.  not compiling yet.
----------------------------
revision 1.4
date: 2004/08/10 22:13:48;  author: yitao;  state: Exp;  lines: +18 -10

added some minor comments in the code.  no significant change.
----------------------------
revision 1.3
date: 2004/08/09 21:40:36;  author: yitao;  state: Exp;  lines: +122 -93

1.  fixed some bugs in Live-Decode API.  changed kb.c, kb.h, utt.c, live_decode.c, liv
e_decode.h.
2.  changed some filenames in src/programs/.  now there are 2 sets of livedecode and l
ivepretend: one that uses the old API (livedecode and livepretend), and one that uses 
the new API (livedecode2 and livepretend2).
3.  modified Makefile.am to reflect the filename changes above.
----------------------------
revision 1.2
date: 2004/08/08 23:34:50;  author: arthchan2003;  state: Exp;  lines: +1 -1
temporary fixes of live_decode.c and live_decode.h
----------------------------
revision 1.1
date: 2004/08/06 15:07:39;  author: yitao;  state: Exp;
*** empty log message ***
=============================================================================
*/

#include <libutil/libutil.h>
#include "live_decode_API.h"
#include "live_decode_args.h"
#include "utt.h"
#include <time.h>

/* Decoder states */
enum {
  LD_STATE_IDLE,
  LD_STATE_DECODING,
  LD_STATE_FINISHED
};

/* Utility function declarations */
static int
ld_init_impl(live_decoder_t *_decoder, int32 _internal_cmdln);

static int
ld_set_uttid(live_decoder_t *_decoder, char *_uttid);

static int
ld_record_hyps(live_decoder_t *_decoder, int _end_utt);

static void
ld_free_hyps(live_decoder_t *_decoder);

static void
ld_process_raw_impl(live_decoder_t *_decoder,
		    int16 *_samples,
		    int32 _num_samples,
		    int32 _begin_utt,
		    int32 _end_utt);

static int
ld_init_impl(live_decoder_t *_decoder, int32 _internal_cmdln)
{
  param_t fe_param;
	
  assert(_decoder != NULL);

  unlimit();
	
  /* allocate and initialize front-end */
  fe_param.SAMPLING_RATE = (float32)cmd_ln_int32 ("-samprate");
  fe_param.FRAME_RATE = cmd_ln_int32("-frate");
  fe_param.WINDOW_LENGTH = cmd_ln_float32("-wlen");
  fe_param.FB_TYPE = strcmp("mel_scale", cmd_ln_str("-fbtype")) == 0 ?
    MEL_SCALE : LOG_LINEAR;
  fe_param.NUM_CEPSTRA = cmd_ln_int32("-ncep");
  fe_param.NUM_FILTERS = cmd_ln_int32("-nfilt");
  fe_param.FFT_SIZE = cmd_ln_int32("-nfft");
  fe_param.LOWER_FILT_FREQ = cmd_ln_float32("-lowerf");
  fe_param.UPPER_FILT_FREQ = cmd_ln_float32("-upperf");
  fe_param.PRE_EMPHASIS_ALPHA = cmd_ln_float32("-alpha");
  if ((_decoder->fe = fe_init(&fe_param)) == NULL) {
    goto ld_init_impl_cleanup;
  }

  /* capture decoder parameters */
  kb_init(&_decoder->kb);
  _decoder->hmm_log = cmd_ln_int32("-hmmdump") ? stderr : NULL;
  _decoder->max_wpf = cmd_ln_int32("-maxwpf");;
  _decoder->max_histpf = cmd_ln_int32("-maxhistpf");
  _decoder->max_hmmpf = cmd_ln_int32("-maxhmmpf");
  _decoder->phones_skip = cmd_ln_int32 ("-ptranskip");

  /* initialize decoder variables */
  _decoder->kbcore = _decoder->kb.kbcore;
  _decoder->hyp_frame_num = -1;
  _decoder->uttid = NULL;
  _decoder->ld_state = LD_STATE_IDLE;
  _decoder->hyp_str = NULL;
  _decoder->hyp_segs = NULL;
  _decoder->internal_cmdln = _internal_cmdln;
  _decoder->features =
    feat_array_alloc(kbcore_fcb(_decoder->kbcore), LIVEBUFBLOCKSIZE);
  if (_decoder->features == NULL) {
    goto ld_init_impl_cleanup;
  }

  return 0;

 ld_init_impl_cleanup:
  if (_decoder->fe != NULL) {
    fe_close(_decoder->fe);
  }
  if (_decoder->features != NULL) {
    /* consult the implementation of feat_array_alloc() for how to free our
     * internal feature vector buffer */
    ckd_free((void *)**_decoder->features);
    ckd_free_2d((void **)_decoder->features);
  }
  if (_internal_cmdln == TRUE) {
    cmd_ln_free();
  }
  _decoder->ld_state = LD_STATE_FINISHED;

  return -1;
}

int
ld_init_with_args(live_decoder_t *_decoder, int _argc, char **_argv)
{
  assert(_decoder != NULL);

  if (cmd_ln_parse(arg_def, _argc, _argv) != 0) {
    return -1;
  }

  return ld_init_impl(_decoder, TRUE);
}

int
ld_init(live_decoder_t *_decoder)
{
  return ld_init_impl(_decoder, FALSE);
}

void
ld_finish(live_decoder_t *_decoder)
{
  assert(_decoder != NULL);

  if (_decoder->fe != NULL) {
    fe_close(_decoder->fe);
  }
  if (_decoder->features != NULL) {
    /* consult the implementation of feat_array_alloc() for how to free our
     * internal feature vector buffer */
    ckd_free((void *)**_decoder->features);
    ckd_free_2d((void **)_decoder->features);
  }
  if (_decoder->internal_cmdln == TRUE) {
    cmd_ln_free();
  }
  kb_free(&_decoder->kb);
  ld_free_hyps(_decoder);
  if (_decoder->uttid != NULL) {
    ckd_free(_decoder->uttid);
    _decoder->uttid = NULL;
  }
  _decoder->ld_state = LD_STATE_FINISHED;
}

int
ld_begin_utt(live_decoder_t *_decoder, char *_uttid)
{
  assert(_decoder != NULL);
  assert(_decoder->ld_state == LD_STATE_IDLE);

  ld_free_hyps(_decoder);

  fe_start_utt(_decoder->fe);
  utt_begin(&_decoder->kb);

  _decoder->frame_num = 0;
  _decoder->kb.nfr = 0;
  _decoder->kb.utt_hmm_eval = 0;
  _decoder->kb.utt_sen_eval = 0;
  _decoder->kb.utt_gau_eval = 0;

  return ld_set_uttid(_decoder, _uttid);
}

void
ld_end_utt(live_decoder_t *_decoder)
{
  assert(_decoder != NULL);

  ld_process_raw_impl(_decoder, NULL, 0, _decoder->frame_num == 0, TRUE);
  _decoder->kb.tot_fr += _decoder->kb.nfr;
  ld_record_hyps(_decoder, TRUE);
  utt_end(&_decoder->kb);
}

void
ld_process_raw(live_decoder_t *_decoder, int16 *_samples, int32 _num_samples)
{
  ld_process_raw_impl(_decoder, _samples, _num_samples,
		      _decoder->frame_num == 0, FALSE);
}

void
ld_process_ceps(live_decoder_t *_decoder, 
		float32 **_cep_frames,
		int32 _num_frames)
{
  int32 num_features = 0;

  assert(_decoder != NULL);
	
  if (_num_frames > 0) {
    num_features = feat_s2mfc2feat_block(kbcore_fcb(_decoder->kbcore),
					 _cep_frames,
					 _num_frames,
					 _decoder->frame_num == 0,
					 FALSE,
					 _decoder->features);
  }

  if (num_features > 0) {
    utt_decode_block(_decoder->features, 
		     num_features, 
		     &_decoder->frame_num, 
		     &_decoder->kb, 
		     _decoder->max_wpf, 
		     _decoder->max_histpf, 
		     _decoder->max_hmmpf, 
		     _decoder->phones_skip, 
		     _decoder->hmm_log);
  }
}

int
ld_retrieve_hyps(live_decoder_t *_decoder, char **_uttid, char **_hyp_str, 
		 hyp_t ***_hyp_segs)
{
  int rv = 0;

  assert(_decoder != NULL);

  /* re-record the hypothesis if there is a frame number mismatch */
  if (_decoder->frame_num != _decoder->hyp_frame_num) {
    rv = ld_record_hyps(_decoder, FALSE);
  }
  
  if (_uttid != NULL) {
    *_uttid = _decoder->uttid;
  }
  if (_hyp_str != NULL) {
    *_hyp_str = _decoder->hyp_str;
  }
  if (_hyp_segs != NULL) {
    *_hyp_segs = _decoder->hyp_segs;
  }
  
  return rv;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

int
ld_set_uttid(live_decoder_t *_decoder, char *_uttid)
{
  char *local_uttid = NULL;
  struct tm *times;

  assert(_decoder != NULL);

  if (_decoder->uttid != NULL) {
    ckd_free(_decoder->uttid);
    _decoder->uttid = NULL;
  }

  /* automatically-generated uttid */
  if (_uttid == NULL) {
    times = localtime(NULL);
    if ((local_uttid = ckd_malloc(17)) == NULL) {
      return -1;
    }
    snprintf(local_uttid, 20, "*%4d%2d%2dZ%2d%2d%2d",
	     times->tm_year, times->tm_mon, times->tm_mday,
	     times->tm_hour, times->tm_min, times->tm_sec);
  }
  /* user-defined uttid */
  else {
    if ((local_uttid = ckd_malloc(strlen(_uttid) + 1)) == NULL) {
      return -1;
    }
    strcpy(local_uttid, _uttid);
  }
  _decoder->uttid = local_uttid;

  return 0;
}

int
ld_record_hyps(live_decoder_t *_decoder, int _end_utt)
{
  int32	id;
  int32	i = 0;
  glist_t hyp_list;
  gnode_t *node;
  hyp_t *hyp;
  char *hyp_strptr = 0;
  char *hyp_str = 0;
  hyp_t **hyp_segs = 0;
  int hyp_seglen = 0;
  int hyp_strlen = 0;
  int finish_wid = 0;
  kb_t *kb = 0;
  dict_t *dict;

  assert(_decoder != NULL);

  ld_free_hyps(_decoder);

  kb = &_decoder->kb;
  dict = kbcore_dict(_decoder->kbcore);
  id = _end_utt ?
    vithist_utt_end(kb->vithist, _decoder->kbcore) :
    vithist_partialutt_end(kb->vithist, _decoder->kbcore);
  if (id < 0) {
    return -1;
  }

  /** record the segment length and the overall string length */
  hyp_list = vithist_backtrace(kb->vithist, id);
  finish_wid = dict_finishwid(dict);
  for (node = hyp_list; node; node = gnode_next(node)) {
    hyp = (hyp_t *)gnode_ptr(node);
    hyp_seglen++;
    if (!dict_filler_word(dict, hyp->id) && hyp->id != finish_wid) {
      hyp_strlen +=
	strlen(dict_wordstr(dict, dict_basewid(dict, hyp->id))) + 1;
    }
  }
  /** if hyp_str is non-trivial, we've counted one too many byte */
  if (hyp_strlen > 0) {
    hyp_strlen--;
  }

  /** allocate array to hold the segments and/or decoded string */
  hyp_segs = (hyp_t **)ckd_calloc(hyp_seglen + 1, sizeof(hyp_t *));
  hyp_str = (char *)ckd_calloc(hyp_strlen + 1, sizeof(char));
  if (hyp_segs == 0 || hyp_str == 0) {
    return -1;
  }
		
  /** iterate thru to fill in the array of segments and/or decoded string */
  i = 0;
  hyp_strptr = hyp_str;
  for (node = hyp_list; node; node = gnode_next(node), i++) {
    hyp = (hyp_t *)gnode_ptr(node);
    hyp_segs[i] = hyp;

    if (!dict_filler_word(dict, hyp->id) && hyp->id != finish_wid) {
      strcat(hyp_strptr, dict_wordstr(dict, dict_basewid(dict, hyp->id)));
      hyp_strptr += strlen(hyp_strptr);
      *hyp_strptr = ' ';
      hyp_strptr += 1;
    }
  }
  glist_free(hyp_list);
  
  hyp_str[hyp_strlen] = '\0';
  hyp_segs[hyp_seglen] = 0;
  _decoder->hyp_frame_num = _decoder->frame_num;
  _decoder->hyp_segs = hyp_segs;
  _decoder->hyp_str = hyp_str;

  return 0;
}

void
ld_free_hyps(live_decoder_t *_decoder)
{
  hyp_t **h;

  /** set the reference frame number to something invalid */
  _decoder->hyp_frame_num = -1;

  /** free and reset the hypothesis string */
  if (_decoder->hyp_str) {
    ckd_free(_decoder->hyp_str);
    _decoder->hyp_str = 0;
  }
  
  /** free and reset the hypothesis word segments */
  if (_decoder->hyp_segs) {
    for (h = _decoder->hyp_segs; *h; h++) {
      ckd_free(*h);
    }
    ckd_free(_decoder->hyp_segs);
    _decoder->hyp_segs = 0;
  }
}

void
ld_process_raw_impl(live_decoder_t *_decoder,
		    int16 *samples,
		    int32 num_samples,
		    int32 begin_utt,
		    int32 end_utt)
{
  float32 dummy_frame[MAX_CEP_LEN];
  float32 **frames = 0;
  int32 num_frames = 0;
  int32 num_features = 0;

  assert(_decoder != NULL);
	
  num_frames = fe_process_utt(_decoder->fe, samples, num_samples, &frames);
	
  if (end_utt) {
    fe_end_utt(_decoder->fe, dummy_frame);
  }
	
  if (num_frames > 0) {
    num_features = feat_s2mfc2feat_block(kbcore_fcb(_decoder->kbcore),
					 frames,
					 num_frames,
					 begin_utt,
					 end_utt,
					 _decoder->features);
  }

  if (num_features > 0) {
    utt_decode_block(_decoder->features, 
		     num_features, 
		     &_decoder->frame_num, 
		     &_decoder->kb, 
		     _decoder->max_wpf, 
		     _decoder->max_histpf, 
		     _decoder->max_hmmpf, 
		     _decoder->phones_skip, 
		     _decoder->hmm_log);
  }
	
  if (frames) {
    ckd_free_2d((void **)frames);
  }
}

