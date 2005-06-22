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

/* srch_word_switch_tree.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2005/06/22  08:00:09  arthchan2003
 * Completed all doxygen documentation on file description for libs3decoder/libutil/libs3audio and programs.
 * 
 * Revision 1.1  2005/06/22 02:45:52  arthchan2003
 * Log. Implementation of word-switching tree. Currently only work for a
 * very small test case and it's deliberately fend-off from users. Detail
 * omitted.
 *
 * Revision 1.9  2005/05/11 06:10:39  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.8  2005/05/03 04:09:10  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. Word condition tree search. Aka lexical tree copies. 
 */

/** \file srch_word_switch_tree.h
    \brief Implementation of word-switching tree search. 
 */

#include "s3types.h"
#include "kb.h"

typedef struct {
  int32 n_static_lextree;	/**< Number of static lexical tree for word switching  */

  lextree_t *curroottree;        /**< The current unigram tree that used
                                  in the search for this utterance. */

  lextree_t *fillertree;         /**< The filler tree which is unique. (Should it be?) */

  lextree_t **expandtree;         /**< The expanded trees */
  lextree_t **roottree;           /**< The pool of trees that stores all
                                   word trees. An array with dimension. #lm*/

  hash_table_t *active_word;    /**< Hash table that map word end to the index of the expandtree. */


  int32 no_active_word;         /**< No of active word */
  
  lmset_t* lmset;               /**< The LM set */
  int32 isLMLA;  /**< Is LM lookahead used?*/

  histprune_t *histprune; /**< Structure that wraps up parameters related to  */
  
} srch_WST_graph_t ;

int srch_WST_init(kb_t *kb, /**< The KB */
		  void* srch_struct /**< The pointer to a search structure */
		  );

int srch_WST_uninit(void* srch_struct);
int srch_WST_begin(void* srch_struct);
int srch_WST_end(void* srch_struct);
int srch_WST_decode(void* srch_struct);
int srch_WST_set_lm(void* srch_struct, const char* lmname);
int srch_WST_gmm_compute_lv2(void* srch_struct, float32 *feat, int32 time);
int srch_WST_hmm_compute_lv1(void* srch_struct);
int srch_WST_hmm_compute_lv2(void* srch_struct, int32 frmno);
int srch_WST_eval_beams_lv1 (void* srch_struct);
int srch_WST_eval_beams_lv2 (void* srch_struct);
int srch_WST_propagate_graph_ph_lv1(void* srch_struct);
int srch_WST_propagate_graph_wd_lv1(void* srch_struct);

int srch_WST_propagate_graph_ph_lv2(void* srch_struct, int32 frmno);
int srch_WST_propagate_graph_wd_lv2(void* srch_struct, int32 frmno);
int srch_WST_rescoring(void *srch, int32 frmno);

int srch_WST_compute_heuristic(void *srch, int32 win_efv);
int srch_WST_frame_windup(void *srch_struct,int32 frmno);
int srch_WST_shift_one_cache_frame(void *srch,int32 win_efv);
int srch_WST_select_active_gmm(void *srch);

