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


#include "ngram.h"
#include "idngram2lm.h"
#include <stdlib.h>

void increment_context(ng_t *ng,int k,int verbosity) {

  count_t current_count;
  int j;
  int current_table;
  int *current_pos;
  int *end_pos;
  flag discounted;

  /* Scan all the (k+1)-grams (i.e. those in table k). If any of them
     are followed by only one (k+2)-gram, and its count is bigger
     than the discounting range, then increment the count of the
     (k+1)-gram. Technique first introduced by Doug Paul. */

  current_pos = (int *)rr_calloc(k+1,sizeof(int));
  end_pos = (int *)rr_calloc(k+1,sizeof(int)); 

  current_count = 0;
  discounted = 0;
  
  for (current_pos[0]=ng->first_id;
       current_pos[0]<=ng->vocab_size;
       current_pos[0]++) {

    
    /*    fprintf(stderr, "first_id %d, vocab_size %ld, current_pos[0] %d\n", ng->first_id, ng->vocab_size, current_pos[0]); 

    fflush(stderr);*/
    if (return_count(ng->four_byte_counts,
		     ng->count_table[0],
		     ng->marg_counts,
		     ng->marg_counts4,
		     current_pos[0]) > 0) {

      current_table = 1;
      
      if (current_pos[0] == ng->vocab_size) {
	end_pos[1] = ng->num_kgrams[1]-1;
      }
      else {
	end_pos[1] = get_full_index(ng->ind[0][current_pos[0]+1],
				    ng->ptr_table[0],
 				    ng->ptr_table_size[0],
				    current_pos[0]+1)-1;
      }
      
      while (current_table > 0) {
	
	if (current_table == k) {
	  
	  if (current_pos[k] <= end_pos[k]) {

	    current_count += return_count(ng->four_byte_counts,
					  ng->count_table[k],
					  ng->count[k],
					  ng->count4[k],
					  current_pos[k]);

	    if (return_count(ng->four_byte_counts,
			     ng->count_table[k],
			     ng->count[k],
			     ng->count4[k],
			     current_pos[k]) <= ng->disc_range[k]) {
	      discounted = 1;
	    }
	    current_pos[k]++;
	  }else {

	    if (k == 1) {

	      /*	      fprintf(stderr, "k %d, first_id %d, vocab_size %ld, current_pos[k] %d\n", k, ng->first_id, ng->vocab_size, current_pos[k-1]); */
	      if (current_count >= return_count(ng->four_byte_counts,
						ng->count_table[0],
						ng->marg_counts,
						ng->marg_counts4,
						current_pos[k-1]) 
		  && !discounted) {
		

		store_count(ng->four_byte_counts,
			    ng->count_table[0],
			    ng->count_table_size,
			    ng->marg_counts,
			    ng->marg_counts4,
			    current_pos[0],
			    return_count(ng->four_byte_counts,
					 ng->count_table[0],
					 ng->marg_counts,
					 ng->marg_counts4,
					 current_pos[0])+1); 


	      }
	    }else {

	      if ((current_count >= return_count(ng->four_byte_counts,
						 ng->count_table[k-1],
						 ng->count[k-1],
						 ng->count4[k-1],
						 current_pos[k-1])) && 
		  !discounted) {

		for (j=1;j<=k-1;j++) {

		  store_count(ng->four_byte_counts,
			      ng->count_table[j],
			      ng->count_table_size,
			      ng->count[j],
			      ng->count4[j],
			      current_pos[j],
			      return_count(ng->four_byte_counts,
					   ng->count_table[j],
					   ng->count[j],
					   ng->count4[j],
					   current_pos[j])+1);
		}
		
		/*		fprintf(stderr, "j %d first_id %d, vocab_size %ld, current_pos[0] %d\n", j, ng->first_id, ng->vocab_size, current_pos[0]); */
		store_count(ng->four_byte_counts,
			    ng->count_table[0],
			    ng->count_table_size,
			    ng->marg_counts,
			    ng->marg_counts4,
			    current_pos[0],
			    return_count(ng->four_byte_counts,
					 ng->count_table[0],
					 ng->marg_counts,
					 ng->marg_counts4,
					 current_pos[0])+1);

	      }
	    }
	    current_count = 0;
	    discounted = 0;
	    current_table--;
	    if (current_table > 0)
	      current_pos[current_table]++;
	  }
	}
	else {
	  if (current_pos[current_table] <= end_pos[current_table]) {
	    current_table++;
	    if (current_pos[current_table-1] == 
		ng->num_kgrams[current_table-1]-1) {
	      end_pos[current_table] = ng->num_kgrams[current_table]-1;
	    }else {
	      end_pos[current_table] = 
		get_full_index(ng->ind[current_table-1][current_pos[current_table-1]+1],
			       ng->ptr_table[current_table-1],
			       ng->ptr_table_size[current_table-1],
			       current_pos[current_table-1]+1)-1;
	    }
	  }else {
	    current_table--;
	    if (current_table > 0)
	      current_pos[current_table]++;
	  }
	}
      }
    }
  } 

  free(current_pos);
  free(end_pos);

}

