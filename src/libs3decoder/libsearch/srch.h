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

/* srch.h
 * HISTORY
 * $Log$
 * Revision 1.1  2005/06/22  02:24:42  arthchan2003
 * Log. A search interface implementation are checked in. I will call
 * srch_t to be search abstraction or search mechanism from now on.  The
 * major reason of separating with the search implementation routine
 * (srch_*.[ch]) is that search is something that people could come up
 * with thousands of ways to implement.
 * 
 * Such a design shows a certain sense of defiance of conventional ways
 * of designing speech recognition. Namely, **always** using generic
 * graph as the grandfather ancester of every search lattice.  This could
 * 1) break a lot of legacy optimization code. 2) could be slow depends
 * on the implementation.
 * 
 * The current design only specify the operations that are supposed to be
 * generic in every search (or atomic search operations (ASOs)).
 * Ideally, users only need to implement the interface to make the code
 * work for another search.
 * 
 * From this point of view, the current check-in still have some
 * fundamental flaws.  For example, the communication mechanism between
 * different atomic search operations are not clearly defined. Scores are
 * now computed and put into structures of ascr. (ascr has no clear
 * interface to outside world). This is something we need to improve.
 * 
 * Revision 1.18  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.17  2005/06/10 03:40:57  archan
 * 1, Fixed doxygen documentation of srch.h, 2, eliminate srch.h C-style functions. 3, Start to fend off the users for using mode 5.  We are ready to merge the code.
 *
 * Revision 1.16  2005/06/10 03:01:50  archan
 * Fixed file_open.
 *
 * Revision 1.15  2005/06/09 21:03:33  archan
 * Update srch.h and srch_debug.c such that include files doesn't depend on explicitly specified directory name.  Rather it would be taken care by -I option in Makefile.am
 *
 * Revision 1.14  2005/05/11 06:10:38  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.13  2005/05/11 00:18:45  archan
 * Add comments on srch.h and srch_time_switch_tree.h and srch_debug.h on how things work. A very detail comment is added in srch.h to describe how generally srch_t is interacting with other parts of the code.
 *
 * Revision 1.12  2005/05/04 05:15:25  archan
 * reverted the last change, seems to be not working because of compilation issue. Try not to deal with it now.
 *
 * Revision 1.1  2005/05/04 04:46:04  archan
 * Move srch.c and srch.h to search. More and more this type of refactoring will be done in future
 *
 * Revision 1.10  2005/05/03 04:09:09  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.9  2005/04/25 19:22:47  archan
 * Refactor out the code of rescoring from lexical tree. Potentially we want to turn off the rescoring if we need.
 *
 * Revision 1.8  2005/04/22 04:22:36  archan
 * Add gmm_wrap, this will share code across op_mode 4 and op_mode 5. Also it also separate active senone selection into a different process.  I hope this is the final step before making the WST search works.  At the current stage, the code of mode-5 looks very much alike mode-4.  This is intended because in Prototype 4, tail sharing will be used to reduce memory.
 *
 * Revision 1.7  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.6  2005/04/20 03:42:55  archan
 * srch.c now is the only of the master search driver. When there is any change in the **interaction** of different blocks, srch.c should be changed first.  Then the search implenetation, such as srch_time_switch_tree.c
 *
 * Revision 1.5  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 * 1            Started. This replaced utt.c starting from Sphinx 3.6. 
 */

#include <s3types.h>
#include "vithist.h"
#include "kbcore.h"
#include "kb.h"
#include "srch_time_switch_tree.h"
#include "srch_word_switch_tree.h"
#include "gmm_wrap.h"
#include "srch_debug.h"


#define SRCH_SUCCESS 0
#define SRCH_FAILURE 1

#if 0
#define SRCH_STATE_BEGIN 0
#define SRCH_STATE_DECODE 1
#define SRCH_STATE_END 2
#endif

#define OPERATION_ALIGN 0
#define OPERATION_ALLPHONE 1
#define OPERATION_GRAPH 2
#define OPERATION_FLAT_DECODE 3
#define OPERATION_TST_DECODE 4
#define OPERATION_WST_DECODE 5
#define OPERATION_DEBUG 1369  /** ARCHAN 20050329: 1369 has no meaning
                                  at all, I just love to use it. */

#define GRAPH_STRUCT_FLAT 0
#define GRAPH_STRUCT_TST 1
#define GRAPH_STRUCT_WST 2
#define GRAPH_STRUCT_GENGRAPH 3

#define GMM_STRUCT_CDHMM 0
#define GMM_STRUCT_SCHMM 1


typedef struct {
  void *graph_struct; /**< The graph structure */
  int32 graph_type;   /**< The graph type */
}grp_str_t;


/** 
   \file srch.h 
   \brief search abstraction. 

   Written at 20050510 by ARCHAN.

   Mechanism/implementation separation

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   Starting from Sphinx 3.6, the implementation of the search has
   conceptually separated into two layers.  The top layer srch.c
   defines the top level atomic function defintion (implemented as
   function pointers) that every search-like operations will share. 

   To understand what it means, consider two seemingly different
   search-like operations, alignment and decoding.  Both of them
   required 1) initialization, 2) propropagte 1 frame. 3) termniation.
   Each of these operation can well be defined as one single
   interface. 

   Some people call this type of implementation as "C-class" which I
   think is very appropiate.  Because in general, C++, Objective-C and
   D actually used the same mechanism to implement the concept
   "class".  Obviously, the actual implementation was hidden in the
   compiler in those cases so that's why so many people are confused. 
   
   Why polymorphism is important?

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   Before sphinx 3.6, maintainers (that includes me) tend to think
   that different operations would require different treatment.  The
   consequence of that is different opertations tends to have
   different implementations, styles.  Code tends to be duplicated if
   programmers think in this way.  One big problem is the fact that we
   had so called s3 slow and s3 fast and there were 3000 lines
   duplication. 

   Code duplication also tends compound. i.e. if code was duplicated
   twice, it will be duplicated four times.  That reduces
   productivitiy of the team. 

   Other reasons why srch.c is written

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   At the time when we implemented this routine, we actually didn't
   know whether mode 5 (word tree copies) will work or not.
   Therefore, letting the code of mode 4 (lucky wheel search) and mode
   5 to coexist will be the best.


   "Embedding" of multi-stage search into the dynamic programming
   
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   Another interesting part (Interesting in the sense that I would
   write a paper on this.) of the search is that we (I will say mainly
   me because this is not new at all.) realize that approximiate
   search (i.e. something like phoneme-lookahead), detail search
   (i.e. something like hard-core tree and fsm search) and high level
   rescoring using knowledge source (e.g. using LM to rescore N-best)
   could all be incorporated into the first pass search and
   potentially make the code more verstille.

   They key concept on how to do this is here: at one frame, one can
   carry out the above three operations and make the search faster
   because of 1) heuristics value obtained in the approximate search
   2) early incorporation of high-level knowledge in the rescoring stage. 

   The naming of function interfaces shows this tendency. lv1 means the
   approximate first-stage search. lv2 means the detail second-stage
   search. rescoring means the use of high-level information in
   rescoring during the search.

   The scope of what srch.c defines

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   Written at 20050510

   At the time I wrote this, we only implement mode 4 (Ravi's
   implementation in Sphinx 3.2, I usually call it "lucky wheel"
   implementation.  See my description in srch_time_switch_tree.h),
   mode 5 (My implementation using tree copies.)  and mode 1369 (A
   debug mode of the search mechanism where only a prompt will be
   displayed and show that a function is called. Why 1369? coz I am a
   nerd. :-))

   Though only three modes of the search were written. I found it to
   be utmost important to share my imagination of what we can actually
   do using this interface of the search. In theory, the slow search
   (pre-defined as mode 3) and the fast searches (pre-defined as mode
   4 and mode 5) can be incorporated using this interface. s3align and
   s3allphone (pre-defined as mode 0 and 1) in s3.0 family can also be
   implemented in this way. 

   Applications such as keyword spotting and speaker verifcation. They
   are not defined at this point but they are possible using the
   architecture

   What limits this architecture is that it might not be able to
   implement a wide class of segmental models.  I will speculate if a
   recursive formulat can be derived for a particular type of model
   (segmental or non-segmental, HMM or non-HMM).  The current
   interface could be overloaded and allow implementation of them.
   There are definitely some types of segmental model are very hard to
   implement using this structure; e.g. MIT segmental model (pre-1999)
   where segmentation was generated by low-level information first and
   search were performed on segments. 


   A general guide-line on how to write a search implementation

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   
   1, The batch mode search operation and the on-line mode operation
   can always share code. If the implementation is duplicated, it
   shows a bad omen.

   2, Knowledge source (such as LM and finite state grammer) always
   change the graph structure. That is why srch_{set,add,delete}_lm
   was defined, if we have multiple types of knowledge
   sources. Interfaces should be added in srch.c and the search
   operation (e.g. search/srch_time_switch_tree.c) provides the actualy
   implemenation

   3, GMM computation and search can be easily separated into two
   routines. Therefore, instead of wrap it up in individual searches,
   a new set of function called gmmwrap.c is used which conform the
   interface in srch.c. 
   
   4, Mechanism such as phoneme lookahead and generally fast match are
   all similar in the sense that. They look forward a couple of frames
   and compute a heuristic score. The windows controlling mechanism
   should be coded in srch.c . The actual heuristic should be coded
   somewhere else. 

   5, grh contains the actual data structure that stores the graph. We
   didn't use WFST as a common structure because we found that it has
   certain fact we don't know at the time we implemented.

   6, Always get back to the debug mode (mode 1369) if you want to
   debug the search mechanism

   7, (V. Imp.) When in doubt, ask Arthur Chan. If he is not in CMU
   any more, brainstorm how to get him back.  He is a nice guy and has
   no problem in talking.

 */
typedef struct srch_s {
  grp_str_t* grh;     /**< Pointer to search specific structures */
  int op_mode;        /**< The operation mode */
  stat_t *stat;       /**< Pointer to the statistics structure */
  char *uttid;        /**< Copy of UttID */

  int32 cache_win;    /**< The windows lengths of the cache for approximate search */
  int32 cache_win_strt;    /**< The start index of the window near the end of a block */
  int32 senscale;     /**< TEMPORARY VARIABLE: Senone scale */


  vithist_t *vithist;     /**< Viterbi history, built during search */

  /* ARCHAN: Various pruning beams, put them together such that it looks more logical. */
  ascr_t *ascr;		  /**< Pointer to Senone and composite senone scores for one frame */
  beam_t *beam;		  /**< Pointer to Structure that wraps up parameters related to beam pruning */
  fast_gmm_t *fastgmm;    /**< Pointer to Structure that wraps up parameters for fast GMM computation */
  pl_t *pl;              /**< Pointer to Structure that wraps up parameter for phoneme look-ahead */
  adapt_am_t * adapt_am; /** Pointer to AM adaptation structure */
  kbcore_t *kbc;      /**< Pointer to the kbcore */
  FILE *matchfp;          /**< Copy of File handle for the match file */
  FILE *matchsegfp;       /**< Copy of File handle for the match segmentation file */
  FILE *hmmdumpfp;        /**< Copy of File handle for dumping hmms for debugging */


  /** Initialization of the search, coz the graph type can be different */
  int (*srch_init)(kb_t *kb, /**< Pointer of kb_t which srch_init wants to copy from */
		   void* srch_struct /**< a pointer of srch_t */
		   );

  /**< Un-Initialize of the search. */
  int (*srch_uninit)(
		     void* srch_struct /**< a pointer of srch_t */
		     );
  /**< Begin search for one utterance */
  int (*srch_utt_begin)(
			void* srch_struct /**< a pointer of srch_t */
			);

  /** End search for one utterance */
  int (*srch_utt_end)(
		      void* srch_struct /**< a pointer of srch_t */
		      );
  /** Actual decoding operation */
  int (*srch_decode)(
		     void* srch_struct /**< a pointer of srch_t */
		     );

  /** Set LM operation.  */
  int (*srch_set_lm)(
		     void* srch_struct, /**< a pointer of srch_t */
		     const char *lmname /**< The LM name */
		     );

  /** Add LM operation */ 
  int (*srch_add_lm)(void* srch_struct, /**< a pointer of srch_t */
		     lm_t* lm,          /**< A new lm */
		     const char *lmname /**< The LM name */
		     );

  /** Delete LM operation */
  int (*srch_delete_lm)(void* srch_struct,  /**< a pointer of srch_t */
			const char *lmname  /**< The LM name */
			);

  /* The 4 operations that require switching during the approximate search process */
  /**< lv1 stands for approximate search. Currently not used. */

  /** Compute Approximate GMM */
  int (*srch_gmm_compute_lv1)(void* srch_struct,  /**< a pointer of srch_t */
			      float32 *feat,      /**< The feature vector */
			      int32 frmno_lp1,    /**< The frame for the cache */
			      int32 frmno_lp2     /**< The frame for the windows */
			      );

  int (*srch_hmm_compute_lv1)(void* srch_struct);
  int (*srch_eval_beams_lv1)(void* srch_struct);
  int (*srch_propagate_graph_ph_lv1)(void* srch_struct);
  int (*srch_propagate_graph_wd_lv1)(void* srch_struct);

  /* The 4 operations that require switching during the detail search process */
  /** lv2 stands for detail search. */

  /** Compute detail (CD) GMM scores or lv2*/
  int (*srch_gmm_compute_lv2)(void* srch_struct,  /**< a pointer of srch_t */
			      float32 *feat,      /**< A feature vector */
			      int32 time          /**< The frame we want to compute detail score */
			      );

  /** Compute detail (CD) HMM scores or lv2*/
  int (*srch_hmm_compute_lv2)(void* srch_struct,  /**< a pointer of srch_t */
			      int32 frmno         /**< The frame we want to compute detail score */
			      );

  /** Compute the beams*/
  int (*srch_eval_beams_lv2)(void* srch_struct     /**< a pointer of srch_t */
			     );

  /** Propagate the graph in phone level */
  int (*srch_propagate_graph_ph_lv2)(void* srch_struct, /**< a pointer of srch_t */
				     int32 frmno        /**< The frame no. */
				     );

  /** Propagate the graph in word level */
  int (*srch_propagate_graph_wd_lv2)(void* srch_struct,  /**< a pointer of srch_t */
				     int32 frmno       /**< The frame no. */
				     );

  /** Rescoring srch */
  int (*srch_rescoring) (void* srch_struct,  /**< a pointer of srch_t */
			 int32 frmno         /**< The frame no. */
			 );  

  int (*srch_frame_windup) (void * srch_struct, int32 frmno);
  int (*srch_compute_heuristic) (void * srch_struct, int32 win_efv);
  int (*srch_shift_one_cache_frame) (void *srch_struct,int32 win_efv);
  int (*srch_select_active_gmm) (void *srch_struct);

}srch_t;


/* The following are C-style method for srch structure.  In theory,
users could used both C-style and function pointer style to access
functionalities of the code. However, we recommend developers to use
the C-style functions because 1) it won't scare people that match, 2)
it is more consistent with other modules in sphinx 3. 
 */

/** Initialize the search routine, this will specify the type of search
    drivers and initialized all resouces
*/

srch_t* srch_init(kb_t *kb, /**< In: knowledge base */
		  int32 op_mode /**< In: operation mode of the search */
		  );

/** Report the search routine */
/** using file name of the model definition and directory name to initialize */

/** Report the search structure
 */
void srch_report(srch_t* srch /**< In: a search structure */
		 );

/**
   Begin decoding of speech for one utterance. 
 */

int32 srch_utt_begin(srch_t* srch /**< In: a search structure */
		     );

/**
   decode one block of speech. 
 */
int32 srch_utt_decode_blk(srch_t* srch, /**< In: a search structure */
			  float ***block_feat,  /**< In: a pointer of a two dimensional array */
			  int32 block_nfeatvec, /**< In: Number of feature vector */
			  int32 *curfrm  /**< In/Out: a pointer of the current frame index*/
			  );

/**
   End decoding of speech for one utterance. 
 */
int32 srch_utt_end(srch_t* srch);

/** Wrap up the search routine*/
int32 srch_uninit(srch_t* srch);


/** write match segment */
void matchseg_write (FILE *fp,  /**< The file pointer */
		     srch_t *s, /**< The search structure */
		     glist_t hyp, /**< A link-list that contains the hypotheesis*/
		     char *hdr /**< The header */
		     );

/** write a match file */
void match_write (FILE *fp,  /**< The file pointer */
		  srch_t* s, /**< The search structure */
		  glist_t hyp, /**< A link-list that contains the hypothesis */
		  char *hdr    /**< The header */
		  );

/** Dump recognition result */
void reg_result_dump (srch_t* s, /**< A search structure */
		      int32 id   /**< Utterance ID */
		      );

/** using file name of the LM or defined lmctlfn mechanism */
int32 srch_set_lm(srch_t* srch,  /**< A search structure */
		  const char *lmname /**< LM fie name */
		  );

/** delete lm */
int32 srch_delete_lm();

#if 0 /*Not implemented */
int32 srch_set_am();

/** add new am */
int32 srch_add_am();

/** delete am */
int32 srch_delete_am();


/** add new lm */
int32 srch_add_lm();


/** using file name of the regression class mapping or a directory name to initialize  */
int32 srch_set_mllr();

/** add new mllr */
int32 srch_add_mllr();

/** delete mllr */
int32 srch_delete_mllr();

/** using file name of interpolation file to initialize it */
int32 srch_set_lamdafn();

/** add new mllr */
int32 srch_add_lamdafn();

/** delete mllr */
int32 srch_delete_lamdafn();

/** add new words into the dictionary */
int32 srch_add_words_to_dict();
#endif
