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
 *    Rita Singh.  The Live Decode API is the new top level API for Sphinx3.
 *    The goal of the Live Decode API is to provide a well documented and
 *    comprehensive API to control all aspects of the Sphinx3 speech decoder
 *    engine.
 *
 *    The return values, for example, hypothesis segments and string, unlike
 *    the rest of Sphinx3, are read-only, maintained internally, and clobbered
 *    by subsequent calls.
 */

#ifndef __LIVE_DECODE_H
#define __LIVE_DECODE_H

#include "kb.h"
#include "fe.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CEP_LEN				64

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
  char *uttid;

  /*
   * The frame number at which the hypothesis is recorded.
   */
  int32 hyp_frame_num;

  /*
   * Hypothesis string.  Result (or partial result) of the recognition is
   * stored as a complete string.
   */
  char *hyp_str;

  /*
   * Hypothesis word segments.  Result (or partial result) of the recognition
   * is stored as word segments.  Null-terminated array.
   */
  hyp_t **hyp_segs;

  /*
   * Feature buffer.  Re-allocation of feature buffer is quite expensive.  So
   * we allocate once per live decoder.
   */ 
  float32 ***features;

  /*
   * Boolean indicator whether we've internally allocated space for the
   * command line arguments.
   */
  int32 internal_cmd_ln;

} live_decoder_t;

/** Initializes the live-decoder.  Assumes the user has externally allocated
    and parsed arguments by calling <I>cmd_ln_parse()</I> or
    <I>cmd_ln_parse_file()</I>.  The user is responsible for calling
    <I>cmd_ln_free()</I> when he/she is done with the live decoder.
 
    A better alternative would be to wrap the parsed arguments in a structure
    and pass it to <I>ld_init()</I>.  It makes the use of <I>cmd_ln</I> 
    interface more explicit. It would also allow finer control over decoder
    parameters, in case if any user wants to have separate sets of decoder 
    parameters.

    @param decoder Pointer to the decoder.
    @return 0 for success.  -1 for failure.
*/
int ld_init(live_decoder_t *decoder);

/** Initializes the live-decoder.  This version makes the use of the
    <I>cmd_ln.h</I> interface transparent to the user.  Arguments are parsed
    and stored internally, and freed later when <I>ld_finish()</I> is called.

    @param decoder Pointer to the decoder.
    @param argc Argument count.
    @param argv Argument string array.
    @return 0 for success.  -1 for failure.
*/
int ld_init_with_args(live_decoder_t *decoder, int argc, char **argv);

/** Wraps up the live-decoder and frees up allocated memory in the process.

    @param decoder Pointer to the decoder.
    @return Always return 0 (for success).
*/
int ld_finish(live_decoder_t *decoder);

/** Starts decoding an utterance.  This function has to be called before any
    calls to <I>ld_process_raw()</I>, <I>ld_process_frames()</I>, or
    <I>ld_retrieve_hyps()</I> functions.  

    @param decoder Pointer to the decoder.
    @param uttid Utterance id string.
    @return 0 for success.  -1 for failure.
*/
int ld_begin_utt(live_decoder_t *decoder, char *uttid);

/** Finishes decoding the current utterance.  The user can retrieve the final
    hypothesis after calling this function.

    @param decoder Pointer to the decoder
    @return 0 for success.  -1 for failure.
*/
int ld_end_utt(live_decoder_t *decoder);

/** Process raw 16-bit samples for the current utterance decoding.

    @param decoder Pointer to the decoder.
    @param samples Buffer of int16 audio samples.
    @param num_samples Number of samples in the buffer.
    @return 0 for success.  -1 for failure.
*/
int ld_process_raw(live_decoder_t *decoder, 
		   int16 *samples,
		   int32 num_samples);

/** Process a buffer of feature frames for the current utterance decoding.  To
    use this function, make sure that the parameters to the front-end that
    generated the frames matches the parameters to the decoder's accoustic 
    model.

    @param decoder Pointer to the decoder.
    @param frames Buffer of audio feature frames.
    @param num_frames Number of frames in the buffer.
    @return 0 for success.  -1 for failure.
*/
int ld_process_frames(live_decoder_t *decoder, 
		      float32 **frames,
		      int32 num_frames);

/** Retrieve partial or final decoding results.  The result can be returned
    in a plain READ-ONLY string and/or an array of READ-ONLY word segments.
    
    @param decoder Pointer to the decoder.
    @param hyp_str Returned pointer to a READ-ONLY string.  If <I>null</I>, the
    string is not returned.
    @param hyp_segs Returned pointer to a null-terminated array of word
    segments.  If <I>null</I>, the array is not returned.
    @return 0 for success.  -1 for failure.
*/
int ld_retrieve_hyps(live_decoder_t *decoder, char **hyp_str,
		     hyp_t ***hyp_segs);


/** Abort the current decoding process immediately.  Retrieving the hypothesis
    after an abort is not guaranteed.

    @param decoder Pointer to the decoder.
    @return 0 for success.  -1 for failure.
*/
int ld_abort_utt(live_decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif

