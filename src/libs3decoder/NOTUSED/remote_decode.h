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
 *  Aug 19, 2004
 *    Created by Yitao Sun (yitao@cs.cmu.edu).  This is the asynchronous
 *    version of the live-decode API.  It is meant for multi-threaded or server
 *    applications where the decoding process must communicate with the data-
 *    collection process indirectly.  It is NOT meant to decode multiple
 *    sessions simultaneously.
 *
 *    Because there is no standard C library for threading and locking, the
 *    current implementation only support WIN32 and POSIX Thread library.
 *
 *  Aug 23, 2004
 *    Modified by Yitao Sun (yitao@cs.cmu.edu).  Added rd_join() and
 *    rd_interrupt().
 */

#ifndef __REMOTE_DECODE_H
#define __REMOTE_DECODE_H

#include "live_decode.h"
#include <libutil/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _remote_decoder_t
{
  live_decoder_t ld;
  list_t *control_queue;
  list_t *result_queue;
  int32 state;
  int32 internal_cmd_ln;
} remote_decoder_t;

#define RD_SUCCESS	0
#define RD_FAILURE	-1

/**
 * Informs the remote-decoder to initialize and begin a new session.
 */
remote_decoder_t* rd_init();

/**
 * Parses the arguments, then informs the remote-decoder to initialize and
 * begin a new session.
 */
remote_decoder_t* rd_init_with_args(int argc, char **argv);

/**
 * Informs the remote-decoder to finish the session.
 */
int rd_finish(remote_decoder_t *decoder);

/**
 * Informs the remote-decoder to begin utterance decoding.
 */
int rd_begin_utt(remote_decoder_t *decoder, char *uttid);

/**
 * Informs the remote-decoder to end utterance decoding.
 */
int rd_end_utt(remote_decoder_t *decoder);

/**
 * Informs the remote-decoder to abort utterance decoding.
 */
int rd_abort_utt(remote_decoder_t *decoder);

/**
 * Informs the remote-decoder there is new raw data for utterance decoding.
 */
int rd_process_raw(remote_decoder_t *decoder, int16 *samples, int32 len);

/**
 * Informs the remote-decoder there is new framed data for utterance decoding.
 */
int rd_process_ceps(remote_decoder_t *decoder, float32 **ceps, int32 len);

/**
 * Informs the remote-decoder there is new feature vectors for utterance
 * decoding.
 */
int rd_process_feats(remote_decoder_t *decoder, float32 ***feats, int32 len);

/**
 * Informs the remote-decoder to record the hypothesis (at the current stage
 * of decoding).
 */
int rd_record_hyps(remote_decoder_t *decoder);

/**
 * Retrieves the oldest results from the result queue.
 */
int rd_retrieve_hyps(remote_decoder_t *decoder, char **uttid, char **hyp_str,
		     hyp_t ***hyp_segs);

/**
 * Free the results retrieved by rd_retrieve_hyps.
 */
int rd_free_hyps(char *uttid, char *hyp_str, hyp_t **hyp_segs);

/**
 * Main function for decoding.  Should be run on a separate thread.
 */
void rd_run(remote_decoder_t *decoder);
  
/**
 * Informs the remote-decoder to completely stop when it is finished executing
 * all of the previous commands.
 */
int rd_join(remote_decoder_t *decoder);

/**
 * Informs the remote-decoder to completely stop as soon as possible.
 */
int rd_interrupt(remote_decoder_t *decoder);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

