/*
 * linklist.h -- generic module for efficient memory management of linked list elements
 *		 of various sizes; a separate list for each size.  Elements must be
 * 		 a multiple of a pointer size.
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
 * 27-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon.
 * 		Created.
 */


#ifndef _LIBUTIL_LINKLIST_H_
#define _LIBUTIL_LINKLIST_H_


/* Allocate a link-list element of given size and return pointer to it */
char *__listelem_alloc__ (int32 elemsize, char *file, int32 line);
#define listelem_alloc(sz)	__listelem_alloc__((sz),__FILE__,__LINE__)

/* Free link-list element of given size */
void listelem_free (char *elem, int32 elemsize);

/* Print #allocation, #free operation stats */
void linklist_stats ( void );


#endif
