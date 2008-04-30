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
 * disc_meth_absolute.c - implement methods of good turing
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
 * $Log: disc_meth_absolute.c,v $
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

#include "pc_general.h"  // from libs
#include "disc_meth_absolute.h"
#include "idngram2lm.h"

void dump_disc_meth_absolute_info(ng_t* ng)
{
}

void dump_disc_meth_absolute_verbose_method(ng_t* ng, unsigned short verbosity)
{
  pc_message(verbosity,2,"Absolute\n");
}


double dump_absolute_discounted_ngram_count(ng_t* ng, 
					    int N,
					    count_t ngcount,
					    count_t margcount,
					    int* current_pos
					    )
{
  double discounted_ngcount;

  discounted_ngcount =  ngcount - ng->abs_disc_const[N];

  return discounted_ngcount;
}


void disc_meth_absolute_allocate_freq_of_freq(ng_t *ng)
{
  int i;
  for (i=0;i<=ng->n-1;i++)
    ng->freq_of_freq[i] = (int *) rr_calloc(3,sizeof(int));

  ng->abs_disc_const = (double *) rr_malloc(sizeof(double)*ng->n);

}

void disc_meth_absolute_update_freq_of_freq(ng_t *ng, int N, count_t countno)
{
  /* Only update fof if count not is small than or equal to zero */
  if (countno <= 2) {
    if (countno <= 0) 
      show_idngram_corruption_mesg();

    ng->freq_of_freq[N][countno]++;
  }
}

void disc_meth_absolute_reduce_ug_freq_of_freq(ng_t *ng, int i)
{
  if (ng->count[0][i] <= 2) 
    ng->freq_of_freq[0][ng->count[0][i]]--;

}


void disc_meth_absolute_compute_disc_aux(ng_t *ng, int verbosity)
{
  int i,j;
  pc_message(verbosity,1,"Absolute discounting ratios :\n");
  for (i=0;i<=ng->n-1;i++) {
    ng->abs_disc_const[i] = ((float) ng->freq_of_freq[i][1] ) /
      ((float) ng->freq_of_freq[i][1] + (2*ng->freq_of_freq[i][2]) );
    pc_message(verbosity,1,"%d-gram : ",i+1);
    
    for (j=1;j<=5;j++) 
      pc_message(verbosity,1,"%g ",(j-ng->abs_disc_const[i])/j);
    
    pc_message(verbosity,1," ... \n");
  }
}
