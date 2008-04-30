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


/* Function to calculate the memory required for each of the count
   tables, given a path to an id_ngram file, and a pointer to an array
   of cutoffs */



#include <stdio.h>
#include "general.h"  // from libs
#include "ngram.h"
#include "idngram2lm.h"

void calc_mem_req(ng_t *ng,flag is_ascii) {

  ngram current_ngram;
  ngram previous_ngram;
  count_t *ng_count;
  int i,j;

  current_ngram.id_array = (id__t *) rr_malloc(sizeof(id__t)*ng->n);
  previous_ngram.id_array = (id__t *) rr_malloc(sizeof(id__t)*ng->n);
  
  ng_count = (count_t *) rr_calloc(ng->n,sizeof(count_t));

  current_ngram.n = ng->n;

  rewind(ng->id_gram_fp);

  while (!rr_feof(ng->id_gram_fp)) {
    for (i=0;i<=ng->n-1;i++)
      previous_ngram.id_array[i]=current_ngram.id_array[i];

    get_ngram(ng->id_gram_fp,&current_ngram,is_ascii);
    for (i=0;i<=ng->n-1;i++) {
      if (current_ngram.id_array[i] != previous_ngram.id_array[i]) {
	for (j=i;j<=ng->n-1;j++) {
	  if (j>0) {
	    if (ng_count[j] > ng->cutoffs[j-1])
	      ng->table_sizes[j]++;
	  }
	  ng_count[j] =  current_ngram.count;
	}
	i=ng->n;
      }else
	ng_count[i] += current_ngram.count;
    }
  }

  for (i=1;i<=ng->n-1;i++) {
    if (ng_count[i] > ng->cutoffs[i-1])
      ng->table_sizes[i]++;
  }

  /* Add a fudge factor, as problems can crop up with having to
     cut-off last few n-grams. */

  for (i=1;i<=ng->n-1;i++) {
    ng->table_sizes[i]+=10;
  }

  rr_iclose(ng->id_gram_fp);
  ng->id_gram_fp = rr_iopen(ng->id_gram_filename);

}
   






