/*
 * mllr.h -- Application of MLLR regression matrices to codebook means
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
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _MS_MLLR_H_
#define _MS_MLLR_H_

#include <s3types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Load a regression matrix from the given file.  Space for the matrix is allocated
 * by this routine.  (The regression "matrix" is actually a matrix A and a vector B.)
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ms_mllr_read_regmat (const char *regmatfile,	/* In: File to be read */
			float32 ****A,		/* Out: [*A][stream][streamlen][streamlen] */
			float32 ***B,		/* Out: [*B][stream][streamlen] */
			int32 *streamlen,	/* In: Dimension of each feature stream */
			int32 n_stream);	/* In: #Feature streams */

/*
 * Free a regression matrix previously read in by mllr_read_regmat.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ms_mllr_free_regmat (float32 ***A,		/* In: A[stream][streamlen][streamlen] */
			float32 **B,		/* In: B[stream][streamlen] */
			int32 *streamlen,	/* In: Dimension of each feature stream */
			int32 n_stream);	/* In: #Feature streams */

/*
 * Transform a mixture density mean matrix according to the given regression matrix.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ms_mllr_norm_mgau (float32 ***mean,	/* In/Out: Set of vectors to be transformed:
					   mean[stream][density] */
		      int32 n_density,	/* In: #densities in above mean matrix */
		      float32 ***A,	/* In: "matrix" portion of regression matrix */
		      float32 **B,	/* In: "vector" portion of regression matrix */
		      int32 *streamlen,	/* In: Dimension of each feature stream */
		      int32 n_stream);	/* In: #Feature streams */

#ifdef __cplusplus
}
#endif

#endif
