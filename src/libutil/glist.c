/*
 * glist.h -- Module for maintaining a generic, linear linked-list structure.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 09-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added glist_chkdup_*().
 * 
 * 13-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from earlier version.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "glist.h"
#include "ckd_alloc.h"


glist_t glist_add_ptr (glist_t g, void *ptr)
{
    gnode_t *gn;
    
    gn = (gnode_t *) mymalloc (sizeof(gnode_t));
    gn->data.ptr = ptr;
    gn->next = g;
    return ((glist_t) gn);	/* Return the new head of the list */
}


glist_t glist_add_int32 (glist_t g, int32 val)
{
    gnode_t *gn;
    
    gn = (gnode_t *) mymalloc (sizeof(gnode_t));
    gn->data.int32 = val;
    gn->next = g;
    return ((glist_t) gn);	/* Return the new head of the list */
}


glist_t glist_add_uint32 (glist_t g, uint32 val)
{
    gnode_t *gn;
    
    gn = (gnode_t *) mymalloc (sizeof(gnode_t));
    gn->data.uint32 = val;
    gn->next = g;
    return ((glist_t) gn);	/* Return the new head of the list */
}


glist_t glist_add_float32 (glist_t g, float32 val)
{
    gnode_t *gn;
    
    gn = (gnode_t *) mymalloc (sizeof(gnode_t));
    gn->data.float32 = val;
    gn->next = g;
    return ((glist_t) gn);	/* Return the new head of the list */
}


glist_t glist_add_float64 (glist_t g, float64 val)
{
    gnode_t *gn;
    
    gn = (gnode_t *) mymalloc (sizeof(gnode_t));
    gn->data.float64 = val;
    gn->next = g;
    return ((glist_t) gn);	/* Return the new head of the list */
}


int32 glist_chkdup_ptr (glist_t g, void *val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
	if (gnode_ptr(gn) == val)
	    return 1;
    
    return 0;
}


int32 glist_chkdup_int32 (glist_t g, int32 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
	if (gnode_int32(gn) == val)
	    return 1;
    
    return 0;
}


int32 glist_chkdup_uint32 (glist_t g, uint32 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
	if (gnode_uint32(gn) == val)
	    return 1;
    
    return 0;
}


int32 glist_chkdup_float32 (glist_t g, float32 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
	if (gnode_float32(gn) == val)
	    return 1;
    
    return 0;
}


int32 glist_chkdup_float64 (glist_t g, float64 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
	if (gnode_float64(gn) == val)
	    return 1;
    
    return 0;
}


void glist_apply_ptr (glist_t g, void (*func)(void *))
{
    gnode_t *gn;
    
    for (gn = g; gn; gn = gn->next)
	(*func)(gn->data.ptr);
}


void glist_apply_int32 (glist_t g, void (*func)(int32))
{
    gnode_t *gn;
    
    for (gn = g; gn; gn = gn->next)
	(*func)(gn->data.int32);
}


void glist_apply_uint32 (glist_t g, void (*func)(uint32))
{
    gnode_t *gn;
    
    for (gn = g; gn; gn = gn->next)
	(*func)(gn->data.uint32);
}


void glist_apply_float32 (glist_t g, void (*func)(float32))
{
    gnode_t *gn;
    
    for (gn = g; gn; gn = gn->next)
	(*func)(gn->data.float32);
}


void glist_apply_float64 (glist_t g, void (*func)(float64))
{
    gnode_t *gn;
    
    for (gn = g; gn; gn = gn->next)
	(*func)(gn->data.float64);
}


void glist_free (glist_t g)
{
    gnode_t *gn;
    
    while (g) {
	gn = g;
	g = gn->next;
	myfree((char *)gn, sizeof(gnode_t));
    }
}


void glist_myfree (glist_t g, int32 datasize)
{
    gnode_t *gn;
    
    while (g) {
	gn = g;
	g = gn->next;
	myfree((char *)(gn->data.ptr), datasize);
	myfree((char *)gn, sizeof(gnode_t));
    }
}


int32 glist_count (glist_t g)
{
    gnode_t *gn;
    int32 n;
    
    for (gn = g, n = 0; gn; gn = gn->next, n++);
    return n;
}


gnode_t *glist_tail (glist_t g)
{
    gnode_t *gn;
    
    if (! g)
	return NULL;
    
    for (gn = g; gn->next; gn = gn->next);
    return gn;
}


glist_t glist_reverse (glist_t g)
{
    gnode_t *gn, *nextgn;
    gnode_t *rev;
    
    rev = NULL;
    for (gn = g; gn; gn = nextgn) {
	nextgn = gn->next;
	
	gn->next = rev;
	rev = gn;
    }

    return rev;
}
