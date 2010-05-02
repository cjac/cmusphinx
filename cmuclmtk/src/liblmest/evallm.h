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

/* Function prototypes for evallm */

#ifndef _EVALLM_PROTS_
#define _EVALLM_PROTS_

#include "pc_general.h"
#include "general.h"
#include "ngram.h"
#include "toolkit.h"

/* Function prototypes */

unsigned short num_of_types(int k,
			    int ind,
			    ng_t *ng);
void decode_bo_case(int bo_case,
		    int context_length,
		    FILE *annotation_fp);

void display_stats(ng_t *ng);

void display_arpa_stats(arpa_lm_t *arpa_ng);

void load_lm(ng_t *ng,
	     char *lm_filename);

void load_arpa_lm(arpa_lm_t *arpa_lm,
		  char *lm_filename);		  

void robust_load_arpa_lm(arpa_lm_t *arpa_lm,
			 char *lm_filename,
			 char* header,int max_header_length);

void parse_comline(char *input_line,
		  int *num_of_args,
		  char **args);

void compute_perplexity(ng_t *ng,
			arpa_lm_t *arpa_ng,
			char *text_stream_filename,
			char *probs_stream_filename,
			char *annotation_filename,
			char *oov_filename,
			char *fb_list_filename,
			flag backoff_from_unk_inc,
			flag backoff_from_unk_exc,
			flag backoff_from_ccs_inc,
			flag backoff_from_ccs_exc,
			flag arpa_lm,
			flag include_unks,
			double log_base);

fb_info *gen_fb_list(sih_t *vocab_ht,
		     vocab_sz_t vocab_size,
		     char **vocab,
		     flag *context_cue,
		     flag backoff_from_unk_inc,
		     flag backoff_from_unk_exc,
		     flag backoff_from_ccs_inc,
		     flag backoff_from_ccs_exc,
		     char *fb_list_filename);

void validate(ng_t *ng,
	      arpa_lm_t *arpa_ng,
	      char **words,
	      flag backoff_from_unk_inc,
	      flag backoff_from_unk_exc,
	      flag backoff_from_ccs_inc,
	      flag backoff_from_ccs_exc,
	      flag arpa_lm,
	      char *fb_list_filename);

void generate(ng_t *png,
	      arpa_lm_t *arpa_ng,
	      int num_words,
	      int random_seed,
	      char *output_filename);


double calc_prob_of(id__t sought_word,
		    id__t *context,
		    int context_length,
		    ng_t *ng,
		    arpa_lm_t *arpa_ng,
		    fb_info *fb_list,
		    int *bo_case,
		    int *actual_context_length,
		    flag arpa_lm);

void arpa_bo_ng_prob(int context_length,
		     id__t *sought_ngram,
		     arpa_lm_t *arpa_ng,
		     int verbosity,
		     double *p_prob,
		     int *bo_case);

void generate_words(ng_t *png, 
                    arpa_lm_t *pang, 
                    int num_words,
                    int random_seed,
                    char *output_filename);

void check_open_close_vocab(arpa_lm_t *arpa_lm, 
                            char * word_copy, 
                            int *i);

void show_dot(int j);

void write_arpa_k_gram_header(FILE *fp, 
                              unsigned short n);

#endif
