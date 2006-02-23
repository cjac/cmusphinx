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
 * Revision 1.2  2006/02/23  05:22:32  arthchan2003
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

#include "dag.h"
#include "vithist.h"

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
 *
 * ARCHAN Comment: dag_link were used in decode_anytopo, dag and
 * astar. Therefore, the information is unfortunately duplicated.
 * l->bypass and l->is_filler_bypass is one of this example. Likely
 * only one of them need to be there.
 */
int32 dag_link (dag_t* dagp, dagnode_t *pd, dagnode_t *d, int32 ascr, int32 ef, daglink_t *byp)
{
    daglink_t *l;
    
    /* Link d into successor list for pd */
    if (pd) {	/* Special condition for root node which doesn't have a predecessor */
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->node = d;
	l->src = pd;
	l->ascr = ascr;
	l->pscr = (int32)0x80000000;
	l->pscr_valid = 0;
	l->history = NULL;
	l->ef = ef;
	l->next = pd->succlist;

	/* Effect caused by aggregating different stuctures */
	l->bypass = byp;	/* DAG-specific: This is a FORWARD link!! */
	l->is_filler_bypass = 0; /* Astar-specific */
	l->hook=NULL; /* Hopefully, this is the last argument we put into the dag_link structure */

	pd->succlist = l;
    }

    /* Link pd into predecessor list for d */
    l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
    l->node = pd;
    l->src = d;
    l->ascr = ascr;
    l->pscr = (int32)0x80000000;
    l->pscr_valid = 0;
    l->history = NULL;
    l->ef = ef;

    l->bypass = byp;	     /* DAG-specific: This is a FORWARD link!! */
    l->is_filler_bypass = 0; /* Astar-specific */

    l->hook=NULL; /* Hopefully, this is the last argument we put into the dag_link structure */

    l->next = d->predlist;
    d->predlist = l;

    dagp->nlink++;

    return (dagp->nlink > dagp->maxedge) ? -1 : 0;
}


/*
  Badly duplicate with dag_link; It also update the language score. 
 */
int32 dag_link_w_lscr (dag_t* dagp, dagnode_t *pd, dagnode_t *d, int32 ascr, int32 lscr, int32 ef, daglink_t *byp)
{
    daglink_t *l;
    
    /* Link d into successor list for pd */
    if (pd) {	/* Special condition for root node which doesn't have a predecessor */
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->node = d;
	l->src = pd;
	l->ascr = ascr;
	l->lscr = lscr;
	l->pscr = (int32)0x80000000;
	l->pscr_valid = 0;
	l->history = NULL;
	l->ef = ef;
	l->next = pd->succlist;

	/* Effect caused by aggregating different stuctures */
	l->bypass = byp;	/* DAG-specific: This is a FORWARD link!! */
	l->is_filler_bypass = 0; /* Astar-specific */
	l->hook=NULL; /* Hopefully, this is the last argument we put into the dag_link structure */

	pd->succlist = l;
    }

    /* Link pd into predecessor list for d */
    l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
    l->node = pd;
    l->src = d;
    l->ascr = ascr;
    l->lscr = lscr;
    l->pscr = (int32)0x80000000;
    l->pscr_valid = 0;
    l->history = NULL;
    l->ef = ef;

    l->bypass = byp;	     /* DAG-specific: This is a FORWARD link!! */
    l->is_filler_bypass = 0; /* Astar-specific */

    l->hook=NULL; /* Hopefully, this is the last argument we put into the dag_link structure */

    l->next = d->predlist;
    d->predlist = l;

    dagp->nlink++;

    return (dagp->nlink > dagp->maxedge) ? -1 : 0;
}


#if 0
static void daglinks_dump (dagnode_t *d)
{
    daglink_t *l;
    
    for (l = d->succlist; l; l = l->next)
	printf ("%6d %5d %12d %s\n", l->node->seqid, l->node->sf, l->ascr,
		dict_wordstr (dict, l->node->wid));
}
#endif


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
	/*	E_INFO("lwid1 %d, wid1 %d, lwid2 %d, wid2 %d\n",
	       dict2lmwid[dict_basewid(dict,d->wid)],
	       d->wid,
	       dict2lmwid[dict_basewid(dict,src->wid)],
	       src->wid);*/

	if (pl->pscr > (int32)0x80000000) {
	    score = pl->pscr + l->ascr;
	    if (score > l->pscr) {	/* rkm: Added 20-Nov-1996 */
		if (pd)
		    lscr = lwf * lm_tg_score (lm,
					dict2lmwid[dict_basewid(dict,pd->wid)],
					dict2lmwid[dict_basewid(dict,d->wid)],
					dict2lmwid[dict_basewid(dict,src->wid)],
					dict_basewid(dict, src->wid));
		else
		    lscr = lwf * lm_bg_score (lm,
					dict2lmwid[dict_basewid(dict,d->wid)], 
					dict2lmwid[dict_basewid(dict,src->wid)],
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
    }
    
#if 0
    printf ("%s,%d -> %s,%d = %d\n",
	    dict_wordstr (dict,dict_basewid(dict,d->wid)), d->sf,
	    dict_wordstr (dict,dict_basewid(dict,src->wid)), src->sf,
	    l->pscr);
    fflush(stdout);
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
srch_hyp_t *dag_backtrace (srch_hyp_t ** hyp, daglink_t *l, float64 lwf, dict_t* dict, fillpen_t *fpen)
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
	    (*hyp)->lscr = l->lscr;	/* As lscr actually applies to successor node */
	
	if (! l->node) {
	    assert (! l->history);
	    break;
	}
	
	if (! l->bypass) {
	    /* Link did not bypass any filler node */
	    h = (srch_hyp_t *) listelem_alloc (sizeof(srch_hyp_t));
	    h->id = l->node->wid;
	    h->word = dict_wordstr (dict,h->id);
	    h->sf = l->node->sf;
	    h->ef = l->ef;
	    h->ascr = l->ascr;
	    
	    h->next = *hyp;
	    (*hyp) = h;
	} else {
	    /* Link bypassed one or more filler nodes; restore bypassed link seq. */
	    hhead = htail = NULL;

	    src = l->node;	/* Note that l is a PREDECESSOR link */
	    for (; l; l = l->bypass) {
		h = (srch_hyp_t *) listelem_alloc (sizeof(srch_hyp_t));
		h->id = src->wid;
		h->word = dict_wordstr (dict,h->id);
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


void dag_init(dag_t* dagp){

  /* Initialize DAG structure */
  dagp->list = NULL;

  /* Set limit on max DAG edges allowed after which utterance is aborted */
  dagp->maxedge = cmd_ln_int32 ("-maxedge");

  dagp->filler_removed=0;
  dagp->fudged=0;
  dagp->hook=NULL;
}



void dag_write_header (FILE *fp,int32 nfr,int32 printminfr)
{
  char str[1024];
  getcwd (str, sizeof(str));
  fprintf (fp, "# getcwd: %s\n", str);
	  
  /* Print logbase first!!  Other programs look for it early in the
   * DAG */

  fprintf (fp, "# -logbase %e\n", cmd_ln_float32 ("-logbase"));
  
  fprintf (fp, "# -dict %s\n", cmd_ln_str ("-dict"));
  if (cmd_ln_str ("-fdict"))
    fprintf (fp, "# -fdict %s\n", cmd_ln_str ("-fdict"));
  fprintf (fp, "# -lm %s\n", cmd_ln_str ("-lm"));
  fprintf (fp, "# -mdef %s\n", cmd_ln_str ("-mdef"));
  fprintf (fp, "# -senmgau %s\n", cmd_ln_str ("-senmgau"));
  fprintf (fp, "# -mean %s\n", cmd_ln_str ("-mean"));
  fprintf (fp, "# -var %s\n", cmd_ln_str ("-var"));
  fprintf (fp, "# -mixw %s\n", cmd_ln_str ("-mixw"));
  fprintf (fp, "# -tmat %s\n", cmd_ln_str ("-tmat"));
  if(printminfr){
    fprintf (fp, "# -min_endfr %d\n", cmd_ln_int32 ("-min_endfr"));
  }
  fprintf (fp, "#\n");
	  
  fprintf (fp, "Frames %d\n", nfr);
  fprintf (fp, "#\n");

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

srch_hyp_t *dag_search (dag_t *dagp, char *utt, float64 lwf, dagnode_t *final, dict_t *dict, lm_t *lm, fillpen_t *fpen)
{
    daglink_t *l, *bestl;
    dagnode_t *d;
    int32 bestscore;
    srch_hyp_t *hyp;

    /* Find the backward link from the final DAG node that has the best path to root */
    bestscore = (int32) 0x80000000;
    bestl = NULL;

    /* Check that all edge scores are -ve; error if not so. */
    if (dag_chk_linkscr (dagp) < 0){
      E_ERROR("Some edges are not negative\n");
      return NULL;
    }

    assert(final);
    assert(final->predlist);
    assert(dict);
    assert(lm);
    assert(fpen);
    assert(dagp);

    for (l = final->predlist; l; l = l->next) {
	d = l->node;
	if (! dict_filler_word (dict,d->wid)) {	/* Ignore filler node */
	    if(dag_bestpath (dagp, l, final, lwf,dict,lm,lm->dict2lmwid)<0){ /* Best path to root beginning with l */
		E_ERROR("%s: Max LM ops (%d) exceeded\n", utt, dagp->maxlmop);
		bestl = NULL;
		break;
	    }

	    if (l->pscr > bestscore) {
		bestscore = l->pscr;
		bestl = l;
	    }
	}
	
    }

    if (! bestl){
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
    l->ef = dagp->nfrm -1 ;
    
    /* Backtrack through DAG for best path; but first free any old hypothesis */
    hyp = dag_backtrace (&hyp, l, lwf, dict, fpen);

    return (hyp);
}


void dag_add_fudge_edges (dag_t *dagp, int32 fudge, int32 min_ef_range, void *hist, dict_t *dict)
{
    dagnode_t *d, *pd;
    int32 l;
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
		dag_link_w_lscr (dagp, pd, d, lathist->lattice[l].ascr, lathist->lattice[l].lscr, d->sf-1, NULL);
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
		dag_link_w_lscr (dagp, pd, d, lathist->lattice[l].ascr, lathist->lattice[l].lscr, d->sf-1, NULL);
	    }
	}
      }
      dagp->fudged=1;
    }
}


/*
 * Remove filler nodes from DAG by replacing each link TO a filler with links
 * to its successors.  In principle, successors can be fillers and the process
 * must be repeated.  But removing fillers in the order in which they appear in
 * dag.list ensures that succeeding fillers have already been eliminated.
 * Return value: 0 if successful; -1 if DAG maxedge limit exceeded.
 */

int32 dag_remove_filler_nodes ( dag_t* dagp, float64 lwf, dict_t *dict, fillpen_t *fpen )
{
    dagnode_t *d, *pnode, *snode;
    daglink_t *plink, *slink;
    int32 ascr=0;
    
    assert(dagp->list);

    for (d = dagp->list; d; d = d->alloc_next) {

	if (! dict_filler_word (dict, d->wid))
	    continue;

	/* Replace each link TO d with links to d's successors */
	for (plink = d->predlist; plink; plink = plink->next) {
	    pnode = plink->node;

	    ascr = plink->ascr; 
	    ascr += lwf * fillpen(fpen,dict_basewid (dict,d->wid));
	    
	    /* Link this predecessor of d to successors of d */
	    for (slink = d->succlist; slink; slink = slink->next) {
		snode = slink->node;
		/* Link only to non-filler successors; fillers have been eliminated */
		if (! dict_filler_word (dict, snode->wid)) {
		    /* Update because a link may already exist */
		    if (dag_update_link (dagp,pnode, snode,
					 ascr + slink->ascr, plink->ef, slink) < 0)
			return -1;
		}
	    }
	}
    }
    return 0;
}


dag_t* dag_load (  
		char *file,   /**< Input: File to lod from */
		int32 maxedge, /**< Maximum # of edges */
		float32 logbase,  /**< Logbase in float */
		int32 fudge,    /**< The number of fudges added */
		dict_t *dict,       /**< Dictionary */
		fillpen_t *fpen    /**< Filler penalty structure */
		)
{

    FILE *fp;
    char line[16384], wd[1024];
    int32  sf, fef, lef, ef, lineno;
    int32 i, j, k, final, seqid, from, to, ascr;
    int32 node_ascr, node_lscr;
    int32  min_ef_range;
    dagnode_t *d, *pd, *tail, **darray;
    s3wid_t w;
    float32 lb, f32arg;
    int32 ispipe;
    dag_t* dag;
    latticehist_t *lathist;
    s3wid_t finishwid;
    int32 report;
    
    report=0;
    lathist=NULL;
    dag=ckd_calloc(1,sizeof(dag_t));

    finishwid = dict_wordid (dict,S3_FINISH_WORD);

    dag->maxedge = maxedge;
    dag->list = NULL;
    dag->nlink = 0;
    dag->nbypass =0;
    
    tail = NULL;
    darray = NULL;


    lineno = 0;
    
    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen_compchk (file, &ispipe)) == NULL) {
	E_ERROR("fopen_compchk(%s) failed\n", file);
 	return NULL;
    }
    
    /* Read and verify logbase (ONE BIG HACK!!) */
    if (fgets (line, sizeof(line), fp) == NULL) {
	E_ERROR ("Premature EOF(%s)\n", file);
	goto load_error;
    }
    if (strncmp (line, "# getcwd: ", 10) != 0) {
	E_ERROR ("%s does not begin with '# getcwd: '\n", file);
	goto load_error;
    }
    if (fgets (line, sizeof(line), fp) == NULL) {
	E_ERROR ("Premature EOF(%s)\n", file);
	goto load_error;
    }

    f32arg = *((float32 *) cmd_ln_access ("-logbase"));
    if ((strncmp (line, "# -logbase ", 11) != 0) || (sscanf (line+11, "%f", &lb) != 1)) {
	E_WARN ("%s: Cannot find -logbase in header\n", file);
	lb= f32arg;
    } else {

	if ((lb <= 1.0) || (lb > 2.0) || (f32arg <= 1.0) || (f32arg > 2.0))
	    E_ERROR ("%s: logbases out of range; cannot be verified\n", file);
	else {
	    int32 orig, this;
	    float64 diff;
	    
	    orig = logs3 (lb - 1.0);
	    this = logs3 (f32arg - 1.0);
	    diff = ((orig - this) * 1000.0) / orig;
	    if (diff < 0)
		diff = -diff;
	    
	    if (diff > 1.0)		/* Hack!! Hardwired tolerance limits on logbase */
		E_ERROR ("%s: logbase inconsistent: %e\n", file, lb);
	}
    }
    

    /* Min. endframes value that a node must persist for it to be not ignored */
    min_ef_range = *((int32 *) cmd_ln_access ("-min_endfr"));
    
    /* Read Frames parameter */
    dag->nfrm = dag_param_read (fp, "Frames", &lineno);
    if (dag->nfrm <= 0) {
	E_ERROR("Frames parameter missing or invalid\n");
	goto load_error;
    }

    
    /* Read Nodes parameter */
    lineno = 0;
    dag->nnode = dag_param_read (fp, "Nodes", &lineno);
    if (dag->nnode <= 0) {
	E_ERROR("Nodes parameter missing or invalid\n");
	goto load_error;
    }
    
    /* Read nodes */
    darray = (dagnode_t **) ckd_calloc (dag->nnode, sizeof(dagnode_t *));
    for (i = 0; i < dag->nnode; i++) {

      report =1;
	if (fgets (line, 1024, fp) == NULL) {
	    E_ERROR ("Premature EOF while loading Nodes(%s)\n", file);
	    goto load_error;
	}
	lineno++;
	
	if ((k = sscanf (line, "%d %s %d %d %d", &seqid, wd, &sf, &fef, &lef)) != 5) {
	    E_ERROR("Cannot parse line: %s, value of count %d\n", line,k);
	    goto load_error;
	}


	w = dict_wordid (dict, wd);
	if (NOT_S3WID(w)) {
	    E_ERROR("Unknown word in line: %s\n", line);
	    goto load_error;
	}
	
	if (seqid != i) {
	    E_ERROR("Seqno error: %s\n", line);
	    goto load_error;
	}
	
	d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));
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

	if ((k = sscanf (line,  "%d %s %d %d %d %d %d", &seqid, wd, &sf, &fef, &lef, &node_ascr, &node_lscr)) == 7){
	  if(!report)
	    E_WARN("Acoustic score provided is provided in a word node, Only conversion to IBM lattice will show this behavior\n");

	  d->node_ascr=node_ascr;
	  d->node_lscr=node_lscr;
	  report=0;
	}
	
	if (! dag->list)
	    dag->list = d;
	else
	    tail->alloc_next = d;
	tail = d;
    }

    /* Read initial node ID */
    k = dag_param_read (fp, "Initial", &lineno);
    if ((k < 0) || (k >= dag->nnode)) {
	E_ERROR("Initial node parameter missing or invalid\n");
	goto load_error;
    }
    dag->entry.node = darray[k];
    dag->entry.ascr = 0;
    dag->entry.next = NULL;
    dag->entry.pscr_valid = 0;
    
    /* Read final node ID */
    k = dag_param_read (fp, "Final", &lineno);
    if ((k < 0) || (k >= dag->nnode)) {
	E_ERROR("Final node parameter missing or invalid\n");
	goto load_error;
    }
    dag->final.node = darray[k];
    dag->final.ascr = 0;
    dag->final.next = NULL;
    dag->final.pscr_valid = 0;
    dag->final.bypass = NULL;
    final = k;

    /* Read bestsegscore entries */
    if ((k = dag_param_read (fp, "BestSegAscr", &lineno)) < 0) {
	E_ERROR("BestSegAscr parameter missing\n");
	goto load_error;
    }

    lathist=latticehist_init(k,dag->nfrm+1);
    
    j = -1;
    for (i = 0; i < k; i++) {
	if (fgets (line, 1024, fp) == NULL) {
	  E_ERROR("Premature EOF while (%s) loading BestSegAscr\n", line);
	    goto load_error;
	}
	
	lineno++;
	if (sscanf (line, "%d %d %d", &seqid, &ef, &ascr) != 3) {
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
	lathist->lattice[i].ef = ef;
	lathist->lattice[i].ascr = ascr;
	
	if ((seqid == final) && (ef == dag->final.node->lef))
	    dag->final.ascr = ascr;
    }
    for (j++; j <= dag->nfrm; j++)
	lathist->frm_latstart[j] = k;
    
    /* Read in edges */
    while (fgets (line, 1024, fp) != NULL) {
	lineno++;

	if (line[0] == '#')
	    continue;
	if ((sscanf (line, "%s%d", wd,&k) == 1) && (strcmp (wd, "Edges") == 0))
	    break;
    }
    k = 0;
    while (fgets (line, 1024, fp) != NULL) {
	lineno++;
	if (sscanf (line, "%d %d %d", &from, &to, &ascr) != 3)
	    break;
	pd = darray[from];
	if (pd->wid == finishwid)
	    continue;
	d = darray[to];

	/* Skip short-lived nodes */
	if ((pd == dag->entry.node) || (d == dag->final.node) ||
	    ((d->lef - d->fef >= min_ef_range-1) && (pd->lef - pd->fef >= min_ef_range-1))) {
	    if (dag_link (dag,pd, d, ascr, d->sf-1, NULL) < 0) {
		E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, dag->maxedge);
		goto load_error;
	    }
	    
	    k++;
	}
    }
    if (strcmp (line, "End\n") != 0) {
	E_ERROR("Terminating 'End' missing\n");
	goto load_error;
    }

#if 0
    /* Build edges from lattice end-frame scores if no edges input */
    if (k == 0) {
	E_INFO("No edges in dagfile; using lattice scores\n");
	for (d = dag->list; d; d = d->alloc_next) {
	    if (d->sf == 0)
		assert (d->wid == dict->startwid);
	    else if ((d == dag->final.node) || (d->lef - d->fef >= min_ef_range-1)) {
		/* Link from all end points == d->sf-1 to d */
		for (l = lathist->frm_latstart[d->sf-1]; l < lathist->frm_latstart[d->sf]; l++) {
		    pd = lathist->lattice[l].dagnode;		/* Predecessor DAG node */
		    if (pd->wid == finishwid)
			continue;

		    if ((pd == dag->entry.node) || (pd->lef - pd->fef >= min_ef_range-1)) {
			dag_link (&dag,pd, d, lathist->lattice[l].ascr, d->sf-1, NULL);
			k++;
		    }
		}
	    }
	}
    }
#endif
    dag->hook=NULL;

    /* Find initial node.  (BUG HERE: There may be > 1 initial node for multiple <s>) */
    for (d = dag->list; d; d = d->alloc_next) {
	if ((dict_basewid(dict,d->wid) == dict->startwid) && (d->sf == 0))
	    break;
    }
    assert (d);
    dag->root = d;


    dag_add_fudge_edges (dag, 
			 fudge,
			 min_ef_range, 
			 (void*) lathist, dict);


    fclose_comp (fp, ispipe);
    ckd_free (darray);
    if(lathist)
      latticehist_free(lathist);

    return dag;

load_error:
    E_ERROR("Failed to load %s\n", file);
    if (fp)
	fclose_comp (fp, ispipe);
    if (darray)
	ckd_free (darray);
    if (lathist)
      latticehist_free(lathist);
    return NULL;

}

int32 s3dag_dag_load (dag_t **dagpp, float32 lwf, char *file, dict_t* dict, fillpen_t *fpen)
{

    int32 k;

    *dagpp=dag_load(file,
		 cmd_ln_int32("-maxedge"),
		 cmd_ln_float32("-logbase"),
		 cmd_ln_int32("-dagfudge"),
		 dict,
		 fpen);
    
    assert(*dagpp);
    /*
     * HACK!! Change dag.final.node wid to finishwid if some other filler word,
     * to avoid complications with LM scores at this point.
     */
    (*dagpp)->orig_exitwid = (*dagpp)->final.node->wid;
    if (dict_filler_word(dict, (*dagpp)->final.node->wid))
	(*dagpp)->final.node->wid = dict->finishwid;
    

    /* Add links bypassing filler nodes */
    if (dag_remove_filler_nodes ((*dagpp),
				 lwf,
				 dict, fpen) < 0) {
	E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, (*dagpp)->maxedge);
	return -1;
    }else
      (*dagpp)->filler_removed=1;

    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link ((*dagpp),NULL, (*dagpp)->entry.node, 0, -1, NULL);
    
    E_INFO("%5d frames, %6d nodes, %8d edges\n", (*dagpp)->nfrm, (*dagpp)->nnode, (*dagpp)->nlink);
    
    /*
     * Set limit on max LM ops allowed after which utterance is aborted.
     * Limit is lesser of absolute max and per frame max.
     */
    (*dagpp)->maxlmop = cmd_ln_int32 ("-maxlmop");
    k = cmd_ln_int32 ("-maxlpf");
    
    k *= (*dagpp)->nfrm;
    if ((*dagpp)->maxlmop > k)
	(*dagpp)->maxlmop = k;
    (*dagpp)->lmop = 0;
    
    return (*dagpp)->nfrm;
}


/**
 * Build array of candidate words that start around the current frame (cf).
 * Note: filler words are not in this list since they are always searched (see
 * word_trans).
 */
void build_word_cand_cf (int32 cf,
			 dict_t *dict,
			 s3wid_t* wcand_cf,
			 int32 word_cand_win,
			 word_cand_t ** wcand
			 )
{
    int32 f, sf, ef;
    s3wid_t w, n;
    word_cand_t *candp;
    
    for (w = 0; w < dict->n_word; w++)
	wcand_cf[w] = 0;
    
    if ((sf = cf - word_cand_win) < 0)
	sf = 0;
    if ((ef = cf + word_cand_win) >= S3_MAX_FRAMES)
	ef = S3_MAX_FRAMES-1;
    
    for (f = sf; f <= ef; f++) {
	for (candp = wcand[f]; candp; candp = candp->next)
	    wcand_cf[candp->wid] = 1;
    }

    wcand_cf[dict->startwid] = 0;	/* Never hypothesized (except at beginning) */
    for (w = dict->filler_start; w <= dict->filler_end; w++)
	wcand_cf[w] = 0;	/* Handled separately */
    wcand_cf[dict->finishwid] = 1;	/* Always a candidate */

    n = 0;
    for (w = 0; w < dict->n_word; w++)
	if (wcand_cf[w])
	    wcand_cf[n++] = w;
    wcand_cf[n] = BAD_S3WID;
}


int32 word_cand_load (FILE *fp, word_cand_t** wcand,dict_t *dict, char *uttid)
{
    char line[1024], word[1024];
    int32 i, k, n, nn, sf, seqno, lineno;
    s3wid_t w;
    word_cand_t *candp;
    
    /* Skip past Nodes parameter */
    lineno = 0;
    nn = 0;
    word[0] = '\0';
    while (fgets (line, sizeof(line), fp) != NULL) {
	lineno++;
	if ((sscanf (line, "%s %d", word, &nn) == 2) && (strcmp (word, "Nodes") == 0))
	    break;
    }
    if ((strcmp (word, "Nodes") != 0) || (nn <= 0)) {
	E_ERROR("%s: Nodes parameter missing from input lattice\n", uttid);
	return -1;
    }

    n = 0;
    for (i = 0; i < nn; i++) {
	if (fgets (line, 1024, fp) == NULL) {
	    E_ERROR("%s: Incomplete input lattice\n", uttid);
	    return -1;
	}
	lineno++;

	if ((k = sscanf (line, "%d %s %d", &seqno, word, &sf)) != 3) {
	    E_ERROR("%s: Error in lattice, line %d: %s\n", uttid, lineno, line);
	    return -1;
	}
	if (seqno != i) {
	    E_ERROR("%s: Seq# error in lattice, line %d: %s\n", uttid, lineno, line);
	    return -1;
	}
	if ((sf < 0) || (sf >= S3_MAX_FRAMES)) {
	    E_ERROR("%s: Startframe error in lattice, line %d: %s\n", uttid, lineno, line);
	    return -1;
	}
	
	w = dict_wordid (dict,word);
	if (NOT_S3WID(w)) {
	    E_ERROR("%s: Unknown word in lattice: %s; ignored\n", uttid, word);
	    continue;
	}
	w = dict_basewid(dict,w);
	
	/* Check node not already present; avoid duplicates */
	for (candp = wcand[sf]; candp && (candp->wid != w); candp = candp->next);
	if (candp)
	    continue;
	
	candp = (word_cand_t *) listelem_alloc (sizeof(word_cand_t));
	candp->wid = w;
	candp->next = wcand[sf];
	wcand[sf] = candp;

	n++;
    }
    
    return n;
}

void word_cand_free ( word_cand_t ** wcand )
{
    word_cand_t *candp, *next;
    int32 f;
    
    for (f = 0; f < S3_MAX_FRAMES; f++) {
	for (candp = wcand[f]; candp; candp = next) {
	    next = candp->next;
	    listelem_free ((char *)candp, sizeof(word_cand_t));
	}

	wcand[f] = NULL;
    }

}


