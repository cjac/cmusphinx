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
#include <stdlib.h>

void validate(ng_t *ng,
	      arpa_lm_t *arpa_ng,
	      char **words,
	      flag backoff_from_unk_inc,
	      flag backoff_from_unk_exc,
	      flag backoff_from_ccs_inc,
	      flag backoff_from_ccs_exc,
	      flag arpa_lm,
	      char *fb_list_filename) 
{
  vocab_sz_t *context;
  id__t *short_context;
  int dummy1;
  int dummy2;
  int i, beginidx, endidx;
  fb_info *fb_list;
  double prob_so_far;
  flag found_unk_wrongly;
  int n;

  if (arpa_lm)
    n = arpa_ng->n;
  else
    n = ng->n;

  if (arpa_lm) {
    fb_list = gen_fb_list(arpa_ng->vocab_ht,
			  arpa_ng->vocab_size,
			  arpa_ng->vocab,
			  arpa_ng->context_cue,
			  backoff_from_unk_inc,
			  backoff_from_unk_exc,
			  backoff_from_ccs_inc,
			  backoff_from_ccs_exc,
			  fb_list_filename);
  }else {
    fb_list = gen_fb_list(ng->vocab_ht,
			  ng->vocab_size,
			  ng->vocab,
			  ng->context_cue,
			  backoff_from_unk_inc,
			  backoff_from_unk_exc,
			  backoff_from_ccs_inc,
			  backoff_from_ccs_exc,
			  fb_list_filename);
  }
  
  context = (vocab_sz_t *) rr_malloc(sizeof(vocab_sz_t)*(n-1));
  short_context = (id__t *) rr_malloc(sizeof(id__t)*(n-1));
  
  found_unk_wrongly = 0;

  for (i=0;i<=n-2;i++) {
    if (arpa_lm) {
      if (sih_lookup(arpa_ng->vocab_ht,words[i],&context[i]) == 0) {
	if (arpa_ng->vocab_type == CLOSED_VOCAB) {
	  fprintf(stderr,"Error : %s is not in the vocabulary, and this is a closed \nvocabulary model.\n",words[i]);
	  found_unk_wrongly = 1;
	}else
	  fprintf(stderr,"Warning : %s is an unknown word.\n",words[i]);
      }
      if (context[i] > 65535)
	quit(-1,"Error : returned value from sih_lookup is too high.\n");
      else 
	short_context[i] = context[i];

    }else {
      if (sih_lookup(ng->vocab_ht,words[i],&context[i]) == 0) {
	if (ng->vocab_type == CLOSED_VOCAB) {
	  fprintf(stderr,"Error : %s is not in the vocabulary, and this is a closed \nvocabulary model.\n",words[i]);
	  found_unk_wrongly = 1;
	}else
	  fprintf(stderr,"Warning : %s is an unknown word.\n",words[i]);
      }
      if (context[i] > 65535)
	quit(-1,"Error : returned value from sih_lookup is too high.\n");
      else
	short_context[i] = context[i];
    }
  }

  /* Map down from context array to short_context array */
  /* sih_lookup requires the array to be ints, but prob_so_far
     requires short ints. */

  if (!found_unk_wrongly) {

    prob_so_far = 0.0;
    
    if (arpa_lm) {
      beginidx=arpa_ng->first_id;
      endidx=arpa_ng->vocab_size;
    }else {
      beginidx=ng->first_id;
      endidx=ng->vocab_size;
    }
    
    for (i=beginidx;i<=endidx;i++) {
      prob_so_far += calc_prob_of(i,
				  short_context,
				  n-1,
				  ng,
				  arpa_ng,
				  fb_list,
				  &dummy1,
				  &dummy2,
				  arpa_lm);
    }

    printf("Sum of P( * | ");
    for (i=0;i<=n-2;i++)
      printf("%s ",words[i]);

    printf(") = %f\n",prob_so_far);
    
  }

  free(context);
  free(fb_list);

}
