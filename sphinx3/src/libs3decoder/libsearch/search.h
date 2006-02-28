/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
/*
 * search.h -- All exported search-related functions and data structures.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.7  2006/02/28  22:29:39  egouvea
 * Redefined hyp_t as "search_hyp_t" rather than "struct search_hyp_t".
 * 
 * Revision 1.6  2006/02/24 12:43:18  arthchan2003
 * Fixed typedef issue of hyp_t and srch_hyp_t.
 *
 * Revision 1.5  2006/02/23 15:12:09  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Introduced srch_hyp_t and conf_srch_hyp_t. The former unifies the usage of multiple hyp_t in the past.  The latter is only used in confidence estimation.
 *
 * Revision 1.4.4.5  2006/01/16 18:28:19  arthchan2003
 * 1, Fixed dox-doc, 2, Added confidence scores parameter in search.h. Also change names of parameters.
 *
 * Revision 1.4.4.4  2005/11/17 06:30:37  arthchan2003
 * Remove senscale from srch_hyp_t (Also see changes in vithist.[ch]). Added some preliminary structure for confidence score estimation.
 *
 * Revision 1.4.4.3  2005/07/26 02:19:20  arthchan2003
 * Comment out hyp_t, change name of wid in srch_hyp_t to id.
 *
 * Revision 1.4.4.2  2005/07/24 19:34:46  arthchan2003
 * Removed search_hyp_t, used srch_hyp_t instead
 *
 * Revision 1.4.4.1  2005/06/27 05:37:58  arthchan2003
 * Fixes to make the search of fsg in place (NOT WORKING NOW) in Makefile.am.
 *
 * Revision 1.4  2005/06/21 23:34:39  arthchan2003
 * Remove all dag functions. Eventually I may just want to delete the whole file as well.
 *
 * Revision 1.2  2005/06/03 05:46:19  archan
 * Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:00  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.3  2004/12/06 10:52:01  arthchan2003
 * Enable doxygen documentation in libs3decoder
 *
 * Revision 1.2  2004/08/31 08:43:47  arthchan2003
 * Fixing _cpluscplus directive
 *
 * Revision 1.1  2004/08/09 00:17:12  arthchan2003
 * Incorporating s3.0 align, at this point, there are still some small problems in align but they don't hurt. For example, the score doesn't match with s3.0 and the output will have problem if files are piped to /dev/null/. I think we can go for it.
 *
 * Revision 1.2  2002/12/03 23:02:44  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 *
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *  		Added onlynodes argument to dag_dump().
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed fwd_sen_active to flag active senones instead of building a list
 * 		of them.
 *  
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_search().
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added function fwd_sen_active() to obtain list of active senones in
 * 		current frame.
 * 
 * 04-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBFBS_SEARCH_H_
#define _LIBFBS_SEARCH_H_

/** \file search.h
   \brief The temporary header file for sphinx 3 functions. 
 */


#ifdef __cplusplus
extern "C" {
#endif

  /** \struct srch_hyp_t
      \brief a hypothesis structure 
   */
typedef struct srch_hyp_s {
    char     *word;    /**< A pointer to the word string*/

    int32   id;        /**< Token ID; could be s3wid_t, s3cipid_t...
			  Interpreted by client. */


    int32 vhid;         /**< Viterbi history (lattice) ID from which
			   this entry created Specific to Sphinx 3.x
			   mode 4 and mode 5*/

    int32 type;		/**< Uninterpreted data; see vithist_entry_t in vithist.h */

    s3frmid_t sf;         /**< Starting frame */
    s3frmid_t ef;         /**< Ending frame */
    int32     ascr;       /**< AM score */
    int32     lscr;       /**< LM score */
    int32     pscr;       /**< score for heuristic search (Only used in dag and astar)*/
    int32     cscr;       /**< Use when the recognizer is generating word-based confidence scores */

    int32  fsg_state;     /**< At which this entry terminates (FSG mode only) */

  struct srch_hyp_s *next;  /**< a pointer to next structure, a convenient device such 
			       that a programmer could choose to use it instead of using
			       a link list.  Of course one could also use glist
			    */
} srch_hyp_t;

  /** \struct hyp_t
      \brief an alias of srch_hyp_t;
   */

  typedef srch_hyp_t hyp_t;

  /** \struct conf_srch_hyp_t
      \brief a hypothesis structure that stores the confidence scores. Mainly used in confidence.c
   */

typedef struct conf_srch_hyp {
  srch_hyp_t sh; /**< a srch_hyp_t */
  float32 lmtype; /**< Language model type */
  float32 l1, l2, l3;  
  int32 matchtype; /**< (Currently not used) Match type: INSERTION, SUBSTITUTION, CORRECT */
  int compound; /**< (Currently not used) The compound type */
  struct conf_srch_hyp *next; /**< a pointer to the next structure */
} conf_srch_hyp_t;


  /** \struct seg_hyp_line_t
      \brief a strurcture that stores one line of hypothesis. Mainly used in confidence.c
   */

typedef struct seg_hyp_line {
  char seq[1024]; /**< The file name */
  int32 sent_end_cscore;  /**< The confidenece score at the end of the utterance */
  int32 cscore ; /**< Confidence score */
  float32 lmtype;   /**<  LM type, depends on the backoff_modes */
  int32 wordno;     /**< The number of word in a sentence */
  int32 nfr;        /**< The number of frame in a sentence */
  int32 ascr;       /**< The sentence acoustic model score */
  int32 lscr;       /**< The sentence language model score */
  conf_srch_hyp_t *wordlist; /**< The list of words */
} seg_hyp_line_t;



#if 0 /* Only in Sphinx 2 */
    float conf;         /* Confidence measure (roughly prob(correct)) for this word;
                           NOT FILLED IN BY THE RECOGNIZER at the moment!! */
    int32 latden;       /* Average lattice density in segment.  Larger values imply
                           more confusion and less certainty about the result.  To use
                           it for rejection, cutoffs must be found independently */
    double phone_perp;  /* Average phone perplexity in segment.  Larger values imply
                           more confusion and less certainty.  To use it for rejection,
                           cutoffs must be found independently. */
#endif


#ifdef __cplusplus
}
#endif

#endif
