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
 * 12-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#ifndef _GAUDEN_SUBVQ_
#define _GAUDEN_SUBVQ_


#include <libutil/libutil.h>
#include <libmisc/vector.h>


typedef struct {
    arraysize_t origsize;	/* origsize.r = #codebooks (or states) in original model;
				   origsize.c = max #codewords/codebook in original model */
    int32 n_sv;			/* #Subvectors */
    int32 vqsize;		/* #Codewords in each subvector quantized mean/var table */
    int32 *svsize;		/* Length of each subvector */
    int32 **featdim;		/* featdim[s] = Original feature dimensions in subvector s */
    float32 ***mean;		/* mean[s] = vector quantized means for subvector s
				   (of size vqsize x svsize[s]) */
    float32 ***var;		/* var[s] = vector quantized vars for subvector s
				   (vqsize x svsize[s] vectors) */
    float32 **idet;		/* Covariance matrix determinants for this subvq table; actually,
				   log (1/(det x (2pi)^k)), the usual thing. */
    int32 ***map;		/* map[i][j] = map from original codebook(i)/codeword(j) to
				   sequence of nearest vector quantized subvector codewords;
				   so, each map[i][j] is of length n_sv. */
    bitvec_t cb_invalid;	/* Flag for each codebook; TRUE iff entire map for that codebook
				   is invalid */
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
 *   Mappings for state 1 codewords (in original model) to codewords of this subvector codebook
 *   ...
 *   Repeated for each subvector codebook 1
 *   Repeated for each subvector codebook 2
 *   ...
 *   End
 */
subvq_t *subvq_init (char *file);

void subvq_free (subvq_t *s);

/*
 * Compare the input vector to all the subvector codewords in the given codebook, after breaking
 * up the input into subvectors, appropriately, of course.  Enter the computed Mahalanobis
 * distances, converted into logs3 values, into the given score array.  Note the shape of the
 * score array: vqsize x n_sv.
 */
void subvq_dist_eval (subvq_t *vq,
		      float32 *vec,
		      int32 **score);	/* Out: Mahalanobis distance scores (logs3 values), of
					   size and shape vq->vqsize x vq->n_sv.  Caller must
					   allocate this array. */

#endif
