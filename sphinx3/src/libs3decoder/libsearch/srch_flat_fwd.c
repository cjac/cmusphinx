/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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

/* srch_flat_fwd.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.1.2.1  2005/07/24  01:40:37  arthchan2003
 * (Incomplete) The implementation of flat-lexicon decoding.
 * 
 *
 *
 */

#include "srch_flat_fwd.h"
#include "srch.h"
int srch_FLAT_FWD_init(kb_t *kb, /**< The KB */
		       void* srch_struct /**< The pointer to a search structure */
		       )
{
  return SRCH_SUCCESS;

}

int srch_FLAT_FWD_uninit(void* srch_struct)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_begin(void* srch_struct)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_end(void* srch_struct)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_decode(void* srch_struct)
{
  return SRCH_SUCCESS;

}

int srch_FLAT_FWD_set_lm(void* srch_struct, const char* lmname)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_add_lm(void* srch, lm_t *lm, const char *lmname)
{
  return SRCH_SUCCESS;

}
int srch_FLAT_FWD_delete_lm(void* srch, const char *lmname){  return SRCH_SUCCESS;
}

int srch_FLAT_FWD_gmm_compute_lv2(void* srch_struct, float32 *feat, int32 time){return SRCH_SUCCESS;}
int srch_FLAT_FWD_hmm_compute_lv1(void* srch_struct){return SRCH_SUCCESS;}
int srch_FLAT_FWD_hmm_compute_lv2(void* srch_struct, int32 frmno){return SRCH_SUCCESS;}
int srch_FLAT_FWD_eval_beams_lv1 (void* srch_struct){return SRCH_SUCCESS;}
int srch_FLAT_FWD_eval_beams_lv2 (void* srch_struct){return SRCH_SUCCESS;}
int srch_FLAT_FWD_propagate_graph_ph_lv1(void* srch_struct){return SRCH_SUCCESS;}
int srch_FLAT_FWD_propagate_graph_wd_lv1(void* srch_struct){return SRCH_SUCCESS;}

int srch_FLAT_FWD_propagate_graph_ph_lv2(void* srch_struct, int32 frmno){return SRCH_SUCCESS;}
int srch_FLAT_FWD_propagate_graph_wd_lv2(void* srch_struct, int32 frmno){return SRCH_SUCCESS;}
int srch_FLAT_FWD_rescoring(void *srch, int32 frmno){return SRCH_SUCCESS;}

int srch_FLAT_FWD_compute_heuristic(void *srch, int32 win_efv){return SRCH_SUCCESS;}
int srch_FLAT_FWD_frame_windup(void *srch_struct,int32 frmno){return SRCH_SUCCESS;}
int srch_FLAT_FWD_shift_one_cache_frame(void *srch,int32 win_efv){return SRCH_SUCCESS;}
int srch_FLAT_FWD_select_active_gmm(void *srch){return SRCH_SUCCESS;}

