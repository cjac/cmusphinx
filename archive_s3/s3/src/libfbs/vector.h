/*
 * vector.h -- vector routines.
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
 * 30-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created from Eric Thayer's version.
 */

/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * File: vector.h
 * 
 * Traceability: 
 * 
 * Description: 
 * 
 * Author: 
 * 	$Author$
 *********************************************************************/

#ifndef _LIBFBS_VECTOR_H_
#define _LIBFBS_VECTOR_H_

#include <libutil/prim_type.h>

typedef float *vector_t;

vector_t
vector_alloc(int32 n_dim);

int
vector_free(vector_t v);

void
vector_floor(vector_t v, int32 dim, float32 f);

void
vector_nz_floor(vector_t v, int32 dim, float32 f);

int32
vector_normalize(vector_t v, int32 dim);

void
vector_print(vector_t v, uint32 dim);

#endif /* VECTOR_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/04/24  09:39:41  lenzo
 * s3 import.
 * 
 * Revision 1.2  1995/10/09  20:55:35  eht
 * Changes for prim_type.h
 *
 * Revision 1.1  1995/08/15  13:44:14  eht
 * Initial revision
 *
 *
 */
