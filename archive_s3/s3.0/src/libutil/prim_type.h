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
 * prim_type.h -- Primitive types; more machine-independent.
 *
 * 
 * HISTORY
 * 
 * 12-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added arraysize_t, point_t, fpoint_t.
 * 
 * 01-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added anytype_t.
 * 
 * 08-31-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _LIBUTIL_PRIM_TYPE_H_
#define _LIBUTIL_PRIM_TYPE_H_


typedef int		int32;
typedef short		int16;
typedef char		int8;
typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
typedef float		float32;
typedef double		float64;

typedef union anytype_s {
    void *ptr;		/* User defined data types at this ptr */
    int32 int32;
    uint32 uint32;
    float32 float32;
    float64 float64;
} anytype_t;


/* Useful constants */
#define MAX_INT32		((int32) 0x7fffffff)
#define MAX_INT16		((int16) 0x00007fff)
#define MAX_INT8		((int8)  0x0000007f)

#define MAX_NEG_INT32		((int32) 0x80000000)
#define MAX_NEG_INT16		((int16) 0xffff8000)
#define MAX_NEG_INT8		((int8)  0xffffff80)

#define MAX_UINT32		((uint32) 0xffffffff)
#define MAX_UINT16		((uint16) 0x0000ffff)
#define MAX_UINT8		((uint8)  0x000000ff)

/* The following are approximate; IEEE floating point standards might quibble! */
#define MAX_POS_FLOAT32		3.4e+38f
#define MIN_POS_FLOAT32		1.2e-38f	/* But not 0 */
#define MAX_POS_FLOAT64		1.8e+307
#define MIN_POS_FLOAT64		2.2e-308

/* Will the following really work?? */
#define MAX_NEG_FLOAT32		((float32) (-MAX_POS_FLOAT32))
#define MIN_NEG_FLOAT32		((float32) (-MIN_POS_FLOAT32))
#define MAX_NEG_FLOAT64		((float64) (-MAX_POS_FLOAT64))
#define MIN_NEG_FLOAT64		((float64) (-MIN_POS_FLOAT64))


#endif
