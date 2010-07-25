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
 * word_graph.c -- Library for word graph a linked-based DAG. 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * HISTORY
 * 
 * $Log$
 * Revision 1.3  2006/02/28  02:06:46  egouvea
 * Updated MS Visual C++ 6.0 support files. Fixed things that didn't
 * compile in Visual C++ (declarations didn't match, etc). There are
 * still some warnings, so this is not final. Also, sorted files in
 * several Makefile.am.
 * 
 * Revision 1.2  2006/02/23 05:15:12  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Word graphs with word attached to links. Now mainly used in conversion from Sphinx to IBM format
 *
 * Revision 1.1.2.2  2006/01/16 20:17:43  arthchan2003
 * Add flags whether scale to be computed. fixed the arguments in lm_rawscore.
 *
 * Revision 1.1.2.1  2005/11/17 06:39:30  arthchan2003
 * Added a structure for linked-based lattice.
 *
 */



#include "s3types.h"
#include "dag.h"
#include "word_graph.h"
#include "srch_output.h"
#include "corpus.h"

void
word_graph_dump(char *dir, char *uttfile, char *id, char *latfile_ext, dag_t * dag,
                dict_t * dict, lm_t * lm, int32 * senscale)
{
    word_graph_t *wg;
    FILE *fp;
    int32 ispipe;
    char filename[2048];

    wg = dag_to_wordgraph(dag, senscale, lm, dict);

    ctl_outfile(filename, dir, latfile_ext, uttfile, id, TRUE);

    E_INFO("Writing lattice file for IBM format: %s\n", filename);

    if ((fp = fopen_comp(filename, "w", &ispipe)) == NULL) {
        E_ERROR("fopen_comp (%s,w) failed\n", filename);
    }

    print_wg(fp, wg, dict, IBM_LATTICE_FORMAT);

    fclose_comp(fp, ispipe);

    if (wg)
        wordgraph_free(wg);
}

/*
  Insert links into a word graph
  
  @return 0-based index of link
 */
static int32
new_word_graph_link(word_graph_t * wg,              /**< The word graph */
                    int32 snodeid,                  /**< Start node ID */
                    int32 enodeid,                  /**< End node ID */
                    int32 sf,                       /**< Start frame */
                    int32 ef,                       /**< Ending frame */
                    int32 wid,                      /**< Word ID */
                    int32 ascr,                     /**< Acoustic score */
                    int32 lscr,                     /**< Language score */
                    int32 cscr,                      /**< Confidence score */
                    int32 * senscale,                 /**< Array of senscale*/
                    cmd_ln_t *config,
                    logmath_t *logmath
    )
{
    word_graph_link_t *wgl;
    word_graph_node_t *wgn;
    gnode_t *gn;
    int32 i;
    int32 scl;
    i = 0;
    scl = 0;
    wgl = (word_graph_link_t *) ckd_calloc(1, sizeof(word_graph_link_t));
    wgl->srcidx = snodeid;
    wgl->tgtidx = enodeid;

    wgl->wid = wid;
#if 0
    E_INFO("In new word_graph_link\n");
    E_INFO("sf %d ef %d\n", sf, ef);
#endif
    assert(sf <= ef);

    if (cmd_ln_int32_r(config, "-hypsegscore_unscale"))
        scl = compute_scale(sf, ef, senscale);

#if 0
    E_INFO("snodeid %d enodeid %d, sf %d, ef %d, ascr %d scl %d\n",
           snodeid, enodeid, sf, ef, ascr, scl);
#endif
    wgl->ascr = logmath_log_to_ln(logmath, ascr + scl);
    wgl->lscr = logmath_log_to_ln(logmath, lscr);
    wgl->cscr = logmath_log_to_ln(logmath, cscr);

    wgl->linkidx = wg->n_link;

    wg->link = glist_add_ptr(wg->link, (void *) wgl);
    wg->n_link++;
    /*Find the node */
    for (gn = wg->node; gn; gn = gnode_next(gn)) {
        wgn = (word_graph_node_t *) gnode_ptr(gn);
        /*    E_INFO("wgn->nodeidx %d wgl->srcidx %d\n",wgn->nodeidx,wgl->srcidx); */
        if (wgn->nodeidx == wgl->srcidx) {
      /** Found the node, add the target node to the children list */

            wgn->child_node_list =
                glist_add_int32(wgn->child_node_list, wgl->tgtidx);
            break;
        }
    }

    return wg->n_link - 1;

}


/*
  Insert nodes into a word graph
  @return 0-based index of node
*/

static int32
new_word_graph_node(word_graph_t * wg, int32 time)
{
    word_graph_node_t *wgn;
    wgn = (word_graph_node_t *) ckd_calloc(1, sizeof(word_graph_node_t));
    wgn->time = time;

    wgn->nodeidx = wg->n_node;
    wg->node = glist_add_ptr(wg->node, (void *) wgn);

    wg->n_node++;
    return wg->n_node - 1;
}


/**
 * Print the word graph. 
 * 
 */
void
print_wg(FILE * fp, word_graph_t * wg, dict_t * dict, int32 fmt)
{
    gnode_t *gn;
    glist_t reverse_link;
    glist_t reverse_node;

    word_graph_link_t *l;
    word_graph_node_t *n;

    if (fmt == 0) {
        fprintf(fp, "Number of link %d\n", wg->n_link);
        fprintf(fp, "Number of node %d\n", wg->n_node);

        fprintf(fp, "Link Info\n");
        for (gn = wg->link; gn; gn = gnode_next(gn)) {
            l = (word_graph_link_t *) gnode_ptr(gn);
            fprintf(fp,
                    "srcidx %d, tgtidx %d wid %d, ascr %2.4f, lscr %2.4f, cscr %2.4f\n",
                    l->srcidx, l->tgtidx, l->wid, l->ascr, l->lscr,
                    l->cscr);
        }

        fprintf(fp, "Node Info\n");
        for (gn = wg->node; gn; gn = gnode_next(gn)) {
            n = (word_graph_node_t *) gnode_ptr(gn);
            fprintf(fp, "nodeidx %d time %d\n", n->nodeidx, n->time);
        }

    }
    else if (fmt == 1) {

        /* IBM file format.
           0 1 a=-55.0643,l=0  <s>(01)         6.71989
           1 7 a=-56.3294,l=-8.255  Hi(01)          25.8821
           1 6 a=-56.3294,l=-10.2115  </s>(01)       30.3871
           1 5 a=-62.5804,l=0         There(01)     7.63712
           бн
           7631
           7645
           бн
           0 t=32
           1 t=838
         */
        reverse_link = glist_reverse(wg->link);
        /*
           Iterate all arcs first 
         */
        for (gn = reverse_link; gn; gn = gnode_next(gn)) {
            l = (word_graph_link_t *) gnode_ptr(gn);


            fprintf(fp, "%d %d a=%f, l=%f ", l->srcidx, l->tgtidx, l->ascr,
                    l->lscr);

            if (dict_basewid(dict, l->wid) == l->wid)
                fprintf(fp, "%s(01)", dict_wordstr(dict, l->wid));
            else {
                int32 iddiff;
                iddiff = l->wid - dict_basewid(dict, l->wid);
                if (iddiff < 10)
                    fprintf(fp, "%s(0%d)",
                            dict_wordstr(dict, dict_basewid(dict, l->wid)),
                            iddiff);
                else if (iddiff < 100) {
                    fprintf(fp, "%s(%d)",
                            dict_wordstr(dict, dict_basewid(dict, l->wid)),
                            iddiff);
                }
                else {
                    fprintf(fp, "%s(99)",
                            dict_wordstr(dict,
                                         dict_basewid(dict, l->wid)));
                    E_ERROR
                        ("Only able to handle 99 pronounciations variants\n");
                }
            }
            fprintf(fp, " ");

            fprintf(fp, "%f\n", l->cscr);
        }



        reverse_node = glist_reverse(wg->node);

        /*
           Dump all nodes with no outgoing arcs. (ARCHAN: Don't ask me why,
           this is IBM format. Well, great IBM format.)
         */

        for (gn = reverse_node; gn; gn = gnode_next(gn)) {
            n = (word_graph_node_t *) gnode_ptr(gn);
            if (glist_count(n->child_node_list) == 0) {
                fprintf(fp, "%d\n", n->nodeidx);
            }
        }

        /*
           Iterate all nodes. 
         */
        for (gn = reverse_node; gn; gn = gnode_next(gn)) {
            n = (word_graph_node_t *) gnode_ptr(gn);
            fprintf(fp, "%d, t=%d\n", n->nodeidx, n->time);
        }


    }
    else {
        E_ERROR("Unknown file format %d\n", fmt);
    }

}

/*
     Pseudocode of build_wg_from_dag(word_graph,dag,dag_node, dnode_stidx)
      1, Mark dag_node
      2, For all dag_children
           If there is a new start-time
               newnode->nodeidx=NewNode(children->sf);
               NewArc(dag_node->nodeidx, newnode->nodeidx, dag_node->wid, dag_link->ascr, dag_link->lscr)
               build_wg_from_dag(word_graph,dag,children,newnode->nodeidx)
     
      Node: dagnode->reachable is used as the marker 
*/


static void
wg_from_dag(word_graph_t * wg, dag_t * dag, dagnode_t * d,
            int32 dnode_stidx, int32 * senscale, lm_t * lm, dict_t * dict)
{
    daglink_t *l;
    dagnode_t *n;
    int32 i, nodeidx;
    word_graph_node_t *stfr;
    int32 numout = 0;

    stfr =
        (word_graph_node_t *) ckd_calloc(dag->nfrm,
                                         sizeof(word_graph_node_t));

    i = nodeidx = 0;
    /* 1, Mark d */
    dag_node_mark(d) = 1;


    for (i = 0; i < dag->nfrm; i++) {
        stfr[i].time = INVALID_START_FRAME;
        stfr[i].nodeidx = INVALID_START_INDEX;
    }

    /*2, Get all dag_children, find how many start-time need to be allocated 
       Also assert that the start-time of children is in between fef and lef
     */
    for (l = d->succlist; l; l = l->next) {
        numout++;
        n = l->node;

        /*    FIXME! Should it be disabled?
           assert (d->fef <= n->sf && d->lef+1 >= n->sf);
         */
        for (i = 0;
             i < dag->nfrm && stfr[i].time != n->sf
             && stfr[i].time != INVALID_START_FRAME; i++);

        if (stfr[i].time == INVALID_START_FRAME) {

            stfr[i].time = n->sf;
            nodeidx = new_word_graph_node(wg, stfr[i].time);

            stfr[i].nodeidx = nodeidx;
            /* FIX ME ! Currently, confidence score is hard-wired to 0.0 */
            new_word_graph_link(wg, dnode_stidx, nodeidx, d->sf, n->sf,
                                d->wid, l->ascr, lm_rawscore(lm, l->lscr),
                                0.0, senscale, dag->config, dag->logmath);
        }
    }

    /* If numout is zero, this is the ending segments. 
       Allocate the node for ending time. (last ending frame)
     */
    if (numout == 0) {
        nodeidx = new_word_graph_node(wg, d->lef);

        /* FIX ME ! Currently, confidence score is hard-wired to 0.0 */
        new_word_graph_link(wg, dnode_stidx, nodeidx, d->sf, d->lef,
                            d->wid, d->node_ascr, d->node_lscr, 0.0,
                            senscale, dag->config, dag->logmath);
#if 0
        if (dict_filler_word(dict, d->wid)) {
            new_word_graph_link(wg, dnode_stidx, nodeidx, d->sf, d->lef,
                                d->wid, 0.0, cmd_ln_float32_r(dag->config, "-fillprob"),
                                0.0, senscale, dag->config, dag->logmath);

        }
        else {
            new_word_graph_link(wg, dnode_stidx, nodeidx, d->sf, d->lef,
                                d->wid, 0.0, lm_rawscore(lm,
                                                         lm_ug_score(lm,
                                                                     lm->
                                                                     dict2lmwid
                                                                     [d->
                                                                      wid],
                                                                     d->
                                                                     wid)),
                                0.0, senscale, dag->config, dag->logmath);
        }
#endif
    }

#if 0                           /* Operationally show the start time */
    for (i = 0; stfr[i] != INVALID_START_FRAME; i++) {
        E_INFO("Start-time for next node %d\n", stfr[i]);
    }
#endif

    for (l = d->succlist; l; l = l->next) {
        n = l->node;
        for (i = 0;
             i < dag->nfrm && stfr[i].time != n->sf
             && stfr[i].time != INVALID_START_FRAME; i++);
        assert(stfr[i].time != INVALID_START_FRAME);
        if (dag_node_mark(n) == 0)
            wg_from_dag(wg, dag, n, stfr[i].nodeidx, senscale, lm, dict);
    }

    ckd_free(stfr);

}

/**
 * Build a linked-based DAG from the lattice: 
 *
 * The building process is similar to dag_build.  dag_build
 * essentially form a graph of segments where the links actually give
 * the acoustic score for the begin segments only. Also dag_build
 * would "compress" segments by only store the first ef and last ef
 * given the start frame
 *
 */
word_graph_t *
dag_to_wordgraph(dag_t * dag, int32 * senscale, lm_t * _lm, dict_t * _dict)
{
    dagnode_t *d;
    word_graph_t *wg;
    int32 nodeidx;

    wg = (word_graph_t *) ckd_calloc(1, sizeof(word_graph_t));
    wg->n_node = 0;
    wg->n_link = 0;

    d = dag->root;
    nodeidx = new_word_graph_node(wg, d->sf);
    if (nodeidx != 0)
        E_ERROR("The first node allocated doesn't has index 0!\n");

    /*  E_INFO("d->sf %d, d->ef %d, d->wid %d\n",d->sf,d->fef,d->wid); */


    wg_from_dag(wg, dag, d, nodeidx, senscale, _lm, _dict);


    return wg;
}

void
wordgraph_free(word_graph_t * wg)
{
    gnode_t *gn;
    word_graph_link_t *l;
    word_graph_node_t *n;

    if (wg != NULL) {

        for (gn = wg->link; gn; gn = gnode_next(gn)) {
            l = (word_graph_link_t *) gnode_ptr(gn);
            ckd_free(l);
        }
        glist_free(wg->link);

        for (gn = wg->node; gn; gn = gnode_next(gn)) {
            n = (word_graph_node_t *) gnode_ptr(gn);
            glist_free(n->child_node_list);
            ckd_free(n);
        }
        glist_free(wg->node);
    }
    ckd_free(wg);
}
