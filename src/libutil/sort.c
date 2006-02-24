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
 * sort.c -- A sorting routine
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
 * 06-01-04	A. Chan (archan@cs.cmu.edu) of Carnegie Mellon University
 *              first created. 
 */

#include "sort.h"
#include "prim_type.h"
#include <stdlib.h>
#include "ckd_alloc.h"
#include "err.h"

/* Sorting an array to a descending order*/
void insertion_sort(sort_array_t *s)
{
  int32 i,j;
  sort_t se;
  for(i = 1;i < s->size ;i++)
    {
      se.key = s->s_array[i].key; 
      se.val = s->s_array[i].val;

      j=i;
      while ((j>0)&&(s->s_array[j-1].val > se.val))
	{
	  s->s_array[j].key= s->s_array[j-1].key;
	  s->s_array[j].val= s->s_array[j-1].val;
	  j--;
	}
      s->s_array[j].key = se.key;
      s->s_array[j].val = se.val;
    }
}

void reverse_sort_array(sort_array_t *s)
{
  int32 i;
  sort_t se;
  for(i=0;i<(s->size)/2 ;i++)
    {
      se.key=s->s_array[i].key;
      se.val=s->s_array[i].val;
      s->s_array[i].key=s->s_array[s->size-1-i].key;
      s->s_array[i].val=s->s_array[s->size-1-i].val;
      s->s_array[s->size-1-i].key=se.key;
      s->s_array[s->size-1-i].val=se.val;
    }
}

void init_sort_array(int size, sort_array_t *s)
{
  s=(sort_array_t *)malloc(sizeof(sort_array_t));
  s->size=size;
  s->s_array=(sort_t*)ckd_calloc(s->size,sizeof(sort_t));
}

void free_sort_array(sort_array_t *s)
{
  ckd_free((sort_t*)(s->s_array));
  /*free(s);*/
}

void change_size(int size,sort_array_t *s)
{
  s->size=size;
}

void print_sort_array(sort_array_t *s)
{
  int i;
  for(i=0;i<s->size;i++)
    {
      E_INFO("Rank %d, Key %d %f\n",i,s->s_array[i].key,s->s_array[i].val);
    }
}

