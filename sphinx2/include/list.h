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

#ifdef WIN32
#include <posixwin32.h>
#endif

typedef struct {
    int32	size_hint;		/* For initial allocation */
    int32	size;			/* Number entries in the list */
    int32	in_use;			/* Number entries in use in list */
    caddr_t	*list;			/* The list */
} list_t;

list_t *new_list(void);
int list_add (list_t *list, caddr_t sym, int32 idx);
caddr_t list_lookup (list_t const *list, int32 idx);
void list_insert (list_t *list, caddr_t sym);
void list_unique_insert (list_t *list, caddr_t sym);
int list_free (list_t *list);
int32 list_index (list_t const *list, caddr_t sym);
int32 listLength (list_t const *list);
void listWrite (FILE *fs, list_t const *list);
void listRead (FILE *fs, list_t *list);

#endif /* _LIST_H_ */
