/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * sort.h -- Packaged I/O routines.
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
 * Revision 1.2  2006/02/24  03:15:17  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Added a customized sorting algorithm.
 * 
 * Revision 1.1.2.1  2005/09/25 19:38:00  arthchan2003
 * Added a stupid insertion sorting routine. It was used in gau_select (now gauvq) and gau_read.
 *
 *
 * 05-Jan-2004	A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *              created. 
 */

#ifndef _LIBUTIL_SORT_H_
#define _LIBUTIL_SORT_H_

#include "sort.h"
#include "prim_type.h"
#include <stdlib.h>

typedef struct {
  int32 key;
  float32 val;
} sort_t;

typedef struct {
  sort_t* s_array;
  int32 size;
} sort_array_t;

void init_sort_array(int size, sort_array_t *s);
void change_size(int size,sort_array_t *s);
void free_sort_array(sort_array_t *s);
void print_sort_array(sort_array_t *s);
void reverse_sort_array(sort_array_t *s);
void insertion_sort(sort_array_t *s); /* Sorting an array to a descending order*/

#endif
