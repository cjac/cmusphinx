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

/** Initializes the live-decoder.  Internal modules, including the cepstra-
    generating front-end, the language model, and the accoustic models are
    initialized, and live-decoder internal variables are set to a starting
    state.

    This version of the live-decoder assumes the user has externally parsed
    arguments using <I>cmd_ln_parse()</I> or <I>cmd_ln_parse_file()</I>.  The
    user is responsible for calling <I>cmd_ln_free()</I> when he/she is done
    with the live-decoder.

    @param decoder Pointer to the decoder.
    @return 0 for success.  -1 for failure.
    @see ld_finish
*/
int ld_init(live_decoder_t *decoder);

/** Initializes the live-decoder.  Internal modules, including the cepstrum-
    generating front-end, the language model, and the accoustic models are
    initialized, and live-decoder internal variables are set to a starting
    state.
    
    This version uses the <I>cmd_ln.h</I> interface internally.  Arguments are
    parsed and stored internally, and freed later when
    <I>{@link ld_finish ld_finish()}</I> is called.

    @param decoder Pointer to the decoder.
    @param argc Argument count.
    @param argv Argument string array.
    @return 0 for success.  -1 for failure.
    @see ld_finish
*/
int ld_init_with_args(live_decoder_t *decoder, int argc, char **argv);

/** Wraps up the live-decoder.  All internal modules are closed or unloaded.
    Internal variables are either freed or set to a finishing state.  This
    function should be called once the user is finished with the live-decoder.

    @param decoder Pointer to the decoder.
    @return Always return 0 (for success).
    @see ld_init
    @see ld_init_with_args
*/
int ld_finish(live_decoder_t *decoder);

/** Marks the start of the current utterance.  An utterance is a session of
    speech decoding that starts with a call to <I>ld_begin_utt()</I> and ends 
    with a call to <I>{@link ld_end_utt ld_end_utt()}</I>.  In the duration of 
    an utterance, speech data is processed with either
    <I>{@link ld_process_raw ld_process_raw()}</I> or
    <I>{@link ld_process_ceps ld_process_ceps()}</I>.  Decoding results
    (hypothesis) can be retrieved any time after the start of an utterance
    using <I>{@link ld_retrieve_hyps ld_retrieve_hyps()}</I>.  However, all 
    previous results will be clobbered at the start of a new utterance.

    At the moment, there is an undocumented time limit to the length of an
    utterance.

    @param decoder Pointer to the decoder.
    @param uttid Utterance ID string.  If <I>null</I>, the utterance ID is
    ignored.
    @return 0 for success.  -1 for failure.
    @see ld_end_utt
    @see ld_process_raw
    @see ld_process_ceps
    @see ld_retrieve_hyps
*/
int ld_begin_utt(live_decoder_t *decoder, char *uttid);

/** Marks the end of the current utterance.  The Live-Decode API can no longer
    process speech data until the start of the next utterance.  Any hypothesis
    retrieved prior to the end of the utterance is called a partial hypothesis.
    Any hypothesis retrieved after the end of the utterance is called the final
    hypothesis.  See <I>{@link ld_retrieve_hyps ld_retrieve_hyps()}</I> on how
    to retrieve hypothesis.

    @param decoder Pointer to the decoder
    @return 0 for success.  -1 for failure.
    @see ld_begin_utt
    @see ld_process_raw
    @see ld_process_ceps
    @see ld_retrieve_hyps
*/
int ld_end_utt(live_decoder_t *decoder);

/** Process raw 16-bit samples for the current utterance decoding.  This
    function has to be called in the duration of an utterance.  That is,
    in between calls to <I>{@link ld_begin_utt ld_begin_utt()}</I> and 
    <I>{@link ld_end_utt ld_end_utt()}</I>.

    @param decoder Pointer to the decoder.
    @param samples Buffer of int16 audio samples.
    @param num_samples Number of samples in the buffer.
    @return 0 for success.  -1 for failure.
    @see ld_begin_utt
    @see ld_end_utt
    @see ld_process_ceps
*/
int ld_process_raw(live_decoder_t *decoder, 
		   int16 *samples,
		   int32 num_samples);

/** Process a buffer of cepstrum frames for the current utterance.  To use
    this function, make sure that the parameters to the cepstra-generating
    front-end that matches the parameters to the decoder's accoustic 
    model.  This  function has to be called in the duration of an utterance.
    That is, in between calls to <I>{@link ld_begin_utt ld_begin_utt()}</I> and
    <I>{@link ld_end_utt ld_end_utt()}</I>.

    @param decoder Pointer to the decoder.
    @param frames Buffer of audio feature frames.
    @param num_frames Number of frames in the buffer.
    @return 0 for success.  -1 for failure.
    @see ld_begin_utt
    @see ld_end_utt
    @see ld_process_ceps
*/
int ld_process_ceps(live_decoder_t *decoder, 
		    float32 **frames,
		    int32 num_frames);

/** Retrieve partial or final decoding results (hypothesis).  Any
    hypothesis retrieved prior to the end of the utterance is called a 
    partial hypothesis.  Any hypothesis retrieved after the end of the 
    utterance is called the final hypothesis.  The hypothesis can be
    returned in a plain READ-ONLY string and/or an array of READ-ONLY word
    segments.  In the plain string result, all filler and end words are
    filtered out as well as the pronouciation information.  What is left is a
    very readable string representation of the decoding result.  There is no
    filtering in the word segment result.

    Here is an example on how to use the result returned by
    <I>ld_retrieve_hyps</I>:

    <PRE>
    live_decoder_t d;
    char *str;
    hyp_t **segs;

    ...

    ld_retrieve_hyps(&d, &str, &segs);
    printf("Decoded string: %s\n", str);
    for (; *segs; segs++) {
      printf("Word-segment id: %i\n", (*segs)->id);
    }
    </PRE>
    
    @param decoder Pointer to the decoder.
    @param hyp_str Returned pointer to a READ-ONLY string.  If <I>null</I>,
    the string is not returned.
    @param hyp_segs Returned pointer to a null-terminated array of word
    segments.  If <I>null</I>, the array is not returned.
    @return 0 for success.  -1 for failure.
*/
int ld_retrieve_hyps(live_decoder_t *decoder, char **hyp_str,
		     hyp_t ***hyp_segs);


/** Abort the current decoding process immediately.  As opposed to
    <I>{@link ld_end_utt ld_end_utt()}</I>.  Retrieving the hypothesis after an
    abort is not guaranteed.

    <EM>!!! NOT IMPLEMENTED YET !!!</EM>

    @param decoder Pointer to the decoder.
    @return 0 for success.  -1 for failure.
    @see ld_end_utt
*/
int ld_abort_utt(live_decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif

