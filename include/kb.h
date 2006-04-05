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
/*
 * kb.h -- Knowledge bases, search parameters, and auxiliary structures for decoding
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.18  2006/02/23 05:44:59  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH.
 * 1, Added temp_init_vithistory, this will choose to initialize the correct viterbi history given the mode.
 * 2, Moved most of the code in kb_setmllr to adaptor.c
 *
 * Revision 1.17.4.2  2005/09/18 01:21:18  arthchan2003
 * 1, Add a latticehist_t into kb_t, use a temporary method to allow polymorphism of initialization of vithist_t and latticehist_t. 2, remove the logic kb_set_mllr and put it to adapt_set_mllr
 *
 * Revision 1.17.4.1  2005/07/03 23:00:58  arthchan2003
 * Free stat_t, histprune_t and srch_t correctly.
 *
 * Revision 1.17  2005/06/21 23:21:58  arthchan2003
 * Log. This is a big refactoring for kb.c and it is worthwhile to give
 * words on why and how things were done.  There were generally a problem
 * that the kb structure itself is too flat.  That makes it has to
 * maintained many structure that could be maintained by smaller
 * structures.  For example, the count of A and the array of A should
 * well be put into the same structure to increase readability and
 * modularity. One can explain why histprune_t, pl_t, stat_t and
 * adapt_am_t were introduced with that line of reasoning.
 *
 * In srch_t, polymorphism of implementation is also one important
 * element in separting all graph related members from kb_t to srch_t.
 * One could probably implement the polymorphism as an interface of kb
 * but it is not trivial from the semantic meaning of kb.  That is
 * probably why srch_t is introduced as the gateway of search interfaces.
 *
 * Another phenonemon one could see in the code was bad interaction
 * between modules. This is quite serious in two areas: logging and
 * checking. The current policy is unless something required cross
 * checking two structures, they would be done internally inside a module
 * initialization.
 *
 * Finally, kb_setlm is now removed and is replaced by ld_set_lm (by
 * users) or srch_set_lm (by developers). I think this is quite
 * reasonable.
 *
 * Revision 1.10  2005/06/19 19:41:23  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.9  2005/05/11 06:10:38  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.8  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.7  2005/04/20 03:36:18  archan
 * Remove setlm from kb entirely, refactor it to search implementations, do the corresponding change for the changes in ascr and pl
 *
 * Revision 1.6  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 14-Jun-2004  Yitao Sun (yitao@cs.cmu.edu) at Carnegie Mellon University
 *              Modified struct kb_t to save the last hypothesis.
 *
 * 07-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added kb_t.ci_active.
 * 
 * 02-Jun-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_KB_H_
#define _S3_KB_H_

#include <s3types.h>
#include "kbcore.h"
#include "lextree.h"
#include "vithist.h"
#include "ascr.h"
#include "fast_algo_struct.h"
#include "mllr.h"
#include "cmn.h"
#include "stat.h"
#include "adaptor.h"
#include "cb2mllr_io.h"

/** \file kb.h
 * \brief The global wrapper structure for all variables in 3.X
 * search.  We may want to use it for sphinx 3.0 as well.  
 */

#ifdef __cplusplus
extern "C" {
#endif


  /*
 * ARCHAN :20040229. Starting from s3.4, we accept users to specify mutliple LMs
 * using -lmctlfile. To avoid potential overhead caused by rebuilding the trees
 * at every sentence. We allocate all lexical trees in the intialization. If we
 * assume the number of words to be less than 64k and the number of LMs is smaller
 * than 100. This memory loss should be fine.  Later: We should allow users to specify
 * the memory address mode by their own discretion.
 */


  /* The wrapper structure of all operation in the Sphinx 3.X decoder. 
   *
   */
typedef struct {
  /** Core models, defined as acoustic and language models, dictionary
     (pronounciation models_, front-ends, filler-penalties approximate
     acoustic models such as sub vector quantization map and Gaussian
     selector */

  kbcore_t *kbcore;       /**< Core model structures */

  /** Feature generation related variables*/
  float32 ***feat;	  /**< Feature frames */
  cmn_t *cmn;             /**< The structure for cepstral mean normalization. */

  /** Structures of storing parameters for different techniques. */
  ascr_t *ascr;		  /**< Senone and composite senone scores for one frame. */
  beam_t *beam;		  /**< Structure that wraps up parameters related to beam pruning. */
  fast_gmm_t *fastgmm;    /**< Structure that wraps up parameters for fast GMM computation. */
  pl_t *pl;               /**< Structure that wraps up parameters for phoneme look-ahead. */

  /** Structure that wraps up adaptation variables. such as regression matrices in MLLR */
  adapt_am_t * adapt_am;  /**< Structure that wraps up parameters for adaptation such as MLLR. */

  /** Structure that records the search. */
  vithist_t *vithist;	  /**< Structure that stores the viterbi
			     history, built during search. Used only
			     in mode 4 and 5 */

  latticehist_t *lathist;     /**< Structure that stores the viterbi
			     history, appear here because of legacy,
			     built during search. Used only in mode 3
			  */

  stat_t *stat;           /**< Structure of statistics including timers and counters. */

  /** FILE handle that handles output. */
  FILE *matchfp;          /**< File handle for the match file */
  FILE *matchsegfp;       /**< File handle for the match segmentation file */
  FILE *hmmdumpfp;        /**< File handle for dumping hmms for debugging */

  /* The only variable I intend to make it be alone in the whole structure. It has its own uniqueness.  */
  int32 op_mode; /** A mode for specifying operation */
  char *uttid;   /**< Utterance ID. The one thing that should move to somewhere like srch */

  
  void *srch;  /**< The search structure */
} kb_t;


  /** Initialize the kb */
  void kb_init (kb_t *kb /**< In/Out: A empty kb_t */
		);

  
  /** Deallocate the kb structure */
  void kb_free (kb_t *kb /**< In/Out: A empty kb_t */
		);	/* RAH 4.16.01 */
  
  /** Set MLLR */
void kb_setmllr(char* mllrname, /**< In: The name of the mllr model */
		char* cb2mllrname, /**< In: The filename of the MLLR class map */
		kb_t *kb /**< In/Out: A empty kb_t */
		);    /* ARCHAN 20040724 */

  /** Set Utterance ID to uttid */
  void kb_set_uttid(char *uttid, /**< In: the new utt id */
		  kb_t *kb    /**< In/Out: A empty kb_t */
		  );   /* ARCHAN 20041111 */


#ifdef __cplusplus
}
#endif

#endif
