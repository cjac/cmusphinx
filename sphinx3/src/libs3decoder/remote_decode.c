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
* Aug 19, 2004
*   Created by Yitao Sun (yitao@cs.cmu.edu).  This is the implementation of
*   asynchronous remote-decode API.  It is written on top of the synchronous
*   live-decode API, with an additional state machine and control queue.
*/

#include <libutil/libutil.h>
#include "remote_decode.h"
#include "args.h"

enum {
  RD_STATE_UNINIT = 0,
  RD_STATE_IDLE,
  RD_STATE_UTT
};

enum {
  RD_CTRL_INIT,
  RD_CTRL_FINISH,
  RD_CTRL_UTT_BEGIN,
  RD_CTRL_UTT_END,
  RD_CTRL_PROC_RAW,
  RD_CTRL_PROC_FRAME,
  RD_CTRL_PROC_FEAT,
  RD_CTRL_JOIN,
};

typedef struct _control_block
{
  int32 cmd;
  int32 param;
  void* data;
  struct _control_block *next_block;
} control_block;

typedef struct _return_block
{
  char *uttid;
  char *hyp_str;
  int32 hyp_strlen;
  hyp_t *hyp_segs;
  int32 hyp_seglen;
  struct _return_block *next_block;
} return_block;

int
rd_lock_internal(remote_decoder_t *decoder);

int
rd_unlock_internal(remote_decoder_t *decoder);

int
rd_queue_control(remote_decoder_t *decoder, int32 cmd, int32 param,
		 void *data);

int
rd_dequeue_control(remote_decoder_t *decoder, int32 cmd, int32 *param,
		   void **data);

int
rd_queue_result(remote_decoder_t *decoder, char *uttid, char *hyp_str,
		hyp_t *hyp_segs);

int
rd_dequeue_result(remote_decoder_t *decoder, char **uttid, char **hyp_str,
		  hyp_t **hyp_segs);

int
rd_init(remote_decoder_t *decoder)
{
  memset(decoder, 0, sizeof(remote_decoder));
  decoder->internal_cmd_ln = 0;
  return rd_queue_control_block(decoder, RD_CTRL_INIT, 0, 0);
}

int
rd_init_with_args(remote_decoder_t *decoder, int argc, char **argv)
{
  int rv;
  cmd_ln_parse(arg_def, argc, argv);
  rv = rd_init(decoder);
  decoder->internal_cmd_ln = 1;
  return rv;
}

int
rd_finish(remote_decoder_t *decoder)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_FINISH, 0, 0);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_begin(remote_decoder_t *decoder, char *uttid)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_UTT_BEGIN, 0, uttid);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_end(remote_decoder_t *decoder)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_UTT_END, 0, 0);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_abort(remote_decoder_t *decoder)
{
  int rv = 0;
  rd_lock_internal(decoder);
  if (rd->state == RD_STATE_UTT) {
    while (rd_dequeue_control_block(decoder, 0, 0, 0) == 0);
    rv = rd_queue_control_block(decoder, RD_CTRL_UTT_END, 0, 0);
  }
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_proc_raw(remote_decoder_t *decoder, int16 *samples, int32 num_samples)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_PROC_RAW, num_samples, samples);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_proc_frame(remote_decoder_t *decoder,
		  float32 **frames,
		  int32 num_frames)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_PROC_FRAME, num_frames, frames);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_proc_feat(remote_decoder_t *decoder,
		 float32 ***features,
		 int32 num_features)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_PROC_FEAT, num_features,
			      features);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_utt_hyps(remote_decoder_t *decoder, char **uttid, char **hyp_str, 
	    hyp_t ***hyp_segs)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_dequeue_result_block(decoder, uttid, hyp_str, hyp_segs);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_join(remote_decoder_t *decoder)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_JOIN, 0, 0);
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_interrupt(remote_decoder_t *decoder)
{
  int rv = 0;
  rd_lock_internal(decoder);
  if (rd->state == RD_STATE_UTT) {
    while (rd_dequeue_control_block(decoder, 0, 0, 0) == 0);
    rv = rd_queue_control_block(decoder, RD_CTRL_JOIN, 0, 0);
  }
  rd_unlock_internal(decoder);
  return rv;
}

int
rd_run(remote_decoder_t *d)
{
  int rv;
  int32 cmd;
  int32 param;
  void *data;

  while (1) {
    rd_lock_internal(d);
    rv = rd_dequeue_control_block(d, &cmd, &param, &data);
    rd_unlock_internal(d);

    /** dequeue timed out */
    if (rv != 0) {
      continue;
    }

    /** thread is asked to join */
    if (cmd == RD_CTRL_JOIN) {
      if (d->state == RD_STATE_UTT) {
	ld_end_utt(&d->ld);
	ld_finish(&d->ld);
      }
      if (d->state == RD_STATE_IDLE) {
	ld_finish(&d->ld);
      }
      break;
    }

    switch (cmd) {
    case RD_CTRL_INIT: {
      if (d->state == RD_STATE_UNINIT && ld_init(&d->ld) == 0) {
	d->state = RD_STATE_IDLE;
      }
    }
    break;
      
    case RD_CTRL_FINISH: {
      ld_finish(&d->ld);
      d->state = RD_STATE_UNINIT;
    }
    break;
      
    case RD_CTRL_UTT_BEGIN: {
      if (d->state == RD_STATE_IDLE && 
	  ld_utt_begin(&d->ld, (char *)data) == 0) {
	d->state = RD_STATE_UTT;
      }
    }
    break;
      
    case RD_CTRL_UTT_END: {
      if (d->state == RD_STATE_UTT && ld_utt_end(&d->ld) == 0) {
	d->state = RD_STATE_IDLE;
      }
    }
    break;
      
    case RD_CTRL_PROC_RAW: {
      if (d->state == RD_STATE_UTT) {
	d_utt_proc_raw(&d->ld, (int16 *)data, param);
      }
    }
    break;
      
    case RD_CTRL_PROC_FRAME: {
      if (d->state == RD_STATE_UTT) {
	ld_utt_proc_frame(&d->ld, (float32 **)data, param);
      }
    }
    break;
      
    case RD_CTRL_PROC_FEAT: {
      if (d->state == RD_STATE_UTT) {
	ld_utt_proc_feat(&d->ld, (float32 ***)data, param);
      }
    }
    break;
    }

    if (data) {
      ckd_free(data);
    }
  }

  rd_lock_internal(d);
  while (rd_dequeue_control_block(d, 0, 0, 0) == 0);
  rd_unlock_internal(d);

  return rv;
}

