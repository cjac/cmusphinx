/*
 * bitvec.h -- Bit vector type.
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
 * 13-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added bitvec_uint32size().
 * 
 * 05-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added bitvec_count_set().
 * 
 * 17-Jul-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _LIBUTIL_BITVEC_H_
#define _LIBUTIL_BITVEC_H_


#include "prim_type.h"
#include "ckd_alloc.h"


typedef uint32 *bitvec_t;

/*
 * NOTE: The following definitions haven't been designed for arbitrary usage!!
 */

/* No. of uint32 words allocated to represent a bitvector of the given size n */
#define bitvec_uint32size(n)	(((n)+31)>>5)

#define bitvec_alloc(n)		((bitvec_t) ckd_calloc (((n)+31)>>5, sizeof(uint32)))

#define bitvec_free(v)		ckd_free((char *)(v))

#define bitvec_set(v,b)		(v[(b)>>5] |= (1 << ((b) & 0x001f)))

#define bitvec_clear(v,b)	(v[(b)>>5] &= ~(1 << ((b) & 0x001f)))

#define bitvec_clear_all(v,n)	memset(v, 0, (((n)+31)>>5)*sizeof(uint32))

#define bitvec_is_set(v,b)	(v[(b)>>5] & (1 << ((b) & 0x001f)))

#define bitvec_is_clear(v,b)	(! (bitvec_is_set(v,b)))


/*
 * Return the number of bits set in the given bit-vector.
 */
int32 bitvec_count_set (bitvec_t vec,	/* In: Bit vector to search */
			int32 len);	/* In: Lenght of above bit vector */

#endif
