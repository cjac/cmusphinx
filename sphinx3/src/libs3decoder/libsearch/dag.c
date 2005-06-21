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
 * $Log$
 * Revision 1.1  2005/06/21  22:37:47  arthchan2003
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

#include "dag.h"

void hyp_free (srch_hyp_t *list)
{
    srch_hyp_t *h;
    
    while (list) {
	h = list->next;
	listelem_free ((char *)list, sizeof(srch_hyp_t));
	list = h;
    }
}


/*
 * Link two DAG nodes with the given arguments
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
int32 dag_link (dag_t* dagp, dagnode_t *pd, dagnode_t *d, int32 ascr, int32 ef, daglink_t *byp)
{
    daglink_t *l;
    
    /* Link d into successor list for pd */
    if (pd) {	/* Special condition for root node which doesn't have a predecessor */
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->node = d;
	l->src = pd;
	l->bypass = byp;	/* This is a FORWARD link!! */
	l->ascr = ascr;
	l->pscr = (int32)0x80000000;
	l->pscr_valid = 0;
	l->history = NULL;
	l->ef = ef;
	l->next = pd->succlist;
	pd->succlist = l;
    }

    /* Link pd into predecessor list for d */
    l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
    l->node = pd;
    l->src = d;
    l->bypass = byp;		/* This is a FORWARD link!! */
    l->ascr = ascr;
    l->pscr = (int32)0x80000000;
    l->pscr_valid = 0;
    l->history = NULL;
    l->ef = ef;
    l->next = d->predlist;
    d->predlist = l;

    dagp->nlink++;

    return (dagp->nlink > dagp->maxedge) ? -1 : 0;
}


daglink_t *find_succlink (dagnode_t *src, dagnode_t *dst)
{
    daglink_t *l;

    for (l = src->succlist; l && (l->node != dst); l = l->next);
    return l;
}

daglink_t *find_predlink (dagnode_t *src, dagnode_t *dst)
{
    daglink_t *l;

    for (l = src->predlist; l && (l->node != dst); l = l->next);
    return l;
}


/*
 * Like dag_link but check if link already exists.  If so, replace if new score better.
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
int32 dag_update_link (dag_t* dagp, dagnode_t *pd, dagnode_t *d, int32 ascr,
			      int32 ef, daglink_t *byp)
{
    daglink_t *l, *r;
    l = find_succlink (pd, d);

    if (! l)
	return (dag_link (dagp, pd, d, ascr, ef, byp));

    if (l->ascr < ascr) {
	r = find_predlink (d, pd);

	assert (r && (r->ascr == l->ascr));
	l->ascr = r->ascr = ascr;
	l->ef = r->ef = ef;
	l->bypass = r->bypass = byp;
    }
    
    return 0;
}

/* Read parameter from a lattice file*/
int32 dag_param_read (FILE *fp, char *param, int32 *lineno)
{
    char line[1024], wd[1024];
    int32 n;
    
    while (fgets (line, 1024, fp) != NULL) {
	(*lineno)++;
	if (line[0] == '#')
	    continue;
	if ((sscanf (line, "%s %d", wd, &n) == 2) && (strcmp (wd, param) == 0))
	    return n;
    }
    return -1;
}


/*
 * Recursive step in dag_search:  best backward path from src to root beginning with l.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 dag_bestpath (
		    dag_t *dagp,         /* A pointer of the dag */
		    daglink_t *l,	/* Backward link! */
		    dagnode_t *src,	/* Source node for backward link l */
		    float64 lwf,         /* Language weight multiplication factor */ 
		    dict_t *dict,        /* The dictionary */
		    lm_t *lm,             /* The LM */
		    s3lmwid_t *dict2lmwid /* A map from dictionary id to lm id, should use wid2lm insteead*/
 		    )
{
    dagnode_t *d, *pd;
    daglink_t *pl;
    int32 lscr, score;
    
    assert (! l->pscr_valid);
    
    if ((d = l->node) == NULL) {
	/* If no destination at end of l, src is root node.  Recursion termination */
	assert (dict_basewid(dict,src->wid) == dict_wordid(dict,S3_START_WORD));
	l->lscr = 0;
	l->pscr = 0;
	l->pscr_valid = 1;
	l->history = NULL;
	
	return 0;
    }
    
    /* Search all predecessor links of l */
    for (pl = d->predlist; pl; pl = pl->next) {
	pd = pl->node;
	if (pd && dict_filler_word (dict,pd->wid))	/* Skip filler node */
	    continue;

	/* Evaluate best path along pl if not yet evaluated (recursive step) */
	if (! pl->pscr_valid)
	    if (dag_bestpath (dagp, pl, d, lwf,dict,lm,dict2lmwid) < 0)
		return -1;
	
	/* Accumulated path score along pl->l */
	if (pl->pscr > (int32)0x80000000) {
	    score = pl->pscr + l->ascr;
	    if (score > l->pscr) {	/* rkm: Added 20-Nov-1996 */
		if (pd)
		    lscr = lwf + lm_tg_score (lm,
					dict2lmwid[dict_basewid(dict,pd->wid)],
					dict2lmwid[dict_basewid(dict,d->wid)],
					dict2lmwid[dict_basewid(dict,src->wid)],
					src->wid);
		else
		    lscr = lwf + lm_bg_score (lm,
					dict2lmwid[dict_basewid(dict,d->wid)], 
					dict2lmwid[dict_basewid(dict,src->wid)],
					src->wid);
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
    }
    
#if 0
    printf ("%s,%d -> %s,%d = %d\n",
	    dict_wordstr (dict,d->wid), d->sf,
	    dict_wordstr (dict,src->wid), src->sf,
	    l->pscr);
#endif

    l->pscr_valid = 1;

    return 0;
}

int32 dag_chk_linkscr (dag_t *dagp)
{
    dagnode_t *d;
    daglink_t *l;
    
    for (d = dagp->list; d; d = d->alloc_next) {
	for (l = d->succlist; l; l = l->next) {
	  /*	  E_INFO("l->ascr %d\n",l->ascr); */

	  /* 20040909: I change this from >= to > because s3.5 sometimes generate lattice which has 0 as the beginning node of the lattice.  This should be regarded as temporary change*/
	  if (l->ascr > 0){
		return -1;
	  }
	}
    }

    return 0;
}

int32 dag_destroy ( dag_t *dagp )
{
    dagnode_t *d, *nd;
    daglink_t *l, *nl;
    
    for (d = dagp->list; d; d = nd) {
	nd = d->alloc_next;
	
	for (l = d->succlist; l; l = nl) {
	    nl = l->next;
	    listelem_free ((char *)l, sizeof(daglink_t));
	}
	for (l = d->predlist; l; l = nl) {
	    nl = l->next;
	    listelem_free ((char *)l, sizeof(daglink_t));
	}

	listelem_free ((char *)d, sizeof(dagnode_t));
    }

    dagp->list = NULL;

    return 0;
}

/**
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
srch_hyp_t *dag_backtrace (srch_hyp_t * hyp, daglink_t *l, float64 lwf, dict_t* dict, fillpen_t *fpen)
{
    srch_hyp_t *h, *hhead, *htail;
    int32 pscr;
    dagnode_t *src, *dst;
    daglink_t *bl, *hist;
    
    hyp = NULL;
    dst = NULL;
    for (; l; l = hist) {
	hist = l->history;
	
	if (hyp)
	    hyp->lscr = l->lscr;	/* As lscr actually applies to successor node */
	
	if (! l->node) {
	    assert (! l->history);
	    break;
	}
	
	if (! l->bypass) {
	    /* Link did not bypass any filler node */
	    h = (srch_hyp_t *) listelem_alloc (sizeof(srch_hyp_t));
	    h->wid = l->node->wid;
	    h->word = dict_wordstr (dict,h->wid);
	    h->sf = l->node->sf;
	    h->ef = l->ef;
	    h->ascr = l->ascr;
	    
	    h->next = hyp;
	    hyp = h;
	} else {
	    /* Link bypassed one or more filler nodes; restore bypassed link seq. */
	    hhead = htail = NULL;
	    
	    src = l->node;	/* Note that l is a PREDECESSOR link */
	    for (; l; l = l->bypass) {
		h = (srch_hyp_t *) listelem_alloc (sizeof(srch_hyp_t));
		h->wid = src->wid;
		h->word = dict_wordstr (dict,h->wid);
		h->sf = src->sf;

		if (hhead)
		    h->lscr = lwf * fillpen (fpen,dict_basewid (dict,src->wid));
		
		if (l->bypass) {
		    dst = l->bypass->src;
		    assert (dict_filler_word (dict,dst->wid));
		    bl = find_succlink (src, dst);
		    assert (bl);
		} else
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

	    htail->next = hyp;
	    hyp = hhead;
	}
    }

    /* Compute path score for each node in hypothesis */
    pscr = 0;
    for (h = hyp; h; h = h->next) {
	pscr = pscr + h->lscr + h->ascr;
	h->pscr = pscr;
    }
    
    return hyp;
}


void dag_init(dag_t* dagp){

  /* Initialize DAG structure */
  dagp->list = NULL;

  /* Set limit on max DAG edges allowed after which utterance is aborted */
  dagp->maxedge = cmd_ln_int32 ("-maxedge");

}
