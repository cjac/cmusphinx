/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * $Log$
 * Revision 1.9  2006/02/24  03:11:39  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: 1, changed ckd_calloc to my_malloc, 2, add functions to free nodes.
 * 
 * Revision 1.8.4.2  2005/10/17 05:02:05  arthchan2003
 * Usage of mymalloc and myfree leaks memory.  This is told by dmalloc when I tried to debug a dynamic algorithm for lexical tree.
 *
 * Revision 1.8.4.1  2005/06/27 05:41:46  arthchan2003
 * Added glist_delete in glist.h.  glist_delete only delete one node after a specified node. glist_delete and glist_add thus from a simple operation that allow the glist to act like a stack.
 *
 * Revision 1.8  2005/06/22 03:02:51  arthchan2003
 * 1, Fixed doxygen documentation, 2, add  keyword.
 *
 * Revision 1.3  2005/03/30 01:22:48  archan
 * Fixed mistakes in last updates. Add
 *
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


glist_t
glist_add_ptr(glist_t g, void *ptr)
{
    gnode_t *gn;

    gn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    gn->data.ptr = ptr;
    gn->next = g;
    return ((glist_t) gn);      /* Return the new head of the list */
}


glist_t
glist_add_int32(glist_t g, int32 val)
{
    gnode_t *gn;

    gn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    gn->data.i_32 = val;
    gn->next = g;
    return ((glist_t) gn);      /* Return the new head of the list */
}


glist_t
glist_add_uint32(glist_t g, uint32 val)
{
    gnode_t *gn;

    gn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    gn->data.ui_32 = val;
    gn->next = g;
    return ((glist_t) gn);      /* Return the new head of the list */
}


glist_t
glist_add_float32(glist_t g, float32 val)
{
    gnode_t *gn;

    gn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    gn->data.fl_32 = val;
    gn->next = g;
    return ((glist_t) gn);      /* Return the new head of the list */
}


glist_t
glist_add_float64(glist_t g, float64 val)
{
    gnode_t *gn;

    gn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    gn->data.fl_64 = val;
    gn->next = g;
    return ((glist_t) gn);      /* Return the new head of the list */
}


int32
glist_chkdup_ptr(glist_t g, void *val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
        if (gnode_ptr(gn) == val)
            return 1;

    return 0;
}


int32
glist_chkdup_int32(glist_t g, int32 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
        if (gnode_int32(gn) == val)
            return 1;

    return 0;
}


int32
glist_chkdup_uint32(glist_t g, uint32 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
        if (gnode_uint32(gn) == val)
            return 1;

    return 0;
}


int32
glist_chkdup_float32(glist_t g, float32 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
        if (gnode_float32(gn) == val)
            return 1;

    return 0;
}


int32
glist_chkdup_float64(glist_t g, float64 val)
{
    gnode_t *gn;

    for (gn = g; gn; gn = gnode_next(gn))
        if (gnode_float64(gn) == val)
            return 1;

    return 0;
}


void
glist_apply_ptr(glist_t g, void (*func) (void *))
{
    gnode_t *gn;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.ptr);
}


void
glist_apply_int32(glist_t g, void (*func) (int32))
{
    gnode_t *gn;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.i_32);
}


void
glist_apply_uint32(glist_t g, void (*func) (uint32))
{
    gnode_t *gn;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.ui_32);
}


void
glist_apply_float32(glist_t g, void (*func) (float32))
{
    gnode_t *gn;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.fl_32);
}


void
glist_apply_float64(glist_t g, void (*func) (float64))
{
    gnode_t *gn;

    for (gn = g; gn; gn = gn->next)
        (*func) (gn->data.fl_64);
}


void
glist_free(glist_t g)
{
    gnode_t *gn;

    while (g) {
        gn = g;
        g = gn->next;
        ckd_free((void *) gn);
    }
}


void
glist_myfree(glist_t g, int32 datasize)
{
    gnode_t *gn;

    while (g) {
        gn = g;
        g = gn->next;
        ckd_free((char *) (gn->data.ptr));
        ckd_free((char *) gn);
    }
}


int32
glist_count(glist_t g)
{
    gnode_t *gn;
    int32 n;

    for (gn = g, n = 0; gn; gn = gn->next, n++);
    return n;
}


gnode_t *
glist_tail(glist_t g)
{
    gnode_t *gn;

    if (!g)
        return NULL;

    for (gn = g; gn->next; gn = gn->next);
    return gn;
}


glist_t
glist_reverse(glist_t g)
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


gnode_t *
glist_insert_ptr(gnode_t * gn, void *ptr)
{
    gnode_t *newgn;

    newgn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    newgn->data.ptr = ptr;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}


gnode_t *
glist_insert_int32(gnode_t * gn, int32 val)
{
    gnode_t *newgn;

    newgn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    newgn->data.i_32 = val;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}


gnode_t *
glist_insert_uint32(gnode_t * gn, uint32 val)
{
    gnode_t *newgn;

    newgn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    newgn->data.ui_32 = val;
    newgn->next = gn->next;

    gn->next = newgn;

    return newgn;
}


gnode_t *
glist_insert_float32(gnode_t * gn, float32 val)
{
    gnode_t *newgn;

    newgn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    newgn->data.fl_32 = val;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}


gnode_t *
glist_insert_float64(gnode_t * gn, float64 val)
{
    gnode_t *newgn;

    newgn = (gnode_t *) ckd_calloc(1, sizeof(gnode_t));
    newgn->data.fl_64 = val;
    newgn->next = gn->next;
    gn->next = newgn;

    return newgn;
}

gnode_t *
glist_delete(gnode_t * gn)
{
    gnode_t *newgn;
    newgn = gn->next;
    if (newgn) {
        ckd_free((char *) gn);
        return newgn;
    }
    else {
        return gn;
    }
}


gnode_t *
gnode_free(gnode_t * gn, gnode_t * pred)
{
    gnode_t *next;

    next = gn->next;
    if (pred) {
        assert(pred->next == gn);

        pred->next = next;
    }

    ckd_free((char *) gn);

    return next;
}
