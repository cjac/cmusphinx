/* -*- c-basic-offset: 4 -*- */
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


#include <string.h>

#include <sphinxbase/pio.h>

#include "srch.h"
#include "corpus.h"

#define COMPUTE_HEURISTIC 1
#define SHOW_SENONE_SCORE_FOR_FRAME 0

int32
srch_mode_str_to_index(const char *mode_str)
{
    if (!strcmp(mode_str, "allphone")) {
        return OPERATION_ALLPHONE;
    }
    if (!strcmp(mode_str, "fsg")) {
        return OPERATION_GRAPH;
    }
    if (!strcmp(mode_str, "fwdflat")) {
        return OPERATION_FLATFWD;
    }
    if (!strcmp(mode_str, "fwdtree")) {
        return OPERATION_TST_DECODE;
    }

    E_WARN("UNKNOWN MODE NAME %s\n", mode_str);
    return -1;

}

char *
srch_mode_index_to_str(int32 index)
{
    char *str;
    str = NULL;
    if (index == OPERATION_ALLPHONE) {
        str = ckd_salloc("allphone");
    }
    else if (index == OPERATION_GRAPH) {
        str = ckd_salloc("fsg");
    }
    else if (index == OPERATION_FLATFWD) {
        str = ckd_salloc("fwdflat");
    }
    else if (index == OPERATION_TST_DECODE) {
        str = ckd_salloc("fwdtree");
    }
    else if (index == OPERATION_DEBUG) {
        str = ckd_salloc("debug");
    }
    else if (index == OPERATION_DO_NOTHING) {
        str = ckd_salloc("do_nothing");
    }
    return str;
}

void
srch_assert_funcptrs(srch_t * s)
{
    assert(s->funcs->init != NULL);
    assert(s->funcs->uninit != NULL);
    assert(s->funcs->utt_begin != NULL);
    assert(s->funcs->utt_end != NULL);

    if (s->funcs->decode == NULL) {       /* Provide that the implementation does
                                           not override the search abstraction
                                           assert every implementation pointers
                                         */

        assert(s->funcs->init != NULL);
        assert(s->funcs->uninit != NULL);
        assert(s->funcs->utt_begin != NULL);
        assert(s->funcs->set_lm != NULL);
        assert(s->funcs->add_lm != NULL);
        assert(s->funcs->delete_lm != NULL);

        assert(s->funcs->gmm_compute_lv1 != NULL);

        if (s->funcs->one_srch_frame_lv1 != NULL) {       /* If implementation
                                                           provides only 
                                                           how to search one frame after 
                                                           GMM computation */
            assert(s->funcs->hmm_compute_lv1 == NULL);
            assert(s->funcs->eval_beams_lv1 == NULL);
            assert(s->funcs->propagate_graph_ph_lv1 == NULL);
            assert(s->funcs->propagate_graph_wd_lv1 == NULL);
        }
        else {
            if (s->funcs->hmm_compute_lv1 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_hmm_compute_lv1 is not specified\n");
            if (s->funcs->eval_beams_lv1 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_eval_beams_lv1 is not specified\n");
            if (s->funcs->propagate_graph_ph_lv1 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_propagate_graph_ph_lv1 is not specified\n");
            if (s->funcs->propagate_graph_wd_lv1 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_propagate_graph_wd_lv1 is not specified\n");
        }

        assert(s->funcs->gmm_compute_lv2 != NULL);
        if (s->funcs->one_srch_frame_lv2 != NULL) {       /* If implementation
                                                           provides only 
                                                           how to search one frame after 
                                                           GMM computation */

            assert(s->funcs->hmm_compute_lv2 == NULL);
            assert(s->funcs->eval_beams_lv2 == NULL);
            assert(s->funcs->propagate_graph_ph_lv2 == NULL);
            assert(s->funcs->propagate_graph_wd_lv2 == NULL);
        }
        else {
            if (s->funcs->hmm_compute_lv2 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_hmm_compute_lv2 is not specified\n");
            if (s->funcs->eval_beams_lv2 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_eval_beams_lv2 is not specified\n");
            if (s->funcs->propagate_graph_ph_lv2 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_propagate_graph_ph_lv2 is not specified\n");
            if (s->funcs->propagate_graph_wd_lv2 == NULL)
                E_WARN
                    ("Search one frame implementation is not specified but srch_propagate_graph_wd_lv2 is not specified\n");

        }
        assert(s->funcs->frame_windup != NULL);
#if 0                           /* compute_heuristic is marginal so don't take care of it now */
        assert(s->funcs->compute_heuristic != NULL);
#endif
        assert(s->funcs->shift_one_cache_frame != NULL);
        assert(s->funcs->select_active_gmm != NULL);

        assert(s->funcs->utt_end != NULL);
#if 0
        assert(s->funcs->gen_hyp != NULL);
        assert(s->funcs->dump_vithist != NULL);
        /* Not asserts for everything now, mainly because the FST mode
           is not generating dag at this point */
        assert(s->funcs->gen_dag != NULL);

#endif
    }

}

/** Initialize the search routine, this will specify the type of search
    drivers and initialized all resouces*/

srch_t *
srch_init(kb_t * kb, int32 op_mode)
{

    srch_t *s;
    s = (srch_t *) ckd_calloc(1, sizeof(srch_t));

    E_INFO("Search Initialization. \n");
    s->op_mode = op_mode;
    /* A switch here to decide all function pointers */
    if (op_mode == OPERATION_ALIGN) {
        E_ERROR("Alignment mode is not supported yet");
	ckd_free(s);
	return NULL;
    }
    else if (op_mode == OPERATION_ALLPHONE) {
	s->funcs = &srch_allphone_funcs;
    }
    else if (op_mode == OPERATION_GRAPH) {
	s->funcs = &srch_FSG_funcs;
    }
    else if (op_mode == OPERATION_FLATFWD) {
	s->funcs = &srch_FLAT_FWD_funcs;
    }
    else if (op_mode == OPERATION_TST_DECODE) {
	s->funcs = &srch_TST_funcs;
    }
    else if (op_mode == OPERATION_WST_DECODE) {
        E_ERROR("Word Conditioned Tree Search is currently unmaintained.");
	ckd_free(s);
	return NULL;
    }
    else if (op_mode == OPERATION_DEBUG) {
	s->funcs = &srch_debug_funcs;
    }
    else if (op_mode == OPERATION_DO_NOTHING) {
	s->funcs = &srch_do_nothing_funcs;
    }
    else {
        E_ERROR("Unknown mode %d, failed to initialized srch_t\n",
                op_mode);
	return NULL;
    }

    /* Do general search initialization here. */
    s->stat = kb->stat;
    s->ascr = kb->ascr;
    s->exit_id = -1;
    s->beam = kb->beam;
    s->fastgmm = kb->fastgmm;
    s->pl = kb->pl;
    s->adapt_am = kb->adapt_am;
    s->kbc = kb->kbcore;

    s->matchfp = kb->matchfp;
    s->matchsegfp = kb->matchsegfp;
    s->hmmdumpfp = kb->hmmdumpfp;

    s->grh = (grp_str_t *) ckd_calloc(1, sizeof(grp_str_t));
    s->cache_win = cmd_ln_int32_r(kbcore_config(s->kbc), "-pl_window");
    s->cache_win_strt = 0;
    s->senscale = 0;

    s->ascale = (int32 *) ckd_calloc(DFLT_UTT_SIZE, sizeof(int32));
    s->ascale_sz = DFLT_UTT_SIZE;
    s->segsz = (int32 *) ckd_calloc(DFLT_NUM_SEGS, sizeof(int32));
    s->segsz_sz = DFLT_NUM_SEGS;

    srch_assert_funcptrs(s);

    /* Do search-specific initialization here. */
    if (s->funcs->init(kb, s) == SRCH_FAILURE) {
        E_INFO("search initialization failed for op-mode %d\n", op_mode);
	ckd_free(s->grh);
	ckd_free(s->ascale);
	ckd_free(s->segsz);
	ckd_free(s);
	return NULL;
    }

    return s;
}

int32
srch_utt_begin(srch_t * srch)
{

    int32 i;
    if (srch->funcs->utt_begin == NULL) {
        E_ERROR
            ("srch->funcs->utt_begin is NULL. Please make sure it is set.\n");
        return SRCH_FAILURE;
    }

    srch->num_frm = 0;
    srch->num_segs = 0;
    for (i = 0; i < srch->ascale_sz; i++)
        srch->ascale[i] = 0;

    for (i = 0; i < srch->segsz_sz; i++)
        srch->segsz[i] = 0;
    srch->exit_id = -1;
    if (srch->dag)
	dag_destroy(srch->dag);
    srch->dag = NULL;
    stat_clear_utt(srch->stat);

    srch->funcs->utt_begin(srch);

    return SRCH_SUCCESS;
}

int32
srch_utt_end(srch_t * s)
{
    int32 rv;
    glist_t hyp;
    gnode_t *gn;
    stat_t *st = s->stat;

    if (s->funcs->utt_end == NULL) {
        E_ERROR
            ("s->funcs->utt_end is NULL. Please make sure it is set.\n");
        return SRCH_FAILURE;
    }

    if ((rv = s->funcs->utt_end(s)) != SRCH_SUCCESS) {
	E_ERROR("s->funcs->utt_end failed\n");
	return rv;
    }

    if (s->funcs->dump_vithist && cmd_ln_str_r(kbcore_config(s->kbc), "-bptbldir")) {
	if ((rv = s->funcs->dump_vithist(s)) != SRCH_SUCCESS) {
	    E_ERROR("s->funcs->dump_vithist failed\n");
	    return rv;
	}
    }

    /* Get the final 1-best hypothesis */
    if (s->funcs->gen_hyp == NULL) {
	/* Or don't... some modes don't implement this yet. */
        E_WARN("srch->funcs->gen_hyp is NULL.  Please make sure it is set.\n");
	return SRCH_SUCCESS;
    }
    if ((hyp = s->funcs->gen_hyp(s)) == NULL) {
	E_ERROR("s->funcs->gen_hyp failed\n");
	return SRCH_FAILURE;
    }

    /* Generate a DAG if needed */
    if (s->funcs->gen_dag &&
	(cmd_ln_str_r(kbcore_config(s->kbc), "-outlatdir")
	 || cmd_ln_str_r(kbcore_config(s->kbc), "-nbestdir")
	 || cmd_ln_boolean_r(kbcore_config(s->kbc), "-bestpath"))) {
        ptmr_start(&(st->tm_srch));
	if ((s->dag = s->funcs->gen_dag(s, hyp)) == NULL) {
	    E_ERROR("Failed to generate DAG.\n");
	}
        ptmr_stop(&(st->tm_srch));
    }

    /* Write backtrace info */
    if (cmd_ln_boolean_r(kbcore_config(s->kbc), "-backtrace")) {
        E_INFOCONT("\nBacktrace(%s)\n", s->uttid);
        match_detailed(err_get_logfp(), hyp, s->uttid, "FV", "fv", s->ascale,
                       kbcore_dict(s->kbc));
    }

    /* Write hypothesis strings */
    if (!(s->dag && cmd_ln_boolean_r(kbcore_config(s->kbc), "-bestpath"))) {
	if (s->matchfp)
	    match_write(s->matchfp, hyp, s->uttid, kbcore_dict(s->kbc), NULL);
	if (s->matchsegfp)
	    matchseg_write(s->matchsegfp, hyp, s->uttid, NULL,
			   kbcore_lm(s->kbc),
			   kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
			   cmd_ln_int32_r(kbcore_config(s->kbc), "-hypsegscore_unscale"));
    }
    match_write(err_get_logfp(), hyp, s->uttid, kbcore_dict(s->kbc), "\nFWDVIT: ");
    matchseg_write(err_get_logfp(), hyp, s->uttid, "FWDXCT: ",
                   kbcore_lm(s->kbc),
                   kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
                   cmd_ln_int32_r(kbcore_config(s->kbc), "-hypsegscore_unscale"));
    E_INFOCONT("\n");

    /* Write best senone scores */
    if (cmd_ln_str_r(kbcore_config(s->kbc), "-bestsenscrdir")) {
        int32 ispipe;
        char str[2048];
        FILE *bsfp;
        sprintf(str, "%s/%s.bsenscr",
                cmd_ln_str_r(kbcore_config(s->kbc), "-bestsenscrdir"), s->uttid);
        E_INFO("Dumping the Best senone scores.\n");
        if ((bsfp = fopen_comp(str, "w", &ispipe)) == NULL)
            E_ERROR("fopen_comp (%s,w) failed\n", str); 
	else {
	    write_bstsenscr(bsfp, s->stat->nfr, s->ascale);
            fclose_comp(bsfp, ispipe);
        }
    }

    /* Write lattices */
    if (s->dag && cmd_ln_str_r(kbcore_config(s->kbc), "-outlatdir")) {
	/* HTK lattices are generic */
	if (0 == strcmp(cmd_ln_str_r(kbcore_config(s->kbc), "-outlatfmt"), "htk")) {
	    char str[2048];
	    ctl_outfile(str, cmd_ln_str_r(kbcore_config(s->kbc), "-outlatdir"),
			cmd_ln_str_r(kbcore_config(s->kbc), "-latext"),
			(s->uttfile ? s->uttfile : s->uttid), s->uttid,
			cmd_ln_boolean_r(kbcore_config(s->kbc), "-build_outdirs"));
	    E_INFO("Writing lattice file in HTK format: %s\n", str);
	    dag_write_htk(s->dag, str, s->uttid, kbcore_lm(s->kbc), kbcore_dict(s->kbc));
	}
	/* S3 lattices might have a custom implementation */
	else {
	    if (s->funcs->dag_dump) {
		if ((s->funcs->dag_dump(s, s->dag)) != SRCH_SUCCESS) {
		    E_ERROR("Failed to write DAG file.\n");
		}
	    }
	    /* But probably not. */
	    else {
		char str[2048];
		ctl_outfile(str, cmd_ln_str_r(kbcore_config(s->kbc), "-outlatdir"),
			    cmd_ln_str_r(kbcore_config(s->kbc), "-latext"),
			    (s->uttfile ? s->uttfile : s->uttid), s->uttid,
			    cmd_ln_boolean_r(kbcore_config(s->kbc), "-build_outdirs"));

		E_INFO("Writing lattice file: %s\n", str);
		dag_write(s->dag, str, kbcore_lm(s->kbc), kbcore_dict(s->kbc));
	    }
	}
    }

    /* Write N-best lists */
    if (s->dag && s->funcs->nbest_impl && cmd_ln_str_r(kbcore_config(s->kbc), "-nbestdir")) {
	s->funcs->nbest_impl(s, s->dag);
    }

    /* Do second stage search */
    if (s->dag && s->funcs->bestpath_impl &&  cmd_ln_boolean_r(kbcore_config(s->kbc), "-bestpath")) {
	glist_t rhyp;

        ptmr_start(&(st->tm_srch));
	rhyp = s->funcs->bestpath_impl(s, s->dag);
        ptmr_stop(&(st->tm_srch));
	if (rhyp == NULL) {
	    E_ERROR("Bestpath search failed.\n");
	}
	else {
	    if (cmd_ln_int32_r(kbcore_config(s->kbc), "-backtrace"))
		match_detailed(stdout, rhyp, s->uttid, "BP",
			       "bp", s->ascale,
			       kbcore_dict(s->kbc));
	    if (s->matchfp)
		match_write(s->matchfp, rhyp, s->uttid, kbcore_dict(s->kbc), NULL);
	    if (s->matchsegfp)
		matchseg_write(s->matchsegfp, rhyp, s->uttid, NULL,
			       kbcore_lm(s->kbc),
			       kbcore_dict(s->kbc), s->stat->nfr, s->ascale,
			       cmd_ln_int32_r(kbcore_config(s->kbc), "-hypsegscore_unscale"));
	    match_write(err_get_logfp(), rhyp, s->uttid,
			kbcore_dict(s->kbc), "BSTPTH: ");
	    matchseg_write(err_get_logfp(), rhyp, s->uttid, "BSTXCT: ",
			   kbcore_lm(s->kbc), kbcore_dict(s->kbc),
			   s->stat->nfr, s->ascale,
			   cmd_ln_int32_r(kbcore_config(s->kbc), "-hypsegscore_unscale"));
	    for (gn = rhyp; gn; gn = gnode_next(gn)) {
		ckd_free(gnode_ptr(gn));
	    }
	    glist_free(rhyp);
	}
    }

    for (gn = hyp; gn; gn = gnode_next(gn)) {
        ckd_free(gnode_ptr(gn));
    }
    glist_free(hyp);

    stat_report_utt(st, s->uttid);
    stat_update_corpus(st);

    ptmr_reset(&(st->tm_sen));
    ptmr_reset(&(st->tm_srch));
    ptmr_reset(&(st->tm_ovrhd));

    return SRCH_SUCCESS;
}

int32
srch_utt_decode_blk(srch_t * s, float ***block_feat, int32 block_nfeatvec,
                    int32 * curfrm)
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
    if (s->funcs->decode != NULL) {
        return s->funcs->decode((void *) s);
    }

    /* Else, go over the default search abstraction */

    /* the effective window is the min of (s->cache_win, block_nfeatvec) */
    win_efv = s->cache_win;
    if (win_efv > block_nfeatvec)
        win_efv = block_nfeatvec;


    s->num_frm = frmno;         /* Make a copy to the structure */


    /* 20060413 ARCHAN: Bug Fix 1459402, the old segments failed to
       increase the size of the memory buffer properly
       20060624 ARCHAN/: Bug Fix from anonymous user is correct if the
       requested size of memory is within 2 * DFLT_UTT_SIZE, when the 
       requested size of memory is more than that, the code breaks. 
       Change if to while to fix this problem. 
     */


    while (block_nfeatvec + frmno >= s->ascale_sz) {
        E_INFO("Reallocate s->ascale. s->ascale_sz %d\n",
               s->ascale_sz + DFLT_UTT_SIZE);
        s->ascale = (int32 *)
            ckd_realloc(s->ascale, (s->ascale_sz + DFLT_UTT_SIZE) *
                        sizeof(int32));
        s->ascale_sz += DFLT_UTT_SIZE;
    }

    if (s->num_segs >= s->segsz_sz) {
        s->segsz = (int32 *)
            ckd_realloc(s->segsz, (s->segsz_sz + DFLT_NUM_SEGS) *
                        sizeof(int32));
        s->segsz_sz += DFLT_NUM_SEGS;
    }

    s->segsz[s->num_segs] = win_efv;
    s->num_segs++;

#if 0
    s->segsz[s->num_segs] = win_efv;
    s->num_segs++;

    if (frmno + win_efv > s->ascale_sz) {
        s->ascale = (int32 *)
            ckd_realloc(s->ascale, (s->ascale_sz + DFLT_UTT_SIZE));
        s->ascale_sz += DFLT_UTT_SIZE;
    }

    if (s->num_segs == s->segsz_sz) {
        s->segsz = (int32 *)
            ckd_realloc(s->segsz, (s->segsz_sz + DFLT_NUM_SEGS));
        s->segsz_sz += DFLT_NUM_SEGS;
    }
#endif

    s->cache_win_strt = 0;

    /*Compute the CI senone score at here */
    ptmr_start(&(st->tm_sen));
    ptmr_start(&(st->tm_ovrhd));

    for (f = 0; f < win_efv; f++) {
        s->funcs->gmm_compute_lv1(s, block_feat[f][0], f, f);
    }

    ptmr_stop(&(st->tm_ovrhd));
    ptmr_stop(&(st->tm_sen));

    for (t = 0; t < block_nfeatvec; t++, frmno++) {

        /* Acoustic (senone scores) evaluation */
        ptmr_start(&(st->tm_sen));
        s->funcs->select_active_gmm(s);
        s->funcs->gmm_compute_lv2(s, block_feat[t], t);
        s->ascale[s->num_frm + t] = s->senscale;

        /*
           A common situation where the decoding scores screwed up 
           is when some of the senone corrupted by either poor
           model parameters or poorly formed feature parameters. Turn
           the following condition to debug this problem.  Notice 
           that this will generate a lot of information. 
         */
        if (SHOW_SENONE_SCORE_FOR_FRAME) {
            E_INFO("At frame %d \n", t);
            ascr_print_senscr(s->ascr);
        }



        ptmr_stop(&(st->tm_sen));


        /* Propagate graph at phoneme (hmm) level */
        ptmr_start(&(st->tm_srch));


        if (s->funcs->one_srch_frame_lv2 != NULL) {       /* If user provided only how
                                                           to search_one_frame
                                                           function, then use it
                                                           instead of going through
                                                           the standard abstraction.
                                                         */
            s->funcs->one_srch_frame_lv2(s);

        }
        else {
            /* Determine which set of phonemes should be active in next stage
               using the lookahead information */
            /* This should be part of hmm_compute_lv1 */
            if (COMPUTE_HEURISTIC)
                s->funcs->compute_heuristic(s, win_efv);
            /* HMM compute Lv 2, currently, this routine compute hmm for the
             *  data structure and compute the beam. */
            s->funcs->hmm_compute_lv2(s, frmno);

            /* After the HMM scores are computed, tokens are propagate in the
             * phone-level.  */
            if (s->funcs->propagate_graph_ph_lv2(s, frmno) != SRCH_SUCCESS) {
                E_ERROR("Code failed in srch_propagate_graph_ph_lv2\n");
                return SRCH_FAILURE;
            }

            /* Rescoring. Usually happened at the word end.  */
            if (s->funcs->rescoring != NULL)
                s->funcs->rescoring(s, frmno);

            /* Propagate the score on the word-level */
            if (s->funcs->propagate_graph_wd_lv2(s, frmno) != SRCH_SUCCESS) {
                E_ERROR("Code failed in srch_propagate_graph_wd_lv2\n");
                return SRCH_FAILURE;
            }
        }
        ptmr_stop(&(st->tm_srch));

        ptmr_start(&(st->tm_sen));
        ptmr_start(&(st->tm_ovrhd));
        /* if the current block's current frame (t) is less than the total
           frames in this block minus the efv window */

        if (t < block_nfeatvec - win_efv) {
            s->funcs->shift_one_cache_frame(s, win_efv);
            s->funcs->gmm_compute_lv1(s, block_feat[t + win_efv][0],
                                    win_efv - 1, t + win_efv);
        }
        else {                  /* We are near the end of the block, so shrink the window from the left */
            s->cache_win_strt++;
        }

        ptmr_stop(&(st->tm_ovrhd));
        ptmr_stop(&(st->tm_sen));

        s->funcs->frame_windup(s, frmno);

        if (frmno % 10 == 0) {
            E_INFOCONT(".");
        }
    }
    E_INFOCONT("\n");

    st->nfr += block_nfeatvec;

    *curfrm = frmno;

    return SRCH_SUCCESS;
}


/** Wrap up the search routine*/
int32
srch_uninit(srch_t * srch)
{
    if (srch->funcs->uninit == NULL) {
        E_ERROR("Search un-initialization failed\n");
        return SRCH_FAILURE;
    }
    srch->funcs->uninit(srch);

    if (srch->dag)
	dag_destroy(srch->dag);
    ckd_free(srch->segsz);
    ckd_free(srch->ascale);
    ckd_free(srch->grh);
    ckd_free(srch);

    return SRCH_SUCCESS;
}

/** Wrap up the search report routine*/
void
srch_report(srch_t * srch)
{

    char *op_str;
    op_str = srch_mode_index_to_str(srch->op_mode);

    E_INFO_NOFN("Initialization of srch_t, report:\n");
    E_INFO_NOFN("Operation Mode = %d, Operation Name = %s\n",
                srch->op_mode, op_str);
    E_INFO_NOFN("\n");

    ckd_free(op_str);
}

/** Get a current recognition hypothesis. */
glist_t
srch_get_hyp(srch_t *srch)
{
    if (srch->funcs->gen_hyp == NULL) {
        E_ERROR
            ("srch->funcs->gen_hyp is NULL. Please make sure it is set.\n");
        return NULL;
    }
    return srch->funcs->gen_hyp(srch);
}

dag_t *
srch_get_dag(srch_t *s)
{
    glist_t hyp = NULL;
    gnode_t *gn;

    if (s->funcs->gen_dag == NULL) {
	E_ERROR("Cannot generate DAG in current search mode.\n");
	return NULL;
    }
    if (s->dag == NULL) {
	if (s->funcs->gen_hyp == NULL) {
	    E_WARN("srch->funcs->gen_hyp is NULL.  Please make sure it is set.\n");
	    return NULL;
	}
	if ((hyp = s->funcs->gen_hyp(s)) == NULL) {
	    E_ERROR("s->funcs->gen_hyp failed\n");
	    return NULL;
	}
	if ((s->dag = s->funcs->gen_dag(s, hyp)) == NULL) {
	    E_ERROR("Failed to generate DAG.\n");
	    goto free_hyp;
	}
    }
free_hyp:
    for (gn = hyp; gn; gn = gnode_next(gn)) {
        ckd_free(gnode_ptr(gn));
    }
    glist_free(hyp);
    return s->dag;
}

/** using file name of the LM or defined lmctlfn mechanism */
int32
srch_set_lm(srch_t * srch, const char *lmname)
{

    if (srch->funcs->set_lm == NULL) {
        E_INFO
            ("srch->funcs->set_lm is NULL. Please make sure it is set. No change will be made currently. \n");
        return SRCH_FAILURE;
    }
    /* srch should own resource such as kbc and acoustic models */
    srch->funcs->set_lm(srch, lmname);
    return SRCH_SUCCESS;
}

int32
srch_add_lm(srch_t * srch, lm_t * lm, const char *lmname)
{
    if (srch->funcs->add_lm == NULL) {
        E_INFO
            ("srch->funcs->add_lm is NULL. Please make sure it is set. No change will be made currently. \n");
        return SRCH_FAILURE;
    }

    srch->funcs->add_lm(srch, lm, lmname);
    return SRCH_SUCCESS;
}

int32
srch_delete_lm(srch_t * srch, const char *lmname)
{
    if (srch->funcs->delete_lm == NULL) {
        E_INFO
            ("srch->funcs->delete_lm is NULL. Please make sure it is set. No change will be made currently. \n");
        return SRCH_FAILURE;
    }

    srch->funcs->delete_lm(srch, lmname);
    return SRCH_SUCCESS;
}

void
write_bstsenscr(FILE * fp, int32 numframe, int32 * scale)
{

    int32 i;
    fprintf(fp, "NumFrame %d\n", numframe);
    for (i = 0; i < numframe; i++)
        fprintf(fp, "%d %d\n", i, scale[i]);
}
