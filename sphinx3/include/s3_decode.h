/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

#include <sphinxbase/cmd_ln.h>
#include <sphinxbase/fe.h>

#include "s3types.h"
#include "sphinx3_export.h"
#include "kb.h"
#include "kbcore.h"
#include "dag.h"
#include "search.h"

#ifndef __S3_DECODE_H
#define __S3_DECODE_H

/** \file s3_decode.h
 * \brief header for live mode decoding API 
 */
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

S3DECODER_EXPORT
extern arg_t S3_DECODE_ARG_DEFS[];

#define S3_DECODE_SUCCESS			0
#define S3_DECODE_ERROR_OUT_OF_MEMORY		-0x01
#define S3_DECODE_ERROR_NULL_POINTER		-0x02
#define S3_DECODE_ERROR_INVALID_STATE		-0x04
#define S3_DECODE_ERROR_INTERNAL		-0x08

#define S3_DECODE_STATE_IDLE			0
#define S3_DECODE_STATE_DECODING		1
#define S3_DECODE_STATE_FINISHED		2

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
    int32 state;

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
     * Boolean indicates whether we will internally swap the samples. 
     */
    int32 swap;

    /**
     * Boolean indicates whether a partial hypothesis will be dumped. 
     */
    int32 phypdump;

    /**
     * Extension for the raw director 
     */
    const char* rawext;

} s3_decode_t;


/** Initializes a Sphinx3 decoder object (re-entrant).  Internal
    modules, eg. search algorithms, language model, accoustic model,
    etc, are read from file and initialized.  The decoder internal
    variables are set to a starting state.

    This version of the Sphinx3 decoder assumes the user has
    externally parsed arguments using <I>cmd_ln_parse_r()</I> or
    <I>cmd_ln_parse_file_r()</I>.  The user is responsible for calling
    <I>cmd_ln_free_r()</I> when he/she is done with the decoder.

    @param _decode Pointer to the decoder.
    @param _config Pointer to the command-line object
                  returned by <i>cmd_ln_parse_r()</i>.
    @return 0 for success.  -1 for failure.
*/
S3DECODER_EXPORT
int s3_decode_init(s3_decode_t *_decode, cmd_ln_t *_config);

/** Wraps up the Sphinx3 decoder.  All internal modules are closed or unloaded.
    Internal variables are freed and/or set to a finishing state.  This
    function should be called once the user is finished with the Sphinx3
    decoder.

    @param _decode Pointer to the decoder.
    @see s3_decode_init
*/
S3DECODER_EXPORT
void s3_decode_close(s3_decode_t *_decode);

/** Marks the start of the current utterance.  An utterance is a session of
    speech decoding that starts with a call to <I>s3_decode_begin_utt()</I> and
    ends with a call to <I>s3_decode_end_utt()</I>.
    In the duration of an utterance, speech data is processed with either
    <I>s3_decode_process_raw()</I> or
    <I>s3_decode_process_ceps(}</I>.  Decoding
    results (hypothesis) can be retrieved any time after the start of an
    utterance using <I>s3_decode_hypothesis()</I>.
    All previous results will be clobbered at the start of a new utterance.

    At the moment, there is an undocumented time limit to the length of an
    utterance.  (Yitao: there is?)

    @param _decode Pointer to the decoder.
    @param _uttid Utterance ID string.  If <I>null</I>, a somewhat unique 
    utterance id will be generated instead.
    @return 0 for success.  -1 for failure.
    @see s3_decode_end_utt
    @see s3_decode_process
    @see s3_decode_hypothesis
*/
S3DECODER_EXPORT
int s3_decode_begin_utt(s3_decode_t *_decode, char *_uttid);

/** Marks the end of the current utterance.  The Sphinx3 decoder  can no longer
    process speech data until the start of the next utterance.  Any hypothesis
    retrieved prior to the end of the utterance is called a partial hypothesis.
    Any hypothesis retrieved after the end of the utterance is called the final
    hypothesis.  See <I>s3_decode_hypothesis()</I>
    on how to retrieve hypothesis.

    @param _decode Pointer to the decoder
    @see s3_decode_begin_utt
    @see s3_decode_process
    @see s3_decode_hypothesis
*/
S3DECODER_EXPORT
void s3_decode_end_utt(s3_decode_t *_decode);

/** Process a buffer of cepstrum frames for the current utterance.  This 
    function has to be called in the duration of an utterance.  That is, in
    between calls to <I>s3_decode_begin_utt()</I>
    and <I>s3_decode_end_utt()</I>.

    One common issue with Sphinx3 decoder is the mismatch of parameters to
    the signal processor and accoustic model.  Please double check with the
    accoustic model training scripts and your signal processing front-end to
    make sure the cepstrals are generated consistently.

    @param _decode Pointer to the decoder.
    @param _frames Buffer of audio feature frames.
    @param _num_frames Number of frames in the buffer.
    @return 0 for success.  -1 for failure.
    @see s3_decode_begin_utt
    @see s3_decode_end_utt
    @see s3_decode_process_ceps
*/
S3DECODER_EXPORT
int s3_decode_process(s3_decode_t *_decode, 
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
    such filtering in the word segment result.

    Here is an example on how to use the result returned by
    <I>s3_decode_hypothesis</I>:

    <PRE>
    s3_decode_t d;
    char *str, *uttid;
    hyp_t **segs;

    ...

    s3_decode_hypothesis(&d, &uttid, &str, &segs);
    printf("Decoded string: %s\n", str);
    for (; *segs; segs++) {
    printf("Word-segment id: %i\n", (*segs)->id);
    }
    </PRE>
    
    @param _decode Pointer to the decoder.
    @param _uttid Pointer to utterance ID string.
    @param _hyp_str Return pointer to a READ-ONLY string.  If <I>null</I>,
    the string is not returned.
    @param _hyp_segs Return pointer to a null-terminated array of word
    segments.  If <I>null</I>, the array is not returned.
    @return 0 for success.  -1 for failure.
*/
S3DECODER_EXPORT
int s3_decode_hypothesis(s3_decode_t *_decode, char **_uttid,
                         char **_hyp_str, hyp_t ***_hyp_segs);

/** Retrieve a word graph of final hypothesis.  You must call
 * s3_decode_end_utt() before this.  See {@link dag.h} and {@link
 * astar.h} for information on what to do with this structure.
 *
 * @param _decode Pointer to the decoder.
 * @return A dag_t structure, or NULL on failure.  This pointer
 * becomes invalid after a call to s3_decode_begin_utt().
 */
S3DECODER_EXPORT
dag_t *s3_decode_word_graph(s3_decode_t *_decode);

/** Set LM 
    @param _decode Pointer to the decode
    @param lmname the language model name
    @see s3_decode_read_lm s3_decode_delete_lm
*/
S3DECODER_EXPORT
void s3_decode_set_lm(s3_decode_t *_decode, const char *lmname);

/** Delete LM 
    @param _decode Pointer to the live mode decode
    @param lmname the language model name 
    @see s3_decode_set_lm s3_decode_read_lm
*/
S3DECODER_EXPORT
void s3_decode_delete_lm(s3_decode_t *_decode, const char *lmname);


/** Read LM from a file. 
    @param _decode Pointer to the decoder. 
    @param lmfile LM file name. 
    @param lmname LM name associated with this file. 
    @see s3_decode_set_lm
*/
S3DECODER_EXPORT
void s3_decode_read_lm(s3_decode_t *_decode,
                       const char *lmfile, 
                       const char *lmname);

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif




