/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * flat_fwd.c -- Forward Viterbi beam search
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 28-Jul-04    ARCHAN (archan@cs.cmu.edu at Carnegie Mellon Unversity 
 *              First incorporate it from s3 code base. 
 *
 * $Log$
 * Revision 1.14  2006/02/23  05:36:23  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH.  this is a version where function except search are moved to other files.
 * 
 *
 * Revision 1.13  2005/10/05 00:31:14  dhdfu
 * Make int8 be explicitly signed (signedness of 'char' is
 * architecture-dependent).  Then make a bunch of things use uint8 where
 * signedness is unimportant, because on the architecture where 'char' is
 * unsigned, it is that way for a reason (signed chars are slower).
 *
 * Revision 1.12.4.12  2006/02/17 19:30:45  arthchan2003
 * Added specific version of dag_add_fudge_edges, flat_fwd_add_fudge_edges into flat_fwd.c . When using mode3, ascr and lscr need to be gathered using specific function lat_seg_ascr_lscr.  The one in lattice could well be S3_LOGPROB_ZERO and caused a lot of problem in the lattices.
 *
 * Revision 1.12.4.11  2005/11/17 06:27:48  arthchan2003
 * 1, Clean up. 2, removed fwg in dag_build.
 *
 * Revision 1.12.4.10  2005/10/26 03:53:12  arthchan2003
 * Put add_fudge and remove_filler_nodes into srch_flat_fwd.c . This conformed to s3.0 behavior.
 *
 * Revision 1.12.4.9  2005/09/25 19:22:07  arthchan2003
 * Instead of using the real left context, use the compressed ID for left context in expansion.  This effective saves 25%-50% from the dumb implementation. Not tested yet.
 *
 * Revision 1.12.4.8  2005/09/18 01:18:24  arthchan2003
 * Only retain processing of the array whmm_t in flat_fwd.[ch]
 *
 * Revision 1.12.4.7  2005/09/11 02:58:10  arthchan2003
 * remove most dag-related functions except dag_build. Use latticehist_t insteads of loosed arrays.
 *
 * Revision 1.12.4.6  2005/09/07 23:40:06  arthchan2003
 * Several Bug Fixes and Enhancements to the flat-lexicon
 * 1, Fixed Dox-doc.
 * 2, Add -worddumpef and -hmmdumpef in parrallel to -worddumpsf and
 * -hmmdumpsf. Usage is trivial. a structure called fwd_dbg_t now wrapped
 * up all these loose parameters.  Methods of fwd_dbg are implemented.
 * 3, word_ugprob is now initialized by init_word_ugprob
 * 4, Full-triphone expansion is implemented. User can change this
 * behavior by specifying -multiplex_multi and -multiplex_single. The
 * former turn on multiplex triphone for word-begin for multi-phone word.
 * The latter do that for single-phone word. Turning off both could
 * tremendously increase computation.
 * 5, Word expansions of possible right contexts now records independent
 * history.  The behavior in the past was to use only one history for a
 * word.
 *
 * Revision 1.12.4.5  2005/08/02 21:12:45  arthchan2003
 * Changed senlist from 8-bit to 32-bit. It will be compatible to the setting of ascr's sen_active.
 *
 * Revision 1.12.4.4  2005/07/26 02:20:39  arthchan2003
 * merged hyp_t with srch_hyp_t.
 *
 * Revision 1.12.4.3  2005/07/20 21:13:16  arthchan2003
 * Some small clean-up of the code. Use cmd_ln_* instead of cmd_ln_access
 *
 * Revision 1.12.4.2  2005/07/17 05:44:31  arthchan2003
 * Added dag_write_header so that DAG header writer could be shared between 3.x and 3.0. However, because the backtrack pointer structure is different in 3.x and 3.0. The DAG writer still can't be shared yet.
 *
 * Revision 1.12.4.1  2005/07/15 07:50:32  arthchan2003
 * Remove hmm computation and context building code from flat_fwd.c.
 *
 * Revision 1.12  2005/06/21 22:41:32  arthchan2003
 * Log. 1, Removal of several functions of dag_t, 2, removal of static variable stardwid, finishwid and silwid. They are now all handled by dict.  3, Use the lmset interface (lmset_init). Currently it still doesn't support class-based LM.
 *
 * Revision 1.6  2005/06/19 03:58:16  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.5  2005/06/18 18:17:50  archan
 * Update decode_anytopo such that it also used the lmset interface. Notice it still doesn't support multiple LMs and class-based LM at this point
 *
 * Revision 1.4  2005/06/03 06:45:28  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.3  2005/06/03 05:46:19  archan
 * Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 * Revision 1.2  2005/05/26 22:03:06  archan
 * Add support for backtracking without assuming silence </s> has to be the last word.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:00  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.11  2005/02/09 05:59:30  arthchan2003
 * Sychronize the -option names in slow and faster decoders.  This makes many peopple's lives easier. Also update command-line. make test-full is done.
 *
 * Revision 1.10  2004/12/27 19:46:19  arthchan2003
 * 1, Add perf-std to Makefile.am , developers can type make perf-std as the standard performance test target. This only works in CMU. 2, Fix warning in flat_fwd.[ch], 3, Apply Yitao's change in cmd_ln.c . 4, 2,3 are standard regression tested.
 *
 * Revision 1.9  2004/12/23 21:00:51  arthchan2003
 * 1, Fixed problems in the code of -cepext, 2, Enabled the generic HMM computation routine flat_fwd.c. This is the key problem of the decode_anytopo.
 *
 * Revision 1.8  2004/12/06 10:52:00  arthchan2003
 * Enable doxygen documentation in libs3decoder
 *
 * Revision 1.7  2004/12/05 12:01:30  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.6  2004/11/16 05:13:18  arthchan2003
 * 1, s3cipid_t is upgraded to int16 because we need that, I already check that there are no magic code using 8-bit s3cipid_t
 * 2, Refactor the ep code and put a lot of stuffs into fe.c (should be renamed to something else.
 * 3, Check-in codes of wave2feat and cepview. (cepview will not dump core but Evandro will kill me)
 * 4, Make the same command line frontends for decode, align, dag, astar, allphone, decode_anytopo and ep . Allow the use a file to configure the application.
 * 5, Make changes in test such that test-allphone becomes a repeatability test.
 * 6, cepview, wave2feat and decode_anytopo will not be installed in 3.5 RCIII
 * (Known bugs after this commit)
 * 1, decode_anytopo has strange bugs in some situations that it cannot find the end of the lattice. This is urgent.
 * 2, default argument file's mechanism is not yet supported, we need to fix it.
 * 3, the bug discovered by SonicFoundry is still not fixed.
 *
 * Revision 1.2  2004/11/14 07:00:08  arthchan2003
 * 1, Finally, a version of working flat decoder is completed. It is not compiled in the standard compilation yet because there are two many warnings. 2, eliminate the statics variables in  fe_sigproc.c
 *
 * Revision 1.2  2002/12/03 23:02:38  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 *
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 04-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_chk_linkscr().  Added check for renormalization before bestpath.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Added lwf factoring of fillpen in dag_backtrace().
 * 
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed hardwired MIN_EF_RANGE constant into -min_endfr argument.
 * 		Added fudge edges in dag (dag_add_fudge_edges).
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added exact reporting of word sequence and scores from dag_search.
 * 		For this, added daglink_t.bypass, daglink_t.lscr, daglink_t.src, and
 * 		added bypass argument to dag_link and dag_bypass_link, and changed
 * 		dag_backtrace to find exact best path.
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *  		Added onlynodes argument to dag_dump().
 *  
 * 29-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Applied lwf to filler penalties in dag_remove_filler_nodes().
 *  
 * 28-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Corrected for backoff case of LM score in lat_seg_lscr().
 *  
 * 15-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Undid previous change: now the complete DAG is built whether the bestpath
 * 		search is to be run or not.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Replace explicit silpen and noisepen with calls to fillpen().
 *  
 * 05-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Added pscr_valid flag to daglink_t to avoid evaluating the
 * 		same path mulitple times (millions of times, in some cases).
 *  
 * 27-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Added checks in dag_bestpath and dag_search for dealing with
 * 		zero paths through DAG (caused by introduction of MIN_EF_RANGE).
 *  
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added separate language weight (-bestpathlw) for bestpath DAG search.
 * 		Added MIN_EF_RANGE to limit active nodes in DAG search.  Removed internal
 * 		finishwid nodes from DAG search.
 *  
 * 21-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added and used -bptblsize argument.
 * 		Freed rcscore entries in BP table if not running bestpath search (for
 * 		reducing memory requirement; but causes acoustic scores in dumped
 * 		lattices to be inaccurate).
 *  
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed fwd_sen_active to flag active senones instead of building a list
 * 		of them.
 * 
 * 09-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed (> thresh) to (>= thresh) in word_trans, for consistency.
 * 		Added word_ugprob structure and use in word_trans() to speed up unigram
 * 		cross-word transitions.  (Didn't help that much.)
 * 		Postponed pruning and reclaiming of inactive whmm to whmm_eval, to avoid
 * 		unnecessarily deallocating HMMs, only to allocate them again because of
 * 		an incoming transition.
 * 		Changed tp[][] indices to tp[] in 5-state specific eval_nonmpx_whmm and
 * 		eval_mpx_whmm, again to speed up whmm_eval.  (Helped a bit.)
 * 
 * 06-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Create edges in DAG iff bestpath search being done.  Reduces size of
 * 		dumped lattices, but cannot be used to run bestpath search.
 * 
 * 29-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed output lattice file to include edges and best ending scores.
 * 		Changed input lattice file format to conform to output format.
 * 
 * 26-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: </s> never becomes an active candidate if using an input lattice
 * 		to constrain search AND </s> appears in filler dictionary.
 * 
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added DAG search.
 * 
 * 02-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check (through tg_trans_done) in word_trans to avoid backing off to
 * 		bigram transition w2->w3 if trigram transition w1,w2->w3 already done.
 * 
 * 29-Mar-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added the reporting of no. of triphones mapped to ciphones.  (BUG: The
 * 		number reported is not accurate as it counts the number of such INSTANCES
 * 		for within-word triphones, but only the SET of cross-word triphones.)
 * 
 * 12-Mar-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added checks in eval_mpx_whmm and eval_nonmpx_whmm for detecting
 * 		very poor state scores and flooring them to S3_LOGPROB_ZERO.  Otherwise,
 * 		these scores could overflow and turn +ve.
 * 
 * 26-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Fixed bug in build_wwpid (pointed out by purify) that accessed
 * 		out of bounds memory in the case of single-phone words.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added functionality to constrain search to words in given input lattices.
 * 		This mainly affects the word_trans function.
 * 		Added fwd_sen_active() function.
 * 		Added code to increase lattice[] size (realloc) when it overflows, instead
 * 		of exiting with an error message.
 * 
 * 20-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed allocation of whmm state,latid,pid vectors to block mode
 * 		allocation in whmm_alloc (suggested by Paul Placeway).
 * 
 * 10-Aug-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#define ANY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3types.h>
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "logs3.h"
#include "search.h"
#include "ctxt_table.h"
#include "vithist.h"

#include "dag.h"

#include "srch_flat_fwd.h"

#define WHMM_ALLOC_SIZE 32000

/** \file flat_fwd.c 
    \brief Implementation of forward search in a flat lexicon. 
 */

static int32 exist_left_context(whmm_t **whmm, s3wid_t w,s3cipid_t lc)
{
  whmm_t *h;
  if (whmm[w]) {
    for (h = whmm[w]; h; h = h->next){
      /*      E_INFO("h->lc %d, lc %d w %d\n", h->lc,lc, w);*/
      if(h->lc==lc)
	return 1;
    }
    return 0;
  }else{
    return 0;
  }
}

static int32 exist_left_right_context(whmm_t **whmm, s3wid_t w,s3cipid_t lc, s3cipid_t rc)
{
  whmm_t *h;
  if (whmm[w]) {
    for (h = whmm[w]; h; h = h->next){
      /*      E_INFO("h->lc %d, lc %d w %d\n", h->lc,lc, w);*/
      if(h->lc==lc && h->rc==rc)
	return 1;
    }
    return 0;
  }else{
    return 0;
  }
}

void dump_all_whmm (srch_FLAT_FWD_graph_t *fwg, whmm_t **whmm, int32 n_frm, int32 n_state, int32 *senscr)
{
    s3wid_t w;
    whmm_t *h;
    kbcore_t* kbc;
    tmat_t* tmat;
    dict_t* dict;
    mdef_t *mdef;
    
    kbc=fwg->kbcore;
    tmat=kbcore_tmat(kbc);
    dict=kbcore_dict(kbc);
    mdef=kbcore_mdef(kbc);

    for (w = 0; w < dict->n_word; w++) {
	if (whmm[w]) {
	  for (h = whmm[w]; h; h = h->next){

	    if(dict->word[w].pronlen == 1)
	      assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex_singleph));
	    else
	      assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex));

	    dump_whmm(w,h,senscr,tmat,n_frm,n_state,dict,mdef);
	  }
	}
    }
}

void dump_all_word (srch_FLAT_FWD_graph_t *fwg, whmm_t **whmm, int32 n_state)
{
    s3wid_t w;
    whmm_t *h;
    int32 last, bestlast;
    kbcore_t* kbc;
    tmat_t* tmat;
    dict_t* dict;
    
    kbc=fwg->kbcore;
    tmat=kbcore_tmat(kbc);
    dict=kbcore_dict(kbc);
    
    for (w = 0; w < dict->n_word; w++) {
	if (whmm[w]) {
	    printf ("[%4d] %-24s", fwg->n_frm, dict->word[w].word);

	    last = dict->word[w].pronlen - 1;
	    bestlast = (int32) 0x80000000;
	    
	    for (h = whmm[w]; h; h = h->next) {
		if (h->pos < last)
		    printf (" %9d.%2d", -h->score[n_state-1], h->pos);
		else if (bestlast < h->score[n_state-1])
		    bestlast = h->score[n_state-1];
	    }

	    if (bestlast > (int32) 0x80000000)
		printf (" %9d.%2d", -bestlast, last);

	    printf ("\n");
	}
    }
}


int32 whmm_eval (srch_FLAT_FWD_graph_t *fwg, int32 *senscr, int32 n_state) 
{
    int32 best, cf;
    s3wid_t w;
    whmm_t *h, *nexth, *prevh;
    int32 n_mpx, n_nonmpx;
    kbcore_t* kbc;
    tmat_t* tmat;
    dict_t* dict;
    mdef_t *mdef;
    whmm_t **whmm;
    
    kbc=fwg->kbcore;
    tmat=kbcore_tmat(kbc);
    dict=kbcore_dict(kbc);
    mdef=kbcore_mdef(kbc);
    whmm=fwg->whmm;
    
    best = S3_LOGPROB_ZERO;
    n_mpx = n_nonmpx = 0;
    cf = fwg->n_frm;
    
    for (w = 0; w < dict->n_word; w++) {
	prevh = NULL;
	/*	E_INFO("HIHI %d %d\n",w,n_state);*/
	for (h = whmm[w]; h; h = nexth) {
	  /*	  E_INFO("HIHI Inside\n");*/
	    nexth = h->next;

	    if(dict->word[w].pronlen == 1)
	      assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex_singleph));
	    else
	      assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,fwg->multiplex));

	    /*E_INFO("%d %d\n",h->active,cf); */
	    if (h->active == cf) {

	      if(h->type==MULTIPLEX_TYPE){
		  eval_mpx_whmm(h,senscr,tmat,mdef,n_state);
		  n_mpx++;
	      }else{
		  eval_nonmpx_whmm(h,senscr,tmat,mdef,n_state);
		  n_nonmpx++;
	      }
		
	      /*E_INFO("best %d h->bestscore %d\n",best,h->bestscore); */
	      if (best < h->bestscore)
		best = h->bestscore;

	      prevh = h;

	    } else {
		if (prevh)
		    prevh->next = nexth;
		else
		    whmm[w] = nexth;
		
		whmm_free(h);
	    }
	}
    }

    pctr_increment (fwg->ctr_mpx_whmm, n_mpx);
    pctr_increment (fwg->ctr_nonmpx_whmm, n_nonmpx);
    
    return best;
}

void whmm_renorm (srch_FLAT_FWD_graph_t *fwg, whmm_t **whmm,int32 n_state, int32 bestscr)
{
    s3wid_t w;
    whmm_t *h;
    int32 st;
    dict_t *dict;
    
    dict=fwg->kbcore->dict;
    fwg->renormalized = 1;
    
    for (w = 0; w < dict->n_word; w++)
	for (h = whmm[w]; h; h = h->next)
	    for (st = n_state-2; st >= 0; --st)
		h->score[st] -= bestscr;
}


/**
 * Transition from hmm h into the next appropriate one for word w.
 * Threshold check for incoming score already completed.
 * The next HMM may be the last triphone for the word w, in which case, instantiate
 * multiple instances corresponding cross-word triphone modelling for all right context
 * ciphones.
 */
void whmm_transition (srch_FLAT_FWD_graph_t *fwg, whmm_t** whmm, int32 w, whmm_t *h, int32 n_state, int32 final_state) 
{
    int32 lastpos, npid, nf;
    whmm_t *nexth, *prevh;
    s3cipid_t rc;
    s3pid_t *pid;
    ctxt_table_t* ct_table; 
    dict_t *dict;
    kbcore_t *kbc;
    tmat_t *tmat;
    
    kbc=fwg->kbcore;
    tmat=kbcore_tmat(kbc);
    dict=kbcore_dict(kbc);
    ct_table=fwg->ctxt;

    lastpos = dict->word[w].pronlen - 1;
    nf = fwg->n_frm+1;
    
    if (h->pos < lastpos-1) {
	/*
	 * Transition to word HMM that's not the final one in word.  First, allocate
	 * the HMM if not already present.
	 */
	if ((! h->next) || (h->next->pos != h->pos+1)) {
	    nexth = whmm_alloc (h->pos+1,n_state,WHMM_ALLOC_SIZE,0);

	    nexth->pid = &(ct_table->wwpid[w][nexth->pos]);
	    
	    nexth->next = h->next;
	    h->next = nexth;
	}

	/* Transition to next HMM */
	nexth = h->next;
	if (h->score[final_state] > nexth->score[0]) {
	    nexth->score[0] = h->score[final_state];
	    nexth->history[0] = h->history[final_state];
	    nexth->active = nf;		/* Ensure it doesn't get pruned */
	}
    } else {
	/*
	 * Transition to final HMMs in word, with full cross-word rc modelling.  Allocate
	 * all final triphone HMM instances first.
	 */
	prevh = h;
	get_rcpid (ct_table, w, &pid, &npid, dict);
	
	for (rc = 0; rc < npid; rc++) {
	    if ((! prevh->next) || (prevh->next->rc != rc)) {
		nexth = whmm_alloc (h->pos+1,n_state,WHMM_ALLOC_SIZE,0);

		nexth->rc = rc;
		nexth->pid = &(pid[rc]);
		
		nexth->next = prevh->next;
		prevh->next = nexth;
	    }
	    prevh = prevh->next;
	}

	/* Transition to next HMMs */
	for (rc = 0, nexth = h->next; rc < npid; rc++, nexth = nexth->next) {
	    if (h->score[final_state] > nexth->score[0]) {
		nexth->score[0] = h->score[final_state];
		nexth->history[0] = h->history[final_state];
		nexth->active = nf;	/* Ensure it doesn't get pruned */
	    }
	}
    }
}

/* WARNING, be careful when plug in parameters into this guy*/
void whmm_exit (srch_FLAT_FWD_graph_t *fwg,
		whmm_t **whmm, 
		latticehist_t *lathist, 
		int32 n_state, 
		int32 final_state, 
		int32 thresh, 
		int32 wordthresh, 
		int32 phone_penalty)
{
    s3wid_t w;
    whmm_t *h;
    int32 pronlen, nf;
    dict_t *dict;
    kbcore_t *kbc;
    tmat_t *tmat;
    
    kbc=fwg->kbcore;
    tmat=kbcore_tmat(kbc);
    dict=kbcore_dict(kbc);
    
    nf = fwg->n_frm+1;

    for (w = 0; w < dict->n_word; w++) {
	pronlen = dict->word[w].pronlen;

	for (h = whmm[w]; h; h = h->next) {
	    if (h->bestscore >= thresh) {
		if (h->pos == pronlen-1) {
		  if (h->score[final_state] >= wordthresh){
		    lattice_entry (lathist, w, fwg->n_frm, 
				   h->score[final_state],
				   h->history[final_state],
				   h->rc,
				   fwg->ctxt,
				   dict);
		  }
		} else {
		    if (h->score[final_state]+phone_penalty >= thresh)
			whmm_transition (fwg, whmm, w, h, n_state, final_state);
		}

		h->active = nf;
	    }
	}
    }
}


/* ARCHAN: Ah. this is the part where the magical multiplex-triphone
   is implemented.  This how it works.  When a word is being entered,
   the first phone will be entered, if the score is high enough, then
   pid[0] will be replaced by the best PID.  Now, when we go back
   whmm_eval, the first phone will always be computed as multiplexed.
   Then, the pid will then start to propagate just like the score.
   
   You could just treat pid as something like backtracing
   pointer. Then it should be pretty to understand. 
*/

/**
 * Transition into a word w.  Since we transition into the first phone position, the
 * triphone model must be derived from the incoming left context ciphone.  The first
 * state of the whmm instance inherits this triphone model and propagates it along with
 * the score.
 * If the first phone is also the last (single-phone word), we must also model all
 * possible right context ciphones, by instantiating separate whmm models for each rc.
 *
 *
 */

void word_enter (srch_FLAT_FWD_graph_t *fwg, s3wid_t w, int32 n_state, int32 score, s3latid_t l, s3cipid_t lc)
{
    whmm_t *h, *prevh;
    s3cipid_t b, rc;
    s3pid_t pid, *rpid;
    s3pid_t *pidp;
    int32 s, npid, nf;
    kbcore_t* kbc;
    tmat_t* tmat;
    dict_t* dict;
    ctxt_table_t* ct_table;
    s3senid_t *lcmap;

    whmm_t **whmm;

    kbc=fwg->kbcore;
    dict=kbcore_dict(kbc);
    tmat=kbcore_tmat(kbc);
    ct_table=fwg->ctxt;
    whmm=fwg->whmm;

    assert(whmm);
    assert(kbc);
    assert(dict);
    assert(tmat);

    nf = fwg->n_frm+1;
    
    b = dict->word[w].ciphone[0];
    lcmap=get_lc_cimap(ct_table, w, dict);
    /*
      ARCHAN: Though the term rc and lc is used at here.  Be very
      careful that, when they were used in CI-phone, the corresponding
      SSID may not be unique.  While, using what one could find in the
      context of lc_cimap. Things are always unique. The contents in
      the hmm should always be stored using the compressed ID. This
      will make computation be more efficient. 
     */

    /*    E_INFO("word enter\n");*/
    if (dict->word[w].pronlen > 1) {	/* Multi-phone word; no right context problem */

      rc = dict->word[w].ciphone[1];

      pidp = &(ct_table->lcpid[b][rc].pid[ct_table->lcpid[b][rc].cimap[lc]]);
      pid=*(pidp);
      
      if(fwg->multiplex){
	if ((! whmm[w]) || (whmm[w]->pos != 0)) { /* If whmm is not allocated or it is not the first phone */
	  h = whmm_alloc (0,n_state,WHMM_ALLOC_SIZE,1);
	  for (s = 0; s < n_state; s++)
	    h->pid[s] = pid;
	  
	  h->next = whmm[w];
	  whmm[w] = h;
	}
      }else{
	if ((! whmm[w]) || (whmm[w]->pos != 0)|| !exist_left_context(whmm, w, lcmap[lc])) {

	  h = whmm_alloc (0,n_state,WHMM_ALLOC_SIZE,0);
	  h->pid=pidp;
	  h->next = whmm[w];
	  whmm[w] = h;
	}
      }
      
      h = whmm[w];
      
      if (score > h->score[0]) {  
	h->score[0] = score;
	h->history[0] = l;
	h->active = nf;
	
	if(fwg->multiplex){
	  h->pid[0] = pid;
	}else{
	  h->lc=lcmap[lc];
	  h->pid=pidp;
	}
      }

    } else {
	/* Do all right contexts; first make sure all are allocated */
	prevh = NULL;
	h = whmm[w];
	npid = get_rc_npid (ct_table,w,dict);
	rpid = ct_table->lrcpid[b][lc].pid;
	
	for (rc = 0; rc < npid; rc++) {

	  pidp=&(ct_table->lrcpid[b][lc].pid[ct_table->lrcpid[b][lc].cimap[rc]]);
	  pid=*(pidp);

	  if(fwg->multiplex_singleph){
	    if ((! h) || (h->rc != rc)) {
	      h = whmm_alloc (0,n_state,WHMM_ALLOC_SIZE,1);
	      for (s = 0; s < n_state; s++){
		h->pid[s] = rpid[rc];
	      }
	      
	      h->rc = rc;

	      if (prevh) {
		h->next = prevh->next;
		prevh->next = h;
	      } else {
		h->next = whmm[w];
		whmm[w] = h;
	      }
	    }
	  }else {
	    if ((! h) || !exist_left_right_context(whmm, w,lcmap[lc],rc)) {
	      h = whmm_alloc (0,n_state,WHMM_ALLOC_SIZE,0);
	      h->pid=pidp;
	      
	      h->rc = rc;
	      
	      if (prevh) {
		h->next = prevh->next;
		prevh->next = h;
	      } else {
		h->next = whmm[w];
		whmm[w] = h;
	      }
	    }
	  }
	  prevh = h;
	  h = h->next;

	}

	if(fwg->multiplex_singleph)
	  assert (! h);
	
	/* Transition to the allocated HMMs */
	b = dict->word[w].ciphone[0];
	for (rc = 0, h = whmm[w]; rc < npid; rc++, h = h->next) {

	  pidp=&(ct_table->lrcpid[b][lc].pid[ct_table->lrcpid[b][lc].cimap[rc]]);
	  pid=*(pidp);

	    if (score > h->score[0]) {
		h->score[0] = score;
		h->history[0] = l;

		if(fwg->multiplex_singleph)
		  h->pid[0] = rpid[rc];
		else{
		  h->pid=pidp;
		  h->lc = lcmap[lc];
		}

		h->active = nf;
	    }
	}
    }

    /*    dump_all_whmm(fwg,fwg->whmm,fwg->n_frm,fwg->n_state,NULL);*/
}



/** 
 * Transition for one word. 
 *
 * ARCHAN: This is the heart of the flat forward search.  When a word
 * is exited, n_lat_entry will be increased by 1, this will implicitly
 * trigger fwd_frame() to start word_trans (this function).  Word
 * trans will consider all word ends.  They should be now all entries
 * in lattice_t (which is very similar to vithist_entry_t if you look
 * at them closely).
 *
 */
void word_trans (srch_FLAT_FWD_graph_t* fwg, whmm_t **whmm, int32 n_state, latticehist_t* lathist, int32 thresh, int32 phone_penalty)
{
    s3latid_t l;	/* lattice entry index */
    s3cipid_t *rcmap, rc, lc;
    s3wid_t w, bw0, bw1, nextwid;
    s3lmwid_t lw0;
    tg_t *tgptr;
    bg_t *bgptr;
    int32 bowt, acc_bowt, newscore;
    int32 n_tg, n_bg;
    int32 cand, lscr;
    int32 lat_start;
    kbcore_t* kbc;
    tmat_t* tmat;
    dict_t* dict;
    mdef_t *mdef;
    lm_t* lm;
    ctxt_table_t *ct_table;
    fillpen_t *fpen;

    kbc=fwg->kbcore;
    dict=kbcore_dict(kbc);
    tmat=kbcore_tmat(kbc);
    mdef=kbcore_mdef(kbc);
    lm=kbcore_lm(kbc);
    fpen=kbcore_fillpen(kbc);
    ct_table=fwg->ctxt;

    lat_start = lathist->frm_latstart[fwg->n_frm];
    
    for (rc = 0; rc < mdef->n_ciphone; rc++) {
	fwg->ug_backoff[rc].score = S3_LOGPROB_ZERO;
	fwg->filler_backoff[rc].score = S3_LOGPROB_ZERO;
    }
    
    if (fwg->n_word_cand > 0)
	build_word_cand_cf (fwg->n_frm,dict,fwg->word_cand_cf,fwg->word_cand_win, fwg->word_cand);
    
    /* Trigram/Bigram word transitions from words just exited in whmm_exit*/
    for (l = lat_start; l < lathist->n_lat_entry; l++) {
	w = lathist->lattice[l].wid;
	
	if (w == dict->finishwid)
	    continue;
	
	/* Cross-word left context for new words to which we may transition */
	lc = dict->word[w].ciphone[dict->word[w].pronlen-1];
	
	/* Uncompact path scores for all right context ciphones for word just finished */
	rcmap = get_rc_cimap (ct_table,w,dict );
	for (rc = 0; rc < mdef->n_ciphone; rc++)
	  fwg->rcscore[rc] = lathist->lattice[l].rcscore[rcmap[rc]];

	/* Find the last real (non-filler, non-silence) two-word history */

#if SINGLE_RC_HISTORY
	two_word_history (lathist,l, &bw0, &bw1,dict);
#else
	two_word_history (lathist,l, &bw0, &bw1,BAD_S3WID,dict,ct_table);
#endif

	if (fwg->n_word_cand <= 0) {
	    /* Transition to all words in vocab */

	    /* Clear trigram transition flag for each word for this history */
	    memset (fwg->tg_trans_done, 0, dict->n_word*sizeof(uint8));

	    /* First, transition to trigram followers of bw0, bw1 */
	    acc_bowt = 0;
	    if ((IS_S3WID(bw0)) && ((n_tg = lm_tglist (lm,
						       lm->dict2lmwid[dict_basewid(dict,bw0)], 
						       lm->dict2lmwid[dict_basewid(dict,bw1)], &tgptr, &bowt)) > 0)) {
		/* Transition to trigram followers of bw0, bw1, if any */
		for (; n_tg > 0; --n_tg, tgptr++) {
		    /* Transition to all alternative pronunciations for trigram follower */
		    nextwid = LM_DICTWID(lm, tgptr->wid);
		    
		    if (IS_S3WID(nextwid) && (nextwid != dict->startwid)) {
			for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
			    newscore = fwg->rcscore[dict->word[w].ciphone[0]] +
				LM_TGPROB (lm, tgptr) + phone_penalty;
			    
			    if (newscore >= thresh) {
				word_enter (fwg, w, n_state, newscore, l, lc);
				fwg->tg_trans_done[w] = 1;
			    }
			}
		    }
		}
		
		acc_bowt = bowt;
	    }
	    
	    /* Transition to bigram followers of bw1 */
	    if ((n_bg = lm_bglist (lm,
				   lm->dict2lmwid[dict_basewid(dict,bw1)], 
				   &bgptr, 
				   &bowt)) > 0) {
		/* Transition to bigram followers of bw1, if any */
		for (; n_bg > 0; --n_bg, bgptr++) {
		    /* Transition to all alternative pronunciations for bigram follower */
		    nextwid = LM_DICTWID (lm, bgptr->wid);
		    
		    if (IS_S3WID(nextwid) &&
			(! fwg->tg_trans_done[nextwid]) &&	/* TG transition already done */
			(nextwid != dict->startwid)) {	/* No transition to <s> */
			for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
			    newscore = fwg->rcscore[dict->word[w].ciphone[0]] +
				LM_BGPROB (lm, bgptr) + acc_bowt + phone_penalty;
			    
			    if (newscore >= thresh)
				word_enter (fwg, w, n_state, newscore, l, lc);
			}
		    }
		}
		
		acc_bowt += bowt;
	    }
	    
	    /* Update unigram backoff node */
	    for (rc = 0; rc < mdef->n_ciphone; rc++) {
		if (fwg->rcscore[rc] <= S3_LOGPROB_ZERO)
		    continue;

		if (fwg->rcscore[rc]+acc_bowt+phone_penalty > fwg->ug_backoff[rc].score) {
		    fwg->ug_backoff[rc].score = fwg->rcscore[rc]+acc_bowt+phone_penalty;
		    fwg->ug_backoff[rc].latid = l;
		    fwg->ug_backoff[rc].lc = lc;
		}
	    }
	} else {
	    /* Transition to words in word_cand_cf */
	  
	    for (cand = 0; IS_S3WID(fwg->word_cand_cf[cand]); cand++) {
		nextwid = fwg->word_cand_cf[cand];

		lw0 = IS_S3WID(bw0) ? lm->dict2lmwid[dict_basewid(dict,bw0)] : BAD_S3LMWID;
		  
		lscr = lm_tg_score (lm,
				    lw0,
				    lm->dict2lmwid[dict_basewid(dict,bw1)],
				    lm->dict2lmwid[nextwid],
				    nextwid);

		for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
		    newscore = fwg->rcscore[dict->word[w].ciphone[0]] + lscr + phone_penalty;
		    
		    if (newscore >= thresh)
			word_enter (fwg, w, n_state, newscore, l, lc);
		}
	    }
	}
	
	/* Update filler backoff node */
	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    if (fwg->rcscore[rc] <= S3_LOGPROB_ZERO)
		continue;

	    if (fwg->rcscore[rc]+phone_penalty > fwg->filler_backoff[rc].score) {
		fwg->filler_backoff[rc].score = fwg->rcscore[rc]+phone_penalty;
		fwg->filler_backoff[rc].latid = l;
		fwg->filler_backoff[rc].lc = lc;
	    }
	}
    }
    
    /*
     * We have finished transitions to all tg/bg followers of all words just ended.
     * Or, if working from a lattice, transitioned to all words that may start at this
     * point as indicated by the lattice.
     */

    /* Transition to unigrams from backoff nodes (if not working from a lattice) */
    if (fwg->n_word_cand <= 0) {
#if 0
	n_ug = lm_uglist (&ugptr);
	for (; n_ug > 0; --n_ug, ugptr++) {
	    for (w = ugptr->dictwid; IS_S3WID(w); w = dict->word[w].alt) {
		if (w == dict->startwid)
		    continue;
		
		rc = dict->word[w].ciphone[0];
		if (ug_backoff[rc].score >= thresh) {
		    newscore = ug_backoff[rc].score + LM_UGPROB (lm, ugptr);
		    if (newscore >= thresh)
			word_enter (w, newscore, ug_backoff[rc].latid, ug_backoff[rc].lc);
		}
	    }
	}
#else
	word_ugprob_t *wp;
	int32 rcscr;

	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    rcscr = fwg->ug_backoff[rc].score;
	    l = fwg->ug_backoff[rc].latid;
	    lc = fwg->ug_backoff[rc].lc;

	    for (wp = fwg->word_ugprob[rc]; wp; wp = wp->next) {
		newscore = rcscr + wp->ugprob;
		if (newscore < thresh)
		    break;
		word_enter (fwg, wp->wid, n_state, newscore, l, lc);
	    }
	}
#endif
    }

    /*
     * Transition to silence and filler words.  Assume alternative pronunciations
     * are all within filler_start..filler_end
     */
    for (w = dict->filler_start; w <= dict->filler_end; w++) {
	if ((w == dict->startwid) || (w == dict->finishwid))
	    continue;
	
	rc = dict->word[w].ciphone[0];
	if (fwg->filler_backoff[rc].score > S3_LOGPROB_ZERO) {
	    newscore = fwg->filler_backoff[rc].score + fillpen(fpen,dict_basewid(dict,w));
	    if (newscore >= thresh)
		word_enter (fwg, w, n_state, newscore, fwg->filler_backoff[rc].latid, fwg->filler_backoff[rc].lc);
	}
    }

    /* Free rcscore here, if necessary to conserve memory space */
}


void s3flat_fwd_dag_dump(char *dir, int32 onlynodes, char *id, char* latfile_ext, latticehist_t *lathist, int32 n_frm, dag_t *dag, 
			 lm_t *lm, dict_t *dict, ctxt_table_t *ctxt, fillpen_t *fpen
			 )
{
  latticehist_dag_write(lathist,
			dir,
			onlynodes,
			id,
			latfile_ext,
			n_frm,
			dag,
			lm,dict,ctxt,fpen);
}



void flat_fwd_dag_add_fudge_edges (srch_FLAT_FWD_graph_t* fwg, dag_t *dagp, int32 fudge, int32 min_ef_range, void *hist, dict_t *dict)
{
    dagnode_t *d, *pd;
    int32 l, ascr, lscr;
    latticehist_t *lathist;
    
    lathist=(latticehist_t*)hist;
    assert (dagp);

    if(fudge> 0 && !dagp->fudged){
      /* Add "illegal" links that are near misses */
      for (d = dagp->list; d; d = d->alloc_next) {
	if (d->lef - d->fef < min_ef_range-1)
	  continue;
	
	/* Links to d from nodes that first ended just when d started */
	for (l = lathist->frm_latstart[d->sf]; l < lathist->frm_latstart[d->sf+1]; l++) {
	    pd = lathist->lattice[l].dagnode;		/* Predecessor DAG node */

	    if ((pd->wid != dict->finishwid) &&
		(pd->fef == d->sf) &&
		(pd->lef - pd->fef >= min_ef_range-1)) {

	      lat_seg_ascr_lscr (lathist, l, BAD_S3WID, &ascr, &lscr, 
				 kbcore_lm(fwg->kbcore),
				 kbcore_dict(fwg->kbcore),
				 fwg->ctxt,
				 kbcore_fillpen(fwg->kbcore));

	      dag_link_w_lscr (dagp, pd, d, ascr, lscr, d->sf-1, NULL);
	    }
	}
	
	if (fudge < 2)
	    continue;
	
	/* Links to d from nodes that first ended just BEYOND when d started */
	for (l = lathist->frm_latstart[d->sf+1]; l < lathist->frm_latstart[d->sf+2]; l++) {
	    pd = lathist->lattice[l].dagnode;		/* Predecessor DAG node */

	    if ((pd->wid != dict->finishwid) &&
		(pd->fef == d->sf+1) &&
		(pd->lef - pd->fef >= min_ef_range-1)) {

	      lat_seg_ascr_lscr (lathist, l, BAD_S3WID, &ascr, &lscr, 
				 kbcore_lm(fwg->kbcore),
				 kbcore_dict(fwg->kbcore),
				 fwg->ctxt,
				 kbcore_fillpen(fwg->kbcore));

	      dag_link_w_lscr (dagp, pd, d, ascr, lscr, d->sf-1, NULL);
	    }
	}
      }
      dagp->fudged=1;
    }
}


/**
 * Build a DAG from the lattice: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * dagnodes_list = linear list of DAG nodes allocated, ordered such that nodes earlier
 * in the list can follow nodes later in the list, but not vice versa:  Let two DAG
 * nodes d1 and d2 have start times sf1 and sf2, and end time ranges [fef1..lef1] and
 * [fef2..lef2] respectively.  If d1 appears later than d2 in dag.list, then
 * fef2 >= fef1, because d2 showed up later in the word lattice.  If there is a DAG
 * edge from d1 to d2, then sf1 > fef2.  But fef2 >= fef1, so sf1 > fef1.  Reductio ad
 * absurdum.
 */
dag_t* dag_build (s3latid_t endid, latticehist_t * lathist, dict_t *dict, lm_t *lm, ctxt_table_t* ctxt, fillpen_t* fpen, int32 _nfrm)
{
    int32 l;
    s3wid_t w;
    int32 sf;
    dagnode_t *d, *pd;
    int32 ascr, lscr;
    s3latid_t latfinal;
    int32 min_ef_range;
    int32 k;
    dag_t *dag;
    
    dag=ckd_calloc(1,sizeof(dag_t));

    /* Note final lattice entry in DAG structure */

    dag->latfinal = endid;
    dag->list = NULL;

    latfinal = dag->latfinal;
    if (NOT_S3LATID(latfinal)){
      E_INFO("The final lattice ID is not valid\n");
      return NULL;
   }
    
    /* Min. endframes value that a node must persist for it to be not ignored */
    min_ef_range = cmd_ln_int32 ("-min_endfr");
    
    /* Build DAG nodes list from the lattice */
    for (l = 0; l < lathist->n_lat_entry; l++) {
	w = lathist->lattice[l].wid;

	/* ARCHAN SLIGHT BUG: Even though right context has their
	   separate histories now. It is not accounted at here. */
	sf = LATID2SF(lathist, l);
	
	/* Check if node <w,sf> already created */
	for (d = dag->list; d; d = d->alloc_next) {
	    if ((d->wid == w) && (d->sf == sf))
		break;
	}

	if (! d) {
	    d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));

	    d->wid = w;
	    d->sf = sf;
	    d->fef = lathist->lattice[l].frm;
	    d->succlist = NULL;
	    d->predlist = NULL;

	    d->alloc_next = dag->list;
	    dag->list = d;
	}
	d->lef = lathist->lattice[l].frm;

	lathist->lattice[l].dagnode = d;
    }
    
    /* Find initial node.  (BUG HERE: There may be > 1 initial node for multiple <s>) */
    for (d = dag->list; d; d = d->alloc_next) {
	if ((dict_basewid(dict,d->wid) == dict->startwid) && (d->sf == 0))
	    break;
    }
    assert (d);
    dag->root = d;

    /* Build DAG edges: between nodes satisfying time adjacency */
    for (d = dag->list; d; d = d->alloc_next) {
	/* Skip links to this node if too short lived */
	if ((d != lathist->lattice[latfinal].dagnode) && (d->lef - d->fef < min_ef_range-1))
	    continue;
	
	if (d->sf == 0) 
	  {}
/*
	    assert (d->wid == dict->startwid);	*/ /* No predecessors to this */
	else {
	    /* Link from all end points == d->sf-1 to d */
	    for (l = lathist->frm_latstart[d->sf-1]; l < lathist->frm_latstart[d->sf]; l++) {
		pd = lathist->lattice[l].dagnode;	/* Predecessor DAG node */
		
		/* Skip predecessor node under following conditions */
		if (pd->wid == dict->finishwid)	/* BUG: alternative prons for </s>?? */
		    continue;
		if ((pd != dag->root) && (pd->lef - pd->fef < min_ef_range-1))
		    continue;
		
		/*
 		 * Find acoustic score for link from pd to d (for lattice entry l
		 * with pd as right context).
		 */
		lat_seg_ascr_lscr (lathist, l, d->wid, &ascr, &lscr,lm,dict,ctxt,fpen);
		lathist->lattice[l].ascr=ascr;
		lathist->lattice[l].lscr=lscr;
		if (ascr > S3_LOGPROB_ZERO)
		  dag_link_w_lscr (dag, pd, d, ascr,lscr, d->sf-1, NULL);
	    }
	}
    }
    

    dag->filler_removed = 0;
    dag->fudged = 0;
    dag->nfrm=_nfrm;

    dag->maxedge = cmd_ln_int32("-maxedge");
    /*
     * Set limit on max LM ops allowed after which utterance is aborted.
     * Limit is lesser of absolute max and per frame max.
     */
    dag->maxlmop = cmd_ln_int32("-maxlmop");
    k = cmd_ln_int32 ("-maxlpf");

    k *= dag->nfrm;
    if (dag->maxlmop > k)
	dag->maxlmop = k;
    dag->lmop = 0;

    return dag;
}




/**
 * Remove filler nodes from DAG by replacing each link TO a filler with links
 * to its successors.
 * lwf = language weight factor to be applied to LM scores.
 */
void flat_fwd_dag_remove_filler_nodes (dag_t* dag, latticehist_t *lathist, float64 lwf, lm_t *lm, dict_t* dict, ctxt_table_t *ct_table, fillpen_t *fpen)
{
    s3latid_t latfinal;
    daglink_t *plink;
    int32 lscr;
    
    latfinal = dag->latfinal;
    
    /* If Viterbi search terminated in filler word coerce final DAG node to FINISH_WORD */
    if (dict_filler_word (dict,lathist->lattice[latfinal].wid))
	lathist->lattice[latfinal].dagnode->wid = dict->finishwid;

    if (dag_remove_filler_nodes (dag, lwf, dict, fpen) < 0) {
	E_ERROR ("maxedge limit (%d) exceeded\n", dag->maxedge);
    }else
      dag->filler_removed=1;

    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link (dag,NULL, dag->root, 0, -1, NULL);
    
    /* Attach a dummy predecessor link from nowhere into final DAG node */
    plink = &(dag->final);
    plink->node = lathist->lattice[latfinal].dagnode;
    plink->src = NULL;
    lat_seg_ascr_lscr (lathist, latfinal, BAD_S3WID, &(plink->ascr), &lscr, lm,dict,ct_table,fpen);
    plink->pscr = (int32)0x80000000;
    plink->lscr = 0;
    plink->bypass = NULL;
    plink->history = NULL;
    plink->ef = lathist->lattice[latfinal].frm;
    plink->next = NULL;
}


