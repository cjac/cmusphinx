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


/*
  HISTORY
  $Log: short_indices.c,v $
  Revision 1.4  2006/06/19 21:02:07  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.3  2006/04/13 17:33:26  archan
  0, This particular change enable 32bit LM creation in ARPA format.  1, rationalized/messed up the data type, (Careful, with reading and writing for 8-byte data structure, they are not exactly working at this point.) 2, all switches in idngram2lm is changed to be implemented by the disc_meth object.

  Revision 1.2  2006/04/02 23:50:28  archan
  Indented, changed styles, removed junks.

 */
/* Procedures to allow the indices which point into the next table to
   be stored as two-byte integers, rather than four byte integers. 

   All that is stored of the indices are two bytes corresponding to
   the offset. For each table, we also store an array indicating where
   the non-offset part increases (which we can do, since the indices
   will be monotonically increasing).
   
  This is probably best illustrated with an example. Suppose our array
  contains the following:

  ind_table[0]=0
  ind_table[1]=30
  ind_table[2]=73
  
  and the stored indices are called index[0..100]. Then the actual
  indices are

  actual_index[0..29] = (0*key) + index[0..29]
  actual_index[30..72] = (1*key) + index[30..72]
  actual_index[73..100] = (2*key) + index[73..100]

*/

#include "general.h"
#include "ngram.h"

index__t new_index(ngram_sz_t   full_index,
		   ptr_tab_t    *ind_table,
		   ptr_tab_sz_t *ind_table_size,
		   ngram_sz_t   position_in_list
		   ) 
{  
  /*  fprintf(stderr,"%d\n", *ind_table_size);
      fflush(stderr);*/
  if (full_index - (((*ind_table_size)-1)*KEY) >= KEY) {
    ind_table[*ind_table_size] = position_in_list;
    (*ind_table_size)++;
  }
  /*  fprintf(stderr,"%d\n",*ind_table_size);
      fflush(stderr);*/

  return (full_index % KEY);
}

int get_full_index(index__t short_index,
		   ptr_tab_t *ind_table,
		   ptr_tab_sz_t ind_table_size,
		   int position_in_list) 
{

  int lower_bound;
  int upper_bound;
  int new_try;

  /* Binary search for position_in_list */

  lower_bound = 0;
  upper_bound = ind_table_size-1;
  
  /* Search range is inclusive */

  if (ind_table_size > 0) {
  
    if (position_in_list >= ind_table[upper_bound])
      lower_bound = upper_bound;

    while (upper_bound-lower_bound > 1) {
      new_try = (upper_bound + lower_bound) / 2;
      if (ind_table[new_try] > position_in_list)
	upper_bound = new_try;
      else
	lower_bound = new_try;
    }
  }

  /* Return the appropriate value */
  return((lower_bound*KEY)+short_index);

}

