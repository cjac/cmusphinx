/*
 * tmat.h
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
 * 13-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created, liberally borrowed from Eric Thayer's S3 trainer.
 */


#ifndef _LIBFBS_TMAT_H_
#define _LIBFBS_TMAT_H_


#include <libutil/prim_type.h>

#include "s3types.h"
#include <s3.h>


/*
 * Transition matrix data structure.
 */
typedef struct {
    int32 ***tp;	/* The transition matrices; int32 since probs in logs3 domain:
			   tp[tmatid][from-state][to-state] */
    int32 n_tmat;	/* #matrices */
    int32 n_state;	/* #states/matrix (#from states = n_state-1, since the final
			   state is a non-emitting state, with no transition out of it) */
    float32 tpfloor;	/* Floor value applied to real (non-zero-prob) transitions */
} tmat_t;


tmat_t *tmat_init (char *tmatfile,	/* In: input file */
		   float32 tpfloor);	/* In: floor value for each non-zero transition
					   probability */

tmat_t *tmat_gettmat ( void );

/*
 * Check if all tmats are upper triangular.
 * Return value: 0 if yes, -1 if failed.
 */
int32 tmat_chk_uppertri (tmat_t *tmat);	/* In: tmat structure read in by tmat_init */


#endif
