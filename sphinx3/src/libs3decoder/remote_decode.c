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
  RD_CTRL_BEGIN_UTT,
  RD_CTRL_END_UTT,
  RD_CTRL_RECORD_HYPS,
  RD_CTRL_PROCESS_RAW,
  RD_CTRL_PROCESS_FRAMES,
  RD_CTRL_PROCESS_FEATS,
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
  if ((decoder->mutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
    return -1;
  }

  return rd_queue_control_block(decoder, RD_CTRL_INIT, 0, 0);
}

int
rd_init_with_args(remote_decoder_t *decoder, int argc, char **argv)
{
  cmd_ln_parse(arg_def, argc, argv);
  memset(decoder, 0, sizeof(remote_decoder));
  decoder->internal_cmd_ln = 1;
  if ((decoder->mutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
    return -1;
  }

  return rd_queue_control_block(decoder, RD_CTRL_INIT, 0, 0);
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
  char *local_uttid;

  if (uttid && (local_uttid = ckd_calloc(1, strlen(uttid) + 1)) == 0) {
    return -1;
  }
  strcpy(local_uttid, uttid);

  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_BEGIN_UTT, 0, local_uttid);
  rd_unlock_internal(decoder);

  return rv;
}

int
rd_utt_end(remote_decoder_t *decoder)
{
  int rv;

  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_END_UTT, 0, 0);
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
    rv = rd_queue_control_block(decoder, RD_CTRL_END_UTT, 0, 0);
  }
  rd_unlock_internal(decoder);

  return rv;
}

int
rd_utt_proc_raw(remote_decoder_t *decoder, int16 *samples, int32 num_samples)
{
  int rv;
  int16 local_samples;

  if (samples == 0) {
    return -1;
  }

  local_samples = ckd_calloc(sizeof(int16), num_samples);
  if (local_samples == 0) {
    return -1;
  }
  memcpy(local_samples, samples, num_samples * sizeof(int16));

  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_PROC_RAW, num_samples,
			      local_samples);
  rd_unlock_internal(decoder);

  return rv;
}

int
rd_utt_proc_frame(remote_decoder_t *decoder,
		  float32 **frames,
		  int32 num_frames)
{
  int rv;
  int i;
  int frame_size;
  float32 **local_frames;

  if (frames == 0) {
    return -1;
  }

  frame_size = decoder->ld.fe->NUM_CEPSTRA;
  local_frames = ckd_calloc_2d(num_frames, frame_size, sizeof(float32));
  if (local_frames == 0) {
    return -1;
  }
  for (i = num_frames - 1; i >= 0; i--) {
    memcpy(local_frames[i], frames[i], frame_size * sizeof(float32));
  }

  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_PROC_FRAME, num_frames,
			      local_frames);
  rd_unlock_internal(decoder);

  return rv;
}

/** let's not implement this just yet */
#if 0
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
#endif

int
rd_utt_record_hyps(remote_decoder_t *decoder)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_queue_control_block(decoder, RD_CTRL_RECORD_HYPS, 0, 0);
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
rd_retrieve_hyps(remote_decoder_t *decoder, char **uttid, char **hyp_str, 
		 hyp_t ***hyp_segs)
{
  int rv;
  rd_lock_internal(decoder);
  rv = rd_dequeue_result_block(decoder, uttid, hyp_str, hyp_segs);
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

  char *uttid0;
  char *uttid1 = 0;
  char *hyp_str0;
  char *hyp_str1 = 0;
  hyp_t **hyp_segs0;
  hyp_t **hyp_segs1 = 0;
  hyp_t *hyp_seg_buffer = 0;
  int hyp_seglen = 0;
  hyp_t *h;
  int i;

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
    case RD_CTRL_INIT:
      if (d->state == RD_STATE_UNINIT && ld_init(&d->ld) == 0) {
	d->state = RD_STATE_IDLE;
      }
      break;
      
    case RD_CTRL_FINISH:
      ld_finish(&d->ld);
      d->state = RD_STATE_UNINIT;
      break;
      
    case RD_CTRL_BEGIN_UTT:
      if (d->state == RD_STATE_IDLE && ld_begin_utt(&d->ld, data) == 0) {
	d->state = RD_STATE_UTT;
      }
      break;
      
    case RD_CTRL_END_UTT:
      if (d->state == RD_STATE_UTT && ld_end_utt(&d->ld) == 0) {
	d->state = RD_STATE_IDLE;
      }
      break;
      
    case RD_CTRL_PROCESS_RAW:
      if (d->state == RD_STATE_UTT) {
	ld_process_raw(&d->ld, data, param);
      }
      break;
      
    case RD_CTRL_PROCESS_FRAMES:
      if (d->state == RD_STATE_UTT) {
	ld_process_frames(&d->ld, data, param);
      }
      break;
      
    case RD_CTRL_PROCESS_FEATS:
      if (d->state == RD_STATE_UTT) {
	ld_process_features(&d->ld, data, param);
      }
      break;

    case RD_CTRL_RECORD_HYPS:
      if (d->state == RD_STATE UTT || d->state == RD_STATE_IDLE) {
	/** retrieve the hypothesis segments and string */
	ld_retrieve_hyps(&d->ld, &hyp_str0, &hyp_segs0);
	
	/** allocate space for the utterance id */
	uttid0 = d->ld.uttid;
	if (uttid0) {
	  uttid1 = ckd_malloc(strlen(d->ld.uttid) + 1);
	}

	/** allocate space for the hypothesis string */
	if (hyp_str0) {
	  hyp_str1 = ckd_malloc(strlen(hyp_str0) + 1);
	}

	/** allocate space for the hypothesis segments */
	for (hyp_seglen = 0, h = hyp_segs0; h; h++, hyp_seglen++);
	if (hyp_seglen > 0) {
	  hyp_segs1 = ckd_calloc(hyp_seglen + 1, sizeof(hyp_t *));
	  hyp_seg_buffer = ckd_calloc(hyp_seglen, sizeof(hyp_t));
	}

	/**
	 * check whether any of the allocation failed.  if any of them did
	 * fail, free any allocated space.
	 */
	if ((uttid0 && uttid1) || (hyp_str0 && !hyp_str1) ||
	    (hyp_segs0 && (!hyp_segs1 || !hyp_seg_buffer))) {
	  if (uttid1) {
	    ckd_free(uttid1);
	  }
	  if (hyp_str1) {
	    ckd_free(hyp_str1);
	  }
	  if (hyp_segs1) {
	    ckd_free(hyp_segs1);
	  }
	  if (hyp_seg_buffer) {
	    ckd_free(hyp_seg_buffer);
	  }
	  break;
	}

	/** copy uttid, hyp_str, and hyp_segs into new storage */
	strcpy(uttid1, d->ld.uttid);
	strcpy(hyp_str1, hyp_str0);
	for (i = 0; i < hyp_seglen; i++) {
	  hyp_segs1[i] = &hyp_seg_buff[i];
	  memcpy(hyp_seg_buff, hyp_segs0[i], sizeof(hyp_t));
	}

	/** queue the result */
	rd_queue_result(rd, uttid1, hyp_str1, hyp_segs1);
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

  /** Win32 specific code */
  CloseHandle((HANDLE)d->mutex);

  return rv;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

int
rd_lock_internal(remote_decoder_t *decoder)
{
  /** Win32 specific code */
  if (decoder->mutex) {
    WaitForSingleObject(decoder->mutex, INFINITE);
  }

  return 0;
}

int
rd_unlock_internal(remote_decoder_t *decoder)
{
  /** Win32 specific code */
  if (decoder->mutex) {
    ReleaseMutex(decoder->mutex);
  }

  return 0;
}

int
rd_queue_control(remote_decoder_t *decoder, int32 cmd, int32 param, void *data)
{
  control_block *cb;

  if ((cb = ckd_calloc(1, sizeof(control_block))) == 0) {
    return -1;
  }
  
  *cb = { cmd, param, data, decoder->control_queue == 0 ?
	  cb : (struct _control_block *)decoder->control_queue };
  decoder->control_queue = cb;
  return 0;
}

int
rd_dequeue_control(remote_decoder_t *decoder, int32 cmd, int32 *param,
		   void **data)
{
}

int
rd_queue_result(remote_decoder_t *decoder, char *uttid, char *hyp_str,
		hyp_t **hyp_segs)
{
  result_block *rb = 0;
      
  if ((rb = ckd_calloc(1, sizeof(result_block))) == 0) {
    return -1;
  }
  *rb = { uttid, hyp_str, hyp_segs, decoder->result_queue == 0 ?
	  cb : (struct _result_block *)decoder->result_queue };
  decoder->result_queue = rb;

  return rv;
}

int
rd_dequeue_result(remote_decoder_t *decoder, char **uttid, char **hyp_str,
		  hyp_t ***hyp_segs)
{
}
