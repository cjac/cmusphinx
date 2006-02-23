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
 * Revision 1.2  2006/02/23  16:52:56  arthchan2003
 * Merged from branch: SPHINX3_5_2_RCI_IRII_BRANCH: this fills in the code for doing tree propagation.  However, rescoring is still being used.  The code is working. However, it takes up huge amount of memory and I consider this as not elegant.   It also shows that straight forward implementation of tree copies search doesn't work even in these days.
 * 
 * Revision 1.1.4.18  2006/01/17 20:28:48  arthchan2003
 * Uncomment the result dump code in mode 5.
 *
 * Revision 1.1.4.17  2006/01/16 20:15:37  arthchan2003
 * 1, removed the unlinksilences part, 2, added 2nd-stage interface, but now commented.
 *
 * Revision 1.1.4.16  2005/11/17 06:43:25  arthchan2003
 * Removed senone scale in lextree_hmm_propagate.
 *
 * Revision 1.1.4.15  2005/10/09 20:00:45  arthchan2003
 * Added back match file logging in mode 3. Safe-guard the code from using LM switching in mode 3 and mode 5.
 *
 * Revision 1.1.4.14  2005/10/07 20:04:50  arthchan2003
 * When rescoring in full triphone expansion, the code should use the score for the word end with corret right context.
 *
 * Revision 1.1.4.13  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.1.4.12  2005/09/18 01:46:20  arthchan2003
 * Moved unlinkSilences to mode 4 and mode 5 search-specific implementation.
 *
 * Revision 1.1.4.11  2005/09/11 03:01:01  arthchan2003
 * Bug fix on the size of hmmpf and histpf
 *
 * Revision 1.1.4.10  2005/08/03 18:54:32  dhdfu
 * Fix the support for multi-stream / semi-continuous models.  It is
 * still kind of a hack, but it now works.
 *
 * Revision 1.1.4.9  2005/07/24 01:41:52  arthchan2003
 * Use ascr provided clearing function instead of directly clearing the array.
 *
 * Revision 1.1.4.8  2005/07/07 02:41:55  arthchan2003
 * 1, Added an experimental version of tree expansion interface it the code, it does tree expansion without history pruning. Currently disabled because it used to much memory space srch_word_switch_tree.[ch].  2, Remove -lminsearch segments of code, it proves to be unnecessary. 3, Remove the rescoring interface.  In this search, WST_rescoring is actually not doing rescoring, it is rather a segment of code which collect all active word end together and input it into the viterbi history.
 *
 * Revision 1.1.4.7  2005/07/04 07:23:40  arthchan2003
 * 1, Bug fix, put score instead of vh->bestscore[frmno] when entering filler tree, 2, Do poor-man trigram rescoring when entering tree.
 *
 * Revision 1.1.4.6  2005/07/03 23:18:05  arthchan2003
 * 1, correct freeing of the structure is implemented. 2, Revamp srch_WST_propagate_graph_wd_lv2 implementation. The previous implementation contains several hacks which are very hard to reconcile by further hacking.   That includes a, usage of the exact score to indicate whether a tree should be entered. (too magical) b, memory leaks in a rate of 1MB per utterance (no kidding me) c, The code unnecessarily required two pass of all the hypothesis histories. (50 unnecessary lines).  The current fix is much simpler than before.  Instead of trying to keep track of which (word, phone) pair has the highest score using the word_phone_hash hash structure.  We just let every (word, phone) pair to enter the tree and allow lextree_enter to decide whether the history should stay.   This is still not very clever in the sense that the (word, phone) max score is actually the only path it will be propagated in future.  The current implementation will thus make repeated traversal of child nodes for same (word,phone) pair.  However, this is a clean way to do things in the srch_impl layer.  Later, it will be necessary to think of a better way to keep track of max score instead.
 *
 * Revision 1.1.4.5  2005/07/01 04:14:09  arthchan2003
 * Fixes of making histogram pruning working with the WST search (mode 5) 1, As in the TST search, the bins are always zeroed. 2, When the number of bins were initialized, instead of using 3 times the number of nodes, now using n_stalextree number of node.  The histogram bin array are also updated for every utterance. This will ensure when the number of active node is too large, there are invalid write.  3, the code is further safe-guarded in srch_WST_hmm_compute_lv2.  change 2 will not give us assurance if the number of active nodes are too large and spill out of the array.  The code with long comment will make sure that this condition will not happen.
 *
 * Revision 1.1.4.4  2005/06/28 19:11:49  arthchan2003
 * Fix Bugs. When new tree is allocated, tree indices were not inserted in the empty tree list. Now is fixed.
 *
 * Revision 1.1.4.3  2005/06/28 07:05:40  arthchan2003
 * 1, Set lmset correctly in srch_word_switch_tree. 2, Fixed a bug hmm_hist_binsize so that histogram pruning could be used in mode 5. 3, (Most important) When number of allocated tree is smaller than number of tree required, the search dynamically allocated more tree until all memory are used up. This should probably help us to go through most scenarios below 3k vocabulary.
 *
 * Revision 1.1.4.2  2005/06/27 05:37:05  arthchan2003
 * Incorporated several fixes to the search. 1, If a tree is empty, it will be removed and put back to the pool of tree, so number of trees will not be always increasing.  2, In the previous search, the answer is always "STOP P I T G S B U R G H </s>"and filler words never occurred in the search.  The reason is very simple, fillers was not properly propagated in the search at all <**exculamation**>  This version fixed this problem.  The current search will give <sil> P I T T S B U R G H </sil> </s> to me.  This I think it looks much better now.
 *
 * Revision 1.1.4.1  2005/06/24 21:13:52  arthchan2003
 * 1, Turn on mode 5 again, 2, fixed srch_WST_end, 3, Add empty function implementations of add_lm and delete_lm in mode 5. This will make srch.c checking happy.
 *
 * Revision 1.1  2005/06/22 02:45:52  arthchan2003
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
#define NUM_TREE_ALLOC 5



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
  wstg->expandfillertree=(lextree_t **) ckd_calloc (wstg->n_static_lextree, sizeof(lextree_t *));

  if(kbc->lmset->n_lm>1){
    E_FATAL("Multiple lm doesn't work for this mode 5 yet\n");
  }
  for(i=0;i<kbc->lmset->n_lm ; i++){
    wstg->roottree[i]=lextree_init(kbc,kbc->lmset->lmarray[i],kbc->lmset->lmarray[i]->name,
				   wstg->isLMLA,!REPORT_SRCH_WST,LEXTREE_TYPE_UNIGRAM);
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

  /* By default set the current root tree to the first rootree. */
  /* The first word and filler tree */

  wstg->curroottree=wstg->roottree[0];
  wstg->curfillertree = fillertree_init(kbc);
  if(wstg->curfillertree==NULL){
      E_INFO("Fail to allocate filler tree for lm %d \n",i);
  }

  /* The expanded tree */
  for(i=0;i<wstg->n_static_lextree;i++){
    wstg->expandtree[i]=lextree_init(kbc,kbc->lmset->lmarray[0],kbc->lmset->lmarray[0]->name,
				     wstg->isLMLA,!REPORT_SRCH_WST,LEXTREE_TYPE_BIGRAM);

    if(wstg->expandtree[i]==NULL){
      E_INFO("Fail to allocate lexical tree %d for lm %d \n",i, 0);
      return SRCH_FAILURE;
    }

    if(!REPORT_SRCH_WST){
      E_INFO("Expanded Lextrees (%d) for lm, its name is %s, it has %d nodes(ug)\n",
	     i, kbc->lmset->lmarray[0]->name,lextree_n_node(wstg->expandtree[i]));
    }

    wstg->expandfillertree[i]=fillertree_init(kbc);
    if(wstg->expandfillertree[i]==NULL){
      E_INFO("Fail to allocate filler tree %d for lm %d \n",i,0);
      return SRCH_FAILURE;
    }

  }



    
  if (cmd_ln_int32("-lextreedump")) {
    for(i=0;i<kbc->lmset->n_lm;i++){
      fprintf (stderr, "LM %d name %s UGTREE %d\n",i,kbc->lmset->lmarray[i]->name,i);
      lextree_dump (wstg->roottree[i], kbc->dict, kbc->mdef, stderr, cmd_ln_int32("-lextreedump"));
      /*
      fprintf (stderr, "FILLERTREE %d\n", i);
      lextree_dump (wstg->fillertree[i], kbc->dict, kbc->mdef, stderr, cmd_ln_int32("-lextreedump"));
      */

    }

  }


  /* Using wstg->n_static_lextree to decide the number of nodes is still not a very safe measure
   */
  wstg->histprune=histprune_init(cmd_ln_int32("-maxhmmpf"),
				 cmd_ln_int32("-maxhistpf"),
				 cmd_ln_int32("-maxwpf"),
				 cmd_ln_int32("-hmmhistbinsize"),
				 (wstg->curroottree->n_node + wstg->curfillertree->n_node) * wstg->n_static_lextree
				 );

  


  wstg->active_word=hash_new(wstg->n_static_lextree,1);

  /* Glue the graph structure */
  s->grh->graph_struct=wstg;
  s->grh->graph_type=GRAPH_STRUCT_WST;

  wstg->lmset=kbc->lmset;


  /**/
  return SRCH_SUCCESS;
}

int32 srch_WST_uninit(void *srch)
{
  srch_WST_graph_t* wstg ;
  srch_t* s;
  int i;
  s=(srch_t *)srch;

  wstg=(srch_WST_graph_t*) s->grh->graph_struct;

  if(wstg->active_word){
    hash_free(wstg->active_word);
  }

  if(wstg->histprune!=NULL){
    histprune_free((void*) wstg->histprune);
  }

  lextree_free(wstg->curroottree);
  lextree_free(wstg->curfillertree);
  
  for(i=0;i<wstg->n_static_lextree;i++){
    lextree_free(wstg->expandtree[i]);
    lextree_free(wstg->expandfillertree[i]);
  }

  ckd_free(wstg->expandtree);
  ckd_free(wstg->expandfillertree);

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
  glist_t keylist;

  s=(srch_t*) srch;
  assert(s);
  assert(s->op_mode==5);
  assert(s->grh);
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  assert(wstg);

  kbc=s->kbc;
  g=kbc->mgau;

  stat_clear_utt(s->stat);

  histprune_update_histbinsize(wstg->histprune,
			       wstg->histprune->hmm_hist_binsize,  
			       (wstg->curroottree->n_node + wstg->curfillertree->n_node) * wstg->n_static_lextree);

  histprune_zero_histbin(wstg->histprune);

  /* Insert initial <s> into vithist structure */
  pred = vithist_utt_begin (s->vithist, kbc);
  assert (pred == 0);	/* Vithist entry ID for <s> */
  
  /* This reinitialize the cont_mgau routine in a GMM.  */
  if (g){
    for(i=0;i<g->n_mgau;i++){
      g->mgau[i].bstidx=NO_BSTIDX;
      g->mgau[i].updatetime=NOT_UPDATED;
    }
  }

  n = lextree_n_next_active(wstg->curroottree);


  /* Enter into the root tree curroottree */

  /*  E_INFO("Value of n:%d\n", lextree_n_next_active(wstg->curroottree));    */
  assert (n == 0);
  lextree_enter (wstg->curroottree, mdef_silphone(kbc->mdef), -1, 0, pred,s->beam->hmm,s->kbc);

  lextree_active_swap (wstg->curroottree);

  /*E_INFO("Value of n:%d\n", lextree_n_next_active(wstg->curroottree)); */
  /* Enter into filler lextree */
  n = lextree_n_next_active(wstg->curfillertree);
  assert (n == 0);
  lextree_enter (wstg->curfillertree, BAD_S3CIPID, -1, 0, pred, s->beam->hmm, s->kbc);
  lextree_active_swap (wstg->curfillertree);


  keylist=hash_tolist(wstg->active_word,&(wstg->no_active_word));
  assert(glist_count(keylist)==0);

  /*  assert(glist_count(wstg->empty_tree_idx_stack)==0);*/
  /* Initialize the stack of empty tree index */
  for(i=wstg->n_static_lextree-1;i>=0;i--){
    wstg->empty_tree_idx_stack=glist_add_int32(wstg->empty_tree_idx_stack,i);
  }

  /*  E_FATAL ("Forced stop\n");*/
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
  int32 i;
  glist_t keylist;
  gnode_t *key;
  hash_entry_t *he;
  int32 val;


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

#if 1
  if ((id = vithist_utt_end (s->vithist, s->kbc)) >= 0) {
    reg_result_dump(s,id);
  } else
    E_ERROR("%s: No recognition\n\n", uttid);
#endif

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

  lextree_utt_end(wstg->curroottree,s->kbc);
  lextree_utt_end(wstg->curfillertree,s->kbc);
  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree_utt_end(wstg->expandtree[i], s->kbc);
    lextree_utt_end(wstg->expandfillertree[i],s->kbc);
  }
    
  vithist_utt_reset (s->vithist);
  
  lm_cache_stats_dump (kbcore_lm(s->kbc));
  lm_cache_reset (kbcore_lm(s->kbc));
  
  /* Wrap up the utterance reset */

  /* Also empty the active_word list*/
  keylist=hash_tolist(wstg->active_word,&(wstg->no_active_word));

  for (key = keylist; key; key = gnode_next(key)) {
    he = (hash_entry_t *) gnode_ptr (key);
    hash_lookup (wstg->active_word, (char* )hash_entry_key(he),&val);

    /*    printf("From hash table %s From expandtree %s\n", (char* )hash_entry_key(he), wstg->expandtree[val]->prev_word );*/
    assert(hash_delete(wstg->active_word,(char* )hash_entry_key(he))==HASH_OP_SUCCESS);
  }


  if(id>=0)
    return SRCH_SUCCESS;
  else
    return SRCH_FAILURE;
}

int srch_WST_set_lm(void* srch_struct, const char* lmname)
{
  E_INFO("In Mode 5, currently the function set LM is not supported\n");
  return SRCH_FAILURE;
}
int srch_WST_add_lm(void* srch, lm_t *lm, const char *lmname)
{
  E_INFO("In Mode 5, currently the function add LM is not supported\n");
  return SRCH_FAILURE;

}
int srch_WST_delete_lm(void* srch, const char *lmname)
{  
  E_INFO("In Mode 5, currently the function delete LM is not supported\n");
  return SRCH_FAILURE;
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
  maxhistpf = hp->maxhistpf;
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
    fprintf (s->hmmdumpfp, "Fr %d Lextree #HMM %d\n", frmno,
	     lextree->n_active);

  lextree_hmm_eval (lextree, kbcore, ascr, frmno, s->hmmdumpfp);
  if (besthmmscr < lextree->best)
    besthmmscr = lextree->best;
  if (bestwordscr < lextree->wbest)
    bestwordscr = lextree->wbest;

  st->utt_hmm_eval += lextree->n_active;
  frm_nhmm += lextree->n_active;


  lextree = wstg->curfillertree;
  if (s->hmmdumpfp != NULL)
    fprintf (s->hmmdumpfp, "Fr %d Current Filler Tree %d #HMM\n", frmno, 
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

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandfillertree[i];
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

  numhistbins = hp->hmm_hist_bins;


  /* ARCHAN: 20050701

     HACK!  This is not a great hack. But it is what I tried to avoid,
     in WST search, there might be situation where
     frm_nhmm/histbinsize is larger than hp->hmm_hist_bins and causing
     hmm_hist[frm_nhmm/histbinsize]++ to do invalid memory write.
     This is caused by the fact WST search will dynamic increase the
     number of trees and that will in-turn affect the size of
     hmm_hist. What one could do is to allow update of histogram
     binsize within the tree allocation routine.  However, within
     decoding of one utterance, that might cause nasty problem of
     recomputing the value of each bins. Therefore, I tried to avoid
     it. Instead, for that particular utterance where memory is
     allocated, every update which is larger than hp->hmm_hist_bins
     are now put in hp->hmm_hist_bins.  This will ensure the code to be
     safe. 
  */
  if(frm_nhmm/histbinsize > hp->hmm_hist_bins-1)
    hmm_hist[hp->hmm_hist_bins-1]++;
  else
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

    lextree=wstg->curfillertree;
    lextree_hmm_histbin (lextree, besthmmscr, bin, nbin, bw);


    for (i = 0; i< wstg->n_static_lextree; i++) {
      lextree=wstg->expandtree[i];
      lextree_hmm_histbin (lextree, besthmmscr, bin, nbin, bw);
    }

    for (i = 0; i< wstg->n_static_lextree; i++) {
      lextree=wstg->expandfillertree[i];
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

  if(lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
				      s->beam->thres, 
				      s->beam->phone_thres, 
				      s->beam->word_thres,pl)!=LEXTREE_OPERATION_SUCCESS){
    E_ERROR("Propagation Failed for lextree_hmm_propagate_non_leave at the root tree \n");
    lextree_utt_end(lextree,kbcore);
    return SRCH_FAILURE;
  }

  lextree=wstg->curfillertree;
  if(lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
				      s->beam->thres, 
				      s->beam->phone_thres, 
				      s->beam->word_thres,pl)!=LEXTREE_OPERATION_SUCCESS){
    E_ERROR("Propagation Failed for lextree_hmm_propagate_non_leave at the root filler tree \n");
    lextree_utt_end(lextree,kbcore);
    return SRCH_FAILURE;
  }



  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandtree[i];
    /*    E_INFO("For expand tree\n");*/
    if(lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
					s->beam->thres, 
					s->beam->phone_thres, 
					s->beam->word_thres,pl)!=LEXTREE_OPERATION_SUCCESS){
      E_ERROR("Propagation Failed for lextree_hmm_propagate_non_leave at the lexical tree %d\n",i);
      lextree_utt_end(lextree,kbcore);
      return SRCH_FAILURE;
    }
  }

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandfillertree[i];
    /*    E_INFO("For expand tree\n");*/
    
    if(lextree_hmm_propagate_non_leaves(lextree, kbcore, frmno,
					s->beam->thres, 
					s->beam->phone_thres, 
					s->beam->word_thres,pl)!=LEXTREE_OPERATION_SUCCESS){
      E_ERROR("Propagation Failed for lextree_hmm_propagate_non_leave at the filler tree %d\n",i);
      lextree_utt_end(lextree,kbcore);
      return SRCH_FAILURE;
    }
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
  if(lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				  s->beam->word_thres)!=LEXTREE_OPERATION_SUCCESS){
    lextree_utt_end(lextree,kbcore);
    return SRCH_FAILURE;
  }
  lextree=wstg->curfillertree;
  if(lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				  s->beam->word_thres)!=LEXTREE_OPERATION_SUCCESS){
    lextree_utt_end(lextree,kbcore);
    return SRCH_FAILURE;
  }

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree = wstg->expandtree[i];
    if(lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				    s->beam->word_thres)!=LEXTREE_OPERATION_SUCCESS){
      lextree_utt_end(lextree,kbcore);
      return SRCH_FAILURE;
    }

    lextree = wstg->expandfillertree[i];
    if(lextree_hmm_propagate_leaves(lextree, kbcore, vh, frmno,
				    s->beam->word_thres)!=LEXTREE_OPERATION_SUCCESS){
      lextree_utt_end(lextree,kbcore);
      return SRCH_FAILURE;
    }

  }



  return SRCH_SUCCESS;
}


int32 srch_WST_propagate_graph_wd_lv1(void *srch)
{

  return SRCH_SUCCESS;
}

/* This function is not used because it could be very slow with history pruning. */
int32 srch_WST_hmm_propagate_leaves (srch_t* s, lextree_t *lextree, vithist_t *vh,
			    int32 cur_frm, int32 wth)
{
    lextree_node_t **list, *ln;
    hmm_t *hmm;
    mdef_t *mdef;
    int32 i;
    int32 id;
    srch_WST_graph_t* wstg;
    vithist_entry_t *tve;
    int32 p;
    int32 entry;

    wstg=(srch_WST_graph_t*) s->grh->graph_struct;

    list = lextree->active;
    mdef = kbcore_mdef(s->kbc);

    for (i = 0; i < lextree->n_active; i++) {
	ln = list[i];
	hmm = &(ln->hmm);
	
	if (! NOT_S3WID(ln->wid)) {    /* Leaf node; word exit */
	    if (hmm->out.score < wth)
		continue;		/* Word exit score not good enough */

	    if(hmm->out.history==-1)
	      E_ERROR("Hmm->out.history equals to -1 with score %d and active idx %d, lextree->type\n",hmm->out.score,i,lextree->type);

	    /* From now on, we are taking care of all active word ends. */

	    /* This big if just takes care of logistic of allocating new trees. 
	       1, Trees are allocated and initialized. 
	       2, Also add these trees to the pool of empty tree indices. 
	       3, And indicate the trees are now available in the hash table 
	     */ 

	    if(hash_lookup(wstg->active_word,
			   dict_wordstr(s->kbc->dict,
					dict_basewid(s->kbc->dict,ln->wid)),
			   &id) <0){
	      /* If it doesn't, start to use another copy */
	      /* This is also the part one should apply bigram lookahead */
	      
	      /* Pop one indices from the stack*/
	      id = gnode_int32(wstg->empty_tree_idx_stack);
	      wstg->empty_tree_idx_stack=glist_delete(wstg->empty_tree_idx_stack);

	      /*      assert(wstg->expandtree[id]->prev_word==NULL);*/

	      strcpy(wstg->expandtree[id]->prev_word,dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,ln->wid)));
	      wstg->no_active_word++;

	      /* In general, the high scoring ones to be taken care first. */
	      if(wstg->no_active_word == wstg->n_static_lextree){
		/* If another copy is not available, start to allocate a copy of tree.*/
		E_INFO("Allocate another %d more tree(s)\n", NUM_TREE_ALLOC);
		
		wstg->expandtree      =(lextree_t **) ckd_realloc(wstg->expandtree,(wstg->n_static_lextree+NUM_TREE_ALLOC)*sizeof(lextree_t*));
		wstg->expandfillertree=(lextree_t **) ckd_realloc(wstg->expandfillertree,(wstg->n_static_lextree+NUM_TREE_ALLOC)*sizeof(lextree_t*));
		
		for(i=wstg->n_static_lextree;i<(wstg->n_static_lextree+NUM_TREE_ALLOC);i++){
		  wstg->expandtree[i]=NULL;
		  wstg->expandtree[i]=lextree_init(s->kbc,wstg->lmset->lmarray[0],wstg->lmset->lmarray[0]->name,
						   wstg->isLMLA,!REPORT_SRCH_WST,LEXTREE_TYPE_BIGRAM);

		  if(wstg->expandtree[i]==NULL){
		    E_INFO("Fail to allocate lexical tree %d for lm %d  \n",i, 0);
		    return SRCH_FAILURE;
		  }

		  wstg->expandfillertree[i]=fillertree_init(s->kbc);
		  
		  if(wstg->expandfillertree[i]==NULL){
		    E_INFO("Fail to allocate filler tree %d for lm %d \n",i,0);
		    return SRCH_FAILURE;
		  }
		}

		/*Also push the NUM_TREE_ALLOC to the empty list */

		for(i=wstg->n_static_lextree;i<(wstg->n_static_lextree+NUM_TREE_ALLOC);i++){
		  wstg->empty_tree_idx_stack=glist_add_int32(wstg->empty_tree_idx_stack,i);
		}
		
		wstg->n_static_lextree+=NUM_TREE_ALLOC;	

	      }
      
	      if(hash_enter(wstg->active_word,dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,ln->wid)),id ) != id ){
		E_FATAL("hash_enter(local-phonetable, %s) failed\n", dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,ln->wid)));
	      }

	      /*E_INFO("A new tree is started for wid %d, word str %s\n", wid, 
		dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));*/

	    }
	    
	    /* So called rescore.  But it actually just compute the trigram score and insert it into the entry. */   
	    entry=vithist_n_entry(vh)-1;

	    vithist_rescore (vh, s->kbc, ln->wid, cur_frm,
			     hmm->out.score - ln->prob, 
			     hmm->out.history, lextree->type,ln->rc);

	    /* At this point a score is recorded in the viterbi history 
	       That consist of the trigram score
	     */ 
	    /* This part start to propagate the result to another tree */
	    /*At the end also propagate to the lexical tree as well.  */

	    /* Get the latest entry, i.e. the entry just entered the Viterbi history */

	    
	    if(entry<vithist_n_entry(vh)-1){
	      assert(vithist_n_entry(vh)-1>entry);
	      entry=vithist_n_entry(vh)-1;
	      
	      tve= vh->entry[VITHIST_ID2BLK(entry)] + VITHIST_ID2BLKOFFSET(entry);

	      p = dict_last_phone (kbcore_dict(s->kbc), ln->wid);
	      /*	    E_INFO("Entering: The word id %d word str %s, the phone %d phone str %s, the score %d\n", 
			    ln->wid, 
			    dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,ln->wid)),
			    p,
			    mdef_ciphone_str(mdef,p),
			    tve->score);*/


	      if (dict_filler_word (s->kbc->dict, ln->wid)) {
		/*E_INFO("Filler into Word\n");*/
		/*lextree_enter(wstg->expandtree[id], BAD_S3CIPID, cur_frm, tve->score,entry , s->beam->phone_thres);*/
		
	      }else{
		/*E_INFO("Word into Word\n");*/
		lextree_enter(wstg->expandtree[id], (s3cipid_t) p, cur_frm, tve->score,entry , s->beam->phone_thres,s->kbc);
	      }
	    
	      /*E_INFO("For Filler tree\n");*/
	      lextree_enter(wstg->expandfillertree[id], BAD_S3CIPID, cur_frm, tve->score,entry, s->beam->phone_thres,s->kbc);
	      
	    }

	}

    }
    return SRCH_SUCCESS;
}


/* 
 * 1, history entry is first inserted without considering LM score
 * 2, pruning in the history. 
 * 3, Enter the tree based on the word. 
 */
int32 srch_WST_propagate_graph_wd_lv2(void *srch, int32 frmno)
{
  dict_t *dict;
  vithist_t *vh;
  histprune_t *hp;
  kbcore_t *kbcore;
  mdef_t *mdef;

  srch_t* s;
  srch_WST_graph_t* wstg;
  int32 maxwpf;        /* Local version of Max words per frame, don't expect it to change */
  int32 maxhistpf;     /* Local version of Max histories per frame, don't expect it to change */
  int32 maxhmmpf;      /* Local version of Max active HMMs per frame, don't expect it to change  */
  int32 th;
  beam_t *bm;
  int32 active_word_end;

  vithist_entry_t *ve;
  int32 vhid,le,n_ci, score;
  s3wid_t wid;
  int32 p;
  int32 i;

  /*  hash_table_t *word_phone_hash;*/
  glist_t keylist;
  gnode_t *key;
  hash_entry_t *he;
  int32 id;
  int32 succ;
  int32 val;

  /* Call the rescoring at all word end */

  s=(srch_t*) srch;
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;
  kbcore = s->kbc;

  hp = wstg->histprune;
  vh   = s->vithist;
  dict = kbcore_dict (kbcore);
  mdef = kbcore_mdef(s->kbc);
  n_ci = mdef_n_ciphone(mdef);
  maxwpf    = hp->maxwpf;
  maxhistpf = hp->maxhistpf;
  maxhmmpf  = hp->maxhmmpf;
  active_word_end=0;

  s=(srch_t*) srch;
 
  wstg=(srch_WST_graph_t*) s->grh->graph_struct;

  /* Look at all the word ends */
  
  bm=s->beam;

  /*  E_INFO("Time %d\n",frmno);*/

  /* Rescoring at all word ends */
  /* This is confusing. Why do we want to rescore all word end to have trigram rescoring? 
     They just need to enter the node and there is no need to rescore. 
     
     Or better, if we still want to rescore. The new score shouldn't
     be apply to the Viterbi history. 
   */
  srch_WST_rescoring((void*)s, frmno);

  /* Prune the history */
  vithist_prune (vh, dict, frmno, maxwpf, maxhistpf, 
		 s->beam->word_thres-s->beam->bestwordscore); 
  
  vhid = vithist_first_entry (vh, frmno);
  le = vithist_n_entry (vh) - 1;

  th = bm->bestscore + bm->hmm;	/* Pruning threshold */

  /** Collect all entries with same last phone and same words. */
  /** This is slow.*/

  /* At here, just delete the tree with no active entries */
  keylist=hash_tolist(wstg->active_word,&(wstg->no_active_word));

  /*  hash_display(wstg->active_word,1);*/
  for (key = keylist; key; key = gnode_next(key)) {
    he = (hash_entry_t *) gnode_ptr (key);
    assert(glist_count(keylist)>0);

    hash_lookup (wstg->active_word, (char* )hash_entry_key(he),&val);
    /*    E_INFO("Entry %d, word %s\n",i,(char *) hash_entry_key(he));*/
    /*E_INFO("Entry %d, word %s, treeid %d. No of active word %d in the tree\n",i,(char *) hash_entry_key(he),val, wstg->expandtree[val]->n_active);*/

    if(wstg->expandtree[val]->n_active==0){ 
      /* insert this tree back to the list*/
      wstg->empty_tree_idx_stack=glist_add_int32(wstg->empty_tree_idx_stack,val);
      /* delete this entry from the hash*/
      /*E_INFO("val %d, expandtree[val]->prev_word %s\n",val,wstg->expandtree[val]->prev_word);*/
      succ=hash_delete(wstg->active_word,wstg->expandtree[val]->prev_word);
      if(succ==HASH_OP_FAILURE){
	E_WARN("Internal error occur in hash deallocation. \n");
      }else if(succ==HASH_OP_SUCCESS){
	/*	E_INFO("DELETE EMPTY TREE. \n");*/
      /*assert(succ==HASH_OP_SUCCESS);*/
      /* set the number of active word */
	wstg->no_active_word--;
      }
      /* set the word of a lextree to be none*/
      strcpy(wstg->expandtree[val]->prev_word,"");
    }
  }


  /* This is a place one should do all tricks of cross-word triphones */
  /** All entries traverse to tree with word context*/

  vhid = vithist_first_entry (vh, frmno);
  le = vithist_n_entry (vh) - 1;

  for (; vhid <= le; vhid++) {
    ve = vithist_id2entry (vh, vhid);

    /*    vithist_entry_display(ve,NULL);*/
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

      /* Pop one indices from the stack*/
      id = gnode_int32(wstg->empty_tree_idx_stack);
      wstg->empty_tree_idx_stack=glist_delete(wstg->empty_tree_idx_stack);

      /*      assert(wstg->expandtree[id]->prev_word==NULL);*/

      strcpy(wstg->expandtree[id]->prev_word,dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));
      wstg->no_active_word++;

      /* In general, the high scoring ones to be taken care first. */
      if(wstg->no_active_word == wstg->n_static_lextree){
	/* If another copy is not available, start to allocate a copy of tree.*/
	E_INFO("Allocate another %d more tree(s)\n", NUM_TREE_ALLOC);

	wstg->expandtree      =(lextree_t **) ckd_realloc(wstg->expandtree,(wstg->n_static_lextree+NUM_TREE_ALLOC)*sizeof(lextree_t*));
	wstg->expandfillertree=(lextree_t **) ckd_realloc(wstg->expandfillertree,(wstg->n_static_lextree+NUM_TREE_ALLOC)*sizeof(lextree_t*));

	for(i=wstg->n_static_lextree;i<(wstg->n_static_lextree+NUM_TREE_ALLOC);i++){
	  wstg->expandtree[i]=NULL;
	  wstg->expandtree[i]=lextree_init(s->kbc,wstg->lmset->lmarray[0],wstg->lmset->lmarray[0]->name,
					   wstg->isLMLA,!REPORT_SRCH_WST, LEXTREE_TYPE_BIGRAM);

	  if(wstg->expandtree[i]==NULL){
	    E_INFO("Fail to allocate lexical tree %d for lm %d  \n",i, 0);
	    return SRCH_FAILURE;
	  }

	  wstg->expandfillertree[i]=fillertree_init(s->kbc);

	  if(wstg->expandfillertree[i]==NULL){
	    E_INFO("Fail to allocate filler tree %d for lm %d \n",i,0);
	    return SRCH_FAILURE;
	  }

	}

	/*Also push the NUM_TREE_ALLOC to the empty list */

	for(i=wstg->n_static_lextree;i<(wstg->n_static_lextree+NUM_TREE_ALLOC);i++){
	  wstg->empty_tree_idx_stack=glist_add_int32(wstg->empty_tree_idx_stack,i);
	}

	wstg->n_static_lextree+=NUM_TREE_ALLOC;	

      }
      
      if(hash_enter(wstg->active_word,dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)),id ) != id ){
	E_FATAL("hash_enter(local-phonetable, %s) failed\n", dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));
      }

      /*E_INFO("A new tree is started for wid %d, word str %s\n", wid, 
	dict_wordstr(s->kbc->dict,dict_basewid(s->kbc->dict,wid)));*/

    }else{
    }

    lextree_enter(wstg->expandtree[id], (s3cipid_t) p, frmno, score, vhid, th,s->kbc);
    lextree_enter(wstg->expandfillertree[id], BAD_S3CIPID, frmno, score,
		  vh->bestvh[frmno], th,s->kbc);

  }

  
  return SRCH_SUCCESS;

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
  lextree_active_swap(wstg->curfillertree);

  for (i = 0; i < wstg->n_static_lextree; i++) {
    lextree_active_swap(wstg->expandtree[i]);
    lextree_active_swap(wstg->expandfillertree[i]);
  }
   
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


    ascr_clear_ssid_active(ascr);
    ascr_clear_comssid_active(ascr);
#if 0
    memset (ascr->ssid_active, 0, mdef_n_sseq(mdef) * sizeof(int32));
    memset (ascr->comssid_active, 0, dict2pid_n_comsseq(d2p) * sizeof(int32));
#endif

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

    lextree=wstg->curfillertree;
    if(0) E_INFO("In the filler root tree\n");
    lextree_ssid_active (lextree, ascr->ssid_active, ascr->comssid_active);

    /*The word tree copies */
    for(i = 0 ; i < n_ltree; i++){
      if(0) E_INFO("In the expand filler tree %d\n",i);
      lextree=wstg->expandfillertree[i];
      lextree_ssid_active (lextree, ascr->ssid_active, ascr->comssid_active);
    }

    ascr_clear_sen_active(ascr);
#if 0
    memset (ascr->sen_active, 0, mdef_n_sen(mdef) * sizeof(int32));
#endif
    mdef_sseq2sen_active (mdef, ascr->ssid_active, ascr->sen_active);

    /* Add in senones needed for active composite senone-sequences */
    dict2pid_comsseq2sen_active (d2p, mdef, ascr->comssid_active, ascr->sen_active);
  }

  return SRCH_SUCCESS;
}

#if 0
int srch_WST_dump_vithist(void* srch)
{
  srch_t* s;
  FILE  *bptfp;
  char file[8192];

  s=(srch_t*) srch;

  assert(s->vithist);

  sprintf (file, "%s/%s.bpt",cmd_ln_str("-bptbldir") , s->uttid);
  if ((bptfp = fopen (file, "w")) == NULL) {
    E_ERROR("fopen(%s,w) failed; using stdout\n", file);
    bptfp = stdout;
  }

  vithist_dump (s->vithist, -1, s->kbc, bptfp);
    
  if (bptfp != stdout)
    fclose (bptfp);

  return SRCH_SUCCESS;
}


glist_t srch_WST_gen_hyp (void * srch /**< a pointer of srch_t */
			  )
{
  srch_t* s;
  s=(srch_t*) srch;
  int32 id;
  glist_t hyp;

  assert(s->vithist);
  if ((id = vithist_utt_end (s->vithist, s->kbc)) >= 0) {
    assert(id>=0);
    hyp = vithist_backtrace (s->vithist, id, kbcore_dict(s->kbc));

  } else{
    E_ERROR("%s: No recognition\n\n", s->uttid);
    return NULL;
  }
}
#endif
