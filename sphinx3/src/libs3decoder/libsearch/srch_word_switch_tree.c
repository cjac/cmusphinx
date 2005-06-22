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

/* srch_word_switch_tree.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2005/06/22  02:45:52  arthchan2003
 * Log. Implementation of word-switching tree. Currently only work for a
 * very small test case and it's deliberately fend-off from users. Detail
 * omitted.
 * 
 * Revision 1.16  2005/06/20 22:20:18  archan
 * Fix non-conforming problems for Windows plot.
 *
 * Revision 1.15  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.14  2005/06/11 06:58:35  archan
 * In both mode 4 and mode 5 searches, fixed a serious bug that cause the beam computation screwed up.
 *
 * Revision 1.13  2005/06/03 05:22:52  archan
 * Commented variables after refactor both mode 4 and mode 5 search to use reg_result_dump.
 *
 * Revision 1.12  2005/05/27 01:15:45  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.10  2005/05/03 06:57:44  archan
 * Finally. The word switching tree code is completed. Of course, the reporting routine largely duplicate with time switching tree code.  Also, there has to be some bugs in the filler treatment.  But, hey! These stuffs we can work on it.
 *
 * Revision 1.9  2005/05/03 04:09:10  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.8  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. Word condition tree search. Aka lexical tree copies. 
 */

#include "srch.h"
#include "approx_cont_mgau.h"

#define REPORT_SRCH_WST 1 
#define HARDCODE_FACTOR 3
#define DEFAULT_STR_LENGTH 100
#define DEFAULT_HASH_SIZE 1000 /* Just a guess, if there is 50 word
				  ends and there are 10 phones for
				  each, then 1000 is approximately
				  double that amount */


int32 srch_WST_init(kb_t* kb, void *srch)
{
  kbcore_t* kbc;
  srch_WST_graph_t* wstg ;
  srch_t* s;
  int32 i;

  kbc=kb->kbcore;
  s=(srch_t *)srch;
  wstg=(srch_WST_graph_t*)ckd_calloc(1,sizeof(srch_WST_graph_t));

  /* Only initialized one + n_static copies of trees in the initialization. */
  if(cmd_ln_int32("-epl"))
    E_WARN("-epl is omitted in WST search.\n");
  if(cmd_ln_int32("-Nlextree"))
    E_WARN("-Nlextree is omitted in WST search.\n");

  wstg->n_static_lextree=cmd_ln_int32("-Nstalextree");
  wstg->isLMLA=cmd_ln_int32("-treeugprob");

  /* CHECK: Issues warnings if number of lextree is in its boundary value.  */

  if(wstg->n_static_lextree == 0)
    E_WARN("# of static lexical trees instantiated is zero. \n");

  if(wstg->n_static_lextree == 0)
    E_WARN("number lexical tree are instantiated to be zero. This is equivalent to single lexical tree copies. \n");

  if(wstg->n_static_lextree <0 ){
    E_FATAL("# of  lexical tree is zero. \n");
  }
  
  E_INFO("Number of LM %d\n",kbc->lmset->n_lm);
  wstg->roottree=(lextree_t **) ckd_calloc (kbc->lmset->n_lm, sizeof(lextree_t *));

  /*expandtree will be used by all LMs. This will reduce the amount of
    resource switching we need to take care of.  */

  wstg->expandtree=(lextree_t **) ckd_calloc (wstg->n_static_lextree, sizeof(lextree_t *));

  for(i=0;i<kbc->lmset->n_lm ; i++){
    wstg->roottree[i]=lextree_init(kbc,kbc->lmset->lmarray[i],kbc->lmset->lmarray[i]->name,
				   wstg->isLMLA,!REPORT_SRCH_WST);
    if(wstg->roottree[i]==NULL){
      E_INFO("Fail to allocate lexical tree for lm %d \n",i);
      return SRCH_FAILURE;
    }

    if(!REPORT_SRCH_WST){
      E_INFO("Root Lextrees (%d) for lm %d, its name is %s, it has %d nodes(ug)\n",
	     i, i, kbc->lmset->lmarray[i]->name,lextree_n_node(wstg->roottree[i]));
    }
  }

  /*
    HACK!!!!This is a temporary hack that only use the first LM in taking care
    of the expanded trees. When LM is switched, there will be an obvious need
    to dynamically reallocated all the LM. 
  */

  for(i=0;i<wstg->n_static_lextree;i++){

    wstg->expandtree[i]=lextree_init(kbc,kbc->lmset->lmarray[0],kbc->lmset->lmarray[0]->name,
				   wstg->isLMLA,!REPORT_SRCH_WST);

    if(wstg->expandtree[i]==NULL){
      E_INFO("Fail to allocate lexical tree for lm %d \n",i);
      return SRCH_FAILURE;
    }

    if(!REPORT_SRCH_WST){
      E_INFO("Expanded Lextrees (%d) for lm, its name is %s, it has %d nodes(ug)\n",
	     i, kbc->lmset->lmarray[0]->name,lextree_n_node(wstg->expandtree[i]));
    }

  }

  /* By default set the current root tree to the first rootree. */
  wstg->curroottree=wstg->roottree[0];

  if((wstg->fillertree = fillertree_init(kbc))==NULL){
    E_INFO("Fail to allocate filler tree\n",i);
  }

  if(REPORT_SRCH_WST){
    E_INFO("Lextrees(%d), %d nodes(filler)\n",
	   i, 
	   lextree_n_node(wstg->fillertree));    
  }
    
  if (cmd_ln_int32("-lextreedump")) {
    for(i=0;i<kbc->lmset->n_lm;i++){
      fprintf (stderr, "LM %d name %s UGTREE %d\n",i,kbc->lmset->lmarray[i]->name,i);
      lextree_dump (wstg->roottree[i], kbc->dict, stderr);
    }
    fprintf (stderr, "FILLERTREE %d\n", i);
    lextree_dump (wstg->fillertree, kbc->dict, stderr);
  }


  wstg->histprune=histprune_init(cmd_ln_int32("-maxhmmpf"),
				 cmd_ln_int32("-maxhistpf"),
				 cmd_ln_int32("-maxwpf"),
				 cmd_ln_int32("-hmmhistbinsize"),
				 (wstg->curroottree->n_node + wstg->fillertree->n_node) * HARDCODE_FACTOR
				 );

  wstg->active_word=hash_new(wstg->n_static_lextree,1);

  /* Glue the graph structure */
  s->grh->graph_struct=wstg;
  s->grh->graph_type=GRAPH_STRUCT_WST;

  return SRCH_SUCCESS;
}

int32 srch_WST_uninit(void *srch)
{
  srch_WST_graph_t* wstg ;
  srch_t* s;
  s=(srch_t *)srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;

  hash_free(wstg->active_word);
  return SRCH_SUCCESS;
}

int32 srch_WST_begin(void *srch)
{
  kbcore_t *kbc;
  int32 n, pred;
  int32 i;
  mgau_model_t *g;
  srch_t* s;
  srch_WST_graph_t* wstg;

  s=(srch_t*) srch;
  assert(s);
  assert(s->op_mode==5);
  assert(s->grh);
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  assert(wstg);

  kbc=s->kbc;
  g=kbc->mgau;

  stat_clear_utt(s->stat);
  /* Insert initial <s> into vithist structure */
  pred = vithist_utt_begin (s->vithist, kbc);
  assert (pred == 0);	/* Vithist entry ID for <s> */
  
  /* This reinitialize the cont_mgau routine in a GMM.  */
  for(i=0;i<g->n_mgau;i++){
    g->mgau[i].bstidx=NO_BSTIDX;
    g->mgau[i].updatetime=NOT_UPDATED;
  }

  n = lextree_n_next_active(wstg->curroottree);


  /* Enter into the root tree curroottree */

  /*  E_INFO("Value of n:%d\n", lextree_n_next_active(wstg->curroottree));    */
  assert (n == 0);
  lextree_enter (wstg->curroottree, mdef_silphone(kbc->mdef), -1, 0, pred,
		 s->beam->hmm);

  lextree_active_swap (wstg->curroottree);

  /*E_INFO("Value of n:%d\n", lextree_n_next_active(wstg->curroottree)); */
  /* Enter into filler lextree */
  n = lextree_n_next_active(wstg->fillertree);
  assert (n == 0);
  lextree_enter (wstg->fillertree, BAD_S3CIPID, -1, 0, pred, s->beam->hmm);
  lextree_active_swap (wstg->fillertree);

  return SRCH_SUCCESS;
}

int32 srch_WST_end(void *srch)
{
  int32 id;
  /*  int32 ascr, lscr;*/
  /*  glist_t hyp;*/
  /*gnode_t *gn; */
  /*hyp_t *h; */
  FILE *fp; 
  /**latfp, *bptfp;*/
  srch_t* s;
  srch_WST_graph_t* wstg;
  stat_t* st;
  histprune_t* hp;
  dict_t *dict;
  char* uttid;

  s=(srch_t*) srch;
  assert(s);
  assert(s->op_mode==5);
  assert(s->grh);
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  assert(wstg);

  st = s->stat;
  hp = wstg->histprune;
    
  fp = stderr;
  dict = kbcore_dict (s->kbc);
  uttid = s->uttid;

  /* This part is duplicated with TST_end */
  if ((id = vithist_utt_end (s->vithist, s->kbc)) >= 0) {
    reg_result_dump(s,id);
  } else
    E_ERROR("%s: No recognition\n\n", uttid);

  st->utt_wd_exit=vithist_n_entry(s->vithist);
  stat_report_utt(st,uttid);
  histprune_showhistbin(wstg->histprune,st->nfr,uttid);
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
  /* Wrap up the utterance reset */
  return SRCH_SUCCESS;
}

int32 srch_WST_decode(void *srch)
{
  return SRCH_SUCCESS;
}

int32 srch_WST_set_lm(void *srch, const char* lmname)
{
  return SRCH_SUCCESS;
}

int32 srch_WST_hmm_compute_lv1(void *srch)
{
  return SRCH_SUCCESS;
}

int32 srch_WST_hmm_compute_lv2(void *srch, int32 frmno)
{

  /* This is local to this codebase */

  int32 i,j ;
  lextree_t *lextree;
  srch_t* s;
  srch_WST_graph_t* wstg;
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

  i =0 ;
 
  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;

  n_ltree   = wstg->n_static_lextree;
  kbcore = s->kbc;
  hp = wstg->histprune;
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

  lextree = wstg->curroottree;
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

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandtree[i];
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
    

    lextree=wstg->curroottree;
    lextree_hmm_histbin (lextree, besthmmscr, bin, nbin, bw);

    for (i = 0; wstg->n_static_lextree; i++) {
      lextree=wstg->expandtree[i];
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

int32 srch_WST_propagate_graph_ph_lv1(void *srch)
 {
  return SRCH_SUCCESS;
}

int32 srch_WST_propagate_graph_ph_lv2(void *srch, int32 frmno)
{

  int32 i ;
  srch_t* s;
  srch_WST_graph_t* wstg;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  int32 pheurtype;     /* Local version of pheurtype, don't expect it to change */

  lextree_t *lextree;
  kbcore_t *kbcore;
  vithist_t *vh;
  pl_t *pl;

  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;
  vh   = s->vithist;
  pl = s->pl;
  pheurtype = pl->pheurtype;

  n_ltree   = wstg->n_static_lextree;

  lextree=wstg->curroottree;
  /*E_INFO("For root tree\n"); */
  lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
			s->beam->thres, 
			s->beam->phone_thres, 
			s->beam->word_thres,pl);

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandtree[i];
    /*    E_INFO("For expand tree\n");*/
    lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
				     s->beam->thres, 
				     s->beam->phone_thres, 
				     s->beam->word_thres,pl);
  }

  return SRCH_SUCCESS;
}


int32 srch_WST_rescoring(void *srch, int32 frmno)
{

  int32 i ;
  srch_t* s;
  srch_WST_graph_t* wstg;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  int32 pheurtype;     /* Local version of pheurtype, don't expect it to change */

  lextree_t *lextree;
  kbcore_t *kbcore;
  vithist_t *vh;
  pl_t *pl;

  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;
  vh   = s->vithist;
  pl = s->pl;
  pheurtype = pl->pheurtype;

  n_ltree   = wstg->n_static_lextree;

  lextree=wstg->curroottree;
  lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
			s->beam->word_thres,s->senscale);

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandtree[i];
    lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				 s->beam->word_thres,s->senscale);
  }

  return SRCH_SUCCESS;
}


int32 srch_WST_propagate_graph_wd_lv1(void *srch)
{
  return SRCH_SUCCESS;
}


/** The heart of WST implementation. 
    
 */

int32 srch_WST_propagate_graph_wd_lv2(void *srch, int32 frmno)
{
  dict_t *dict;
  vithist_t *vh;
  histprune_t *hp;
  kbcore_t *kbcore;
  lextree_t *lextree;
  mdef_t *mdef;

  srch_t* s;
  srch_WST_graph_t* wstg;
  int32 maxwpf;        /* Local version of Max words per frame, don't expect it to change */
  int32 maxhistpf;     /* Local version of Max histories per frame, don't expect it to change */
  int32 maxhmmpf;      /* Local version of Max active HMMs per frame, don't expect it to change  */
  lextree_node_t **list;
  /*  int32 id;*/
  int32 th;
  beam_t *bm;
  int32 active_word_end;

  vithist_entry_t *ve;
  int32 vhid,le,n_ci, score;
  s3wid_t wid;
  int32 p;
  char *str;
  int32 str_length;
  /*  char *wdstr, *phstr;*/
  /*
  glist_t keylist;
  gnode_t *key;
  hash_entry_t *he;
  int32 list_size;
  */

  hash_table_t *word_phone_hash;

  int32 hash_size;
  int32 hash_score;
  int32 id;


  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;

  hp = wstg->histprune;
  vh   = s->vithist;
  dict = kbcore_dict (kbcore);
  mdef = kbcore_mdef(s->kbc);
  n_ci = mdef_n_ciphone(mdef);
  maxwpf    = hp->maxwpf;
  maxhistpf = hp->maxwpf;
  maxhmmpf  = hp->maxhmmpf;
  active_word_end=0;
  

  s=(srch_t*) srch;
 
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;

  /* Look at all the word ends */
  
  lextree=wstg->curroottree;
  list=lextree->active;
  bm=s->beam;

  /*  E_INFO("Time %d\n",frmno);*/
  vithist_prune (vh, dict, frmno, maxwpf, maxhistpf, 
		 s->beam->word_thres-s->beam->bestwordscore); 
  
  vhid = vithist_first_entry (vh, frmno);
  le = vithist_n_entry (vh) - 1;

  th = bm->bestscore + bm->hmm;	/* Pruning threshold */

  /** Collect all entries with same last phone and same words. */
  /** This is slow.*/

  /** You can just USE ID , FOOL!!!*/

  str=(char*) ckd_calloc(DEFAULT_STR_LENGTH,sizeof(char));

  hash_size=0;
  word_phone_hash=hash_new(DEFAULT_HASH_SIZE,HASH_CASE_NO);


  /* At here, just delete the tree with not many active entries */

  for (; vhid <= le; vhid++) {
    ve = vithist_id2entry (vh, vhid);
    wid = vithist_entry_wid (ve);
    p = dict_last_phone (dict, wid);

    score = vithist_entry_score (ve);

    if (mdef_is_fillerphone(mdef, p))
      p = mdef_silphone(mdef);

    if (! vithist_entry_valid(ve))
      continue;
    
    /* HACK, build a hash for ((word,phone),score) pair */
    /* This is ugly. */
    /* Create an internal represetation look likes "word::::phone".  */
    
    str_length=0;
    str_length+=strlen(dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));
    str_length+=strlen(mdef_ciphone_str(mdef,p));
    str_length+=5;

    str=(char*) ckd_calloc(str_length,sizeof(char));
  
    /*    E_INFO("Entering: The word id %d word str %s, the phone %d phone str %s, the score %d\n", 
	  wid, 
	  dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),
	  p,
	  mdef_ciphone_str(mdef,p),
	  score);*/


    sprintf(str,"%s%s%s",
	    dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),
	    "::::",
	    mdef_ciphone_str(mdef,p));

    /*E_INFO("Composite String %s\n",str); */

    if(hash_lookup(word_phone_hash, str,&hash_score) <0){
      hash_size++;
      /* HACK! In general, the high scoring ones to be taken care first. */
      if(hash_size > DEFAULT_HASH_SIZE){
	/* If another copy is not available, start to allocate a copy of tree.*/
	E_FATAL("Hash size is larger than default hash size %d. \n",hash_size);
      }
      if(hash_enter(word_phone_hash,str,score) != score ){
	E_FATAL("hash_enter(table, %s) failed\n", str);
      }
    }else{
      if(hash_score < score){
	if(hash_enter(word_phone_hash,str,score) != hash_score ){
	  E_FATAL("hash_enter(table, %s) failed\n", str);
	}
      }
    }    
  }


#if 0
    keylist=hash_tolist(word_phone_hash,&list_size);
    
  for (key = keylist; key; key = gnode_next(key)) {
    he = (hash_entry_t *) gnode_ptr (key);
    E_INFO("Key %s: Value %d\n", hash_entry_key(he),hash_entry_val(he));
    /* Extract the word and phone at here */
    wdstr=strtok(hash_entry_key(he),":");
    E_INFO("String %s\n",wdstr);
    phstr=strtok(NULL,":");
    E_INFO("String %s\n",phstr);
  }
#endif
  /* This is a place one should do all tricks of cross-word triphones */
  /** All entries traverse to tree with word context*/

  vhid = vithist_first_entry (vh, frmno);
  le = vithist_n_entry (vh) - 1;

  for (; vhid <= le; vhid++) {
    ve = vithist_id2entry (vh, vhid);
    wid = vithist_entry_wid (ve);
    p = dict_last_phone (dict, wid);
    if (mdef_is_fillerphone(mdef, p))
      p = mdef_silphone(mdef);
    
    score = vithist_entry_score (ve);

    if (! vithist_entry_valid(ve))
      continue;

    /* Look up whether this word is there in the word_end hash */
    
    if(hash_lookup(wstg->active_word,
		   dict_wordstr(s->kbc->dict,
				dict_basewid(s->kbc->dict,wid)),
		   &id) <0){
      /* If it doesn't, start to use another copy */
      /* This is also the part one should apply bigram lookahead */
      
      id  = wstg->no_active_word;
      wstg->no_active_word++;

      /* In general, the high scoring ones to be taken care first. */
      if(wstg->no_active_word > wstg->n_static_lextree){
	/* If another copy is not available, start to allocate a copy of tree.*/
	E_FATAL("Number of active word is more than no of tree allocated. \n");
	E_FATAL("This bug will show up when the number of word end is too huge.\n");
      }
      
      if(hash_enter(wstg->active_word,dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),id ) != id ){
	E_FATAL("hash_enter(local-phonetable, %s) failed\n", dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));
      }

      /*E_INFO("A new tree is started for wid %d, word str %s\n", wid, 
	dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));*/
    }else{
    }

    str_length=0;
    str_length+=strlen(dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));
    str_length+=strlen(mdef_ciphone_str(mdef,p));
    str_length+=5;
    
    str=(char*) ckd_calloc(str_length,sizeof(char));
    
    sprintf(str,"%s%s%s",
	    dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),
	    "::::",
	    mdef_ciphone_str(mdef,p));

    /* No matter what, a tree would be availble at this point. It is time to enter it. */

    /*E_INFO("In the entry loop string: %s\n",str); */


    hash_lookup(word_phone_hash,
		str,
		&hash_score);

    /* E_INFO("hash_score %d , score %d\n", hash_score,score);*/
    /* HACK! This is magical */
    /* If it was entered once, it shouldn't be enter twice. This loop is quite stupid in that sense*/
    /* and here I used a very magical way to check it. This is very wrong, I need to change it back. */

    if(hash_score == score){
      /*      E_INFO("Entering: The word id %d word str %s, the phone %d phone str %s, the score %d\n", 
	wid, 
	dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),
	p,
	mdef_ciphone_str(mdef,p),
	score);*/

      lextree_enter(wstg->expandtree[id], (s3cipid_t) p, frmno, hash_score, vhid, th);
    }

  }

  /* Collect all the garbage tree if the tree is not active at all */
  {
    /*This part should free all trees that should be deallocated. Hmm. How? */
  }

  /* This part should handle filler word.
   */
  {
    lextree_enter (wstg->fillertree, BAD_S3CIPID, frmno, vh->bestscore[frmno],
		   vh->bestvh[frmno], th);

  }


  /** obtained a link-list from the hash_table keys*/
  ckd_free(word_phone_hash);
  ckd_free(str);
  
  return SRCH_SUCCESS;





#if 0 /* This part is useless */

  keylist=hash_tolist(word_phone_hash,&list_size);

  for (key = keylist; key; key = gnode_next(key)) {
    he = (hash_entry_t *) gnode_ptr (key);
    E_INFO("Key %s: Value %d\n", hash_entry_key(he),hash_entry_val(he));
    /* Extract the word and phone at here */
    wdstr=strtok(hash_entry_key(he),":");
    E_INFO("String %s\n",wdstr);
    phstr=strtok(NULL,":");
    E_INFO("String %s\n",phstr);

    if(hash_lookup(wstg->active_word,
		   dict_wordstr(s->kbc->dict,
				dict_basewid(s->kbc->dict,wid)),
		   &id) <0){
      id  = wstg->no_active_word;
      wstg->no_active_word++;

      /* In general, the high scoring ones to be taken care first. */
      if(wstg->no_active_word > wstg->n_static_lextree){
	/* If another copy is not available, start to allocate a copy of tree.*/
	E_FATAL("Number of active word is more than no of tree allocated. \n");
	E_FATAL("This bug will show up when the number of word end is too huge.\n");
      }
      if(hash_enter(wstg->active_word,wdstr,id ) != id ){
	E_FATAL("hash_enter(local-phonetable, %s) failed\n", dict_wordstr(s->kbc->dict,wdstr);
      }

      E_INFO("A new tree is started for word str %s\n", wid, 
	     dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));
    }

    E_INFO("Entering: The word id %d word str %s, the phone %d phone str %s, the score %d\n", 
	   wid, 
	   dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),
	   p,
	   mdef_ciphone_str(mdef,p),
	   score);

    lextree_enter(wstg->expandtree[id], (s3cipid_t) p, frmno, score, vhid, th);

  }
#endif 


}

int srch_WST_eval_beams_lv1 (void* srch_struct)
{
  return SRCH_SUCCESS;
}
int srch_WST_eval_beams_lv2 (void* srch_struct)
{
  return SRCH_SUCCESS;
}

int srch_WST_compute_heuristic(void *srch, int32 win_efv)
{
  srch_t* s;
  srch_WST_graph_t* wstg;
  pl_t *pl;
  ascr_t *ascr;
  mdef_t *mdef;

  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  mdef = kbcore_mdef (s->kbc);

  ascr=s->ascr;
  pl = s->pl;

  if(pl->pheurtype!=0) 
    pl_computePhnHeur(mdef, ascr, pl, pl->pheurtype, s->cache_win_strt,win_efv);

  return SRCH_SUCCESS;
}
int srch_WST_frame_windup(void *srch,int32 frmno)
{
  vithist_t *vh;
  kbcore_t *kbcore;

  int32 i;
  srch_t* s;
  srch_WST_graph_t* wstg;

  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;

  vh = s->vithist;

  /* Wind up this frame */
  vithist_frame_windup (vh, frmno, NULL, kbcore);

  /* Swap the data for all trees in this frame. That include the curroottree and all expandtrees */

  lextree_active_swap(wstg->curroottree);
  for (i = 0; i < wstg->n_static_lextree; i++) 
    lextree_active_swap (wstg->expandtree[i]);
   
  return SRCH_SUCCESS;
}

int srch_WST_shift_one_cache_frame(void *srch,int32 win_efv)
{
  ascr_t *ascr;
  srch_t* s;
  srch_WST_graph_t* wstg;

  s=(srch_t*) srch;

  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  ascr=s->ascr;

  ascr_shift_one_cache_frame(ascr,win_efv);
  return SRCH_SUCCESS;
}

int srch_WST_select_active_gmm(void *srch)
{
  ascr_t *ascr;
  int32 n_ltree;       /* Local version of number of lexical trees used */
  srch_t* s;
  srch_WST_graph_t* wstg;
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
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;

  ascr=s->ascr;

  n_ltree=wstg->n_static_lextree;

  kbcore = s->kbc;
  pl = s->pl;
  fgmm = s->fastgmm;
  st = s->stat;
  mdef = kbcore_mdef (kbcore);
  d2p = kbcore_dict2pid (kbcore);
  mgau = kbcore_mgau (kbcore);
  
  /* Find active senones and composite senones, from active lextree nodes */
  /* The active senones will also be changed in approx_cont_mgau_frame_eval */

  if(ascr->sen_active){
    /*    E_INFO("Decide whether senone is active\n");*/
    /* This part is not completed. */
    memset (ascr->ssid_active, 0, mdef_n_sseq(mdef) * sizeof(int32));
    memset (ascr->comssid_active, 0, dict2pid_n_comsseq(d2p) * sizeof(int32));

    /*The root tree */
    lextree=wstg->curroottree;
    if(0) E_INFO("In the root tree\n");
    lextree_ssid_active (lextree, ascr->ssid_active, ascr->comssid_active);

    /*The word tree copies */
    for(i = 0 ; i < n_ltree; i++){
      if(0) E_INFO("In the expand tree %d\n",i);
      lextree=wstg->expandtree[i];
      lextree_ssid_active (lextree, ascr->ssid_active, ascr->comssid_active);
    }

    memset (ascr->sen_active, 0, mdef_n_sen(mdef) * sizeof(int32));
    mdef_sseq2sen_active (mdef, ascr->ssid_active, ascr->sen_active);

    /* Add in senones needed for active composite senone-sequences */
    dict2pid_comsseq2sen_active (d2p, mdef, ascr->comssid_active, ascr->sen_active);
  }

  return SRCH_SUCCESS;
}




