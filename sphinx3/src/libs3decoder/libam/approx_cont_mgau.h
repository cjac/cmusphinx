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
 * approx_cont_mgau.h
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 23-Jan-2004 Arthur Chan (archan@cs.cmu.edu)
 *             started
 */


#ifndef _S3_APPROXCONGAU_H_
#define _S3_APPROXCONGAU_H_

#include "cont_mgau.h"
#include "vector.h"
#include "subvq.h"
#include "gs.h"
#include "kb.h"

#ifdef __cplusplus
extern "C" {
#endif

  /** \file approx_cont_mgau.h
   * \brief Master function to compute the approximate score of mixture of Gaussians 
      \warning You need to have some knowledge in fast GMM computation in order to modifed this function. 
      
   This is the current schemes included:
   1, VQ-based Gaussian Selection 
   2, Subvq-based Gaussian Selection
   3, Context Independent Phone-based GMM Selection
   4, Down Sampling 
      a, dumb approach,
      b, conditional down sampling (currently can only be used with VQ-based Gaussian Selection
      c, distance-based down sampling 
      
      The above method of categorizing GMM computation in 4 levels are
      presented in ICSLP 2004.  For the publication, please visit
      Arthur Chan's web site at www.cs.cmu.edu/~archan/ .  
  */

/** 
   approx_con_mgau_frame_eval encapsulates all approximations in the
   Gaussian computation.  This assumes programmers NOT to initialize
   the senone scores at every frame before using this function. This
   modularize this routine but complicated issues such as
   frame-dropping which can also be done in the front-end

   This layer of code controls the optimization performance in Frame
   Leval and GMM Level.

   Frame Level:

   ^^^^^^^^^^^^

   We select to compute the scores only if it is not similar to the
   most recently computed frames.  There are multiple ways to
   configures this.

   Naive down-sampling : Skip the computation one every other n-frames

   Conditional down-sampling : Skip the computation only if the
   current frame doesn't belong to the same neighborhood of the same
   frame.  This neighborhood corresponds to the codeword which the
   feature vector found to be the closest.
   
   No matter which down-sampling was used, the following problem will
   appear in the computation.  Active senones of frame which supposed
   to be skipped in computation could be not computed in the most
   recently computed frame.  In those cases, we chose to compute those
   senones completely.

   GMM Level:

   ^^^^^^^^^^

   In the implementation of CI-based GMM selection makes use of the
   fact that in s3.3 , CI models are always placed before all CD
   models. Hence the following logic is implemented:

   if(it is CI senone)
      compute score
   else if (it is CD senone)
      if the ci-phone beam was not set 
          compute score
      else
          if the CD senone's parent has a score within the beam
	     compute_score
	  else CD senone's parent has a score out of the beam
	     back-off using the parent senone score. 

   During s3.5, the idea of bestidx in a GMM has been changed and 
   the above logic becomes

   if(it is CI senone)
      compute score
   else if (it is CD senone)
      if the ci-phone beam was not set 
          compute score
      else
          if the CD senone's parent has a score within the beam
	     compute_score
	  else CD senone's parent has a score out of the beam
	     if the bestindex of the last frame exists
	        compute score using the bestidx
	     then
  	        back-off using the parent senone score. 
	  
   About renormalization

   ^^^^^^^^^^^^^^^^^^^^^

   Sphinx 3.4 generally renormalize the score using the best
   score. Notice that this introduce extra complication to the
   implementation.  I have separated the logic of computing or not
   computing the scores.  This will clarify the code a bit.
   
   Accounting of senone and gaussian computation

   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   This function assumes approx_cont_mgau_ci_eval was run before it,
   hence at the end the score was added on top of the it.
   
   Design

   ^^^^^^ 

   The whole idea of this function is based on my paper on "4-level
   categorization of GMM computation " which basically describe how
   different techniques of fast GMM computation should interact with
   each others.  The current implementation was there to make the
   code to be as short as possible. I hope that no one will try to make
   the code to be longer than 500 lines. 

   Imperfection
   ^^^^^^^^^^^^

   Imperfections of the code can be easily seen by experts so I want
   to point out before they freak out.  There are synchronization
   mechanism in the bestindex and rec_sen_active. That can easily be a
   source of error.  I didn't do it because somehow when you trust
   just the best matching index of the previous frame is slightly
   different from if you trust the score of the previous frame

   The sen_active, rec_sen_active and senscr should be inside the GMM
   structure rather than just a separate array. I didn't fix it
   because this change will also touch other data structures as well. 

   @see approx_mgau_eval 
   @return the best senone score
*/

int32 approx_cont_mgau_frame_eval (kbcore_t * kbc,  /**< Input, kbcore, for mdef, svq and gs*/
				   fast_gmm_t *fastgmm,	 /**< Input/Output: wrapper for
							    parameters for Fast GMM , for
							    all beams and parameters, during
							    the computation, the */
				   float32 *feat,	/**< Input: the current feature vector */
				   int32 frame,         /**< Input: the current frame number */
				   int32 *sen_active,	/**< Input: the current active senones */
				   int32 *rec_sen_active, /**< Input: the most recent active senones */
				   int32 *senscr,         /**< Output: the output senone scores */
				   int32 *cache_ci_senscr, /**< Input: the CI senone scores for CI GMMS */
				   ptmr_t *tm_ovrhd        /**< Output: the timer used for computing overhead */
				   );


  /**
   * Evaluate the approximate gaussian score for CI senone for one
   * frame.  In Sphinx 3.X (X=4,5), this routine is used to precompute
   * CI senone score as a kind of approximate match of the CD score.
   */


void approx_cont_mgau_ci_eval (
			       kbcore_t *kbc, /**< Input, kbcore, for mdef, svq and gs*/
			       fast_gmm_t *fg, /**< Input/Output: wrapper for
							    parameters for Fast GMM , for
							    all beams and parameters, during
							    the computation, the */
			       mdef_t *mdef,  /**< Input : model definition */
			       float32 *feat, /**< Input : the current frame of feature */
			       int32 *ci_senscr, /**< Output : the ci senscore for this frame */
			       int32 fr /**< Input : The frame number */

			       );

#ifdef __cplusplus
}
#endif

#endif
