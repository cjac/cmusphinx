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
 * tmat.h
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added tmat_free to free allocated memory 
 *
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added tmat_chk_1skip(), and made tmat_chk_uppertri() public.
 * 
 * 10-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added tmat_dump().
 * 
 * 11-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started based on original S3 implementation.
 */


#ifndef _S3_TMAT_H_
#define _S3_TMAT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <libutil/libutil.h>
#include "s3types.h"


/*
 * Transition matrix data structure.  All phone HMMs are assumed to have the same
 * topology.
 */
typedef struct {
    int32 ***tp;	/* The transition matrices; int32 since probs in logs3 domain:
			   tp[tmatid][from-state][to-state] */
    int32 n_tmat;	/* #matrices */
    int32 n_state;	/* #source states in matrix (only the emitting states);
			   #destination states = n_state+1, it includes the exit state */
} tmat_t;


tmat_t *tmat_init (char *tmatfile,	/* In: input file */
		   float64 tpfloor);	/* In: floor value for each non-zero transition
					   probability */

void tmat_dump (tmat_t *tmat, FILE *fp);	/* For debugging */


/*
 * Checks that no transition matrix in the given object contains backward arcs.
 * Returns 0 if successful, -1 if check failed.
 */
int32 tmat_chk_uppertri (tmat_t *tmat);


/*
 * Checks that transition matrix arcs in the given object skip over at most 1 state.
 * Returns 0 if successful, -1 if check failed.
 */
int32 tmat_chk_1skip (tmat_t *tmat);

/*
 * RAH, add code to remove memory allocated by tmat_init
 */
void tmat_free (tmat_t *t);


#ifdef __cplusplus
}
#endif

#endif
