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


/* Return the probability of word, given a language
   model a context, and a forced backoff list */

#include <stdlib.h>
#include "evallm.h"
#include "idngram2lm.h"

double calc_prob_of(id__t sought_word,
		    id__t *context,
		    int context_length,
		    ng_t *ng,
		    arpa_lm_t *arpa_ng,
		    fb_info *fb_list,
		    int *bo_case,
		    int *acl,
		    flag arpa_lm) {

  int i;
  flag exc_back_off;
  int most_recent_fb;
  int actual_context_length;
  id__t *sought_ngram;
  double prob;

  exc_back_off = 0;

  if (arpa_lm) {
    if (sought_word == 0 && arpa_ng->vocab_type == CLOSED_VOCAB)
      quit(-1,"Error : Cannot generate probability for <UNK> since this is a closed \nvocabulary model.\n");
  }else {
    if (sought_word == 0 && ng->vocab_type == CLOSED_VOCAB)
      quit(-1,"Error : Cannot generate probability for <UNK> since this is a closed \nvocabulary model.\n");
  }

  most_recent_fb = -1;
  
  /* Find most recent word in the forced back-off list */
  
  for (i=context_length-1;i>=0;i--) {

    if (fb_list[context[i]].backed_off) {
      most_recent_fb = i;
      if (fb_list[context[i]].inclusive) 
	exc_back_off = 0;
      else 
	exc_back_off = 1;

      i = -2;
    }

  }
  
  actual_context_length = context_length - most_recent_fb -1;

  if (!exc_back_off && most_recent_fb != -1) {
    actual_context_length++;
  }

  sought_ngram = (id__t *) rr_malloc(sizeof(id__t)*(actual_context_length+1));

  for (i=0;i<=actual_context_length-1;i++) {
    if (exc_back_off) {
      sought_ngram[i] = context[i+most_recent_fb+1];
    }else {
      if (most_recent_fb == -1)
	sought_ngram[i] = context[i+most_recent_fb+1];
      else
	sought_ngram[i] = context[i+most_recent_fb];
    }
  }
  sought_ngram[actual_context_length] = sought_word;


  if (arpa_lm) {
    arpa_bo_ng_prob(actual_context_length,
		    sought_ngram,
		    arpa_ng,
		    2,       /* Verbosity */
		    &prob,
		    bo_case);
  }else {
    bo_ng_prob(actual_context_length,
	       sought_ngram,
	       ng,
	       2,       /* Verbosity */
	       &prob,
	       bo_case);
  }

  *acl = actual_context_length;

  free(sought_ngram);
  
  return(prob);

}

