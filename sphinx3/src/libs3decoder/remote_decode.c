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
  RD_STATE_INIT,
  RD_STATE_IDLE,
  RD_STATE_UTT_BEGIN,
  RD_STATE_UTT_PROC,
  RD_STATE_UTT_END,
  RD_STATE_FINISH
};

enum {
  RD_CTRL_INIT,
  RD_CTRL_FINISH,
  RD_CTRL_UTT_BEGIN,
  RD_CTRL_UTT_END,
  RD_CTRL_UTT_CANCEL,
  RD_CTRL_PROC_RAW,
  RD_CTRL_PROC_FRAME,
  RD_CTRL_PROC_FEAT
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
rd_queue_control(remote_decoder_t *decoder, int32 param, void *data);

int
rd_dequeue_control(remote_decoder_t *decoder, int32 *param, void **data);

int
rd_queue_result(remote_decoder_t *decoder, char *uttid, char *hyp_str,
		int32 hyp_len, hyp_t *hyp_segs, int32 hyp_seglen);

int
rd_dequeue_result(remote_decoder_t *decoder, char **uttid, char **hyp_str,
		int32 *hyp_len, hyp_t **hyp_segs, int32 *hyp_seglen);

int
rd_init(remote_decoder_t *decoder)
{
  control_block *cb = 0;

  if ((cb = (control_block *)ckd_calloc(1, sizeof(control_block))) == 0) {
    return -1;
  }

  *cb = { RD_CTRL_INIT, 0, 0, 0 };
  if (rd_queue_control_block(decoder, cb) != 0) {
    ckd_free(cb);
    return -1;
  }

  return 0;
}

int rd_init_with_args(remote_decoder_t *decoder, int argc, char **argv)
{
  int rv = 0;

  cmd_ln_parse(arg_def, argc, argv);
  rv = rd_init(decoder);
  decoder->internal_cmd_ln = 0;
  return rv;
}


