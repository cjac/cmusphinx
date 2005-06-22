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
 * Revision 1.1  2005/06/22  02:37:41  arthchan2003
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

#include "s3types.h"
#include "kb.h"

/**
   \file srch_debug.h
   \brief implementation of search debug mode. 
   
   ARCHAN 20050510: An empty search structure. Only used for
   debugging.  Debug mode implementation is useful when programmers
   just want to test out a logic change in srch.c but do not want to
   waste the time in running actual decoding. 

 */


int srch_debug_init(kb_t *kb,void* srch);
int srch_debug_uninit();
int srch_debug_begin();
int srch_debug_end();
int srch_debug_decode();
int srch_debug_set_lm();
int srch_debug_gmm_compute_lv1();
int srch_debug_gmm_compute_lv2();
int srch_debug_hmm_compute_lv1();
int srch_debug_hmm_compute_lv2();
int srch_debug_eval_beams_lv1 (void* srch_struct);
int srch_debug_eval_beams_lv2 (void* srch_struct);

int srch_debug_propagate_graph_ph_lv1();
int srch_debug_propagate_graph_wd_lv1();
int srch_debug_propagate_graph_ph_lv2();
int srch_debug_propagate_graph_wd_lv2();

int srch_debug_compute_heuristic(void *srch, int32 win_efv);
int srch_debug_frame_windup(void *srch_struct,int32 frmno);
int srch_debug_shift_one_cache_frame(void *srch,int32 win_efv);
int srch_debug_select_active_gmm(void *srch);
int srch_debug_rescoring(void* srch, int32 frmno);






