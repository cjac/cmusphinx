/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * bitvec.h -- Bit vector type.
 *
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
