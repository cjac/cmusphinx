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


/* Function prototypes */

#ifndef _IDNGRAM2LM_H_
#define _IDNGRAM2LM_H_

#include "ngram.h"


void show_idngram_nlines(int nlines, int verbosity);


void show_idngram_corruption_mesg();


     /**
	Return the number of types 
      */
unsigned short num_of_types(int k, 
			    int ind,
			    ng_t *ng
			    );

/**
   Get ngram from file stream
 */
int get_ngram(FILE *id_ngram_fp, /**< the file stream, either binary or ascii*/
	      ngram *ng,         /**< The n-gram */
	      flag is_ascii      /**< Whether it is an ascii file */
	      );

void calc_mem_req(ng_t *ng, /**< The n-gram */
		  flag is_ascii /**< Whether it is an ascii file */
		  );
void write_arpa_lm(ng_t *ng,int verbosity);
void write_bin_lm(ng_t *ng,int verbosity);

index__t new_index(ngram_sz_t full_index,
		   ptr_tab_t *ind_table,
		   ptr_tab_sz_t *ind_table_size,
		   ngram_sz_t position_in_list);

int get_full_index(index__t short_index,
		   ptr_tab_t *ind_table,
		   ptr_tab_sz_t ind_table_size,
		   int position_in_list);

void compute_gt_discount(int            n,
			 int            *freq_of_freq,
			 int            fof_size,
			 unsigned short *disc_range,
			 int cutoff,
			 int verbosity,
			 disc_val_t **discounted_values);
int lookup_index_of(int *lookup_table, 
		    int lookup_table_size, 
		    int intintval);
void compute_unigram(ng_t *ng,int verbosity);
void compute_back_off(ng_t *ng,int n,int verbosity);
void bo_ng_prob(int context_length,
		id__t *sought_ngram,
		ng_t *ng,
		int verbosity,
		double *p_prob,
		int *bo_case);
void increment_context(ng_t *ng, int k, int verbosity);
unsigned short short_alpha(double long_alpha,
			   double *alpha_array,
			   unsigned short *size_of_alpha_array,
			   int elements_in_range,
			   double min_range,
			   double max_range);

double double_alpha(unsigned short short_alpha,
		    double *alpha_array,
		    int size_of_alpha_array,
		    int elements_in_range,
		    double min_range,
		    double max_range);

void guess_mem(int total_mem,
	       int middle_size,
	       int end_size,
	       int n,
	       table_size_t *table_sizes,
	       int verbosity);

void read_voc(char *filename, int verbosity,   
	      sih_t *p_vocab_ht, char ***p_vocab, 
	      vocab_sz_t *p_vocab_size);

void store_normal_count(ng_t *ng,
			int position,
			count_t count,
			int N
			);

void store_marginal_count(ng_t *ng,
			  int position,
			  count_t count,
			  int N
			  );

void store_count(flag four_byte_counts,
		 count_t *count_table,
		 int count_table_size,
		 count_ind_t *short_counts,
		 count_t *long_counts,
		 int position,
		 count_t count);

count_t return_count(flag four_byte_counts,
		 count_t *count_table,
		 count_ind_t *short_counts,
		 count_t *long_counts,
		 int position);

void ngram_check_order(ngram *cur, 
			 ngram *prev, 
			 int N, 
			 int nlines);

int ngram_find_pos_of_novelty(ngram *cur, ngram *prev, int N, int nlines);

#endif
