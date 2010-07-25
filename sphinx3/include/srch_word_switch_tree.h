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

/* srch_word_switch_tree.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.3  2006/02/23 16:52:56  arthchan2003
 * Merged from branch: SPHINX3_5_2_RCI_IRII_BRANCH: this fills in the code for doing tree propagation.  However, rescoring is still being used.  The code is working. However, it takes up huge amount of memory and I consider this as not elegant.   It also shows that straight forward implementation of tree copies search doesn't work even in these days.
 *
 *
 * Revision 1.1.4.5  2006/01/16 20:15:37  arthchan2003
 * 1, removed the unlinksilences part, 2, added 2nd-stage interface, but now commented.
 *
 * Revision 1.1.4.4  2005/07/07 02:41:55  arthchan2003
 * 1, Added an experimental version of tree expansion interface it the code, it does tree expansion without history pruning. Currently disabled because it used to much memory space srch_word_switch_tree.[ch].  2, Remove -lminsearch segments of code, it proves to be unnecessary. 3, Remove the rescoring interface.  In this search, WST_rescoring is actually not doing rescoring, it is rather a segment of code which collect all active word end together and input it into the viterbi history.
 *
 * Revision 1.1.4.3  2005/07/04 07:24:15  arthchan2003
 * Added some comments
 *
 * Revision 1.1.4.2  2005/06/27 05:37:05  arthchan2003
 * Incorporated several fixes to the search. 1, If a tree is empty, it will be removed and put back to the pool of tree, so number of trees will not be always increasing.  2, In the previous search, the answer is always "STOP P I T G S B U R G H </s>"and filler words never occurred in the search.  The reason is very simple, fillers was not properly propagated in the search at all <**exculamation**>  This version fixed this problem.  The current search will give <sil> P I T T S B U R G H </sil> </s> to me.  This I think it looks much better now.
 *
 * Revision 1.1.4.1  2005/06/24 21:13:52  arthchan2003
 * 1, Turn on mode 5 again, 2, fixed srch_WST_end, 3, Add empty function implementations of add_lm and delete_lm in mode 5. This will make srch.c checking happy.
 *
 * Revision 1.2  2005/06/22 08:00:09  arthchan2003
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

#ifndef _SRCH_WST_H_
#define _SRCH_WST_H_

/** \file srch_word_switch_tree.h
    \brief Implementation of word-switching tree search. 
*/

#include "s3types.h"
#include "kb.h"
#include "lm.h"
#include "lextree.h"
#include "fast_algo_struct.h"

/**
   A note by ARCHAN at 20050703

   In general, what made a LVCSR search to be unique has three major
   factors.  They are 1) how high level knowledge source (LM, FSG) is
   applied, 2) how the lexicon is organized, 3, how cross word
   triphones is realized and implemented.  It is important to realize
   the interplay between these 3 components to realize how a search
   actually really works.
   
*/


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

typedef struct {
    int32 n_static_lextree;	/**< Number of static lexical tree for word switching  */

    lextree_t *curroottree;        /**< The first unigram tree that used
                                      in the search for this utterance. */
    lextree_t **expandtree;         /**< The expanded trees */


    lextree_t *curfillertree;         /**< The first filler tree */
    lextree_t **expandfillertree;         /**< The expanded fillertrees 
                                           */



    lextree_t **roottree;           /**< The pool of trees that stores all
                                       word trees. An array with dimension. \#lm*/

    hash_table_t *active_word;    /**< Hash table that map word end to the index of the expandtree. */

    glist_t  empty_tree_idx_stack;      /**< Store a pool of indices which
                                           indicate a tree is empty. It acts like 
                                           a stack
                                        */
  

    int32 no_active_word;         /**< No of active word */
  
    lmset_t* lmset;               /**< The LM set */
    int32 isLMLA;  /**< Is LM lookahead used?*/

    histprune_t *histprune; /**< Structure that wraps up parameters related to  */


} srch_WST_graph_t ;


extern struct srch_funcs_s srch_WST_funcs;


int srch_WST_init(kb_t *kb, /**< The KB */
		  void* srch_struct /**< The pointer to a search structure */
    );

int srch_WST_uninit(void* srch_struct);
int srch_WST_begin(void* srch_struct);
int srch_WST_end(void* srch_struct);
int srch_WST_decode(void* srch_struct);

int srch_WST_set_lm(void* srch_struct, const char* lmname);
int srch_WST_add_lm(void* srch, lm_t *lm, const char *lmname);
int srch_WST_delete_lm(void* srch, const char *lmname);

int srch_WST_gmm_compute_lv2(void* srch_struct, float32 *feat, int32 time);
int srch_WST_hmm_compute_lv1(void* srch_struct);
int srch_WST_hmm_compute_lv2(void* srch_struct, int32 frmno);
int srch_WST_eval_beams_lv1 (void* srch_struct);
int srch_WST_eval_beams_lv2 (void* srch_struct);
int srch_WST_propagate_graph_ph_lv1(void* srch_struct);
int srch_WST_propagate_graph_wd_lv1(void* srch_struct);

int srch_WST_propagate_graph_ph_lv2(void* srch_struct, int32 frmno);

/**
 */

int srch_WST_propagate_graph_wd_lv2(void* srch_struct, int32 frmno);


int srch_WST_compute_heuristic(void *srch, int32 win_efv);
int srch_WST_frame_windup(void *srch_struct,int32 frmno);
int srch_WST_shift_one_cache_frame(void *srch,int32 win_efv);
int srch_WST_select_active_gmm(void *srch);

glist_t srch_WST_gen_hyp(void* srch_struct /**< A void pointer to a search structure */
    ); 

int srch_WST_dump_vithist(void* srch_struct /**< A void pointer to a search structure */
    );

int srch_WST_bestpath_impl(void * srch_struct /**< A void pointer to a search structure */
    );

#ifdef __cplusplus
}
#endif

#endif /* _SRCH_WST_H_ */
