/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * subvq.h
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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Updated subvq_free () to free more allocated memory
 * 
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Moved subvq_t.{frm_sen_eval,frm_gau_eval} to cont_mgau.h.
 * 
 * 14-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_t.{frm_sen_eval,frm_gau_eval}.  Changed subvq_frame_eval to
 * 		return the normalization factor.
 * 
 * 06-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_subvec_eval_logs3().
 * 
 * 14-Oct-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed ci_active flags input to sen_active in subvq_frame_eval().
 * 
 * 20-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added subvq_gautbl_eval_logs3().
 * 
 * 12-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_SUBVQ_H_
#define _S3_SUBVQ_H_

#include <libutil/libutil.h>
#include "cont_mgau.h"
#include "vector.h"
#include "logs3.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    arraysize_t origsize;	/* origsize.r = #codebooks (or states) in original model;
				   origsize.c = max #codewords/codebook in original model */
    int32 n_sv;			/* #Subvectors */
    int32 vqsize;		/* #Codewords in each subvector quantized mean/var table */
    int32 **featdim;		/* featdim[s] = Original feature dimensions in subvector s */
    vector_gautbl_t *gautbl;	/* Vector-quantized Gaussians table for each sub-vector */
    int32 ***map;		/* map[i][j] = map from original codebook(i)/codeword(j) to
				   sequence of nearest vector quantized subvector codewords;
				   so, each map[i][j] is of length n_sv.  Finally, map is
				   LINEARIZED, so that it indexes into a 1-D array of scores
				   rather than a 2-D array (for faster access). */
    
    /* Working space used during evaluation */
    float32 *subvec;		/* Subvector extracted from feature vector */
    int32 **vqdist;		/* vqdist[i][j] = score (distance) for i-th subvector compared
				   to j-th subvector-codeword */
    int32 *gauscore;		/* Subvq-based approx. Gaussian density scores for one mixture */
    int32 *mgau_sl;		/* Shortlist for one mixture (based on gauscore[]) */
} subvq_t;


/*
 * SubVQ file format:
 *   VQParam #Original-Codebooks #Original-Codewords/codebook(max) -> #Subvectors #VQ-codewords
 *   Subvector 0 length <length> <feature-dim> <feature-dim> <feature-dim> ...
 *   Subvector 1 length <length> <feature-dim> <feature-dim> <feature-dim> ...
 *   ...
 *   Codebook 0
 *   Row 0 of mean/var values (interleaved) for subvector 0 codebook (in 1 line)
 *   Row 1 of above
 *   Row 2 of above
 *   ...
 *   Map 0
 *   Mappings for state 0 codewords (in original model) to codewords of this subvector codebook
 *   Mappings for state 1 codewords (in original model) to codewords of this subvector codebook
 *   Mappings for state 2 codewords (in original model) to codewords of this subvector codebook
 *   ...
 *   Repeated for each subvector codebook 1
 *   Repeated for each subvector codebook 2
 *   ...
 *   End
 */
subvq_t *subvq_init (char *file,	/* In: Subvector model file */
		     float64 varfloor,	/* In: Floor to be applied to variance values */
		     int32 max_sv,	/* In: Use the first so many subvectors instead of all;
					   if <0, use all */
		     mgau_model_t *g);	/* In: Original model from which this subvq model was
					   built, for cross-validation; optional */

void subvq_free (subvq_t *vq);


/*
 * Evaluate senone scores for one frame.  If subvq model is available, for each senone, first
 * get approximate Gaussian density scores using it; obtain a shortlist of Gaussians using
 * these scores, then evaluate the shortlist exactly.  If no subvq model, evaluate senones
 * using all Gaussian densities.  Finally, scale senone scores by subtracting the best.
 * Return value: The normalization factor (best senone absolute score).
 */
int32 subvq_frame_eval (subvq_t *vq,	/* In: Sub-vector model */
			mgau_model_t *g,/* In: Exact mixture Gaussian model */
			int32 beam,	/* In: (Logs3) threshold for selecting shortlist;
					   range = [-infinity(widest beam), 0(narrowest)] */
			float32 *feat,	/* In: Input feature vector for this frame */
			int32 *sen_active,	/* In: Active flags for each senone (optional).
						   If not NULL, only active ones evaluated */
			int32 *senscr);	/* Out: Normalized senone scores */

/*
 * Evaluate the Mahalanobis distances between the given feature vector and each entry in the
 * given subvq codebook.  Save results, as logs3 values, in vq->vqdist[][].
 */
void subvq_gautbl_eval_logs3 (subvq_t *vq,	/* In/Out: Reference subvq structure */
			      float32 *feat);	/* In: Subvectors extracted from this, and
						   compared to relevant subvq codewords */

/*
 * Evaluate the codewords for a single given subvector sv, wrt the input feature vector.
 * Save results, as logs3 values, in vq->vqdist[sv][].
 * (Basically, like subvq_gautbl_eval_logs3, but for a single given subvector instead of all.)
 */
void subvq_subvec_eval_logs3 (subvq_t *vq,	/* In/Out: Reference subvq structure */
			      float32 *feat,	/* In: Input feature subvector extracted from
						 * this, and compared to relevant codewords */
			      int32 sv);	/* In: ID of subvector being evaluated */

/*
 * Based on previously computed subvq scores (Mahalanobis distances), determine the active
 * components in the given mixture (using the vq->map).
 * Return value: #Candidates in the returned shortlist.
 */
int32 subvq_mgau_shortlist (subvq_t *vq,
				   int32 m,	/* In: GMM index */
				   int32 n,	/* In: #Components in specified mixture */
				   int32 beam);	/* In: Threshold to select active components */


/*
 * Compute the scores of a gaussian using only sum of the sub-vector scores. 
 */

int32 subvq_mgau_eval (mgau_model_t *g,
		       subvq_t *vq, /*the SVQ */
		       int32 m, /*In: GMM Index */
		       int32 n, /* #Components in a specified mixture */
		       int32 *active /*Active list of mixture */
		       );

#ifdef __cplusplus
}
#endif

#endif
