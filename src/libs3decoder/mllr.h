/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * 24-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon University
 *              First checked in from sphinx 3.0 to sphinx 3.5
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _MLLR_H_
#define _MLLR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <s3types.h>
#include "cont_mgau.h"
#include "mdef.h"


/* Dump the regression matrix from a given file */
void mllr_dump(float32 **A, float32 *B,int32 veclen);
/*
 * Load a regression matrix from the given file.  Space for the matrix is allocated
 * by this routine.  (The regression "matrix" is actually a matrix A and a vector B.)
 * Return value: 0 if successful, -1 otherwise.
 */
int32 mllr_read_regmat (const char *regmatfile,	/* In: File to be read */
			float32 ***A,		/* Out: [*A][streamlen][streamlen] */
			float32 **B,		/* Out: [*B][streamlen] */
			int32 ceplen);          /* In: vector length */

/*
 * Free a regression matrix previously read in by mllr_read_regmat.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 mllr_free_regmat (float32 **A,		/* In: A[streamlen][streamlen] */
			float32 *B		/* In: B[streamlen] */
			);


/*
 * Transform a mixture density mean matrix according to the given regression matrix.
 * Return value: 0 if successful, -1 otherwise.
 */

int32 mllr_norm_mgau (mgau_model_t *mgauset, /* In/Out: The gaussian distribution needs to be transformed */
		      float32 **A,	/* In: "matrix" portion of regression matrix */
		      float32 *B,	/* In: "vector" portion of regression matrix */
		      mdef_t *mdef      /* In : model definition file */
		      );

#ifdef __cplusplus
}
#endif

#endif
