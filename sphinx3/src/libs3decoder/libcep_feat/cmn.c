/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * cmn.c -- Various forms of cepstral mean normalization
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added cmn_free() and moved *mean and *var out global space and named them cmn_mean and cmn_var
 * 
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the name norm_mean() to cmn().
 * 
 * 19-Jun-1996	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed to compute CMN over ALL dimensions of cep instead of 1..12.
 * 
 * 04-Nov-1995	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include "cmn.h"

cmn_t* cmn_init()
{
  cmn_t *cmn;
  cmn=(cmn_t*)ckd_calloc (1, sizeof (cmn_t));
  cmn->cmn_mean=NULL;
  cmn->cmn_var=NULL;
  cmn->cur_mean=NULL;
  cmn->sum=NULL;
  cmn->nframe=0;
  return cmn;
}

void cmn (float32 **mfc, int32 varnorm, int32 n_frame, int32 veclen, cmn_t *cmn)
{
    float32 *mfcp;
    float32 t;
    int32 i, f;
    float32 *cmn_mean;
    float32 *cmn_var;

    assert(mfc!=NULL);
    cmn_mean=cmn->cmn_mean;
    cmn_var=cmn->cmn_var;

    /* assert ((n_frame > 0) && (veclen > 0)); */
    /* Added by PPK to prevent this assert from aborting Sphinx 3 */
    if ((n_frame <= 0) || (veclen <= 0)) {
        return;
    }
    
    if (cmn_mean == NULL)
	cmn_mean = (float32 *) ckd_calloc (veclen, sizeof (float32));

    /* Find mean cep vector for this utterance */
    for (i = 0; i < veclen; i++)
      cmn_mean[i] = 0.0;
    for (f = 0; f < n_frame; f++) {
	mfcp = mfc[f];
	for (i = 0; i < veclen; i++)
	    cmn_mean[i] += mfcp[i];
    }
    for (i = 0; i < veclen; i++)
	cmn_mean[i] /= n_frame;
    
    if (! varnorm) {
	/* Subtract mean from each cep vector */
	for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
		mfcp[i] -= cmn_mean[i];
	}
    } else {
	/* Scale cep vectors to have unit variance along each dimension, and subtract means */
        if (cmn_var == NULL)
    	    cmn_var = (float32 *) ckd_calloc (veclen, sizeof (float32));
	
        for (i = 0; i < veclen; i++)
	    cmn_var[i] = 0.0;
	
        for (f = 0; f < n_frame; f++) {
    	    mfcp = mfc[f];
	    
	    for (i = 0; i < veclen; i++) {
                t = mfcp[i] - cmn_mean[i];
                cmn_var[i] += t * t;
            }
        }
        for (i = 0; i < veclen; i++) 
	  cmn_var[i] = (float32) sqrt ((float64) n_frame / cmn_var[i]); /* Inverse Std. Dev, RAH added type case from sqrt */

        for (f = 0; f < n_frame; f++) {
	    mfcp = mfc[f];
	    for (i = 0; i < veclen; i++)
	        mfcp[i] = (mfcp[i] - cmn_mean[i]) * cmn_var[i];
        }
    }
}

#if 0
void cmn_prior(float32 **incep, int32 varnorm, int32 nfr, int32 ceplen, 
							   int32 endutt, cmn_t *cmn)
{
  float32 *cur_mean;
  float32 *sum;
  float32 sf;
  int32   i, j;
  
  cur_mean = cmn-> cur_mean;
  sum = cmn-> sum;

  assert(incep!=NULL);
  if (varnorm)
    E_FATAL("Variance normalization not implemented in live mode decode\n");
  

  if(cur_mean == NULL){
    cur_mean = (float32 *) ckd_calloc(ceplen, sizeof(float32));
    
    /* A front-end dependent magic number */
    cur_mean[0] = 12.0;

    if(sum == NULL)
    sum      = (float32 *) ckd_calloc(ceplen, sizeof(float32));
  }

  E_INFO("mean[0]= %.2f, mean[1..%d]= 0.0\n", cur_mean[0], ceplen-1);
  
  
  if (nfr <= 0)
    return;
  
  for (i = 0; i < nfr; i++){
    for (j = 0; j < ceplen; j++){
      sum[j] += incep[i][j];
      incep[i][j] -= cur_mean[j];
    }
    ++(cmn->nframe);
  }
  
  /* Shift buffer down if we have more than CMN_WIN_HWM frames */
  if (cmn->nframe > CMN_WIN_HWM) {
    sf = (float32) (1.0/cmn->nframe);
    for (i = 0; i < ceplen; i++)
      cur_mean[i] = sum[i] * sf;
    
    /* Make the accumulation decay exponentially */
    if (cmn->nframe >= CMN_WIN_HWM) {
      sf = CMN_WIN * sf;
      for (i = 0; i < ceplen; i++)
	sum[i] *= sf;
      cmn->nframe = CMN_WIN;
    }
  }
  
  if (endutt) {
    /* Update mean buffer */
    
    sf = (float32) (1.0/cmn->nframe);
    for (i = 0; i < ceplen; i++)
      cur_mean[i] = sum[i] * sf;
    
    /* Make the accumulation decay exponentially */
    if (cmn->nframe > CMN_WIN_HWM) {
      sf = CMN_WIN * sf;
      for (i = 0; i < ceplen; i++)
	sum[i] *= sf;
      cmn->nframe = CMN_WIN;
    }
    
  }
/*  E_INFO("Hihi\n");*/
}
#endif
/* 
 * RAH, free previously allocated memory
 */
void cmn_free (cmn_t *cmn)
{
  if(cmn->cmn_var)
  ckd_free ((void *) cmn->cmn_var);
  if(cmn->cmn_mean)
  ckd_free ((void *) cmn->cmn_mean);
}
