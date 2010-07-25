/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * dag.c -- Library for DAG
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log: dag.c,v $
 * Revision 1.3  2006/02/28 02:06:46  egouvea
 * Updated MS Visual C++ 6.0 support files. Fixed things that didn't
 * compile in Visual C++ (declarations didn't match, etc). There are
 * still some warnings, so this is not final. Also, sorted files in
 * several Makefile.am.
 *
 * Revision 1.2  2006/02/23 05:22:32  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed bugs from last check in, lw should be * instead of +, 2, Moved most of the functions from flat_fwd.c and s3_dag.c to here.  Things that required specified will be prefixed.
 *
 * Revision 1.1.4.8  2006/02/17 19:27:37  arthchan2003
 * Fixed a conformance issue of dag.c, this will transform the word to its basewid, in computing LM.
 *
 * Revision 1.1.4.7  2006/01/16 18:18:12  arthchan2003
 * Added a flag on DAG loading when acoustic scores are presented in the lattice file.
 *
 * Revision 1.1.4.6  2005/11/17 06:25:04  arthchan2003
 * 1, Added structure to record node-based ascr and lscr. 2, Added a version of dag_link that copies the langauge model score as well.
 *
 * Revision 1.1.4.5  2005/09/25 19:20:43  arthchan2003
 * Added hooks in dag_node and dag_link. Probably need some time to use it various routines of ours.
 *
 * Revision 1.1.4.4  2005/09/11 23:07:28  arthchan2003
 * srch.c now support lattice rescoring by rereading the generated lattice in a file. When it is operated, silence cannot be unlinked from the dictionary.  This is a hack and its reflected in the code of dag, kbcore and srch. code
 *
 * Revision 1.1.4.3  2005/09/11 02:56:47  arthchan2003
 * Log. Incorporated all dag related functions from s3_dag.c and
 * flat_fwd.c.  dag_search, dag_add_fudge, dag_remove_filler is now
 * shared by dag and decode_anytopo. (Hurray!). s3_astar.c still has
 * special functions and it probably unavoidable.
 *
 * Revision 1.1.4.2  2005/07/26 02:20:39  arthchan2003
 * merged hyp_t with srch_hyp_t.
 *
 * Revision 1.1.4.1  2005/07/17 05:44:30  arthchan2003
 * Added dag_write_header so that DAG header writer could be shared between 3.x and 3.0. However, because the backtrack pointer structure is different in 3.x and 3.0. The DAG writer still can't be shared yet.
 *
 * Revision 1.1  2005/06/21 22:37:47  arthchan2003
 * Build a stand-alone wrapper for direct acyclic graph, it is now shared across dag/astar and decode_anytopo.  This eliminate about 500 lines of code in decode_anytopo/dag and astar. However, its existence still can't exterminate code duplication between dag/decode_anytopo.  That effectively means we have many refactoring to do.  Things that are still pretty difficult to merge include dag_search(decode_anytopo/dag) and dag_read (dag/astar).
 *
 * Revision 1.2  2005/06/03 06:45:28  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.1  2005/06/03 05:46:19  archan
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
 * Revision 1.2  2005/03/30 00:43:41  archan
 *
 * 28-Jul-04    ARCHAN at Carnegie Mellon Unversity
 *              First adapted from s3.              
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 04-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_chk_linkscr().
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Implemented -maxedge argument to control memory usage.
 * 
 * 21-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxlmop and -maxlpf options to abort utterance if exceeded.
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added exact reporting of word sequence and scores from dag_search.
 * 		For this, added daglink_t.bypass, daglink_t.lscr, daglink_t.src, and
 * 		added bypass argument to dag_link and dag_bypass_link, and changed
 * 		dag_backtrace to find exact best path.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 * 
 * 03-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#include <string.h>
#if defined(WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sphinxbase/listelem_alloc.h>
#include <sphinxbase/pio.h>

#include "dag.h"
#include "vithist.h"
#include "logs3.h"

void
hyp_free(srch_hyp_t * list)
{
    srch_hyp_t *h;

    while (list) {
        h = list->next;
        ckd_free((char *) list);
        list = h;
    }
}


/*
 * Link two DAG nodes with the given arguments
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
int32
dag_link(dag_t * dagp, dagnode_t * pd, dagnode_t * d, int32 ascr,
	 int32 lscr, int32 ef, daglink_t * byp)
{
    daglink_t *l;

    /* HACK: silently refuse to create positive edges, since we won't accept them. */
    if (ascr > 0)
	    return 0;

    /* Link d into successor list for pd */
    if (pd) { /* Special condition for root node which doesn't have a predecessor */
        l = listelem_malloc(dagp->link_alloc);
        l->node = d;
        l->src = pd;
        l->ascr = ascr;
        l->lscr = lscr;
        l->hscr = 0;
        l->pscr = (int32) 0x80000000;
        l->pscr_valid = 0;
        l->history = NULL;
        l->ef = ef;
        l->next = pd->succlist;
	assert(pd->succlist != l);
        l->bypass = byp;
        l->hook = NULL;

        pd->succlist = l;
    }

    /* Link pd into predecessor list for d */
    l = listelem_malloc(dagp->link_alloc);
    l->node = pd;
    l->src = d;
    l->ascr = ascr;
    l->lscr = lscr;
    l->hscr = 0;
    l->pscr = (int32) 0x80000000;
    l->pscr_valid = 0;
    l->history = NULL;
    l->ef = ef;
    l->bypass = byp;
    l->hook = NULL;

    l->next = d->predlist;
    assert(d->predlist != l);
    d->predlist = l;

    if (byp) dagp->nbypass++;
    dagp->nlink++;

    return (dagp->nlink > dagp->maxedge) ? -1 : 0;
}

daglink_t *
find_succlink(dagnode_t * src, dagnode_t * dst, int32 bypass)
{
    daglink_t *l;

    for (l = src->succlist; l; l = l->next) {
        if (l->node == dst) {
            if (bypass && !l->bypass)
                continue;
            else
                break;
        }
    }
    return l;
}

daglink_t *
find_predlink(dagnode_t * src, dagnode_t * dst, int32 bypass)
{
    daglink_t *l;

    for (l = src->predlist; l; l = l->next) {
        if (l->node == dst) {
            if (bypass && !l->bypass)
                continue;
            else
                break;
        }
    }
    return l;
}


/*
 * Like dag_link but check if link already exists.  If so, replace if new score better.
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
int32
dag_update_link(dag_t * dagp, dagnode_t * pd, dagnode_t * d, int32 ascr,
                int32 ef, daglink_t * byp)
{
    daglink_t *l, *r;
    l = find_succlink(pd, d, (byp != NULL));

    if (!l)
        return (dag_link(dagp, pd, d, ascr, 0, ef, byp));

    if (l->ascr < ascr) {
        r = find_predlink(d, pd, (byp != NULL));

        assert(r && (r->ascr == l->ascr));
        l->ascr = r->ascr = ascr;
        l->ef = r->ef = ef;
        l->bypass = r->bypass = byp;
    }

    return 0;
}

/**
 * Mark every node that has a path to the argument dagnode as "reachable".
 */
static void
dag_mark_reachable(dagnode_t * d)
{
    daglink_t *l;

    d->reachable = 1;
    for (l = d->predlist; l; l = l->next)
        if (l->node && !l->node->reachable)
            dag_mark_reachable(l->node);
}


void
dag_remove_unreachable(dag_t *dag)
{
    dagnode_t *d, *dd;
    daglink_t *l, *pl, *nl;

    dag_mark_reachable(dag->end);
    for (d = dag->list; d; d = d->alloc_next) {
        if (!d->reachable) {
            /* Remove successor node links */
            for (l = d->succlist; l; l = nl) {
                nl = l->next;
                --dag->nlink;
                listelem_free(dag->link_alloc, l);
            }
            d->succlist = NULL;

            /* Remove predecessor links */
            for (l = d->predlist; l; l = nl) {
                nl = l->next;
                listelem_free(dag->link_alloc, l);
            }
            d->predlist = NULL;
        }
        else {
            /* Remove successor links to unreachable nodes; predecessors are reachable */
            pl = NULL;
            for (l = d->succlist; l; l = nl) {
                nl = l->next;
                if (!l->node->reachable) {
                    if (!pl)
                        d->succlist = nl;
                    else
                        pl->next = nl;
                    --dag->nlink;
                    listelem_free(dag->link_alloc, l);
                }
                else
                    pl = l;
            }
        }
    }

    for (d = dag->list, dd = NULL; d; d = d->alloc_next) {
        if (!d->reachable && dd) {
            /* Remove unreachable nodes */
            dd->alloc_next = d->alloc_next;
            listelem_free(dag->node_alloc, d);
            d = dd;
            --dag->nnode;
        }
        dd = d;
    }
}

/* Read parameter from a lattice file*/
int32
dag_param_read(FILE * fp, char *param, int32 * lineno)
{
    char line[1024], wd[1024];
    int32 n;

    while (fgets(line, 1024, fp) != NULL) {
        (*lineno)++;
        if (line[0] == '#')
            continue;
        if ((sscanf(line, "%s %d", wd, &n) == 2)
            && (strcmp(wd, param) == 0))
            return n;
    }
    return -1;
}


/*
 * Recursive step in dag_search: best backward path from src to root
 * beginning with l.
 *
 * FIXME: Why is this implemented recursively like this?
 *
 * Return value: 0 if successful, -1 otherwise.
 */
int32
dag_bestpath(dag_t * dagp,      /* A pointer of the dag */
             daglink_t * l,     /* Backward link! */
             dagnode_t * src,   /* Source node for backward link l */
             float64 lwf,       /* Language weight multiplication factor */
             dict_t * dict,     /* The dictionary */
             lm_t * lm,         /* The LM */
             s3lmwid32_t * dict2lmwid   /* A map from dictionary id to lm id, should use wid2lm insteead */
    )
{
    dagnode_t *d, *pd;
    daglink_t *pl;
    int32 lscr, score;

    assert(!l->pscr_valid);

    if ((d = l->node) == NULL) {
        /* If no destination at end of l, src is root node.  Recursion termination */
        /* This doesn't necessarily have to be <s>. But it should be the root of the DAG. */
        assert(src == dagp->root);
        l->lscr = 0;
        l->pscr = 0;
        l->pscr_valid = 1;
        l->history = NULL;

        return 0;
    }

    /* Search all predecessor links of l */
    for (pl = d->predlist; pl; pl = pl->next) {
        pd = pl->node;
        if (pd && dict_filler_word(dict, pd->wid))      /* Skip filler node */
            continue;

        /* Evaluate best path along pl if not yet evaluated (recursive step) */
        if (!pl->pscr_valid)
            if (dag_bestpath(dagp, pl, d, lwf, dict, lm, dict2lmwid) < 0)
                return -1;

        /* Accumulated path score along pl->l */
        /*      E_INFO("lwid1 %d, wid1 %d, lwid2 %d, wid2 %d\n",
           dict2lmwid[dict_basewid(dict,d->wid)],
           d->wid,
           dict2lmwid[dict_basewid(dict,src->wid)],
           src->wid); */
        score = pl->pscr + l->ascr;
        if (score > l->pscr) {      /* rkm: Added 20-Nov-1996 */
            /* FIXME: This scales the wip implicitly */
            if (pd)
                lscr = lwf * lm_tg_score(lm,
                                         dict2lmwid[dict_basewid
                                                    (dict, pd->wid)],
                                         dict2lmwid[dict_basewid
                                                    (dict, d->wid)],
                                         dict2lmwid[dict_basewid
                                                    (dict, src->wid)],
                                         dict_basewid(dict, src->wid));
            else
                lscr = lwf * lm_bg_score(lm,
                                         dict2lmwid[dict_basewid
                                                    (dict, d->wid)],
                                         dict2lmwid[dict_basewid
                                                    (dict, src->wid)],
                                         dict_basewid(dict, src->wid));
            score += lscr;

            if (dagp->lmop++ >= dagp->maxlmop)
                return -1;

            /* Update best path and score beginning with l */
            if (score > l->pscr) {
                l->lscr = lscr;
                l->pscr = score;
                l->history = pl;
            }
        }
    }

#if 0
    printf("%s,%d -> %s,%d = %d\n",
           dict_wordstr(dict, dict_basewid(dict, d->wid)), d->sf,
           dict_wordstr(dict, dict_basewid(dict, src->wid)), src->sf,
           l->pscr);
    fflush(stdout);
#endif

    l->pscr_valid = 1;

    return 0;
}

int32
dag_chk_linkscr(dag_t * dagp)
{
    dagnode_t *d;
    daglink_t *l;

    for (d = dagp->list; d; d = d->alloc_next) {
        for (l = d->succlist; l; l = l->next) {
            /*      E_INFO("l->ascr %d\n",l->ascr); */

            /* 20040909: I change this from >= to > because s3.5 sometimes generate lattice which has 0 as the beginning node of the lattice.  This should be regarded as temporary change */
            if (l->ascr > 0) {
                return -1;
            }
        }
    }

    return 0;
}

int32
dag_destroy(dag_t * dagp)
{
    if (dagp == NULL)
        return 0;

    /* No need to crawl the structure!  Just free the allocators (hooray) */
    listelem_alloc_free(dagp->node_alloc);
    listelem_alloc_free(dagp->link_alloc);
    ckd_free(dagp);
    return 0;
}

/**
 * For each link compute the heuristic score (hscr) from the END of the link to the
 * end of the utterance; i.e. the best score from the end of the link to the dag
 * exit node.
 */
void
dag_compute_hscr(dag_t *dag, dict_t *dict, lm_t *lm, float64 lwf)
{
    dagnode_t *d, *d1, *d2;
    daglink_t *l1, *l2;
    s3wid_t bw0, bw1, bw2;
    int32 hscr, best_hscr;

    for (d = dag->list; d; d = d->alloc_next) {
        bw0 =
            dict_filler_word(dict, d->wid)
            ? BAD_S3WID
            : dict_basewid(dict, d->wid);

        /* For each link from d, compute heuristic score */
        for (l1 = d->succlist; l1; l1 = l1->next) {
            assert(l1->node->reachable);

            d1 = l1->node;
            if (d1 == dag->final.node)
                l1->hscr = 0;
            else {
                bw1 =
                    dict_filler_word(dict, d1->wid)
                    ? BAD_S3WID
                    : dict_basewid(dict, d1->wid);
                if (NOT_S3WID(bw1)) {
                    bw1 = bw0;
                    bw0 = BAD_S3WID;
                }

                best_hscr = (int32) 0x80000000;
                for (l2 = d1->succlist; l2; l2 = l2->next) {
                    d2 = l2->node;
                    if (dict_filler_word(dict, d2->wid))
                        continue;

                    bw2 = dict_basewid(dict, d2->wid);

                    /* ARCHAN , bw2 is bypassed, so we can savely ignored it */
                    hscr = l2->hscr
                        + l2->ascr
                        /* FIXME: This scales the wip implicitly */
                        + lwf * lm_tg_score(lm,
                                            (bw0 == BAD_S3WID)
                                            ? BAD_LMWID(lm) : lm->dict2lmwid[bw0],
                                            (bw1 == BAD_S3WID)
                                            ? BAD_LMWID(lm) : lm->dict2lmwid[bw1],
                                            lm->dict2lmwid[bw2], bw2);


                    if (hscr > best_hscr)
                        best_hscr = hscr;
                }

                l1->hscr = best_hscr;
            }
        }
    }
}

/**
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
srch_hyp_t *
dag_backtrace(srch_hyp_t ** hyp, daglink_t * l, float64 lwf, dict_t * dict,
              fillpen_t * fpen)
{
    srch_hyp_t *h, *hhead, *htail;
    int32 pscr;
    dagnode_t *src, *dst;
    daglink_t *bl, *hist;

    *hyp = NULL;
    dst = NULL;
    for (; l; l = hist) {
        hist = l->history;
        if (*hyp)
            (*hyp)->lscr = l->lscr;     /* As lscr actually applies to successor node */

        if (!l->node) {
            assert(!l->history);
            break;
        }

        if (!l->bypass) {
            /* Link did not bypass any filler node */
            h = (srch_hyp_t *) ckd_calloc(1, sizeof(srch_hyp_t));
            h->id = l->node->wid;
            h->word = dict_wordstr(dict, h->id);
            h->sf = l->node->sf;
            h->ef = l->ef;
            h->ascr = l->ascr;

            h->next = *hyp;
            (*hyp) = h;
        }
        else {
            /* Link bypassed one or more filler nodes; restore bypassed link seq. */
            hhead = htail = NULL;

            src = l->node;      /* Note that l is a PREDECESSOR link */
            for (; l; l = l->bypass) {
                h = (srch_hyp_t *) ckd_calloc(1, sizeof(srch_hyp_t));
                h->id = src->wid;
                h->word = dict_wordstr(dict, h->id);
                h->sf = src->sf;

                if (hhead)
                    h->lscr = /* FIXME: This scales the wip implicitly... */
                        lwf * fillpen(fpen, dict_basewid(dict, src->wid));

                if (l->bypass) {
                    dst = l->bypass->src;
                    assert(dict_filler_word(dict, dst->wid));
                    bl = find_succlink(src, dst, FALSE);
                    assert(bl);
                }
                else
                    bl = l;

                h->ef = bl->ef;
                h->ascr = bl->ascr;
                if (htail)
                    htail->next = h;
                else
                    hhead = h;
                htail = h;

                src = dst;
            }

            htail->next = (*hyp);
            (*hyp) = hhead;
        }
    }

    /* Compute path score for each node in hypothesis */
    pscr = 0;
    for (h = (*hyp); h; h = h->next) {
        pscr = pscr + h->lscr + h->ascr;
        h->pscr = pscr;
    }

    return (*hyp);
}

void
dag_init(dag_t * dagp, cmd_ln_t *config, logmath_t *logmath)
{

    /* Initialize DAG structure */
    dagp->list = NULL;
    dagp->config = config;
    dagp->node_alloc = listelem_alloc_init(sizeof(dagnode_t));
    dagp->link_alloc = listelem_alloc_init(sizeof(daglink_t));

    /* Set limit on max DAG edges allowed after which utterance is aborted */
    dagp->maxedge = cmd_ln_int32_r(config, "-maxedge");

    dagp->filler_removed = 0;
    dagp->fudged = 0;
    dagp->hook = NULL;
    dagp->logmath = logmath;
}


void
dag_write_header(FILE * fp, cmd_ln_t *config)
{
    char str[1024];
    getcwd(str, sizeof(str));
    fprintf(fp, "# getcwd: %s\n", str);

    /* Print logbase first!!  Other programs look for it early in the
     * DAG */
    fprintf(fp, "# -logbase %e\n", cmd_ln_float32_r(config, "-logbase"));
    fprintf(fp, "# -dict %s\n", cmd_ln_str_r(config, "-dict"));
    if (cmd_ln_str_r(config, "-fdict"))
        fprintf(fp, "# -fdict %s\n", cmd_ln_str_r(config, "-fdict"));
    /* Allphone mode doesn't always have a LM. */
    if (cmd_ln_str_r(config, "-lm"))
	fprintf(fp, "# -lm %s\n", cmd_ln_str_r(config, "-lm"));
    /* We might have one or many of these arguments */
    if (cmd_ln_exists_r(config, "-hmm") && cmd_ln_str_r(config, "-hmm")) 
        fprintf(fp, "# -hmm %s\n", cmd_ln_str_r(config, "-hmm"));
    if (cmd_ln_exists_r(config, "-mdef") && cmd_ln_str_r(config, "-mdef")) 
        fprintf(fp, "# -mdef %s\n", cmd_ln_str_r(config, "-mdef"));
    if (cmd_ln_exists_r(config, "-mean") && cmd_ln_str_r(config, "-mean")) 
        fprintf(fp, "# -mean %s\n", cmd_ln_str_r(config, "-mean"));
    if (cmd_ln_exists_r(config, "-var") && cmd_ln_str_r(config, "-var")) 
        fprintf(fp, "# -var %s\n", cmd_ln_str_r(config, "-var"));
    if (cmd_ln_exists_r(config, "-mixw") && cmd_ln_str_r(config, "-mixw")) 
        fprintf(fp, "# -mixw %s\n", cmd_ln_str_r(config, "-mixw"));
    if (cmd_ln_exists_r(config, "-tmat") && cmd_ln_str_r(config, "-tmat")) 
        fprintf(fp, "# -tmat %s\n", cmd_ln_str_r(config, "-tmat"));
    if (cmd_ln_exists_r(config, "-senmgau") && cmd_ln_str_r(config, "-senmgau"))
        fprintf(fp, "# -senmgau %s\n", cmd_ln_str_r(config, "-senmgau"));
    if (cmd_ln_exists_r(config, "-min_endfr")) {
        fprintf(fp, "# -min_endfr %ld\n", cmd_ln_int32_r(config, "-min_endfr"));
    }
    fprintf(fp, "#\n");
}

int32
dag_write(dag_t * dag,
          const char *filename,
          lm_t * lm,
          dict_t * dict)
{
    /* WARNING!!!! DO NOT INSERT a # in the format arbitrarily because the dag_reader is not very robust */
    int32 i;
    dagnode_t *d, *initial, *final;
    daglink_t *l;
    FILE *fp;
    int32 ispipe;

    initial = dag->root;
    final = dag->end;

    E_INFO("Writing lattice file in Sphinx III format: %s\n", filename);
    if ((fp = fopen_comp(filename, "w", &ispipe)) == NULL) {
        E_ERROR("fopen_comp (%s,w) failed\n", filename);
        return -1;
    }

    dag_write_header(fp, dag->config);

    fprintf(fp, "Frames %d\n", dag->nfrm);
    fprintf(fp, "#\n");

    for (i = 0, d = dag->list; d; d = d->alloc_next, i++);
    fprintf(fp,
            "Nodes %d (NODEID WORD STARTFRAME FIRST-ENDFRAME LAST-ENDFRAME)\n",
            i);
    for (i = 0, d = dag->list; d; d = d->alloc_next, i++) {
        d->seqid = i;
        fprintf(fp, "%d %s %d %d %d\n", i, dict_wordstr(dict, d->wid),
                d->sf, d->fef, d->lef);
    }


    fprintf(fp, "#\n");

    fprintf(fp, "Initial %d\nFinal %d\n", initial->seqid, final->seqid);

    /* Best score (i.e., regardless of Right Context) for word segments in word lattice */
    fprintf(fp, "BestSegAscr 0 (NODEID ENDFRAME ASCORE)\n");
    fprintf(fp, "#\n");

    fprintf(fp, "Edges (FROM-NODEID TO-NODEID ASCORE)\n");
    for (d = dag->list; d; d = d->alloc_next) {
        for (l = d->succlist; l; l = l->next) {
            /* Skip bypass edges */
            if (l->bypass)
                continue;
            fprintf(fp, "%d %d %d\n", d->seqid, l->node->seqid, l->ascr);
        }
    }
    fprintf(fp, "End\n");

    fclose_comp(fp, ispipe);

    return 0;
}

int32
dag_write_htk(dag_t *dag,
              const char *filename,
              const char *uttid,
              lm_t * lm,
              dict_t * dict)
{
    int32 i, n_nodes, n_links;
    dagnode_t *d;
    daglink_t *l;
    FILE *fp;
    float fps;
    int32 ispipe;

    E_INFO("Writing lattice file in HTK format: %s\n", filename);
    if ((fp = fopen_comp(filename, "w", &ispipe)) == NULL) {
        E_ERROR("fopen_comp (%s,w) failed\n", filename);
        return -1;
    }

    fprintf(fp, "# Lattice generated by Sphinx-III\n");
    dag_write_header(fp, dag->config);
    fprintf(fp, "VERSION=1.0\n");
    fprintf(fp, "UTTERANCE=%s\n", uttid);
    /* We could specify base= and tscale= here but I fear that
     * third-party tools might make assumptions about them, so we will
     * just convert to base e and seconds. */
    if (lm) {
        if (lm->name)
            fprintf(fp, "lmname=%s\n", lm->name);
        fprintf(fp, "lmscale=%f\n", cmd_ln_float32_r(dag->config, "-lw"));
        fprintf(fp, "wdpenalty=%f\n", cmd_ln_float32_r(dag->config, "-wip"));
    }

    n_nodes = n_links = 0;
    for (d = dag->list; d; d = d->alloc_next) {
        ++n_nodes;
        for (l = d->predlist; l; l = l->next) {
            /* Skip bypass links */
            if (l->bypass)
                continue;
            ++n_links;
        }
    }
    fprintf(fp, "N=%d\tL=%d\n", n_nodes + 1, n_links + 1);

    if (cmd_ln_exists_r(dag->config, "-frate"))
        fps = (float)cmd_ln_int32_r(dag->config, "-frate");
    else
        fps = 100.0;
    /* Extra node at the very end of the sentence. */
    fprintf(fp, "I=%-5d t=%-10.2f\n", 0, (float)dag->nfrm / fps);
    for (i = 1, d = dag->list; d; d = d->alloc_next, i++) {
        d->seqid = i;
        fprintf(fp, "I=%-5d t=%-10.2f\n", i, (float)d->sf / fps);
    }

    /* Add a </s> link to the end of the sentence. */
    fprintf(fp, "J=%-10d S=%-5d E=%-5d W=%-20s a=%-10.2f v=%-5d l=%-10.2f\n",
            0, dag->end->seqid, 0,
            dict_wordstr(dict, dag->end->wid),
            0.0, 1, 0.0);
    for (i = 1, d = dag->list; d; d = d->alloc_next) {
        for (l = d->predlist; l; l = l->next) {
            int32 nalt;
            s3wid_t b, a;

            /* Skip bypass links */
            if (l->bypass)
                continue;

            /* Find the pronunciation alternate index. */
            nalt = 1;
            a = b = dict_basewid(dict, l->node->wid);
            while (dict_nextalt(dict, a) != BAD_S3WID) {
                a = dict_nextalt(dict, a);
                ++nalt;
            }

            fprintf(fp, "J=%-10d S=%-5d E=%-5d W=%-20s a=%-10.2f v=%-5d l=%-10.2f\n",
                    i, l->node->seqid, d->seqid,
                    dict_wordstr(dict, b), logmath_log_to_ln(dag->logmath, l->ascr), nalt,
                    logmath_log_to_ln(dag->logmath, lm ? lm_rawscore(lm, l->lscr) : l->lscr));
            ++i;
        }
    }

    fclose_comp(fp, ispipe);
    return 0;
}

/**
 * Final global best path through DAG constructed from the word lattice.
 * Assumes that the DAG has already been constructed and is consistent with the word
 * lattice.
 * The search uses a recursive algorithm to find the best (reverse) path from the final
 * DAG node to the root:  The best path from any node (beginning with a particular link L)
 * depends on a similar best path for all links leaving the endpoint of L.  (This is
 * sufficient to handle trigram LMs.)
 */

srch_hyp_t *
dag_search(dag_t * dagp, char *utt, float64 lwf, dagnode_t * final,
           dict_t * dict, lm_t * lm, fillpen_t * fpen)
{
    daglink_t *l, *bestl;
    dagnode_t *d;
    int32 bestscore;
    srch_hyp_t *hyp;

    assert(dagp);
    assert(dagp->root);

    /* Find the backward link from the final DAG node that has the best path to root */
    bestscore = (int32) 0x80000000;
    bestl = NULL;

    /* Check that all edge scores are -ve; error if not so. */
    if (dag_chk_linkscr(dagp) < 0) {
        E_ERROR("Some edges are not negative\n");
        return NULL;
    }

    assert(final);
    assert(final->predlist);
    assert(dict);
    assert(lm);
    assert(fpen);

    /* Add a "stop" link to the entry node if none exists. */
    if (dagp->root->predlist == NULL)
        dag_link(dagp, NULL, dagp->root, 0, 0, -1, NULL);

    for (l = final->predlist; l; l = l->next) {
        d = l->node;
        if (!dict_filler_word(dict, d->wid)) {  /* Ignore filler node */
            if (dag_bestpath(dagp, l, final, lwf, dict, lm, lm->dict2lmwid) < 0) {      /* Best path to root beginning with l */
                E_ERROR("%s: Max LM ops (%d) exceeded\n", utt,
                        dagp->maxlmop);
                bestl = NULL;
                break;
            }

            if (l->pscr > bestscore) {
                bestscore = l->pscr;
                bestl = l;
            }
        }

    }

    /*
     * Remove the "stop" link but DO NOT free it yet!
     * DO NOT PANIC, the link will be freed in dag_destroy()
     */
    dagp->root->predlist = NULL;

    if (!bestl) {
        E_ERROR("Bestpath search failed for %s\n", utt);
        return NULL;
    }

    /*
     * At this point bestl is the best (reverse) link/path leaving the final node.  But
     * this does not include the acoustic score for the final node itself.  Add it.
     */
    l = &(dagp->final);
    l->history = bestl;
    l->pscr = bestl->pscr + l->ascr;
    l->ef = dagp->nfrm - 1;

    /* Backtrack through DAG for best path; but first free any old hypothesis */
    hyp = dag_backtrace(&hyp, l, lwf, dict, fpen);

    return (hyp);
}


void
dag_add_fudge_edges(dag_t * dagp, int32 fudge, int32 min_ef_range,
                    void *hist, dict_t * dict)
{
    dagnode_t *d, *pd;
    int32 l;
    latticehist_t *lathist;

    lathist = (latticehist_t *) hist;
    assert(dagp);

    if (fudge > 0 && !dagp->fudged) {
        /* Add "illegal" links that are near misses */
        for (d = dagp->list; d; d = d->alloc_next) {
            if (d->lef - d->fef < min_ef_range - 1)
                continue;

            /* As this part of the code will actually access 2 frames beyond. 
               This checking make sure spurious link will be removed. */

            if (d->sf >= lathist->n_frm - 3)
                continue;

            /* Links to d from nodes that first ended just when d started */
            for (l = lathist->frm_latstart[d->sf];
                 l < lathist->frm_latstart[d->sf + 1]; l++) {
                pd = lathist->lattice[l].dagnode;       /* Predecessor DAG node */

                if ((pd->wid != dict->finishwid) &&
                    (pd->fef == d->sf) &&
                    (pd->lef - pd->fef >= min_ef_range - 1)) {
                    dag_link(dagp, pd, d, lathist->lattice[l].ascr,
			     lathist->lattice[l].lscr, d->sf - 1,
			     NULL);
                }
            }

            if (fudge < 2)
                continue;

            /* Links to d from nodes that first ended just BEYOND when d started */
            for (l = lathist->frm_latstart[d->sf + 1];
                 l < lathist->frm_latstart[d->sf + 2]; l++) {
                pd = lathist->lattice[l].dagnode;       /* Predecessor DAG node */

                if ((pd->wid != dict->finishwid) &&
                    (pd->fef == d->sf + 1) &&
                    (pd->lef - pd->fef >= min_ef_range - 1)) {
                    dag_link(dagp, pd, d, lathist->lattice[l].ascr,
			     lathist->lattice[l].lscr, d->sf - 1,
			     NULL);
                }
            }
        }
        dagp->fudged = 1;
    }
}


/**
 * Add auxiliary links bypassing filler nodes in DAG.  In principle, a new such
 * auxiliary link can end up at ANOTHER filler node, and the process must be repeated
 * for complete transitive closure.  But removing fillers in the order in which they
 * appear in dag->list ensures that succeeding fillers have already been bypassed.
 * @return: 0 if successful; -1 if DAG maxedge limit exceeded.
 */

int32
dag_bypass_filler_nodes(dag_t * dag, float64 lwf, dict_t * dict,
                        fillpen_t * fpen)
{
    dagnode_t *d, *pnode, *snode;
    daglink_t *plink, *slink;
    int32 ascr = 0;

    assert(dag->list);

    /* Create additional links in DAG bypassing filler nodes */
    for (d = dag->list; d; d = d->alloc_next) {
        if (!dict_filler_word(dict, d->wid))    /* No need to bypass this node */
            continue;

        /* For each link TO d add a link to d's successors */
        for (plink = d->predlist; plink; plink = plink->next) {
            pnode = plink->node;

            ascr = plink->ascr;
            ascr += ((fillpen(fpen, dict_basewid(dict, d->wid))
                      - logs3(dag->logmath, fpen->wip)) * lwf
                     + logs3(dag->logmath, fpen->wip));

            /* Link this predecessor of d to successors of d */
            for (slink = d->succlist; slink; slink = slink->next) {
                snode = slink->node;

                /* Link only to non-filler successors; fillers have been bypassed */
                if (!dict_filler_word(dict, snode->wid))
                    if (dag_update_link
                        (dag, pnode, snode, ascr + slink->ascr, plink->ef, slink) < 0)
                        return -1;
            }
        }
    }

    return 0;
}

void
dag_remove_bypass_links(dag_t *dag)
{
    dagnode_t *d;
    daglink_t *l, *pl, *nl;

    for (d = dag->list; d; d = d->alloc_next) {
        pl = NULL;
        for (l = d->succlist; l; l = nl) {
            nl = l->next;
            if (l->bypass) {
                if (!pl)
                    d->succlist = nl;
                else
                    pl->next = nl;
                --dag->nbypass;
                listelem_free(dag->link_alloc, l);
            }
            else
                pl = l;
        }
        pl = NULL;
        for (l = d->predlist; l; l = nl) {
            nl = l->next;
            if (l->bypass) {
                if (!pl)
                    d->predlist = nl;
                else
                    pl->next = nl;
                listelem_free(dag->link_alloc, l);
            }
            else
                pl = l;
        }
    }
}

dag_t *
dag_load(char *file,          /**< Input: File to lod from */
           int32 maxedge,        /**< Maximum # of edges */
           float32 logbase,         /**< Logbase in float */
           int32 fudge,           /**< The number of fudges added */
           dict_t * dict,             /**< Dictionary */
           fillpen_t * fpen,          /**< Filler penalty structure */
           cmd_ln_t *config,
           logmath_t *logmath
    )
{

    FILE *fp;
    char line[16384], wd[1024];
    int32 sf, fef, lef, ef, lineno;
    int32 i, j, k, final, seqid, from, to, ascr;
    int32 node_ascr, node_lscr;
    int32 min_ef_range;
    dagnode_t *d, *pd, *tail, **darray;
    s3wid_t w;
    float32 lb, f32arg;
    int32 ispipe;
    dag_t *dag;
    latticehist_t *lathist = NULL;
    s3wid_t finishwid;
    int32 report;

    report = 0;
    lathist = NULL;
    dag = ckd_calloc(1, sizeof(dag_t));
    dag_init(dag, config, logmath);

    finishwid = dict_wordid(dict, S3_FINISH_WORD);

    dag->maxedge = maxedge;
    dag->list = NULL;
    dag->nlink = 0;
    dag->nbypass = 0;

    tail = NULL;
    darray = NULL;


    lineno = 0;

    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen_compchk(file, &ispipe)) == NULL) {
        E_ERROR("fopen_compchk(%s) failed\n", file);
        return NULL;
    }

    /* Read and verify logbase (ONE BIG HACK!!) */
    if (fgets(line, sizeof(line), fp) == NULL) {
        E_ERROR("Premature EOF(%s)\n", file);
        goto load_error;
    }
    if (strncmp(line, "# getcwd: ", 10) != 0) {
        E_ERROR("%s does not begin with '# getcwd: '\n", file);
        goto load_error;
    }
    if (fgets(line, sizeof(line), fp) == NULL) {
        E_ERROR("Premature EOF(%s)\n", file);
        goto load_error;
    }

    f32arg = cmd_ln_float32_r(dag->config, "-logbase");
    if ((strncmp(line, "# -logbase ", 11) != 0)
        || (sscanf(line + 11, "%f", &lb) != 1)) {
        E_WARN("%s: Cannot find -logbase in header\n", file);
        lb = f32arg;
    }
    else {

        if ((lb <= 1.0) || (lb > 2.0) || (f32arg <= 1.0) || (f32arg > 2.0))
            E_ERROR("%s: logbases out of range; cannot be verified\n",
                    file);
        else {
            int32 orig, this;
            float64 diff;

            orig = logs3(logmath, lb - 1.0);
            this = logs3(logmath, f32arg - 1.0);
            diff = ((orig - this) * 1000.0) / orig;
            if (diff < 0)
                diff = -diff;

            if (diff > 1.0)     /* Hack!! Hardwired tolerance limits on logbase */
                E_ERROR("%s: logbase inconsistent: %e\n", file, lb);
        }
    }


    /* Min. endframes value that a node must persist for it to be not ignored */
    min_ef_range = cmd_ln_int32_r(dag->config, "-min_endfr");

    /* Read Frames parameter */
    dag->nfrm = dag_param_read(fp, "Frames", &lineno);
    if (dag->nfrm <= 0) {
        E_ERROR("Frames parameter missing or invalid\n");
        goto load_error;
    }


    /* Read Nodes parameter */
    lineno = 0;
    dag->nnode = dag_param_read(fp, "Nodes", &lineno);
    if (dag->nnode <= 0) {
        E_ERROR("Nodes parameter missing or invalid\n");
        goto load_error;
    }

    /* Read nodes */
    darray = (dagnode_t **) ckd_calloc(dag->nnode, sizeof(dagnode_t *));
    for (i = 0; i < dag->nnode; i++) {

        report = 1;
        if (fgets(line, 1024, fp) == NULL) {
            E_ERROR("Premature EOF while loading Nodes(%s)\n", file);
            goto load_error;
        }
        lineno++;

        if ((k =
             sscanf(line, "%d %s %d %d %d", &seqid, wd, &sf, &fef,
                    &lef)) != 5) {
            E_ERROR("Cannot parse line: %s, value of count %d\n", line, k);
            goto load_error;
        }


        w = dict_wordid(dict, wd);
        if (NOT_S3WID(w)) {
            E_ERROR("Unknown word in line: %s\n", line);
            goto load_error;
        }

        if (seqid != i) {
            E_ERROR("Seqno error: %s\n", line);
            goto load_error;
        }

        d = listelem_malloc(dag->node_alloc);
        darray[i] = d;

        d->wid = w;
        d->seqid = seqid;
        d->sf = sf;
        d->fef = fef;
        d->lef = lef;
        d->reachable = 0;
        d->succlist = NULL;
        d->predlist = NULL;
        d->alloc_next = NULL;

        if ((k =
             sscanf(line, "%d %s %d %d %d %d %d", &seqid, wd, &sf, &fef,
                    &lef, &node_ascr, &node_lscr)) == 7) {
            if (!report)
                E_WARN
                    ("Acoustic score provided is provided in a word node, Only conversion to IBM lattice will show this behavior\n");

            d->node_ascr = node_ascr;
            d->node_lscr = node_lscr;
            report = 0;
        }
        else {
            d->node_ascr = d->node_lscr = 0;
        }

        if (!dag->list)
            dag->list = d;
        else
            tail->alloc_next = d;
        tail = d;
    }

    /* Read initial node ID */
    k = dag_param_read(fp, "Initial", &lineno);
    if ((k < 0) || (k >= dag->nnode)) {
        E_ERROR("Initial node parameter missing or invalid\n");
        goto load_error;
    }
    dag->entry.node = darray[k];
    dag->entry.ascr = 0;
    dag->entry.next = NULL;
    dag->entry.pscr_valid = 0;

    /* Read final node ID */
    k = dag_param_read(fp, "Final", &lineno);
    if ((k < 0) || (k >= dag->nnode)) {
        E_ERROR("Final node parameter missing or invalid\n");
        goto load_error;
    }
    dag->end = darray[k];
    dag->final.node = darray[k];
    dag->final.ascr = 0;
    dag->final.next = NULL;
    dag->final.pscr_valid = 0;
    dag->final.bypass = NULL;
    final = k;

    E_INFO("dag->nfrm+1, %d\n", dag->nfrm + 1);

    /* Read bestsegscore entries */
    if ((k = dag_param_read(fp, "BestSegAscr", &lineno)) < 0) {
        E_ERROR("BestSegAscr parameter missing\n");
        goto load_error;
    }

    if (k > 0) {
        lathist = latticehist_init(k, dag->nfrm + 1);

        j = -1;
        for (i = 0; i < k; i++) {
            if (fgets(line, 1024, fp) == NULL) {
                E_ERROR("Premature EOF while (%s) loading BestSegAscr\n",
                        line);
                goto load_error;
            }

            lineno++;
            if (sscanf(line, "%d %d %d", &seqid, &ef, &ascr) != 3) {
                E_ERROR("Cannot parse line: %s\n", line);
                goto load_error;
            }

            if ((seqid < 0) || (seqid >= dag->nnode)) {
                E_ERROR("Seqno error: %s\n", line);
                goto load_error;
            }

            if (ef != j) {
                for (j++; j <= ef; j++)
                    lathist->frm_latstart[j] = i;
                --j;
            }
            lathist->lattice[i].dagnode = darray[seqid];
            lathist->lattice[i].frm = ef;
            lathist->lattice[i].ascr = ascr;

            if ((seqid == final) && (ef == dag->final.node->lef))
                dag->final.ascr = ascr;
        }
        for (j++; j <= dag->nfrm; j++)
            lathist->frm_latstart[j] = k;
    }

    /* Read in edges */
    while (fgets(line, 1024, fp) != NULL) {
        lineno++;

        if (line[0] == '#')
            continue;
        if ((sscanf(line, "%s%d", wd, &k) == 1)
            && (strcmp(wd, "Edges") == 0))
            break;
    }
    k = 0;
    while (fgets(line, 1024, fp) != NULL) {
        lineno++;
        if (sscanf(line, "%d %d %d", &from, &to, &ascr) != 3)
            break;
        pd = darray[from];
        if (pd->wid == finishwid)
            continue;
        d = darray[to];

        if (dag_link(dag, pd, d, ascr, 0, d->sf - 1, NULL) < 0) {
            E_ERROR("%s: maxedge limit (%d) exceeded\n", file,
                    dag->maxedge);
            goto load_error;
        }
        k++;
    }
    if (strcmp(line, "End\n") != 0) {
        E_ERROR("Terminating 'End' missing\n");
        goto load_error;
    }

#if 0
    /* Build edges from lattice end-frame scores if no edges input */
    if (k == 0 && lathist) {
        E_INFO("No edges in dagfile; using lattice scores\n");
        for (d = dag->list; d; d = d->alloc_next) {
            if (d->sf == 0)
                assert(d->wid == dict->startwid);
            else if ((d == dag->final.node)
                     || (d->lef - d->fef >= min_ef_range - 1)) {
                /* Link from all end points == d->sf-1 to d */
                int32 l;
                for (l = lathist->frm_latstart[d->sf - 1];
                     l < lathist->frm_latstart[d->sf]; l++) {
                    pd = lathist->lattice[l].dagnode;   /* Predecessor DAG node */
                    if (pd == NULL) {
                        E_ERROR("Cannot determine DAG node!\n");
                        goto load_error;
                    }
                    if (pd->wid == finishwid)
                        continue;

                    if ((pd == dag->entry.node)
                        || (pd->lef - pd->fef >= min_ef_range - 1)) {
                        dag_link(dag, pd, d, lathist->lattice[l].ascr,
                                 0, d->sf - 1, NULL);
                        k++;
                    }
                }
            }
        }
    }
#endif
    dag->hook = NULL;

    /* Find initial node.  (BUG HERE: There may be > 1 initial node for multiple <s>) */
    for (d = dag->list; d; d = d->alloc_next) {
        if ((dict_basewid(dict, d->wid) == dict->startwid) && (d->sf == 0))
            break;
    }
    assert(d);
    dag->root = d;

    /*
     * Set limit on max LM ops allowed after which utterance is aborted.
     * Limit is lesser of absolute max and per frame max.
     */
    dag->maxlmop = cmd_ln_int32_r(dag->config, "-maxlmop");
    k = cmd_ln_int32_r(dag->config, "-maxlpf");
    k *= dag->nfrm;
    if (k > 0 && dag->maxlmop > k)
        dag->maxlmop = k;
    dag->lmop = 0;

    if (lathist)
        dag_add_fudge_edges(dag, fudge, min_ef_range, (void *) lathist, dict);


    fclose_comp(fp, ispipe);
    ckd_free(darray);
    if (lathist)
        latticehist_free(lathist);

    return dag;

  load_error:
    E_ERROR("Failed to load %s\n", file);
    if (fp)
        fclose_comp(fp, ispipe);
    if (darray)
        ckd_free(darray);
    if (lathist)
        latticehist_free(lathist);
    return NULL;
}
