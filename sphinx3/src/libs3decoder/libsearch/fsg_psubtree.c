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
 * fsg_psubtree.c -- Phone-level FSG subtree representing all transitions
 * out of a single FSG state.
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
 * $Log$
 * Revision 1.2  2006/02/23  05:10:18  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Adaptation of Sphinx 2's FSG search into Sphinx 3
 * 
 * Revision 1.1.2.9  2005/07/24 19:34:46  arthchan2003
 * Removed search_hyp_t, used srch_hyp_t instead
 *
 * Revision 1.1.2.8  2005/07/24 01:34:54  arthchan2003
 * Mode 2 is basically running. Still need to fix function such as resulting and build the correct utterance ID
 *
 * Revision 1.1.2.7  2005/07/22 03:37:49  arthchan2003
 * Removal of word_fsg's context table initialization initialization.
 *
 * Revision 1.1.2.6  2005/07/20 21:18:30  arthchan2003
 * FSG can now be read, srch_fsg_init can now be initialized, psubtree can be built. Sounds like it is time to plug in other function pointers.
 *
 * Revision 1.1.2.5  2005/07/17 05:49:37  arthchan2003
 * Mistakes in last update therefore made small changes to give comment.  Implemented 2 major hacks: hack 1 replaced fsg_hmm_t with whmm_t (was used in decode_anytopo in sphinx 3.0,  hack 2, use the arrays in ctxt_table_t to implement psubtree_add_trans.
 *
 * Revision 1.1.2.4  2005/07/17 05:44:32  arthchan2003
 * Added dag_write_header so that DAG header writer could be shared between 3.x and 3.0. However, because the backtrack pointer structure is different in 3.x and 3.0. The DAG writer still can't be shared yet.
 *
 * Revision 1.1.2.3  2005/07/13 18:39:47  arthchan2003
 * (For Fun) Remove the hmm_t hack. Consider each s2 global functions one-by-one and replace them by sphinx 3's macro.  There are 8 minor HACKs where functions need to be removed temporarily.  Also, there are three major hacks. 1,  there are no concept of "phone" in sphinx3 dict_t, there is only ciphone. That is to say we need to build it ourselves. 2, sphinx2 dict_t will be a bunch of left and right context tables.  This is currently bypass. 3, the fsg routine is using fsg_hmm_t which is just a duplication of CHAN_T in sphinx2, I will guess using hmm_evaluate should be a good replacement.  But I haven't figure it out yet.
 *
 * Revision 1.1.2.2  2005/06/28 07:01:20  arthchan2003
 * General fix of fsg routines to make a prototype of fsg_init and fsg_read. Not completed.  The number of empty functions in fsg_search is now decreased from 35 to 30.
 *
 * Revision 1.1.2.1  2005/06/27 05:26:29  arthchan2003
 * Sphinx 2 fsg mainpulation routines.  Compiled with faked functions.  Currently fended off from users.
 *
 * Revision 1.1  2004/07/16 00:57:11  egouvea
 * Added Ravi's implementation of FSG support.
 *
 * Revision 1.3  2004/06/25 14:49:08  rkm
 * Optimized size of history table and speed of word transitions by maintaining only best scoring word exits at each state
 *
 * Revision 1.2  2004/05/27 14:22:57  rkm
 * FSG cross-word triphones completed (but for single-phone words)
 *
 * Revision 1.1.1.1  2004/03/01 14:30:30  rkm
 *
 *
 * Revision 1.3  2004/02/27 21:01:25  rkm
 * Many bug fixes in multiple FSGs
 *
 * Revision 1.2  2004/02/27 15:05:21  rkm
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/23 15:53:45  rkm
 * Renamed from fst to fsg
 *
 * Revision 1.3  2004/02/23 15:09:50  rkm
 * *** empty log message ***
 *
 * Revision 1.2  2004/02/19 21:16:54  rkm
 * Added fsg_search.{c,h}
 *
 * Revision 1.1  2004/02/17 21:11:49  rkm
 * *** empty log message ***
 *
 * 
 * 10-Feb-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */



#include <stdio.h>
#include <string.h>
#include <assert.h>


#include "fsg_psubtree.h"
/*#include "kb.h"*/
#include "dict.h"
/*#include "phone.h"*/
#include "logs3.h"
#include "hmm.h"
#include "search.h"

void
fsg_pnode_add_all_ctxt(fsg_pnode_ctxt_t * ctxt)
{
    int32 i;

    for (i = 0; i < FSG_PNODE_CTXT_BVSZ; i++)
        ctxt->bv[i] = 0xffffffff;
}


uint32
fsg_pnode_ctxt_sub(fsg_pnode_ctxt_t * src, fsg_pnode_ctxt_t * sub)
{
    int32 i;
    uint32 non_zero;

    non_zero = 0;

    for (i = 0; i < FSG_PNODE_CTXT_BVSZ; i++) {
        src->bv[i] = ~(sub->bv[i]) & src->bv[i];
        non_zero |= src->bv[i];
    }

    return non_zero;
}

/*
 * Add the word emitted by the given transition (fsglink) to the given lextree
 * (rooted at root), and return the new lextree root.  (There may actually be
 * several root nodes, maintained in a linked list via fsg_pnode_t.sibling.
 * "root" is the head of this list.)
 * lclist, rclist: sets of left and right context phones for this link.
 * alloc_head: head of a linear list of all allocated pnodes for the parent
 * FSG state, kept elsewhere and updated by this routine.
 * 
 * NOTE: No lextree structure for now; using a flat representation.
 *
 *
 */
static fsg_pnode_t *
psubtree_add_trans(fsg_pnode_t * root,
                   hmm_context_t *ctx,
                   word_fsglink_t * fsglink,
                   int8 * lclist, int8 * rclist,
                   fsg_pnode_t ** alloc_head,
                   dict_t * dict,
                   mdef_t * mdef,
                   ctxt_table_t * ctxt_tab,
                   cmd_ln_t *config,
                   logmath_t *logmath)
{

    int32 wip;                  /* Word Insertion Penalty */
    int32 pip;                  /* Phone Insertion Penalty */
    int32 pronlen;              /* Pronunciation length */
    float32 lw;                 /* Language weight */

    s3cipid_t silcipid;         /* Silence CI phone ID */
    s3wid_t wid;                /* Word ID */
    s3cipid_t bid, lc, rc;      /* The base phone ID */
    s3ssid_t ssid;              /* Senone Sequence ID */

    gnode_t *gn;
    fsg_pnode_t *pnode, *pred, *head;
    int32 n_ci, p;

    glist_t lc_pnodelist;       /* Temp pnodes list for different left contexts */
    glist_t rc_pnodelist;       /* Temp pnodes list for different right contexts */
    fsg_pnode_t **ssid_pnode_map;       /* Temp array of ssid->pnode mapping */
    int32 i;

    lw = cmd_ln_float32_r(config, "-lw");
    pip = (int32) (logs3(logmath, cmd_ln_float32_r(config, "-phonepen")) * lw);
    wip = (int32) (logs3(logmath, cmd_ln_float32_r(config, "-wip")) * lw);

    silcipid = mdef->sil;
    n_ci = mdef_n_ciphone(mdef);

    wid = word_fsglink_wid(fsglink);
    assert(wid >= 0);           /* Cannot be a null transition */

    pronlen = dict_pronlen(dict, wid);
    assert(pronlen >= 1);
    if (pronlen > 255) {
        E_FATAL
            ("Pronlen too long (%d); cannot use int8 for fsg_pnode_t.ppos\n",
             pronlen);
    }

    assert(lclist[0] >= 0);     /* At least one phonetic context provided */
    assert(rclist[0] >= 0);

    head = *alloc_head;
    pred = NULL;

    if (pronlen == 1) {         /* Single-phone word */

        bid = dict_pron(dict, wid, 0);

        /* HACK! Mimic this behavior of Sphinx 2 by asking whether the word is a filler 
           Sphinx 2 will actually ask whether a word is multiplexed.  That is to say whether
           it is a single phone non-filer word. 
         */

        if (!dict_filler_word(dict, wid)) {     /* Only non-filler words are mpx */
            /*
             * Left diphone ID for single-phone words already assumes SIL is right
             * context; only left contexts need to be handled.
             */
            lc_pnodelist = NULL;

            for (i = 0; lclist[i] >= 0; i++) {
                lc = lclist[i];

#if 0                           /* Sphinx 2 logic */
                lcfwd[bid][lc]; /*Use lcfwd for single-phone word, not lcbwd,
                                   as lcbwd would use only SIL as context */

#endif

                /* Always use silence as the right context 

                   "lc <- bid -> sil"

                 */

                /* HACK! Is this correct? */
                ssid = ctxt_table_single_phone_ssid(ctxt_tab, lc, bid, silcipid);

                /* Check if this ssid already allocated for some other context */
                for (gn = lc_pnodelist; gn; gn = gnode_next(gn)) {
                    pnode = (fsg_pnode_t *) gnode_ptr(gn);
                    /*      pnode->hmm = whmm_alloc_light(n_state_hmm); */

                    if (hmm_nonmpx_ssid(&pnode->hmm) == ssid) {
                        /* already allocated; share it for this context phone */
                        fsg_pnode_add_ctxt(pnode, lc);
                        break;
                    }
                }

                if (!gn) {      /* ssid not already allocated */
                    pnode =
                        (fsg_pnode_t *) ckd_calloc(1, sizeof(fsg_pnode_t));
                    pnode->next.fsglink = fsglink;
                    pnode->logs2prob =
                        word_fsglink_logs2prob(fsglink) + wip + pip;
                    pnode->ci_ext = (int8) dict_pron(dict, wid, 0);
                    pnode->ppos = 0;
                    pnode->leaf = TRUE;
                    pnode->sibling = root;      /* All root nodes linked together */
                    fsg_pnode_add_ctxt(pnode, lc);      /* Initially zeroed by calloc above */
                    pnode->alloc_next = head;
                    head = pnode;
                    root = pnode;

                    hmm_init(ctx, &pnode->hmm, FALSE, ssid, pnode->ci_ext);

                    lc_pnodelist =
                        glist_add_ptr(lc_pnodelist, (void *) pnode);
                }
            }

            glist_free(lc_pnodelist);
        }
        else {                  /* Filler word; no context modelled */
	    ssid = mdef_pid2ssid(mdef, silcipid);

            pnode = (fsg_pnode_t *) ckd_calloc(1, sizeof(fsg_pnode_t));
            pnode->next.fsglink = fsglink;
            pnode->logs2prob = word_fsglink_logs2prob(fsglink) + wip + pip;
            pnode->ci_ext = silcipid;   /* Presents SIL as context to neighbors */
            pnode->ppos = 0;
            pnode->leaf = TRUE;
            pnode->sibling = root;
            fsg_pnode_add_all_ctxt(&(pnode->ctxt));
            pnode->alloc_next = head;
            head = pnode;
            root = pnode;

            hmm_init(ctx, &pnode->hmm, FALSE, ssid, pnode->ci_ext);
        }
    }
    else {                      /* Multi-phone word */

#if 0
        assert(dict_mpx(dict, wid));    /* S2 HACK: pronlen>1 => mpx?? */
#endif
        assert(pronlen > 1);
        /* HACK! Mimic the behavior by checking whether it is a non-filler */
        assert(!dict_filler_word(dict, wid));   /* S2 HACK: pronlen>1 => mpx?? */

        ssid_pnode_map =
            (fsg_pnode_t **) ckd_calloc(n_ci, sizeof(fsg_pnode_t *));
        lc_pnodelist = NULL;
        rc_pnodelist = NULL;

        for (p = 0; p < pronlen; p++) {

            bid = dict_pron(dict, wid, p);

            if (p == 0) {       /* Root phone, handle required left contexts 
                                 */
                /*
                   The first phone of the word. Multiple left contexts
                   is now considered.

                   lc <-
                   \
                   lc <-  b -> rc
                   /
                   lc <-
                 */
                for (i = 0; lclist[i] >= 0; i++) {

                    /*
                       lc = lclist[i];
                       j = lcbwdperm[did][lc];
                       ssid = lcbwd[did][j];
                       pnode = ssid_pnode_map[j];
                     */

                    /* HACK! Is this correct? 
                     */

                    lc = lclist[i];
                    rc = dict_pron(dict, wid, p + 1);
                    ssid = ctxt_table_left_ctxt_ssid(ctxt_tab, lc, bid, rc);

                    pnode = ssid_pnode_map[lc];

                    /*      E_INFO("wstr %s, lc %d %s, bid %d %s rc %d %s ssid %d\n",dict_wordstr(dict,wid),lc,mdef_ciphone_str(mdef,lc),bid, mdef_ciphone_str(mdef,bid), rc,mdef_ciphone_str(mdef,rc), *ssid);             */
                    if (!pnode) {       /* Allocate pnode for this new ssid */
                        pnode =
                            (fsg_pnode_t *) ckd_calloc(1,
                                                       sizeof
                                                       (fsg_pnode_t));
                        pnode->logs2prob =
                            word_fsglink_logs2prob(fsglink) + wip + pip;

                        pnode->ci_ext = (int8) dict_pron(dict, wid, 0);
                        pnode->ppos = 0;
                        pnode->leaf = FALSE;
                        pnode->sibling = root;  /* All root nodes linked together */
                        pnode->alloc_next = head;
                        head = pnode;
                        root = pnode;

                        hmm_init(ctx, &pnode->hmm, FALSE, ssid, pnode->ci_ext);

                        lc_pnodelist =
                            glist_add_ptr(lc_pnodelist, (void *) pnode);
                        ssid_pnode_map[lc] = pnode;
                    }
                    else {
                        assert(hmm_nonmpx_ssid(&pnode->hmm) == ssid);
                    }
                    fsg_pnode_add_ctxt(pnode, lc);
                }
            }
            else if (p != pronlen - 1) {        /* Word internal phone 

                                                   In this case, there is only one
                                                   unique left-context and right context. 
                                                   So, we can directly use wwpid to gather
                                                   the information
                                                   i.e

                                                   lc <- b -> rc
                                                 */
                ssid = ctxt_table_word_int_ssid(ctxt_tab, wid, p);

                pnode = (fsg_pnode_t *) ckd_calloc(1, sizeof(fsg_pnode_t));
                pnode->logs2prob = pip;

                pnode->ci_ext = (int8) dict_pron(dict, wid, p);

                pnode->ppos = p;
                pnode->leaf = FALSE;
                pnode->sibling = NULL;
                if (p == 1) {   /* Predecessor = set of root nodes for left ctxts */
                    for (gn = lc_pnodelist; gn; gn = gnode_next(gn)) {
                        pred = (fsg_pnode_t *) gnode_ptr(gn);
                        pred->next.succ = pnode;
                    }
                }
                else {          /* Predecessor = word internal node */
                    pred->next.succ = pnode;
                }
                pnode->alloc_next = head;
                head = pnode;

                hmm_init(ctx, &pnode->hmm, FALSE, ssid, pnode->ci_ext);

                pred = pnode;
            }
            else {              /* Leaf phone, handle required right contexts */
                memset((void *) ssid_pnode_map, 0,
                       n_ci * sizeof(fsg_pnode_t *));
                /*
                   The first phone of the word. Multiple left contexts
                   is now considered.

                   > rc
                   /
                   lc <-  b -> rc
                   \
                   > rc

                 */

                for (i = 0; rclist[i] >= 0; i++) {
                    rc = rclist[i];

                    /* HACK! 
                       Am I correct?
                     */
                    lc = dict_pron(dict, wid, p - 1);
                    ssid = ctxt_table_right_ctxt_ssid(ctxt_tab, lc, bid, rc);
                    pnode = ssid_pnode_map[rc];

                    /*E_INFO("wstr %s lc %d %s, bid %d %s rc %d %s ssid %d\n",dict_wordstr(dict,wid),lc,mdef_ciphone_str(mdef,lc),bid, mdef_ciphone_str(mdef,bid), rc,mdef_ciphone_str(mdef,rc), *ssid);            */
                    if (!pnode) {       /* Allocate pnode for this new ssid */
                        pnode =
                            (fsg_pnode_t *) ckd_calloc(1,
                                                       sizeof
                                                       (fsg_pnode_t));
                        pnode->logs2prob = pip;
			pnode->ci_ext = (int8) dict_pron(dict, wid, p);
                        pnode->ppos = p;
                        pnode->leaf = TRUE;
                        pnode->sibling = rc_pnodelist ?
                            (fsg_pnode_t *) gnode_ptr(rc_pnodelist) : NULL;
                        pnode->next.fsglink = fsglink;
                        pnode->alloc_next = head;
                        head = pnode;

                        hmm_init(ctx, &pnode->hmm, FALSE, ssid, pnode->ci_ext);

                        rc_pnodelist =
                            glist_add_ptr(rc_pnodelist, (void *) pnode);
                        ssid_pnode_map[rc] = pnode;
                    }
                    else {
                        assert(hmm_nonmpx_ssid(&pnode->hmm) == ssid);
                    }
                    fsg_pnode_add_ctxt(pnode, rc);
                }

                if (p == 1) {   /* Predecessor = set of root nodes for left ctxts */
                    for (gn = lc_pnodelist; gn; gn = gnode_next(gn)) {
                        pred = (fsg_pnode_t *) gnode_ptr(gn);
                        pred->next.succ =
                            (fsg_pnode_t *) gnode_ptr(rc_pnodelist);
                    }
                }
                else {          /* Predecessor = word internal node */
                    pred->next.succ =
                        (fsg_pnode_t *) gnode_ptr(rc_pnodelist);
                }
            }
        }

        ckd_free((void *) ssid_pnode_map);
        glist_free(lc_pnodelist);
        glist_free(rc_pnodelist);
    }

    *alloc_head = head;

    return root;
}


/*
 * For now, this "tree" will be "flat"
 */
fsg_pnode_t *
fsg_psubtree_init(hmm_context_t *ctx,
                  word_fsg_t * fsg, int32 from_state,
                  fsg_pnode_t ** alloc_head, cmd_ln_t *config, logmath_t *logmath)
{
    int32 dst;
    gnode_t *gn;
    word_fsglink_t *fsglink;
    fsg_pnode_t *root;
    int32 n_ci;

    root = NULL;
    assert(*alloc_head == NULL);

    n_ci = fsg->n_ciphone;
    if (n_ci > (FSG_PNODE_CTXT_BVSZ * 32)) {
        E_FATAL
            ("#phones > %d; increase FSG_PNODE_CTXT_BVSZ and recompile\n",
             FSG_PNODE_CTXT_BVSZ * 32);
    }

    for (dst = 0; dst < word_fsg_n_state(fsg); dst++) {
        /* Add all links from from_state to dst */
        for (gn = word_fsg_trans(fsg, from_state, dst); gn;
             gn = gnode_next(gn)) {
            /* Add word emitted by this transition (fsglink) to lextree */
            fsglink = (word_fsglink_t *) gnode_ptr(gn);

            assert(word_fsglink_wid(fsglink) >= 0);     /* Cannot be a null trans */

            root = psubtree_add_trans(root, ctx, fsglink,
                                      word_fsg_lc(fsg, from_state),
                                      word_fsg_rc(fsg, dst),
                                      alloc_head,
                                      fsg->dict,
                                      fsg->mdef, fsg->ctxt, config, logmath);
        }
    }

    return root;
}


void
fsg_psubtree_free(fsg_pnode_t * head)
{
    fsg_pnode_t *next;

    while (head) {
        next = head->alloc_next;

        hmm_deinit(&head->hmm);
        ckd_free((void *) head);

        head = next;
    }
}


void
fsg_psubtree_dump(fsg_pnode_t * head, FILE * fp, dict_t * dict,
                  mdef_t * mdef)
{
    int32 i;
    word_fsglink_t *tl;

    for (; head; head = head->alloc_next) {
        /* Indentation */
        for (i = 0; i <= head->ppos; i++)
            fprintf(fp, "  ");

        fprintf(fp, "%p.@", head);    /* Pointer used as node ID */
        fprintf(fp, " %5d.SS", hmm_nonmpx_ssid(&head->hmm));
        fprintf(fp, " %10d.LP", head->logs2prob);
        fprintf(fp, " %p.SIB", head->sibling);
        fprintf(fp, " %s.%d", mdef_ciphone_str(mdef, (head->ci_ext)),
                head->ppos);
        if ((head->ppos == 0) || head->leaf) {
            fprintf(fp, " [");
            for (i = 0; i < FSG_PNODE_CTXT_BVSZ; i++)
                fprintf(fp, "%08x", head->ctxt.bv[i]);
            fprintf(fp, "]");
        }
        if (head->leaf) {
            tl = head->next.fsglink;
            fprintf(fp, " {%s[%d->%d](%d)}",
                    dict_wordstr(dict, tl->wid),
                    tl->from_state, tl->to_state, tl->logs2prob);
        }
        else {
            fprintf(fp, " %p.NXT", head->next.succ);
        }
        fprintf(fp, "\n");
    }

    fflush(fp);
}


int
fsg_psubtree_pnode_enter(fsg_pnode_t * pnode,
                         int32 score, int32 frame, int32 bpidx)
{
    int activate;

    assert(hmm_frame(&pnode->hmm) <= frame);

    score += pnode->logs2prob;

    activate = FALSE;

    if (hmm_in_score(&pnode->hmm) < score) {
        if (hmm_frame(&pnode->hmm) < frame)
            activate = TRUE;
	hmm_enter(&pnode->hmm, score, bpidx, frame);
    }

    return activate;
}


void
fsg_psubtree_pnode_deactivate(fsg_pnode_t * pnode)
{
    hmm_clear(&pnode->hmm);
}
