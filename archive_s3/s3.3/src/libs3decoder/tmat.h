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


#endif
