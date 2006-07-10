/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.10  2006/02/24 03:11:39  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: 1, changed ckd_calloc to my_malloc, 2, add functions to free nodes.
 *
 * Revision 1.9.4.1  2005/06/27 05:41:46  arthchan2003
 * Added glist_delete in glist.h.  glist_delete only delete one node after a specified node. glist_delete and glist_add thus from a simple operation that allow the glist to act like a stack.
 *
 * Revision 1.9  2005/06/22 03:02:51  arthchan2003
 * 1, Fixed doxygen documentation, 2, add  keyword.
 *
 * Revision 1.4  2005/05/03 04:09:11  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
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


/**
 * \file glist.h
 * \brief Generic linked-lists maintenance.
 *
 * Only insert at the head of the list.  A convenient little
 * linked-list package, but a double-edged sword: the user must keep
 * track of the data type within the linked list elements.  When it
 * was first written, there was no selective deletions except to
 * destroy the entire list.  This is modified in later version. 
 * 
 * 
 * (C++ would be good for this, but that's a double-edged sword as well.)
 */


#ifndef _LIBUTIL_GLIST_H_
#define _LIBUTIL_GLIST_H_

#include <stdlib.h>
#include "prim_type.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** A node in a generic list 
 */
typedef struct gnode_s {
    anytype_t data;		/** See prim_type.h */
    struct gnode_s *next;	/** Next node in list */
} gnode_t;
typedef gnode_t *glist_t;	/** Head of a list of gnodes */


/** Access macros, for convenience 
 */
#define gnode_ptr(g)		((g)->data.ptr)
#define gnode_int32(g)		((g)->data.i_32)
#define gnode_uint32(g)		((g)->data.ui_32)
#define gnode_float32(g)	((g)->data.fl_32)
#define gnode_float64(g)	((g)->data.fl_64)
#define gnode_next(g)		((g)->next)


/**
 * Create and insert a new list node, with the given user-defined data, at the HEAD
 * of the given generic list.  Return the new list thus formed.
 * g may be NULL to indicate an initially empty list.
 * (Too bad there's no function overloading.)
 */
glist_t glist_add_ptr (glist_t g,  /**< a link list */
		       void *ptr   /**< a pointer */
    );
  
glist_t glist_add_int32 (glist_t g, /**< a link list */
			 int32 val  /**< an integer value */
    );
glist_t glist_add_uint32 (glist_t g,  /**< a link list */
			  uint32 val  /**< an unsigned integer value */
    );
glist_t glist_add_float32 (glist_t g, /**< a link list */
			   float32 val /**< a float32 vlaue */
    );
glist_t glist_add_float64 (glist_t g, /**< a link list */
			   float64 val  /**< a float64 vlaue */
    );



/**
 * Create and insert a new list node, with the given user-defined data, after
 * the given generic node gn.  gn cannot be NULL.
 * Return ptr to the newly created gnode_t.
 */

gnode_t *glist_insert_ptr (gnode_t *gn, /**< a generic node which ptr will be inserted after it*/
			   void *ptr /**< pointer inserted */
    );
gnode_t *glist_insert_int32 (gnode_t *gn, /**< a generic node which a value will be inserted after it*/
			     int32 val /**< int32 inserted */
    );
gnode_t *glist_insert_uint32 (gnode_t *gn, /**< a generic node which a value will be inserted after it*/
			      uint32 val /**< uint32 inserted */
    );
gnode_t *glist_insert_float32 (gnode_t *gn, /**< a generic node which a value will be inserted after it*/
			       float32 val /**< float32 inserted */
    );
gnode_t *glist_insert_float64 (gnode_t *gn, /**< a generic node which a value will be inserted after it*/
			       float64 val /**< float64 inserted */
    );

/**
 * Delete a list node, with the given user-defined data, after
 * the given generic node gn.  gn cannot be NULL.
 * Return ptr to the newly created gnode_t.
 * It is more a mirror image of glist_add_* family of functions.
 */

gnode_t *glist_delete (gnode_t *gn /**< a generic node which ptr will be deleted after it. */
    );

/**
 * Check the given glist to see if it already contains the given value (of appropriate type).
 * In the case of the ptr, only the pointer values are compared, not the data pointed to by them.
 * Return value: 1 if match found, 0 if not.
 */
int32 glist_chkdup_ptr (glist_t g, void *val);	/* List and value to check for */
int32 glist_chkdup_int32 (glist_t g, int32 val);
int32 glist_chkdup_uint32 (glist_t g, uint32 val);
int32 glist_chkdup_float32 (glist_t g, float32 val);
int32 glist_chkdup_float64 (glist_t g, float64 val);


/**
 * Reverse the order of the given glist.  (glist_add() adds to the head; one might
 * ultimately want the reverse of that.)
 * NOTE: The list is reversed "in place"; i.e., no new memory is allocated.
 * @return: The head of the new list.
 */
glist_t glist_reverse (glist_t g /**< input link list */
    );


/**
   Count the number of element in a given link list 
   @return the number of elements in the given glist_t 
*/
int32 glist_count (glist_t g /**< input link list */
    );


/**
 * Apply the given function to the user-defined data.ptr for each node in the list.
 * (Again, too bad there's no function overloading in C.)
 */
void glist_apply_ptr (glist_t g, void (*func)(void *));
void glist_apply_int32 (glist_t g, void (*func)(int32));
void glist_apply_uint32 (glist_t g, void (*func)(uint32));
void glist_apply_float32 (glist_t g, void (*func)(float32));
void glist_apply_float64 (glist_t g, void (*func)(float64));


/**
 * Free the given generic list; user-defined data contained within is not
 * automatically freed.  The caller must have done that already.
 */
void glist_free (glist_t g);


/**
 * Free the user-defined data (i.e., g->data.ptr) contained at each node of the given
 * glist (using myfree()).  Then free the glist.  "datasize" is the size of the
 * user-defined data at each node, and is needed by myfree().
 */
void glist_myfree (glist_t g, int32 datasize);


/**
 * Free the given node, gn, of a glist, pred being its predecessor in the list.
 * Return ptr to the next node in the list after the freed node.
 */
gnode_t *gnode_free(gnode_t *gn, 
		    gnode_t *pred
    );

/**
 * Return the last node in the given list.
 */
gnode_t *glist_tail (glist_t g);

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
