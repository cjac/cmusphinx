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

/* srch_time_switch_tree.h
 * HISTORY
 * $Log$
 * Revision 1.1  2005/06/22  02:44:02  arthchan2003
 * Log. Time-switching tree search (aka Ravi's search or Mode 4) is a
 * wrapped up of the search we had before Sphinx 3.6.  It is not a
 * conventional way to do a lexical tree search and it provide three
 * interesting approach in solving the problem of cross-word triphone,
 * word segmentation and languag model.
 * 
 * Though, the functions are now wrapped up using either srch interface,
 * the performance of the code has not changed.  We deliberately keep
 * this search because we knew that the new search (mode 5) could be
 * risky.
 * 
 * Revision 1.8  2005/05/11 00:18:46  archan
 * Add comments on srch.h and srch_time_switch_tree.h and srch_debug.h on how things work. A very detail comment is added in srch.h to describe how generally srch_t is interacting with other parts of the code.
 *
 * 
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. Time switching tree implementation. 
 */


#ifndef _SRCH_TST_H_
#define _SRCH_TST_H_

#include "s3types.h"
#include "kb.h"
#include "lextree.h"
#include "lm.h"

/** 
    \file srch_time_switch_tree.h 
    \brief implementation of Sphinx 3.5 search (aka s3 fast). 

    ARCHAN 20050324: The time condition tree search structure.  The
    structure is reserved for legacy reason and it is explained as
    follows. During the development of Sphinx 2 (pre-1994), we (folks
    in CMU) already realized that using single unigram lexical tree
    will cause so called word segmentation error.  I.e. In bigram
    search, words with different word begins will be collected and
    treated as one.

    At the time of Sphinx 3.1 (~1998-1999) (the first attempt to build
    a fast CDHMM decoder in CMU Sphinx group), Ravi Mosur considered
    this problem and would like to solve this by introducing copies of
    tree.  The standard method to do it was to introduce
    word-conditioned tree copies which probably first introduced by
    the RWTH group. Without careful adjustment of the allocation of
    tree, this implementation method will requires (w+1) * lexical
    trees in search.  Where w is the number of words in the search
    architecture. 

    Therefore, he decided to use an interesting way to deal with the
    word segmentation problem (and I personally feel that it is very
    innovative).  What he did was instead of making word-conditioned
    tree copies.  He used time-conditioned tree copies. That is to
    say, at every certain frame, he will switch lexical tree in the
    search.  This results in higher survival chance of tokens with an
    early low score and allows Ravi to create a recognizer which has a
    difference only relatively 10% from the s3.0 decode_anytopo. 

    I usually refer this method as "lucky wheel" search because 1)
    there is no principled reason why the switching of tree should be
    done in this way. 2) The switching is done without considering the
    maximum scores. 3) It might well be possible that some word ends
    are active but they are just not "lucky" enough to enter next
    tree.  So, it might amaze you that this search was used and it only
    give 10% relative degradation from s3 slow. 

    To understand why this search works, one could think in this
    way. (i.e.  this is my opinion.).  When it boils down to the
    highest scoring paths in a search. People observed that the time
    information and word information each gives a relative good guess
    to tell whether a path should be preserved or not.  This is
    probably the reason in one version of IBM stack decoder, they
    actually rescoring the priority queue by considering whether a
    word has earlier word-begin.  RWTH's group also has one journal
    paper on making use of the time information.  

    The "lucky wheel" search is one type of search that tries to
    tackling word segmentation problem using time information. It did
    a reasonably good job. Of course, it will still be limited by the
    fact time information is still poorer than the word information as 
    a heuristic measure. 

    This method is remained in sphinx 3.6 for three reasons.  First, I
    personally think that there might be ways to trade-off his method
    and conventional tree copies method. This could result in a better
    memory/accuracy/speed trade-off. Second, I actually don't know
    whether mode 5 (word copies searrch ) will work well in
    general. Last but not least, I preserve the search because of my
    respect to Ravi. :-)
  */

typedef struct {

  /**
   * There can be several unigram lextrees.  If we're at the end of frame f, we can only
   * transition into the roots of lextree[(f+1) % n_lextree]; same for fillertree[].  This
   * alleviates the problem of excessive Viterbi pruning in lextrees.
   */

  int32 n_lextree;		/**< Number of lexical tree for time switching: n_lextree */
  lextree_t **curugtree;        /**< The current unigram tree that used in the search for this utterance. */

  lextree_t **ugtree;           /**< The pool of trees that stores all word trees. */
  lextree_t **fillertree;       /**< The pool of trees that stores all filler trees. */
  int32 n_lextrans;		/**< #Transitions to lextree (root) made so far */
  int32 epl ;                   /**< The number of entry per lexical tree */

#if 0
  lextree_t **ugtreeMulti;  /** This data structure allocate all trees for all LMs specified by the users */
  /** Because the name is confusing, in sphinx 3.6, ugtree and ugtreeMulti have changed their name to curugtree and ugtree. */
#endif
  
  lmset_t* lmset;               /**< The LM set */
  int32 isLMLA;  /**< Is LM lookahead used?*/

  histprune_t *histprune; /**< Structure that wraps up parameters related to  */
  


} srch_TST_graph_t ;

int srch_TST_init(kb_t *kb,  /**< The KB. */
		  void* srch_struct /** The pointer to a search structure. */
		  );
int srch_TST_uninit(void *srch);
int srch_TST_begin(void *srch);
int srch_TST_end(void * srch);
int srch_TST_decode(void *srch);

int srch_TST_set_lm(void* srch,const char* lmname);
int srch_TST_add_lm(void* srch, lm_t *lm, const char *lmname);
int srch_TST_delete_lm(void* srch, const char *lmname);

int srch_TST_gmm_compute_lv2(void *srch, float32 *feat, int32 time);
int srch_TST_hmm_compute_lv1(void *srch);
int srch_TST_hmm_compute_lv2(void *srch, int32 frmno);
int srch_TST_eval_beams_lv1 (void* srch);
int srch_TST_eval_beams_lv2 (void* srch);
int srch_TST_propagate_graph_ph_lv1(void *srch_struct);
int srch_TST_propagate_graph_wd_lv1(void *srch_struct);

int srch_TST_propagate_graph_ph_lv2(void *srch_struct, int32 frmno);
int srch_TST_propagate_graph_wd_lv2(void *srch_struct, int32 frmno);

int srch_TST_compute_heuristic(void *srch, int32 win_efv);
int srch_TST_frame_windup(void *srch_struct,int32 frmno);
int srch_TST_shift_one_cache_frame(void *srch,int32 win_efv);
int srch_TST_select_active_gmm(void *srch);
int srch_TST_rescoring(void *s,int32 frmno);

#endif






