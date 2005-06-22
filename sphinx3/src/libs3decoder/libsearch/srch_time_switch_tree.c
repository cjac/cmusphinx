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

/* srch_time_switch_tree.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2005/06/22  02:44:02  arthchan2003
 * Log. Time-switching tree search (aka Ravi's search or Mode 4) is a
 * wrapped up of the search we had before Sphinx 3.6.  It is not a
 * conventional way to do a lexical tree search and it provide three
 * interesting approach in solving the problem of cross-word triphone,
 * word segmentation and languag model.
 * 
 * Though, the functions are now wrapped up using either srch interface,
 * the performance of the code has not changed.  We deliberately keep
 * this search because we knew that the new search (mode 5) could be
 * risky.
 * 
 * Revision 1.15  2005/06/17 23:44:40  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.14  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.13  2005/06/11 06:58:35  archan
 * In both mode 4 and mode 5 searches, fixed a serious bug that cause the beam computation screwed up.
 *
 * Revision 1.12  2005/06/03 05:22:52  archan
 * Commented variables after refactor both mode 4 and mode 5 search to use reg_result_dump.
 *
 * Revision 1.11  2005/05/11 06:10:39  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.10  2005/05/04 04:02:25  archan
 * Implementation of lm addition, deletion in (mode 4) time-switching tree implementation of search.  Not yet tested. Just want to keep up my own momentum.
 *
 * Revision 1.9  2005/05/03 06:57:44  archan
 * Finally. The word switching tree code is completed. Of course, the reporting routine largely duplicate with time switching tree code.  Also, there has to be some bugs in the filler treatment.  But, hey! These stuffs we can work on it.
 *
 * Revision 1.8  2005/05/03 04:09:10  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.7  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.6  2005/04/25 19:22:48  archan
 * Refactor out the code of rescoring from lexical tree. Potentially we want to turn off the rescoring if we need.
 *
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. This replaced utt.c starting from Sphinx 3.6. 
 */

#include "srch.h"
#include "dict.h"
#include "libutil/cmd_ln.h"
#include "approx_cont_mgau.h"

#define REPORT_SRCH_TST 1 

int srch_TST_init(kb_t *kb, void *srch)
{

  int32 n_ltree;
  kbcore_t* kbc;
  srch_TST_graph_t* tstg ;
  int32 i,j;
  srch_t* s;
  ptmr_t tm_build;
  
  kbc=kb->kbcore;
  
  s=(srch_t *)srch;

  if(cmd_ln_int32("-Nstalextree"))
    E_WARN("-Nstalextree is omitted in TST search.\n");

  /** STRUCTURE : allocation of the srch graphs */
  tstg=ckd_calloc(1,sizeof(srch_TST_graph_t));

  tstg->epl = cmd_ln_int32("-epl");
  tstg->n_lextree = cmd_ln_int32("-Nlextree");
  tstg->isLMLA=cmd_ln_int32("-treeugprob");

  /* CHECK: make sure the number of lexical tree is at least one. */

  if (tstg->n_lextree < 1) {
    E_WARN("No. of ugtrees specified: %d; will instantiate 1 ugtree\n",tstg->n_lextree);
    tstg->n_lextree = 1;
  }

  n_ltree=tstg->n_lextree;


  /* STRUCTURE and REPORT: Initialize lexical tree. Filler tree's
     initialization is followed.  */

  tstg->ugtree = (lextree_t **) ckd_calloc (kbc->lmset->n_lm*n_ltree, sizeof(lextree_t *));

  /* curugtree is a pointer pointing the current unigram tree which
     were being used. */

  tstg->curugtree = (lextree_t **) ckd_calloc (n_ltree, sizeof(lextree_t *));

  /* Just allocate pointers*/

  ptmr_reset(&(tm_build));
  for(i=0;i<kbc->lmset->n_lm;i++){
    for (j = 0; j < n_ltree; j++) {
      /*     ptmr_reset(&(tm_build));*/
      ptmr_start(&tm_build);
      tstg->ugtree[i*n_ltree+j]=lextree_init(kbc,kbc->lmset->lmarray[i],lmset_idx_to_name(kbc->lmset,i),
					     tstg->isLMLA,REPORT_SRCH_TST);
      ptmr_stop(&tm_build);
	
      if(tstg->ugtree[i*n_ltree+j]==NULL){
	E_INFO("Fail to allocate lexical tree for lm %d and lextree %d\n",i,j);
	return SRCH_FAILURE;
      }
      
      if(REPORT_SRCH_TST){
	E_INFO("Lextrees (%d) for lm %d, its name is %s, it has %d nodes(ug)\n",
	       j, i, lmset_idx_to_name(kbc->lmset,i),lextree_n_node(tstg->ugtree[i*n_ltree+j]));
      }
    }
  }
  E_INFO("Time for building trees, %4.4f CPU %4.4f Clk\n",tm_build.t_cpu,tm_build.t_elapsed);




  /* By default, curugtree will be pointed to the first sets of tree */
  for(j=0;j<n_ltree;j++)
    tstg->curugtree[j]=tstg->ugtree[j];
  
  
  /* STRUCTURE: Create filler lextrees */
  /* ARCHAN : only one filler tree is supposed to be built even for dynamic LMs */
  /* Get the number of lexical tree */
  tstg->fillertree = (lextree_t **)ckd_calloc(n_ltree, sizeof(lextree_t *));
  
  for (i = 0; i < n_ltree; i++) {
    if((tstg->fillertree[i]=fillertree_init(kbc))==NULL){
      E_INFO("Fail to allocate filler tree  %d\n",i);
      return SRCH_FAILURE;
    }
    if(REPORT_SRCH_TST){
      E_INFO("Lextrees(%d), %d nodes(filler)\n",
	     i, 
	     lextree_n_node(tstg->fillertree[0]));    
    }
  }
  
  if (cmd_ln_int32("-lextreedump")) {
    for(i=0;i<kbc->lmset->n_lm;i++){
      for (j = 0; j < n_ltree; j++) {
	fprintf (stderr, "LM %d name %s UGTREE %d\n",i,lmset_idx_to_name(kbc->lmset,i),j);
	lextree_dump (tstg->ugtree[i*n_ltree+j], kbc->dict, stderr);
      }
    }
    for (i = 0; i < n_ltree; i++) {
      fprintf (stderr, "FILLERTREE %d\n", i);
      lextree_dump (tstg->fillertree[i], kbc->dict, stderr);
    }
  }

  tstg->histprune=histprune_init(cmd_ln_int32("-maxhmmpf"),
		 cmd_ln_int32("-maxhistpf"),
		 cmd_ln_int32("-maxwpf"),
		 cmd_ln_int32("-hmmhistbinsize"),
		 (tstg->curugtree[0]->n_node + tstg->fillertree[0]->n_node) *tstg->n_lextree
		 );


  /* Glue the graph structure */
  s->grh->graph_struct=tstg;
  s->grh->graph_type=GRAPH_STRUCT_TST;

  return SRCH_SUCCESS;

}
int srch_TST_uninit(void *srch)
{
  return SRCH_SUCCESS;
}
int srch_TST_begin(void *srch)
{
  kbcore_t *kbc;
  int32 n, pred;
  int32 i;
  mgau_model_t *g;
  srch_t* s;
  srch_TST_graph_t* tstg;

  s=(srch_t*) srch;
  assert(s);
  assert(s->op_mode==4);
  assert(s->grh);
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  assert(tstg);

  kbc = s->kbc;
  g=kbc->mgau;

  stat_clear_utt(s->stat);
  histprune_zero_histbin(tstg->histprune);

  /* Insert initial <s> into vithist structure */
  pred = vithist_utt_begin (s->vithist, kbc);
  assert (pred == 0);	/* Vithist entry ID for <s> */
  
  /* This reinitialize the cont_mgau routine in a GMM.  */
  for(i=0;i<g->n_mgau;i++){
    g->mgau[i].bstidx=NO_BSTIDX;
    g->mgau[i].updatetime=NOT_UPDATED;
  }

  /* Enter into unigram lextree[0] */
  n = lextree_n_next_active(tstg->curugtree[0]);
  assert (n == 0);
  lextree_enter (tstg->curugtree[0], mdef_silphone(kbc->mdef), -1, 0, pred,
		 s->beam->hmm);
    
  /* Enter into filler lextree */
  n = lextree_n_next_active(tstg->fillertree[0]);
  assert (n == 0);
  lextree_enter (tstg->fillertree[0], BAD_S3CIPID, -1, 0, pred, s->beam->hmm);
    
  tstg->n_lextrans = 1;

  for (i = 0; i < tstg->n_lextree; i++) {
    lextree_active_swap (tstg->curugtree[i]);
    lextree_active_swap (tstg->fillertree[i]);
  }

  return SRCH_SUCCESS;
}


int srch_TST_end(void *srch)
{
  int32 id;
  /*int32 ascr, lscr;*/
  /*glist_t hyp;*/
  /*gnode_t *gn;*/
  /*hyp_t *h;*/
  int32 i;
  FILE *fp;
  /**latfp, *bptfp;*/
  srch_t* s;
  srch_TST_graph_t* tstg;
  stat_t* st;
  histprune_t* hp;
  dict_t *dict;
  char* uttid;


  s=(srch_t*) srch;
  assert(s);
  assert(s->op_mode==4);
  assert(s->grh);
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  assert(tstg);

  st = s->stat;
  hp = tstg->histprune;
    
  fp = stderr;
  dict = kbcore_dict (s->kbc);
  uttid = s->uttid;
    
  if ((id = vithist_utt_end (s->vithist, s->kbc)) >= 0) {
    reg_result_dump(s,id);
  } else
    E_ERROR("%s: No recognition\n\n", uttid);

  /* Statistics update/report*/
  st->utt_wd_exit=vithist_n_entry(s->vithist);
  stat_report_utt(st,uttid);
  histprune_showhistbin(tstg->histprune,st->nfr,uttid);
  stat_update_corpus(st);
    
  ptmr_reset (&(st->tm_sen));
  ptmr_reset (&(st->tm_srch));
  ptmr_reset (&(st->tm_ovrhd));
  
#if (!defined(WIN32))
  if (! system("ps auxgw > /dev/null 2>&1")) {
    system ("ps aguxwww | grep /live | grep -v grep");
    system ("ps aguxwww | grep /dec | grep -v grep");
  }
#endif
    
  for (i = 0; i < tstg->n_lextree; i++) {
    lextree_utt_end (tstg->curugtree[i], s->kbc);
    lextree_utt_end (tstg->fillertree[i], s->kbc);
  }
    
  vithist_utt_reset (s->vithist);
  
  lm_cache_stats_dump (kbcore_lm(s->kbc));
  lm_cache_reset (kbcore_lm(s->kbc));
  
  return SRCH_SUCCESS;
}

int srch_TST_decode(void *srch)
{
  return SRCH_SUCCESS;
}

int srch_TST_add_lm(void* srch, lm_t *lm, const char *lmname)
{
  lmset_t* lms;
  srch_t* s;
  srch_TST_graph_t* tstg;
  kbcore_t* kbc;
  int32 n_ltree;
  int32 j;
  int32 idx;

  s=(srch_t*)srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;

  kbc=s->kbc;
  lms=kbc->lmset;

  n_ltree   = tstg->n_lextree;

  /* 1, Add a new LM */ 
  lmset_add_lm(lms,lm,lmname); /* No. of lm will be increased by 1 */

  /* 2, Create a new set of trees for this LM. */
  
  tstg->ugtree = (lextree_t **) ckd_realloc (tstg->ugtree, (lms->n_lm*n_ltree)*sizeof(lextree_t *));

  idx=lms->n_lm-1;
  for(j=0 ; j < n_ltree ;j++){
    tstg->ugtree[(idx)*n_ltree+j]=lextree_init(kbc,
					       lms->lmarray[idx],
					       lmset_idx_to_name(lms,idx),
					       tstg->isLMLA,
					       REPORT_SRCH_TST);
	
    if(tstg->ugtree[idx*n_ltree+j]==NULL){
      E_INFO("Fail to allocate lexical tree for lm %d and lextree %d\n",idx,j);
      return SRCH_FAILURE;
    }
    
    if(REPORT_SRCH_TST){
      E_INFO("Lextrees (%d) for lm %d, its name is %s, it has %d nodes(ug)\n",
	     j, idx, lmset_idx_to_name(kbc->lmset,idx),lextree_n_node(tstg->ugtree[idx*n_ltree+j]));
    }
  }

  return SRCH_SUCCESS;
}

int srch_TST_delete_lm(void* srch, const char *lmname)
{
  lmset_t* lms;
  kbcore_t* kbc=NULL;
  srch_t* s;
  srch_TST_graph_t* tstg;
  int32 lmidx;
  int32 n_ltree;
  int i,j;

  s=(srch_t*)srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbc=s->kbc;
  lms=kbc->lmset;
  n_ltree=tstg->n_lextree;

  /* Get the index of a the lm name */
  lmidx=lmset_name_to_idx(lms,lmname);

  /* Free the n_ltree copies of tree */
  for(j=0;j<n_ltree;j++){
    lextree_free(tstg->curugtree[lmidx*n_ltree+j]);
    tstg->curugtree[lmidx*n_ltree+j]=NULL;
  }

  /* Shift the pointer by one in the trees */
  for(i=lmidx ; i< kbc->lmset->n_lm ; i++){
    for(j=0; j < n_ltree ; j++){
      tstg->curugtree[i*n_ltree+j]=tstg->curugtree[(i+1)*n_ltree+j];
    }
  }
  /* Tree is handled, now also handled the lmset */

  /* Delete the LM */
  /* Remember that the n_lm is minus by 1 at this point */
  lmset_delete_lm(lms,lmname);

  return SRCH_SUCCESS;
}

int srch_TST_set_lm(void *srch, const char* lmname)
{
  lmset_t* lms;
  lm_t* lm;
  kbcore_t* kbc=NULL;
  int j;
  int idx;
  srch_t* s;
  srch_TST_graph_t* tstg;

  /*  s3wid_t dictid;*/

  s=(srch_t*)srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbc=s->kbc;
  lms=kbc->lmset;

  kbc->lmset->cur_lm=NULL;

  for(j=0;j<tstg->n_lextree;j++){
    tstg->curugtree[j]=NULL;
  }

  assert(lms!=NULL);
  assert(lms->lmarray!=NULL);

  idx=lmset_name_to_idx(lms,lmname);

  if(idx==LM_NOT_FOUND){
    E_ERROR("LM name %s cannot be found, use the first language model");
    idx=0;
  }

  /* Don't switch LM if we are already using this one. */
  if(lms->cur_lm==lms->lmarray[idx]){ /* What if idx=0 and the first LM is not initialized? The previous assert safe-guard this.*/
    return SRCH_SUCCESS;
  }

  lmset_set_curlm_widx(lms,idx);

  for(j=0;j<tstg->n_lextree;j++){
    tstg->curugtree[j]=tstg->ugtree[idx*tstg->n_lextree+j];
  }

  lm=kbc->lmset->cur_lm;

  if((s->vithist->lms2vh_root=
      (vh_lms2vh_t**)ckd_realloc(s->vithist->lms2vh_root,
				lm_n_ug(lm)*sizeof(vh_lms2vh_t *)
				))==NULL){
    E_FATAL("failed to allocate memory for vithist\n");
  }

  histprune_update_histbinsize(tstg->histprune,
			       tstg->histprune->hmm_hist_binsize,
			       ((tstg->curugtree[0]->n_node) + (tstg->fillertree[0]->n_node)) *tstg->n_lextree);


  if(REPORT_SRCH_TST){
    E_INFO("Current LM name %s\n",lmset_idx_to_name(kbc->lmset,idx));
    E_INFO("LM ug size %d\n",lm->n_ug);
    E_INFO("LM bg size %d\n",lm->n_bg);
    E_INFO("LM tg size %d\n",lm->n_tg);
    E_INFO("HMM history bin size %d\n", tstg->histprune->hmm_hist_bins+1);
    
    for(j=0;j<tstg->n_lextree;j++){
      E_INFO("Lextrees(%d), %d nodes(ug)\n",
	     j, lextree_n_node(tstg->curugtree[j]));
    }
  }
  return SRCH_SUCCESS;
}

int srch_TST_eval_beams_lv1 (void* srch)
{
  return SRCH_SUCCESS;
}
int srch_TST_eval_beams_lv2 (void* srch)
{
  return SRCH_SUCCESS;
}

int srch_TST_hmm_compute_lv1(void *srch)
{
  return SRCH_SUCCESS;
}

int srch_TST_compute_heuristic(void *srch, int32 win_efv)
{
  srch_t* s;
  srch_TST_graph_t* tstg;
  pl_t *pl;
  ascr_t *ascr;
  mdef_t *mdef;

  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  mdef = kbcore_mdef (s->kbc);

  ascr=s->ascr;
  pl = s->pl;

  if(pl->pheurtype!=0) 
    pl_computePhnHeur(mdef, ascr, pl, pl->pheurtype, s->cache_win_strt,win_efv);

  return SRCH_SUCCESS;
}


int srch_TST_hmm_compute_lv2(void *srch, int32 frmno)
{
  /* This is local to this codebase */

  int32 i,j ;
  lextree_t *lextree;
  srch_t* s;
  srch_TST_graph_t* tstg;
  kbcore_t *kbcore;
  beam_t *bm;
  ascr_t *ascr;
  histprune_t *hp;
  stat_t *st;
  pl_t *pl;

  int32 besthmmscr, bestwordscr; 
  int32 frm_nhmm, hb, pb, wb;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  int32 maxwpf;        /* Local version of Max words per frame, don't expect it to change */
  int32 maxhistpf;     /* Local version of Max histories per frame, don't expect it to change */
  int32 maxhmmpf;      /* Local version of Max active HMMs per frame, don't expect it to change  */
  int32 histbinsize;   /* Local version of histogram bin size, don't expect it to change */
  int32 numhistbins;    /* Local version of number of histogram bins, don't expect it to change */
  int32 hmmbeam;       /* Local version of hmm beam, don't expect it to change. */
  int32 pbeam;         /* Local version of phone beam, don't expect it to change. */
  int32 wbeam;         /* Local version of word beam , don't expect it to change */
  int32* hmm_hist;      /* local version of histogram bins. */

  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;

  n_ltree   = tstg->n_lextree;
  kbcore = s->kbc;
  hp = tstg->histprune;
  bm = s->beam;
  ascr=s->ascr;
  hmm_hist=hp->hmm_hist;
  st=s->stat;
  pl= s->pl;
    

  maxwpf    = hp->maxwpf;
  maxhistpf = hp->maxwpf;
  maxhmmpf  = hp->maxhmmpf;
  histbinsize = hp->hmm_hist_binsize;
  numhistbins = hp->hmm_hist_bins;
  hmmbeam   = s->beam->hmm;
  pbeam     = s->beam->ptrans;
  wbeam     = s->beam->word;

  /* Evaluate active HMMs in each lextree; note best HMM state score */
  besthmmscr = MAX_NEG_INT32;
  bestwordscr = MAX_NEG_INT32;
  frm_nhmm = 0;

  for (i = 0; i < (n_ltree <<1); i++) {
    lextree = (i < n_ltree) ? tstg->curugtree[i] : tstg->fillertree[i - n_ltree];
    if (s->hmmdumpfp != NULL)
      fprintf (s->hmmdumpfp, "Fr %d Lextree %d #HMM %d\n", frmno, i, 
	       lextree->n_active);
    lextree_hmm_eval (lextree, kbcore, ascr, frmno, s->hmmdumpfp);
    
    if (besthmmscr < lextree->best)
      besthmmscr = lextree->best;
    if (bestwordscr < lextree->wbest)
      bestwordscr = lextree->wbest;
      
    st->utt_hmm_eval += lextree->n_active;
    frm_nhmm += lextree->n_active;
  }
  if (besthmmscr > 0) {
    E_ERROR("***ERROR*** Fr %d, best HMM score > 0 (%d); int32 wraparound?\n",
	    frmno, besthmmscr);
  }
  
  hmm_hist[frm_nhmm / histbinsize]++;
    
  /* Set pruning threshold depending on whether number of active HMMs 
   * is within limit 
   */
  /* ARCHAN: MAGIC */
  if (frm_nhmm > (maxhmmpf + (maxhmmpf >> 1))) {
    int32 *bin, nbin, bw;
    
    /* Use histogram pruning */
    nbin = 1000;
    bw = -(hmmbeam) / nbin;
    bin = (int32 *) ckd_calloc (nbin, sizeof(int32));
    
    for (i = 0; i < (n_ltree <<1); i++) {
      lextree = (i < n_ltree) ?
	tstg->curugtree[i] : tstg->fillertree[i - n_ltree];
      
      lextree_hmm_histbin (lextree, besthmmscr, bin, nbin, bw);
    }
    
    for (i = 0, j = 0; (i < nbin) && (j < maxhmmpf); i++, j += bin[i]);
    ckd_free ((void *) bin);
    
    /* Determine hmm, phone, word beams */
    hb = -(i * bw);
    pb = (hb > pbeam) ? hb : pbeam;
    wb = (hb > wbeam) ? hb : wbeam;
  } else {
    hb = hmmbeam;
    pb = pbeam;
    wb = wbeam;
  }
   
  bm->bestscore = besthmmscr;
  bm->bestwordscore = bestwordscr;
    
  bm->thres = bm->bestscore + hb;	/* HMM survival threshold */
  bm->phone_thres = bm->bestscore + pb;	/* Cross-HMM transition threshold */
  bm->word_thres = bm->bestwordscore + wb;	/* Word exit threshold */

  return SRCH_SUCCESS;
}
int srch_TST_propagate_graph_ph_lv1(void *srch)
{

  return SRCH_SUCCESS;
}
int srch_TST_propagate_graph_wd_lv1(void *srch)
{

  return SRCH_SUCCESS;
}

int srch_TST_propagate_graph_ph_lv2(void *srch, int32 frmno)
{
  int32 i ;
  srch_t* s;
  srch_TST_graph_t* tstg;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  int32 ptranskip;     /* intervals at which wbeam is used for phone transitions, don't expect it to change */
  int32 pheurtype;     /* Local version of pheurtype, don't expect it to change */

  lextree_t *lextree;
  kbcore_t *kbcore;
  vithist_t *vh;
  pl_t *pl;

  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;
  vh   = s->vithist;
  pl = s->pl;
  pheurtype = pl->pheurtype;

  n_ltree   = tstg->n_lextree;
  ptranskip = s->beam->ptranskip;

  /*  st->utt_node_active*/
  if(ptranskip==0){
    for (i = 0; i < (n_ltree <<1); i++) {
      lextree = (i < n_ltree) ? tstg->curugtree[i] : tstg->fillertree[i - tstg->n_lextree];
      lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
			    s->beam->thres, 
			    s->beam->phone_thres, 
			    s->beam->word_thres,pl);

    }
  }else{
    for (i = 0; i < (n_ltree <<1); i++) {
      lextree = (i < n_ltree) ? tstg->curugtree[i] : tstg->fillertree[i - n_ltree];
      
      if ((frmno % ptranskip) != 0){
	lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
			      s->beam->thres, 
			      s->beam->phone_thres, 
			      s->beam->word_thres,pl);

      }
      else{
	lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
			      s->beam->thres, 
			      s->beam->word_thres, 
			      s->beam->word_thres,pl);

      }
    }
  }

  return SRCH_SUCCESS;
}

int srch_TST_rescoring(void *srch,int32 frmno)
{
  int32 i ;
  srch_t* s;
  srch_TST_graph_t* tstg;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  int32 ptranskip;     /* intervals at which wbeam is used for phone transitions, don't expect it to change */
  lextree_t *lextree;
  kbcore_t *kbcore;
  vithist_t *vh;

  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;
  vh   = s->vithist;

  n_ltree   = tstg->n_lextree;
  ptranskip = s->beam->ptranskip;

  if(ptranskip==0){
    for (i = 0; i < (n_ltree <<1); i++) {
      lextree = (i < n_ltree) ? tstg->curugtree[i] : tstg->fillertree[i - tstg->n_lextree];
      lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				   s->beam->word_thres,s->senscale);

    }
  }else{
    for (i = 0; i < (n_ltree <<1); i++) {
      lextree = (i < n_ltree) ? tstg->curugtree[i] : tstg->fillertree[i - n_ltree];
      
      if ((frmno % ptranskip) != 0){
	lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				     s->beam->word_thres,s->senscale);

      }
      else{
  	lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
	  s->beam->word_thres,s->senscale);

      }
    }
  }

  return SRCH_SUCCESS;
}


static void srch_utt_word_trans (srch_t* s, int32 cf)
{
  int32 k, th;
  vithist_t *vh;
  vithist_entry_t *ve;
  int32 vhid, le, n_ci, score;
  int32 maxpscore;
  int32 *bs = NULL, *bv = NULL, epl;
  beam_t *bm;
  s3wid_t wid;
  int32 p;
  dict_t *dict;
  mdef_t *mdef;
  srch_TST_graph_t* tstg ;

  maxpscore=MAX_NEG_INT32;
  bm=s->beam;

  tstg=(srch_TST_graph_t*) s->grh->graph_struct;

  vh = s->vithist;
  th = bm->bestscore + bm->hmm;	/* Pruning threshold */
  
  if (vh->bestvh[cf] < 0)
    return;
  
  dict = kbcore_dict(s->kbc);
  mdef = kbcore_mdef(s->kbc);
  n_ci = mdef_n_ciphone(mdef);
  
  /* Initialize best exit for each distinct word-final CI phone to NONE */
  
  bs=bm->wordbestscores;
  bv=bm->wordbestexits;
  epl=tstg->epl;

  for (p = 0; p < n_ci; p++) {
    bs[p] = MAX_NEG_INT32;
    bv[p] = -1;
  }
  
  /* Find best word exit in this frame for each distinct word-final CI phone */
  vhid = vithist_first_entry (vh, cf);
  le = vithist_n_entry (vh) - 1;
  for (; vhid <= le; vhid++) {
    ve = vithist_id2entry (vh, vhid);
    if (! vithist_entry_valid(ve))
      continue;
    
    wid = vithist_entry_wid (ve);
    p = dict_last_phone (dict, wid);
    if (mdef_is_fillerphone(mdef, p))
      p = mdef_silphone(mdef);
    
    score = vithist_entry_score (ve);
    if (score > bs[p]) {
      bs[p] = score;
      bv[p] = vhid;
      if (maxpscore < score)
	{
	  maxpscore=score;
	  /*	E_INFO("maxscore = %d\n", maxpscore); */
	}
    }
  }
  
  /* Find lextree instance to be entered */
  k = tstg->n_lextrans++;
  k = (k % (tstg->n_lextree * epl)) / epl;
  
  /* Transition to unigram lextrees */
  for (p = 0; p < n_ci; p++) {
    if (bv[p] >= 0){
      if (s->beam->wordend==0 || bs[p]> s->beam->wordend + maxpscore)
	{
	  /* RAH, typecast p to (s3cipid_t) to make compiler happy */
	  lextree_enter (tstg->curugtree[k], (s3cipid_t) p, cf, bs[p], bv[p], th); 
	}
    }

  }
  
  /* Transition to filler lextrees */
  lextree_enter (tstg->fillertree[k], BAD_S3CIPID, cf, vh->bestscore[cf],
		 vh->bestvh[cf], th);
}

int srch_TST_propagate_graph_wd_lv2(void *srch, int32 frmno)
{
  dict_t *dict;
  vithist_t *vh;
  histprune_t *hp;
  kbcore_t *kbcore;

  srch_t* s;
  srch_TST_graph_t* tstg;
  int32 maxwpf;        /* Local version of Max words per frame, don't expect it to change */
  int32 maxhistpf;     /* Local version of Max histories per frame, don't expect it to change */
  int32 maxhmmpf;      /* Local version of Max active HMMs per frame, don't expect it to change  */


  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;

  hp = tstg->histprune;
  vh   = s->vithist;
  dict = kbcore_dict (kbcore);

  maxwpf    = hp->maxwpf;
  maxhistpf = hp->maxwpf;
  maxhmmpf  = hp->maxhmmpf;


  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;

  vithist_prune (vh, dict, frmno, maxwpf, maxhistpf, 
		 s->beam->word_thres-s->beam->bestwordscore);

  srch_utt_word_trans (s, frmno);
  return SRCH_SUCCESS;
}

int srch_TST_frame_windup(void *srch,int32 frmno)
{
  vithist_t *vh;
  kbcore_t *kbcore;
  int32 i;

  srch_t* s;
  srch_TST_graph_t* tstg;

  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;

  vh = s->vithist;

  /* Wind up this frame */
  vithist_frame_windup (vh, frmno, NULL, kbcore);
    

  for (i = 0; i < tstg->n_lextree; i++) {
    lextree_active_swap (tstg->curugtree[i]);
    lextree_active_swap (tstg->fillertree[i]);
  }
  return SRCH_SUCCESS;
}

int srch_TST_shift_one_cache_frame(void *srch,int32 win_efv)
{
  ascr_t *ascr;
  srch_t* s;

  s=(srch_t*) srch;

  ascr=s->ascr;

  ascr_shift_one_cache_frame(ascr,win_efv);
  return SRCH_SUCCESS;
}

int srch_TST_select_active_gmm(void *srch)
{

  ascr_t *ascr;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  srch_t* s;
  srch_TST_graph_t* tstg;
  dict2pid_t *d2p;
  mdef_t *mdef;
  fast_gmm_t *fgmm;
  lextree_t *lextree;
  pl_t *pl;
  stat_t *st;
  mgau_model_t *mgau;

  kbcore_t *kbcore;
  int32 i;

  s=(srch_t*) srch;
  tstg=(srch_TST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;

  ascr=s->ascr;
  n_ltree   = tstg->n_lextree;

  mdef = kbcore_mdef (kbcore);
  d2p = kbcore_dict2pid (kbcore);
  mgau = kbcore_mgau (kbcore);
  pl = s->pl;
  fgmm = s->fastgmm;
  st = s->stat;

  if (ascr->sen_active) {
    /*    E_INFO("Decide whether senone is active\n");*/

    memset (ascr->ssid_active, 0, mdef_n_sseq(mdef) * sizeof(int32));
    memset (ascr->comssid_active, 0, dict2pid_n_comsseq(d2p) * sizeof(int32));
    /* Find active senone-sequence IDs (including composite ones) */
    for (i = 0; i < (n_ltree <<1); i++) {
      lextree = (i < n_ltree) ? tstg->curugtree[i] :
	tstg->fillertree[i - n_ltree];
      lextree_ssid_active (lextree, ascr->ssid_active, ascr->comssid_active);
    }
    
    /* Find active senones from active senone-sequences */
    memset (ascr->sen_active, 0, mdef_n_sen(mdef) * sizeof(int32));
    mdef_sseq2sen_active (mdef, ascr->ssid_active, ascr->sen_active);
    
    /* Add in senones needed for active composite senone-sequences */
    dict2pid_comsseq2sen_active (d2p, mdef, ascr->comssid_active, ascr->sen_active);
  }

  return SRCH_SUCCESS;
}



