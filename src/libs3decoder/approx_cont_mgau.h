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

/*Determine whether a frame's computation should be skipped or not based on the down sampling ratio and other parameters*/

int32 approx_isskip(int32 frame,int32 ds_ratio,int32 cond_ds,int32 isSameBestIdx,int32 *skip_count);

/* Master function to compute the approximate score of mixture of Gaussians 
   This is the current schemes included:
   1, VQ-based Gaussian Selection 
   2, Subvq-based Gaussian Selection
   3, Context Independent Phone-based GMM Selection
   4, Down Sampling 
      a, dumb approach,
      b, conditional down sampling (currently can only be used with VQ-based Gaussian Selection
*/

/*
 * Evaluate the approximation gaussian score for one frame. 
 */
int32 approx_cont_mgau_frame_eval (mgau_model_t *g,
				   gs_t *gs,
				   subvq_t *svq,
				   int32 svq_beam,
				   float32 *feat,	
				   int32 *sen_active,	
				   int32 *senscr,
				   int32 *cache_ci_senscr,
				   kb_t *kb,
				   int32 frame);	

void approx_cont_mgau_ci_eval (mgau_model_t *g, float32 *feat,int32 *ci_senscr, kb_t *kb);

#ifdef __cplusplus
}
#endif

#endif
