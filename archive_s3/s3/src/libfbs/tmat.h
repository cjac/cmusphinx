/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
