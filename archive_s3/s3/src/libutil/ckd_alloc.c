/*
 * ckd_alloc.c -- Memory allocation package.
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
 * file: ckd_alloc.c
 * 
 * traceability: 
 * 
 * description: 
 * 
 * author: 
 * 
 *********************************************************************/

static char rcsid[] = "@(#)$Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "prim_type.h"
#include "err.h"
#include "ckd_alloc.h"


/* Detailed malloc debugging -- rkm@cs.cmu.edu */

typedef struct {
    char *alloc_begin, *alloc_end;	/* Beginning/end of allocated memory block,
					   including blank spaceo on either side */
    char *use_begin, *use_end;		/* Beginning/end of portion of allocated block
					   that is passed on to user */
    int32 use_size;			/* For verification; must be equal to
					   use_end - use_begin + 1 */
    int32 freed;			/* Whether user has freed this block */
} memblk_t;

#define CKD_PADDING	128	/* Must allow proper alignment */
#define CKD_BLANK	0x5a	/* Pattern for blank space which shouldn't be corrupted */

static memblk_t *memblk = NULL;
static int32 n_memblk = 0;	/* #memory blocks actually allocated */
static int32 max_memblk = 0;	/* Max #memory blocks that can be allocated */
static int32 debug = 0;		/* Whether detailed debugging has been initiated */


static void chkblk (int32 b)
{
    memblk_t *m;
    int32 i, j;
    
    assert ((b >= 0) && (b < n_memblk) && (b < max_memblk));
    
    /* Make sure sizes of this block are consistent */
    m = memblk+b;
    if (m->use_begin - m->alloc_begin != CKD_PADDING)
	E_FATAL("Blk %d bad\n", b);
    if (m->alloc_end - m->use_end != CKD_PADDING)
	E_FATAL("Blk %d bad\n", b);
    if (m->use_end - m->use_begin + 1 != m->use_size)
	E_FATAL("Blk %d bad\n", b);
    
    /* Make sure padding on either side of user-useable block still contains CKD_BLANKs */
    for (i = 0, j = m->use_size+CKD_PADDING; i < CKD_PADDING; i++, j++)
	if ((m->alloc_begin[i] != CKD_BLANK) || (m->alloc_begin[j] != CKD_BLANK))
	    E_FATAL("Blk %d corrupted\n", b);
    
    /* If freed, make sure entire block contains CKD_BLANKs */
    if (m->freed) {
	for (i = 0; i < m->use_size; i++)
	    if (m->use_begin[i] != CKD_BLANK)
		E_FATAL("Blk %d corrupted\n", b);
    }
}


void ckd_chkmem ( void )
{
    int32 i;
    
    assert ((n_memblk >= 0) && (n_memblk < max_memblk));
    
    for (i = 0; i < n_memblk; i++)
	chkblk (i);
    E_INFO("%d blks OK\n", n_memblk);
}


static char *mycalloc (int32 size)
{
    char *buf;
    int32 i, j;
    memblk_t *m;

    ckd_chkmem ();
    
    /* Allocate memory plus padding on either side */
    if ((buf = calloc ((size+(CKD_PADDING<<1)), 1)) == NULL)
	return NULL;
    
    /* Fill padding with CKD_BLANK char which ought never be disturbed */
    for (i = 0, j = size+CKD_PADDING; i < CKD_PADDING; i++, j++) {
	buf[i] = buf[j] = CKD_BLANK;
    }
    
    m = memblk + n_memblk;
    
    m->alloc_begin = buf;
    m->alloc_end = buf + size + (CKD_PADDING<<1) - 1;
    m->use_begin = buf + CKD_PADDING;
    m->use_end = m->use_begin + size - 1;
    m->use_size = size;
    m->freed = 0;
    
    n_memblk++;
    
    return (m->use_begin);
}


static char *myrealloc (char *buf, int32 size)
{
    E_FATAL("myrecalloc not implemented\n");
}


static void myfree (char *buf)
{
    int32 i;
    memblk_t *m;
    
    ckd_chkmem ();
    
    /* Locate block being freed by user */
    for (i = 0; (i < n_memblk) && (memblk[i].use_begin != buf); i++);
    if (i >= n_memblk)
	E_FATAL("Unknown buf\n");
    
    /* Check if block was already freed */
    m = memblk + i;
    if (m->freed)
	E_FATAL("Already freed buf\n");

    /* Fill entire block with CKD_BLANK char */
    m->freed = 1;
    for (i = 0; i < m->use_size; i++)
	m->use_begin[i] = CKD_BLANK;
}


/* Eric's simple memory debugger */
#if defined(MEM_DEBUG)

#define MAXPTR	100000

static void *ptr_list[MAXPTR];
static char was_present[MAXPTR];
static char *file[MAXPTR];
static int32 line[MAXPTR];

static int max_ptr = 0;


void
ckd_alloc_snapshot()
{
    uint32 i;

    bzero(was_present, MAXPTR);

    for (i = 0; i < max_ptr; i++) {
	if (ptr_list[i] != NULL) {
	    was_present[i] = 1;
	}
    }
}


void
ckd_alloc_growth_report()
{
    uint32 i;

    printf("%s(%d): allocation growth report:\n",
	   __FILE__, __LINE__);
    for (i = 0; i < max_ptr; i++) {
	if ((ptr_list[i] != NULL) && !was_present[i]) {
	    printf("\t%s(%d)\n", file[i], line[i]);
	}
    }
}


static int full_rep = FALSE;

static void
add_ptr(void *mem, char *alloc_file, int alloc_line)
{
    int i;

    for (i = 0; i < MAXPTR; i++)
	if (ptr_list[i] == NULL)
	    break;

    if (i == MAXPTR) {
	if (!full_rep) {
	    printf("%s(%d): memory table full\n",
		   __FILE__, __LINE__);
	    
	    full_rep = 1;
	}

	return;
    }

    ptr_list[i] = mem;
    file[i] = alloc_file;
    line[i] = alloc_line;

    if (i+1 > max_ptr) max_ptr = i+1;
}


static int
find_ptr(void *mem)
{
    int i;

    for (i = 0; i < max_ptr; i++) {
	if (ptr_list[i] == mem)
	    break;
    }

    return i;
}


static int
del_ptr(void *mem)
{
    int loc;

    loc = find_ptr(mem);

    if (loc == max_ptr) {
	printf("del_ptr: not on list %p\n", mem);

	return FALSE;
    }

    ptr_list[loc] = NULL;

    return TRUE;
}


void
ptr_stat()
{
    int i;

    for (i = 0; i < max_ptr; i++) {
	if (ptr_list[i] != NULL) {
	    printf("%p <- %s(%d)\n", 
		   ptr_list[i], file[i], line[i]);
	}
    }
}

#endif	/* MEM_DEBUG */


void ckd_debug (int32 n_blk)
{
    if (debug) {
	E_WARN("Memory debugging already on; call ignored\n");
	return;
    }
    
    E_INFO("Enabling memory debugging with %d blocks\n", n_blk);
    
    if (n_blk < 1000) {
	E_WARN("Bad #blocks argument to ckd_debug: %d, using 1000\n", n_blk);
	n_blk = 1000;
    }
    max_memblk = n_blk;
    
    if ((memblk = (memblk_t *) malloc (max_memblk * sizeof(memblk_t))) == NULL)
	E_FATAL("malloc failed\n");
    n_memblk = 0;
    
    debug = 1;
}


void *__ckd_calloc__(size_t n_elem, size_t elem_size,
		     char *caller_file, int caller_line)
{
    void *mem;

    mem = debug ? mycalloc (n_elem * elem_size) : calloc(n_elem, elem_size);
    if (mem == NULL) {
	E_FATAL("calloc(%d,%d) failed from %s(%d)\n", n_elem, elem_size,
		caller_file, caller_line);
    }

#if defined(MEM_DEBUG)
    add_ptr(mem, caller_file, caller_line);
    printf ("ckd_calloc from %s(%d): %d %d = %08x\n",
	    caller_file, caller_line, n_elem, elem_size, mem);
    fflush (stdout);
#endif

    return mem;
}


void *__ckd_malloc__(size_t size,
		     char *caller_file, int caller_line)
{
    void *mem;

    mem = debug ? mycalloc (size) : malloc(size);
    if (mem == NULL) {
	E_FATAL("malloc(%d) failed %s(%d)\n", size,
		caller_file, caller_line);
    }

#if defined(MEM_DEBUG)
    add_ptr(mem, caller_file, caller_line);
    printf ("ckd_malloc from %s(%d): %d = %08x\n",
	    caller_file, caller_line, size, mem);
    fflush (stdout);
#endif

    return mem;
}


void *__ckd_realloc__(void *ptr, size_t new_size,
		      char *caller_file, int caller_line)
{
    void *mem;

#if defined(MEM_DEBUG)
    del_ptr(ptr);
#endif

    mem = debug ? myrealloc(ptr, new_size) : realloc(ptr, new_size);
    if (mem == NULL) {
	E_FATAL("realloc(%d) failed from %s(%d)\n", new_size,
		caller_file, caller_line);
    }

#if defined(MEM_DEBUG)
    add_ptr(mem, caller_file, caller_line);
    printf ("ckd_realloc from %s(%d): %08x %d = %08x\n",
	    caller_file, caller_line, ptr, new_size, mem);
    fflush (stdout);
#endif

    return mem;
}


void __ckd_free__(void *ptr, char *file, int line)
{
#if defined(MEM_DEBUG)
    if (!del_ptr(ptr)) {
	E_WARN("%p not in allocated ptr list for call %s(%d)\n",
	       ptr, file, line);
    }
    printf ("ckd_free from %s(%d): %08x\n",
	    file, line, ptr);
    fflush (stdout);
#endif

    if (debug)
	myfree(ptr);
    else
	free(ptr);
}


char *__ckd_salloc__ (char *orig, char *caller_file, int32 caller_line)
{
    int32 len = strlen(orig)+1;
    char *buf;

    buf = (char *) __ckd_malloc__(len, caller_file, caller_line);

#if defined(MEM_DEBUG)
    printf ("ckd_salloc from %s(%d): %08x %d = %08x\n",
	    caller_file, caller_line, orig, len, buf);
    fflush (stdout);
#endif

    strcpy (buf, orig);
    return (buf);
}


void **__ckd_calloc_2d__ (int32 d1, int32 d2, int32 elemsize,
			  char *caller_file, int32 caller_line)
{
    char **ref, *mem;
    int32 i, offset;

    mem = (char *) __ckd_calloc__(d1*d2, elemsize, caller_file, caller_line);
    ref = (char **) __ckd_malloc__(d1 * sizeof(void *), caller_file, caller_line);

#if defined(MEM_DEBUG)
    printf ("ckd_calloc_2d from %s(%d): %d %d %d = ref= %08x mem= %08x\n",
	    caller_file, caller_line, d1, d2, elemsize, ref, mem);
    fflush (stdout);
#endif

    for (i = 0, offset = 0; i < d1; i++, offset += d2*elemsize)
	ref[i] = mem + offset;

    return ((void **) ref);
}


void __ckd_free_2d__ (void **ptr, char *caller_file, int32 caller_line)
{
#if defined(MEM_DEBUG)
    printf ("ckd_free_2d from %s(%d): %08x %08x\n",
	    caller_file, caller_line, ptr, ptr[0]);
    fflush (stdout);
#endif
    __ckd_free__(ptr[0], caller_file, caller_line);
    __ckd_free__(ptr, caller_file, caller_line);
}


void ***__ckd_calloc_3d__ (int32 d1, int32 d2, int32 d3, int32 elemsize,
			   char *caller_file, int32 caller_line)
{
    char ***ref1, **ref2, *mem;
    int32 i, j, offset;

    mem = (char *) __ckd_calloc__(d1*d2*d3, elemsize, caller_file, caller_line);
    ref1 = (char ***) __ckd_malloc__(d1 * sizeof(void **), caller_file, caller_line);
    ref2 = (char **) __ckd_malloc__(d1*d2 * sizeof(void *), caller_file, caller_line);

#if defined(MEM_DEBUG)
    printf ("ckd_calloc_3d from %s(%d): %d %d %d %d = ref1= %08x ref2= %08x mem= %08x\n",
	    caller_file, caller_line, d1, d2, d3, elemsize, ref1, ref2, mem);
    fflush (stdout);
#endif

    for (i = 0, offset = 0; i < d1; i++, offset += d2)
	ref1[i] = ref2+offset;

    offset = 0;
    for (i = 0; i < d1; i++) {
	for (j = 0; j < d2; j++) {
	    ref1[i][j] = mem + offset;
	    offset += d3*elemsize;
	}
    }

    return ((void ***) ref1);
}


void ___ckd_free_3d__ (void ***ptr, char *caller_file, int32 caller_line)
{
#if defined(MEM_DEBUG)
    printf ("ckd_free_3d from %s(%d): %08x %08x %08x\n",
	    caller_file, caller_line, ptr, ptr[0], ptr[0][0]);
    fflush (stdout);
#endif

    __ckd_free__(ptr[0][0], caller_file, caller_line);
    __ckd_free__(ptr[0], caller_file, caller_line);
    __ckd_free__(ptr, caller_file, caller_line);
}


/*
 * Log record.  Maintained by CVS.
 *
 * $Log$
 * Revision 1.1  2000/04/24  09:39:42  lenzo
 * s3 import.
 * 
 * Revision 1.2  1995/06/02  14:52:54  eht
 * Use pwp's error printing package
 *
 * Revision 1.1  1995/02/13  15:47:16  eht
 * Initial revision
 * 
 * 02-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added detailed malloc debugging feature (ckd_chkmem()).
 */
