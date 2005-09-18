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
/************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 ************************************************
 * 
 * HISTORY
 *  
 * Jan-24-2005  A Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *              Started
 */

#include "kb.h"
#include "corpus.h"
#include "utt.h"
#include "logs3.h"
#include "pio.h"
#include "gmm_compute.h"
#define MAX_PREV_FRAMES 20

typedef struct score_s{
  int32 frmidx;
  int32 senidx;
  int32 senscr;
  int32 bstidx;
} score_t; 

extern stats_t cd_st;
extern stats_t ci_st;
extern stats_t *sen_st;
extern stats_t  *cur_sen_st;
extern stats_t *sen_Nbest_st;
extern stats_t *cur_sen_Nbest_st;

int32 *cd;

int intcmp_gmm_compute(const void *v1, const void *v2){
    return (cd[*(int32*) v2 ] - cd[*(int32*)v1]);
}


void gmm_compute (void *data, utt_res_t *ur, int32 sf, int32 ef, char *uttid)
{
  kb_t *kb;
  kbcore_t *kbcore;
  mdef_t *mdef;
  dict_t *dict;
  dict2pid_t *d2p;
  mgau_model_t *mgau;
  subvq_t *svq;
  gs_t * gs;

  
  int32 ptranskip;
  int32 s,f,t;
  int32 single_el_list[2];
  stats_t cur_ci_st;
  stats_t cur_cd_st;
  stats_t cur_cd_Nbest_st;
  stats_t *stptr;
  char str[100];
  int32* idx;

  int32* cur_bstidx;
  int32* last_bstidx;
  int32* cur_scr;
  int32* last_scr;
  int32 tmpint;

  int32 pheurtype;
  E_INFO("Processing: %s\n", uttid);

  kb = (kb_t *) data;
  kbcore = kb->kbcore;
  mdef = kbcore_mdef (kbcore);
  dict = kbcore_dict (kbcore);
  d2p = kbcore_dict2pid (kbcore);
  mgau = kbcore_mgau (kbcore);	
  svq = kbcore_svq (kbcore);
  gs = kbcore_gs(kbcore);
  kb->uttid = uttid;

  s3senid_t *cd2cisen;

  ptranskip = kb->beam->ptranskip;

  pheurtype = kb->pl->pheurtype;

  single_el_list[0]=-1;
  single_el_list[1]=-1;
 

  /* Read mfc file and build feature vectors for entire utterance */
  kb->stat->nfr = feat_s2mfc2feat(kbcore_fcb(kbcore), ur->uttfile, cmd_ln_str("-cepdir"),".mfc",
			    sf, ef, kb->feat, S3_MAX_FRAMES);

  cd2cisen=mdef_cd2cisen(mdef);
  /*This should be a procedure instead of just logic */

  init_stat(&cur_cd_st,"Current CD Senone");
  init_stat(&cur_ci_st,"Current CI Senone");
  init_stat(&cur_cd_Nbest_st,"Current CD NBest Senone");
  
  for(s=0;s<mdef->n_ci_sen;s++){
    sprintf(str,"Cur Senone %d",s);
    init_stat(&cur_sen_st[s],str);
  }

  for(t=0;t<(int32) (mdef->n_sen - mdef->n_ci_sen)/NBEST_STEP;t++){
    sprintf(str," %d -Cur Best Senone",t*NBEST_STEP);
    init_stat(&cur_sen_Nbest_st[t],str);
  }

  idx=ckd_calloc(mdef->n_sen-mdef->n_ci_sen,sizeof(int32));
  /* Allocate temporary array for CurScr and Curbst indx and Lat index*/
  
  cur_bstidx=ckd_calloc(mdef->n_sen,sizeof(int32));
  last_bstidx=ckd_calloc(mdef->n_sen,sizeof(int32));
  cur_scr=ckd_calloc(mdef->n_sen,sizeof(int32));
  last_scr=ckd_calloc(mdef->n_sen,sizeof(int32));



  for(f = 0; f < kb->stat->nfr; f++){
    for(s =0 ; s <mgau->n_mgau ; s++){
      /*1, Compute the approximate scores with the last best index. */
	
      if(mgau->mgau[s].bstidx!=NO_BSTIDX){
	single_el_list[0]=mgau->mgau[s].bstidx;
	last_bstidx[s]=mgau->mgau[s].bstidx;
	last_scr[s]=mgau_eval(mgau,s,single_el_list,kb->feat[f][0],f,0); 
      }else{
	last_bstidx[s]=NO_BSTIDX;
      }

      /*2, Compute the exact scores and sort them and get the ranking. */

      kb->ascr->senscr[s]=mgau_eval(mgau,s,NULL,kb->feat[f][0],f,1);      

      /*3, Compute the approximate scores with the current best index */
      if(mgau->mgau[s].bstidx!=NO_BSTIDX){
	single_el_list[0]=mgau->mgau[s].bstidx;
	cur_bstidx[s]=mgau->mgau[s].bstidx;
	cur_scr[s]=mgau_eval(mgau,s,single_el_list,kb->feat[f][0],f,0); 
      }else{
	cur_bstidx[s]=NO_BSTIDX;
	
      }
      
      /* Only test for CD senones, test for best index hit */

      /*Update either CI senone and CD senone)*/

	
      if(!mdef_is_cisenone(mdef,s))
	stptr=&cur_cd_st;
      else
	stptr=&cur_ci_st;

      increment_stat(stptr,
		     abs(last_scr[s]-kb->ascr->senscr[s]),
		     abs(cur_scr[s]-kb->ascr->senscr[s]),
		     abs(kb->ascr->senscr[cd2cisen[s]]-kb->ascr->senscr[s]),
		     (cur_bstidx[s]==last_bstidx[s]));


      if(!mdef_is_cisenone(mdef,s)){
	stptr=&cur_sen_st[cd2cisen[s]];
	increment_stat(stptr,
		       abs(last_scr[s]-kb->ascr->senscr[s]),
		       abs(cur_scr[s]-kb->ascr->senscr[s]),
		       abs(kb->ascr->senscr[cd2cisen[s]]-kb->ascr->senscr[s]),
		       (cur_bstidx[s]==last_bstidx[s]));

	stptr->total_senone+=1;
      }
    }

    cur_cd_st.total_fr++;
    cur_cd_st.total_senone+=mdef->n_sen-mdef->n_ci_sen;
    cur_ci_st.total_fr++;
    cur_ci_st.total_senone+=mdef->n_ci_sen;

    for(s =0 ; s <mdef->n_ci_sen ;s++){
      cur_sen_st[s].total_fr++;
    }

    /*This is the part we need to do sorting */
    /*1, sort the scores in the current frames */
    /*E_INFO("At frame %d\n",f);*/

    /*Pointer trick at here. */
    for(s=0; s<mdef->n_sen-mdef->n_ci_sen;s++){
      idx[s]= s;
    }

    cd=&(kb->ascr->senscr[mdef->n_ci_sen]);
    qsort(idx,mdef->n_sen-mdef->n_ci_sen,sizeof(int32),intcmp_gmm_compute);

    /*This loop is stupid and it is just a hack. */
    for(s=0; s<mdef->n_sen-mdef->n_ci_sen;s++){
      tmpint=idx[s]+mdef->n_ci_sen;

      for(t=0 ; t< (int32) ((float)(mdef->n_sen - mdef->n_ci_sen)/ (float)NBEST_STEP) ; t++){

	if( s < t * NBEST_STEP){
	  
	  increment_stat(&cur_sen_Nbest_st[t],
			 abs(last_scr[tmpint]-kb->ascr->senscr[tmpint]),
			 abs(cur_scr[tmpint]-kb->ascr->senscr[tmpint]),
			 abs(kb->ascr->senscr[cd2cisen[tmpint]]-kb->ascr->senscr[tmpint]),
			 (cur_bstidx[tmpint]==last_bstidx[tmpint]));
	  
	  cur_sen_Nbest_st[t].total_senone+=1;

	}
      }

    }
    
    for(t=0 ; t< (int32) ((float)(mdef->n_sen - mdef->n_ci_sen)/ (float)NBEST_STEP) ; t++){
      cur_sen_Nbest_st[t].total_fr++;
    }
  }

  print_stat(&cur_cd_st);
  print_stat(&cur_ci_st);
  print_stat(&cur_sen_Nbest_st[1]); /*Only show the first NBEST_STEP best */

  


  add_stat(&cd_st,&cur_cd_st);
  add_stat(&ci_st,&cur_ci_st);

  for(s =0 ; s <mdef->n_ci_sen ;s++){
    add_stat(&sen_st[s],&cur_sen_st[s]);
  }
  
  for(s=0 ; s< (int32) (mdef->n_sen - mdef->n_ci_sen)/NBEST_STEP ; s++){
    add_stat(&sen_Nbest_st[s],&cur_sen_Nbest_st[s]);
  }

  ckd_free(idx);
  ckd_free(cur_bstidx);
  ckd_free(last_bstidx);
  ckd_free(cur_scr);
  ckd_free(last_scr);
}

void init_stat(stats_t *st, char* _desc){
  st->total_hit=0;
  st->total_senone=0;
  st->total_lastidx_distortion=0;
  st->total_curidx_distortion=0;
  st->total_ci_distortion=0;
  st->total_fr=0;
  strcpy((char*)st->description,_desc);
}

void add_stat(stats_t *a, stats_t *b){
  a->total_hit+=b->total_hit;
  a->total_senone+=b->total_senone;
  a->total_lastidx_distortion+=b->total_lastidx_distortion;
  a->total_curidx_distortion+=b->total_curidx_distortion;
  a->total_ci_distortion+=b->total_ci_distortion;
  a->total_fr+=b->total_fr;

}

void increment_stat(stats_t *a,int32 lst_idx_distort,int32 cur_idx_distort,int32 ci_distort, int32 hit)
{
  a->total_lastidx_distortion+=logs3_to_log(lst_idx_distort);
  a->total_curidx_distortion+=logs3_to_log(cur_idx_distort);
  a->total_ci_distortion+=logs3_to_log(ci_distort);
  
  if(hit){
    a->total_hit+=1;
  }

}

void print_stat(stats_t *st){
    E_INFO("TOTAL of %s %f, hit percentage from last index of %s %f\n",
	   (char*)st->description, 
	   st->total_senone,
	   (char*)st->description, 
	   (float32)(st->total_hit)/(float32)(st->total_senone)
	   );

    E_INFO("Distortion/Frame of %s if the current index is used %f\n",
	   (char*)st->description, 
	   st->total_curidx_distortion/(float32)(st->total_fr)
	   );

    E_INFO("Distortion/Frame of %s if the last index is used %f\n",
	   (char*)st->description, 
	   st->total_lastidx_distortion/(float32)(st->total_fr)
	   );

    E_INFO("Distortion/Frame of %s if the CI index is used %f\n",
	   (char*)st->description, 
	   st->total_ci_distortion/(float32)(st->total_fr));


}
