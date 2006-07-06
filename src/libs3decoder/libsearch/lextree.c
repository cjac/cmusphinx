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
/*
 * lextree.c -- 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.11  2006/02/24  12:42:43  arthchan2003
 * Removed warnings in srch_flat_fwd.c and lextree.c
 * 
 * Revision 1.10  2006/02/23 15:08:24  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH:
 *
 * 1, Fixed memory leaks.
 * 2, Add logic for full triphone expansion.  At this point, the
 * propagation of scores in word end is still incorrect. So composite
 * triphone should still be used by default.
 * 3, Removed lextree_copies_hmm_propagate.
 *
 * Revision 1.9.4.10  2005/11/17 06:28:50  arthchan2003
 * Changed the code to used compressed triphones. Not yet correct at this point
 *
 * Revision 1.9.4.9  2005/10/17 04:53:44  arthchan2003
 * Shrub the trees so that the run-time memory could be controlled.
 *
 * Revision 1.9.4.8  2005/10/07 19:34:29  arthchan2003
 * In full cross-word triphones expansion, the previous implementation has several flaws, e.g, 1, it didn't consider the phone beam on cross word triphones. 2, Also, when the cross word triphone phone is used, children of the last phones will be regarded as cross word triphone. So, the last phone should not be evaluated at all.  Last implementation has not safe-guaded that. 3, The rescoring for language model is not done correctly.  What we still need to do: a, test the algorithm in more databases. b,  implement some speed up schemes.
 *
 * Revision 1.9.4.7  2005/09/25 19:27:04  arthchan2003
 * (Change for Comment) 1, Added lexical tree reporting. 2, Added a function for getting the number of links. 3, Added support for doing full triphone expansion. 4, In lextree_dump separate hmm evaluation and hmm dumping.
 *
 * Revision 1.9.4.6  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.9.4.5  2005/09/18 01:36:47  arthchan2003
 * Add implementation for lextree_report.
 *
 * Revision 1.9.4.4  2005/08/02 21:35:05  arthchan2003
 * Change sen to senscr.
 *
 * Revision 1.9.4.3  2005/07/17 05:44:32  arthchan2003
 * Added dag_write_header so that DAG header writer could be shared between 3.x and 3.0. However, because the backtrack pointer structure is different in 3.x and 3.0. The DAG writer still can't be shared yet.
 *
 * Revision 1.9.4.2  2005/07/07 02:34:36  arthchan2003
 * Remove empty lextree_tree_copies_hmm_propagate
 *
 * Revision 1.9.4.1  2005/06/27 05:37:05  arthchan2003
 * Incorporated several fixes to the search. 1, If a tree is empty, it will be removed and put back to the pool of tree, so number of trees will not be always increasing.  2, In the previous search, the answer is always "STOP P I T G S B U R G H </s>"and filler words never occurred in the search.  The reason is very simple, fillers was not properly propagated in the search at all <**exculamation**>  This version fixed this problem.  The current search will give <sil> P I T T S B U R G H </sil> </s> to me.  This I think it looks much better now.
 *
 * Revision 1.9  2005/06/21 23:32:58  arthchan2003
 * Log. Introduced lextree_init and filler_init to wrap up lextree_build
 * process. Split the hmm propagation to propagation for leaves and
 * non-leaves node.  This allows an easier time for turning off the
 * rescoring stage. However, the implementation is not clever enough. One
 * should split the array to leave array and non-leave array.
 *
 * Revision 1.11  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.10  2005/06/11 00:15:14  archan
 * Add an assert that could save lives
 *
 * Revision 1.9  2005/05/03 06:57:43  archan
 * Finally. The word switching tree code is completed. Of course, the reporting routine largely duplicate with time switching tree code.  Also, there has to be some bugs in the filler treatment.  But, hey! These stuffs we can work on it.
 *
 * Revision 1.8  2005/05/03 04:09:09  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.7  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.6  2005/04/25 19:22:47  archan
 * Refactor out the code of rescoring from lexical tree. Potentially we want to turn off the rescoring if we need.
 *
 * Revision 1.5  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified some functions to be able to deal with HMMs with any number
 * 		of states.  Modified lextree_hmm_eval() to dynamically call the
 * 		appropriate hmm_vit_eval routine.
 * 
 * 07-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lextree_node_t.ci and lextree_ci_active().
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */



#include "lextree.h"

/*
 * Lextree nodes, and the HMMs contained within, are cleared upon creation, and whenever
 * they become active during search.  Thus, when activated during search, they are always
 * "ready-to-use".  And, when cleaning up after an utterance, only the active nodes need
 * be cleaned up.
 */


static lextree_node_t *
lextree_node_alloc(int32 wid, int32 prob,
                   int32 comp, int32 ssid, int32 n_state, int32 ci,
                   int32 rc)
{
    lextree_node_t *ln;

    /*    ln = (lextree_node_t *) mymalloc (sizeof(lextree_node_t)); */

    ln = (lextree_node_t *) ckd_calloc(1, sizeof(lextree_node_t));
    ln->children = NULL;
    ln->wid = wid;
    ln->prob = prob;
    ln->ssid = ssid;
    ln->ci = (s3cipid_t) ci;
    ln->rc = rc;
    ln->composite = comp;
    ln->frame = -1;
    ln->hmm.state =
        (hmm_state_t *) ckd_calloc(n_state, sizeof(hmm_state_t));

    hmm_clear(&(ln->hmm), n_state);

    return ln;
}

static void
lextree_node_free(lextree_node_t * ln)
{
    if (ln) {
        if (ln->hmm.state)
            ckd_free(ln->hmm.state);
        ckd_free(ln);
    }
}

lextree_t *
lextree_init(kbcore_t * kbc, lm_t * lm, char *lmname, int32 istreeUgProb,
             int32 bReport, int32 type)
{
    s3cipid_t *lc;
    s3cipid_t ci;
    bitvec_t lc_active;
    s3wid_t w;
    int32 n, n_lc;
    int32 i, j;
    wordprob_t *wp;
    mdef_t *mdef;
    dict_t *dict;
    lextree_t *ltree;

    assert(kbc);
    assert(lm);
    assert(kbc->mdef);
    assert(kbc->dict);

    mdef = kbc->mdef;
    dict = kbc->dict;

    /* Build set of all possible left contexts */
    lc = (s3cipid_t *) ckd_calloc(mdef_n_ciphone(mdef) + 1,
                                  sizeof(s3cipid_t));
    lc_active = bitvec_alloc(mdef_n_ciphone(mdef));
    wp = (wordprob_t *) ckd_calloc(dict_size(dict), sizeof(wordprob_t));

    for (w = 0; w < dict_size(dict); w++) {
        ci = dict_pron(dict, w, dict_pronlen(dict, w) - 1);
        if (!mdef_is_fillerphone(mdef, (int) ci))
            bitvec_set(lc_active, ci);
    }
    ci = mdef_silphone(mdef);
    bitvec_set(lc_active, ci);

    for (ci = 0, n_lc = 0; ci < mdef_n_ciphone(mdef); ci++) {
        if (bitvec_is_set(lc_active, ci))
            lc[n_lc++] = ci;
    }
    lc[n_lc] = BAD_S3CIPID;

    if (bReport)
        E_INFO("Creating Unigram Table for lm (name: %s)\n", lmname);

    /* Build active word list */

    n = 0;
    /*try to be very careful again */
    for (j = 0; j < dict_size(dict); j++) {
        wp[j].wid = -1;
        wp[j].prob = -1;
    }
    n = lm_ug_wordprob(lm, dict, MAX_NEG_INT32, wp);

    if (bReport)
        E_INFO("Size of word table after unigram + words in class: %d.\n",
               n);

    if (n < 1)
        E_FATAL("%d active words in %s\n", n, lmname);

    n = wid_wordprob2alt(dict, wp, n);

    if (bReport)
        E_INFO("Size of word table after adding alternative prons: %d.\n",
               n);
    if (istreeUgProb == 0) {
        for (i = 0; i < n; i++) {
            wp[i].prob = -1;    /* Flatten all initial probabilities */
        }
    }
    ltree = lextree_build(kbc, wp, n, lc);
    lextree_type(ltree) = type;

    ckd_free((void *) wp);
    ckd_free((void *) lc);
    bitvec_free(lc_active);

    strcpy(ltree->prev_word, "");
    return ltree;
}

lextree_t *
fillertree_init(kbcore_t * kbc)
{
    int32 n;
    int32 i;
    dict_t *dict;
    wordprob_t *wp;
    lextree_t *ltree;

    assert(kbc);
    assert(kbc->dict);

    dict = kbc->dict;

    n = 0;

    wp = (wordprob_t *) ckd_calloc(dict_size(dict), sizeof(wordprob_t));

    for (i = dict_filler_start(dict); i <= dict_filler_end(dict); i++) {
        if (dict_filler_word(dict, i)) {
            wp[n].wid = i;
            wp[n].prob = fillpen(kbc->fillpen, i);
            n++;
        }
    }

    ltree = lextree_build(kbc, wp, n, NULL);
    lextree_type(ltree) = LEXTREE_TYPE_FILLER;

    ckd_free(wp);
    return ltree;
}

void
lextree_report(lextree_t * ltree)
{
    /*EMPTY, because it is quite hard to report a set of lexical trees at this point. */

    E_INFO_NOFN("lextree_t, report:\n");
    E_INFO_NOFN("Parameters of the lexical tree. \n");
    E_INFO_NOFN("Type of the tree %d (0:unigram, 1: 2g, 2: 3g etc.)\n",
                ltree->type);
    E_INFO_NOFN("Number of left contexts %d \n", ltree->n_lc);
    E_INFO_NOFN("Number of node %d \n", ltree->n_node);
    E_INFO_NOFN("Number of links in the tree %d\n",
                num_lextree_links(ltree));
    /*
       E_INFO_NOFN("Number of active node in this frame %d \n",ltree->n_active);
       E_INFO_NOFN("Number of active node in next frame %d \n",ltree->n_next_active);
       E_INFO_NOFN("Best HMM score of the current frame %d \n",ltree->best);
       E_INFO_NOFN("Best Word score of the current frame %d \n",ltree->wbest);
     */
    if (ltree->prev_word)
        E_INFO_NOFN("The previous word for this tree %s \n",
                    ltree->prev_word);

    E_INFO_NOFN("The size of a node of the lexical tree %d \n",
                sizeof(lextree_node_t));
    E_INFO_NOFN("The size of a gnode_t %d \n", sizeof(gnode_t));
    E_INFO_NOFN("\n");

}

int32
lextree_subtree_num_links(lextree_node_t * ln)
{
    gnode_t *gn;
    int32 numlink = 0;
    if (ln == NULL) {
        return 0;
    }
    else {
        for (gn = ln->children; gn; gn = gnode_next(gn)) {
            ln = (lextree_node_t *) gnode_ptr(gn);
            numlink += 1 + lextree_subtree_num_links(ln);
        }
        return numlink;
    }
}

int32
num_lextree_links(lextree_t * ltree)
{
    gnode_t *gn;
    int32 numlink = 0;
    lextree_node_t *ln;

    for (gn = ltree->root; gn; gn = gnode_next(gn)) {
        ln = (lextree_node_t *) gnode_ptr(gn);
        numlink += 1 + lextree_subtree_num_links(ln);
    }

    return numlink;

}


lextree_t *
lextree_build(kbcore_t * kbc, wordprob_t * wordprob, int32 n_word,
              s3cipid_t * lc)
{
    mdef_t *mdef;
    dict_t *dict;
    tmat_t *tmat;
    dict2pid_t *d2p;
    s3ssid_t *ldiph_lc;
    lextree_t *lextree;
    lextree_lcroot_t *lcroot;
    int32 n_lc, n_node, n_ci, n_sseq, pronlen, ssid, prob, ci, rc, wid, np,
        n_st;
    lextree_node_t *ln = 0, **parent, **ssid2ln;
    gnode_t *gn = 0;
    bitvec_t *ssid_lc;
    int32 i, j, k, p;

    mdef = kbc->mdef;
    dict = kbc->dict;
    tmat = kbc->tmat;
    d2p = kbc->dict2pid;
    n_ci = mdef_n_ciphone(mdef);
    n_sseq = mdef_n_sseq(mdef);
    n_st = mdef_n_emit_state(mdef);

    lextree = (lextree_t *) ckd_calloc(1, sizeof(lextree_t));
    lextree->root = NULL;

    /* Table mapping from root level ssid to lexnode (temporary) */
    ssid2ln =
        (lextree_node_t **) ckd_calloc(n_sseq, sizeof(lextree_node_t *));

    /* ssid_lc[ssid] = bitvec indicating which lc's this (root) ssid is entered under */
    ssid_lc = (bitvec_t *) ckd_calloc(n_sseq, sizeof(bitvec_t));
    for (i = 0; i < n_sseq; i++)
        ssid_lc[i] = bitvec_alloc(n_ci);

    n_node = 0;

    /* Create top-level structures pointing to (shared) lextrees for each left context */
    n_lc = 0;
    lcroot = NULL;
    if (!lc) {
        lextree->n_lc = 0;
        lextree->lcroot = NULL;

        parent =
            (lextree_node_t **) ckd_calloc(1, sizeof(lextree_node_t *));
    }
    else {
        for (n_lc = 0; IS_S3CIPID(lc[n_lc]); n_lc++);
        assert(n_lc > 0);

        lextree->n_lc = n_lc;
        lcroot =
            (lextree_lcroot_t *) ckd_calloc(n_lc,
                                            sizeof(lextree_lcroot_t));
        lextree->lcroot = lcroot;

        for (i = 0; i < n_lc; i++) {
            lcroot[i].lc = lc[i];
            lcroot[i].root = NULL;
        }

        parent =
            (lextree_node_t **) ckd_calloc(n_lc, sizeof(lextree_node_t *));
    }

    /*
     * Build up lextree for each word.  For each word:
     *   for each phone position {
     *     see if node already exists in lextree built so far;
     *     if so, share it, otherwise create one (this becomes the parent whose subtree will be
     *       searched for the next phone position);
     *   }
     * 
     * parent[]: A temporary structure during the addition of one word W to the lextree.
     * Normally, when a phone position p of W is added to the lextree, it has one parent node.
     * But when the parent is at the root level, there can actually be several parents, for the
     * different left contexts.  (Hence, parent[] instead of a scalar parent.  Beyond the root
     * level, only parent[0] is useful.)  Furthermore, root parents may share nodes (with same
     * ssid).  Maintain only the unique set.
     * 
     * Other points worth mentioning:
     * - Leaf nodes are not shared among words
     * - (LM) prob at any node is the max of the probs of words reachable from that node
     */
    for (i = 0; i < n_word; i++) {
        wid = wordprob[i].wid;
        prob = wordprob[i].prob;

        pronlen = dict_pronlen(dict, wid);

        if (pronlen == 1) {
            /* Single phone word; node(s) not shared with any other word */
            ci = dict_pron(dict, wid, 0);
            if (!lc) {

                if (d2p->is_composite)
                    ssid = d2p->internal[wid][0];
                else            /* MAJOR HACK! use the phone filler tree */
                    ssid = mdef_pid2ssid(mdef, ci);

                ln = lextree_node_alloc(wid, prob, d2p->is_composite, ssid,
                                        n_st, dict_pron(dict, wid, 0),
                                        BAD_S3CIPID);

                ln->hmm.tp = tmat->tp[mdef_pid2tmatid(mdef, ci)];       /* Assuming CI tmat!! */

                lextree->root = glist_add_ptr(lextree->root, (void *) ln);
                n_node++;
            }
            else {
                np = 0;
                for (j = 0; j < n_lc; j++) {

                    if (d2p->is_composite) {    /* In composite triphone */
                        ssid = d2p->single_lc[ci][(int) lc[j]]; /* This is a composite triphone */
                    }
                    else {      /* Use approximation to get the SSID */
                        ssid = d2p->lrdiph_rc[ci][(int) lc[j]][mdef_ciphone_id(mdef, "sil")];   /* HACK, always assume right context is silence */
                    }

                    /* Check if this ssid already allocated for another lc */
                    for (k = 0; (k < np) && (parent[k]->ssid != ssid);
                         k++);
                    if (k >= np) {      /* Not found; allocate new node */

                        ln = lextree_node_alloc(wid, prob, d2p->is_composite, ssid, n_st, ci, BAD_S3CIPID);  /**< ARCHAN: This is a composite triphone */

                        ln->hmm.tp = tmat->tp[mdef_pid2tmatid(mdef, ci)];

                        lextree->root =
                            glist_add_ptr(lextree->root, (void *) ln);
                        n_node++;

                        lcroot[j].root =
                            glist_add_ptr(lcroot[j].root, (void *) ln);
                        parent[np++] = ln;
                    }
                    else {      /* Already exists; link to lcroot[j] */
                        lcroot[j].root =
                            glist_add_ptr(lcroot[j].root,
                                          (void *) parent[k]);
                    }
                }
            }
        }
        else {
            assert(pronlen > 1);

            /* Multi-phone word; allocate root node(s) first, if not already present */
            if (!lc) {
                ssid = d2p->internal[wid][0];
                ci = dict_pron(dict, wid, 0);

                /* Check if this ssid already allocated for another word */
                for (gn = lextree->root; gn; gn = gnode_next(gn)) {
                    ln = (lextree_node_t *) gnode_ptr(gn);
                    if ((ln->ssid == ssid) && ln->composite
                        && NOT_S3WID(ln->wid))
                        break;
                }
                if (!gn) {
                    ln = lextree_node_alloc(BAD_S3WID, prob,
                                            d2p->is_composite, ssid, n_st,
                                            ci, BAD_S3CIPID);
                    ln->hmm.tp = tmat->tp[mdef_pid2tmatid(mdef, ci)];

                    lextree->root =
                        glist_add_ptr(lextree->root, (void *) ln);
                    n_node++;
                }
                else {
                    if (ln->prob < prob)
                        ln->prob = prob;
                }
                parent[0] = ln;
                np = 1;
            }
            else {
                ci = dict_pron(dict, wid, 0);
                rc = dict_pron(dict, wid, 1);
                ldiph_lc = d2p->ldiph_lc[ci][rc];

                np = 0;
                for (j = 0; j < n_lc; j++) {
                    ssid = ldiph_lc[(int) lc[j]];

                    /* Check if ssid already allocated */
                    ln = ssid2ln[ssid];
                    if (!ln) {
                        ln = lextree_node_alloc(BAD_S3WID, prob,
                                                NOT_COMPOSITE, ssid, n_st,
                                                ci, BAD_S3CIPID);
                        ln->hmm.tp = tmat->tp[mdef_pid2tmatid(mdef, ci)];
                        lextree->root =
                            glist_add_ptr(lextree->root, (void *) ln);
                        n_node++;

                        ssid2ln[ssid] = ln;
                    }
                    else if (ln->prob < prob)
                        ln->prob = prob;

                    /* Check if lexnode already entered under lcroot[lc] */
                    if (bitvec_is_clear(ssid_lc[ssid], lc[j])) {
                        lcroot[j].root =
                            glist_add_ptr(lcroot[j].root, (void *) ln);
                        bitvec_set(ssid_lc[ssid], lc[j]);
                    }

                    /* Add to parent_list if not already there */
                    for (k = 0; (k < np) && (parent[k]->ssid != ssid);
                         k++);
                    if (k >= np)
                        parent[np++] = ln;
                }
            }

            /* Rest of the pronunciation except the final one */
            for (p = 1; p < pronlen - 1; p++) {
                ssid = d2p->internal[wid][p];
                ci = dict_pron(dict, wid, p);

                /* Check for ssid under each parent (#parents(np) > 1 only when p==1) */
                for (j = 0; j < np; j++) {
                    for (gn = parent[j]->children; gn; gn = gnode_next(gn)) {
                        ln = (lextree_node_t *) gnode_ptr(gn);

                        if ((ln->ssid == ssid) && (!ln->composite)) {
                            assert(NOT_S3WID(ln->wid));
                            break;
                        }
                    }
                    if (gn)
                        break;
                }

                if (!gn) {      /* Not found under any parent; allocate new node */
                    ln = lextree_node_alloc(BAD_S3WID, prob, NOT_COMPOSITE,
                                            ssid, n_st, ci, BAD_S3CIPID);
                    ln->hmm.tp = tmat->tp[mdef_pid2tmatid(mdef, ci)];

                    for (j = 0; j < np; j++)
                        parent[j]->children =
                            glist_add_ptr(parent[j]->children,
                                          (void *) ln);
                    n_node++;
                }
                else {          /* Already exists under parent[j] */
                    if (ln->prob < prob)
                        ln->prob = prob;

                    k = j;

                    /* Child was not found under parent[0..k-1]; add */
                    for (j = 0; j < k; j++)
                        parent[j]->children =
                            glist_add_ptr(parent[j]->children,
                                          (void *) ln);

                    /* Parents beyond k have not been checked; add if not present */
                    for (j = k + 1; j < np; j++) {
                        if (!glist_chkdup_ptr
                            (parent[j]->children, (void *) ln))
                            parent[j]->children =
                                glist_add_ptr(parent[j]->children,
                                              (void *) ln);
                    }
                }

                parent[0] = ln;
                np = 1;
            }

            /* Final (leaf) node, no sharing */
            ssid = d2p->internal[wid][p];
            ci = dict_pron(dict, wid, p);
            ln = lextree_node_alloc(wid, prob, d2p->is_composite, ssid,
                                    n_st, ci, BAD_S3CIPID);
            ln->hmm.tp = tmat->tp[mdef_pid2tmatid(mdef, ci)];

            for (j = 0; j < np; j++)
                parent[j]->children =
                    glist_add_ptr(parent[j]->children, (void *) ln);
            n_node++;
        }
    }

    lextree->n_node = n_node;
    lextree->n_alloc_node = n_node;     /* dynamically allocated. */

    /* Assuming each time, when re-allocation of active, next_active
       need to be done only one eighth of all possible cross word
       expansion is needed to be allocated. That would mean in the
       worst case, for a time moment, that can be a possibility for 8
       allocations. 
     */
    lextree->n_alloc_blk_sz = ((n_word * mdef_n_ciphone(mdef)) >> 3);

    lextree->active =
        (lextree_node_t **) ckd_calloc(n_node +
                                       n_word * mdef_n_ciphone(mdef),
                                       sizeof(lextree_node_t *));
    lextree->next_active =
        (lextree_node_t **) ckd_calloc(n_node +
                                       n_word * mdef_n_ciphone(mdef),
                                       sizeof(lextree_node_t *));

    /*    lextree->active = (lextree_node_t **) ckd_calloc (n_node, sizeof(lextree_node_t *));
       lextree->next_active = (lextree_node_t **) ckd_calloc (n_node, sizeof(lextree_node_t *)); */


    lextree->n_active = 0;
    lextree->n_next_active = 0;

    ckd_free((void *) ssid2ln);
    for (i = 0; i < n_sseq; i++)
        bitvec_free(ssid_lc[i]);
    ckd_free((void *) ssid_lc);
    ckd_free(parent);

    return lextree;
}


/* Also work for the case where triphone is allocated */
static int32
lextree_subtree_free(lextree_node_t * ln, int32 level)
{
    gnode_t *gn;
    lextree_node_t *ln2;
    int32 k;

    k = 0;

    /* Free subtrees below this node */
    for (gn = ln->children; gn; gn = gnode_next(gn)) {
        ln2 = (lextree_node_t *) gnode_ptr(gn);
        k += lextree_subtree_free(ln2, level + 1);
    }
    glist_free(ln->children);
    ln->children = NULL;

    /* Free this node, but for level-1 nodes only if reference count drops to 0 */
    if ((level != 1) || (--ln->ssid == 0)) {
        /*        myfree ((void *) ln, sizeof(lextree_node_t)); */

        lextree_node_free((void *) ln);
        k++;
    }

    return k;
}

static int32
lextree_shrub_subtree_cw_leaves(lextree_node_t * ln, int32 level)
{
    gnode_t *gn;
    lextree_node_t *ln2;
    int32 k;
    k = 0;

    /* If it is a leave and it is a WID and it has not SSID, then, it is
       a mother of the list of cross-word triphones.
     */

    if (IS_S3WID(ln->wid) && !IS_S3SSID(ln->ssid)) {

        /* If it is a node with WID and have bad senone sequence */

        if (ln->children != NULL) {

#if 0
            E_INFO
                ("Free Cross word is carried out  for wid %d, ln->children %d\n",
                 ln->wid, ln->children);
#endif

            for (gn = ln->children; gn; gn = gnode_next(gn)) {
                ln2 = (lextree_node_t *) gnode_ptr(gn);
                /*      E_INFO("I am freeing something! WID, %d, rc %d\n",ln->wid,ln2->rc); */

                /* Free this node, but for level-1 nodes only if reference count drops to 0 */

                lextree_node_free(ln2);
                k++;
            }
            glist_free(ln->children);
            ln->children = NULL;
        }

    }
    else {
        /* Free subtrees below this node */
        for (gn = ln->children; gn; gn = gnode_next(gn)) {
            ln2 = (lextree_node_t *) gnode_ptr(gn);
            k += lextree_shrub_subtree_cw_leaves(ln2, level + 1);
        }
    }


    return k;

}

void
lextree_shrub_cw_leaves(lextree_t * lextree)
{
    gnode_t *gn, *cwgn;
    glist_t root;
    lextree_node_t *ln;
    lextree_node_t *cwln;
    int32 k, i;

    /* Free lextree */
    k = 0;

    if (lextree->n_lc > 0) {
        for (i = 0; i < lextree->n_lc; i++) {
            root = lextree->lcroot[i].root;

            if (root != NULL) {
                for (gn = root; gn; gn = gnode_next(gn)) {

                    ln = (lextree_node_t *) gnode_ptr(gn);

                    if (IS_S3WID(ln->wid) && ln->children != NULL) {

#if 0
                        E_INFO
                            ("Tree %d, lc %d, Free Cross word is carried out  for wid %d, ln->children %d\n",
                             lextree, lextree->lcroot[i].lc, ln->wid,
                             ln->children);
#endif
                        for (cwgn = ln->children; cwgn;
                             cwgn = gnode_next(cwgn)) {
                            cwln = (lextree_node_t *) gnode_ptr(cwgn);
                            lextree_node_free(cwln);
                        }
                        glist_free(ln->children);
                        ln->children = NULL;
                    }
                }
            }
        }
    }

    for (gn = lextree->root; gn; gn = gnode_next(gn)) {
        ln = (lextree_node_t *) gnode_ptr(gn);
        k += lextree_shrub_subtree_cw_leaves(ln, 0);
    }
    lextree_n_node(lextree) -= k;
}

/*
 * This is a bit tricky because of the replication of root nodes for different left-contexts.
 * A node just below the root can have more than one parent.  Use reference counts to know how
 * many parents refer to such a node.  Use the lextree_node_t.ssid field for such counts.
 */
void
lextree_free(lextree_t * lextree)
{
    gnode_t *gn, *gn2;
    lextree_node_t *ln, *ln2;
    int32 i, k;

    if (lextree->n_lc > 0) {
        for (i = 0; i < lextree->n_lc; i++) {
            glist_free(lextree->lcroot[i].root);
            lextree->lcroot[i].root = NULL;
        }

        ckd_free(lextree->lcroot);
    }

    /* Build reference counts for level-1 nodes (nodes just below root) */
    for (gn = lextree->root; gn; gn = gnode_next(gn)) {
        ln = (lextree_node_t *) gnode_ptr(gn);
        for (gn2 = ln->children; gn2; gn2 = gnode_next(gn2)) {
            ln2 = (lextree_node_t *) gnode_ptr(gn2);
            if (ln2->composite >= 0) {  /* First visit to this node */
                ln2->composite = -1;
                ln2->ssid = 1;  /* Ref count = 1 */
            }
            else
                ln2->ssid++;    /* Increment ref count */
        }
    }

    /* Free lextree */
    k = 0;
    for (gn = lextree->root; gn; gn = gnode_next(gn)) {
        ln = (lextree_node_t *) gnode_ptr(gn);
        k += lextree_subtree_free(ln, 0);
    }
    glist_free(lextree->root);

    ckd_free((void *) lextree->active);
    ckd_free((void *) lextree->next_active);

    /*    E_INFO("%d %d\n",k,lextree->n_node); */
    if (k != lextree->n_node)
        E_ERROR("#Nodes allocated(%d) != #nodes freed(%d)\n",
                lextree->n_node, k);

    ckd_free(lextree);
}

/* Not full triphone expansion aware */
void
lextree_ci_active(lextree_t * lextree, bitvec_t ci_active)
{
    lextree_node_t **list, *ln;
    int32 i;

    list = lextree->active;

    for (i = 0; i < lextree->n_active; i++) {
        ln = list[i];
        bitvec_set(ci_active, ln->ci);
    }
}


void
lextree_ssid_active(lextree_t * lextree, int32 * ssid, int32 * comssid)
{
    lextree_node_t **list, *ln;
    int32 i;

    list = lextree->active;

    for (i = 0; i < lextree->n_active; i++) {
        ln = list[i];


        /*      if(IS_S3WID(ln->wid)){
           E_INFO("Is WID %d,  ln->ssid %d, Do I have children %d?\n",ln ->wid, ln->ssid, (ln->children!=NULL));

           assert(ln->ssid!=BAD_S3SSID);
           } */

        if (ln->composite)
            comssid[ln->ssid] = 1;
        else
            ssid[ln->ssid] = 1;
    }
}


void
lextree_utt_end(lextree_t * l, kbcore_t * kbc)
{
    mdef_t *mdef;
    lextree_node_t *ln;
    int32 i;

    mdef = kbcore_mdef(kbc);

    for (i = 0; i < l->n_active; i++) { /* The inactive ones should already be reset */
        ln = l->active[i];

        ln->frame = -1;
        hmm_clear(&(ln->hmm), mdef_n_emit_state(mdef));
    }

    l->n_active = 0;
    l->n_next_active = 0;

    strcpy(l->prev_word, "");

    /* If the tree has crossword triphone, shrub them off at the end
       of the utterance */


    if (!dict2pid_is_composite(kbc->dict2pid)) {
        lextree_shrub_cw_leaves(l);
    }


}


static void
lextree_node_print(lextree_node_t * ln, dict_t * dict, FILE * fp)
{
    fprintf(fp, "wid(%d)pr(%d)com(%d)ss(%d)rc(%d)", ln->wid, ln->prob,
            ln->composite, ln->ssid, ln->rc);
    if (IS_S3WID(ln->wid))
        fprintf(fp, "%s", dict_wordstr(dict, ln->wid));
    fprintf(fp, "\n");
}



static void
lextree_subtree_print(lextree_node_t * ln, int32 level, dict_t * dict,
                      FILE * fp)
{
    int32 i;
    gnode_t *gn;

    for (i = 0; i < level; i++)
        fprintf(fp, "    ");
    lextree_node_print(ln, dict, fp);

    for (gn = ln->children; gn; gn = gnode_next(gn)) {
        ln = (lextree_node_t *) gnode_ptr(gn);
        lextree_subtree_print(ln, level + 1, dict, fp);
    }
}

static void
lextree_subtree_print_dot(lextree_node_t * ln, int32 level, dict_t * dict,
                          mdef_t * mdef, FILE * fp)
{
    gnode_t *gn;


    if (IS_S3WID(ln->wid)) {
        fprintf(fp, "\"%s\";\n", dict_wordstr(dict, ln->wid));
    }
    else {
        for (gn = ln->children; gn; gn = gnode_next(gn)) {
            ln = (lextree_node_t *) gnode_ptr(gn);
            fprintf(fp, " \"%s\" -> ", mdef_ciphone_str(mdef, ln->ci));
            lextree_subtree_print_dot(ln, level + 1, dict, mdef, fp);
        }
    }
}

#define GRAPH_RAVIFMT 1
#define GRAPH_DOTFMT 2

void
lextree_dump(lextree_t * lextree, dict_t * dict, mdef_t * mdef, FILE * fp,
             int32 fmt)
{
    gnode_t *gn;
    lextree_node_t *ln;
    int32 i;

    if (fmt > 2) {
        fmt = GRAPH_RAVIFMT;
    }
    if (fmt == GRAPH_RAVIFMT) { /*Ravi's format */
        for (gn = lextree->root; gn; gn = gnode_next(gn)) {
            ln = (lextree_node_t *) gnode_ptr(gn);
            lextree_subtree_print(ln, 0, dict, fp);
        }

        if (lextree->n_lc > 0) {
            for (i = 0; i < lextree->n_lc; i++) {
                fprintf(fp, "lcroot %d\n", lextree->lcroot[i].lc);
                for (gn = lextree->lcroot[i].root; gn; gn = gnode_next(gn)) {
                    ln = (lextree_node_t *) gnode_ptr(gn);
                    lextree_node_print(ln, dict, fp);
                }
            }
        }
    }
    else if (fmt == GRAPH_DOTFMT) {
        fprintf(fp, "digraph G {\n");
        fprintf(fp, "rankdir=LR \n");
        for (gn = lextree->root; gn; gn = gnode_next(gn)) {
            ln = (lextree_node_t *) gnode_ptr(gn);

            fprintf(fp, " \"%s\" -> ", mdef_ciphone_str(mdef, ln->ci));

            lextree_subtree_print_dot(ln, 0, dict, mdef, fp);
        }
        fprintf(fp, "}\n");
    }
    fflush(fp);
}

/*
  Hmm. For some reason, it doesn't really work yet. 
 */
static void
lextree_realloc_active_list(lextree_t * lt, int32 num_active)
{
    if (num_active >= lt->n_alloc_node && lt->type != LEXTREE_TYPE_FILLER) {
        E_INFO("num_active %d, n_alloc_node %d, n_node %d\n", num_active,
               lt->n_alloc_node, lt->n_node);
        lextree_n_alloc_node(lt) = lextree_n_node(lt);
        lt->active =
            (lextree_node_t **) ckd_realloc(lt->active,
                                            lextree_n_alloc_node(lt) *
                                            sizeof(lextree_node_t *));
        if (lt->active == NULL) {
            E_INFO("help.");
        }
        lt->next_active =
            (lextree_node_t **) ckd_realloc(lt->next_active,
                                            lextree_n_alloc_node(lt) *
                                            sizeof(lextree_node_t *));
        if (lt->next_active == NULL) {
            E_INFO("help.");
        }

        E_INFO("Reallocating more memory, now has node %d\n",
               lextree_n_alloc_node(lt));
    }

}


void
lextree_enter(lextree_t * lextree, s3cipid_t lc, int32 cf,
              int32 inscore, int32 inhist, int32 thresh, kbcore_t * kbc)
{
    glist_t root;
    gnode_t *gn, *cwgn;
    lextree_node_t *ln, *cwln;
    int32 nf, scr;
    int32 i, n;
    hmm_t *hmm, *cwhmm;
    int32 rc;
    int32 n_ci, n_st, n_rc;
    /*    int32 tmp_lc; */
    tmat_t *tmat;
    s3ssid_t *rmap;

    nf = cf + 1;

    n_ci = mdef_n_ciphone(kbc->mdef);
    n_st = mdef_n_emit_state(kbc->mdef);
    tmat = kbc->tmat;
    rc = 0;

    assert(lextree);
    /* Locate root nodes list */
    if (lextree->n_lc == 0) {
        assert(NOT_S3CIPID(lc));
        root = lextree->root;
    }
    else {
        for (i = 0; (i < lextree->n_lc) && (lextree->lcroot[i].lc != lc);
             i++);
        /*      E_INFO("i=%d, lextree->n_lc %d\n",i,lextree->n_lc); */
        assert(i < lextree->n_lc);

        root = lextree->lcroot[i].root;
    }

    /* Enter root nodes */
    n = lextree->n_next_active;


    for (gn = root; gn; gn = gnode_next(gn)) {
        ln = (lextree_node_t *) gnode_ptr(gn);

        hmm = &(ln->hmm);

        if (NOT_S3WID(ln->wid) ||       /* If the first node we see it a non leave */
            (IS_S3WID(ln->wid) && dict2pid_is_composite(kbc->dict2pid)) /* Or it is a leave but we are using composite triphone */
            ) {
            scr = inscore + ln->prob;
            if ((scr >= thresh) && (hmm->in.score < scr)) {
                hmm->in.score = scr;
                hmm->in.history = inhist;

                if (ln->frame != nf) {
                    ln->frame = nf;
                    lextree->next_active[n++] = ln;
                }
            }                   /* else it is activated separately */
        }
        else {                  /* It is a leave node, so we consider all possible contexts */

            /* FIX ME! To allow extra flexibility, one should allow 
               optionally composite single phone */

            assert(IS_S3WID(ln->wid));
            if (ln->children == NULL) {
#if 0
                E_INFO
                    ("Tree %d, lc %d, Cross word expansion is carried out at cf %d for wid %d, wstr %s, ln->children %d\n",
                     lextree, lc, cf, ln->wid, dict_wordstr(kbc->dict,
                                                            ln->wid),
                     ln->children);
#endif

                n_ci = mdef_n_ciphone(kbc->mdef);
                /* HACK, assuming the left context is sil */

                /*      if(lc==-1)
                   tmp_lc=0;
                   else
                   tmp_lc=lc; */

                rmap = kbc->dict2pid->lrssid[ln->ci][0].ssid;
                n_rc = get_rc_nssid(kbc->dict2pid, ln->wid, kbc->dict);

                /*      n_rc = kbc->dict2pid->lrssid[ln->ci][tmp_lc].n_ssid; */

                /*      E_INFO("I am here\n");
                   E_INFO("lrssid n_rc %d, get_nssid %d\n", n_rc, get_rc_nssid(kbc->dict2pid,ln->wid,kbc->dict));
                 */



                if (!dict_filler_word(kbc->dict, ln->wid)) {

                    for (rc = 0; rc < n_rc; rc++) {
                        cwln =
                            lextree_node_alloc(ln->wid, ln->prob,
                                               NOT_COMPOSITE, rmap[rc],
                                               n_st, ln->ci, rc);
                        cwln->hmm.tp =
                            tmat->tp[mdef_pid2tmatid(kbc->mdef, ln->ci)];
                        ln->children =
                            glist_add_ptr(ln->children, (void *) cwln);
                    }

                }
                else {
                    /* Assume there is no context when filler. Still expands to keep the program
                       consistency. */

                    cwln =
                        lextree_node_alloc(ln->wid, ln->prob,
                                           NOT_COMPOSITE, rmap[0], n_st,
                                           ln->ci, 0);
                    lextree_n_node(lextree) += 1;

                    cwln->hmm.tp =
                        tmat->tp[mdef_pid2tmatid(kbc->mdef, ln->ci)];
                    ln->children =
                        glist_add_ptr(ln->children, (void *) cwln);
                }

            }

            /* This part should be moved to a function */
            for (cwgn = ln->children; cwgn; cwgn = gnode_next(cwgn)) {
                cwln = (lextree_node_t *) gnode_ptr(cwgn);
                cwhmm = &(cwln->hmm);

                scr = inscore + cwln->prob;
                if ((scr >= thresh) && (cwhmm->in.score < scr)) {
                    cwhmm->in.score = scr;
                    cwhmm->in.history = inhist;
                    if (cwln->frame != nf) {
                        cwln->frame = nf;
                        lextree->next_active[n++] = cwln;
                    }
                }
            }
        }

    }
    lextree->n_next_active = n;
}


void
lextree_active_swap(lextree_t * lextree)
{
    lextree_node_t **t;

    t = lextree->active;
    lextree->active = lextree->next_active;
    lextree->next_active = t;
    lextree->n_active = lextree->n_next_active;
    lextree->n_next_active = 0;
}


int32
lextree_hmm_eval(lextree_t * lextree, kbcore_t * kbc, ascr_t * ascr,
                 int32 frm, FILE * fp)
{
    int32 best, wbest, n_st;
    int32 i, k;
    lextree_node_t **list, *ln;
    mdef_t *mdef;
    dict2pid_t *d2p;

    mdef = kbc->mdef;
    d2p = kbc->dict2pid;
    n_st = mdef_n_emit_state(mdef);

    list = lextree->active;
    best = MAX_NEG_INT32;
    wbest = MAX_NEG_INT32;


    for (i = 0; i < lextree->n_active; i++) {
        ln = list[i];

        if (IS_S3WID(ln->wid)) {
            /*      E_INFO("Frm %d, Is WID %d, wdstr %s, ln->ssid %d\n",frm, ln->wid,dict_wordstr(kbc->dict,ln->wid), ln->ssid); */
        }

        assert(ln->frame == frm);
        assert(ln->ssid >= 0);

        if (fp) {
            /*      lextree_node_print (ln, kbc->dict, fp); */
            if (!ln->composite)
                hmm_dump(&(ln->hmm), n_st, mdef->sseq[ln->ssid],
                         ascr->senscr, fp);
            else
                hmm_dump(&(ln->hmm), n_st, d2p->comsseq[ln->ssid],
                         ascr->comsen, fp);
        }

        if (!ln->composite) {
            k = hmm_vit_eval(&(ln->hmm), n_st,
                             mdef->sseq[ln->ssid], ascr->senscr);

        }
        else {
            k = hmm_vit_eval(&(ln->hmm), n_st,
                             d2p->comsseq[ln->ssid], ascr->comsen);
        }

        if (best < k)
            best = k;

        if (IS_S3WID(ln->wid)) {
            if (wbest < k)
                wbest = k;
        }

    }

    lextree->best = best;
    lextree->wbest = wbest;

    if (fp) {
        fprintf(fp, "Fr %d  #active %d  best %d  wbest %d\n",
                frm, lextree->n_active, best, wbest);
        fflush(fp);
    }

    return best;

#if 0                           /* The old logic, duplicated with hmm_dump_vit_eval */
    if (fp) {

    }
    else {
        if (n_st == 3) {
            for (i = 0; i < lextree->n_active; i++) {
                ln = list[i];
                assert(ln->frame == frm);

                if (!ln->composite) {

                    k = hmm_vit_eval_3st(&(ln->hmm), mdef->sseq[ln->ssid],
                                         ascr->senscr);
                }
                else {
                    k = hmm_vit_eval_3st(&(ln->hmm),
                                         d2p->comsseq[ln->ssid],
                                         ascr->comsen);

                }
                if (best < k)
                    best = k;

                if (IS_S3WID(ln->wid)) {
                    if (wbest < k)
                        wbest = k;
                }
            }
        }
        else if (n_st == 5) {
            for (i = 0; i < lextree->n_active; i++) {
                ln = list[i];
                assert(ln->frame == frm);

                if (!ln->composite)
                    k = hmm_vit_eval_5st(&(ln->hmm), mdef->sseq[ln->ssid],
                                         ascr->senscr);
                else
                    k = hmm_vit_eval_5st(&(ln->hmm),
                                         d2p->comsseq[ln->ssid],
                                         ascr->comsen);

                if (best < k)
                    best = k;

                if (IS_S3WID(ln->wid)) {
                    if (wbest < k)
                        wbest = k;
                }
            }
        }
        else
            E_FATAL("#State= %d unsupported\n", n_st);
    }
#endif

}


void
lextree_hmm_histbin(lextree_t * lextree, int32 bestscr, int32 * bin,
                    int32 nbin, int32 bw)
{
    lextree_node_t **list, *ln;
    hmm_t *hmm;
    int32 i, k;
    glist_t *binln;
    gnode_t *gn;

    binln = (glist_t *) ckd_calloc(nbin, sizeof(glist_t));

    list = lextree->active;

    for (i = 0; i < lextree->n_active; i++) {
        ln = list[i];

        if (IS_S3WID(ln->wid)) {
            assert(ln->ssid != BAD_S3SSID);
        }

        /*      if(IS_S3WID(ln->wid)){
           E_INFO("Is WID\n");
           } */

        hmm = &(ln->hmm);

        k = (bestscr - hmm->bestscore) / bw;
        if (k >= nbin)
            k = nbin - 1;
        assert(k >= 0);

        bin[k]++;
        binln[k] = glist_add_ptr(binln[k], (void *) ln);
    }

    /* Reorder the active lexnodes in APPROXIMATELY descending scores */
    k = 0;
    for (i = 0; i < nbin; i++) {
        for (gn = binln[i]; gn; gn = gnode_next(gn)) {
            ln = (lextree_node_t *) gnode_ptr(gn);
            list[k++] = ln;
        }
        glist_free(binln[i]);
    }
    assert(k == lextree->n_active);

    ckd_free((void *) binln);
}





int32
lextree_hmm_propagate_non_leaves(lextree_t * lextree, kbcore_t * kbc,
                                 int32 cf, int32 th, int32 pth, int32 wth,
                                 pl_t * pl)
{
    mdef_t *mdef;
    dict2pid_t *d2p;
    dict_t *dict;
    int32 nf, newscore, newHeurScore;
    lextree_node_t **list, *ln, *ln2;
    lextree_node_t *cwln;
    hmm_t *hmm, *hmm2, *cwhmm;
    tmat_t *tmat;
    gnode_t *gn, *gn2;
    int32 i, n;
    int32 hth;
    int32 *phn_heur_list;
    int32 heur_beam;
    int32 heur_type;
    int32 rc;
    int32 n_ci, n_st, n_rc;
    s3ssid_t *rmap;

    /* Code for heursitic score */
    kbc->maxNewHeurScore = MAX_NEG_INT32;
    kbc->lastfrm = -1;
    hth = 0;
    mdef = kbcore_mdef(kbc);
    n_ci = mdef_n_ciphone(mdef);
    n_st = mdef_n_emit_state(mdef);
    d2p = kbc->dict2pid;
    dict = kbc->dict;
    tmat = kbc->tmat;

    phn_heur_list = pl->phn_heur_list;
    heur_beam = pl->pl_beam;
    heur_type = pl->pheurtype;

    nf = cf + 1;

    list = lextree->active;

    n = lextree->n_next_active;
    /*    E_INFO("The size of n: %d\n",n); */
    assert(n == 0);

    /*    E_INFO("No. of active node within the lexical tree: %d\n",lextree->n_active); */

    for (i = 0; i < lextree->n_active; i++) {
        /*      E_INFO("%d, %d\n", i,  lextree->n_alloc_node); */
        ln = list[i];
        hmm = &(ln->hmm);

        if (IS_S3WID(ln->wid)) {
            /*      E_INFO("Is WID %d, ln->rc %d, ln->ssid %d\n",ln->wid, ln->rc, ln->ssid); */
            assert(ln->ssid != BAD_S3SSID);
        }


        /* This if will activate nodes */
        if (ln->frame < nf) {
            if (hmm->bestscore >= th) { /* Active in next frm */
                ln->frame = nf;
                lextree->next_active[n++] = ln;
            }
            else {              /* Deactivate */
                ln->frame = -1;
                hmm_clear(hmm, mdef_n_emit_state(mdef));
            }
        }

        if (NOT_S3WID(ln->wid)) {       /* Not a leaf node */
#if 0
            if (((cf % 3) == 0) || (hmm->out.score < pth))
                continue;       /* HMM exit score not good enough */
#else
            if (hmm->out.score < pth)
                continue;       /* HMM exit score not good enough */
#endif
            if (heur_type > 0) {        /* In full expansion, this part is not
                                           really correct */
                if (cf != kbc->lastfrm) {
                    kbc->lastfrm = cf;
                    kbc->maxNewHeurScore = MAX_NEG_INT32;
                }

                for (gn = ln->children; gn; gn = gnode_next(gn)) {
                    ln2 = gnode_ptr(gn);

                    newHeurScore =
                        hmm->out.score + (ln2->prob - ln->prob) +
                        phn_heur_list[(int32) ln2->ci];
                    if (kbc->maxNewHeurScore < newHeurScore)
                        kbc->maxNewHeurScore = newHeurScore;
                }
                hth = kbc->maxNewHeurScore + heur_beam;
            }

            /* Transition to each child */
            for (gn = ln->children; gn; gn = gnode_next(gn)) {

                ln2 = gnode_ptr(gn);
                hmm2 = &(ln2->hmm);

                /*Sorry, code of composite triphone mode and full expansion mode are mixed 
                   If not, it could run on. 
                 */
                if (dict2pid_is_composite(d2p) ||
                    (!dict2pid_is_composite(d2p) && NOT_S3WID(ln2->wid))
                    ) {         /* If we use composite triphone mode. Or If we are 
                                   not using composite triphone mode but the next node
                                   is not a leave node .
                                   Just enter like it is a simple triphone. 
                                 */
                    newscore = hmm->out.score + (ln2->prob - ln->prob);
                    newHeurScore =
                        newscore + phn_heur_list[(int32) ln2->ci];

                    if (((heur_type == 0) ||    /*If the heuristic type is 0, 
                                                   by-pass heuristic score OR */
                         (heur_type > 0 && newHeurScore >= hth)) &&     /*If the heuristic type is other 
                                                                           and if the heur score is within threshold */
                        (newscore >= th) &&     /*If the score is smaller than the
                                                   phone score, prune away */
                        (hmm2->in.score < newscore)     /*Just the Viterbi Update */
                        ) {

                        hmm2->in.score = newscore;
                        hmm2->in.history = hmm->out.history;

                        if (ln2->frame != nf) {
                            ln2->frame = nf;
                            /*                  lextree_realloc_active_list(lextree,n+1); */
                            lextree->next_active[n++] = ln2;
                        }
                    }
                }
                else {
                    assert(IS_S3WID(ln2->wid));
                    assert(ln2->ssid == BAD_S3SSID && ln2->rc == BAD_S3CIPID);  /* Make sure that the another indication is 
                                                                                   proved. */
                    assert(!dict2pid_is_composite(d2p));

                    /* If the node doens't has children */
                    if (ln2->children == NULL) {        /*Is children not allocated, then allocate it first. */
                        assert(dict_pronlen(dict, ln2->wid) > 1);       /* Because word enter should have already taken care 
                                                                           expansion of single word case. 
                                                                         */
                        assert(ln2->ssid == BAD_S3SSID);        /*First timer of being expanded */
                        n_ci = mdef_n_ciphone(mdef);


#if 0
                        E_INFO
                            ("Tree %d, Cross word expansion is carried out at cf %d for wid %d, wstr %s, ln->children %d\n",
                             lextree, cf, ln2->wid, dict_wordstr(kbc->dict,
                                                                 ln2->wid),
                             ln2->children);
#endif

                        rmap = kbc->dict2pid->rssid[ln2->ci][ln->ci].ssid;
                        n_rc =
                            kbc->dict2pid->rssid[ln2->ci][ln->ci].n_ssid;

                        assert(n_rc ==
                               get_rc_nssid(kbc->dict2pid, ln2->wid,
                                            kbc->dict));

                        for (rc = 0; rc < n_rc; rc++) {

                            cwln =
                                lextree_node_alloc(ln2->wid, ln2->prob,
                                                   NOT_COMPOSITE, rmap[rc],
                                                   n_st, ln2->ci, rc);
                            lextree_n_node(lextree) += 1;

                            cwln->hmm.tp =
                                tmat->tp[mdef_pid2tmatid(mdef, ln2->ci)];
                            ln2->children =
                                glist_add_ptr(ln2->children,
                                              (void *) cwln);
                        }
                    }

                    /* For each of them sum of the scores and decide which one should be enter */
                    for (gn2 = ln2->children; gn2; gn2 = gnode_next(gn2)) {
                        cwln = gnode_ptr(gn2);
                        cwhmm = &(cwln->hmm);

                        /* The following two can actually be saved. However, there is a possiblity
                           that one might want to use different lookahead probability for different
                           cw triphone */

                        newscore = hmm->out.score + (cwln->prob - ln->prob);    /*< This is correct! because ln->prob is directly
                                                                                   feed into the cross word */
                        newHeurScore =
                            newscore + phn_heur_list[(int32) cwln->ci];

                        if (((heur_type == 0) ||        /*If the heuristic type is 0, 
                                                           by-pass heuristic score OR */
                             (heur_type > 0 && newHeurScore >= hth)) && /*If the heuristic type is other 
                                                                           and if the heur score is within threshold */
                            (newscore >= th) && /*If the score is smaller than the
                                                   phone score, prune away */
                            (cwhmm->in.score < newscore)        /*Just the Viterbi Update */
                            ) {

                            cwhmm->in.score = newscore;
                            cwhmm->in.history = hmm->out.history;

                            if (cwln->frame != nf) {
                                cwln->frame = nf;
                                /*                        lextree_realloc_active_list(lextree,n+1); */
                                lextree->next_active[n++] = cwln;
                            }
                        }
                    }
                    assert(ln2->ssid == BAD_S3SSID && ln2->rc == BAD_S3CIPID);  /* Make sure that the mother of all cross-word expansion
                                                                                   is not touched */
                }
            }
        }
    }

    lextree->n_next_active = n;
#if 0
    E_INFO("Debugging.\n");
    for (i = 0; i < lextree->n_next_active; i++) {
        ln = lextree->next_active[i];
        hmm = &(ln->hmm);

        E_INFO(" ln->wid %d, str %s, ln->ssid %d, ln->rc %d,\n", ln->wid,
               dict_wordstr(dict, ln->wid), ln->ssid, ln->rc);
    }
#endif
    /*    E_INFO("lextree->n_next_active %d\n",    lextree->n_next_active); */
    return LEXTREE_OPERATION_SUCCESS;
}

int32
lextree_hmm_propagate_leaves(lextree_t * lextree, kbcore_t * kbc,
                             vithist_t * vh, int32 cf, int32 wth)
{

    lextree_node_t **list, *ln;
    hmm_t *hmm;
    int32 i;
#if 0
    int32 active_word_end = 0;
#endif

    /* Code for heursitic score */
    list = lextree->active;

    for (i = 0; i < lextree->n_active; i++) {
        ln = list[i];
        hmm = &(ln->hmm);

        if (IS_S3WID(ln->wid)) {        /* Leaf node; word exit */

            if (hmm->out.score < wth)
                continue;       /* Word exit score not good enough */

            if (hmm->out.history == -1) {       /* This is a case where continue
                                                   subsituting out.history into
                                                   vithist_rescore will cause
                                                   core-dump */
                E_ERROR("out.history==-1, error\n");
                return LEXTREE_OPERATION_FAILURE;
            }


            /* Rescore the LM prob for this word wrt all possible predecessors */

            if (dict2pid_is_composite(kbc->dict2pid)) {
                vithist_rescore(vh, kbc, ln->wid, cf,
                                hmm->out.score - ln->prob,
                                hmm->out.history, lextree->type, -1);
            }
            else {
                /*              lextree_node_print(ln,kbc->dict,stdout); */
                assert(ln->ssid != BAD_S3SSID); /*This make we are not using the mother of cross-word triphone */
                assert(ln->rc != BAD_S3CIPID);

                vithist_rescore(vh, kbc, ln->wid, cf,
                                hmm->out.score - ln->prob,
                                hmm->out.history, lextree->type, ln->rc);

            }


#if 0
            active_word_end++;

            /*      E_INFO("What is the hmm->out.score %d wth %d\n", hmm->out.score,wth);
               E_INFO("\nActive word end id %d, word end %s\n", ln->wid, dict_wordstr(kbc->dict,dict_basewid(kbc->dict,ln->wid))); */
#endif
        }
    }

    /*    E_INFO("No of active word end %d\n\n",active_word_end); */
    return LEXTREE_OPERATION_SUCCESS;
}
