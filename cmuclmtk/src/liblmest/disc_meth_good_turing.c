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
 * disc_meth_good_turing.c - implement methods of good turing
 * discounting. 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2006 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: disc_meth_good_turing.c,v $
 * Revision 1.3  2006/04/13 17:33:26  archan
 * 0, This particular change enable 32bit LM creation in ARPA format.  1, rationalized/messed up the data type, (Careful, with reading and writing for 8-byte data structure, they are not exactly working at this point.) 2, all switches in idngram2lm is changed to be implemented by the disc_meth object.
 *
 * Revision 1.2  2006/04/02 23:53:39  archan
 * Implemented all methods on different discounting.
 *
 * Revision 1.1  2006/03/31 04:10:18  archan
 * Start to add a class for smoothing methods.  Only one method is implemented. In the Project L's case, we probably want to mix C-class and switch for a while.
 *
 */

#include "pc_general.h"   // from libs
#include "disc_meth_good_turing.h"
#include "idngram2lm.h"

void dump_disc_meth_good_turing_info(ng_t* ng)
{

}

void dump_disc_meth_good_turing_verbose_method(ng_t* ng, unsigned short verbosity)
{
  int i;
  pc_message(verbosity,2,"Good-Turing\n");
  pc_message(verbosity,2,"     Discounting ranges :\n        ");
  for (i=0;i<=ng->n-1;i++) 
    pc_message(verbosity,2,"%d-gram : %d     ",i+1,ng->disc_range[i]);
  
  pc_message(verbosity,2,"\n");

}

double dump_good_turing_discounted_ngram_count(ng_t* ng, 
					       int N,      /**< Which N in the N-gram */
					       count_t ngcount, /**< The ngram count.  Used in every type of discountings */
					       count_t margcount,
					       int* current_pos /**< Used in Witten-Bell */
					       )
{
  double discounted_ngcount;
  if (ngcount <= ng->disc_range[N])
    discounted_ngcount = ng->gt_disc_ratio[N][ngcount] * ngcount;
  else
    discounted_ngcount = ngcount;
  return discounted_ngcount;
}

void disc_meth_good_turing_allocate_freq_of_freq(ng_t *ng)
{
  int i;
  for (i=0;i<=ng->n-1;i++) 
    ng->freq_of_freq[i] = (int *) rr_calloc(ng->fof_size[i]+1,sizeof(int));
}


void disc_meth_good_turing_update_freq_of_freq(ng_t *ng, int N, count_t countno)
{
  if(countno <= ng->fof_size[N]){
    if (countno <= 0) 
      show_idngram_corruption_mesg();

    ng->freq_of_freq[N][countno]++;
  }

}

void disc_meth_good_turing_reduce_ug_freq_of_freq(ng_t *ng, int i)
{
  if (ng->count[0][i] <= ng->fof_size[0]) 
    ng->freq_of_freq[0][ng->count[0][i]]--;
}

/*
  Mainly compute good turing discounting ratios 
 */
void disc_meth_good_turing_compute_disc_aux(ng_t *ng,int verbosity)
{
  int i;
  ng->gt_disc_ratio = (disc_val_t **) rr_malloc(sizeof(disc_val_t *)*ng->n);
    
  for (i=0;i<=ng->n-1;i++) 
    ng->gt_disc_ratio[i] = (disc_val_t *) rr_malloc(sizeof(disc_val_t)*ng->fof_size[i]);
  
  for (i=0;i<=ng->n-1;i++) {
    /* ARCHAN i==0? 0: ng->cutoffs[i-1] tells the formula */
    compute_gt_discount(i+1,
			ng->freq_of_freq[i],
			ng->fof_size[i],
			&ng->disc_range[i],
			i==0 ? 0 : ng->cutoffs[i-1],
			verbosity,
			&ng->gt_disc_ratio[i]);
  }
}


