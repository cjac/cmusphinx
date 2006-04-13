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
 * Revision 1.5  2006/04/13  16:08:09  arthchan2003
 * Fix a Priority 9 Bug 1459402.
 * 
 * Revision 1.4  2006/02/23 16:47:16  arthchan2003
 * Safe-guarded the use of composite triphones.
 *
 * Revision 1.3  2006/02/23 15:26:10  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII:
 *
 * Summary of changes. Detail could be seen in the comments from the
 * branches.
 *
 *  After 6 months, we have two more searches using interface
 * provided by srch.c. That included an adapted version of Sphinx 2's FSG
 * search.  Also, the original version of flat-lexicon decoding search.
 *
 * Second stage search operation is still not properly put in the srch_t
 * structure.  We should create function hooks that allow developer to
 * put the code more properly than now.
 *
 * The interface of srch.c is still not very completed. Things we should
 * support include switching of AM and MLLR.  They are currently
 * commented.
 *
 * Mode 5, the word-dependent tree copies are now fended off from the
 * users.
 *
 * Mode 2, the FSG search are opened.  It is not very well tested so the
 * user will be warned about its nature.
 *
 *
 * Revision 1.1.4.21  2006/01/16 20:01:20  arthchan2003
 * Added Commented code in srch.[ch] for second-stage rescoring. Not used for now.
 *
 * Revision 1.2  2005/11/24 07:34:04  arthchan2003
 * Fixed a typo in the code which caused the header of the lattice wasn't dumped correctly.
 *
 *
 * Revision 1.1.4.20  2005/11/17 06:38:43  arthchan2003
 * change for comment, srch.c also now support conversion from s3 lattice to IBM lattice format.
 *
 * Revision 1.1.4.19  2005/11/17 06:36:36  arthchan2003
 * There are several important changes. 1, acoustic score scale has changed back to put it the search structure.  This fixed a bug introduced pre-2005 code branching where only the scaling factor of the last frame. 2, Added a fmt argument of matchseg_write , implemented segmentation output for s2 and ctm file format. matchseg_write also now shared across the flat and tree decoder now. 3, Added Rong's read_seg_hyp_line.
 *
 * Revision 1.1.4.18  2005/10/17 04:54:20  arthchan2003
 * Freed graph correctly.
 *
 * Revision 1.1.4.17  2005/09/25 19:30:21  arthchan2003
 * (Change for comments) Track error messages from propagate_leave and propagate_non_leave, this allow us to return error when bugs occur in internal of search.
 *
 * Revision 1.1.4.16  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.1.4.15  2005/09/18 01:44:12  arthchan2003
 * Very boldly, started to support flat lexicon decoding (mode 3) in srch.c.  Add log_hypseg. Mode 3 is implemented as srch-one-frame implementation. Scaling doesn't work at this point.
 *
 * Revision 1.1.4.14  2005/09/11 23:07:28  arthchan2003
 * srch.c now support lattice rescoring by rereading the generated lattice in a file. When it is operated, silence cannot be unlinked from the dictionary.  This is a hack and its reflected in the code of dag, kbcore and srch. code
 *
 * Revision 1.1.4.13  2005/08/02 21:37:28  arthchan2003
 * 1, Used s3_cd_gmm_compute_sen instead of approx_cd_gmm_compute_sen in mode 2, 4 and 5.  This will suppose to make s3.0 to be able to read SCHMM and use them as well. 2, Change srch_gmm_compute_lv2 to accept a two-dimensional array (no_stream*no_coeff) instead of a one dimensional array (no_coeff).
 *
 * Revision 1.1.4.12  2005/07/26 02:21:14  arthchan2003
 * merged hyp_t with srch_hyp_t.
 *
 * Revision 1.1.4.11  2005/07/24 01:39:26  arthchan2003
 * Added srch_on_srch_frame_lv[12] in the search abstraction routine.  This will allow implementation just provide the search for one frame without supplying all function pointer in the standard abstraction.
 *
 * Revision 1.1.4.10  2005/07/22 03:41:05  arthchan2003
 * 1, (Incomplete) Add function pointers for flat foward search. Notice implementation is not yet filled in. 2, adding log_hypstr and log_hyp_detailed.  It is sphinx 3.0 version of matchwrite.  Add it to possible code merge.
 *
 * Revision 1.1.4.9  2005/07/20 21:21:59  arthchan2003
 * Removed graph search fend-off.
 *
 * Revision 1.1.4.8  2005/07/17 05:54:55  arthchan2003
 * replace vithist_dag_write_header with dag_write_header
 *
 * Revision 1.1.4.7  2005/07/13 18:42:35  arthchan2003
 * Re-enabled function assignments for mode 3 in srch.c. Code compiled. Still has 3 major hacks and 8 minor hacks.  See message in fsg_*
 *
 * Revision 1.1.4.6  2005/07/07 02:37:39  arthchan2003
 * 1, Changed names of srchmode* functions to srch_mode*, 2, complete srch_mode_index_to_str, 3, Remove srch_rescoring and ask implementation to call these "rescoring functions" themselves.  The reason is rescoring is not as universal as I would think in the general search. I think search implementer should be the one who decide whether rescoring is one part of their search algorithms
 *
 * Revision 1.1.4.5  2005/07/04 07:18:49  arthchan2003
 * Disabled support of FSG. Added comments for srch_utt_begin and srch_utt_end.
 *
 * Revision 1.1.4.4  2005/07/03 23:04:55  arthchan2003
 * 1, Added srchmode_str_to_index, 2, called the deallocation routine of the search implementation layer in srch_uninit
 *
 * Revision 1.1.4.3  2005/06/28 07:03:01  arthchan2003
 * Added read_fsg operation as one method. Currently, it is still not clear how it should iteract with lm
 *
 * Revision 1.1.4.2  2005/06/27 05:32:35  arthchan2003
 * Started to give pointer function to mode 3. (It is already in my todolist to give better names to modes. )
 *
 * Revision 1.1.4.1  2005/06/24 21:13:52  arthchan2003
 * 1, Turn on mode 5 again, 2, fixed srch_WST_end, 3, Add empty function implementations of add_lm and delete_lm in mode 5. This will make srch.c checking happy.
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


static void write_bestsenscore(srch_t *s);

static void display_dump_hypothesis (srch_t *s, 
			      glist_t hyp, 
			      char* hyptag, 
			      char* hypsegtag, 
			      char* shortformCap, 
			      char* shortformSmall
			      );

int32 srch_mode_str_to_index(const char* mode_str)
{
  if(!strcmp(mode_str,"OP_ALIGN")){
    return OPERATION_ALIGN;
  }

  if(!strcmp(mode_str,"OP_ALLPHONE")){
    return OPERATION_ALLPHONE;
  }

  if(!strcmp(mode_str,"OP_FSG")){
    return OPERATION_GRAPH;
  }

  if(!strcmp(mode_str,"OP_FLATFWD")){
    return OPERATION_FLATFWD;
  }

  if(!strcmp(mode_str,"OP_MAGICWHEEL")){
    return OPERATION_TST_DECODE;
  }

  if(!strcmp(mode_str,"OP_TST_DECODE")){
    return OPERATION_TST_DECODE;
  }

  if(!strcmp(mode_str,"OP_WST_DECODE")){
    return OPERATION_WST_DECODE;
  }

  if(!strcmp(mode_str,"OP_DEBUG")){
    return OPERATION_DEBUG;
  }

  E_WARN("UNKNOWN MODE NAME %s\n",mode_str);
  return -1; 

}

char* srch_mode_index_to_str(int32 index)
{
  char* str;
  str=NULL;
  if(index==OPERATION_ALIGN){
    str=ckd_salloc("OP_ALIGN");
  }else if (index==OPERATION_ALLPHONE){
    str=ckd_salloc("OP_ALLPHONE");
  }else if (index==OPERATION_GRAPH){
    str=ckd_salloc("OP_FSG");
  }else if (index==OPERATION_FLATFWD){
    str=ckd_salloc("OP_FLATFWD");
  }else if (index==OPERATION_TST_DECODE){
    str=ckd_salloc("OP_TST_DECODE");
  }else if (index==OPERATION_WST_DECODE){
    str=ckd_salloc("OP_WST_DECODE");
  }else if (index==OPERATION_DEBUG){
    str=ckd_salloc("OP_DEBUG");
  }
  return str;
}

void srch_assert_funcptrs(srch_t *s){

  if(s->srch_decode!=NULL){    /* Provide that the implementation does
				  not override the search abstraction
				  assert every implementation pointers
			       */

    assert(s->srch_init!=NULL);
    assert(s->srch_uninit!=NULL);
    assert(s->srch_utt_begin!=NULL);
    assert(s->srch_set_lm!=NULL);
    assert(s->srch_add_lm!=NULL);
    assert(s->srch_delete_lm!=NULL);
    assert(s->srch_compute_heuristic!=NULL);
    
    assert(s->srch_gmm_compute_lv1!=NULL);

    if(s->srch_one_srch_frame_lv1!=NULL){ /* If implementation
					     provides only 
					     how to search one frame after 
					     GMM computation*/
      assert(s->srch_hmm_compute_lv1==NULL);
      assert(s->srch_eval_beams_lv1==NULL);
      assert(s->srch_propagate_graph_ph_lv1==NULL);
      assert(s->srch_propagate_graph_wd_lv1==NULL);
    }else{
      if(s->srch_hmm_compute_lv1==NULL)
	E_WARN("Search one frame implementation is not specified but srch_hmm_compute_lv1 is not specified\n");
      if(s->srch_eval_beams_lv1==NULL)
	E_WARN("Search one frame implementation is not specified but srch_eval_beams_lv1 is not specified\n");
      if(s->srch_propagate_graph_ph_lv1==NULL)
	E_WARN("Search one frame implementation is not specified but srch_propagate_graph_ph_lv1 is not specified\n");
      if(s->srch_propagate_graph_wd_lv1==NULL)
	E_WARN("Search one frame implementation is not specified but srch_propagate_graph_wd_lv1 is not specified\n");
    }

    assert(s->srch_gmm_compute_lv2!=NULL);
    if(s->srch_one_srch_frame_lv2!=NULL){ /* If implementation
					     provides only 
					     how to search one frame after 
					     GMM computation*/

      assert(s->srch_hmm_compute_lv2==NULL);
      assert(s->srch_eval_beams_lv2==NULL);
      assert(s->srch_propagate_graph_ph_lv2==NULL);
      assert(s->srch_propagate_graph_wd_lv2==NULL);
    }else{
      if(s->srch_hmm_compute_lv2==NULL)
	E_WARN("Search one frame implementation is not specified but srch_hmm_compute_lv2 is not specified\n");
      if(s->srch_eval_beams_lv2==NULL)
	E_WARN("Search one frame implementation is not specified but srch_eval_beams_lv2 is not specified\n");
      if(s->srch_propagate_graph_ph_lv2==NULL)
	E_WARN("Search one frame implementation is not specified but srch_propagate_graph_ph_lv2 is not specified\n");
      if(s->srch_propagate_graph_wd_lv2==NULL)
	E_WARN("Search one frame implementation is not specified but srch_propagate_graph_wd_lv2 is not specified\n");

    }
    assert(s->srch_frame_windup!=NULL);
    assert(s->srch_compute_heuristic!=NULL);
    assert(s->srch_shift_one_cache_frame!=NULL);
    assert(s->srch_select_active_gmm!=NULL);

    assert(s->srch_utt_end!=NULL);
    assert(s->srch_gen_hyp!=NULL);
    assert(s->srch_dump_vithist!=NULL);

#if 0 /* Not asserts for everything now, mainly because the FST mode
	 is not generating dag at this point */
    assert(s->srch_gen_dag!=NULL);

#endif
  }
}

void srch_clear_funcptrs(srch_t *s){
  s->srch_init=NULL;
  s->srch_uninit=NULL;
  s->srch_utt_begin=NULL;
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
  s->srch_one_srch_frame_lv2=NULL;
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

  s->srch_utt_end=NULL;
  s->srch_gen_hyp=NULL;
  s->srch_gen_dag=NULL;
  s->srch_dump_vithist=NULL;


  /* For convenience, clear but not check at this point */
  /*  s->srch_read_fsgfile=NULL;*/

}
/** Initialize the search routine, this will specify the type of search
    drivers and initialized all resouces*/

srch_t* srch_init(kb_t* kb, int32 op_mode){

  srch_t *s;
  char *str;
  s=(srch_t*) ckd_calloc(1, sizeof(srch_t));

  E_INFO("Search Initialization. \n");
  s->op_mode=op_mode;
  s->grh=(grp_str_t*) ckd_calloc(1,sizeof(grp_str_t));
  s->cache_win=cmd_ln_int32("-pl_window");
  s->cache_win_strt=0 ; 
  s->senscale=0;

  s->ascale= (int32 *) ckd_calloc(DFLT_UTT_SIZE,sizeof(int32));
  s->ascale_sz = DFLT_UTT_SIZE;
  s->segsz= (int32 *) ckd_calloc(DFLT_NUM_SEGS,sizeof(int32));
  s->segsz_sz = DFLT_NUM_SEGS;

  srch_clear_funcptrs(s);


  /* A switch here to decide all function pointers */
  if(op_mode==OPERATION_ALIGN){

    E_FATAL("Alignment mode is not supported yet");

  }else if(op_mode==OPERATION_ALLPHONE){

    E_FATAL("Allphone mode is not supported yet");

  }else if(op_mode==OPERATION_GRAPH){

    E_WARN("In Sphinx 3.6 RCI, Graph search is still recommended for internal use only. ");

    s->srch_init=&srch_FSG_init;
    /*    s->srch_read_fsgfile=&srch_FSG_read_fsgfile;*/

    s->srch_uninit=&srch_FSG_uninit;
    s->srch_utt_begin=&srch_FSG_begin;

    s->srch_set_lm=&srch_FSG_set_lm;
    s->srch_add_lm=&srch_FSG_add_lm;
    s->srch_delete_lm=&srch_FSG_delete_lm;

    s->srch_select_active_gmm=&srch_FSG_select_active_gmm;
    s->srch_gmm_compute_lv1=&approx_ci_gmm_compute;
    s->srch_gmm_compute_lv2=&s3_cd_gmm_compute_sen;

    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_one_srch_frame_lv2=&srch_FSG_srch_one_frame_lv2;

    s->srch_frame_windup=&srch_FSG_windup;
    s->srch_shift_one_cache_frame=&srch_FSG_shift_one_cache_frame;

    s->srch_utt_end=&srch_FSG_end;

#if 0
    s->srch_gen_hyp=&srch_FSG_gen_hyp;
    s->srch_dump_vithist=&srch_FSG_dump_vithist;

    s->srch_gen_dag=NULL;
#endif

  }else if(op_mode==OPERATION_FLATFWD){

    s->srch_init=&srch_FLAT_FWD_init;
    s->srch_uninit=&srch_FLAT_FWD_uninit;
    s->srch_utt_begin=&srch_FLAT_FWD_begin;
    s->srch_utt_end=&srch_FLAT_FWD_end;

    s->srch_set_lm=&srch_FLAT_FWD_set_lm;
    s->srch_add_lm=&srch_FLAT_FWD_add_lm;
    s->srch_delete_lm=&srch_FLAT_FWD_delete_lm;

    s->srch_select_active_gmm=&srch_FLAT_FWD_select_active_gmm;
    s->srch_gmm_compute_lv1=&approx_ci_gmm_compute;
    s->srch_gmm_compute_lv2=&s3_cd_gmm_compute_sen;

    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_eval_beams_lv2=&srch_debug_eval_beams_lv2;


    s->srch_one_srch_frame_lv2=&srch_FLAT_FWD_srch_one_frame_lv2;
    s->srch_frame_windup=&srch_FLAT_FWD_frame_windup;
    s->srch_shift_one_cache_frame=&srch_FLAT_FWD_shift_one_cache_frame;

#if 0
    s->srch_gen_hyp=&srch_FLAT_FWD_gen_hyp;
    s->srch_gen_dag=&srch_FLAT_FWD_gen_dag;
    s->srch_dump_vithist=&srch_FLAT_FWD_dump_vithist;
    s->srch_bestpath_impl=&srch_FLAT_FWD_bestpath_impl;
    s->srch_dag_dump=&srch_FLAT_FWD_dag_dump;
#endif

  }else if(op_mode==OPERATION_TST_DECODE){


    if(!cmd_ln_int32("-composite")){
      E_ERROR("Full triphone expansion is not supported at this point.\n");
      return NULL;
    }

    s->srch_init=&srch_TST_init;
    s->srch_uninit=&srch_TST_uninit;
    s->srch_utt_begin=&srch_TST_begin;
    s->srch_utt_end=&srch_TST_end;

    s->srch_set_lm=&srch_TST_set_lm;
    s->srch_add_lm=&srch_TST_add_lm;
    s->srch_delete_lm=&srch_TST_delete_lm;

    s->srch_select_active_gmm=&srch_TST_select_active_gmm;
    s->srch_gmm_compute_lv1=&approx_ci_gmm_compute;
    s->srch_gmm_compute_lv2=&s3_cd_gmm_compute_sen_comp;

    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_eval_beams_lv2=&srch_debug_eval_beams_lv2;

    s->srch_hmm_compute_lv2=&srch_TST_hmm_compute_lv2;
    s->srch_propagate_graph_ph_lv2=&srch_TST_propagate_graph_ph_lv2;
    s->srch_propagate_graph_wd_lv2=&srch_TST_propagate_graph_wd_lv2;

    s->srch_compute_heuristic=&srch_TST_compute_heuristic;
    s->srch_frame_windup=&srch_TST_frame_windup;
    s->srch_shift_one_cache_frame=&srch_TST_shift_one_cache_frame;

#if 0
    s->srch_gen_hyp=&srch_TST_gen_hyp;
    s->srch_dump_vithist=&srch_TST_dump_vithist;
    s->srch_gen_dag=&srch_TST_gen_dag;
    s->srch_bestpath_impl=&srch_TST_bestpath_impl;
    s->srch_dag_dump=&srch_TST_dag_dump;
#endif

  }else if(op_mode==OPERATION_WST_DECODE){

    E_INFO("You have used a function which is still under development\n");
    E_FATAL("Word Conditioned Tree Search is still under development. It is now fended off from the users.");

    s->srch_init=&srch_WST_init;
    s->srch_uninit=&srch_WST_uninit;
    s->srch_utt_begin=&srch_WST_begin;
    s->srch_utt_end=&srch_WST_end;
    s->srch_set_lm=&srch_WST_set_lm;
    s->srch_add_lm=&srch_TST_add_lm;
    s->srch_delete_lm=&srch_TST_delete_lm;

    s->srch_select_active_gmm=&srch_WST_select_active_gmm;
    s->srch_gmm_compute_lv1=&approx_ci_gmm_compute;
    s->srch_gmm_compute_lv2=&s3_cd_gmm_compute_sen_comp;

    s->srch_eval_beams_lv2=&srch_debug_eval_beams_lv2;
    /*    s->srch_rescoring=&srch_WST_rescoring;*/

    s->srch_hmm_compute_lv1=&srch_debug_hmm_compute_lv1;
    s->srch_eval_beams_lv1=&srch_debug_eval_beams_lv1;
    s->srch_propagate_graph_ph_lv1=&srch_debug_propagate_graph_ph_lv1;
    s->srch_propagate_graph_wd_lv1=&srch_debug_propagate_graph_wd_lv1;

    s->srch_hmm_compute_lv2=&srch_WST_hmm_compute_lv2;
    s->srch_propagate_graph_ph_lv2=&srch_WST_propagate_graph_ph_lv2;

    /*    s->srch_propagate_graph_wd_lv2=&srch_WST_propagate_graph_wd_lv2_with_rescoring;*/

    s->srch_propagate_graph_wd_lv2=&srch_WST_propagate_graph_wd_lv2;

    s->srch_compute_heuristic=&srch_WST_compute_heuristic;
    s->srch_frame_windup=&srch_WST_frame_windup;
    s->srch_shift_one_cache_frame=&srch_WST_shift_one_cache_frame;

#if 0
    /* Yes, use TST gen DAG to avoid duplication */
    s->srch_gen_hyp=&srch_TST_gen_hyp;
    s->srch_dump_vithist=&srch_TST_dump_vithist;
    s->srch_gen_dag=&srch_TST_gen_dag; 
    s->srch_bestpath_impl=&srch_TST_bestpath_impl;
    s->srch_dag_dump=&srch_TST_dag_dump;
#endif

  }else if(op_mode==OPERATION_DEBUG){

    s->srch_init=&srch_debug_init;
    s->srch_uninit=&srch_debug_uninit;
    s->srch_utt_begin=&srch_debug_begin;
    s->srch_utt_end=&srch_debug_end;
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

    s->srch_compute_heuristic=&srch_debug_compute_heuristic;
    s->srch_frame_windup=&srch_debug_frame_windup;
    s->srch_shift_one_cache_frame=&srch_debug_shift_one_cache_frame;

    s->srch_gen_hyp=&srch_debug_gen_hyp;
    s->srch_dump_vithist=&srch_debug_dump_vithist;
    s->srch_gen_dag=&srch_debug_gen_dag;
    s->srch_bestpath_impl=&srch_debug_bestpath_impl;
    s->srch_dag_dump=&srch_debug_dag_dump;
  }else{
    E_ERROR("Unknown mode %d, failed to initialized srch_t\n",op_mode);

  }

  srch_assert_funcptrs(s);
  
  /* Do general search initialization here. */
  s->stat=kb->stat;
  s->ascr=kb->ascr;
  s->vithist=kb->vithist;
  s->lathist=kb->lathist;
  s->beam=kb->beam;
  s->fastgmm=kb->fastgmm;
  s->pl=kb->pl;
  s->adapt_am=kb->adapt_am;
  s->kbc=kb->kbcore;

  s->matchfp=kb->matchfp;
  s->matchsegfp=kb->matchsegfp;
  s->hmmdumpfp=kb->hmmdumpfp;



  str=srch_mode_index_to_str(op_mode);

  /* FIXME! Do search-specific checking here. In a true OO
     programming, this should not happen. This happens because we have
     duplicated code. */
  if(op_mode==OPERATION_TST_DECODE||op_mode==OPERATION_WST_DECODE){
    if(s->kbc->lmset==NULL||s->vithist==NULL){      
      E_INFO("lmset is NULL and vithist is NULL in op_mode %s, wrong operation mode?\n",str);
      goto check_error;
    }
  }

  if(op_mode==OPERATION_FLATFWD){
    if(s->kbc->lmset==NULL||s->lathist==NULL){      
      E_INFO("lmset is NULL and lathist is NULL in op_mode %s, wrong operation mode?\n",str);
      goto check_error;
    }
  }

  if(op_mode==OPERATION_GRAPH){
    if(!cmd_ln_str("-fsg")&&!cmd_ln_str("-fsgfile")){
      E_INFO("-fsg and -fsgfile are not specified in op_mode %s, wrong operation mode?\n",str);
      goto check_error;
    }

  }
  /* Do search-specific initialization here. */ 

  if(s->srch_init(kb,s)==SRCH_FAILURE){
    E_INFO("search initialization failed for op-mode %d\n",op_mode);
    goto check_error;
  }

  ckd_free(str);
  return s;

check_error:
  ckd_free(str);
  return NULL;

}

int32 srch_utt_begin(srch_t* srch){

  int32 i;
  if(srch->srch_utt_begin==NULL){
    E_ERROR("srch->srch_utt_begin is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  srch->num_frm=0;
  srch->num_segs=0;
  for(i=0;i<srch->ascale_sz;i++)
    srch->ascale[i]=0;

  for(i=0;i<srch->segsz_sz;i++)
    srch->segsz[i]=0;

  srch->srch_utt_begin(srch);

  
  return SRCH_SUCCESS;
}

  /* Several things we want to take care in srch_utt_end 
     1, (Optionally) Generate the history table. 
     2, (Optionally) Generate the best senone score. 
     3, Generate hypothesis.
     4, Display the hypothesis.
     5, (Optionally) If enerate the DAG. 
     6, (Optionally) Do best path search  <- Given 5 is done
     7, (Optionally) Confidence estimation based on Lattice <- Given 5 is done. 
     Result could come from either 1 or 6.  
     8, Then, finally of course you want to clean up. 
     The following essentially follows the above procedure. 

     The first 1-7 will be mainly taken by function pointers supplied
     by different modes.

     The first 8 will be taken care by s->srch_utt_end. That is very
     different from the past because s->srch_utt_end takes of
     everything.  
   */

int32 srch_utt_end(srch_t* srch){

  glist_t hyp, bphyp;
  dag_t *dag;

  hyp=bphyp=NULL;
  dag=NULL;

  if(srch->srch_utt_end==NULL){
    E_ERROR("srch->srch_utt_end is NULL. Please make sure it is set.\n");
    return SRCH_FAILURE;
  }

  return srch->srch_utt_end(srch);
}

int32 srch_utt_decode_blk(srch_t* s, float ***block_feat, int32 block_nfeatvec, int32 *curfrm)
{
  /*This is the basic backbone of the search. */
  stat_t *st;
  int32 frmno, f, t;
  int32 win_efv;
  st = s->stat;

  frmno = *curfrm;

  /* Overriding this implementation. Use search implementation
     provided search abstraction routine instead of the default search
     abstraction */
  if(s->srch_decode!=NULL){
    return s->srch_decode((void*)s);
  }
   
  /* Else, go over the default search abstraction*/

  /* the effective window is the min of (s->cache_win, block_nfeatvec) */
  win_efv = s->cache_win;
  if(win_efv > block_nfeatvec) 
    win_efv = block_nfeatvec;


  s->num_frm =frmno;  /* Make a copy to the structure */


  /* 20060413 ARCHAN: Bug Fix 1459402, the old segments failed to
     increase the size of the memory buffer properly
  */

  if (block_nfeatvec + frmno >= s->ascale_sz){
    s->ascale= (int32*)
      ckd_realloc(s->ascale,(s->ascale_sz+DFLT_UTT_SIZE) *
		  sizeof(int32));
    s->ascale_sz+=DFLT_UTT_SIZE;
  }
  
  if (s->num_segs >= s->segsz_sz) {
    s->segsz= (int32*)
      ckd_realloc(s->segsz,(s->segsz_sz+DFLT_NUM_SEGS) *
		  sizeof(int32));
    s->segsz_sz+=DFLT_NUM_SEGS;
  }

  s->segsz[s->num_segs] = win_efv;
  s->num_segs++;

#if 0
s->segsz[s->num_segs] = win_efv;
s->num_segs++;

if(frmno+win_efv > s->ascale_sz){
s->ascale= (int32 *)
ckd_realloc(s->ascale,(s->ascale_sz+DFLT_UTT_SIZE));
s->ascale_sz+=DFLT_UTT_SIZE;
}

if(s->num_segs==s->segsz_sz){
s->segsz= (int32* )
ckd_realloc(s->segsz,(s->segsz_sz+DFLT_NUM_SEGS));
s->segsz_sz+=DFLT_NUM_SEGS;
}
#endif

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

    /* Acoustic (senone scores) evaluation */
    ptmr_start (&(st->tm_sen));
    s->srch_select_active_gmm(s);
    s->srch_gmm_compute_lv2(s,block_feat[t],t);
    s->ascale[s->num_frm+t]=s->senscale;

    ptmr_stop (&(st->tm_sen));

    
    /* Propagate graph at phoneme (hmm) level */
    ptmr_start (&(st->tm_srch));


    if(s->srch_one_srch_frame_lv2!=NULL){ /* If user provided only how
					     to search for one frame, then
					     use it instead of going through
					     the standard abstraction. 
					  */
      s->srch_one_srch_frame_lv2(s);
      
    }else{
      /* Determine which set of phonemes should be active in next stage
	 using the lookahead information*/
      /* This should be part of hmm_compute_lv1 */
      if(COMPUTE_HEURISTIC) s->srch_compute_heuristic(s,win_efv);
      /* HMM compute Lv 2, currently, this routine compute hmm for the
       *  data structure and compute the beam. */
      s->srch_hmm_compute_lv2(s,frmno);
      
      /* After the HMM scores are computed, tokens are propagate in the
       * phone-level.  */
      if(s->srch_propagate_graph_ph_lv2(s,frmno)!=SRCH_SUCCESS){
	E_ERROR("Code failed in srch_propagate_graph_ph_lv2\n");
	return SRCH_FAILURE;
      }
      
      /* Rescoring. Usually happened at the word end.  */
      if(s->srch_rescoring!=NULL)
	s->srch_rescoring(s,frmno);
      
      /* Propagate the score on the word-level */
      if(s->srch_propagate_graph_wd_lv2(s,frmno)!=SRCH_SUCCESS){
	E_ERROR("Code failed in srch_propagate_graph_wd_lv2\n");
	return SRCH_FAILURE;
      }
    }
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
    
    if(frmno%10==0){
      fprintf(stdout,".");
      fflush(stdout);
    }
  }
  fprintf(stdout,"\n");

  st->nfr += block_nfeatvec;
  
  *curfrm = frmno;

  return SRCH_SUCCESS;
}


/** Wrap up the search routine*/
int32 srch_uninit(srch_t* srch){
  if(srch->srch_uninit==NULL){
    E_ERROR("Search un-initialization failed\n");
    return SRCH_FAILURE;
  }
  srch->srch_uninit(srch);

  ckd_free(srch->segsz);
  ckd_free(srch->ascale);
  ckd_free(srch->grh);
  ckd_free(srch);

  return SRCH_SUCCESS;
}

/** Wrap up the search report routine*/
void srch_report(srch_t* srch){

  char *op_str;
  op_str=srch_mode_index_to_str(srch->op_mode);

  E_INFO_NOFN("Initialization of srch_t, report:\n");
  E_INFO_NOFN("Operation Mode = %d, Operation Name = %s\n",srch->op_mode,op_str);
  E_INFO_NOFN("\n");

  ckd_free(op_str);
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

#if 1 /* Currently only used by tree searches*/
void reg_result_dump (srch_t* s, int32 id )
{
  FILE *fp, *latfp, *bptfp;
  glist_t hyp;
  glist_t ghyp,rhyp;
  stat_t* st; 
  gnode_t *gn;
  srch_hyp_t *bph, *tmph;
  int32 bp;
  float32 *f32arg;
  float64 lwf;
  dag_t *dag;
  int32 nfrm;
  dict_t *dict;
  
  st= s->stat;
  fp = stderr;
  dag=NULL;

  dict=s->kbc->dict;

  hyp=ghyp=rhyp=NULL;

  if (cmd_ln_str("-bptbldir")) {
    char file[8192];
    sprintf (file, "%s/%s.bpt",cmd_ln_str("-bptbldir") , s->uttid);
    if ((bptfp = fopen (file, "w")) == NULL) {
      E_ERROR("fopen(%s,w) failed; using stdout\n", file);
      bptfp = stdout;
    }

    if(s->vithist)
     vithist_dump (s->vithist, -1, s->kbc, bptfp);
    
    if (bptfp != stdout)
	fclose (bptfp);
  }

  if(s->vithist){
    assert(id>=0);
    hyp = vithist_backtrace (s->vithist, id, dict);
  }


  /* Detailed backtrace */
  if (cmd_ln_int32("-backtrace")) {
    fprintf (fp, "\nBacktrace(%s)\n", s->uttid);
    match_detailed(fp, hyp, s->uttid, "FV", "fv", s->ascale, kbcore_dict(s->kbc));
  }
  
  /* Match */
  if (s->matchfp)
    match_write (s->matchfp, hyp, s->uttid,kbcore_dict(s->kbc), NULL);
  match_write(fp, hyp, s->uttid, kbcore_dict(s->kbc),"\nFWDVIT: ");
  
  /* Matchseg */
  if (s->matchsegfp)
    matchseg_write (s->matchsegfp, hyp, s->uttid, NULL,cmd_ln_int32("-hypsegfmt"),
		    kbcore_lm(s->kbc), kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
		    cmd_ln_int32("-hypsegscore_unscale")
		    );
  matchseg_write (fp, hyp, s->uttid, "FWDXCT: ",cmd_ln_int32("-hypsegfmt"), 
		  kbcore_lm(s->kbc), kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
		  cmd_ln_int32("-hypsegscore_unscale")

		  );
  fprintf (fp, "\n");

  if(cmd_ln_str("-bestsenscrdir")){
    int32 ispipe;
    char str[2048];
    FILE* bsfp;
    sprintf (str, "%s/%s.bsenscr",
	     cmd_ln_str("-bestsenscrdir"), s->uttid);
    E_INFO("Dumping the Best senone scores.\n");
    if ((bsfp = fopen_comp (str, "w", &ispipe)) == NULL)
      E_ERROR("fopen_comp (%s,w) failed\n", str);
    else{
      write_bestsenscore(s);
      fclose_comp (bsfp, ispipe);
    }
  }
      
  if(cmd_ln_str ("-outlatdir")) {

    int32 ispipe;
    char str[2048];
    sprintf (str, "%s/%s.%s",
	     cmd_ln_str("-outlatdir"), s->uttid, cmd_ln_str("-latext"));
    
    E_INFO("Writing lattice file: %s\n", str);
    
    if ((latfp = fopen_comp (str, "w", &ispipe)) == NULL)
      E_ERROR("fopen_comp (%s,w) failed\n", str);
    else {


      /* Write header info */
      /* Do link and unlink silences at here */
      dag_write_header(latfp,st->nfr, 0); /* Very fragile, if 1 is specifed, 
					     the code will just be stopped */
					     
      vithist_dag_write (s->vithist, hyp, dict,
			 cmd_ln_int32("-outlatoldfmt"), latfp, 
			 cmd_ln_int32("-outlatfmt")==OUTLATFMT_IBM);
      fclose_comp (latfp, ispipe);

      /* This should be moved out from the functions and allow polymorphism*/
      /* Reread the lattice and create rescore it */
      bp=cmd_ln_int32("-bestpath");

      if(bp){
	f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
	lwf = f32arg ? ((*f32arg) / *((float32 *) cmd_ln_access ("-lw"))) : 1.0;


	if(( nfrm= s3dag_dag_load(&dag,lwf,
				  str,
				  dict,
				  kbcore_fillpen(s->kbc)))
	    >=0 ){


	  /** Link back silences */
	  linksilences(kbcore_lm(s->kbc),s->kbc,kbcore_dict(s->kbc));

	  bph=dag_search (dag,s->uttid, 
			  lwf,
			  dag->final.node,
			  dict,
			  kbcore_lm(s->kbc),
			  kbcore_fillpen(s->kbc)
			  );

	  if(bph!=NULL){
	    ghyp=NULL;
	    for(tmph= bph ; tmph ; tmph = tmph->next)
	      ghyp=glist_add_ptr(ghyp,(void*)tmph);
	    
	    rhyp=glist_reverse(ghyp);
	    if ( cmd_ln_int32("-backtrace") )
	      match_detailed (stdout, rhyp, s->uttid, "BP", "bp",s->ascale, kbcore_dict(s->kbc));

	  }else{
	    E_ERROR("%s: Bestpath search failed; using Viterbi result\n", s->uttid);
	    rhyp=hyp;
	  }

	  match_write(stdout,rhyp,s->uttid, kbcore_dict(s->kbc), "BSTPTH: ");
	  matchseg_write (stdout, rhyp, s->uttid, "BSTXCT: ",cmd_ln_int32("-hypsegfmt"), 
			  kbcore_lm(s->kbc), kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
			  cmd_ln_int32("-hypsegscore_unscale")
			  );

	  if(ghyp)
	    glist_free(ghyp);

	  /** unlink silences again */
	  unlinksilences(kbcore_lm(s->kbc),s->kbc,kbcore_dict(s->kbc));

	}else{
	  E_ERROR("DAG search (%s) failed\n",s->uttid);
	  bph=NULL;
	}

	if(dag)
	  dag_destroy (dag);

	lm_cache_stats_dump (kbcore_lm(s->kbc));
	lm_cache_reset (kbcore_lm(s->kbc));

      }

      /* If IBM format is required to dumped */
      if(cmd_ln_int32("-outlatfmt")==OUTLATFMT_IBM){
	dag=dag_load(str,
		     cmd_ln_int32("-maxedge"),
		     cmd_ln_float32("-logbase"),
		     0,  /** No fudge added */
		     dict,
		     kbcore_fillpen(s->kbc));
		
	if(dag!=NULL){

	  word_graph_dump(cmd_ln_str("-outlatdir"),s->uttid,cmd_ln_str("-latext"),
			  dag, dict, kbcore_lm(s->kbc), s->ascale);
	  
	}else{
	  E_ERROR("DAG conversion (%s) failed\n",s->uttid);
	}

	if(dag){
	  dag_destroy (dag);
	}
      }
    }
  }
      

  /** free the list containing hyps */
  for (gn = hyp; gn; gn = gnode_next(gn)) {
    ckd_free(gnode_ptr(gn));
  }
  glist_free(hyp);

}
#endif

void write_bstsenscr(FILE *fp,int32 numframe, int32* scale)
{

  int32 i;
  fprintf(fp,"NumFrame %d\n",numframe);
  for(i=0;i<numframe;i++)
    fprintf(fp,"%d %d\n",i,scale[i]);
}

static void write_bestsenscore(srch_t *s)
{
  int32 ispipe;
  char str[2048];
  FILE* bsfp;

  E_INFO("Dumping the Best senone scores.\n");
  sprintf (str, "%s/%s.bsenscr",
	   cmd_ln_str("-bestsenscrdir"), s->uttid);

  if ((bsfp = fopen_comp (str, "w", &ispipe)) == NULL)
    E_ERROR("fopen_comp (%s,w) failed\n", str);
  else{
    write_bstsenscr(bsfp,s->stat->nfr,s->ascale);
    fclose_comp (bsfp, ispipe);
  }
}

static void display_dump_hypothesis (srch_t *s, glist_t hyp, char* hyptag, char* hypsegtag, char* shortformCap, char* shortformSmall)
{
  FILE *fp;

  fp=stderr;
  
  if (cmd_ln_int32("-backtrace")) {
    fprintf (fp, "\nBacktrace(%s)\n", s->uttid);
    match_detailed(fp, hyp, s->uttid, shortformCap, shortformSmall, s->ascale, kbcore_dict(s->kbc));
  }
  
  /* Match */
  if (s->matchfp)
    match_write (s->matchfp, hyp, s->uttid,kbcore_dict(s->kbc), NULL);
  fprintf(fp,"\n");
  match_write(fp, hyp, s->uttid, kbcore_dict(s->kbc),hyptag);
  
  /* Matchseg */
  if (s->matchsegfp)
    matchseg_write (s->matchsegfp, hyp, s->uttid, NULL,cmd_ln_int32("-hypsegfmt"),
		    kbcore_lm(s->kbc), kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
		    cmd_ln_int32("-hypsegscore_unscale")
		    );
  matchseg_write (fp, hyp, s->uttid, hypsegtag,cmd_ln_int32("-hypsegfmt"), 
		  kbcore_lm(s->kbc), kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
		  cmd_ln_int32("-hypsegscore_unscale")

		  );
  fprintf (fp, "\n");
}

#if 0
int32 srch_bestpath_srch(srch_t *srch,dag_t *dag)
{
  float32 *f32arg;
  float64 lwf;

  f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
  lwf = f32arg ? ((*f32arg) / *((float32 *) cmd_ln_access ("-lw"))) : 1.0;
  
  bph=dag_search (dag,srch->uttid, 
		  lwf,
		  dag->final.node,
		  dict,
		  kbcore_lm(srch->kbc),
		  kbcore_fillpen(srch->kbc)
		  );

  if(bph!=NULL){
    ghyp=NULL;
    for(tmph= bph ; tmph ; tmph = tmph->next)
      ghyp=glist_add_ptr(ghyp,(void*)tmph);
	    
    rhyp=glist_reverse(ghyp);
    return rhyp;
  }else{
    return NULL;
  }
}

#endif



#if 0 /* Insert this part of the code when score is needed to be dumped */
    /* This should be written as a small function*/
    int32 ind;

    if(t==0){
      ms_mgau_model_t *mg;
      gauden_t *g;
      mg=kbcore_ms_mgau(s->kbc);
      g=mg->g;
      E_INFO("Time %d, Accum. Time %d, Segments %d Senscale %d. \n", t, s->num_frm+t, s->num_segs-1, s->senscale);
      for(ind=0;ind<g->n_mgau;ind++){
	
	if(s->ascr->sen_active[ind]){
#if 0	  
	  int32 i,j;
	  gauden_dump_ind(g,ind);

	  printf("Feature vector:\n");
	  for(i=0;i<g->n_feat;i++){
	    for(j=0;j<g->featlen[i];j++){
	      printf("%f ",block_feat[t][i][j]);
	    }
	  }
	  printf("\n");
	  fflush(stderr);
#endif
	  E_INFO("Time %d Ind %d Active %d Senscr %d %f actual %f (Unnorm) %d %f actual %f \n", t, ind, s->ascr->sen_active[ind],
		 s->ascr->senscr[ind], logs3_to_log(s->ascr->senscr[ind]), 
		 exp(logs3_to_log(s->ascr->senscr[ind])),
		 s->ascr->senscr[ind] + s->senscale, logs3_to_log(s->ascr->senscr[ind] + s->senscale),
		 exp(logs3_to_log(s->ascr->senscr[ind] + s->senscale))
	       );
	}
      }
    }
#endif


/* Some draft code for 2nd stage: Disabled at this point. */

#if 0 /** Refactor code that should be used, but I just don't have time to test them */
  /*  1, (Optionally) Generate the history table. */
  if(cmd_ln_str("-bptbldir"))
    srch->srch_dump_vithist(srch);

  /* 2, (Optionally) Generate the best senone score. */
  if(cmd_ln_str("-bestsenscrdir"))
    write_bestsenscore(srch);

  /* 3, Generate hypothesis.*/
  hyp=srch->srch_gen_hyp(srch);

  /* 4, Display the hypthesis */
  if(hyp==NULL)
    E_WARN("No recognition result\n");

  display_dump_hypothesis(srch,hyp,"FWDVIT: ","FWDXCT: ","FV","fv");

  if(srch->op_mode==OPERATION_GRAPH){
    E_WARN("Currently mode 2 (FSG decoding) doesn't support DAG generation.\n");
    return srch->srch_utt_end(srch);
  }

  /*5, (Optionally) Generate the DAG. */
  if(cmd_ln_int32("-bestpath") ||   /* Generate lattice for best path search */
     cmd_ln_int32("-confidence")||  /* Generate lattice for confidence scoring. */
     cmd_ln_str("-outlatdir")       /* Generate lattice for the sake of generating lattice */
     ){

#if 1
    /* Hack! */
    /* Take care of the situation where mode 4 and 5 requires lattice
       write back to the harddiscs . That is if the users need to use bestpath
    */
    if(
       (cmd_ln_int32("-bestpath")|| cmd_ln_int32("-confidence"))&&
       cmd_ln_str("-outlatdir")==NULL &&
       (srch->op_mode==OPERATION_TST_DECODE ||  srch->op_mode==OPERATION_WST_DECODE)
       )
      E_FATAL("In TST and WST search (Mode 4 or 5), specifying -bestpath and -confidence also require -outlatdir");
#endif    

    dag=srch->srch_gen_dag(srch,hyp);
  }  



  /* 6, (Optionally) Do best path search  <- Given 5 is done*/
  if(cmd_ln_int32("-bestpath")){

    bphyp=srch->srch_bestpath_impl(srch,dag);
    
    if(bphyp==NULL) {
      E_ERROR("%s: Bestpath search failed; using Viterbi result\n", srch->uttid);
      bphyp=hyp;
    }

    display_dump_hypothesis(srch, bphyp, "BSTPTH: ","BSTXCT: ","BP","bp");

    E_INFO("LM Cache After bestpath search\n");
    lm_cache_stats_dump (kbcore_lm(srch->kbc));
    lm_cache_reset (kbcore_lm(srch->kbc));

  }
  
  /*7, (Optionally) Confidence estimation based on Lattice <- Given 5 is done. */
  if(cmd_ln_int32("-confidence")){
  }


  if(cmd_ln_int32("-outlatdir")){
    if(cmd_ln_int32("-outlatfmt")==OUTLATFMT_IBM){

      if(srch->op_mode==OPERATION_FLATFWD){
	E_WARN("Not dumping IBM lattice file format at this point\n");
      }else{
	if(dag!=NULL)
	  word_graph_dump(cmd_ln_str("-outlatdir"),srch->uttid,cmd_ln_str("-latext"),
			  dag, kbcore_dict(srch->kbc), kbcore_lm(srch->kbc), srch->ascale);	
	else
	  E_ERROR("DAG conversion (%s) failed\n",srch->uttid);
      }
    }else if (cmd_ln_int32("-outlatfmt")==OUTLATFMT_SPHINX3){

      if(
	 (cmd_ln_int32("-bestpath")|| cmd_ln_int32("-confidence"))&&
	 cmd_ln_str("-outlatdir")==NULL &&
	 (srch->op_mode==OPERATION_TST_DECODE ||  srch->op_mode==OPERATION_WST_DECODE)
	 )
	{
	  
	/* Do nothing, because the code has already dumped the lattice
	   as intermediates. 
	*/
	}else{
	  srch->srch_dag_dump(srch,hyp);
	}
      
    }else{
      E_ERROR("Unknown format ,do nothing in dumping files out\n");
    }
  }


  if(hyp!=NULL) {
    /** free the list containing hyps */
    for (gn = hyp; gn; gn = gnode_next(gn))
      ckd_free(gnode_ptr(gn));
    glist_free(hyp);
  }
  if(bphyp!=NULL) {
    for (gn = bphyp; gn; gn = gnode_next(gn))
      ckd_free(gnode_ptr(gn));
    glist_free(bphyp);
  } 
  if(dag!=NULL) 
    dag_destroy(dag);

  return srch->srch_utt_end(srch);
#endif
