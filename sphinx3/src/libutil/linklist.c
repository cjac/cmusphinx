/*
 * linklist.c -- generic module for efficient memory management of linked list elements
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
 * $Log$
 * Revision 1.1  2004/08/09  00:17:13  arthchan2003
 * Incorporating s3.0 align, at this point, there are still some small problems in align but they don't hurt. For example, the score doesn't match with s3.0 and the output will have problem if files are piped to /dev/null/. I think we can go for it.
 * 
 * Revision 1.1  2003/02/12 16:30:42  cbq
 * A simple n-best program for sphinx.
 *
 * Revision 1.1  2000/04/24 09:39:42  lenzo
 * s3 import.
 *
 * 
 * 03-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon.
 * 		Rewrote to keep a list of lists rather than an array of lists,
 * 		and to increase allocation blocksize, gradually, within a given list.
 * 
 * 15-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon.
 * 		Added "static" declaration to list[] and n_list.
 * 
 * 27-Apr-94	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon.
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <ckd_alloc.h>
#include "linklist.h"


/*
 * A separate linked list for each element-size.  Element-size must be a multiple
 * of pointer-size.
 */
typedef struct list_s {
    char **freelist;		/* ptr to first element in freelist */
    struct list_s *next;	/* Next linked list */
    int32 elemsize;		/* #(char *) in element */
    int32 blocksize;		/* #elements to alloc if run out of free elments */
    int32 blk_alloc;		/* #Alloc operations before increasing blocksize */
    int32 n_alloc;
    int32 n_freed;
} list_t;
static list_t *head = NULL;

#define MIN_ALLOC	50	/* Min #elements to allocate in one block */


char *__listelem_alloc__ (int32 elemsize, char *caller_file, int32 caller_line)
{
    int32 j;
    char **cpp, *cp;
    list_t *prev, *list;
    
    /* Find list for elemsize, if existing */
    prev = NULL;
    for (list = head; list && (list->elemsize != elemsize); list = list->next)
	prev = list;

    if (! list) {
	/* New list element size encountered, create new list entry */
	if ((elemsize % sizeof(void *)) != 0)
	    E_FATAL("List item size (%d) not multiple of sizeof(void *)\n", elemsize);
	
	list = (list_t *) ckd_calloc (1, sizeof(list_t));
	list->freelist = NULL;
	list->elemsize = elemsize;
	list->blocksize = MIN_ALLOC;
	list->blk_alloc = (1<<18) / (list->blocksize * sizeof(elemsize));
	list->n_alloc = 0;
	list->n_freed = 0;

	/* Link entry at head of list */
	list->next = head;
	head = list;
    } else if (prev) {
	/* List found; move entry to head of list */
	prev->next = list->next;
	list->next = head;
	head = list;
    }
    
    /* Allocate a new block if list empty */
    if (list->freelist == NULL) {
	/* Check if block size should be increased (if many requests for this size) */
	if (list->blk_alloc == 0) {
	    list->blocksize <<= 1;
	    list->blk_alloc = (1<<18) / (list->blocksize * sizeof(elemsize));
	    if (list->blk_alloc <= 0)
		list->blk_alloc = (int32)0x70000000;	/* Limit blocksize to new value */
	}

	/* Allocate block */
	cpp = list->freelist = (char **) __ckd_calloc__ (list->blocksize, elemsize,
							 caller_file, caller_line);
	cp = (char *) cpp;
	for (j = list->blocksize - 1; j > 0; --j) {
	    cp += elemsize;
	    *cpp = cp;
	    cpp = (char **)cp;
	}
	*cpp = NULL;
	--(list->blk_alloc);
    }
    
    /* Unlink and return first element in freelist */
    cp = (char *)(list->freelist);
    list->freelist = (char **)(*(list->freelist));
    (list->n_alloc)++;
    
    return (cp);
}


void listelem_free (char *elem, int32 elemsize)
{
    char **cpp;
    list_t *prev, *list;
    
    /* Find list for elemsize */
    prev = NULL;
    for (list = head; list && (list->elemsize != elemsize); list = list->next)
	prev = list;

    if (! list) {
	E_FATAL("Unknown list item size: %d\n", elemsize);
    } else if (prev) {
	/* List found; move entry to head of list */
	prev->next = list->next;
	list->next = head;
	head = list;
    }

    /*
     * Insert freed item at head of list.
     * NOTE: skipping check for size being multiple of (void *).
     */
    cpp = (char **) elem;
    *cpp = (char *) list->freelist;
    list->freelist = cpp;
    (list->n_freed)++;
}


void linklist_stats ( void )
{
    list_t *list;
    char **cpp;
    int32 n;
    
    E_INFO("Linklist stats:\n");
    for (list = head; list; list = list->next) {
	for (n = 0, cpp = list->freelist; cpp; cpp = (char **)(*cpp), n++);
	printf ("\telemsize %d, #alloc %d, #freed %d, #freelist %d\n",
		list->elemsize, list->n_alloc, list->n_freed, n);
    }
}
