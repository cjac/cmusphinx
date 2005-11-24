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


/* srch.c
 * HISTORY
 * $Log$
 * Revision 1.2  2005/11/24  07:34:04  arthchan2003
 * Fixed a typo in the code which caused the header of the lattice wasn't dumped correctly.
 * 
 * Revision 1.1  2005/06/22 02:24:42  arthchan2003
 * Log. A search interface implementation are checked in. I will call
 * srch_t to be search abstraction or search mechanism from now on.  The
 * major reason of separating with the search implementation routine
 * (srch_*.[ch]) is that search is something that people could come up
 * with thousands of ways to implement.
 *
 * Such a design shows a certain sense of defiance of conventional ways
 * of designing speech recognition. Namely, **always** using generic
 * graph as the grandfather ancester of every search lattice.  This could
 * 1) break a lot of legacy optimization code. 2) could be slow depends
 * on the implementation.
 *
 * The current design only specify the operations that are supposed to be
 * generic in every search (or atomic search operations (ASOs)).
 * Ideally, users only need to implement the interface to make the code
 * work for another search.
 *
 * From this point of view, the current check-in still have some
 * fundamental flaws.  For example, the communication mechanism between
 * different atomic search operations are not clearly defined. Scores are
 * now computed and put into structures of ascr. (ascr has no clear
 * interface to outside world). This is something we need to improve.
 *
 * Revision 1.21  2005/06/17 21:22:59  archan
 * Added comments for future programmers.  That allow potential turn back when we need to match the score of the code in the past.
 *
 * Revision 1.20  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.19  2005/06/15 21:36:00  archan
 * Added strong safe-guarding for the function pointers in the search structure. This is now done in the initialization time and make sure programmers will set these pointers responsibly.
 *
 * Revision 1.18  2005/06/15 21:13:28  archan
 * Open ld_set_lm, ld_delete_lm in live_decode_API.[ch], Not yet decided whether ld_add_lm and ld_update_lm should be added at this point.
 *
 * Revision 1.17  2005/06/10 03:40:57  archan
 * 1, Fixed doxygen documentation of srch.h, 2, eliminate srch.h C-style functions. 3, Start to fend off the users for using mode 5.  We are ready to merge the code.
 *
 * Revision 1.16  2005/05/11 06:10:38  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.15  2005/05/04 05:15:25  archan
 * reverted the last change, seems to be not working because of compilation issue. Try not to deal with it now.
 *
 * Revision 1.1  2005/05/04 04:46:04  archan
 * Move srch.c and srch.h to search. More and more this type of refactoring will be done in future
 *
 * Revision 1.13  2005/05/03 06:57:43  archan
 * Finally. The word switching tree code is completed. Of course, the reporting routine largely duplicate with time switching tree code.  Also, there has to be some bugs in the filler treatment.  But, hey! These stuffs we can work on it.
 *
 * Revision 1.12  2005/05/03 04:09:09  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.11  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.10  2005/04/25 19:22:47  archan
 * Refactor out the code of rescoring from lexical tree. Potentially we want to turn off the rescoring if we need.
 *
 * Revision 1.9  2005/04/22 04:22:36  archan
 * Add gmm_wrap, this will share code across op_mode 4 and op_mode 5. Also it also separate active senone selection into a different process.  I hope this is the final step before making the WST search works.  At the current stage, the code of mode-5 looks very much alike mode-4.  This is intended because in Prototype 4, tail sharing will be used to reduce memory.
 *
 * Revision 1.8  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.7  2005/04/20 03:42:55  archan
 * srch.c now is the only of the master search driver. When there is any change in the **interaction** of different blocks, srch.c should be changed first.  Then the search implenetation, such as srch_time_switch_tree.c
 *
 * Revision 1.6  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. This replaced utt.c starting from Sphinx 3.6. 
 */


#include "srch.h"
#define COMPUTE_HEURISTIC 1


void srch_assert_funcptrs(srch_t *s){
  assert(s->srch_init!=NULL);
  assert(s->srch_uninit!=NULL);
  assert(s->srch_utt_begin!=NULL);
  assert(s->srch_utt_end!=NULL);
  assert(s->srch_decode!=NULL);
  assert(s->srch_set_lm!=NULL);
  assert(s->srch_add_lm!=NULL);
  assert(s->srch_delete_lm!=NULL);
  assert(s->srch_compute_heuristic!=NULL);
  assert(s->srch_gmm_compute_lv1!=NULL);
  assert(s->srch_hmm_compute_lv1!=NULL);
  assert(s->srch_eval_beams_lv1!=NULL);
  assert(s->srch_propagate_graph_ph_lv1!=NULL);
  assert(s->srch_propagate_graph_wd_lv1!=NULL);
  assert(s->srch_gmm_compute_lv2!=NULL);
  assert(s->srch_hmm_compute_lv2!=NULL);
  assert(s->srch_eval_beams_lv2!=NULL);
  assert(s->srch_propagate_graph_ph_lv2!=NULL);
  assert(s->srch_propagate_graph_wd_lv2!=NULL);
  assert(s->srch_rescoring!=NULL);
  assert(s->srch_frame_windup!=NULL);
  assert(s->srch_compute_heuristic!=NULL);
  assert(s->srch_shift_one_cache_frame!=NULL);
  assert(s->srch_select_active_gmm!=NULL);

}

void srch_clear_funcptrs(srch_t *s){
  s->srch_init=NULL;
  s->srch_uninit=NULL;
  s->srch_utt_begin=NULL;
  s->srch_utt_end=NULL;
  s->srch_decode=NULL;
  s->srch_set_lm=NULL;
  s->srch_add_lm=NULL;
  s->srch_delete_lm=NULL;
  s->srch_compute_heuristic=NULL;
  s->srch_gmm_compute_lv1=NULL;
  s->srch_hmm_compute_lv1=NULL;
  s->srch_eval_beams_lv1=NULL;
  s->srch_propagate_graph_ph_lv1=NULL;
  s->srch_propagate_graph_wd_lv1=NULL;
  s->srch_gmm_compute_lv2=NULL;
  s->srch_hmm_compute_lv2=NULL;
  s->srch_eval_beams_lv2=NULL;
  s->srch_propagate_graph_ph_lv2=NULL;
  s->srch_propagate_graph_wd_lv2=NULL;
  s->srch_rescoring=NULL;
  s->srch_frame_windup=NULL;
  s->srch_compute_heuristic=NULL;
  s->srch_shift_one_cache_frame=NULL;
  s->srch_select_active_gmm=NULL;

}
/** Initialize the search routine, this will specify the type of search
    drivers and initialized all resouces*/

srch_t* srch_init(kb_t* kb, int32 op_mode){

  srch_t *s;
  s=(srch_t*) ckd_calloc(1, sizeof(srch_t));

  E_INFO("Search Initialization. \n");
  s->op_mode=op_mode;
  s->grh=(grp_str_t*) ckd_calloc(1,sizeof(grp_str_t));
  s->cache_win=cmd_ln_int32("-pl_window");
  s->cache_win_strt=0 ; 
  s->senscale=0;

  srch_clear_funcptrs(s);


  /* A switch here to decide all function pointers */
  if(op_mode==OPERATION_ALIGN){

    E_FATAL("Alignment mode is not supported yet");

  }else if(op_mode==OPERATION_ALLPHONE){

    E_FATAL("Allphone mode is not supported yet");

  }else if(op_mode==OPERATION_GRAPH){

    E_FATAL("Finite state graphs search is not supported yet");

  }else if(op_mode==OPERATION_FLAT_DECODE){

    E_FATAL("Flat decoding is not supported yet");

  }else if(op_mode==OPERATION_TST_DECODE){

    s->srch_init=&srch_TST_init;
    s->srch_uninit=&srch_TST_uninit;
    s->srch_utt_begin=&srch_TST_begin;
    s->srch_utt_end=&srch_TST_end;
    s->srch_decode=&srch_TST_decode;

    s->srch_set_lm=&srch_TST_set_lm;
    s->srch_add_lm=&srch_TST_add_lm;
    s->srch_delete_lm=&srch_TST_delete_lm;

    s->srch_select_active_gmm=&srch_TST_select_active_gmm;
    s->srch_gmm_compute_lv1=&approx_ci_gmm_compute;
    s->srch_gmm_compute_lv2=&approx_cd_gmm_compute;

    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_eval_beams_lv2=&srch_debug_eval_beams_lv2;

    s->srch_hmm_compute_lv2=&srch_TST_hmm_compute_lv2;
    s->srch_propagate_graph_ph_lv2=&srch_TST_propagate_graph_ph_lv2;
    s->srch_propagate_graph_wd_lv2=&srch_TST_propagate_graph_wd_lv2;
    s->srch_rescoring=&srch_TST_rescoring;

    s->srch_compute_heuristic=&srch_TST_compute_heuristic;
    s->srch_frame_windup=&srch_TST_frame_windup;
    s->srch_shift_one_cache_frame=&srch_TST_shift_one_cache_frame;

  }else if(op_mode==OPERATION_WST_DECODE){

    E_FATAL("Word Conditioned Tree Search is still under development. It is now fended off from the users.");

    s->srch_init=&srch_WST_init;
    s->srch_uninit=&srch_WST_uninit;
    s->srch_utt_begin=&srch_WST_begin;
    s->srch_utt_end=&srch_WST_end;
    s->srch_decode=&srch_WST_decode;
    s->srch_set_lm=&srch_WST_set_lm;

    s->srch_select_active_gmm=&srch_WST_select_active_gmm;
    s->srch_gmm_compute_lv1=&approx_ci_gmm_compute;
    s->srch_gmm_compute_lv2=&approx_cd_gmm_compute;

    s->srch_eval_beams_lv2=&srch_debug_eval_beams_lv2;

    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_hmm_compute_lv2=&srch_WST_hmm_compute_lv2;
    s->srch_propagate_graph_ph_lv2=&srch_WST_propagate_graph_ph_lv2;

    s->srch_propagate_graph_wd_lv2=&srch_WST_propagate_graph_wd_lv2;
    s->srch_rescoring=&srch_WST_rescoring;

    s->srch_compute_heuristic=&srch_WST_compute_heuristic;
    s->srch_frame_windup=&srch_WST_frame_windup;
    s->srch_shift_one_cache_frame=&srch_WST_shift_one_cache_frame;

  }else if(op_mode==OPERATION_DEBUG){

    s->srch_init=&srch_debug_init;
    s->srch_uninit=&srch_debug_uninit;
    s->srch_utt_begin=&srch_debug_begin;
    s->srch_utt_end=&srch_debug_end;
    s->srch_decode=&srch_debug_decode;
    s->srch_set_lm=&srch_debug_set_lm;

    s->srch_select_active_gmm=&srch_debug_select_active_gmm;    
    s->srch_gmm_compute_lv1=&srch_debug_gmm_compute_lv1;
    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_gmm_compute_lv2=&srch_debug_gmm_compute_lv2;
    s->srch_hmm_compute_lv2=&srch_debug_hmm_compute_lv2;
    s->srch_eval_beams_lv2=&srch_debug_eval_beams_lv2;
    s->srch_propagate_graph_ph_lv2=&srch_debug_propagate_graph_ph_lv2;
    s->srch_propagate_graph_wd_lv2=&srch_debug_propagate_graph_wd_lv2;

    s->srch_rescoring=&srch_debug_rescoring;
    s->srch_compute_heuristic=&srch_debug_compute_heuristic;
    s->srch_frame_windup=&srch_debug_frame_windup;
    s->srch_shift_one_cache_frame=&srch_debug_shift_one_cache_frame;
  }else{
    E_FATAL("Unknown mode %d, failed to initialized srch_t\n",op_mode);
  }

  srch_assert_funcptrs(s);
  
  /* Do general search initialization here. */
  s->stat=kb->stat;
  s->ascr=kb->ascr;
  s->vithist=kb->vithist;
  s->beam=kb->beam;
  s->fastgmm=kb->fastgmm;
  s->pl=kb->pl;
  s->adapt_am=kb->adapt_am;
  s->kbc=kb->kbcore;

  s->matchfp=kb->matchfp;
  s->matchsegfp=kb->matchsegfp;
  s->hmmdumpfp=kb->hmmdumpfp;



  /* Do search-specific initialization here. */ 

  if(s->srch_init(kb,s)==SRCH_FAILURE){
    E_INFO("search initialization failed for op-mode %d\n",op_mode);
    return NULL;
  }
  
  return s;
}

int32 srch_utt_begin(srch_t* srch){
  if(srch->srch_utt_begin==NULL){
    E_INFO("srch->srch_utt_begin is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  srch->srch_utt_begin(srch);
  return SRCH_SUCCESS;
}

int32 srch_utt_end(srch_t* srch){

  if(srch->srch_utt_end==NULL){
    E_INFO("srch->srch_utt_end is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  srch->srch_utt_end(srch);
  return SRCH_SUCCESS;
}

int32 srch_utt_decode_blk(srch_t* s, float ***block_feat, int32 block_nfeatvec, int32 *curfrm)
{
  /*This is the basic backbone of the search. */
  stat_t *st;
  int32 frmno, f, t;
  int32 win_efv;
  st = s->stat;

  frmno = *curfrm;

  /* the effective window is the min of (s->cache_win, block_nfeatvec) */
  win_efv = s->cache_win;
  if(win_efv > block_nfeatvec) 
    win_efv = block_nfeatvec;

  s->cache_win_strt=0;

  /*Compute the CI senone score at here */
  ptmr_start (&(st->tm_sen));  
  ptmr_start (&(st->tm_ovrhd));
  
  for(f = 0; f < win_efv; f++){
    s->srch_gmm_compute_lv1(s,block_feat[f][0],f,f);
  }
  
  ptmr_stop (&(st->tm_ovrhd));  
  ptmr_stop (&(st->tm_sen));

  for (t = 0; t < block_nfeatvec; t++,frmno++) {
    /*    E_INFO("Time: %d\n",t);*/
    /* Acoustic (senone scores) evaluation */
    ptmr_start (&(st->tm_sen));
    s->srch_select_active_gmm(s);
    s->srch_gmm_compute_lv2(s,block_feat[t][0],t);
    ptmr_stop (&(st->tm_sen));

    /* Propagate graph at phoneme (hmm) level */
    ptmr_start (&(st->tm_srch));
    
    /* Determine which set of phonemes should be active in next stage
       using the lookahead information*/
    /* This should be part of hmm_compute_lv1 */
    if(COMPUTE_HEURISTIC) s->srch_compute_heuristic(s,win_efv);
    /* HMM compute Lv 2, currently, this routine compute hmm for the
     *  data structure and compute the beam. */
    s->srch_hmm_compute_lv2(s,frmno);

    /* After the HMM scores are computed, tokens are propagate in the
     * phone-level.  */
    s->srch_propagate_graph_ph_lv2(s,frmno);

    /* Rescoring. Usually happened at the word end.  */
    s->srch_rescoring(s,frmno);

    /* Propagate the score on the word-level */
    s->srch_propagate_graph_wd_lv2(s,frmno);
    ptmr_stop (&(st->tm_srch));

    ptmr_start (&(st->tm_sen));
    ptmr_start (&(st->tm_ovrhd));    
    /* if the current block's current frame (t) is less than the total
       frames in this block minus the efv window */

    if(t<block_nfeatvec-win_efv){
      s->srch_shift_one_cache_frame(s,win_efv);
      s->srch_gmm_compute_lv1(s,block_feat[t+win_efv][0],win_efv-1,t+win_efv);
    } else {/* We are near the end of the block, so shrink the window from the left*/
      s->cache_win_strt++;
    }
  
    ptmr_stop (&(st->tm_ovrhd));
    ptmr_stop (&(st->tm_sen));

    s->srch_frame_windup(s,frmno);
  }

  st->nfr += block_nfeatvec;
  
  
  *curfrm = frmno;

  return SRCH_SUCCESS;
}


/** Wrap up the search routine*/
int32 srch_uninit(srch_t* srch){
  if(srch->srch_uninit==NULL){
    return SRCH_FAILURE;
  }
  srch->srch_uninit(srch);
  return SRCH_SUCCESS;
}

/** Wrap up the search report routine*/
void srch_report(srch_t* srch){
  E_INFO_NOFN("Initialization of srch_t, report:\n");
  E_INFO_NOFN("Operation Mode = %d\n",srch->op_mode);
  E_INFO_NOFN("\n");
}


/** using file name of the LM or defined lmctlfn mechanism */
int32 srch_set_lm(srch_t* srch, const char *lmname ){

  if(srch->srch_set_lm==NULL){
    E_INFO("srch->srch_set_lm is NULL. Please make sure it is set. No change will be made currently. \n");
    return SRCH_FAILURE;
  }
  /* srch should own resource such as kbc and acoustic models */
  srch->srch_set_lm(srch, lmname);
  return SRCH_SUCCESS;
}

int32 srch_add_lm(srch_t* srch, lm_t* lm, const char *lmname)
{
  if(srch->srch_add_lm==NULL){
    E_INFO("srch->srch_add_lm is NULL. Please make sure it is set. No change will be made currently. \n");
    return SRCH_FAILURE;
  }

  srch->srch_add_lm(srch,lm,lmname);
  return SRCH_SUCCESS;
}


int32 srch_delete_lm(srch_t *srch, const char* lmname)
{
  if(srch->srch_delete_lm==NULL){
    E_INFO("srch->srch_delete_lm is NULL. Please make sure it is set. No change will be made currently. \n");
    return SRCH_FAILURE;
  }

  srch->srch_delete_lm(srch,lmname);
  return SRCH_SUCCESS;
}


#if 0 /** fend off*/
/** using file name of the model definition and directory name to initialize */
int32 srch_set_am(srch_t* srch){
  if(srch->srch_set_am==NULL){
    E_INFO("srch->srch_set_am is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  return SRCH_SUCCESS;
}

/** using file name of the regression class mapping or a directory name to initialize  */
int32 srch_set_mllr(srch_t* srch){
  if(srch->srch_set_mllr==NULL){
    E_INFO("srch->srch_set_mllr is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  return SRCH_SUCCESS;
}

/** using file name of interpolation file to initialize it */
int32 srch_set_lamdafn(srch_t* srch){
  if(srch->srch_set_lamdafn==NULL){
    E_INFO("srch->srch_set_lamdafn is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  return SRCH_SUCCESS;
}
#endif


void matchseg_write (FILE *fp, srch_t *s,  glist_t hyp, char *hdr)
{
    gnode_t *gn;
    hyp_t *h;
    int32 ascr, lscr, scl;
    dict_t *dict;

    ascr = 0;
    lscr = 0;
    scl =0;
    
    for (gn = hyp; gn; gn = gnode_next(gn)) {
	h = (hyp_t *) gnode_ptr (gn);
	ascr += h->ascr;
	lscr += h->lscr;
	scl += h->senscale;
    }
    
    dict = kbcore_dict(s->kbc);
    
    fprintf (fp, "%s%s S %d T %d A %d L %d", (hdr ? hdr : ""), s->uttid,
	     scl, ascr+lscr, ascr, lscr);
    
    for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
	h = (hyp_t *) gnode_ptr (gn);
	fprintf (fp, " %d %d %d %s", h->sf, h->ascr, h->lscr,
		 dict_wordstr(dict, h->id));
    }
    fprintf (fp, " %d\n", s->stat->nfr);
    fflush (fp);
}

void match_write (FILE *fp, srch_t* s, glist_t hyp, char *hdr)
{
    gnode_t *gn;
    hyp_t *h;
    dict_t *dict;
    int counter=0;

    dict = kbcore_dict(s->kbc);

    fprintf (fp, "%s ", (hdr ? hdr : ""));

    for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
      h = (hyp_t *) gnode_ptr (gn);
      if((!dict_filler_word(dict,h->id)) && (h->id!=dict_finishwid(dict)))
	fprintf(fp,"%s ",dict_wordstr(dict, dict_basewid(dict,h->id)));
      counter++;
    }
    if(counter==0) fprintf(fp," ");
    fprintf (fp, "(%s)\n", s->uttid);
    fflush (fp);
}


void reg_result_dump (srch_t* s, int32 id)
{
  FILE *fp, *latfp, *bptfp;
  int32 ascr, lscr;
  glist_t hyp;
  stat_t* st; 
  gnode_t *gn;
  hyp_t *h;

  st= s->stat;

  fp = stderr;

  if (cmd_ln_str("-bptbldir")) {
    char file[8192];
    sprintf (file, "%s/%s.bpt",cmd_ln_str("-bptbldir") , s->uttid);
    if ((bptfp = fopen (file, "w")) == NULL) {
      E_ERROR("fopen(%s,w) failed; using stdout\n", file);
      bptfp = stdout;
    }
    
    vithist_dump (s->vithist, -1, s->kbc, bptfp);
    
    if (bptfp != stdout)
	fclose (bptfp);
  }
  
  hyp = vithist_backtrace (s->vithist, id);
  
  /* Detailed backtrace */
  if (cmd_ln_int32("-backtrace")) {
    fprintf (fp, "\nBacktrace(%s)\n", s->uttid);
    fprintf (fp, "%6s %5s %5s %11s %8s %4s\n",
	     "LatID", "SFrm", "EFrm", "AScr", "LScr", "Type");
      
    ascr = 0;
    lscr = 0;
    
    for (gn = hyp; gn; gn = gnode_next(gn)) {
      h = (hyp_t *) gnode_ptr (gn);
      fprintf (fp, "%6d %5d %5d %11d %8d %4d %s\n",
	       h->vhid, h->sf, h->ef, h->ascr + h->senscale, h->lscr, h->type,
	       dict_wordstr(s->kbc->dict, h->id));

#if 1      
      ascr += h->ascr + h->senscale;
#else /* ARCHAN: If you want the score of without scaling. This behavior appeas in 3.1 to 3.5
	 During the time of CALO, we found that it is not in-sync with s3.0 family of tools. So,
	 we decided to change it back. I leave this line for the future purpose of debugging. 
       */
      ascr += h->ascr;
#endif

      lscr += h->lscr;
    }

    fprintf (fp, "       %5d %5d %11d %8d (Total)\n",0,st->nfr,ascr,lscr);
  }
  
  /* Match */
  if (s->matchfp)
    match_write (s->matchfp, s, hyp, NULL);
  match_write(fp, s, hyp, "\nFWDVIT: ");
  
  /* Matchseg */
  if (s->matchsegfp)
    matchseg_write (s->matchsegfp, s, hyp, NULL);
  matchseg_write (fp, s, hyp, "FWDXCT: ");
  fprintf (fp, "\n");
  
      
  if (cmd_ln_str ("-outlatdir")) {
    int32 ispipe;
    char str[16384];
    sprintf (str, "%s/%s.%s",
	     cmd_ln_str("-outlatdir"), s->uttid, cmd_ln_str("-latext"));
    
    E_INFO("Writing lattice file: %s\n", str);
    
    if ((latfp = fopen_comp (str, "w", &ispipe)) == NULL)
      E_ERROR("fopen_comp (%s,w) failed\n", str);
    else {
      /* Write header info */
      vithist_dag_write_header(latfp,st->nfr,str);
      vithist_dag_write (s->vithist, hyp, s->kbc->dict,
			 cmd_ln_int32("-outlatoldfmt"), latfp);
      fclose_comp (latfp, ispipe);
    }
  }
      
  /** free the list containing hyps */
  for (gn = hyp; gn; gn = gnode_next(gn)) {
    ckd_free(gnode_ptr(gn));
  }
  glist_free(hyp);

}

