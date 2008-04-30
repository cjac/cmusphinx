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


/* Compute the back off alphas for table n */
/* ie if called with n=2, then compute the bigram alphas */

#include <stdlib.h>
#include "ngram.h"
#include "idngram2lm.h"
#include "disc_meth.h"
#include "pc_general.h"   // from libs

void compute_back_off(ng_t *ng,int n, int verbosity) {

  int  *current_pos;
  int  *end_pos;
  id__t *sought_ngram;
  int current_table;
  count_t ng_count;
  count_t marg_count;
  count_t i;
  double sum_cond_prob;
  double sum_bo_prob;
  double discounted_ngcount;
  double cond_prob;
  double bo_prob;
  double discount_mass;
  double leftout_bo_prob;
  double alpha;
  int bo_case;
  sum_cond_prob = 0.0;
  sum_bo_prob = 0.0;
  discounted_ngcount = 0.0;
  
  current_pos = (int *)rr_calloc(n+1,sizeof(int));
  sought_ngram = (id__t *) rr_calloc(n+1,sizeof(id__t));
  end_pos = (int *)rr_calloc(n+1,sizeof(int)); 
  
  /* Process the tree so that we get all the n-grams out in the right
     order. */
  
  for (current_pos[0]=ng->first_id;current_pos[0]<=ng->vocab_size;current_pos[0]++) {

    /*    fprintf(stderr,"%d \n",current_pos[0]);*/

    if (return_count(ng->four_byte_counts,
		     ng->count_table[0],
		     ng->marg_counts,
		     ng->marg_counts4,
		     current_pos[0]) > 0) {

      current_table = 1;
      
      if (current_pos[0] == ng->vocab_size) 
	end_pos[1] = ng->num_kgrams[1]-1;
      else 
 	end_pos[1] = get_full_index(ng->ind[0][current_pos[0]+1],
				    ng->ptr_table[0],
				    ng->ptr_table_size[0],
				    current_pos[0]+1)-1;

      while (current_table > 0) {

	if (current_table == n) {

	  if (current_pos[n] <= end_pos[n]){
	    ng_count = return_count(ng->four_byte_counts,
				    ng->count_table[n],
				    ng->count[n],
				    ng->count4[n],
				    current_pos[n]);

	    if(n==1){
	      marg_count = return_count(ng->four_byte_counts,
				       ng->count_table[0],
				       ng->marg_counts,
				       ng->marg_counts4,
				       current_pos[0]);
	    }else{
	      marg_count = return_count(ng->four_byte_counts,
				       ng->count_table[n-1],
				       ng->count[n-1],
				       ng->count4[n-1],
				       current_pos[n-1]);
	    }

	    assert(ng->disc_meth);
	    discounted_ngcount = 
	      NG_DISC_METH(ng)->dump_discounted_ngram_count(ng,n,ng_count,marg_count,current_pos);
	      
	    cond_prob = (double) discounted_ngcount / marg_count;
	    sum_cond_prob += cond_prob;

	    /* Fill up sought ngram array with correct stuff */

	    for (i=1;i<=n;i++) 
	      sought_ngram[i-1] = ng->word_id[i][current_pos[i]];

	    bo_ng_prob(n-1,sought_ngram,ng,verbosity,&bo_prob,&bo_case);
	    sum_bo_prob += bo_prob;
	    current_pos[n]++;			
					       
	  }else {

	    discount_mass = 1.0 - sum_cond_prob;

	    if (discount_mass < 1e-10) {
	      discount_mass = 0.0;
	      pc_message(verbosity,2,"Warning : Back off weight for %s(id %d) ",
			 ng->vocab[current_pos[0]],current_pos[0]);
	      for (i=1;i<=n-1;i++) 
		pc_message(verbosity,2,"%s(id %d) ",ng->vocab[ng->word_id[i][current_pos[i]]],ng->word_id[i][current_pos[i]]);
	      pc_message(verbosity,2,
			 "is set to 0 (sum of probs = %f).\nMay cause problems with zero probabilities.\n",sum_cond_prob);
	    }

	    leftout_bo_prob = 1.0 - sum_bo_prob;
	    if (leftout_bo_prob < 1e-10) 
	      leftout_bo_prob = 0.0;

	    if (leftout_bo_prob > 0.0) 
	      alpha = discount_mass / leftout_bo_prob;
	    else {
	      alpha = 0.0;	/* Will not be used. Should happen very rarely. */
	      pc_message(verbosity,2,"Warning : Back off weight for %s(id %d) ",
			 ng->vocab[current_pos[0]],current_pos[0]);
	      for (i=1;i<=n-1;i++) 
		pc_message(verbosity,2,"%s(id %d) ",ng->vocab[ng->word_id[i][current_pos[i]]],ng->word_id[i][current_pos[i]]);
	      pc_message(verbosity,2,
			 "is set to 0.\nMay cause problems with zero probabilities.\n");
	    }

	    ng_short_alpha(ng,alpha, n-1, current_pos[n-1]);
	  
	    /* Finished current (n-1)-gram */
	    sum_cond_prob = 0.0;
	    sum_bo_prob = 0.0;
	    current_table--;
	    if (current_table > 0)
	      current_pos[current_table]++;

	  }
	}else {
	  if (current_pos[current_table] <= end_pos[current_table]) {
	    current_table++;
	    if (current_pos[current_table-1] == ng->num_kgrams[current_table-1]-1) 
	      end_pos[current_table] = ng->num_kgrams[current_table]-1;
	    else 
	      end_pos[current_table] = get_full_index(ng->ind[current_table-1][current_pos[current_table-1]+1],
						      ng->ptr_table[current_table-1],
						      ng->ptr_table_size[current_table-1],
						      current_pos[current_table-1]+1)-1;
	  }else {
	    current_table--;
	    if (current_table > 0) 
	      current_pos[current_table]++;
	  }
	}
      }
    } else {     /* Now deal with zeroton unigrams */
      if (n == 1)
	ng_short_alpha(ng,1.0,0,current_pos[0]);
    }
  }
  free(end_pos);
  free(current_pos);
  free(sought_ngram);
  
}
