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

#include <libutil/libutil.h>
#include "cont_mgau.h"
#include "vector.h"
#include "subvq.h"
#include "gs.h"
#include "kb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Master function to compute the approximate score of mixture of Gaussians 
   This is the current schemes included:
   1, VQ-based Gaussian Selection 
   2, Subvq-based Gaussian Selection
   3, Context Independent Phone-based GMM Selection
   4, Down Sampling 
      a, dumb approach,
      b, conditional down sampling (currently can only be used with VQ-based Gaussian Selection
      c, distance-based down sampling 
*/

/*
 * Evaluate the approximation gaussian score for one frame. 
 */
int32 approx_cont_mgau_frame_eval (kbcore_t * kbc,  /* Input, kbcore, for mdef, svq and gs*/
				   fast_gmm_t *fastgmm,	 /* Input/Output: wrapper for
							    parameters for Fast GMM , for
							    all beams and parameters, during
							    the computation, the */
				   float32 *feat,	/*Input: the current feature vector */
				   int32 frame,         /*Input: the current frame number */
				   int32 *sen_active,	/*Input: the current active senones */
				   int32 *rec_sen_active, /*Input: the most recent active senones */
				   int32 *senscr,         /*Output: the output senone scores */
				   int32 *cache_ci_senscr, /*Input: the CI senone scores for CI GMMS */
				   ptmr_t *tm_ovrhd        /*Output: the timer used for computing overhead */
				   );


void approx_cont_mgau_ci_eval (/*mgau_model_t *g, */
			       kbcore_t *kbc,
				 fast_gmm_t *fg,
			       mdef_t *mdef, 
			       float32 *feat,
			       int32 *ci_senscr);

#ifdef __cplusplus
}
#endif

#endif
