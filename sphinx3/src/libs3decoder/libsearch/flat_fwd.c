/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * $Log: flat_fwd.c,v $
 * Revision 1.14  2006/02/23 05:36:23  arthchan2003
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "logs3.h"
#include "search.h"
#include "ctxt_table.h"

#include "dag.h"

#include "srch_flat_fwd_internal.h"

/** \file flat_fwd.c 
    \brief Implementation of forward search in a flat lexicon. 
 */

void
dump_all_whmm(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm, int32 n_frm, int32 * senscr)
{
    s3wid_t w;
    whmm_t *h;
    kbcore_t *kbc;
    tmat_t *tmat;
    dict_t *dict;
    mdef_t *mdef;

    kbc = fwg->kbcore;
    tmat = kbcore_tmat(kbc);
    dict = kbcore_dict(kbc);
    mdef = kbcore_mdef(kbc);

    for (w = 0; w < dict->n_word; w++) {
        if (whmm[w]) {
            for (h = whmm[w]; h; h = h->next) {
                dump_whmm(w, h, senscr, tmat, n_frm, dict, mdef);
            }
        }
    }
}

void
dump_all_word(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm)
{
    s3wid_t w;
    whmm_t *h;
    int32 last, bestlast;
    kbcore_t *kbc;
    tmat_t *tmat;
    dict_t *dict;

    kbc = fwg->kbcore;
    tmat = kbcore_tmat(kbc);
    dict = kbcore_dict(kbc);

    for (w = 0; w < dict->n_word; w++) {
        if (whmm[w]) {
            printf("[%4d] %-24s", fwg->n_frm, dict->word[w].word);

            last = dict->word[w].pronlen - 1;
            bestlast = (int32) 0x80000000;

            for (h = whmm[w]; h; h = h->next) {
                if (h->pos < last)
                    printf(" %9d.%2d", -hmm_out_score(h), h->pos);
                else if (bestlast < hmm_out_score(h))
                    bestlast = hmm_out_score(h);
            }

            if (bestlast > (int32) 0x80000000)
                printf(" %9d.%2d", -bestlast, last);

            printf("\n");
        }
    }
}


int32
whmm_eval(srch_FLAT_FWD_graph_t * fwg, int32 * senscr)
{
    int32 best, cf;
    s3wid_t w;
    whmm_t *h, *nexth, *prevh;
    int32 n_mpx, n_nonmpx;
    kbcore_t *kbc;
    tmat_t *tmat;
    dict_t *dict;
    mdef_t *mdef;
    whmm_t **whmm;

    kbc = fwg->kbcore;
    tmat = kbcore_tmat(kbc);
    dict = kbcore_dict(kbc);
    mdef = kbcore_mdef(kbc);
    whmm = fwg->whmm;

    best = S3_LOGPROB_ZERO;
    n_mpx = n_nonmpx = 0;
    cf = fwg->n_frm;

    hmm_context_set_senscore(fwg->hmmctx, senscr);
    for (w = 0; w < dict->n_word; w++) {
        prevh = NULL;
        for (h = whmm[w]; h; h = nexth) {
            nexth = h->next;
            if (hmm_frame(h) == cf) {
                int32 score;

                score = hmm_vit_eval((hmm_t *)h);
                if (hmm_is_mpx(h))
                    n_mpx++;
                else
                    n_nonmpx++;

                if (best < score)
                    best = score;

                prevh = h;

            }
            else {
                if (prevh)
                    prevh->next = nexth;
                else
                    whmm[w] = nexth;

                whmm_free(h);
            }
        }
    }

    pctr_increment(fwg->ctr_mpx_whmm, n_mpx);
    pctr_increment(fwg->ctr_nonmpx_whmm, n_nonmpx);

    return best;
}

void
whmm_renorm(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm, int32 bestscr)
{
    s3wid_t w;
    whmm_t *h;
    dict_t *dict;

    dict = fwg->kbcore->dict;
    fwg->renormalized = 1;

    for (w = 0; w < dict->n_word; w++)
        for (h = whmm[w]; h; h = h->next)
            hmm_normalize((hmm_t *)h, bestscr);
}


/**
 * Transition from hmm h into the next appropriate one for word w.
 * Threshold check for incoming score already completed.
 * The next HMM may be the last triphone for the word w, in which case, instantiate
 * multiple instances corresponding cross-word triphone modelling for all right context
 * ciphones.
 */
void
whmm_transition(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm, int32 w, whmm_t * h)
{
    int32 lastpos, nssid, nf;
    whmm_t *nexth, *prevh;
    s3cipid_t rc;
    s3pid_t *ssid;
    ctxt_table_t *ct_table;
    dict_t *dict;
    kbcore_t *kbc;
    tmat_t *tmat;

    kbc = fwg->kbcore;
    tmat = kbcore_tmat(kbc);
    dict = kbcore_dict(kbc);
    ct_table = fwg->ctxt;

    lastpos = dict->word[w].pronlen - 1;
    nf = fwg->n_frm + 1;

    if (h->pos < lastpos - 1) {
        /*
         * Transition to word HMM that's not the final one in word.  First, allocate
         * the HMM if not already present.
         */
        if ((!h->next) || (h->next->pos != h->pos + 1)) {
            nexth = whmm_alloc(fwg->hmmctx, h->pos + 1, FALSE,
                               ctxt_table_word_int_ssid(ct_table, w, h->pos + 1),
                               dict->word[w].ciphone[h->pos + 1]);
            nexth->next = h->next;
            h->next = nexth;
        }

        /* Transition to next HMM */
        nexth = h->next;
        if (hmm_out_score(h) > hmm_in_score(nexth))
            hmm_enter((hmm_t *)nexth, hmm_out_score(h), hmm_out_history(h), nf);
    }
    else {
        /*
         * Transition to final HMMs in word, with full cross-word rc modelling.  Allocate
         * all final triphone HMM instances first.
         */
        prevh = h;
        get_rcssid(ct_table, w, &ssid, &nssid, dict);
        for (rc = 0; rc < nssid; rc++) {
            if ((!prevh->next) || (prevh->next->rc != rc)) {
                nexth = whmm_alloc(fwg->hmmctx, h->pos + 1, FALSE,
                                   ssid[rc],
                                   dict->word[w].ciphone[h->pos + 1]);

                nexth->rc = rc;
                nexth->next = prevh->next;
                prevh->next = nexth;
            }
            prevh = prevh->next;
        }

        /* Transition to next HMMs */
        for (rc = 0, nexth = h->next; rc < nssid; rc++, nexth = nexth->next) {
            if (hmm_out_score(h) > hmm_in_score(nexth))
                hmm_enter((hmm_t *)nexth, hmm_out_score(h), hmm_out_history(h), nf);
        }
    }
}

void
whmm_exit(srch_FLAT_FWD_graph_t * fwg,
          whmm_t ** whmm,
          latticehist_t * lathist,
          int32 thresh, int32 wordthresh, int32 phone_penalty)
{
    s3wid_t w;
    whmm_t *h;
    int32 pronlen, nf;
    dict_t *dict;
    kbcore_t *kbc;
    tmat_t *tmat;

    kbc = fwg->kbcore;
    tmat = kbcore_tmat(kbc);
    dict = kbcore_dict(kbc);

    nf = fwg->n_frm + 1;

    for (w = 0; w < dict->n_word; w++) {
        pronlen = dict->word[w].pronlen;

        for (h = whmm[w]; h; h = h->next) {
            if (hmm_bestscore(h) >= thresh) {
                if (h->pos == pronlen - 1) {
                    if (hmm_out_score(h) >= wordthresh) {
                        lattice_entry(lathist, w, fwg->n_frm,
                                      hmm_out_score(h),
                                      hmm_out_history(h),
                                      h->rc, fwg->ctxt, dict);
                    }
                }
                else {
                    if (hmm_out_score(h) + phone_penalty >= thresh)
                        whmm_transition(fwg, whmm, w, h);
                }

                hmm_frame(h) = nf;
            }
        }
    }
}


/* ARCHAN: Ah. this is the part where the magical multiplex-triphone
   is implemented.  This how it works.  When a word is being entered,
   the first phone will be entered, if the score is high enough, then
   ssid[0] will be replaced by the best SSID.  Now, when we go back
   whmm_eval, the first phone will always be computed as multiplexed.
   Then, the ssid will then start to propagate just like the score.
   
   You could just treat ssid as something like backtracing
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

void
word_enter(srch_FLAT_FWD_graph_t * fwg, s3wid_t w,
           int32 score, s3latid_t l, s3cipid_t lc)
{
    whmm_t *h, *prevh;
    s3cipid_t b, rc;
    s3ssid_t ssid, *rssid;
    s3ssid_t *ssidp;
    int32 nssid, nf;
    kbcore_t *kbc;
    tmat_t *tmat;
    dict_t *dict;
    ctxt_table_t *ct_table;
    s3senid_t *lcmap;

    whmm_t **whmm;

    kbc = fwg->kbcore;
    dict = kbcore_dict(kbc);
    tmat = kbcore_tmat(kbc);
    ct_table = fwg->ctxt;
    whmm = fwg->whmm;

    assert(whmm);
    assert(kbc);
    assert(dict);
    assert(tmat);

    nf = fwg->n_frm + 1;

    b = dict->word[w].ciphone[0];
    lcmap = get_lc_cimap(ct_table, w, dict);

    if (dict->word[w].pronlen > 1) {    /* Multi-phone word; no right context problem */

        rc = dict->word[w].ciphone[1];

        /* Get a pointer to the senone sequence ID for the triphone
         * b(lc,rc) where b,rc are the first two phones of the next
         * word and lc is the last phone of the last word. */
        /* Note that b, lc, and rc are all fully known, so the
         * ctxt_table is really not doing much of anything the model
         * definition couldn't already do. */
        ssidp = &ctxt_table_left_ctxt_ssid(ct_table, lc, b, rc);
        /* &(ct_table->lcpid[b][rc].pid[ct_table->lcpid[b][rc].cimap[lc]]); */
        ssid = *(ssidp);

        /* Allocate and initialize a multiplex HMM for the next phone if necessary. */
        if ((!whmm[w]) || (whmm[w]->pos != 0)) {
            /* If whmm is not allocated or it is not the first phone */
            h = whmm_alloc(fwg->hmmctx, 0, TRUE, ssid, b);
            h->next = whmm[w];
            whmm[w] = h;
        }
        h = whmm[w];

        /* And now enter the next HMM in the usual Viterbi fashion. */
        if (score > hmm_in_score(h)) {
            hmm_enter((hmm_t *)h, score, l, nf);
            if (hmm_is_mpx(h)) {
                hmm_mpx_ssid(h, 0) = ssid;
            }
            else {
                h->lc = lcmap[lc];
                hmm_nonmpx_ssid(h) = ssid;
            }
        }

    }
    else {
        /* Do all right contexts; first make sure all are allocated */
        prevh = NULL;
        h = whmm[w];
        nssid = ct_get_rc_nssid(ct_table, w, dict);
        rssid = ct_table->lrcssid[b][lc].ssid;

        for (rc = 0; rc < nssid; rc++) {

            ssidp = &ctxt_table_single_phone_ssid(ct_table, lc, b, rc);
            /* &(ct_table->lrcpid[b][lc].pid[ct_table->lrcpid[b][lc].cimap[rc]]); */
            ssid = *(ssidp);

            if ((!h) || (h->rc != rc)) {
                h = whmm_alloc(fwg->hmmctx, 0, TRUE, rssid[rc], b);
                h->rc = rc;

                if (prevh) {
                    h->next = prevh->next;
                    prevh->next = h;
                }
                else {
                    h->next = whmm[w];
                    whmm[w] = h;
                }
            }
            prevh = h;
            h = h->next;

        }

        /* Transition to the allocated HMMs */
        b = dict->word[w].ciphone[0];
        for (rc = 0, h = whmm[w]; rc < nssid; rc++, h = h->next) {

            ssidp = &ctxt_table_single_phone_ssid(ct_table, lc, b, rc);
            /* &(ct_table->lrcpid[b][lc].pid[ct_table->lrcpid[b][lc].cimap[rc]]); */
            ssid = *(ssidp);

            if (score > hmm_in_score(h)) {
                hmm_enter((hmm_t *)h, score, l, nf);
                if (hmm_is_mpx(h)) {
                    hmm_mpx_ssid(h, 0) = rssid[rc];
                }
                else {
                    hmm_nonmpx_ssid(h) = ssid;
                    h->lc = lcmap[lc];
                }
            }
        }
    }

    /*    dump_all_whmm(fwg,fwg->whmm,fwg->n_frm,fwg->n_state,NULL); */
}


/**
 * Enter successor words from language model.
 */
static void
enter_lm_words(srch_FLAT_FWD_graph_t * fwg, latticehist_t *lathist,
               s3latid_t l, s3cipid_t lc, int32 thresh, int32 phone_penalty)
{
    kbcore_t *kbc = fwg->kbcore;
    lm_t *lm = kbcore_lm(kbc);
    dict_t *dict = kbcore_dict(kbc);
    mdef_t *mdef = kbcore_mdef(kbc);
    s3wid_t bw0, bw1;
    int is32bits = lm->is32bits;
    int32 acc_bowt, bowt;
    int32 n_bg;
    bg_t *bgptr;
    bg32_t *bgptr32;
    s3cipid_t rc;

    /* Find the last real (non-filler, non-silence) two-word history */
    two_word_history(lathist, l, &bw0, &bw1, dict);

    /* Clear trigram transition flag for each word for this history */
    memset(fwg->tg_trans_done, 0, dict->n_word * sizeof(*fwg->tg_trans_done));

    /* First, transition to trigram followers of bw0, bw1 */
    acc_bowt = 0;
    if (IS_S3WID(bw0)) {
        int32 n_tg;
        tg_t *tgptr;
        tg32_t *tgptr32;

        /* FIXME: THIS SUCKS */
        if (is32bits) {
            n_tg = lm_tg32list(lm,
                               lm->
                               dict2lmwid[dict_basewid(dict, bw0)],
                               lm->
                               dict2lmwid[dict_basewid(dict, bw1)],
                               &tgptr32, &bowt);

        }
        else {
            n_tg = lm_tglist(lm,
                             lm->
                             dict2lmwid[dict_basewid(dict, bw0)],
                             lm->
                             dict2lmwid[dict_basewid(dict, bw1)],
                             &tgptr, &bowt);

        }

        if (n_tg > 0) {
            /* Transition to trigram followers of bw0, bw1, if any */
            while (n_tg > 0) {
                s3wid_t nextwid;

                nextwid = is32bits ?
                    LM_DICTWID(lm, tgptr32->wid) :
                    LM_DICTWID(lm, tgptr->wid);


                /* Transition to all alternative pronunciations for trigram follower */


                if (IS_S3WID(nextwid)
                    && (nextwid != dict->startwid)) {
                    s3wid_t w;

                    for (w = nextwid; IS_S3WID(w);
                         w = dict->word[w].alt) {
                        int32 newscore;

                        newscore = fwg->rcscore[dict->word[w].ciphone[0]];      /* right context scores with phone ciphone[0] */
                        newscore += is32bits ? LM_TGPROB(lm, tgptr32) : LM_TGPROB(lm, tgptr);   /* The LM scores */
                        newscore += phone_penalty;

                        if (newscore >= thresh) {
                            word_enter(fwg, w, newscore, l, lc);
                            fwg->tg_trans_done[w] = 1;
                        }
                    }
                }
                --n_tg;
                if (is32bits)
                    ++tgptr32;
                else
                    ++tgptr;
            }
            acc_bowt = bowt;
        }
    }

    /* Transition to bigram followers of bw1 */
    if (is32bits)
        n_bg = lm_bg32list(lm,
                           lm->dict2lmwid[dict_basewid(dict, bw1)],
                           &bgptr32, &bowt);
    else
        n_bg = lm_bglist(lm,
                         lm->dict2lmwid[dict_basewid(dict, bw1)],
                         &bgptr, &bowt);

    if (n_bg > 0) {
        /* Transition to bigram followers of bw1, if any */
        for (; n_bg > 0; --n_bg) {
            s3wid_t nextwid;

            /* Transition to all alternative pronunciations for bigram follower */
            nextwid =
                is32bits ? LM_DICTWID(lm,
                                      bgptr32->
                                      wid) : LM_DICTWID(lm,
                                                        bgptr->
                                                        wid);

            if (IS_S3WID(nextwid) &&
                (!fwg->tg_trans_done[nextwid]) &&  /* TG transition already done */
                (nextwid != dict->startwid)) {  /* No transition to <s> */
                s3wid_t w;

                for (w = nextwid; IS_S3WID(w);
                     w = dict->word[w].alt) {
                    int32 newscore;

                    newscore =
                        fwg->rcscore[dict->word[w].ciphone[0]];
                    newscore +=
                        is32bits ? LM_BGPROB(lm,
                                             bgptr32) :
                        LM_BGPROB(lm, bgptr);
                    newscore += acc_bowt;
                    newscore += phone_penalty;

                    if (newscore >= thresh)
                        word_enter(fwg, w, newscore, l, lc);
                }
            }
            if (is32bits)
                ++bgptr32;
            else
                ++bgptr;
        }

        acc_bowt += bowt;
    }

    /* Update unigram backoff node */
    for (rc = 0; rc < mdef->n_ciphone; rc++) {
        if (fwg->rcscore[rc] <= S3_LOGPROB_ZERO)
            continue;

        if (fwg->rcscore[rc] + acc_bowt + phone_penalty >
            fwg->ug_backoff[rc].score) {
            fwg->ug_backoff[rc].score =
                fwg->rcscore[rc] + acc_bowt + phone_penalty;
            fwg->ug_backoff[rc].latid = l;
            fwg->ug_backoff[rc].lc = lc;
        }
    }
}

/**
 * Enter words from list of candidates (i.e. lattice rescoring as in Sphinx2)
 **/
static void
enter_cand_words(srch_FLAT_FWD_graph_t * fwg, latticehist_t *lathist,
                 s3latid_t l, s3cipid_t lc, int32 thresh, int32 phone_penalty)
{
    int32 cand;
    kbcore_t *kbc = fwg->kbcore;
    lm_t *lm = kbcore_lm(kbc);
    dict_t *dict = kbcore_dict(kbc);
    s3wid_t bw0, bw1;

    /* Find the last real (non-filler, non-silence) two-word history */
    two_word_history(lathist, l, &bw0, &bw1, dict);

    /* Transition to words in word_cand_cf */
    for (cand = 0; IS_S3WID(fwg->word_cand_cf[cand]); cand++) {
        s3wid_t w, nextwid;
        s3lmwid32_t lw0;
        int32 lscr;

        nextwid = fwg->word_cand_cf[cand];
        lw0 =
            IS_S3WID(bw0) ? lm->
            dict2lmwid[dict_basewid(dict, bw0)] : BAD_LMWID(lm);
        lscr = lm_tg_score(lm,
                           lw0,
                           lm->dict2lmwid[dict_basewid(dict, bw1)],
                           lm->dict2lmwid[nextwid], nextwid);

        for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
            int32 newscore;

            newscore =
                fwg->rcscore[dict->word[w].ciphone[0]] + lscr +
                phone_penalty;

            if (newscore >= thresh)
                word_enter(fwg, w, newscore, l, lc);
        }
    }
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
void
word_trans(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm,
           latticehist_t * lathist, int32 thresh, int32 phone_penalty)
{
    kbcore_t *kbc = fwg->kbcore;
    dict_t *dict = kbcore_dict(kbc);
    mdef_t *mdef = kbcore_mdef(kbc);
    ctxt_table_t *ct_table = fwg->ctxt;
    fillpen_t *fpen = kbcore_fillpen(kbc);

    s3latid_t l;                /* lattice entry index */
    s3wid_t w;
    s3cipid_t *rcmap, rc, lc;
    int32 lat_start;

    lat_start = lathist->frm_latstart[fwg->n_frm];

    for (rc = 0; rc < mdef->n_ciphone; rc++) {
        fwg->ug_backoff[rc].score = S3_LOGPROB_ZERO;
        fwg->filler_backoff[rc].score = S3_LOGPROB_ZERO;
    }

    if (fwg->n_word_cand > 0)
        build_word_cand_cf(fwg->n_frm, dict, fwg->word_cand_cf,
                           fwg->word_cand_win, fwg->word_cand);

    /* Trigram/Bigram word transitions from words just exited in whmm_exit */
    for (l = lat_start; l < lathist->n_lat_entry; l++) {
        w = lathist->lattice[l].wid;

        if (w == dict->finishwid)
            continue;

        /* Cross-word left context for new words to which we may transition */
        lc = dict->word[w].ciphone[dict->word[w].pronlen - 1];

        /* Uncompact path scores for all right context ciphones for word just finished */
        rcmap = get_rc_cimap(ct_table, w, dict);
        for (rc = 0; rc < mdef->n_ciphone; rc++)
            fwg->rcscore[rc] = lathist->lattice[l].rcscore[rcmap[rc]];

        if (fwg->n_word_cand <= 0) {
            enter_lm_words(fwg, lathist, l, lc, thresh, phone_penalty);
        }
        else {
            enter_cand_words(fwg, lathist, l, lc, thresh, phone_penalty);
        }

        /* Update filler backoff node */
        for (rc = 0; rc < mdef->n_ciphone; rc++) {
            if (fwg->rcscore[rc] <= S3_LOGPROB_ZERO)
                continue;

            if (fwg->rcscore[rc] + phone_penalty >
                fwg->filler_backoff[rc].score) {
                fwg->filler_backoff[rc].score =
                    fwg->rcscore[rc] + phone_penalty;
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
#if 0 /* FIXME: WHAT IS THE DIFFERENCE BETWEEN THESE TWO VERSIONS?  #@!$!@# */
        n_ug = lm_uglist(&ugptr);
        for (; n_ug > 0; --n_ug, ugptr++) {
            for (w = ugptr->dictwid; IS_S3WID(w); w = dict->word[w].alt) {
                if (w == dict->startwid)
                    continue;

                rc = dict->word[w].ciphone[0];
                if (ug_backoff[rc].score >= thresh) {
                    newscore = ug_backoff[rc].score + LM_UGPROB(lm, ugptr);
                    if (newscore >= thresh)
                        word_enter(w, newscore, ug_backoff[rc].latid,
                                   ug_backoff[rc].lc);
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
                int32 newscore;

                newscore = rcscr + wp->ugprob;
                if (newscore < thresh)
                    break;
                word_enter(fwg, wp->wid, newscore, l, lc);
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
            int32 newscore;

            newscore =
                fwg->filler_backoff[rc].score + fillpen(fpen,
                                                        dict_basewid(dict,
                                                                     w));
            if (newscore >= thresh)
                word_enter(fwg, w, newscore,
                           fwg->filler_backoff[rc].latid,
                           fwg->filler_backoff[rc].lc);
        }
    }

    /* Free rcscore here, if necessary to conserve memory space */
}


void
flat_fwd_dag_add_fudge_edges(srch_FLAT_FWD_graph_t * fwg, dag_t * dagp,
                             int32 fudge, int32 min_ef_range, void *hist,
                             dict_t * dict)
{
    dagnode_t *d, *pd;
    int32 l, ascr, lscr;
    latticehist_t *lathist;

    lathist = (latticehist_t *) hist;
    assert(dagp);

    if (fudge > 0 && !dagp->fudged) {
        /* Add "illegal" links that are near misses */
        for (d = dagp->list; d; d = d->alloc_next) {
            if (d->lef - d->fef < min_ef_range - 1)
                continue;

            /* Links to d from nodes that first ended just when d started */
            for (l = lathist->frm_latstart[d->sf];
                 l < lathist->frm_latstart[d->sf + 1]; l++) {
                pd = lathist->lattice[l].dagnode;       /* Predecessor DAG node */
                if (pd == NULL)
                    continue;

                if ((pd->wid != dict->finishwid) &&
                    (pd->fef == d->sf) &&
                    (pd->lef - pd->fef >= min_ef_range - 1)) {

                    lat_seg_ascr_lscr(lathist, l, BAD_S3WID, &ascr, &lscr,
                                      kbcore_lm(fwg->kbcore),
                                      kbcore_dict(fwg->kbcore),
                                      fwg->ctxt,
                                      kbcore_fillpen(fwg->kbcore));

                    dag_link(dagp, pd, d, ascr, lscr, d->sf - 1, NULL);
                }
            }

            if (fudge < 2)
                continue;

            /* Links to d from nodes that first ended just BEYOND when d started */
            for (l = lathist->frm_latstart[d->sf + 1];
                 l < lathist->frm_latstart[d->sf + 2]; l++) {
                pd = lathist->lattice[l].dagnode;       /* Predecessor DAG node */
                if (pd == NULL)
                    continue;

                if ((pd->wid != dict->finishwid) &&
                    (pd->fef == d->sf + 1) &&
                    (pd->lef - pd->fef >= min_ef_range - 1)) {

                    lat_seg_ascr_lscr(lathist, l, BAD_S3WID, &ascr, &lscr,
                                      kbcore_lm(fwg->kbcore),
                                      kbcore_dict(fwg->kbcore),
                                      fwg->ctxt,
                                      kbcore_fillpen(fwg->kbcore));

                    dag_link(dagp, pd, d, ascr, lscr, d->sf - 1, NULL);
                }
            }
        }
        dagp->fudged = 1;
    }
}

/**
 * Build array of candidate words that start around the current frame (cf).
 * Note: filler words are not in this list since they are always searched (see
 * word_trans).
 */
void
build_word_cand_cf(int32 cf,
                   dict_t * dict,
                   s3wid_t * wcand_cf,
                   int32 word_cand_win, word_cand_t ** wcand)
{
    int32 f, sf, ef;
    s3wid_t w, n;
    word_cand_t *candp;

    for (w = 0; w < dict->n_word; w++)
        wcand_cf[w] = 0;

    if ((sf = cf - word_cand_win) < 0)
        sf = 0;
    if ((ef = cf + word_cand_win) >= S3_MAX_FRAMES)
        ef = S3_MAX_FRAMES - 1;

    for (f = sf; f <= ef; f++) {
        for (candp = wcand[f]; candp; candp = candp->next)
            wcand_cf[candp->wid] = 1;
    }

    wcand_cf[dict->startwid] = 0;       /* Never hypothesized (except at beginning) */
    for (w = dict->filler_start; w <= dict->filler_end; w++)
        wcand_cf[w] = 0;        /* Handled separately */
    wcand_cf[dict->finishwid] = 1;      /* Always a candidate */

    n = 0;
    for (w = 0; w < dict->n_word; w++)
        if (wcand_cf[w])
            wcand_cf[n++] = w;
    wcand_cf[n] = BAD_S3WID;
}


int32
word_cand_load(FILE * fp, word_cand_t ** wcand, dict_t * dict, char *uttid)
{
    char line[1024], word[1024];
    int32 i, k, n, nn, sf, seqno, lineno;
    s3wid_t w;
    word_cand_t *candp;

    /* Skip past Nodes parameter */
    lineno = 0;
    nn = 0;
    word[0] = '\0';
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineno++;
        if ((sscanf(line, "%s %d", word, &nn) == 2)
            && (strcmp(word, "Nodes") == 0))
            break;
    }
    if ((strcmp(word, "Nodes") != 0) || (nn <= 0)) {
        E_ERROR("%s: Nodes parameter missing from input lattice\n", uttid);
        return -1;
    }

    n = 0;
    for (i = 0; i < nn; i++) {
        if (fgets(line, 1024, fp) == NULL) {
            E_ERROR("%s: Incomplete input lattice\n", uttid);
            return -1;
        }
        lineno++;

        if ((k = sscanf(line, "%d %s %d", &seqno, word, &sf)) != 3) {
            E_ERROR("%s: Error in lattice, line %d: %s\n", uttid, lineno,
                    line);
            return -1;
        }
        if (seqno != i) {
            E_ERROR("%s: Seq# error in lattice, line %d: %s\n", uttid,
                    lineno, line);
            return -1;
        }
        if ((sf < 0) || (sf >= S3_MAX_FRAMES)) {
            E_ERROR("%s: Startframe error in lattice, line %d: %s\n",
                    uttid, lineno, line);
            return -1;
        }

        w = dict_wordid(dict, word);
        if (NOT_S3WID(w)) {
            E_ERROR("%s: Unknown word in lattice: %s; ignored\n", uttid,
                    word);
            continue;
        }
        w = dict_basewid(dict, w);

        /* Check node not already present; avoid duplicates */
        for (candp = wcand[sf]; candp && (candp->wid != w);
             candp = candp->next);
        if (candp)
            continue;

        candp = (word_cand_t *) ckd_calloc(1, sizeof(word_cand_t));
        candp->wid = w;
        candp->next = wcand[sf];
        wcand[sf] = candp;

        n++;
    }

    return n;
}

void
word_cand_free(word_cand_t ** wcand)
{
    word_cand_t *candp, *next;
    int32 f;

    for (f = 0; f < S3_MAX_FRAMES; f++) {
        for (candp = wcand[f]; candp; candp = next) {
            next = candp->next;
            ckd_free((char *) candp);
        }

        wcand[f] = NULL;
    }

}
