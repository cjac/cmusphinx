/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * approx_cont_mgau.c
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2003 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 23-Jan-2004 Arthur Chan (archan@cs.cmu.edu)
 *             started
 */

#include "approx_cont_mgau.h"
#include "s3types.h"
#include "gs.h"
#include "mdef.h"

int32 most_recent_best_cid=-1;



#define DEBUG_GSCORE 1


int32 approx_isskip(int32 frame,int32 ds_ratio,int32 cond_ds,int32 isSameBestIdx,int32 *skip_count)
{
  /*Consider cond_ds first if specified*/
  if(cond_ds>0) {
      if(isSameBestIdx){
	if(*skip_count<ds_ratio-1){
	  *skip_count++;
	  return 1;
	}else{
	  *skip_count=0;
	  return 0;
	}
      }
      else
	return 0;
    }

  /*Consider the effect of ds_ratio*/
  if(frame%ds_ratio==0)
    return 0;
  else
    return 1;
}




/*Update the senone score given index, return the number of gaussians compute, 
  This took care of Gaussian level of optimization. This will called Feature Level Optimization routine 
   Gaussian Level:
   ^^^^^^^^^^^^^^^
   Shortlist of Gaussians was determined using Gaussian-Selection (Bochierri 93) in gs_mgau_shortlist or Sub-VQ-based Gaussian Selection (Ravi 98) in subvq_mgau_shortlist. Note that the term "shortlist" was also used in (P. Douglas 99) which is basically are variant of (Bochierri 93) with a clever scheme which resolves the back-off problem. 
   We have plans to further enhance schemes of Gaussian Selection by combining them using machine learning techniques. 
   Feature Component Level:
   ^^^^^^^^^^^^^^^^^^^^^^^^
   SVQ is used for feature level optimization only if svq4svq is set to 1.  This use the sum of sub-vector scores will be used as the gaussian scores. 

   Safe Guarding abnomal scores:
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   We discover that even using our Gaussian Selection routine, the code can break because of some gaussian score is extremely low, we tried to detect them and avoid them by 
   
*/
int32 approx_mgau_eval (gs_t* gs,
			subvq_t* svq,
			mgau_model_t* g,
			kb_t *kb,
			int32 s, /*senone index*/
			int32 *senscr,
			float32 *feat,
			int32 best_cid,
			int32 svq_beam
			) 
{
  int32 ng=0;
  int32 *mgau_sl;

  if(gs&&kb->gs4gs){
    ng = gs_mgau_shortlist (gs, s, mgau_n_comp(g,s),feat,best_cid);
    mgau_sl=gs->mgau_sl;
  }else if (svq){
    ng = subvq_mgau_shortlist (svq, s, mgau_n_comp(g,s), svq_beam);
    mgau_sl=svq->mgau_sl;
  }else{
    ng = mgau_n_comp (g, s);
    mgau_sl=NULL;
  }

  /*  
  E_INFO("Full computation: Idx %d using subvq, Senscr %d ng %d\n",s,senscr[s],ng);
  senscr[s] = mgau_eval (g, s, NULL, feat);
  E_INFO("Full computationIdx %d using normal, Senscr %d ng %d\n",s,senscr[s],ng);
  senscr[s] = subvq_mgau_eval(g, svq, s, mgau_n_comp(g,s),mgau_sl);
  E_INFO("Partial Computation: Idx %d using subvq, Senscr %d ng %d\n",s,senscr[s],ng);
  senscr[s] = mgau_eval (g, s, mgau_sl, feat);
  E_INFO("Partial Computation: Idx %d using normal, Senscr %d ng %d\n",s,senscr[s],ng);*/

  /* This safe guard the algorithmic error of other 3rd party converter*/
  if(ng==0){
    /* Not dumping any information because it costs a lot of space*/
    /* E_INFO("short list has 0 element, turn to compute all, please check the Gaussian Selection algorithm\n");*/
    mgau_sl=NULL;
    ng=mgau_n_comp(g,s);
  }

  if(svq&&kb->svq4svq)
    senscr[s] = subvq_mgau_eval(g, svq, s, mgau_n_comp(g,s),mgau_sl);
  else
    senscr[s] = mgau_eval (g, s, mgau_sl, feat);


  /*This routine safe guard the computation such that no abnomality will occur */
  if(senscr[s] < S3_LOGPROB_ZERO+100000){ /* a range of value which recomputation is necessary */
    if(mgau_sl==NULL){
      /*E_INFO("WARNING!! Score is S3_LOGPROB_ZERO even computing full gaussians! %d\n",s);*/
    }
    else{
      mgau_sl=NULL;
      ng+=mgau_n_comp(g,s);
      if(svq&&kb->svq4svq)
	senscr[s] = subvq_mgau_eval(g, svq, s, mgau_n_comp(g,s),NULL);
      else
	senscr[s] = mgau_eval (g, s, NULL, feat);

    }
  }



  return ng;
}


/* This function,
  1, It only compute the ci-phones score.
  2, There is no optimization schemes applied to this routine.
  3, The score is not normalize, this routine is supposed to be used before approx_cont_mgau_frame_eval,
     The best score is determined by the later function. 
*/

void approx_cont_mgau_ci_eval (mgau_model_t *g,float32 *feat,int32 *ci_senscr, kb_t *kb)
{
  int32 s;
  mdef_t* mdef;
  s3senid_t *cd2cisen;

  mdef=kb->kbcore->mdef;
  cd2cisen=mdef_cd2cisen(mdef);

  for (s = 0; s==cd2cisen[s]; s++) {
    ci_senscr[s] = mgau_eval (g, s, NULL, feat);
    /*    E_INFO("Inside ci_eval At senone %d, CI phoneme score %d \n",s,ci_senscr[s]);*/
  }
}

/* approx_con_mgau_frame_eval encapsulates all approximations in the Gaussian computation.
   This assumes programmers NOT to initialize the senone scores at every frame (FIX me!) before
   using this function. 
   This layer of code controls the optimization performance in Frame Leval and GMM Level. 
   Frame Level:
   ^^^^^^^^^^^^
   We select to compute the scores only if it is not similar to the most recently computed frames.  There are multiple ways to configures this. 
   Naive down-sampling : Skip the computation one every other n-frames
   Conditional down-sampling : Skip the computation only if the current frame doesn't belong to the same neighborhood of the same frame.  This neighborhood corresponds to the codeword which the feature vector found to be the closest. 
   
   No matter which down-sampling was used, the following problem will appear in the computation.  Active senones of frame which supposed to be skipped in computation were not computed in the most recently computed frame.  In those case, we chose to compute those senones 

   GMM Level:
   ^^^^^^^^^^
   In the implementation of CI-based GMM selection makes use of the fact that in s3.3 , CI models are always placed before all CD models. Hence the following logic is implemented:
   if(it is CI senone)
      compute score
   else if (it is CD senone)
      if the ci-phone beam was not set 
          compute score
      else
          if the CD senone's parent has a score within the beam
	     compute_score
	  else CD senone's parent has a score out of the beam
	     back-off using the parent senone score. 

   About renormalization
   ^^^^^^^^^^^^^^^^^^^^^
   Sphinx 3.4 generally renormalize the score using the best score. Notice that this introduce extra complication to the implementation.  I have separated the logic of computing or not computing the scores.  This will clarify the code a bit.  
   
   Accounting of senone and gaussian computation
   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   This function assumes approx_cont_mgau_ci_eval was run before it, hence at the end the score was added on top of the it. 
*/


int32 approx_cont_mgau_frame_eval (mgau_model_t *g,
				   gs_t* gs, 
				   subvq_t *svq,
				   int32 svq_beam,
				   float32 *feat,	
				   int32 *sen_active,  
				   int32 *senscr,
				   int32 *cache_ci_senscr,
				   kb_t *kb,
				   int32 frame)
{
  int32 s;
  int32 t;
  int32 best, ns, ng;
  int32 best_cid;
  int32 is_skip;
  int32 is_compute;
  int32 pbest;
  int32 is_ciphone;
  mdef_t* mdef;
  s3senid_t *cd2cisen;

  best = MAX_NEG_INT32;
  pbest = MAX_NEG_INT32;
  ns = 0;
  ng = 0;
  best_cid=-1;

  /*if(gs && svq) E_FATAL("Currently, there is no combination scheme of gs and svq\n");*/

  ptmr_start(&(kb->tm_ovrhd));
  if(gs)  best_cid=gc_compute_closest_cw(gs,feat);
  if(svq) subvq_gautbl_eval_logs3 (svq, feat);
  ptmr_stop(&(kb->tm_ovrhd));

  is_skip=approx_isskip(frame,kb->ds_ratio,kb->cond_ds,kb->rec_bstcid==best_cid,&(kb->skip_count));
  kb->rec_bstcid=best_cid; /*Well, if it is not skip, you need to update, if it is skip, the update will give you the same thing , why bother to check then?*/

  mdef=kb->kbcore->mdef;
  cd2cisen=mdef_cd2cisen(mdef);

  /*  E_INFO("At frame %d, is_skip? %d best_cid %d skip count %d isSameBestIdx? %d\n",frame,is_skip,best_cid,kb->skip_count,kb->rec_bstcid==best_cid);*/

  /* Use the original */
  for (s = 0; s < g->n_mgau; s++) {
    is_compute = !sen_active || sen_active[s];
    is_ciphone  =  (s==cd2cisen[s]);

#if _DEBUG_GSCORE_
    E_INFO("Sen active %d, kb->rec_sen_active %d, sen id %d \n",sen_active[s],kb->rec_sen_active[s],s);
#endif

    if(!is_skip){ /* Loop handling main computation*/
      /* Compute the score of the CI phone even if it is not active. */
      if(is_ciphone){

	/*Just copied from the cache, we just do accouting here */

	/*ng+=approx_mgau_eval (gs,svq,g,kb,s,senscr,feat,best_cid,svq_beam);*/

	/*E_INFO("At senone %d, CI phoneme score %d \n",s,cache_ci_senscr[s]);*/
	senscr[s]=cache_ci_senscr[s];
	if (pbest < senscr[s]) pbest = senscr[s];
	if (best < senscr[s]) best = senscr[s];
	sen_active[s]=1;
	ng+=mgau_n_comp(g,s); /*Assume all CIs are computed fully*/
	ns++;

      }else{
	if(is_compute) {
	  if((pbest-senscr[cd2cisen[s]]<kb->ci_pbeam)){
	    ng+=approx_mgau_eval (gs,svq,g,kb,s,senscr,feat,best_cid,svq_beam);
	    ns++;
	  }else {
	    senscr[s]=senscr[cd2cisen[s]]; /* backoff to CI score, not gaussians computed */
	  }
	  if (best < senscr[s]) best = senscr[s];
	}
      }
      /*Make a copy to the most recent active list */
      kb->rec_sen_active[s]=sen_active[s];

    }else{ /* Loop handling no computation*/
      /* All complexity of the skipping loop will be coded here */
      if(is_compute){
	if(kb->rec_sen_active[s])
	  senscr[s]=senscr[s]; /*Yes. No change to the score */
	else{
	  kb->rec_sen_active[s]=1;
	  ng+=approx_mgau_eval (gs,svq,g,kb,s,senscr,feat,best_cid,svq_beam);
	  ns++;

	  if(senscr[s]>kb->rec_bst_senscr){ /* Rescore everything if we are better best scores*/
	    E_INFO("Re-normalizing the previous score\n");

	    /*Every thing except the new score are recomputed*/
	    for (t = 0; t < g->n_mgau; t++) {
	      if(kb->rec_sen_active[t]&& t!=s ){
		senscr[t]-=(senscr[s]-kb->rec_bst_senscr);
	      }
	    }
	    /*Update the best senone score*/
	    kb->rec_bst_senscr=senscr[s];
	  }
	  /*Update the new senone score*/
	  senscr[s]-=kb->rec_bst_senscr; /*if senscr[s]> kb->rec_bst_senscr, senscr will equals to 0 at this point*/
	}
      }
    }
  }

  if(!is_skip){
    for (s = 0; s < g->n_mgau; s++){
      if(sen_active[s])
	senscr[s]-=best;
    }
  }else{
   
#if 0
    E_INFO("Best score %d\n",kb->rec_bst_senscr);
    for(s=0 ;s < g-> n_mgau; s++){
      if(sen_active[s]){
	  E_INFO("At the end: Senone %d has scores %d\n",s,senscr[s]);
	if(senscr[s]>0){
	  E_FATAL("Something wrong, senone score > 0\n",s,senscr[s]);
	}
      }
    }
#endif
  }

    
  g->frm_sen_eval = ns;
  g->frm_gau_eval = ng;
    
  return best;

}

