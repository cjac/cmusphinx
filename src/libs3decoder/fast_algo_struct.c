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
 * fast_algo_struct.c -- Various forms of pruning beam
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 09-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "fast_algo_struct.h"
#include "logs3.h"
#include "s3types.h"


beam_t *beam_init (float64 hmm, float64 ptr, float64 wd, float64 wdend, int32 ptranskip)
{
    beam_t *beam;
    
    beam = (beam_t *) ckd_calloc (1, sizeof(beam_t));
    
    beam->hmm = logs3 (hmm);
    beam->ptrans = logs3 (ptr);
    beam->word = logs3 (wd);
    beam->wordend =logs3 (wdend);

    beam->ptranskip=ptranskip;

    return beam;
}

histprune_t *histprune_init (int32 maxhmm,int32 maxhist, int32 maxword)
{
  histprune_t *hp;
  hp = (histprune_t *) ckd_calloc (1, sizeof(histprune_t));
  hp->maxwpf=maxword;
  hp->maxhmmpf=maxhmm;
  hp->maxhistpf=maxhist;
  return hp;
}

fast_gmm_t *fast_gmm_init (int32 down_sampling_ratio, 
			   int32 mode_cond_ds,
			   int32 mode_dist_ds,
			   int32 isGS4GS,
			   int32 isSVQ4SVQ,
			   float32 subvqbeam,
			   float32 cipbeam,
			   int32 maxcd,
			   int32 n_ci_sen)
{
  fast_gmm_t *fg;

  fg = (fast_gmm_t *) ckd_calloc (1, sizeof(fast_gmm_t));

  fg->rec_bst_senscr=0;
  fg->last_feat=NULL;

  fg->gs4gs=isGS4GS;
  fg->svq4svq=isSVQ4SVQ;
  fg->downs = (downsampling_t *) ckd_calloc(1,sizeof(downsampling_t));
  fg->gmms = (gmm_select_t*) ckd_calloc(1,sizeof(gmm_select_t));
  fg->gaus = (gau_select_t*) ckd_calloc(1,sizeof(gau_select_t));

  fg->gmms->ci_pbeam=logs3(cipbeam);
  if(fg->gmms->ci_pbeam < -10000000)
    E_INFO("Virtually no CI phone beam is applied now. (ci_pbeam <-1000000)\n");
  fg->gmms->ci_occu= (int32*) ckd_calloc(n_ci_sen,sizeof(int32));
  fg->gmms->idx=     (int32*) ckd_calloc(n_ci_sen,sizeof(int32));
  fg->gmms->max_cd = maxcd;

  E_INFO("max cd %d\n",maxcd);
  fg->gaus->rec_bstcid=-1;

  fg->gaus->subvqbeam=logs3(subvqbeam);

  fg->downs->ds_ratio=down_sampling_ratio;
  fg->downs->cond_ds=mode_cond_ds;
  fg->downs->dist_ds=mode_dist_ds;
  fg->downs->skip_count=0;
  
  if(fg->downs->cond_ds && fg->downs->dist_ds)
    E_FATAL("-cond_ds and -dist_ds cannot be specified together\n");



  return fg;
}

