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
 * ckd_alloc.h -- Memory allocation package.
 *
 * 
 * HISTORY
 * 
 * 19-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Removed file,line arguments from free functions.
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * file: ckd_alloc.h
 * 
 * traceability: 
 * 
 * description: 
 * 
 * author: 
 * 
 *********************************************************************/


#ifndef _LIBUTIL_CKD_ALLOC_H_
#define _LIBUTIL_CKD_ALLOC_H_


#include <stdlib.h>
#include "prim_type.h"


/*
 * The following functions are similar to the malloc family, except that they have
 * two additional parameters, caller_file and caller_line, for error reporting.
 * All functions print a diagnostic message and exit if any error occurs.
 */

void *__ckd_calloc__(size_t n_elem, size_t elem_size,
		     const char *caller_file, int caller_line);

void *__ckd_malloc__(size_t size,
		     const char *caller_file, int caller_line);

void *__ckd_realloc__(void *ptr, size_t new_size,
		      const char *caller_file, int caller_line);

/*
 * Like strdup, except that if an error occurs it prints a diagnostic message
 * and exits.
 */
char *__ckd_salloc__(const char *origstr,
		     const char *caller_file, int32 caller_line);

/*
 * Allocate a 2-D array and return ptr to it (ie, ptr to vector of ptrs).
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
void **__ckd_calloc_2d__(int32 d1, int32 d2,	/* In: #elements in the 2 dimensions */
			 int32 elemsize,	/* In: Size (#bytes) of each element */
			 const char *caller_file, int32 caller_line);	/* In */

/*
 * Allocate a 3-D array and return ptr to it.
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
void ***__ckd_calloc_3d__(int32 d1, int32 d2, int32 d3,	/* In: #elems in the dims */
			  int32 elemsize,		/* In: Size (#bytes) per element */
			  const char *caller_file, int32 caller_line);	/* In */

/* Free a 2-D array (ptr) previously allocated by ckd_calloc_2d */
void ckd_free_2d(void **ptr);


/* Free a 3-D array (ptr) previously allocated by ckd_calloc_3d */
void ckd_free_3d(void ***ptr);


/*
 * Macros to simplify the use of above functions.
 * One should use these, rather than target functions directly.
 */
#define ckd_calloc(n,sz)	__ckd_calloc__((n),(sz),__FILE__,__LINE__)
#define ckd_malloc(sz)		__ckd_malloc__((sz),__FILE__,__LINE__)
#define ckd_realloc(ptr,sz)	__ckd_realloc__(ptr,(sz),__FILE__,__LINE__)
#define ckd_salloc(ptr)		__ckd_salloc__(ptr,__FILE__,__LINE__)
#define ckd_calloc_2d(d1,d2,sz)	__ckd_calloc_2d__((d1),(d2),(sz),__FILE__,__LINE__)
#define ckd_calloc_3d(d1,d2,d3,sz) __ckd_calloc_3d__((d1),(d2),(d3),(sz),__FILE__,__LINE__)

#define ckd_free(ptr)		free(ptr)


/*
 * Functions for managing graph elements without wasting memory.
 * Allocate and return an element of size elemsize.  elemsize must be a multiple of
 * a pointer size.  The allocated element is not zeroed, unlike calloc.
 * The function internally allocates a block that can accommodate several such elements,
 * anticipating future allocation requests.
 */
char *__mymalloc__ (int32 elemsize, char *file, int32 line);

/*
 * Free a graph element (of given size) that was previously allocated using mymalloc.
 * The element is not really freed; it is returned to an internally maintained
 * freelist pool.
 */
void __myfree__ (char *elem, int32 elemsize, char *file, int32 line);

/*
 * Macros to simplify the use of above functions.  One should use these, rather
 * than target functions directly.
 * For debugging purposes one can redefine the following to malloc() and free().
 */
#define mymalloc(sz)		__mymalloc__((sz),__FILE__,__LINE__)
#define myfree(ptr,sz)		__myfree__(ptr,(sz),__FILE__,__LINE__)


#endif
