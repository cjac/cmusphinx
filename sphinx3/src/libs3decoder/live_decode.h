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
   * The frame number at which the hypothesis is recorded.
   */
  int32 hyp_frame_num;

  /*
   * Hypothesis string.  Result (or partial result) of the recognition is
   * stored as a complete string.
   */
  char *hyp_str;

  /*
   * Length of the hypothesis string above.
   */
  int32 hyp_strlen;

  /*
   * Hypothesis word segments.  Result (or partial result) of the recognition
   * is stored as word segments.
   */
  hyp_t **hyp_segs;

  /*
   * Number of word/hypothesis segments.
   */
  int32 hyp_seglen;

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

/*
 * Initialize thes live-decoder.  Assumes the user has externally allocated
 * and parsed arguments by calling cmd_ln_parse() or cmd_ln_parse_file().  The
 * user is responsible for calling cmd_ln_free() when he/she is done with
 * the live decoder.
 *
 * A better alternative is to wrap the parsed arguments in a structure and pass  * it to ld_init().  It makes the use of cmd_ln interface much more explicit.
 * It would also allow finer control over decoder parameters, in case if any
 * user want to have separate sets of decoder parameters.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_init(live_decoder_t *decoder);

/*
 * Initializes the live-decoder.  The users who call ld_init_with_args might
 * not want to deal with the cmd_ln interface at all.  This initialization
 * function allocates and parses arguments internally, and frees them later
 * when ld_finish() is called.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_init_with_args(live_decoder_t *decoder, int argc, char **argv);

/*
 * Wraps up the live-decoder and frees up allocated memory in the process.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_finish(live_decoder_t *decoder);

/*
 * Starts decoding an utterance.  This function has to be called before any
 * calls to ld_utt_proc_*() and ld_utt_hyps() functions.  
 *
 * Arguments:
 *   char *uttid - The utterance id, or utterance label, for the next
 *   utterance.  If null, then a generated id/label will be applied to the 
 *   utterance.  !!! NOT IMPLEMENTED AT ALL !!!
 * 
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_begin(live_decoder_t *decoder, char *uttid);

/*
 * Finishes decoding the current utterance.  This function can be used to abort
 * the decoding as well.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_end(live_decoder_t *decoder);

/*
 * Process raw 16-bit samples for the current utterance decoding.  This is the
 * most basic of ld_utt_proc_*() functions.  Audio data is passed directly to
 * the decoder for processing.
 *
 * Arguments:
 *   int16 *samples - A buffer of audio sample.
 *   int32 num_samples - Number of samples in the buffer above.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_proc_raw(live_decoder_t *decoder, 
		    int16 *samples,
		    int32 num_samples);

/*
 * Process a buffer of framed data for the current utterance decoding.  
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_proc_frame(live_decoder_t *decoder, 
		      float32 **frames,
		      int32 num_frames);

/*
 * Process a block of feature vectors for the current utterance decoding.  The
 * user is responsible for converting audio samples to feature vectors using
 * Sphinx3 front-end.  This might be the case when the user is interested in
 * the feature vectors themselves.
 *
 * Arguments:
 *   float32 ***features - An array of 2-dimensional feature vectors.
 *   int32 num_features - Number of entries in the array above.
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
 *   char **hyp_str - A pointer to a READ-ONLY string.
 *   hyp_t ***hyp_segs - A pointer to a list of word segments.
 *
 * Return value:
 *   0 for success.  -1 for failure.
 */
int ld_utt_hyps(live_decoder_t *decoder, char **hyp_str, hyp_t ***hyp_segs);


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

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

