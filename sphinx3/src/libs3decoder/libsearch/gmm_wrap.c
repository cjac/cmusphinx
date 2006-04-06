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

/* gmm_wrap.c
 * HISTORY
 * $Log$
 * Revision 1.4  2006/04/06  14:03:02  dhdfu
 * Prevent confusion among future generations by calling this s2_semi_mgau instead of sc_vq
 * 
 * Revision 1.3  2006/04/05 20:27:34  dhdfu
 * A Great Reorganzation of header files and executables
 *
 * Revision 1.2  2006/02/23 05:38:39  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH: Added multi-stream GMM computation routine.
 *
 * Revision 1.1.4.4  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.1.4.3  2005/08/03 18:54:33  dhdfu
 * Fix the support for multi-stream / semi-continuous models.  It is
 * still kind of a hack, but it now works.
 *
 * Revision 1.1.4.2  2005/08/02 21:31:21  arthchan2003
 * Added interface for 1, doing multi stream gmm computation with/without composite senone. 2, doing gmm computation (ms or ss optimized) with/wihout composite senone.  Haven't tested on the SCHMM on s3.x yet.  I think it will work though.
 *
 * Revision 1.1.4.1  2005/07/24 01:35:41  arthchan2003
 * Add a wrapper for computing senone score without computing composite senone score. Mainly used in mode FSG now
 *
 * Revision 1.1  2005/06/21 22:48:14  arthchan2003
 * A wrapper that provide the function pointer interface of approx_cont_mgau_ci_eval  and approx_cont_mgau_frame_eval.  They are used in srch_gmm_compute_lv1  and srch_gmm_compute_lv2 respectively.  This will also be the home of other gmm computation routine. (Say the s3.0 version of GMM computation)
 *
 * Revision 1.3  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.2  2005/06/11 06:57:50  archan
 * 1, Add senscalre for gmm_wrap.c
 *
 * Revision 1.1  2005/04/22 04:22:37  archan
 * Add gmm_wrap, this will share code across op_mode 4 and op_mode 5. Also it also separate active senone selection into a different process.  I hope this is the final step before making the WST search works.  At the current stage, the code of mode-5 looks very much alike mode-4.  This is intended because in Prototype 4, tail sharing will be used to reduce memory.
 *
 */

#include "srch.h"

int32 s3_cd_gmm_compute_sen_comp(void *srch, float32 **feat, int32 wav_idx)
{
  int32 flag;
  srch_t* s;
  ascr_t *ascr;
  kbcore_t *kbcore;

  s=(srch_t*) srch;
  kbcore = s->kbc;
  ascr=s->ascr;

  assert (kbcore->ms_mgau || kbcore->mgau || kbcore->s2_mgau);
  assert (!(kbcore->ms_mgau && kbcore->mgau && kbcore->s2_mgau));

  flag=s3_cd_gmm_compute_sen(srch, feat,wav_idx);

  if(flag!=SRCH_SUCCESS){
    E_INFO("Computation of senone failed\n");
    return flag;
  }
  /* Evaluate composite senone scores from senone scores */
  dict2pid_comsenscr (kbcore_dict2pid(kbcore), ascr->senscr, ascr->comsen);
  return SRCH_SUCCESS;

}

int32 s3_cd_gmm_compute_sen(void *srch, float32 **feat, int32 wav_idx)
{
  srch_t* s;
  mdef_t *mdef;
  ms_mgau_model_t *ms_mgau;
  mgau_model_t *mgau;
  ascr_t *ascr;
  kbcore_t *kbcore;
  fast_gmm_t *fgmm;
  pl_t *pl;
  stat_t *st;
  float32 *fv;

  s=(srch_t*) srch;
  kbcore = s->kbc;

  pl = s->pl;
  fgmm = s->fastgmm;
  st = s->stat;

  mdef = kbcore_mdef (kbcore);
  ms_mgau = kbcore_ms_mgau (kbcore);
  mgau = kbcore_mgau(kbcore);
  ascr=s->ascr;

  assert (kbcore->ms_mgau || kbcore->mgau || kbcore->s2_mgau);
  assert (!(kbcore->ms_mgau && kbcore->mgau && kbcore->s2_mgau));

  /* Always use the first buffer in the cache*/
  if(kbcore->ms_mgau){
    s->senscale=ms_cont_mgau_frame_eval(ascr,ms_mgau,mdef,feat);
    /* FIXME: Statistics is not correctly updated */
  }
  else if (kbcore->s2_mgau) {
    s->senscale = s2_semi_mgau_frame_eval(kbcore->s2_mgau, ascr, fgmm, feat, wav_idx);
    /* FIXME: Statistics is not correctly updated */
  }
  else if(kbcore->mgau){
    fv=feat[0];
    s->senscale=approx_cont_mgau_frame_eval(kbcore,fgmm,ascr,fv,wav_idx,
			      ascr->cache_ci_senscr[s->cache_win_strt],
			      &(st->tm_ovrhd));
    st->utt_sen_eval += mgau_frm_sen_eval(mgau);
    st->utt_gau_eval += mgau_frm_gau_eval(mgau);
  }
  else
    E_FATAL("Panic, someone delete the assertion before this block\n");
  

  return SRCH_SUCCESS;
}


int32 approx_ci_gmm_compute(void *srch, float32 *feat, int32 cache_idx, int32 wav_idx)
{
  srch_t* s;
  stat_t *st;
  fast_gmm_t *fgmm;
  mdef_t *mdef;
  mgau_model_t *mgau;
  kbcore_t *kbcore;
  ascr_t *ascr;


  s=(srch_t*) srch;

  kbcore = s->kbc;
  mdef = kbcore_mdef (kbcore);
  mgau = kbcore_mgau (kbcore);
  fgmm = s->fastgmm;
  st = s->stat;
  ascr=s->ascr;

  /* No CI-GMM done for multistream models (for now) */
  if (mgau == NULL) {
    assert(kbcore_ms_mgau(kbcore) || kbcore_s2_mgau(kbcore));
    return SRCH_SUCCESS;
  }

  approx_cont_mgau_ci_eval(kbcore,
			   fgmm,
			   mdef,
			   feat,
			   ascr->cache_ci_senscr[cache_idx],
			   &(ascr->cache_best_list[cache_idx]),
			   wav_idx);
  
  st->utt_cisen_eval += mgau_frm_cisen_eval(mgau);
  st->utt_cigau_eval += mgau_frm_cigau_eval(mgau);
  return SRCH_SUCCESS;
}


int32 approx_cd_gmm_compute_sen_comp(void *srch, float32 **feat, int32 wav_idx)
{
  int32 flag;
  srch_t* s;
  ascr_t *ascr;
  kbcore_t *kbcore;

  s=(srch_t*) srch;
  kbcore = s->kbc;
  ascr=s->ascr;

  assert(kbcore->mgau);
  assert(!kbcore->ms_mgau);

  flag=approx_cd_gmm_compute_sen(srch, feat,wav_idx);

  if(flag!=SRCH_SUCCESS){
    E_INFO("Computation of senone failed\n");
    return flag;
  }
  /* Evaluate composite senone scores from senone scores */
  dict2pid_comsenscr (kbcore_dict2pid(kbcore), ascr->senscr, ascr->comsen);
  return SRCH_SUCCESS;

}



int32 approx_cd_gmm_compute_sen(void *srch, float32 **feat, int32 wav_idx)
{
  srch_t* s;
  mdef_t *mdef;
  fast_gmm_t *fgmm;
  pl_t *pl;
  stat_t *st;
  mgau_model_t *mgau;
  ascr_t *ascr;
  kbcore_t *kbcore;
  float32 *fv;

  fv=feat[0]; /*HACK ALERT. See! we are only using the first feature. So, this could 
		only be used for CMU's CDHMM i.e stream=1*/

  s=(srch_t*) srch;
  kbcore = s->kbc;

  mdef = kbcore_mdef (kbcore);
  mgau = kbcore_mgau (kbcore);

  pl = s->pl;
  fgmm = s->fastgmm;
  st = s->stat;
  ascr=s->ascr;

  assert(kbcore->mgau);
  assert(!kbcore->ms_mgau);
  /* Always use the first buffer in the cache*/
  s->senscale=approx_cont_mgau_frame_eval(kbcore,fgmm,ascr,fv,wav_idx,
			      ascr->cache_ci_senscr[s->cache_win_strt],
			      &(st->tm_ovrhd));
  
  st->utt_sen_eval += mgau_frm_sen_eval(mgau);
  st->utt_gau_eval += mgau_frm_gau_eval(mgau);
  

  return SRCH_SUCCESS;
}





