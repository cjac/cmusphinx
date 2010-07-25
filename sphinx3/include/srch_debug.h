/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/* srch_debug.h
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.3  2006/02/28 02:06:46  egouvea
 * Updated MS Visual C++ 6.0 support files. Fixed things that didn't
 * compile in Visual C++ (declarations didn't match, etc). There are
 * still some warnings, so this is not final. Also, sorted files in
 * several Makefile.am.
 *
 * Revision 1.2  2006/02/23 15:50:25  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Fixed dox-doc, Added empty functions into srch_debug.[ch]
 *
 * Revision 1.1.4.1  2006/01/16 20:02:08  arthchan2003
 * Added interfaces for second stage operations
 *
 * Revision 1.1  2005/06/22 02:37:41  arthchan2003
 * Log. A search debugging implementation.  Users will only see a text
 * message provided in this search implementation.
 *
 * Revision 1.6  2005/05/11 06:10:39  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 *
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. Time switching tree implementation. 
 */

#ifndef _SRCH_DEBUG_H_
#define _SRCH_DEBUG_H_

#include "s3types.h"
#include "dag.h"
#include "lm.h"
#include "kb.h"

/**
   \file srch_debug.h
   \brief implementation of search debug mode. 
   
   ARCHAN 20050510: An empty search structure. Only used for
   debugging.  Debug mode implementation is useful when programmers
   just want to test out a logic change in srch.c but do not want to
   waste the time in running actual decoding. 

*/


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

extern struct srch_funcs_s srch_debug_funcs;

int srch_debug_init(kb_t *kb,void* srch);
int srch_debug_uninit(void* srch);
int srch_debug_begin(void* srch);
int srch_debug_end(void* srch);
int srch_debug_decode(void);
int srch_debug_set_lm(void* srch, const char *lmname);
int srch_debug_add_lm(void* srch, lm_t *lm, const char *lmname);
int srch_debug_delete_lm(void* srch, const char *lmname);
int srch_debug_gmm_compute_lv1(void *srch, float32 *feat, int32 cache_idx, int32 wav_idx);
int srch_debug_gmm_compute_lv2(void *srch, float32 **feat, int32 wav_idx);
int srch_debug_hmm_compute_lv1(void* srch);
int srch_debug_hmm_compute_lv2(void *srch, int32 wav_idx);
int srch_debug_eval_beams_lv1 (void* srch);
int srch_debug_eval_beams_lv2 (void* srch);

int srch_debug_propagate_graph_ph_lv1(void* srch);
int srch_debug_propagate_graph_wd_lv1(void* srch);
int srch_debug_propagate_graph_ph_lv2(void *srch, int32 wav_idx);
int srch_debug_propagate_graph_wd_lv2(void *srch, int32 wav_idx);

int srch_debug_compute_heuristic(void *srch, int32 win_efv);
int srch_debug_frame_windup(void *srch_struct, int32 frmno);
int srch_debug_shift_one_cache_frame(void *srch, int32 win_efv);
int srch_debug_select_active_gmm(void *srch);
int srch_debug_rescoring(void* srch, int32 frmno);

glist_t srch_debug_gen_hyp(void* srch_struct /**< A void pointer to a search structure */
    ); 

int srch_debug_dump_vithist(void* srch_struct /**< A void pointer to a search structure */
    );

dag_t* srch_debug_gen_dag(void * srch_struct, /**< A void pointer to a search structure */
			  glist_t hyp
    );

glist_t srch_debug_bestpath_impl(void * srch_struct, /**< A void pointer to a search structure */
				 dag_t *dag 
    );

int32 srch_debug_dag_dump(void *srch_struct, dag_t *dag);

#ifdef __cplusplus
}
#endif

#endif /* _SRCH_DEBUG_H_ */
