/*
 * list.h -- Linked list related
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
 * 16-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Fil Alleva's original.
 */


#ifndef _LIST_H_
#define _LIST_H_

#include <sys/types.h>
#include <s2types.h>

#if (WIN32)
#include <posixwin32.h>
#endif

typedef struct {
    int32	size_hint;		/* For initial allocation */
    int32	size;			/* Number entries in the list */
    int32	in_use;			/* Number entries in use in list */
    caddr_t	*list;			/* The list */
} list_t;

extern list_t *new_list();


#endif  _LIST_H_
