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

/* Basically copied from version 1. 
   ARCHAN 20060331: Oh my gosh.
*/

#include "general.h"   // from libs
#include "ngram.h"
#include "pc_general.h"   // from libs

void compute_gt_discount(int n,
			 fof_t *freq_of_freq,
			 fof_sz_t fof_size,
			 unsigned short *disc_range,
			 int cutoff,
			 int verbosity,
			 disc_val_t     **discounted_values) {


  /* Lots of this is lifted straight from V.1 */

  flag done;
  int r;
  int K;
  double common_term;
  double first_term;
  double *D;

  D = (double *) rr_calloc((*disc_range)+1,sizeof(double));
  *discounted_values = D; 

  /* Trap standard things (taken from V.1) */

  if (fof_size == 0) 
    return;

  if (freq_of_freq[1] == 0) {
    pc_message(verbosity,2,"Warning : %d-gram : f-of-f[1] = 0 --> %d-gram discounting is disabled.\n",n,n);
    *disc_range=0;
    return;
  }

  if (*disc_range + 1 > fof_size) {
    pc_message(verbosity,2,"Warning : %d-gram : max. recorded f-o-f is only %d\n",n,fof_size);
    pc_message(verbosity,2,"%d-gram discounting range is reset to %d.\n",fof_size,n,fof_size-1);
    *disc_range = fof_size-1;
  }

  done = 0;

  while (!done) {
    if (*disc_range == 0) {
      pc_message(verbosity,2,"Warning : %d-gram : Discounting is disabled.\n",n);
      return;
    }

    if (*disc_range == 1) {
      /* special treatment for 1gram if there is a zeroton count: */
      if ((n==1) && freq_of_freq[0]>0) {
	D[1] = freq_of_freq[1] / ((float) (freq_of_freq[1] + freq_of_freq[0]));
	pc_message(verbosity,2,"Warning : %d-gram : Discounting range is 1; setting P(zeroton)=P(singleton).\nDiscounted value : %.2f\n",n,D[1]);
	return;
      }else 
	pc_message(verbosity,2,"Warning : %d-gram : Discounting range of 1 is equivalent to excluding \nsingletons.\n",n);

    }

    K = *disc_range;
    common_term = ((double) (K+1) * freq_of_freq[K+1]) / freq_of_freq[1];

    if (common_term<=0.0 || common_term>=1.0) {
      pc_message(verbosity,2,"Warning : %d-gram : GT statistics are out of range; lowering cutoff to %d.\n",n,K-1);
      (*disc_range)--;
    }else {
      for (r=1;r<=K;r++) {
	first_term = ((double) ((r+1) * freq_of_freq[r+1]))
		             /  (r    * freq_of_freq[r]);
	D[r]=(first_term - common_term)/(1.0 - common_term);
      }
      pc_message(verbosity,3,"%d-gram : cutoff = %d, discounted values:",n,K);
      for (r=1;r<=K;r++) 
	pc_message(verbosity,3," %.2f",D[r]);

      pc_message(verbosity,3,"\n");
      done = 1;
      for (r=1; r<=K; r++) {
	if (D[r]<0 || D[r]>1.0) {
	  pc_message(verbosity,2,"Warning : %d-gram : Some discount values are out of range;\nlowering discounting range to %d.\n",n,K-1);
	  (*disc_range)--;
	  r=K+1;
	  done = 0;
	}
      }
    }	      
  }

   for (r=1; r<=MIN(cutoff,K); r++) D[r] = 0.0;

}

