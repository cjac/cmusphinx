/* ====================================================================
 * Copyright (c) 1999-2006 Carnegie Mellon University.  All rights
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

/* Procedures to deal with counts correctly, taking account of whether
we are dealing with two- or four-byte counts. */

#include "ngram.h"
#include "idngram2lm.h"


void store_normal_count(ng_t* ng, int position, count_t cnt, int N)
{
  store_count(ng->four_byte_counts,
	      ng->count_table[N],
	      ng->count_table_size,
	      ng->count[N],
	      ng->count4[N],
	      position,
	      cnt); 
}

void store_marginal_count(ng_t* ng, int position, count_t cnt, int N)
{
  store_count(ng->four_byte_counts,
	      ng->count_table[N],
	      ng->count_table_size,
	      ng->marg_counts,
	      ng->marg_counts4,
	      position,
	      cnt); 
}

/*
  ARCHAN: Please, hide this from the applications. Please
 */
void store_count(flag four_byte_counts,
		 count_t *count_table,
		 int count_table_size,
		 count_ind_t *short_counts,
		 count_t *long_counts,
		 int position,
		 count_t count) 
{

  if (four_byte_counts) 
    long_counts[position] = count;
  else 
    short_counts[position] = lookup_index_of(count_table,
					     count_table_size,
					     count);
}

count_t return_count(flag four_byte_counts,
		 count_t *count_table,
		 count_ind_t *short_counts,
		 count_t *long_counts,
		 int position) 
{
  count_t ct=0;
  count_ind_t sc=0;
    
  if (four_byte_counts){
    assert(long_counts);
    ct = long_counts[position];
  }else {
    assert(short_counts);
    assert(count_table);
    sc=short_counts[position];
    /*    fprintf(stderr, "ct %d, position %d, count_table %d\n",ct,position, count_table);
	  fflush(stderr);*/
    ct= count_table[(count_t)sc];
  }
  return ct;
}
