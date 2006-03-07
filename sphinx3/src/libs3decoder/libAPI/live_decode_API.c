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
* HISTORY
 * $Log$
 * Revision 1.26  2006/03/07  21:04:19  dhdfu
 * Fill in the word string in srch_hyp_t
 * 
 * Revision 1.25  2006/02/24 13:41:26  arthchan2003
 * Used lm_read_advance instead of lm_read
 *
 * Revision 1.24  2006/02/22 22:36:56  arthchan2003
 * changed cmd_ln_access to cmd_ln_int32 in live_decode_API.c
 *
 * Revision 1.23  2006/02/22 21:46:51  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII:
 *
 * 1, Supported -dither (also -seed).
 * 2, Changed implementation from hyp_t to srch_hyp_t.  The definition will have NO CHANGE because hyp_t will be typedef as srch_hyp_t.
 * 3, Supported reading, addition and deletion of LMs through ld_read,
 * ld_set_lm and ld_delete_lm.
 *
 * Revision 1.22.4.7  2005/09/25 18:56:11  arthchan2003
 * Added dict argument into vithist_backtrace.
 *
 * Revision 1.22.4.6  2005/07/26 02:16:42  arthchan2003
 * Merged hyp_t with srch_hyp_t
 *
 * Revision 1.22.4.5  2005/07/20 19:42:30  arthchan2003
 * Completed live decode layer of lm add. Added command-line arguments for fsg and phone insertion.
 *
 * Revision 1.22.4.4  2005/07/18 19:00:36  arthchan2003
 * Changed the type of machine_endian and input_endian to "little" or "big" , changed the type of sampling rate to float32.
 *
 * Revision 1.22.4.3  2005/07/13 02:02:52  arthchan2003
 * Added -dither and -seed in the option.  Dithering is also support in livepretend. The behavior will be conformed s3.x's wave2feat, start to re-incorproate lm_read. Not completed yet.
 *
 * Revision 1.22.4.2  2005/07/07 02:31:54  arthchan2003
 * Remove -lminsearch, it proves to be useless and FSG implementation.
 *
 * Revision 1.22.4.1  2005/06/28 06:57:41  arthchan2003
 * Added protype of initializing and reading FSG, still not working.
 *
 * Revision 1.22  2005/06/22 02:49:34  arthchan2003
 * 1, changed ld_set_lm to call srch_set_lm, 2, hand the accounting tasks to stat_t, 3, added several empty functions for future use.
 *
 * Revision 1.6  2005/06/15 21:13:27  archan
 * Open ld_set_lm, ld_delete_lm in live_decode_API.[ch], Not yet decided whether ld_add_lm and ld_update_lm should be added at this point.
 *
 * Revision 1.5  2005/04/20 03:40:23  archan
 * Add many empty functions into ld, many of them are not yet implemented.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
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
#include "live_decode_API.h"
#include "live_decode_args.h"
#include "utt.h"
#include "bio.h"
#include "lm.h"
#include <time.h>

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
		    int32 _end_utt);

static int
ld_init_impl(live_decoder_t *_decoder, int32 _internal_cmdln)
{
  param_t fe_param;
  int rv = LD_SUCCESS;

  assert(_decoder != NULL);

  unlimit();

  /* ARCHAN 20050708: This part should be factored with fe_parse_option*/
  /* allocate and initialize front-end */
  fe_param.SAMPLING_RATE = cmd_ln_float32 ("-samprate");
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
  fe_param.dither=cmd_ln_int32("-dither");

  if(fe_param.dither){
    fe_init_dither(cmd_ln_int32("-seed"));
  }

  if ((_decoder->fe = fe_init(&fe_param)) == NULL) {
    E_WARN("Failed to initialize front-end.\n");
    rv = LD_ERROR_OUT_OF_MEMORY;
    goto ld_init_impl_cleanup;
  }

  /* capture decoder parameters */
  kb_init(&_decoder->kb);

  /* initialize decoder variables */
  _decoder->kbcore = _decoder->kb.kbcore;
  _decoder->hyp_frame_num = -1;
  _decoder->uttid = NULL;
  _decoder->ld_state = LD_STATE_IDLE;
  _decoder->hyp_str = NULL;
  _decoder->hyp_segs = NULL;

  /*
  _decoder->swap= (cmd_ln_int32("-machine_endian") != cmd_ln_int32("-input_endian"));
  */

  _decoder->swap= (strcmp(cmd_ln_str("-machine_endian"),cmd_ln_str("-input_endian"))!=0);

  _decoder->phypdump= (cmd_ln_int32("-phypdump"));
  _decoder->rawext= (cmd_ln_str("-rawext"));

  if(_decoder->phypdump)
    E_INFO("Partial hypothesis WILL be dumped\n");
  else
    E_INFO("Partial hypothesis will NOT be dumped\n");


  if(_decoder->swap) 
    E_INFO("Input data WILL be byte swapped\n");
  else 
    E_INFO("Input data will NOT be byte swapped\n");
  

  _decoder->internal_cmdln = _internal_cmdln;
  _decoder->features =
    feat_array_alloc(kbcore_fcb(_decoder->kbcore), LIVEBUFBLOCKSIZE);
  if (_decoder->features == NULL) {
    E_WARN("Failed to allocate internal feature buffer.\n");
    rv = LD_ERROR_OUT_OF_MEMORY;
    goto ld_init_impl_cleanup;
  }

  return LD_SUCCESS;

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

  return rv;
}

int
ld_init_with_args(live_decoder_t *_decoder, int _argc, char **_argv)
{
  assert(_decoder != NULL);

  if (cmd_ln_parse(arg_def, _argc, _argv) != 0) {
    E_WARN("Failed to parse arguments.\n");
    return LD_ERROR_INTERNAL;
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

  if (_decoder->ld_state != LD_STATE_IDLE) {
    E_WARN("Failed to begin a new utterance because decoder is not idle.\n");
    return LD_ERROR_INVALID_STATE;
  }

  ld_free_hyps(_decoder);

  utt_begin(&_decoder->kb);

  _decoder->num_frames_decoded = 0;
  _decoder->num_frames_entered = 0;
  _decoder->ld_state = LD_STATE_DECODING;

  stat_clear_utt(_decoder->kb.stat);

  return ld_set_uttid(_decoder, _uttid);
}

void
ld_end_utt(live_decoder_t *_decoder)
{
  assert(_decoder != NULL);

  ld_process_raw_impl(_decoder, NULL, 0, TRUE);
  _decoder->kb.stat->tot_fr += _decoder->kb.stat->nfr;
  ld_record_hyps(_decoder, TRUE);
  utt_end(&_decoder->kb);
  _decoder->ld_state = LD_STATE_IDLE;
}

void
ld_process_raw(live_decoder_t *_decoder, int16 *_samples, int32 _num_samples)
{
  ld_process_raw_impl(_decoder, _samples, _num_samples, FALSE);
}

void
ld_process_ceps(live_decoder_t *_decoder, 
		float32 **_cep_frames,
		int32 _num_frames)
{
  int32 num_features = 0;
  int32 begin_utt = _decoder->num_frames_entered == 0;

  assert(_decoder != NULL);
	
  if (_num_frames > 0) {
    num_features = feat_s2mfc2feat_block(kbcore_fcb(_decoder->kbcore),
					 _cep_frames,
					 _num_frames,
					 begin_utt,
					 FALSE,
					 _decoder->features);
    _decoder->num_frames_entered += _num_frames;
  }

  if (num_features > 0) {
    utt_decode_block(_decoder->features, 
		     num_features, 
		     &_decoder->num_frames_decoded,
		     &_decoder->kb);
  }
}

int
ld_retrieve_hyps(live_decoder_t *_decoder, char **_uttid, char **_hyp_str, 
		 hyp_t ***_hyp_segs)
{
  int rv = LD_SUCCESS;

  assert(_decoder != NULL);

  /* re-record the hypothesis if there is a frame number mismatch */
  if (_decoder->num_frames_decoded != _decoder->hyp_frame_num) {
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
  time_t t;

  assert(_decoder != NULL);

  if (_decoder->uttid != NULL) {
    ckd_free(_decoder->uttid);
    _decoder->uttid = NULL;
  }

  /* automatically-generated uttid */
  if (_uttid == NULL) {
    t = time(NULL);
    times = localtime(&t);
    if ((local_uttid = ckd_malloc(17)) == NULL) {
      E_WARN("Failed to allocate space for utterance id.\n");
      return LD_ERROR_OUT_OF_MEMORY;
    }
    sprintf(local_uttid, "*%4d%2d%2dZ%2d%2d%2d",
	    times->tm_year, times->tm_mon, times->tm_mday,
	    times->tm_hour, times->tm_min, times->tm_sec);
  }
  /* user-defined uttid */
  else {
    if ((local_uttid = ckd_malloc(strlen(_uttid) + 1)) == NULL) {
      E_WARN("Failed to allocate space for utterance id.\n");
      return LD_ERROR_OUT_OF_MEMORY;
    }
    strcpy(local_uttid, _uttid);
  }
  _decoder->uttid = local_uttid;
  /* Also set the kb internal uttid. This makes the uttid in the results. */
  kb_set_uttid(_decoder->uttid, &(_decoder->kb));

  return LD_SUCCESS;
}

int
ld_record_hyps(live_decoder_t *_decoder, int _end_utt)
{
  int32	id;
  int32	i = 0;
  glist_t hyp_list;
  gnode_t *node;
  srch_hyp_t *hyp;
  char *hyp_strptr = 0;
  char *hyp_str = 0;
  srch_hyp_t **hyp_segs = 0;
  int hyp_seglen = 0;
  int hyp_strlen = 0;
  int finish_wid = 0;
  kb_t *kb = 0;
  dict_t *dict;
  int rv;

  assert(_decoder != NULL);

  ld_free_hyps(_decoder);

  kb = &_decoder->kb;
  dict = kbcore_dict(_decoder->kbcore);
  id = _end_utt ?
    vithist_utt_end(kb->vithist, _decoder->kbcore) :
    vithist_partialutt_end(kb->vithist, _decoder->kbcore);
  if (id < 0) {
    E_WARN("Failed to retrieve viterbi history.\n");
    return LD_ERROR_INTERNAL;
  }

  /** record the segment length and the overall string length */
  hyp_list = vithist_backtrace(kb->vithist, id, dict);
  finish_wid = dict_finishwid(dict);
  for (node = hyp_list; node != NULL; node = gnode_next(node)) {
    hyp = (srch_hyp_t *)gnode_ptr(node);
    hyp_seglen++;
    if (!dict_filler_word(dict, hyp->id) && hyp->id != finish_wid) {
      hyp_strlen +=
	strlen(dict_wordstr(dict, dict_basewid(dict, hyp->id))) + 1;
    }
  }

  if (hyp_strlen == 0) {
    hyp_strlen = 1;
  }

  /** allocate array to hold the segments and/or decoded string */
  hyp_str = (char *)ckd_calloc(hyp_strlen, sizeof(char));
  hyp_segs = (srch_hyp_t **)ckd_calloc(hyp_seglen + 1, sizeof(srch_hyp_t *));
  if (hyp_segs == NULL || hyp_str == NULL) {
    E_WARN("Failed to allocate storage for hypothesis.\n");
    rv = LD_ERROR_OUT_OF_MEMORY;
    goto ld_record_hyps_cleanup;
  }
		
  /** iterate thru to fill in the array of segments and/or decoded string */
  i = 0;
  hyp_strptr = hyp_str;
  for (node = hyp_list; node != NULL; node = gnode_next(node), i++) {
    hyp = (srch_hyp_t *)gnode_ptr(node);
    hyp_segs[i] = hyp;

    hyp->word = dict_wordstr(dict, dict_basewid(dict, hyp->id));
    if (!dict_filler_word(dict, hyp->id) && hyp->id != finish_wid) {
      strcat(hyp_strptr, dict_wordstr(dict, dict_basewid(dict, hyp->id)));
      hyp_strptr += strlen(hyp_strptr);
      *hyp_strptr = ' ';
      hyp_strptr += 1;
    }
  }
  glist_free(hyp_list);
  
  hyp_str[hyp_strlen - 1] = '\0';
  hyp_segs[hyp_seglen] = 0;
  _decoder->hyp_frame_num = _decoder->num_frames_decoded;
  _decoder->hyp_segs = hyp_segs;
  _decoder->hyp_str = hyp_str;

  return LD_SUCCESS;

 ld_record_hyps_cleanup:
  if (hyp_segs != NULL) {
    ckd_free(hyp_segs);
  }
  if (hyp_str != NULL) {
    ckd_free(hyp_segs);
  }
  if (hyp_list != NULL) {
    for (node = hyp_list; node != NULL; node = gnode_next(node)) {
      if ((hyp = (srch_hyp_t *)gnode_ptr(node)) != NULL) {
	ckd_free(hyp);
      }
    }
  }

  return rv;
}

void
ld_free_hyps(live_decoder_t *_decoder)
{
  srch_hyp_t **h;

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
		    int32 end_utt)
{
  float32 dummy_frame[MAX_CEP_LEN];
  float32 **frames = 0;
  int32 num_frames = 0;
  int32 num_features = 0;
  int32 begin_utt = _decoder->num_frames_entered == 0;
  int32 return_value;
  int i;

  assert(_decoder != NULL);

  if (begin_utt) {
    fe_start_utt(_decoder->fe);
  }
  
  if(_decoder->swap){
    for (i = 0; i < num_samples; i++) {
      SWAP_INT16(samples + i);
    }
    if(_decoder->fe->dither){
      fe_dither(samples,num_samples);
    }
  }

  return_value = fe_process_utt(_decoder->fe, samples, num_samples, &frames, &num_frames);

  if (end_utt) {
    return_value = fe_end_utt(_decoder->fe, dummy_frame, &num_frames);
    if(num_frames!=0){
      /* ARCHAN: If num_frames !=0, assign this last ending frame to
	 frames again.  The computation will then be correct.  Should
	 clean up the finite state logic in fe_interface layer. 
      */
      frames=(float32 **)ckd_calloc_2d(1,_decoder->fe->NUM_CEPSTRA,sizeof(float32));
      memcpy(frames[0],dummy_frame,_decoder->fe->NUM_CEPSTRA*sizeof(float32));
    }
  }
	
  if (FE_ZERO_ENERGY_ERROR == return_value) {
    E_WARN("Zero energy frame(s). Consider using dither\n");
  }

  if (num_frames > 0) {
    num_features = feat_s2mfc2feat_block(kbcore_fcb(_decoder->kbcore),
					 frames,
					 num_frames,
					 begin_utt,
					 end_utt,
					 _decoder->features);
    _decoder->num_frames_entered += num_frames;
  }

  if (num_features > 0) {
    utt_decode_block(_decoder->features, 
		     num_features, 
		     &_decoder->num_frames_decoded, 
		     &_decoder->kb);
  }
	
  if (frames != NULL) {
    ckd_free_2d((void **)frames);
  }
}

void ld_read_lm(live_decoder_t *_decoder, 
		const char* lmpath, 
		const char* lmname)
{
  srch_t* s;
  lm_t* lm;
  int32 ndict;
  s=(srch_t*)_decoder->kb.srch;

  ndict=dict_size(_decoder->kb.kbcore->dict);


  lm=lm_read_advance(lmpath,lmname,
	     cmd_ln_float32("-lw"),
	     cmd_ln_float32("-wip"),
	     cmd_ln_float32("-uw"),
	     ndict,NULL,1 /* Weight apply */
	     );

  s->srch_add_lm(s,lm,lmname);
}



void ld_set_lm(live_decoder_t *_decoder,const char *lmname)
{
  srch_t* s;
  s=(srch_t*)_decoder->kb.srch;
  s->srch_set_lm(s,lmname);
}

void ld_delete_lm(live_decoder_t *_decoder, const char *lmname)
{
  srch_t* s;
  s=(srch_t*)_decoder->kb.srch;
  s->srch_delete_lm(s,lmname);
}















