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
 *    Created by Yitao Sun (yitao@cs.cmu.edu) based on the live.h created by
 *    Rita Singh.  This version is meant to expose features with a simpler and
 *    more explicit API.
 *
 *    The return values, unlike most of the Sphinx3 API, are read-only, 
 *    maintained internally, and clobbered by subsequent calls.
 */

#ifndef __LIVE2_H
#define __LIVE2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "kb.h"
#include "utt.h"
#include "new_fe.h"		   /* 01.15.01 - RAH, use new_fe.h instead */

#define MAX_UTTID_LEN				64
#define MAX_CEP_LEN				64
#define MAX_HYPSEG_LEN				64
#define MAX_HYPSTR_LEN				4095

typedef struct
{
  /*
   * Knowledge base.
   */
  kb_t kb;

  /*
   * Pointer to the knowledge base core.
   */
  kbcore_t *kbcore;

  /*
   * Pointer to the front-end.
   */
  fe_t *fe;

  /*
   * File pointer to the HMM logfile.
   */
  FILE *hmm_log;

  /*
   * Parameter: maximum words per frame.
   */
  int32 max_wpf;

  /*
   * Parameter: maximum histories per frame.
   */
  int32 max_histpf;

  /*
   * Parameter: maximum HMMs per frame.
   */
  int32 max_hmmpf;

  /*
   * Parameter: intervals at which wbeam is used for phone transitions.
   */
  int32 phones_skip;

  /*
   * Current frame number.
   */
  int32 frame_num;

  /*
   * Current state of the live decoder.
   */
  int32 ld_state;

  /*
   * UTTID (obviously NOT) filled in by knowledge-base.
   */
  char uttid[MAX_UTTID_LEN];

  /*
   * Hypothesis string.  Result (or partial result) of the recognition is
   * stored a complete string.
   */
  char hypstr[MAX_HYPSTR_LEN + 1];

  /*
   * Size of the hypothesis string above.
   */
  int32 hypstr_len;

  /*
   * Hypothesis segments.  Result (or partial result) of the recognition is
   * stored as word segments.
   */
  hyp_t **hypsegs;

  /*
   * Number of hypothesis segments.
   */
  int32 num_hypsegs;

  /*
   * The frame number at which the last recognition result was recorded.  We
   * use this as a time-flag to determine whether this is the latest result.
   */
  int32 hyps_frame_num;

  /*
   * The uttid associated with the last recorded recognition result.  We use
   * this along with the time-flag above to determine whether we have the
   * latest result.
   */
  char hyps_uttid[MAX_UTTID_LEN];

  /*
   * Feature buffer.  Re-allocation of feature buffer is quite expensive.  So
   * we allocate once per live decoder.
   */ 
  float32 ***features;

} live_decoder_t;

/*
 * Initialize the live-decoder.  Assume arguments have been parsed.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_init(live_decoder_t *decoder);

/*
 * Initialize the live-decoder.  Arguments are passed in to be parsed.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_init_with_args(live_decoder_t *decoder, int argc, char **argv);

/*
 * Wrap up the live-decoder and free up allocated memory in the process.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_finish(live_decoder_t *decoder);

/*
 * Start decoding an utterance.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_begin(live_decoder_t *decoder, char *uttid);

/*
 * Finish decoding an utterance.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_end(live_decoder_t *decoder);

/*
 * Process raw 16-bit data samples for utterance decoding.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_proc_raw(live_decoder_t *decoder, 
		    int16 *samples,
		    int32 num_samples);

/*
 * Process a buffer of framed data for utterance decoding.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_proc_frame(live_decoder_t *decoder, 
		      float32 **frames,
		      int32 num_frames);

/*
 * Process a block of feature vectors for utterance decoding.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_proc_feat(live_decoder_t *decoder, 
		     float32 ***features,
		     int32 num_features);

/*
 * Retrieve any partial or final decoding results in a plain READ-ONLY string
 * as well as an array of READ-ONLY word segments.  Each call to this function
 * may clobber the return value of previous calls.
 *
 * Arguments:
 *   char **hypstr - OUTPUT - a pointer to a READ-ONLY string.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_hyps(live_decoder_t *decoder, char **hyp_str, hyp_t ***hyp_segs);

//////////////////////////////////////////////////////////////////////////////

/*
 * Abort the current decoding process (in case you don't care about any partial
 * or final results returned by the decoder).
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_abort(live_decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif

