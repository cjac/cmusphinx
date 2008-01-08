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
 * discounting_method.c - the wrapper of all discounting methods. 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2006 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: disc_meth.c,v $
 * Revision 1.3  2006/04/13 17:33:26  archan
 * 0, This particular change enable 32bit LM creation in ARPA format.  1, rationalized/messed up the data type, (Careful, with reading and writing for 8-byte data structure, they are not exactly working at this point.) 2, all switches in idngram2lm is changed to be implemented by the disc_meth object.
 *
 * Revision 1.2  2006/04/02 23:41:02  archan
 * Introduced 2 more methods in disc_meth object.
 *
 * Revision 1.1  2006/03/31 04:10:18  archan
 * Start to add a class for smoothing methods.  Only one method is implemented. In the Project L's case, we probably want to mix C-class and switch for a while.
 *
 *
 */
#include "disc_meth.h"

void init_ng_disc_method(ng_t *ng, /**< The ngram */
			 flag linear, /**< Is it linear discounting? */
			 flag absolute, /**< Is it abolute discounting? */
			 flag witten_bell, /**< Is it Witten Bell? */
			 flag good_turing  /**< Is it Good Turing? */
			 )
{
 ng->discounting_method = 0;
  
  if (linear) 
    ng->discounting_method = LINEAR;

  if (absolute) {
    if (ng->discounting_method != 0) 
      quit(-1,"Error : Assigned two contradictory discounting methods.\nSpecify one of -linear, -absolute, -good_turing or -witten_bell.\n");    
    ng->discounting_method = ABSOLUTE;
  }

  if (witten_bell) {
    if (ng->discounting_method != 0) 
      quit(-1,"Error : Assigned two contradictory discounting methods.\nSpecify one of  -linear, -absolute, -good_turing or -witten_bell.\n");    
    ng->discounting_method = WITTEN_BELL;
  }

  if (good_turing) {
    if (ng->discounting_method != 0) 
      quit(-1,"Error : Assigned two contradictory discounting methods.\nSpecify one of -linear, -absolute, -good_turing or -witten_bell.\n");    
    ng->discounting_method = GOOD_TURING;
  }

  if (ng->discounting_method == 0) 
    ng->discounting_method = GOOD_TURING;

  ng->disc_meth=(disc_meth_t*)disc_meth_init(ng->discounting_method);

}

disc_meth_t* disc_meth_init(int32 disc_meth)
{
  disc_meth_t *dm;
  dm=(disc_meth_t*) rr_malloc(1 * sizeof(disc_meth_t));
  dm->type=disc_meth;

  if(disc_meth==GOOD_TURING){

    dm->verbose_method=dump_disc_meth_good_turing_verbose_method;
    dm->dump_discounted_ngram_count=dump_good_turing_discounted_ngram_count;
    dm->allocate_freq_of_freq=disc_meth_good_turing_allocate_freq_of_freq;
    dm->update_freq_of_freq=disc_meth_good_turing_update_freq_of_freq;
    dm->reduce_ug_freq_of_freq=disc_meth_good_turing_reduce_ug_freq_of_freq;
    dm->compute_discount_aux=disc_meth_good_turing_compute_disc_aux;
    return dm;

  }else if(disc_meth==ABSOLUTE){

    dm->verbose_method=dump_disc_meth_absolute_verbose_method;
    dm->dump_discounted_ngram_count=dump_absolute_discounted_ngram_count;
    dm->allocate_freq_of_freq=disc_meth_absolute_allocate_freq_of_freq;
    dm->update_freq_of_freq=disc_meth_absolute_update_freq_of_freq;
    dm->reduce_ug_freq_of_freq=disc_meth_absolute_reduce_ug_freq_of_freq;
    dm->compute_discount_aux=disc_meth_absolute_compute_disc_aux;
    return dm;

  }else if(disc_meth==LINEAR){

    dm->verbose_method=dump_disc_meth_linear_verbose_method;
    dm->dump_discounted_ngram_count=dump_linear_discounted_ngram_count;
    dm->allocate_freq_of_freq=disc_meth_linear_allocate_freq_of_freq;
    dm->update_freq_of_freq=disc_meth_linear_update_freq_of_freq;
    dm->reduce_ug_freq_of_freq=disc_meth_linear_reduce_ug_freq_of_freq;
    dm->compute_discount_aux=disc_meth_linear_compute_disc_aux;
    return dm;

  }else if(disc_meth==WITTEN_BELL){

    dm->verbose_method=dump_disc_meth_witten_bell_verbose_method;
    dm->dump_discounted_ngram_count=dump_witten_bell_discounted_ngram_count;
    dm->allocate_freq_of_freq=disc_meth_witten_bell_allocate_freq_of_freq;
    dm->update_freq_of_freq=disc_meth_witten_bell_update_freq_of_freq;
    dm->reduce_ug_freq_of_freq=disc_meth_witten_bell_reduce_ug_freq_of_freq;
    dm->compute_discount_aux=disc_meth_witten_bell_compute_disc_aux;
    return dm;
  }

  return NULL;
}
