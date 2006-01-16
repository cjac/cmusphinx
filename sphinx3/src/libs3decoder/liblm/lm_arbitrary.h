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
 * lm_arbitrary.h - Arbitrary LM data structure 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * $Log$
 * Revision 1.1.2.2  2006/01/16  19:59:40  arthchan2003
 * Commented out the unfinished code.
 * 
 * Revision 1.1.2.1  2005/07/17 05:24:31  arthchan2003
 * (Incomplete) Added lm_arbitrary.[ch], an arbitrary n-gram data structure.  Far from completed. Don't expect too much.
 *
 *
 */

#ifndef _S3_LM_ARBITRARY_H_
#define _S3_LM_ARBITRARY_H_

#include <prim_type.h>
#include <lm.h>
#include <s3types.h> 



#if 0 /*Unfinished code of 
#define DEFAULT_COUNT_TABLE_SIZE MAX_INT32
#define KEY  4294960000

#define GEN_LM_READ_SUCCEED 1
#define GEN_LM_READ_FAIL 0


/** \struct genlm_t
    \brief Generic arbitrary LM structure 
 */
typedef struct {
  uint16 n; /**< n=3 for trigram, n=4 for 4-gram etc. */

} genlm_t ;

/**
   CMU/Cambridge LM Toolkit specific structure;
 */

#define CLOSED_VOCAB 0
#define OPEN_VOCAB_1 1
#define OPEN_VOCAB_2 2

#define SLM_GOOD_TURING 1
#define SLM_ABSOLUTE 2
#define SLM_LINEAR 3
#define SLM_WITTEN_BELL 4

typedef uint16 cnt_ind_t; /**< The count's index in the count 
			     table. */
typedef uint16 flag;  /**< A flag that is similar to what is defined in CMU/Cam LM Tk*/

/** \struct slm_t
    \brief A children structure of genlm_t, useful for reading/writing CMU/Cambridge LM Toolkit v2. 
 */

typedef struct {

  /** Added to make it compatible to CMU-Cambridge LM header toolkit header*/
  genlm_t *ng;       /**< Generic LM */

  int32 version; /**< Version number */
  uint16 voc_sz;     /**< Vocabulary size */
  uint16 no_ctxt_cue; /**< Number of context cues */
  uint16 voc_type;   /**< Vocabulary type 
			0: for CLOSE vocabulary, 1 for 
		      */
  cnt_ind_t cnt_tab_sz; /**< Count table size */
  uint16 disc_method; /**< Discounting methods.  1: for GOOD turing, 2,
			for absolute discounting, 3 for learn method, 4, for Witten Bell.
		      */
  float64 min_alpha; /**< The minimum alpha in the table */
  float64 max_alpha; /**< The maximum alpha in the table */
  uint16 out_of_range_alphas; /**< The maximum number of out of range 
				 alphas that we are going to allow. */
  uint16 sz_alpha_array;    /**< Size of the alpha array*/

  uint32 n_ug; /**< Number of unigrams */
  float64 zeroton_fract; /**<cap on prob(zeroton) as fraction of 
			    P(singleton) */
  float64 oov_frac; /**< OOV fraction */
  int16 fourbyte_counts; /**< Whether we used four byte counts */
  int16 fourbyte_alpha; /**< Whether we used four byte alpha */

  uint16 first_id;       /* 0 if open vocab, 1 if closed vocab. */
  
} slm_t;
#endif 

#endif /* _S3_LM_ARBITRARY_H_*/
