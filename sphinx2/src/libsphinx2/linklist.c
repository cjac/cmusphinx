/* ====================================================================
 * Copyright (c) 1994-2000 Carnegie Mellon University.  All rights 
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
 * linklist.c -- generic module for efficient memory management of linked list elements
 *		 of various sizes; a separate list for each size.  Elements must be
 * 		 a multiple of a pointer size.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.3  2001/03/29  21:09:41  lenzo
 * Getting the compile under windows back to normal
 * 
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:50  lenzo
 * Initial import of sphinx2
 *
 *
 * 
 * 15-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon.
 * 		Added "static" declaration to list[] and n_list.
 * 
 * 27-Apr-94	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon.
 * 		Created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "s2types.h"

#define QUIT(x)		{fprintf x; exit(-1);}


#define MAX_LIST	16
#define MAX_ALLOC	40944


/*
 * Elements are seen as structures with an array of void pointers.  So element size
 * must be integral muliple of (void *).
 */
typedef struct list_s {
    void **freelist;	/* ptr to first element in freelist */
    int32 elem_size;	/* #(char *) in element */
    int32 n_malloc;	/* #elements to malloc if run out of free elments */
} list_t;
static list_t list[MAX_LIST];
static int32 n_list = 0;


void *listelem_alloc (int32 elem_size)
{
    int32 i, j;
    void **cpp, *cp;
    
    for (i = 0; i < n_list; i++) {
	if (list[i].elem_size == elem_size)
	    break;
    }
    if (i >= n_list) {
	/* New list element size encountered, create new list entry */
	if (n_list >= MAX_LIST)
	    QUIT((stdout, "%s(%d): **ERROR** Increase MAX_LIST\n", __FILE__, __LINE__));
	if (elem_size > MAX_ALLOC)
	    QUIT((stdout, "%s(%d): **ERROR** Increase MAX_ALLOC to %d\n",
		  __FILE__, __LINE__, elem_size));
	if ((elem_size % sizeof(char *)))
	    QUIT((stdout, "%s(%d): **ERROR** Element size (%d) not multiple of (char *)\n",
		  __FILE__, __LINE__, elem_size));
	
	list[n_list].freelist = NULL;
	list[n_list].elem_size = elem_size;
	list[n_list].n_malloc = MAX_ALLOC / elem_size;
	i = n_list++;
    }
    
    if (list[i].freelist == NULL) {
	cpp = list[i].freelist = (void **) malloc (list[i].n_malloc * elem_size);
	cp = (void *) cpp;
	for (j = list[i].n_malloc-1; j > 0; --j) {
	    (char*)cp += elem_size;
	    *cpp = cp;
	    cpp = (void **)cp;
	}
	*cpp = NULL;
    }
    
    cp = list[i].freelist;
    list[i].freelist = *(list[i].freelist);
    return (cp);
}

void listelem_free (void *elem, int32 elem_size)
{
    int32 i;
    void **cpp;
    
    for (i = 0; i < n_list; i++) {
	if (list[i].elem_size == elem_size)
	    break;
    }
    if (i >= n_list)
	QUIT((stdout, "%s(%d): **ERROR** elem_size (%d) not in known list\n",
	      __FILE__, __LINE__, elem_size));
    
    cpp = elem;
    *cpp = list[i].freelist;
    list[i].freelist = cpp;
}
