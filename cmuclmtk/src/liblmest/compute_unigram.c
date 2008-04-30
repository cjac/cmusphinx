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

/* Calculate the probabilities for each 1-gram */

/* Basically copied from version 1 */


/* From comments to v1: 

A) open_vocab == 0: 

  Closed vocab model.
  P(UNK)=0.  UNK is not part of the vocab.
  The discount mass is divided equally among all zerotons.  If this
results in P(zeroton)>Q*P(singleton) for some appropriate fraction Q,
P(zeroton) is reduced to Q*P(singleton) and the entire array is then
renormalized.

B) open vocab == 1: 

  Open vocab model, where the vocab was chosen without knowing the
partition of the data into training and testing, so unicount[UNK]/N is
probably a reasonable estimate of P(UNK).  So:
  Treat UNK as any other word.
  As in (A), discount mass is divided among all zerotons.  If some
is left over (b/c/o the constraint P(zeroton)<=Q*P(singleton)),
renormalizing everything to absorb it.

C) open vocab == 2: 

  Open vocab model, where the vocab was defined to include all the
training data, hence unicount[UNK]=0.  So: 
  The discount mass is split: one part (1-OOV_fraction) is divided
among the zerotons, as above.  The other part, plus any leftover from
the first part, is put into P(UNK).


note: UNK is hardwired to id=0, here and elsewhere.
*/


#include <math.h>
#include <stdio.h>
#include "ngram.h"
#include "idngram2lm.h"
#include "pc_general.h"   // from libs

void compute_unigram(ng_t *ng,int verbosity) {

  count_t i;
  count_t count;
  count_t n_zerotons;

  int num_of_types;
  double floatN;
  double prob;
  double total_prob;
  double discount_mass;
  double total_zeroton_mass;
  double prob_zeroton;
  double prob_singleton;
  double leftover_mass;

  /* Make sure that we don't have a type 2 vocab and an UNK */

  if (ng->vocab_type==OPEN_VOCAB_2 && return_count(ng->four_byte_counts,
						   ng->count_table[0],
						   ng->count[0],
						   ng->count4[0],
						   0) != 0) {
    quit(-1,"Error : Open vocabulary type 2 requested, but there were OOVs in the \ntraining data.\n");
  }

  if (ng->vocab_type == CLOSED_VOCAB) 
    ng->uni_probs[0] = BAD_PROB;

  /* Make sure all context cues have a zero count */

  if (ng->no_of_ccs > 0) {
    for (i=ng->first_id;i<=ng->vocab_size;i++) {
      if (ng->context_cue[i] && return_count(ng->four_byte_counts,
					     ng->count_table[0],
					     ng->count[0],
					     ng->count4[0],
					     i) != 0) {
	quit(-1,"Error : Context cue word has a non zero count.\n");
      }
    }
  }

  /* Compute the discounted unigram, and the total */

  floatN = (double) ng->n_unigrams;

  total_prob = 0.0;

  num_of_types = 0;

  for (i=ng->first_id;i<=ng->vocab_size;i++) {
    if (return_count(ng->four_byte_counts,
		     ng->count_table[0],
		     ng->count[0],
		     ng->count4[0],
		     i) > 0) {
      num_of_types++;
    }
  }


  for (i=ng->first_id;i<=ng->vocab_size;i++) {
    
    count = return_count(ng->four_byte_counts,
			 ng->count_table[0],
			 ng->count[0],
			 ng->count4[0],
			 i);
    prob = count/floatN;
    switch (ng->discounting_method) {
    case GOOD_TURING:
      if (count > 0 && count <= ng->disc_range[0])
	prob *= ng->gt_disc_ratio[0][count];
      else {
	if (count == 0)
	  prob = BAD_PROB;	
      }
      break;
    case LINEAR:
      if (count > 0)
	prob *= ng->lin_disc_ratio[0];
      else 
	prob = BAD_PROB;
      break;
    case ABSOLUTE:
      if (count > 0)
	prob *= (count - ng->abs_disc_const[0])/count;
      else 
	prob = BAD_PROB;
      break;
    case WITTEN_BELL:
      if (count > 0)
	prob *= floatN/(floatN+num_of_types);
      else
	prob = BAD_PROB;
      break;
    }
    pc_message(verbosity,4,"   prob[%d] = %.8g count = %d \n",i,prob,count);
    ng->uni_probs[i] = prob;
    total_prob += prob;
  }

  /* Compute the discount mass */

  discount_mass = 1.0 - total_prob;

  /*  if(ng->discounting_method!=WITTEN_BELL)*/

    pc_message(verbosity,2,"Unigrams's discount mass is %g (n1/N = %g)\n",
	     discount_mass,ng->freq_of_freq[0][1]/floatN);
 

  if (discount_mass < 1e-10 && discount_mass != 0.0) {
    discount_mass = 0.0;
    pc_message(verbosity,2,"Discount mass was rounded to zero.\n");
  }
  
  /* Compute P(zeroton) & assign it to all zerotons (except context
     cues) */

  leftover_mass = discount_mass;
  n_zerotons = ng->freq_of_freq[0][0] - ng->no_of_ccs;

  if ((n_zerotons > 0) && (discount_mass > 0.0)) {
    total_zeroton_mass = discount_mass;
    if (ng->vocab_type == OPEN_VOCAB_2) {
      total_zeroton_mass = (1.0 - ng->oov_fraction)*discount_mass;
    }
    prob_zeroton = total_zeroton_mass / n_zerotons;
    prob_singleton = 1 / floatN;
    switch (ng->discounting_method) {
    case GOOD_TURING:
      if (ng->disc_range[0] >= 1)
	prob_singleton *= ng->gt_disc_ratio[0][1];
      break;
    case LINEAR:
      prob_singleton *= ng->lin_disc_ratio[0];
      break;
    case ABSOLUTE:
      prob_singleton *= (1-ng->abs_disc_const[0]);
      break;
    case WITTEN_BELL:
      prob_singleton *= floatN/(floatN + num_of_types);
      break;
    }
    pc_message(verbosity,2,"%d zerotons, P(zeroton) = %g P(singleton) = %g\n",
	       n_zerotons,prob_zeroton,prob_singleton);
    if (prob_zeroton > ng->zeroton_fraction*prob_singleton) {
      prob_zeroton = ng->zeroton_fraction*prob_singleton;
      pc_message(verbosity,1,"P(zeroton) was reduced to %.10f (%.3f of P(singleton))\n",prob_zeroton,ng->zeroton_fraction);
    }

    for (i=ng->first_id;i<=ng->vocab_size;i++) {
      if ((return_count(ng->four_byte_counts,
			ng->count_table[0],
			ng->count[0],
			ng->count4[0],
			i) == 0) && (!ng->context_cue[i])) {
	ng->uni_probs[i] = prob_zeroton;
      }
    }

    total_zeroton_mass = n_zerotons * prob_zeroton;
    leftover_mass = discount_mass - total_zeroton_mass;
  }

  /* Do renormalisation due to UNK */
 
  if (ng->vocab_type == OPEN_VOCAB_2) {
    ng->uni_probs[0] += leftover_mass;
    if (ng->uni_probs[0] <= 0.0)
      ng->uni_probs[0] = BAD_PROB;
  }else {
    if (fabs(leftover_mass) > 1e-10) {
      for (i=ng->first_id;i<=ng->vocab_size;i++)
	ng->uni_probs[i] /= (1.0 - leftover_mass);
      
      if (fabs(leftover_mass)>1e-8)
	pc_message(verbosity,1,"Unigram was renormalized to absorb a mass of %g\n",leftover_mass);
    }
  }
  pc_message(verbosity,1,"prob[UNK] = %g\n",ng->uni_probs[0]);
  if ((n_zerotons>0) && (discount_mass<=0.0)) 
    pc_message(verbosity,1,"WARNING: %d non-context-cue words have zero probability\n\n",n_zerotons);

  if (verbosity>=4) {
    fprintf(stderr,"THE FINAL UNIGRAM:\n");
    for (i=ng->first_id;i<=ng->vocab_size;i++)
      fprintf(stderr," unigram[%d]=%g\n",i,ng->uni_probs[i]);
  }

  /* Test resulting unigram for consistency */

  total_prob = 0.0;
  for (i=ng->first_id;i<=ng->vocab_size;i++)
    total_prob += ng->uni_probs[i];

  if (fabs(1.0-total_prob) > 1e-6)
    quit(-1,"ERROR: sum[P(w)] = %.10f\n",total_prob);

  if (fabs(1.0-total_prob) > 1e-9)
    pc_message(verbosity,1,"WARNING: sum[P(w)] = %.10f\n\n",total_prob);

  /* Precompute logprobs */
  for (i=ng->first_id;i<=ng->vocab_size;i++)
    ng->uni_log_probs[i] = log(ng->uni_probs[i]);

}
