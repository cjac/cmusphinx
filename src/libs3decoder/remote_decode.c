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
* Aug 19, 2004
*   Created by Yitao Sun (yitao@cs.cmu.edu).  This is the implementation of
*   asynchronous remote-decode API.  It is written on top of the synchronous
*   live-decode API, with an additional state machine and control queue.
*/

#include <libutil/libutil.h>
#include "remote_decode.h"
#include "args.h"

/** for win32 threads */
#include <windows.h>

enum {
  RD_STATE_UNINIT = 0,
  RD_STATE_IDLE,
  RD_STATE_UTT
};

enum {
  RD_CTRL_NONE,
  RD_CTRL_INIT,
  RD_CTRL_FINISH,
  RD_CTRL_BEGIN_UTT,
  RD_CTRL_END_UTT,
  RD_CTRL_RECORD_HYPS,
  RD_CTRL_PROCESS_RAW,
  RD_CTRL_PROCESS_CEPS,
  RD_CTRL_PROCESS_FEATS,
  RD_CTRL_JOIN,
};

#define RD_PARAM_NONE  0
#define RD_DATA_NONE   0

typedef struct
{
  int32 cmd;
  int32 param;
  void* data;
} control_t;

typedef struct
{
  char *uttid;
  char *hyp_str;
  hyp_t **hyp_segs;
} result_t;

static control_t* create_control(remote_decoder_t *, int32, int32, void *);
static void free_control(control *);
static result_t* create_result(remote_decoder_t *, char *, char *, hyp_t **);
static int free_result(result *);
static int rd_control_impl(remote_decoder_t *, int32, int32, void *);
static int rd_init_impl(remote_decoder_t *, int32);

static int
rd_control_impl(remote_decoder_t *_decoder, int32 _cmd, int32 _param,
		void *_data)
{
  control_t *ctrl;

  assert(_decoder != null);

  ctrl = create_control(_decoder, _cmd, _param, _data);
  if (ctrl == null) {
    free_control(ctrl);
    return RD_FAILURE;
  }

  if (list_insert_tail(_decoder->control_queue, ctrl) != 0) {
    free_control(ctrl);
    return RD_FAILURE;
  }

  return RD_SUCCESS;
}

static remote_decoder_t*
rd_init_impl(int32 _internal_cmd_ln)
{
  remote_decoder_t *decoder;

  /******************************** execution *******************************/

  decoder = (remote_decoder_t *)ckd_malloc(sizeof(remote_decoder_t));
  if (decoder == null) {
    return null;
  }

  decoder->state = RD_STATE_UNINIT;
  decoder->internal_cmd_ln = _internal_cmd_ln;
  decoder->control_queue = null;
  decoder->result_queue = null;

  if ((decoder->control_queue = list_init(&free_control)) == null) {
    goto rd_init_impl_cleanup;
  }

  if ((decoder->result_queue = list_init(&free_result)) == null) {
    goto rd_init_impl_cleanup;
  }

  if (rd_control_impl(decoder, RD_CTRL_INIT, RD_PARAM_NONE, null) != 0) {
    goto rd_init_impl_cleanup;
  }

  return decoder;

  /********************************** cleanup *******************************/
  rd_init_impl_cleanup:

  if (decoder->control_queue != null) {
    list_free(_decoder->control_queue);
  }

  if (_decoder->result_queue != null) {
    list_free(_decoder->result_queue);
  }

  return null;
}

remote_decoder_t *
rd_init()
{
  return rd_init_impl(_decoder, false);
}

remote_decoder_t *
rd_init_with_args(int _argc, char **_argv)
{
  cmd_ln_parse(arg_def, argc, argv);
  return rd_init_impl(_decoder, true);
}

int
rd_finish(remote_decoder_t *_decoder)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_FINISH, RD_PARAM_NONE, null);
}

int
rd_begin_utt(remote_decoder_t *_decoder, char *_uttid)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_BEGIN_UTT, RD_PARAM_NONE, _uttid);
}

int
rd_end_utt(remote_decoder_t *_decoder)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_END_UTT, RD_PARAM_NONE, null);
}

int
rd_abort_utt(remote_decoder_t *_decoder)
{
  control_t *ctrl;

  assert(_decoder != null);

  for (;;) {
    if (list_peek_head(_decoder->control_queue, &ctrl) != 0) {
      break;
    }

    if (ctrl->cmd == RD_END_UTT) {
      return RD_SUCCESS;
    }
    else if (list_remove_head(_decoder->control_queue, null) != 0) {
      return RD_FAILURE;
    }      
  }

  return rd_control_impl(_decoder, RD_CTRL_END_UTT, RD_PARAM_NONE, null);
}

int
rd_process_raw(remote_decoder_t *_decoder, int16 *_samples, int32 _len)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_PROCESS_RAW, len, samples);
}

int
rd_process_ceps(remote_decoder_t *_decoder, float32 **_ceps, int32 _len)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_PROCESS_CEPS, _len, _ceps);
}

/** let's not implement this just yet */
#if 0
int
rd_process_feats(remote_decoder_t *_decoder, float32 ***_feats, int32 _len)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_PROCESS_FEATS, _len, _feats);
}
#endif

int
rd_record_hyps(remote_decoder_t *_decoder)
{
  assert(_decoder != null);

  return rd_control_impl(_decoder, RD_CTRL_RECORD_HYPS, RD_PARAM_NONE, null);
}

int
rd_join(remote_decoder_t *_decoder)
{
  return rd_control_impl(_decoder, RD_CTRL_JOIN, RD_PARAM_NONE, null);
}

int
rd_interrupt(remote_decoder_t *_decoder)
{
  assert(_decoder != null);

  list_clear(_decoder->control_queue);
  return rd_control_impl(_decoder, RD_CTRL_JOIN, RD_PARAM_NONE, null);
}

int
rd_retrieve_hyps(remote_decoder_t *_decoder, char **_uttid, char **_hyp_str, 
		 hyp_t ***_hyp_segs)
{
  result_t *rv = null;

  if (list_remove_head(_decoder, &rv) != 0) {
    return RD_FAILURE;
  }

  if (_uttid) {
    *_uttid = rv->uttid;
  }
  else if (rv->uttid) {
    ckd_free(rv->uttid);
  }

  if (_hyp_str) {
    *_hyp_str = rv->hyp_str;
  }
  else if (rv->hyp_str) {
    ckd_free(rv->hyp_str);
  }

  if (_hyp_segs) {
    *_hyp_segs = rv->hyp_segs;
  }
  else if (rv->hyp_segs) {
    if (rv->hyp_segs[0]) {
      ckd_free(rv->hyp_segs[0]);
    }
    ckd_free(rv->hyp_segs);
  }

  return RD_SUCCESS;
}

int
rd_free_hyps(char *uttid, char *hyp_str, hyp_t **hyp_segs)
{
  if (uttid) {
    ckd_free(uttid);
  }

  if (hyp_str) {
    ckd_free(hyp_str);
  }

  if (hyp_segs) {
    if (hyp_segs[0]) {
      ckd_free(hyp_segs[0]);
    }
    ckd_free(hyp_segs);
  }

  return 0;
}

void
rd_run(remote_decoder_t *_decoder)
{
  control_t *ctrl;
  result_t *result;

  char *uttid;
  char *hyp_str;
  hyp_t **hyp_segs;

  while (1) {
    if (list_remove_head(_decoder->control_queue, &ctrl) != 0) {
      list_wait(_decoder->control_queue, 0);
    }

    /** thread is asked to join */
    if (ctrl->cmd == RD_CTRL_JOIN) {
      if (_decoder->state == RD_STATE_UTT) {
	ld_end_utt(&_decoder->ld);
	ld_finish(&_decoder->ld);
      }
      if (_decoder->state == RD_STATE_IDLE) {
	ld_finish(&_decoder->ld);
      }
      break;
    }
    
    switch (ctrl->cmd) {case RD_CTRL_INIT:
      if (_decoder->state == RD_STATE_UNINIT && ld_init(&_decoder->ld) == 0) {
	_decoder->state = RD_STATE_IDLE;
      }
      break;
      
    case RD_CTRL_FINISH:
      ld_finish(&_decoder->ld);
      _decoder->state = RD_STATE_UNINIT;
      break;
      
    case RD_CTRL_BEGIN_UTT:
      if (_decoder->state == RD_STATE_IDLE &&
	  ld_begin_utt(&_decoder->ld, ctrl->data) == 0) {
	_decoder->state = RD_STATE_UTT;
      }
      break;
      
    case RD_CTRL_END_UTT:
      if (_decoder->state == RD_STATE_UTT && ld_end_utt(&_decoder->ld) == 0) {
	_decoder->state = RD_STATE_IDLE;
      }
      break;
      
    case RD_CTRL_PROCESS_RAW:
      if (_decoder->state == RD_STATE_UTT) {
	ld_process_raw(&_decoder->ld, ctrl->data, ctrl->param);
      }
      break;
      
    case RD_CTRL_PROCESS_FRAMES:
      if (_decoder->state == RD_STATE_UTT) {
	ld_process_ceps(&_decoder->ld, ctrl->data, ctrl->param);
      }
      break;

#if 0
    case RD_CTRL_PROCESS_FEATURES:
      if (_decoder->state == RD_STATE_UTT) {
	ld_process_features(&_decoder->ld, data, param);
      }
      break;
#endif

    case RD_CTRL_RECORD_HYPS:
      if (_decoder->state == RD_STATE_UTT || 
	  _decoder->state == RD_STATE_IDLE) {
	/** retrieve the hypothesis segments and string */
	ld_retrieve_hyps(&_decoder->ld, &hyp_str, &hyp_segs);
	/** copy the retrieved segments and string */
	result = create_result(_decoder->ld.uttid, hyp_str, hyp_segs);
	if (result == null) {
	  break;
	}
	/** queue the result */
	if (list_queue_tail(_decoder->result_queue, result) != 0) {
	  free_result(result);
	}
      }
      break;
    }

    free_control(ctrl);
  }

  list_free(_decoder->control_queue);

  return rv;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static control_t*
create_control(remote_decoder_t *_decoder, int32 _cmd, int32 _param,
	       void *_data)
{
  control_t *ctrl;
  void *data;
  int ncep;

  assert(_decoder != null);

  /******************************** execution *******************************/
  if ((ctrl = (control_t *)ckd_malloc(sizeof(control_t))) == null) {
    goto create_control_cleanup;
  }

  switch (cmd) {
  case RD_CTRL_BEGIN_UTT:
    if ((data = ckd_malloc(strlen((char *)_data) + 1)) == null) {
      goto create_control_cleanup;
    }
    strcpy((char *)data, (char *)_data);
    break;
    
  case RD_CTRL_PROCESS_RAW:
    if ((data = (int16 *)ckd_malloc(_param * sizeof(int16))) == null) {
      goto create_control_cleanup;
    }
    memcpy((int16 *)data, (int16 *)_data, _param * sizeof(int16));
    break;

  case RD_CTRL_PROCESS_CEPS:
    ncep = _decoder->ld.fe->NUM_CEPSTRA;
    data = (float32 **)ckd_calloc_2d(_param, ncep, sizeof(float32));
    if (data == null) {
      goto create_control_cleanup;
    }
    for (i = _param - 1; i >= 0; i--) {
      memcpy((float32 *)data[i], (float32 *)_data[i], ncep * sizeof(float32));
    }
    break;
  }
  
  /** return the new control_t structure */
  ctrl->cmd = _cmd;
  ctrl->param = _param;
  ctrl->data = data;

  return ctrl;

  /********************************** cleanup *******************************/
  create_control_cleanup:

  if (ctrl != null) {
    ckd_free(ctrl);
  }
  if (data != null) {
    if (_cmd == RD_CTRL_PROCESS_CEPS) {
      ckd_free_2d(data);
    }
    else {
      ckd_free(data);
    }
  }

  return null;
}

void
free_control(control_t _ctrl)
{
  if (_ctrl->data) {
    if (_ctrl->cmd == RD_CTRL_PROCESS_CEPS) {
      ckd_free_2d(ctrl->data);
    }
    else {
      ckd_free(_data);
    }
  }
  ckd_free(ctrl);
}

result_block*
create_result(char *_uttid, char *_hyp_str, hyp_t **_hyp_segs)
{
  result_t *result;
  char *uttid = null;
  char *hyp_str = null;
  hyp_t **hyp_segs = null;
  hyp_t *hyp_seg_buffer = null;
  int hyp_seglen;
  hyp_t *hyp_ptr;

  /******************************** execution *******************************/
  if ((result = (result_t *)ckd_malloc(sizeof(result_t))) == null) {
    goto create_result_cleanup;
  }
  
  /** allocate space for the utterance id (if needed) */
  if (_uttid != null) {
    if ((uttid = (char *)ckd_malloc(strlen(_uttid) + 1)) == null) {
      goto create_result_cleanup;
    }
    strcpy(uttid, _uttid);
  }

  /** allocate space for the hypothesis string */
  if (_hyp_str != null) {
    if ((hyp_str = (char *)ckd_malloc(strlen(_hyp_str) + 1)) == null) {
      goto create_result_cleanup;
    }
    strcpy(hyp_str, _hyp_str);
  }

  /** allocate space for the hypothesis segments */
  for (hyp_seglen = 0, hyp_ptr = *_hyp_segs; hyp_ptr != null;
       hyp_ptr++, hyp_seglen++);
  if (hyp_seglen > 0) {
    hyp_seg_buffer = (hyp_t *)ckd_calloc(hyp_seglen, sizeof(hyp_t));
    if (hyp_seg_buffer == null) {
      goto create_result_cleanup;
    }

    /** allocate extra spot for null-termination */
    hyp_segs = (hyp_t **)ckd_calloc(hyp_seglen + 1, sizeof(hyp_t *));
    if (hyp_segs == null) {
      goto create_result_cleanup;
    }
    /** null-terminate the hypothesis segment array */
    hyp_segs[hyp_seglen--] = null;
    for (; hyp_seglen >= 0; hyp_seglen--) {
      hyp_segs[hyp_seglen] = &hyp_seg_buffer[hyp_seglen];
      memcpy(hyp_segs[hyp_seglen], _hyp_segs[hyp_seglen], sizeof(hyp_t));
    }
  }

  /** return the new result block */
  result->uttid = uttid;
  result->hyp_str = hyp_str;
  result->hyp_segs = hyp_segs;

  return result;

  /********************************** cleanup *******************************/
  create_result_cleanup:

  if (result != null) {
    ckd_free(result);
  }
  if (uttid) {
    ckd_free(uttid);
  }
  if (hyp_str) {
    ckd_free(hyp_str);
  }
  if (hyp_segs) {
    ckd_free(hyp_segs);
  }
  if (hyp_seg_buffer) {
    ckd_free(hyp_seg_buffer);
  }

  return null;
}

void
free_result(result_t *_result)
{
  if (_result) {
    if (_result->uttid) {
      ckd_free(result->uttid);
    }
    if (result->hyp_str) {
      ckd_free(result->hyp_str);
    }
    if (result->hyp_segs) {
      if (result->hyp_segs[0]) {
	ckd_free(result->hyp_segs[0]);
      }
      ckd_free(result->hyp_segs);
    }
  }
}
