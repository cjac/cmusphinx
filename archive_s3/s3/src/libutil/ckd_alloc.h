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
 * Revision 1.1  2000/04/24  09:39:42  lenzo
 * s3 import.
 * 
 * 
 * 07-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Liberally copied from Eric Thayer's original and adapted.
 * 		This pacakge helps trace memory allocation for debugging and
 * 		detecting memory leaks, when MEM_DEBUG is defined.
 */
