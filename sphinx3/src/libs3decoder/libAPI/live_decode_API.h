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

/*
revision 1.9
date: 2004/09/03 21:45:26;  author: yitao;  state: Exp;  lines: +2 -2

cleaning up remote_decode API by moving list operations into a list API
----------------------------
revision 1.8
date: 2004/09/03 16:50:56;  author: yitao;  state: Exp;  lines: +108 -37


modified comments to suit the use of doc++-
----------------------------
revision 1.7
date: 2004/08/27 05:22:43;  author: yitao;  state: Exp;  lines: +75 -105


removed remote-decode API from the linux compile.  added doc++ comments for live_decod
e.h-
----------------------------
revision 1.6
date: 2004/08/25 20:44:31;  author: yitao;  state: Exp;  lines: +13 -15


1.  added code to record uttid in live-decode
2.  added more code to flesh out remote-decode.  not compiling yet.
----------------------------
revision 1.5
date: 2004/08/23 20:41:38;  author: yitao;  state: Exp;  lines: +1 -11

basic implementation for remote-decode API.  not compiling yet.
----------------------------
revision 1.4
date: 2004/08/19 19:12:50;  author: yitao;  state: Exp;  lines: +1 -1

incompleted files remote-decode API.
----------------------------
revision 1.3
date: 2004/08/09 21:40:36;  author: yitao;  state: Exp;  lines: +11 -20

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
date: 2004/08/06 15:07:38;  author: yitao;  state: Exp;
*** empty log message ***
=============================================================================

*/


#ifndef __LIVE_DECODE_H
#define __LIVE_DECODE_H

#include "kb.h"
#include "fe.h"
#include "srch.h"
#include "hyp.h"

/** \file live_decode_API.h
 * \brief header for live mode decoding API 
 */
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CEP_LEN				64

#define LD_SUCCESS				0
#define LD_ERROR_OUT_OF_MEMORY			-0x01
#define LD_ERROR_NULL_POINTER			-0x02
#define LD_ERROR_INVALID_STATE			-0x04
#define LD_ERROR_INTERNAL			-0x08

#define LD_STATE_IDLE				0
#define LD_STATE_DECODING			1
#define LD_STATE_FINISHED			2

  /** Wrapper structure for live-mode recognition
   */
typedef struct
{
  /**
   * Knowledge base.
   */
  kb_t kb;

  /**
   * Pointer to the knowledge base core.
   */
  kbcore_t *kbcore;

  /**
   * Pointer to the front-end.
   */
  fe_t *fe;

  /**
   * Parameter: intervals at which wbeam is used for phone transitions.
   */
  int32 phones_skip;

  /**
   * Number of frames decoded.
   */
  int32 num_frames_decoded;

  /**
   * Number of frames entered.
   */
  int32 num_frames_entered;

  /**
   * Current state of the live decoder.
   */
  int32 ld_state;

  /**
   * UTTID (obviously NOT) filled in by knowledge-base.
   */
  char *uttid;

  /**
   * The frame number at which the hypothesis is recorded.
   */
  int32 hyp_frame_num;

  /**
   * Hypothesis string.  Result (or partial result) of the recognition is
   * stored as a complete string.
   */
  char *hyp_str;

  /**
   * Hypothesis word segments.  Result (or partial result) of the recognition
   * is stored as word segments.  Null-terminated array.
   */
  hyp_t **hyp_segs;

  /**
   * Boolean indicator whether we've internally allocated space for the
   * command line arguments.
   */
  int32 internal_cmdln;

  /**
   * Boolean indicates whether we will internally swap the samples. 
   */
  int32 swap;

  /**
   * Boolean indicates whether a partial hypothesis will be dumped. 
   */
  int32 phypdump;

  /**
   * Feature buffer.  Re-allocation of feature buffer is quite expensive.  So
   * we allocate once per live decoder.
   */ 
  float32 ***features;

  /**
   * Extenstion for the raw director 
   */
  char* rawext;

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
int ld_init(live_decoder_t *_decoder);

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
int ld_init_with_args(live_decoder_t *_decoder, int _argc, char **_argv);

/** Wraps up the live-decoder.  All internal modules are closed or unloaded.
    Internal variables are either freed or set to a finishing state.  This
    function should be called once the user is finished with the live-decoder.

    @param decoder Pointer to the decoder.
    @see ld_init
    @see ld_init_with_args
*/
void ld_finish(live_decoder_t *_decoder);

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
int ld_begin_utt(live_decoder_t *_decoder, char *_uttid);

/** Marks the end of the current utterance.  The Live-Decode API can no longer
    process speech data until the start of the next utterance.  Any hypothesis
    retrieved prior to the end of the utterance is called a partial hypothesis.
    Any hypothesis retrieved after the end of the utterance is called the final
    hypothesis.  See <I>{@link ld_retrieve_hyps ld_retrieve_hyps()}</I> on how
    to retrieve hypothesis.

    @param decoder Pointer to the decoder
    @see ld_begin_utt
    @see ld_process_raw
    @see ld_process_ceps
    @see ld_retrieve_hyps
*/
void ld_end_utt(live_decoder_t *_decoder);

/** Process raw 16-bit samples for the current utterance decoding.  This
    function has to be called in the duration of an utterance.  That is,
    in between calls to <I>{@link ld_begin_utt ld_begin_utt()}</I> and 
    <I>{@link ld_end_utt ld_end_utt()}</I>.

    @param decoder Pointer to the decoder.
    @param samples Buffer of int16 audio samples.
    @param num_samples Number of samples in the buffer.
    @see ld_begin_utt
    @see ld_end_utt
    @see ld_process_ceps
*/
void ld_process_raw(live_decoder_t *_decoder, 
		    int16 *_samples,
		    int32 _num_samples);
  
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
void ld_process_ceps(live_decoder_t *_decoder, 
		     float32 **_frames,
		     int32 _num_frames);

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
int ld_retrieve_hyps(live_decoder_t *_decoder, char **_uttid, char **_hyp_str,
		     hyp_t ***_hyp_segs);
  
/** Abort the current decoding process immediately.  As opposed to
    <I>{@link ld_end_utt ld_end_utt()}</I>.  Retrieving the hypothesis after an
    abort is not guaranteed.

    <EM>!!! NOT IMPLEMENTED YET !!!</EM>

    @param decoder Pointer to the decoder.
    @see ld_end_utt
*/
void ld_abort_utt(live_decoder_t *_decoder);


/** Set LM 
    @param _decode Pointer to the decode
    @param lmname the language model name
    @see ld_read_lm ld_delete_lm
 */

void ld_set_lm(live_decoder_t *_decoder,const char *lmname);

/** Delete LM 
    @param _decoder Pointer to the live mode decode
    @param lmname the language model name 
    @see ld_set_lm ld_read_lm
*/

void ld_delete_lm(live_decoder_t *_decoder, const char *lmname);


/** Read LM from a file. 
    @param _decoder Pointer to the decoder. 
    @param lmfile LM file name. 
    @param lmname LM name associated with this file. 
    @see ld_set_lm
 */

void ld_read_lm(live_decoder_t *_decoder, 
		const char *lmfile, 
		const char *lmname
		);

#ifdef __cplusplus
}
#endif

#endif




