/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * fsg_search.c -- Search structures for FSM decoding.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 *
 * 18-Feb-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */

/* System headers. */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* SphinxBase headers. */
#include <err.h>
#include <ckd_alloc.h>
#include <cmd_ln.h>

/* Local headers. */
#include "search.h"
#include "search_const.h"
#include "phone.h"
#include "fsg_search.h"
#include "senscr.h"
#include "kb.h"
#include "fbs.h"
#include "dict.h"
#include "log.h"


#define FSG_SEARCH_IDLE		0
#define FSG_SEARCH_BUSY		1

/* Turn this on for detailed debugging dump */
#define __FSG_DBG__		0
#define __FSG_DBG_CHAN__	0


fsg_search_t *
fsg_search_init(cmd_ln_t *config, word_fsg_t * fsg)
{
    fsg_search_t *search;
    float32 lw;
    int32 pip, wip;

    search = (fsg_search_t *) ckd_calloc(1, sizeof(fsg_search_t));
    search->config = config;
    search->fsg = fsg;

    if (fsg) {
        search->fsglist = glist_add_ptr(NULL, (void *) fsg);
        search->lextree = fsg_lextree_init(config, fsg);
    }
    else {
        search->fsglist = NULL;
        search->lextree = NULL;
    }

    /* Intialize the search history object */
    search->history = fsg_history_init(fsg);

    /* Initialize the active lists */
    search->pnode_active = NULL;
    search->pnode_active_next = NULL;

    search->frame = -1;

    search->hyp = NULL;

    search->state = FSG_SEARCH_IDLE;

    /* Get search pruning parameters */
    search_get_logbeams(&(search->beam_orig),
                        &(search->pbeam_orig), &(search->wbeam_orig));
    search->beam_factor = 1.0f;
    search->beam = search->beam_orig;
    search->pbeam = search->pbeam_orig;
    search->wbeam = search->wbeam_orig;

    /* LM related weights/penalties */
    lw = cmd_ln_float32_r(config, "-lw");
    pip = (int32) (logmath_log(lmath, cmd_ln_float32_r(config, "-pip")) * lw);
    wip = (int32) (logmath_log(lmath, cmd_ln_float32_r(config, "-wip")) * lw);

    E_INFO("FSG(beam: %d, pbeam: %d, wbeam: %d; wip: %d, pip: %d)\n",
           search->beam_orig, search->pbeam_orig, search->wbeam_orig,
           wip, pip);

    return search;
}


word_fsg_t *
fsg_search_fsgname_to_fsg(fsg_search_t * search, char *name)
{
    gnode_t *gn;
    word_fsg_t *fsg;

    for (gn = search->fsglist; gn; gn = gnode_next(gn)) {
        fsg = (word_fsg_t *) gnode_ptr(gn);
        if (strcmp(name, word_fsg_name(fsg)) == 0)
            return fsg;
    }

    return NULL;
}


boolean
fsg_search_add_fsg(fsg_search_t * search, word_fsg_t * fsg)
{
    word_fsg_t *oldfsg;

    /* Check to make sure search is in a quiescent state */
    if (search->state != FSG_SEARCH_IDLE) {
        E_ERROR("Attempt to switch FSG inside an utterance\n");
        return FALSE;
    }

    /* Make sure no existing FSG has the same name as the given one */
    oldfsg = fsg_search_fsgname_to_fsg(search, word_fsg_name(fsg));
    if (oldfsg) {
        E_ERROR("FSG name '%s' already exists\n", word_fsg_name(fsg));
        return FALSE;
    }

    search->fsglist = glist_add_ptr(search->fsglist, (void *) fsg);
    return TRUE;
}


boolean
fsg_search_del_fsg(fsg_search_t * search, word_fsg_t * fsg)
{
    gnode_t *gn, *prev, *next;
    word_fsg_t *oldfsg;

    /* Check to make sure search is in a quiescent state */
    if (search->state != FSG_SEARCH_IDLE) {
        E_ERROR("Attempt to switch FSG inside an utterance\n");
        return FALSE;
    }

    /* Search fsglist for the given fsg */
    prev = NULL;
    for (gn = search->fsglist; gn; gn = next) {
        oldfsg = (word_fsg_t *) gnode_ptr(gn);
        next = gnode_next(gn);
        if (oldfsg == fsg) {
            /* Found the FSG to be deleted; remove it from fsglist */
            if (prev == NULL)
                search->fsglist = next;

            gnode_free (gn, prev);

            /* If this was the currently active FSG, also delete other stuff */
            if (search->fsg == fsg) {
                fsg_lextree_free(search->lextree);
                search->lextree = NULL;

                fsg_history_set_fsg(search->history, NULL);

                search->fsg = NULL;
            }

            E_INFO("Deleting FSG '%s'\n", word_fsg_name(fsg));

            word_fsg_free(fsg);

            return TRUE;
        }
        else
            prev = gn;
    }

    E_WARN("FSG '%s' to be deleted not found\n", word_fsg_name(fsg));

    return TRUE;
}


boolean
fsg_search_del_fsg_byname(fsg_search_t * search, char *name)
{
    word_fsg_t *fsg;

    fsg = fsg_search_fsgname_to_fsg(search, name);
    if (!fsg) {
        E_WARN("FSG name '%s' to be deleted not found\n", name);
        return TRUE;
    }
    else
        return fsg_search_del_fsg(search, fsg);
}


boolean
fsg_search_set_current_fsg(fsg_search_t * search, char *name)
{
    word_fsg_t *fsg;

    /* Check to make sure search is in a quiescent state */
    if (search->state != FSG_SEARCH_IDLE) {
        E_ERROR("Attempt to switch FSG inside an utterance\n");
        return FALSE;
    }

    fsg = fsg_search_fsgname_to_fsg(search, name);
    if (!fsg) {
        E_ERROR("FSG '%s' not known; cannot make it current\n", name);
        return FALSE;
    }

    /* Free the old lextree */
    if (search->lextree)
        fsg_lextree_free(search->lextree);

    /* Allocate new lextree for the given FSG */
    search->lextree = fsg_lextree_init(search->config, fsg);

    /* Inform the history module of the new fsg */
    fsg_history_set_fsg(search->history, fsg);

    search->fsg = fsg;

    return TRUE;
}


void
fsg_search_free(fsg_search_t * search)
{
    E_FATAL("NOT IMPLEMENTED\n");
}


void
fsg_search_sen_active(fsg_search_t * search)
{
    gnode_t *gn;
    fsg_pnode_t *pnode;
    hmm_t *hmm;

    sen_active_clear();

    for (gn = search->pnode_active; gn; gn = gnode_next(gn)) {
        pnode = (fsg_pnode_t *) gnode_ptr(gn);
        hmm = fsg_pnode_hmmptr(pnode);
        assert(hmm_frame(hmm) == search->frame);

        hmm_sen_active(hmm);
    }

    sen_active_flags2list();

    search->n_sen_eval += n_senone_active;
}


/*
 * Evaluate all the active HMMs.
 * (Executed once per frame.)
 */
void
fsg_search_hmm_eval(fsg_search_t * search)
{
    gnode_t *gn;
    fsg_pnode_t *pnode;
    hmm_t *hmm;
    int32 bestscore;
    int32 n, maxhmmpf;

    bestscore = WORST_SCORE;

    if (!search->pnode_active) {
        E_ERROR("Frame %d: No active HMM!!\n", search->frame);
        return;
    }

    for (n = 0, gn = search->pnode_active; gn; gn = gnode_next(gn), n++) {
        int32 score;

        pnode = (fsg_pnode_t *) gnode_ptr(gn);
        hmm = fsg_pnode_hmmptr(pnode);
        assert(hmm_frame(hmm) == search->frame);

#if __FSG_DBG__
        E_INFO("pnode(%08x) active @frm %5d\n", (int32) pnode,
               search->frame);
        hmm_dump(hmm, stdout);
#endif
        score = hmm_vit_eval(hmm);
#if __FSG_DBG_CHAN__
        E_INFO("pnode(%08x) after eval @frm %5d\n",
               (int32) pnode, search->frame);
        hmm_dump(hmm, stdout);
#endif

        if (bestscore < score)
            bestscore = score;
    }

#if __FSG_DBG__
    E_INFO("[%5d] %6d HMM; bestscr: %11d\n", search->frame, n, bestscore);
#endif
    search->n_hmm_eval += n;

    /* Adjust beams if #active HMMs larger than absolute threshold */
    maxhmmpf = cmd_ln_int32_r(search->config, "-maxhmmpf");
    if (maxhmmpf != -1 && n > maxhmmpf) {
        /*
         * Too many HMMs active; reduce the beam factor applied to the default
         * beams, but not if the factor is already at a floor (0.1).
         */
        if (search->beam_factor > 0.1) {        /* Hack!!  Hardwired constant 0.1 */
            search->beam_factor *= 0.9f;        /* Hack!!  Hardwired constant 0.9 */
            search->beam =
                (int32) (search->beam_orig * search->beam_factor);
            search->pbeam =
                (int32) (search->pbeam_orig * search->beam_factor);
            search->wbeam =
                (int32) (search->wbeam_orig * search->beam_factor);
        }
    }
    else {
        search->beam_factor = 1.0f;
        search->beam = search->beam_orig;
        search->pbeam = search->pbeam_orig;
        search->wbeam = search->wbeam_orig;
    }

    if (n > fsg_lextree_n_pnode(search->lextree))
        E_FATAL("PANIC! Frame %d: #HMM evaluated(%d) > #PNodes(%d)\n",
                search->frame, n, fsg_lextree_n_pnode(search->lextree));

    search->bestscore = bestscore;
}


static void
fsg_search_pnode_trans(fsg_search_t * search, fsg_pnode_t * pnode)
{
    fsg_pnode_t *child;
    hmm_t *hmm;
    int32 newscore, thresh, nf;

    assert(pnode);
    assert(!fsg_pnode_leaf(pnode));

    nf = search->frame + 1;
    thresh = search->bestscore + search->beam;

    hmm = fsg_pnode_hmmptr(pnode);

    for (child = fsg_pnode_succ(pnode);
         child; child = fsg_pnode_sibling(child)) {
        newscore = hmm_out_score(hmm) + child->logs2prob;

        if ((newscore >= thresh) && (newscore > hmm_in_score(&child->hmm))) {
            /* Incoming score > pruning threshold and > target's existing score */
            if (hmm_frame(&child->hmm) < nf) {
                /* Child node not yet activated; do so */
                search->pnode_active_next =
                    glist_add_ptr(search->pnode_active_next,
                                  (void *) child);
            }

            hmm_enter(&child->hmm, newscore, hmm_out_history(hmm), nf);
        }
    }
}


static void
fsg_search_pnode_exit(fsg_search_t * search, fsg_pnode_t * pnode)
{
    hmm_t *hmm;
    word_fsglink_t *fl;
    int32 wid, endwid;
    fsg_pnode_ctxt_t ctxt;

    assert(pnode);
    assert(fsg_pnode_leaf(pnode));

    hmm = fsg_pnode_hmmptr(pnode);
    fl = fsg_pnode_fsglink(pnode);
    assert(fl);

    endwid = kb_get_word_id("</s>");

    wid = word_fsglink_wid(fl);
    assert(wid >= 0);

#if __FSG_DBG__
    E_INFO("[%5d] Exit(%08x) %10d(score) %5d(pred)\n",
           search->frame, (int32) pnode,
           hmm_out_score(hmm), hmm_out_history(hmm));
#endif

    /*
     * Check if this is filler or single phone word; these do not model right
     * context (i.e., the exit score applies to all right contexts).
     */
    if (dict_is_filler_word(word_dict, wid) ||
        (wid == endwid) || (dict_pronlen(word_dict, wid) == 1)) {
        /* Create a dummy context structure that applies to all right contexts */
        fsg_pnode_add_all_ctxt(&ctxt);

        /* Create history table entry for this word exit */
        fsg_history_entry_add(search->history,
                              fl,
                              search->frame,
                              hmm_out_score(hmm),
                              hmm_out_history(hmm),
                              pnode->ci_ext, ctxt);

    }
    else {
        /* Create history table entry for this word exit */
        fsg_history_entry_add(search->history,
                              fl,
                              search->frame,
                              hmm_out_score(hmm),
                              hmm_out_history(hmm),
                              pnode->ci_ext, pnode->ctxt);
    }
}


/*
 * (Beam) prune the just evaluated HMMs, determine which ones remain
 * active, which ones transition to successors, which ones exit and
 * terminate in their respective destination FSM states.
 * (Executed once per frame.)
 */
void
fsg_search_hmm_prune_prop(fsg_search_t * search)
{
    gnode_t *gn;
    fsg_pnode_t *pnode;
    hmm_t *hmm;
    int32 thresh, word_thresh, phone_thresh;

    assert(search->pnode_active_next == NULL);

    thresh = search->bestscore + search->beam;
    phone_thresh = search->bestscore + search->pbeam;
    word_thresh = search->bestscore + search->wbeam;

    for (gn = search->pnode_active; gn; gn = gnode_next(gn)) {
        pnode = (fsg_pnode_t *) gnode_ptr(gn);
        hmm = fsg_pnode_hmmptr(pnode);

        if (hmm_bestscore(hmm) >= thresh) {
            /* Keep this HMM active in the next frame */
            if (hmm_frame(hmm) == search->frame) {
                hmm_frame(hmm) = search->frame + 1;
                search->pnode_active_next =
                    glist_add_ptr(search->pnode_active_next,
                                  (void *) pnode);
            }
            else {
                assert(hmm_frame(hmm) == search->frame + 1);
            }

            if (!fsg_pnode_leaf(pnode)) {
                if (hmm_out_score(hmm) >= phone_thresh) {
                    /* Transition out of this phone into its children */
                    fsg_search_pnode_trans(search, pnode);
                }
            }
            else {
                if (hmm_out_score(hmm) >= word_thresh) {
                    /* Transition out of leaf node into destination FSG state */
                    fsg_search_pnode_exit(search, pnode);
                }
            }
        }
    }
}


/*
 * Propagate newly created history entries through null transitions.
 */
static void
fsg_search_null_prop(fsg_search_t * search)
{
    int32 bpidx, n_entries, thresh, newscore;
    fsg_hist_entry_t *hist_entry;
    word_fsglink_t *l;
    int32 s, d;
    word_fsg_t *fsg;

    fsg = search->fsg;
    thresh = search->bestscore + search->wbeam; /* Which beam really?? */

    n_entries = fsg_history_n_entries(search->history);

    for (bpidx = search->bpidx_start; bpidx < n_entries; bpidx++) {
        hist_entry = fsg_history_entry_get(search->history, bpidx);

        l = fsg_hist_entry_fsglink(hist_entry);

        /* Destination FSG state for history entry */
        s = l ? word_fsglink_to_state(l) : word_fsg_start_state(fsg);

        /*
         * Check null transitions from d to all other states.  (Only need to
         * propagate one step, since FSG contains transitive closure of null
         * transitions.)
         */
        for (d = 0; d < word_fsg_n_state(fsg); d++) {
            l = word_fsg_null_trans(fsg, s, d);

            if (l) {            /* Propagate history entry through this null transition */
                newscore =
                    fsg_hist_entry_score(hist_entry) +
                    word_fsglink_logs2prob(l);

                if (newscore >= thresh) {
                    fsg_history_entry_add(search->history, l,
                                          fsg_hist_entry_frame(hist_entry),
                                          newscore,
                                          bpidx,
                                          fsg_hist_entry_lc(hist_entry),
                                          fsg_hist_entry_rc(hist_entry));
                }
            }
        }
    }
}


/*
 * Perform cross-word transitions; propagate each history entry created in this
 * frame to lextree roots attached to the target FSG state for that entry.
 */
static void
fsg_search_word_trans(fsg_search_t * search)
{
    int32 bpidx, n_entries;
    fsg_hist_entry_t *hist_entry;
    word_fsglink_t *l;
    int32 score, newscore, thresh, nf, d;
    fsg_pnode_t *root;
    int32 lc, rc;

    n_entries = fsg_history_n_entries(search->history);

    thresh = search->bestscore + search->beam;
    nf = search->frame + 1;

    for (bpidx = search->bpidx_start; bpidx < n_entries; bpidx++) {
        hist_entry = fsg_history_entry_get(search->history, bpidx);
        assert(hist_entry);
        score = fsg_hist_entry_score(hist_entry);
        assert(search->frame == fsg_hist_entry_frame(hist_entry));

        l = fsg_hist_entry_fsglink(hist_entry);

        /* Destination state for hist_entry */
        d = l ? word_fsglink_to_state(l) : word_fsg_start_state(search->
                                                                fsg);

        lc = fsg_hist_entry_lc(hist_entry);

        /* Transition to all root nodes attached to state d */
        for (root = fsg_lextree_root(search->lextree, d);
             root; root = root->sibling) {
            rc = root->ci_ext;

            if ((root->ctxt.bv[lc >> 5] & (1 << (lc & 0x001f))) &&
                (hist_entry->rc.bv[rc >> 5] & (1 << (rc & 0x001f)))) {
                /*
                 * Last CIphone of history entry is in left-context list supported by
                 * target root node, and
                 * first CIphone of target root node is in right context list supported
                 * by history entry;
                 * So the transition can go ahead (if new score is good enough).
                 */
                newscore = score + root->logs2prob;

                if ((newscore >= thresh)
                    && (newscore > hmm_in_score(&root->hmm))) {
                    if (hmm_frame(&root->hmm) < nf) {
                        /* Newly activated node; add to active list */
                        search->pnode_active_next =
                            glist_add_ptr(search->pnode_active_next,
                                          (void *) root);
#if __FSG_DBG__
                        E_INFO
                            ("[%5d] WordTrans bpidx[%d] -> pnode[%08x] (activated)\n",
                             search->frame, bpidx, (int32) root);
#endif
                    }
                    else {
#if __FSG_DBG__
                        E_INFO
                            ("[%5d] WordTrans bpidx[%d] -> pnode[%08x]\n",
                             search->frame, bpidx, (int32) root);
#endif
                    }

                    hmm_enter(&root->hmm, newscore, bpidx, nf);
                }
            }
        }
    }
}


void
fsg_search_frame_fwd(fsg_search_t * search)
{
    gnode_t *gn;
    fsg_pnode_t *pnode;
    hmm_t *hmm;

    search->bpidx_start = fsg_history_n_entries(search->history);

    /* Evaluate all active pnodes (HMMs) */
    fsg_search_hmm_eval(search);

    /*
     * Prune and propagate the HMMs evaluated; create history entries for
     * word exits.  The words exits are tentative, and may be pruned; make
     * the survivors permanent via fsg_history_end_frame().
     */
    fsg_search_hmm_prune_prop(search);
    fsg_history_end_frame(search->history);

    /*
     * Propagate new history entries through any null transitions, creating
     * new history entries, and then make the survivors permanent.
     */
    fsg_search_null_prop(search);
    fsg_history_end_frame(search->history);

    /*
     * Perform cross-word transitions; propagate each history entry across its
     * terminating state to the root nodes of the lextree attached to the state.
     */
    fsg_search_word_trans(search);

    /*
     * We've now come full circle, HMM and FSG states have been updated for
     * the next frame.
     * Update the active lists, deactivate any currently active HMMs that
     * did not survive into the next frame
     */
    for (gn = search->pnode_active; gn; gn = gnode_next(gn)) {
        pnode = (fsg_pnode_t *) gnode_ptr(gn);
        hmm = fsg_pnode_hmmptr(pnode);

        if (hmm_frame(hmm) == search->frame) {
            /* This HMM NOT activated for the next frame; reset it */
            fsg_psubtree_pnode_deactivate(pnode);
        }
        else {
            assert(hmm_frame(hmm) == (search->frame + 1));
        }
    }

    /* Free the currently active list */
    glist_free(search->pnode_active);

    /* Make the next-frame active list the current one */
    search->pnode_active = search->pnode_active_next;
    search->pnode_active_next = NULL;

    /* End of this frame; ready for the next */
    (search->frame)++;
}


static void
fsg_search_hyp_free(fsg_search_t * search)
{
    search_hyp_t *hyp, *nexthyp;

    for (hyp = search->hyp; hyp; hyp = nexthyp) {
        nexthyp = hyp->next;
        ckd_free(hyp);
    }
    search->hyp = NULL;
}


/*
 * Set all HMMs to inactive, clear active lists, initialize FSM start
 * state to be the only active node.
 * (Executed at the start of each utterance.)
 */
void
fsg_search_utt_start(fsg_search_t * search)
{
    int32 silcipid;
    fsg_pnode_ctxt_t ctxt;

    /* Reset dynamic adjustment factor for beams */
    search->beam_factor = 1.0f;
    search->beam = search->beam_orig;
    search->pbeam = search->pbeam_orig;
    search->wbeam = search->wbeam_orig;

    silcipid = phone_to_id("SIL",  TRUE);

    /* Initialize EVERYTHING to be inactive */
    assert(search->pnode_active == NULL);
    assert(search->pnode_active_next == NULL);

    fsg_lextree_utt_start(search->lextree);
    fsg_history_utt_start(search->history);

    /* Dummy context structure that allows all right contexts to use this entry */
    fsg_pnode_add_all_ctxt(&ctxt);

    /* Create dummy history entry leading to start state */
    search->frame = -1;
    search->bestscore = 0;
    fsg_history_entry_add(search->history,
                          NULL, -1, 0, -1, silcipid, ctxt);
    search->bpidx_start = 0;

    /* Propagate dummy history entry through NULL transitions from start state */
    fsg_search_null_prop(search);

    /* Perform word transitions from this dummy history entry */
    fsg_search_word_trans(search);

    /* Make the next-frame active list the current one */
    search->pnode_active = search->pnode_active_next;
    search->pnode_active_next = NULL;

    (search->frame)++;

    fsg_search_hyp_free(search);

    search->n_hmm_eval = 0;
    search->n_sen_eval = 0;

    search->state = FSG_SEARCH_BUSY;
}


static void
fsg_search_hyp_dump(fsg_search_t * search, FILE * fp)
{
    search_hyp_t *hyp;
    int32 nf;

    /* Print backtrace */
    fprintf(fp, "\t%4s %4s %10s %11s %9s %11s %10s %6s  %s (FSG) (%s)\n",
            "SFrm", "EFrm", "AScr/Frm", "AScr", "LScr", "AScr+LScr",
            "(A-BS)/Frm", "State", "Word", uttproc_get_uttid());
    fprintf(fp,
            "\t-------------------------------------------------------------------------------\n");
    for (hyp = search->hyp; hyp; hyp = hyp->next) {
        nf = hyp->ef - hyp->sf + 1;
        fprintf(fp, "\t%4d %4d %10d %11d %9d %11d %10d %6d  %s\n",
                hyp->sf, hyp->ef,
                (nf > 0) ? hyp->ascr / nf : 0,
                hyp->ascr, hyp->lscr, hyp->ascr + hyp->lscr, ((nf > 0)
                                                              && (hyp->
                                                                  ascr !=
                                                                  0))
                ? (seg_topsen_score(hyp->sf, hyp->ef) -
                   hyp->ascr) / nf : 0, hyp->fsg_state, hyp->word);
    }
    fprintf(fp,
            "\t-------------------------------------------------------------------------------\n");
    fprintf(fp, "\t%4d %4d %10d %11d %9d %11d %10d %6dF %s(TOTAL)\n", 0,
            search->frame - 1,
            (search->frame > 0) ? (search->ascr / search->frame) : 0,
            search->ascr, search->lscr, search->ascr + search->lscr,
            (search->frame >
             0) ? (seg_topsen_score(0,
                                    search->frame - 1) -
                   search->ascr) / search->frame : 0,
            word_fsg_final_state(search->fsg), uttproc_get_uttid());

    fflush(fp);
}


/* Fill in hyp_str in search.c; filtering out fillers and null trans */
static void
fsg_search_hyp_filter(fsg_search_t * search)
{
    search_hyp_t *hyp, *filt_hyp;
    int32 i;
    int32 startwid, finishwid;
    int32 altpron;

    filt_hyp = search_get_hyp();
    startwid = kb_get_word_id("<s>");
    finishwid = kb_get_word_id("</s>");
    altpron = cmd_ln_boolean_r(search->config, "-reportpron");

    i = 0;
    for (hyp = search->hyp; hyp; hyp = hyp->next) {
        if ((hyp->wid < 0) ||
            (hyp->wid == startwid) || (hyp->wid >= finishwid))
            continue;

        /* Copy this hyp entry to filtered result */
        filt_hyp[i] = *hyp;

        /* Replace specific word pronunciation ID with base ID */
        if (!altpron)
            filt_hyp[i].wid = dictid_to_baseid(word_dict, filt_hyp[i].wid);

        i++;
        if ((i + 1) >= HYP_SZ)
            E_FATAL("Hyp array overflow; increase HYP_SZ in search.h\n");
    }

    filt_hyp[i].wid = -1;       /* Sentinel */
}


/*
 * Push result into old search module, from where we can obtain the result
 * using the old API.
 */
static void
fsg_search_set_result(fsg_search_t * search)
{
    fsg_search_hyp_filter(search);
    searchSetFrame(search->frame);
    search_set_hyp_wid(search->hyp);
    search_hyp_to_str();
    search_set_hyp_total_score(search->ascr + search->lscr);
    search_set_hyp_total_lscr(search->lscr);
}


void
fsg_search_history_backtrace(fsg_search_t * search,
                             boolean check_fsg_final_state)
{
    word_fsg_t *fsg;
    fsg_hist_entry_t *hist_entry;
    word_fsglink_t *fl;
    int32 bestscore, bestscore_finalstate, besthist_finalstate, besthist;
    int32 bpidx, score, frm, last_frm;
    search_hyp_t *hyp, *head;

    /* Free any existing search hypothesis */
    fsg_search_hyp_free(search);
    search->ascr = 0;
    search->lscr = 0;

    fsg = search->fsg;

    /* Find most recent bestscoring history entry */
    bpidx = fsg_history_n_entries(search->history) - 1;
    if (bpidx > 0) {
        hist_entry = fsg_history_entry_get(search->history, bpidx);
        last_frm = frm = fsg_hist_entry_frame(hist_entry);
        assert(frm < search->frame);
    }
    else {
        hist_entry = NULL;
        last_frm = frm = -1;
    }

    if ((bpidx <= 0) || (last_frm < 0)) {
        /* Only the dummy root entry, or null transitions from it, exist */
        if (check_fsg_final_state)
            E_WARN("Empty utterance: %s\n", uttproc_get_uttid());

        /* Set result (empty recognition) in backward compatible format */
        fsg_search_set_result(search);

        return;
    }

    if (check_fsg_final_state) {
        if (frm < (search->frame - 1)) {
            E_WARN
                ("No history entry in the final frame %d; using last entry at frame %d\n",
                 search->frame - 1, frm);
        }
    }

    /*
     * Find best history entry, as well as best entry leading to FSG final state
     * in final frame.
     */
    bestscore = bestscore_finalstate = (int32) 0x80000000;
    besthist = besthist_finalstate = -1;

    while (frm == last_frm) {
        fl = fsg_hist_entry_fsglink(hist_entry);
        score = fsg_hist_entry_score(hist_entry);

        if (word_fsglink_to_state(fl) == word_fsg_final_state(fsg)) {
            if (score > bestscore_finalstate) {
                bestscore_finalstate = score;
                besthist_finalstate = bpidx;
            }
        }

        if (score > bestscore) {
            bestscore = score;
            besthist = bpidx;
        }

        --bpidx;
        if (bpidx < 0)
            break;

        hist_entry = fsg_history_entry_get(search->history, bpidx);
        frm = fsg_hist_entry_frame(hist_entry);
    }

    if (check_fsg_final_state) {
        if (besthist_finalstate > 0) {
            /*
             * Final state entry found; discard the plain best entry.
             * (Policy decision!  Is this the right thing to do??)
             */
            if (bestscore > bestscore_finalstate)
                E_INFO
                    ("Best score (%d) > best final state score (%d); but using latter\n",
                     bestscore, bestscore_finalstate);

            bestscore = bestscore_finalstate;
            besthist = besthist_finalstate;
        }
        else
            E_ERROR
                ("Final state not reached; backtracing from best scoring entry\n");
    }

    /* Backtrace through the search history, starting from besthist */
    head = NULL;
    for (bpidx = besthist; bpidx > 0;) {
        hist_entry = fsg_history_entry_get(search->history, bpidx);

        hyp = (search_hyp_t *) ckd_calloc(1, sizeof(search_hyp_t));

        if (fsg_history_entry_hyp_extract(search->history, bpidx, hyp) <=
            0)
            E_FATAL("fsg_history_entry_hyp_extract() returned <= 0\n");
        hyp->next = head;
        head = hyp;

        search->lscr += hyp->lscr;
        search->ascr += hyp->ascr;

        bpidx = fsg_hist_entry_pred(hist_entry);
    }
    search->hyp = head;

    /* For backward compatibility with existing API for obtaining results */
    fsg_search_set_result(search);
}


/*
 * Cleanup at the end of each utterance.
 */
void
fsg_search_utt_end(fsg_search_t * search)
{
    gnode_t *gn;
    fsg_pnode_t *pnode;
    int32 n_hist, nfr;
    char *result;
    FILE *latfp;
    char file[4096];

    /* Write history table if needed */
    if (cmd_ln_str_r(search->config, "-outlatdir")) {
        sprintf(file, "%s/%s.hist", cmd_ln_str_r(search->config, "-outlatdir"),
                uttproc_get_uttid());
        if ((latfp = fopen(file, "w")) == NULL)
            E_ERROR("fopen(%s,w) failed\n", file);
        else {
            fsg_history_dump(search->history, uttproc_get_uttid(), latfp);
            fclose(latfp);
        }
    }

    /*
     * Backtrace through Viterbi history to get the best recognition.
     * First check if the final state has been reached; otherwise just use
     * the best scoring state.
     */
    fsg_search_history_backtrace(search, cmd_ln_boolean_r(search->config, "-fsgbfs"));

    if (cmd_ln_boolean_r(search->config, "-backtrace"))
        fsg_search_hyp_dump(search, stdout);

    search_result(&nfr, &result);
    printf("FSGSRCH: %s (%s %d (A=%d L=%d))\n",
           result, uttproc_get_uttid(), search->ascr + search->lscr,
           search->ascr, search->lscr);
    fflush(stdout);

    n_hist = fsg_history_n_entries(search->history);
    fsg_history_reset(search->history);

    fsg_lextree_utt_end(search->lextree);

    /* Deactivate all nodes in the current and next-frame active lists */
    for (gn = search->pnode_active; gn; gn = gnode_next(gn)) {
        pnode = (fsg_pnode_t *) gnode_ptr(gn);
        fsg_psubtree_pnode_deactivate(pnode);
    }
    for (gn = search->pnode_active_next; gn; gn = gnode_next(gn)) {
        pnode = (fsg_pnode_t *) gnode_ptr(gn);
        fsg_psubtree_pnode_deactivate(pnode);
    }

    glist_free(search->pnode_active);
    search->pnode_active = NULL;
    glist_free(search->pnode_active_next);
    search->pnode_active_next = NULL;

    /* Do NOT reset search->frame, or search->hyp */

    search->state = FSG_SEARCH_IDLE;

    E_INFO
        ("Utt %s: %d frames, %d HMMs (%d/fr), %d senones (%d/fr), %d history entries (%d/fr)\n\n",
         uttproc_get_uttid(), search->frame, search->n_hmm_eval,
         (search->frame > 0) ? search->n_hmm_eval / search->frame : 0,
         search->n_sen_eval,
         (search->frame > 0) ? search->n_sen_eval / search->frame : 0,
         n_hist, (search->frame > 0) ? n_hist / search->frame : 0);

    /* Sanity check */
    if (search->n_hmm_eval >
        fsg_lextree_n_pnode(search->lextree) * search->frame) {
        E_ERROR
            ("SANITY CHECK #HMMEval(%d) > %d (#HMMs(%d)*#frames(%d)) FAILED\n",
             search->n_hmm_eval,
             fsg_lextree_n_pnode(search->lextree) * search->frame,
             fsg_lextree_n_pnode(search->lextree), search->frame);
    }
}


int32
fsg_search_get_start_state(fsg_search_t * search)
{
    if ((!search) || (!search->fsg))
        return -1;
    return word_fsg_start_state(search->fsg);
}


int32
fsg_search_get_final_state(fsg_search_t * search)
{
    if ((!search) || (!search->fsg))
        return -1;
    return word_fsg_final_state(search->fsg);
}


int32
fsg_search_set_start_state(fsg_search_t * search, int32 state)
{
    if (!search)
        return -1;

    if (search->state != FSG_SEARCH_IDLE) {
        E_ERROR("Attempt to switch FSG start state inside an utterance\n");
        return -1;
    }

    return (word_fsg_set_start_state(search->fsg, state));
}


int32
fsg_search_set_final_state(fsg_search_t * search, int32 state)
{
    if (!search)
        return -1;

    if (search->state != FSG_SEARCH_IDLE) {
        E_ERROR("Attempt to switch FSG start state inside an utterance\n");
        return -1;
    }

    return (word_fsg_set_final_state(search->fsg, state));
}
