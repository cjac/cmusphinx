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
 *  Aug 19, 2004
 *    Created by Yitao Sun (yitao@cs.cmu.edu).  This is the asynchronous
 *    version of the live-decode API.  It is meant for multi-threaded or server
 *    applications where the decoding process must communicate with the data-
 *    collection process indirectly.  It is NOT meant to decode multiple
 *    sessions simultaneously.
 *
 *    Because there is no standard C library for threading and locking, the
 *    current implementation only support WIN32 and POSIX Thread library.
 */

#ifndef __LIVE_DECODE_H
#define __LIVE_DECODE_H

#include "live_decode.h"

#ifdef __cplusplus
extern "C" {
#endif
  
typedef struct
{
  live_decoder_t ld;
  void *control_queue;
  void *return_queue;
  int32 state;
  void *mutex;
  int32 internal_cmd_ln;
} remote_decoder_t;

/**
 */
int rd_init(remote_decoder_t *decoder);

/**
 */
int rd_init_with_args(remote_decoder_t *decoder, int argc, char **argv);

/**
 */
int rd_finish(remote_decoder_t *decoder);

/**
 */
int rd_utt_begin(remote_decoder_t *decoder, char *uttid);

/**
 */
int rd_utt_end(remote_decoder_t *decoder);

/**
 */
int rd_utt_abort(remote_decoder_t *decoder);

/**
 */
int rd_utt_proc_raw(remote_decoder_t *decoder, 
		    int16 *samples,
		    int32 num_samples);

/**
 */
int rd_utt_proc_frame(remote_decoder_t *decoder, 
		      float32 **frames,
		      int32 num_frames);

/**
 */
int rd_utt_proc_feat(remote_decoder_t *decoder, 
		     float32 ***features,
		     int32 num_features);

/**
 */
int rd_utt_hyps(remote_decoder_t *decoder, char **hyp_str, hyp_t ***hyp_segs);

/**
 */
int rd_run(remote_decoder_t *decoder);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

