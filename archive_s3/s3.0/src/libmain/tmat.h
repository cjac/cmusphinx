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
 * 11-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started based on original S3 implementation.
 */


#ifndef _LIBMAIN_TMAT_H_
#define _LIBMAIN_TMAT_H_


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

#endif
