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
 * ckd_alloc.h -- Memory allocation package.
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

/*
 * The following functions are similar to the malloc family, except that they have
 * two additional parameters, caller_file and caller_line, for error reporting.
 * All functions print a diagnostic message and exit if any error occurs.
 */

void *__ckd_calloc__(size_t n_elem, size_t elem_size, char *caller_file, int caller_line);

void *__ckd_malloc__(size_t size, char *caller_file, int caller_line);

void *__ckd_realloc__(void *ptr, size_t new_size, char *caller_file, int caller_line);

void __ckd_free__(void *ptr, char *caller_file, int caller_line);

/*
 * Like strdup, except if an error occurs, in which case it prints a diagnostic message
 * and exits.
 */
char *__ckd_salloc__(char *origstr, char *caller_file, int32 caller_line);

/*
 * Allocate a 2-D array and return ptr to it (ie, ptr to vector of ptrs).
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
void **__ckd_calloc_2d__(int32 d1, int32 d2,	/* In: #elements in the 2 dimensions */
			 int32 elemsize,	/* In: Size (#bytes) of each element */
			 char *caller_file, int32 caller_line);	/* In */

/*
 * Allocate a 3-D array and return ptr to it.
 * The data area is allocated in one block so it can also be treated as a 1-D array.
 */
void ***__ckd_calloc_3d__(int32 d1, int32 d2, int32 d3,	/* In: #elems in the dims */
			  int32 elemsize,		/* In: Size (#bytes) per element */
			  char *caller_file, int32 caller_line);	/* In */

/* Free a 2-D array (ptr) previously allocated by ckd_calloc_2d */
void __ckd_free_2d__(void **ptr, char *caller_file, int32 caller_line);

/* Free a 3-D array (ptr) previously allocated by ckd_calloc_3d */
void __ckd_free_3d__(void ***ptr, char *caller_file, int32 caller_line);


/*
 * Macros to simplify the use of above functions.
 * One should use these, rather than target functions directly.
 */
#define ckd_calloc(n,sz)	__ckd_calloc__((n),(sz),__FILE__,__LINE__)
#define ckd_malloc(sz)		__ckd_malloc__((sz),__FILE__,__LINE__)
#define ckd_realloc(ptr,sz)	__ckd_realloc__(ptr,(sz),__FILE__,__LINE__)
#define ckd_free(ptr)		__ckd_free__(ptr,__FILE__,__LINE__)
#define ckd_salloc(ptr)		__ckd_salloc__(ptr,__FILE__,__LINE__)
#define ckd_calloc_2d(d1,d2,sz)	__ckd_calloc_2d__((d1),(d2),(sz),__FILE__,__LINE__)
#define ckd_free_2d(ptr)	__ckd_free_2d__(ptr,__FILE__,__LINE__);
#define ckd_calloc_3d(d1,d2,d3,sz) __ckd_calloc_3d__((d1),(d2),(d3),(sz),__FILE__,__LINE__)
#define ckd_free_3d(ptr)	__ckd_free_3d__(ptr,__FILE__,__LINE__);


/* For memory allocation tracing and debugging */
#if defined(MEM_DEBUG)

void
ckd_alloc_snapshot(void);

void
ckd_alloc_growth_report(void);

#endif /* MEM_DEBUG */

#endif /* CKD_ALLOC_H */ 


/*
 * Log record.  Maintained by CVS.
 *
 * $Log$
 * Revision 1.4  2002/12/03  23:03:02  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 * 
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 07-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Liberally copied from Eric Thayer's original and adapted.
 * 		This pacakge helps trace memory allocation for debugging and
 * 		detecting memory leaks, when MEM_DEBUG is defined.
 */
