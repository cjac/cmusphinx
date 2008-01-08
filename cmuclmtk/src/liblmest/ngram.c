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
 * ngram.c - implement methods of ngram manipulation. 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2006 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: ngram.c,v $
 * Revision 1.2  2006/04/13 17:33:26  archan
 * 0, This particular change enable 32bit LM creation in ARPA format.  1, rationalized/messed up the data type, (Careful, with reading and writing for 8-byte data structure, they are not exactly working at this point.) 2, all switches in idngram2lm is changed to be implemented by the disc_meth object.
 *
 * Revision 1.1  2006/04/02 23:52:11  archan
 * Added routines to specialize in methods of idngram and ngram.
 *
 *
 */

#include "idngram2lm.h"
#include "ngram.h"

void ngram_copy(ngram *tgt, ngram *src,int N)
{
  int i;
  for (i=0;i<=N-1;i++) 
    tgt->id_array[i] = src->id_array[i];

  tgt->count = src->count;	

}

flag ngram_chk_contains_unks(ngram *gm, int N)
{
  flag contains_unks=0;
  int i=0;

  for(i=0;i<=N-1;i++){
    if (gm->id_array[i] == 0)
      contains_unks = 1;
  }
  return contains_unks;
}

void ngram_print(ngram *gm, int N)
{
  int i;
  fprintf(stderr,"n=%d ",N);
  for(i=0;i<=N-1;i++)
    fprintf(stderr,"id[%d] %d ",i,gm->id_array[i]);
  fprintf(stderr,"count=%d ",gm->count);
  fprintf(stderr,"\n");
}

void ngram_check_order(ngram *cur, ngram *prev, int N, int nlines)
{
  int i;
  for (i=0;i<=N-1;i++) {
    if (cur->id_array[i] < prev->id_array[i]) {
      if (nlines < 5)  /* Error ocurred early - file format? */
	quit(-1,"Error : n-gram ordering problem - could be due to using wrong file format.\nCheck whether id n-gram file is in ascii or binary format.\n");
      else 
	quit(-1,"Error : n-grams are not correctly ordered. Error occurred at ngram %d.\n",nlines);
    }
  }
}

int ngram_find_pos_of_novelty(ngram *cur, ngram *prev, int N, int nlines)
{
  int i;
  int pon; /* pos of novelty */
  pon = N;

  for (i=0;i<=N-1;i++) {
    if (cur->id_array[i] > prev->id_array[i]) {
      pon = i;
      i=N;
    }else if(cur->id_array[i] < prev->id_array[i]) {
      if (nlines < 5)  /* Error ocurred early - file format? */
	quit(-1,"Error : n-gram ordering problem - could be due to using wrong file format.\nCheck whether id n-gram file is in ascii or binary format.\n");
      else 
	quit(-1,"Error : n-grams are not correctly ordered. Error occurred at ngram %d.\n",nlines);
    }
  }
  
  if (pon == N) {
    if (nlines > 3) 
      quit(-1,"Error - same n-gram appears twice in idngram stream.\n");
    else 
      show_idngram_corruption_mesg();
  }
  return pon;
}

/*
  Alright, two data structures, two data structures they are. 
 */

void ng_allocate_ptr_table(ng_t * ng, arpa_lm_t *arpa_ng, flag is_arpa )
{
  int i;
  if(is_arpa){
    assert(arpa_ng);
    arpa_ng->ptr_table = (ptr_tab_t **) rr_malloc(sizeof(ptr_tab_t *)*arpa_ng->n);
    arpa_ng->ptr_table_size = (ptr_tab_sz_t *) rr_calloc(arpa_ng->n,sizeof(ptr_tab_sz_t));
  
    for (i=0;i<=arpa_ng->n-1;i++)
      arpa_ng->ptr_table[i] = (ptr_tab_t *) rr_calloc(65535,sizeof(ptr_tab_t));
  }else{
    assert(ng);
    ng->ptr_table = (ptr_tab_t **) rr_malloc(sizeof(ptr_tab_t *)*ng->n);
    ng->ptr_table_size = (ptr_tab_sz_t *) rr_calloc(ng->n,sizeof(ptr_tab_sz_t));
  
    for (i=0;i<=ng->n-1;i++)
      ng->ptr_table[i] = (ptr_tab_t *) rr_calloc(65535,sizeof(ptr_tab_t));
  }

}

void ng_allocate_vocab_ht(ng_t *ng, /**< ng_t  with binary format stuffs */
			  arpa_lm_t *arpa_ng,  /**< arpa_lm_t */
			  flag is_arpa
			  )
{
  if(is_arpa){
    arpa_ng->vocab_ht = sih_create(1000,0.5,2.0,1);
    arpa_ng->vocab = (char **) rr_malloc(sizeof(char *)*
					     (arpa_ng->table_sizes[0]+1));
    arpa_ng->vocab_size = arpa_ng->table_sizes[0];
  }else{
    ng->vocab_ht = sih_create(1000,0.5,2.0,1);
    ng->vocab = (char **) rr_malloc(sizeof(char *)*
					     (ng->table_sizes[0]+1));
    ng->vocab_size = ng->table_sizes[0];
  }
}


void ng_arpa_lm_alloc_struct (arpa_lm_t *arpa_lm  /**< arpa_lm_t */
			      )
{
  int i;
  printf("Reading in a %d-gram language model.\n",arpa_lm->n);
  for (i=0;i<=arpa_lm->n-1;i++) {
    printf("Number of %d-grams = %d.\n",i+1,arpa_lm->table_sizes[i]);
    arpa_lm->num_kgrams[i]=arpa_lm->table_sizes[i];
  }

  /* Allocate memory */

  arpa_lm->word_id = (id__t **) rr_malloc(sizeof(id__t *) * arpa_lm->n);
  for (i=1;i<=arpa_lm->n-1;i++) /* Don't allocate for i = 0 */
    arpa_lm->word_id[i] = (id__t *) rr_malloc(sizeof(id__t) * 
					     arpa_lm->table_sizes[i]);

  arpa_lm->bo_weight = (bo_t **) rr_malloc(sizeof(bo_t *) * (arpa_lm->n-1));
  for (i=0;i<=arpa_lm->n-2;i++)
    arpa_lm->bo_weight[i] = (bo_t *) rr_malloc(sizeof(bo_t) * 
					     arpa_lm->table_sizes[i]);

  arpa_lm->ind = (index__t **) rr_malloc(sizeof(index__t *) * (arpa_lm->n-1));
  for (i=0;i<=arpa_lm->n-2;i++)
    arpa_lm->ind[i] = (index__t *) rr_malloc(sizeof(index__t) * 
					   arpa_lm->table_sizes[i]);

  arpa_lm->probs = (prob_t **) rr_malloc(sizeof(prob_t *) * arpa_lm->n);
  for (i=0;i<=arpa_lm->n-1;i++)
    arpa_lm->probs[i] = (prob_t *) rr_malloc(sizeof(prob_t) * 
					     arpa_lm->table_sizes[i]);

  ng_allocate_ptr_table(NULL,arpa_lm,1);
  ng_allocate_vocab_ht(NULL,arpa_lm,1);

}

double ng_double_alpha(ng_t *ng, int N, int i)
{
  if(ng->four_byte_alphas)
    return ng->bo_weight4[N][i];
  else{
    return double_alpha(ng->bo_weight[N][i],
			ng->alpha_array,
			ng->size_of_alpha_array,
			65535 - ng->out_of_range_alphas,
			ng->min_alpha,
			ng->max_alpha);
  }
}

void ng_short_alpha(ng_t *ng, double alpha, int N, int i)
{
  if (ng->four_byte_alphas) 
    ng->bo_weight4[N][i] = (four_byte_t)alpha;
  else {
    ng->bo_weight[N][i] = short_alpha(alpha,
				      ng->alpha_array,
				      &(ng->size_of_alpha_array),
				      65535 - ng->out_of_range_alphas,
				      ng->min_alpha,
				      ng->max_alpha);
  }
}

