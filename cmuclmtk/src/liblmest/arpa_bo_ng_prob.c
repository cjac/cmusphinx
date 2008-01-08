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


#include "evallm.h"
#include "idngram2lm.h"
#include "bo_ng_prob.h"
#include <stdlib.h>
#include <math.h>

void arpa_bo_ng_prob(int context_length,
		     id__t *sought_ngram,
		     arpa_lm_t *arpa_ng,
		     int verbosity,
		     double *p_prob,
		     int *bo_case) 
{  
  flag found;
  flag found_ngram;
  flag found_context;
  flag still_found;

  int length_exists;
  int ng_begin;
  int ng_end;
  int ng_middle;
  int *ng_index;

  int temp_case;

  double alpha;
  double prob;

  alpha = 0.0; /* To give no warnings at compilation time */

  ng_index = (int *) rr_malloc((context_length+1)*sizeof(int));

  if (context_length == 0) 
    *p_prob = pow(10.0,arpa_ng->probs[0][sought_ngram[0]]);
  else {

    found_ngram = 0;
    found_context = 0;

    /* Find the appropriate (context-length+1)-gram */

    length_exists = 0;
    still_found = 1;

    while (still_found && (length_exists < (context_length+1))) {
      
      found = 0;

      /* Look for (length_exists+1)-gram */

      if (length_exists == 0) {
	if (get_full_index(arpa_ng->ind[0][sought_ngram[0]],
			   arpa_ng->ptr_table[0],
			   arpa_ng->ptr_table_size[0],
			   sought_ngram[0]) <
	    get_full_index(arpa_ng->ind[0][sought_ngram[0]+1],
			   arpa_ng->ptr_table[0],
			   arpa_ng->ptr_table_size[0],
			   sought_ngram[0]+1)) {
	   found = 1;
	  ng_index[0] = sought_ngram[0];
	}
      }else {

	/* Binary search for right ngram */
	ng_begin = 
	  get_full_index(arpa_ng->ind[length_exists-1][ng_index[length_exists-1]],
			 arpa_ng->ptr_table[length_exists-1],
			 arpa_ng->ptr_table_size[length_exists-1],
			 ng_index[length_exists-1]);

	if (length_exists == 1) {
	  if (ng_index[0] < arpa_ng->vocab_size) {
	    ng_end = 
	      get_full_index(arpa_ng->ind[length_exists-1][ng_index[length_exists-1]+1],
			     arpa_ng->ptr_table[length_exists-1],
			     arpa_ng->ptr_table_size[length_exists-1],
			     ng_index[length_exists-1]+1)-1;
	  }else
	    ng_end = arpa_ng->num_kgrams[1];
	  
	}else{
	  if (ng_index[length_exists-1] < 
	      arpa_ng->num_kgrams[length_exists-1]-1) {
	    ng_end = 
	      get_full_index(arpa_ng->ind[length_exists-1][ng_index[length_exists-1]+1],
			     arpa_ng->ptr_table[length_exists-1],
			     arpa_ng->ptr_table_size[length_exists-1],
			     ng_index[length_exists-1]+1)-1;
	  }else
	    ng_end = arpa_ng->num_kgrams[length_exists];
	} 

	while (ng_begin <= ng_end) {
	  ng_middle = ng_begin + ((ng_end - ng_begin) >> 1);
	  if (sought_ngram[length_exists] < 
	      arpa_ng->word_id[length_exists][ng_middle]) {
	    ng_end = ng_middle - 1;
	  }else {
	    if (sought_ngram[length_exists] > 
		arpa_ng->word_id[length_exists][ng_middle]) {
	      ng_begin = ng_middle + 1;
	    }else {
	      found = 1;
	      ng_index[length_exists] = ng_middle;
	      break;
	    }
	  }
	}
      }

      if (found) 
	length_exists++;
      else 
	still_found = 0;

    }
    if (length_exists == (context_length+1))
      found_ngram = 1;
    
    if (length_exists >= context_length)
      found_context = 1;

    if (found_context)
      alpha = pow(10.0,arpa_ng->bo_weight[context_length-1][ng_index[context_length-1]]);

    if (found_ngram) {
      prob = pow(10.0,arpa_ng->probs[context_length][ng_index[context_length]]);
      temp_case = 0;
    }else {
      arpa_bo_ng_prob(context_length-1,
		      &sought_ngram[1],
		      arpa_ng,
		      verbosity,
		      &prob,
		      bo_case);

      temp_case = 2;      
      if (found_context) {
	prob*=alpha;
	temp_case=1;
      }            
    }

    /*
     * PWP: coding change.  Numbers were previously coded base-3.
     * Now base-4, since (int) pow(4,i) == 1 << (2*i)
     */
    
    *p_prob = prob;
    *bo_case += temp_case * (1 << (2*(context_length-1)));
  }

  if (*p_prob > 1.0) 
    warn_prob_error(sought_ngram, context_length, *p_prob);

  free(ng_index);

}
