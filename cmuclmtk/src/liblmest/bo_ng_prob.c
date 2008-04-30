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


/* Return the probability of the (context_length+1)-gram stored in
   sought_ngram */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "pc_general.h"  // from libs
#include "general.h"    // from libs
#include "ngram.h"
#include "idngram2lm.h"
#include "disc_meth.h"

void warn_prob_error(id__t *sought_ngram, unsigned short context_length, double prob)
{
  int i;
  fprintf(stderr,"Error : P( %d | ",sought_ngram[context_length]);
  for (i=0;i<=context_length-1;i++)
    fprintf(stderr,"%d ",sought_ngram[i]);

  fprintf(stderr,") = %g\n",prob);
  exit(1);
}

void bo_ng_prob(int context_length,
		id__t *sought_ngram,
		ng_t *ng,
		int verbosity,
		double *p_prob,
		int *bo_case) {

  flag found;
  flag found_ngram;
  flag found_context;
  flag still_found;

  int length_exists;
  int ng_begin;
  int ng_end;
  int ng_middle;
  int ncount;
  int contcount;
  int *ng_index;

  int temp_case;

  double alpha;
  double prob;
  double discounted_ncount;

  /* Initialise variables (unnecessary, but gives warning-free compilation */

  ncount = 0;
  contcount = 0;
  alpha = 0.0;
  discounted_ncount = 0.0;

  ng_index = (int *) rr_malloc((context_length+1)*sizeof(int));

  if (context_length == 0) {

    *p_prob = ng->uni_probs[sought_ngram[0]];
    if (*p_prob<= 0.0 || *p_prob >= 1.0) {
      pc_message(verbosity,1,"Warning : P( %d ) == %g\n", 
		 sought_ngram[0], *p_prob);
    }
  }else {

    found_ngram = 0;
    found_context = 0;
    ncount = -1;

    /* Find the appropriate (context-length+1)-gram */

    length_exists = 0;
    still_found = 1;

    while (still_found && (length_exists < (context_length+1))) {
      
      found = 0;

      /* Look for (length_exists+1)-gram */
      if (length_exists == 0) {
	if (return_count(ng->four_byte_counts,
			 ng->count_table[0],
			 ng->marg_counts,
			 ng->marg_counts4,
			 sought_ngram[0]) > 0) {
	  found = 1;
	  ng_index[0] = sought_ngram[0];
	}
      }else {

	/* Binary search for right ngram */	
	ng_begin = 
	  get_full_index(ng->ind[length_exists-1][ng_index[length_exists-1]],
			 ng->ptr_table[length_exists-1],
			 ng->ptr_table_size[length_exists-1],
			 ng_index[length_exists-1]);

	if (length_exists == 1) {
	  if (ng_index[0] < ng->vocab_size) {
	    ng_end = 
	      get_full_index(ng->ind[length_exists-1][ng_index[length_exists-1]+1],
			     ng->ptr_table[length_exists-1],
			     ng->ptr_table_size[length_exists-1],
			     ng_index[length_exists-1]+1)-1;
	  }else
	    ng_end = ng->num_kgrams[1];
	}else {
	  if (ng_index[length_exists-1] < ng->num_kgrams[length_exists-1]-1) {
	    ng_end = 
	      get_full_index(ng->ind[length_exists-1][ng_index[length_exists-1]+1],
			     ng->ptr_table[length_exists-1],
			     ng->ptr_table_size[length_exists-1],
			     ng_index[length_exists-1]+1)-1;
	  }else 
	    ng_end = ng->num_kgrams[length_exists];
	}

	while (ng_begin <= ng_end) {
	  ng_middle = ng_begin + ((ng_end - ng_begin) >> 1);
	  if (sought_ngram[length_exists] < 
	      ng->word_id[length_exists][ng_middle]) {
	    ng_end = ng_middle - 1;
	  }else {
	    if (sought_ngram[length_exists] > ng->word_id[length_exists][ng_middle]) 
	      ng_begin = ng_middle + 1;
	    else {
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

    if (length_exists == (context_length+1)) {
      found_ngram = 1;

      ncount = return_count(ng->four_byte_counts,
			    ng->count_table[context_length],
			    ng->count[context_length],
			    ng->count4[context_length],
			    ng_index[context_length]);
    }

    if (length_exists >= context_length) {
      found_context = 1;
      if (context_length == 1) {
	contcount = return_count(ng->four_byte_counts,
				 ng->count_table[0],
				 ng->marg_counts,
				 ng->marg_counts4,
				 ng_index[0]);
      }else {
	contcount = return_count(ng->four_byte_counts,
				 ng->count_table[context_length-1],
				 ng->count[context_length-1],
				 ng->count4[context_length-1],
				 ng_index[context_length-1]);
      }
    }

    if (found_context)
      alpha = ng_double_alpha(ng, context_length-1,ng_index[context_length-1]);

    /* If it occurred then return appropriate prob */

    if (found_ngram) {
      
      if(ng->disc_meth==NULL)
	ng->disc_meth=(disc_meth_t *) disc_meth_init(ng->discounting_method);

      assert(ng->disc_meth);      	
      discounted_ncount = 
	NG_DISC_METH(ng)->dump_discounted_ngram_count(ng,context_length,ncount,contcount,ng_index);

      prob = discounted_ncount / (double) contcount;
      temp_case = 0;

      if (prob <= 0.0 || prob >= 1.0) {
	pc_message(verbosity,1,"Warning : P(%d) = %g (%g / %d)\n",
		   sought_ngram[0],prob, discounted_ncount,contcount);
	pc_message(verbosity,1,"ncount = %d\n",ncount);
      }

    }else {
      bo_ng_prob(context_length-1,
		 &sought_ngram[1],
		 ng,
		 verbosity,
		 &prob,
		 bo_case);

      temp_case = 2;

      if (found_context) {
	prob*=alpha;
	temp_case=1;
      }
    }
    
    *p_prob = prob;
    *bo_case += temp_case * (1 << (2*(context_length-1)));    
  }

  if (*p_prob > 1.0) 
    warn_prob_error(sought_ngram, context_length, *p_prob);

  free(ng_index);

}
