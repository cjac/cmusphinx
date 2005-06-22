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
 * astar.c -- A* DAG search to create N-best lists
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
 * Revision 1.8  2005/06/22  05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 * 
 * Revision 1.8  2005/06/19 03:58:17  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.7  2005/06/18 21:16:36  archan
 * Fixed a bug in astar, introduced another two tests to test on functionality of class-based LM in dag and astar.
 *
 * Revision 1.6  2005/06/18 03:23:13  archan
 * Change to lmset interface.
 *
 * Revision 1.5  2005/06/03 06:45:30  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.4  2005/06/03 05:46:42  archan
 * Log. Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
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
 * Revision 1.3  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.2  2005/03/30 00:43:41  archan
 *
 * 27-Feb-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check in building DAG for avoiding cycles with dagfudge.
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice and nbest files.
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxedge argument to control memory usage.
 * 		Added -maxlmop and -maxlpf options to control execution time.
 * 		Added -maxppath option to control CPU/memory usage.
 * 
 * 18-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added reporting of acoustic and LM scores for each word in each hyp.
 * 		Changed LM scores in nbest files to be unscaled (i.e., without any
 * 		language weight or insertion penalty.
 * 
 * 09-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added code in dag_link_bypass to update an existing link, if any,
 * 		instead of adding several bypass links between the same two nodes.
 * 		This reduces CPU and memory requirements considerably.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 * 
 * 28-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started, copying from nbest.c.
 */

/** \file s3_astar.c
 * \brief engine for s3.0 astar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3types.h>
#include "s3_dag.h"
#include <mdef.h>
#include <tmat.h>
#include <dict.h>
#include <lm.h>
#include <fillpen.h>
#include <search.h>
#include <logs3.h>
#include <dag.h>




/****************************************
 * Globals!   This is a hack!		*
 ****************************************/

dict_t *dict;		/* The dictionary */

#if 0
lm_t *lm;		/* The Language model */
s3lmwid_t *dict2lmwid;	/* Mapping from decoding dictionary wid's to lm ones.  They may not be the same! */
#endif

static int32 beam;

lmset_t* lmset;           /* The language model set */

fillpen_t *fpen;	/* The filler penalty structure. */

dag_t dag;              /* The dag used by main_astar.c */

static int32 maxlmop;		/* Max LM ops allowed before utterance aborted */
static int32 lmop;		/* #LM ops actually made */
static int32 maxedge;		/* Max #edges in DAG allowed before utterance aborted */


/**
 * Link two DAG nodes with the given arguments
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
static int32 astar_dag_link (dagnode_t *pd, dagnode_t *d, int32 ascr)
{
    daglink_t *l;
    
    /* Link d into successor list for pd */
    l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
    l->node = d;
    l->ascr = ascr;
    l->is_filler_bypass = 0;
    l->next = pd->succlist;
    pd->succlist = l;

    l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
    l->node = pd;
    l->ascr = ascr;
    l->is_filler_bypass = 0;
    l->next = d->predlist;
    d->predlist = l;

    dag.nlink++;

    return (dag.nlink > maxedge) ? -1 : 0;
}


/**
 * Add a filler-bypass link between the two nodes or update an existing bypass link if
 * the new score is better.
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
static int32 astar_dag_link_bypass (dagnode_t *pd, dagnode_t *d, int32 ascr)
{
    daglink_t *l;
    
    /* Find the existing bypass link, if any, between pd and d */
    for (l = pd->succlist; l && ((! l->is_filler_bypass) || (l->node != d)); l = l->next);

    if (! l) {
	/* No existing bypass link; create one from pd to d */
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->node = d;
	l->ascr = ascr;
	l->is_filler_bypass = 1;
	l->next = pd->succlist;
	pd->succlist = l;
	
	dag.nlink++;
	dag.nbypass++;
    } else if (l->ascr < ascr) {
	/* Link pd -> d exists; update link score for it */
	l->ascr = ascr;
    }

    return (dag.nlink > maxedge) ? -1 : 0;
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


/**
 * Mark every node that has a path to the argument dagnode as "reachable".
 */
static void dag_mark_reachable (dagnode_t *d)
{
    daglink_t *l;
    
    d->reachable = 1;
    for (l = d->predlist; l; l = l->next)
	if (! l->node->reachable)
	    dag_mark_reachable (l->node);
}


static void dag_remove_unreachable ( void )
{
    dagnode_t *d;
    daglink_t *l, *pl, *nl;
    
    for (d = dag.list; d; d = d->alloc_next) {
	if (! d->reachable) {
	    /* Remove successor node links */
	    for (l = d->succlist; l; l = nl) {
		nl = l->next;
		listelem_free ((char *) l, sizeof(daglink_t));
	    }
	    d->succlist = NULL;

	    /* Remove predecessor links */
	    for (l = d->predlist; l; l = nl) {
		nl = l->next;
		listelem_free ((char *) l, sizeof(daglink_t));
	    }
	    d->predlist = NULL;
	} else {
	    /* Remove successor links to unreachable nodes; predecessors are reachable */
	    pl = NULL;
	    for (l = d->succlist; l; l = nl) {
		nl = l->next;
		if (! l->node->reachable) {
		    if (! pl)
			d->succlist = nl;
		    else
			pl->next = nl;
		    listelem_free ((char *) l, sizeof(daglink_t));
		} else
		    pl = l;
	    }
	}
    }
}


/**
 * Add auxiliary links bypassing filler nodes in DAG.  In principle, a new such
 * auxiliary link can end up at ANOTHER filler node, and the process must be repeated
 * for complete transitive closure.  But removing fillers in the order in which they
 * appear in dag.list ensures that succeeding fillers have already been bypassed.
 * (See comment before s3astar_dag_load.)
 * Return value: 0 if successful; -1 if DAG maxedge limit exceeded.
 */
static int32 dag_bypass_filler_nodes ( void )
{
    dagnode_t *d, *pnode, *snode;
    daglink_t *plink, *slink;
    int32 scr;
    
    /* Create additional links in DAG bypassing filler nodes */
    for (d = dag.list; d; d = d->alloc_next) {
	if (! dict_filler_word (dict,d->wid))	/* No need to bypass this node */
	    continue;
	
	/* For each link TO d add a link to d's successors */
	for (plink = d->predlist; plink; plink = plink->next) {
	    pnode = plink->node;
	    scr = plink->ascr + fillpen(fpen, dict_basewid (dict, d->wid));
	    
	    /* Link this predecessor of d to successors of d */
	    for (slink = d->succlist; slink; slink = slink->next) {
		snode = slink->node;

		/* Link only to non-filler successors; fillers have been bypassed */
		if (! dict_filler_word (dict,snode->wid))
		    if (astar_dag_link_bypass (pnode, snode, scr + slink->ascr) < 0)
			return -1;
	    }
	}
    }

    return 0;
}


/**
 * For each link compute the heuristic score (hscr) from the END of the link to the
 * end of the utterance; i.e. the best score from the end of the link to the dag
 * exit node.
 */
static void dag_compute_hscr ( void )
{
    dagnode_t *d, *d1, *d2;
    daglink_t *l1, *l2;
    s3wid_t bw0, bw1, bw2;
    int32 hscr, best_hscr;
    lm_t* lm;

    lm=lmset->cur_lm;

    for (d = dag.list; d; d = d->alloc_next) {
	bw0 = dict_filler_word (dict, d->wid) ? BAD_S3WID : dict_basewid (dict, d->wid);

	/* For each link from d, compute heuristic score */
	for (l1 = d->succlist; l1; l1 = l1->next) {
	    assert (l1->node->reachable);

	    d1 = l1->node;
	    if (d1 == dag.exit.node)
		l1->hscr = 0;
	    else {
		bw1 = dict_filler_word (dict, d1->wid) ? BAD_S3WID : dict_basewid (dict, d1->wid);
		if (NOT_S3WID(bw1)) {
		    bw1 = bw0;
		    bw0 = BAD_S3WID;
		}
		
		best_hscr = (int32)0x80000000;
		for (l2 = d1->succlist; l2; l2 = l2->next) {
		    d2 = l2->node;
		    if (dict_filler_word (dict, d2->wid))
			continue;
		    
		    bw2 = dict_basewid (dict, d2->wid);
		    hscr = l2->hscr + l2->ascr + lm_tg_score (lm, lm->dict2lmwid[bw0], lm->dict2lmwid[bw1], lm->dict2lmwid[bw2], bw2);
		    
		    if (hscr > best_hscr)
			best_hscr = hscr;
		}

		l1->hscr = best_hscr;
	    }
	}
    }
}


static void dag_remove_bypass_links ( void )
{
    dagnode_t *d;
    daglink_t *l, *pl, *nl;
    
    for (d = dag.list; d; d = d->alloc_next) {
	pl = NULL;
	for (l = d->succlist; l; l = nl) {
	    nl = l->next;
	    if (l->is_filler_bypass) {
		if (! pl)
		    d->succlist = nl;
		else
		    pl->next = nl;
		listelem_free ((char *) l, sizeof(daglink_t));
	    } else
		pl = l;
	}
    }
}


/**
 * Load a DAG from a file: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * dagnodes_list = linear list of DAG nodes allocated, ordered such that nodes earlier
 * in the list can follow nodes later in the list, but not vice versa:  Let two DAG
 * nodes d1 and d2 have start times sf1 and sf2, and end time ranges [fef1..lef1] and
 * [fef2..lef2] respectively.  If d1 appears later than d2 in dag.list, then
 * fef2 >= fef1, because d2 showed up later in the word lattice.  If there is a DAG
 * edge from d1 to d2, then sf1 > fef2.  But fef2 >= fef1, so sf1 > fef1.  Reductio ad
 * absurdum.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 s3astar_dag_load (char *file)
{
    FILE *fp;
    char line[16384], wd[1024];
    int32 nfrm, nnode, sf, fef, lef, ef, lineno;
    int32 i, j, k, l, final, seqid, from, to, ascr;
    int32 fudge, min_ef_range;
    dagnode_t *d, *pd, *tail, **darray;
    s3wid_t w;
    struct lat_s {
	dagnode_t *node;
	int32 ef;
	int32 ascr;
    } *lat;		/* Lattice (bptable) entries in each frame */
    int32 *frm2lat;	/* frm2lat[f] = first lattice entry for frame f */
    float32 lb, f32arg;
    int32 ispipe;
    
    /* Set limit on max DAG edges allowed after which utterance is aborted */
    maxedge = *((int32 *) cmd_ln_access ("-maxedge"));
    
    dag.list = NULL;
    dag.nlink = 0;
    dag.nbypass = 0;
    
    tail = NULL;
    darray = NULL;
    lat = NULL;
    frm2lat = NULL;
    lineno = 0;
    
    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen_compchk (file, &ispipe)) == NULL) {
	E_ERROR("fopen_compchk (%s) failed\n", file);
	return -1;
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
	lb = f32arg;
    }
    
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
    
    /* Min. endframes value that a node must persist for it to be not ignored */
    min_ef_range = *((int32 *) cmd_ln_access ("-min_endfr"));
    
    /* Read Frames parameter */
    nfrm = dag_param_read (fp, "Frames", &lineno);
    if (nfrm <= 0) {
	E_ERROR("Frames parameter missing or invalid\n");
	goto load_error;
    }
    dag.nfrm = nfrm;
    
    /* Read Nodes parameter */
    lineno = 0;
    nnode = dag_param_read (fp, "Nodes", &lineno);
    if (nnode <= 0) {
	E_ERROR("Nodes parameter missing or invalid\n");
	goto load_error;
    }

    /* Read nodes */
    darray = (dagnode_t **) ckd_calloc (nnode, sizeof(dagnode_t *));
    for (i = 0; i < nnode; i++) {
	if (fgets (line, 1024, fp) == NULL) {
	    E_ERROR ("Premature EOF(%s) while loading Nodes\n", file);
	    goto load_error;
	}

	lineno++;
	
	if ((k = sscanf (line, "%d %s %d %d %d", &seqid, wd, &sf, &fef, &lef)) != 5) {
	    E_ERROR("Cannot parse line: %s\n", line);
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
	
	if (! dag.list)
	    dag.list = d;
	else
	    tail->alloc_next = d;
	tail = d;
    }

    /* Read initial node ID */
    k = dag_param_read (fp, "Initial", &lineno);
    if ((k < 0) || (k >= nnode)) {
	E_ERROR("Initial node parameter missing or invalid\n");
	goto load_error;
    }
    dag.entry.node = darray[k];
    dag.entry.ascr = 0;
    dag.entry.next = NULL;
    dag.entry.pscr_valid = 0;

    /* Read final node ID */
    k = dag_param_read (fp, "Final", &lineno);
    if ((k < 0) || (k >= nnode)) {
	E_ERROR("Final node parameter missing or invalid\n");
	goto load_error;
    }
    dag.exit.node = darray[k];
    dag.exit.ascr = 0;
    dag.exit.next = NULL;
    dag.exit.pscr_valid = 0;
    dag.exit.bypass = NULL;
    final = k;

    /* Read bestsegscore entries */
    if ((k = dag_param_read (fp, "BestSegAscr", &lineno)) < 0) {
	E_ERROR("BestSegAscr parameter missing\n");
	goto load_error;
    }
    lat = (struct lat_s *) ckd_calloc (k, sizeof(struct lat_s));
    frm2lat = (int32 *) ckd_calloc (nfrm+1, sizeof(int32));
    
    j = -1;
    for (i = 0; i < k; i++) {
	if (fgets (line, 1024, fp) == NULL) {
	    E_ERROR ("Premature EOF(%s) while loading BestSegAscr\n", file);
	    goto load_error;
	}
	
	lineno++;

	if (sscanf (line, "%d %d %d", &seqid, &ef, &ascr) != 3) {
	    E_ERROR("Cannot parse line: %s\n", line);
	    goto load_error;
	}
	
	if ((seqid < 0) || (seqid >= nnode)) {
	    E_ERROR("Seqno error: %s\n", line);
	    goto load_error;
	}
	
	if (ef != j) {
	    for (j++; j <= ef; j++)
		frm2lat[j] = i;
	    --j;
	}
	lat[i].node = darray[seqid];
	lat[i].ef = ef;
	lat[i].ascr = ascr;
	
	if ((seqid == final) && (ef == dag.exit.node->lef))
	    dag.exit.ascr = ascr;
    }
    for (j++; j <= nfrm; j++)
	frm2lat[j] = k;
    
    /* Read in edges */
    while (fgets (line, 1024, fp) != NULL) {
	lineno++;

	if (line[0] == '#')
	    continue;
	if ((sscanf (line, "%s%d", wd, &k) == 1) && (strcmp (wd, "Edges") == 0))
	    break;
    }
    k = 0;
    while (fgets (line, 1024, fp) != NULL) {
	lineno++;
	if (sscanf (line, "%d %d %d", &from, &to, &ascr) != 3)
	    break;
	pd = darray[from];
	if (pd->wid == dict->finishwid)
	    continue;
	d = darray[to];

	/* Skip short-lived nodes */
	if ((pd == dag.entry.node) || (d == dag.exit.node) ||
	    ((d->lef - d->fef >= min_ef_range-1) && (pd->lef - pd->fef >= min_ef_range-1))) {
	    if (astar_dag_link (pd, d, ascr) < 0) {
		E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, maxedge);
		goto load_error;
	    }
	    
	    k++;
	}
    }
    if (strcmp (line, "End\n") != 0) {
	E_ERROR("Terminating End missing\n");
	goto load_error;
    }
    
#if 0
    /* Build edges from lattice end-frame scores if no edges input */
    if (k == 0) {
	E_INFO("No edges in dagfile; using lattice scores\n");
	for (d = dag.list; d; d = d->alloc_next) {
	    if (d->sf == 0)
		assert (d->wid == dict->startwid);
	    else if ((d == dag.exit.node) || (d->lef - d->fef >= min_ef_range-1)) {
		/* Link from all end points == d->sf-1 to d */
		for (l = frm2lat[d->sf-1]; l < frm2lat[d->sf]; l++) {
		    pd = lat[l].node;		/* Predecessor DAG node */
		    if (pd->wid == dict->finishwid)
			continue;

		    if ((pd == dag.entry.node) || (pd->lef - pd->fef >= min_ef_range-1)) {
			astar_dag_link (pd, d, lat[l].ascr);
			k++;
		    }
		}
	    }
	}
    }
#endif

    fudge = *((int32 *) cmd_ln_access ("-dagfudge"));
    if (fudge > 0) {
	/* Add "illegal" links that are near misses */
	for (d = dag.list; d; d = d->alloc_next) {
	    if (d->lef - d->fef < min_ef_range-1)
		continue;
	    
	    /* Links to d from nodes that first ended just when d started */
	    for (l = frm2lat[d->sf]; l < frm2lat[d->sf+1]; l++) {
		pd = lat[l].node;		/* Predecessor DAG node */
		if ((pd->wid != dict->finishwid) && (pd->fef == d->sf) &&
		    (pd->sf < d->sf) &&
		    (pd->lef - pd->fef >= min_ef_range-1)) {
		    astar_dag_link (pd, d, lat[l].ascr);
		    k++;
		}
	    }
	    
	    if (fudge < 2)
		continue;
	    
	    /* Links to d from nodes that first ended just BEYOND when d started */
	    for (l = frm2lat[d->sf+1]; l < frm2lat[d->sf+2]; l++) {
		pd = lat[l].node;		/* Predecessor DAG node */
		if ((pd->wid != dict->finishwid) && (pd->fef == d->sf+1) &&
		    (pd->sf < d->sf) &&
		    (pd->lef - pd->fef >= min_ef_range-1)) {
		    astar_dag_link (pd, d, lat[l].ascr);
		    k++;
		}
	    }
	}
    }
    
    fclose_comp (fp, ispipe);
    ckd_free (darray);
    ckd_free (lat);
    ckd_free (frm2lat);
    
    /*
     * HACK!! Change dag.exit.node wid to finishwid if some other filler word,
     * to avoid complications with LM scores at this point.
     */
    dag.orig_exitwid = dag.exit.node->wid;
    if (dict_filler_word(dict, dag.exit.node->wid))
	dag.exit.node->wid = dict->finishwid;
    
    /*
     * Mark nodes from which final exit node is reachable.  Add links bypassing filler
     * nodes, and compute heuristic score (hscr) from each node to end of utterance.
     */
    dag_mark_reachable (dag.exit.node);
    dag_remove_unreachable ();
    if (dag_bypass_filler_nodes () < 0) {
	E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, maxedge);
	return -1;
    }
    
    dag_compute_hscr ();
    dag_remove_bypass_links ();

    E_INFO("%5d frames, %6d nodes, %8d edges, %8d bypass\n",
	   nfrm, nnode, dag.nlink, dag.nbypass);
    
    return dag.nfrm;

load_error:
    E_ERROR("Failed to load %s\n", file);
    if (fp)
	fclose_comp (fp, ispipe);
    if (darray)
	ckd_free (darray);
    if (lat)
	ckd_free (lat);
    if (frm2lat)
	ckd_free (frm2lat);
    return -1;
}



/* ---------------------------- NBEST CODE ---------------------------- */

/**
 * A node along each partial path.  Partial paths form a tree structure rooted at the
 * start node.
 */
typedef struct ppath_s {
    struct ppath_s *hist;	/** Immediately previous ppath node; NULL if none */
    struct ppath_s *lmhist;	/** Previous LM (non-filler) ppath node; NULL if none */
    dagnode_t *dagnode;		/** Dagnode (word,startfrm) represented by ppath node */
    int32 lscr;		/** LM score for this node given past history */
    int32 pscr;		/** Path score (from initial node) ending at this node startfrm */
    int32 tscr;		/** pscr + heuristic score (this node startfrm -> end of utt) */
    uint32 histhash;	/** Hash value of complete history, for spotting duplicates */
    int32 pruned;	/** If superseded by another with same history and better score */
    struct ppath_s *hashnext;	/** Next node with same hashmod value */
    struct ppath_s *next;	/** Links all allocated nodes for reclamation at the end;
				   NULL if last in list */
} ppath_t;
static ppath_t *ppath_list;	/** Complete list of allocated ppath nodes */
static int32 n_ppath;		/** #Partial paths allocated (to control memory usage) */
static int32 maxppath;		/** Max partial paths allowed before aborting */

/** Heap (sorted) partial path nodes */
typedef struct heap_s {
    ppath_t *ppath;	/** Partial path node */
    int32 nl, nr;	/** #left/right descendants from this node (for balancing tree) */
    struct heap_s *left;	/** Root of left descendant heap */
    struct heap_s *right;	/** Root of right descendant heap */
} aheap_t;
static aheap_t *heap_root;

/**
 * For tracking ppath nodes with identical histories.  For rapid location of duplicates
 * keep a separate list of nodes for each (ppath_t.histhash % HISTHASH_MOD) value.
 * Two paths have identical histories (are duplicates) iff:
 * 	1. Their tail nodes have the same dagnode (same <wid,sf> value), and
 * 	2. Their LM histories are identical.
 */
#define HISTHASH_MOD	200003	/* A prime */
static ppath_t **hash_list;	/* A separate list for each hashmod value (see above) */


#if 0
static void heap_dump (aheap_t *top, int32 level)
{
    int32 i;
    
    if (! top)
	return;
    
    for (i = 0; i < level; i++)
	printf ("  ");
    
    printf ("%s %d %d %d %d\n", dict_wordstr(dict, top->nb->dagnode->wid),
	    top->nb->dagnode->sf, top->nb->dagnode->fef, top->nb->dagnode->lef,
	    top->nb->score);
    heap_dump (top->left, level+1);
    heap_dump (top->right, level+1);
}
#endif


/**
 * Insert the given new ppath node in sorted (sub)heap rooted at the given heap node
 * Heap is sorted by tscr (i.e., total path score + heuristic score to end of utt).
 * Return the root of the new, updated (sub)heap.
 */
static aheap_t *aheap_insert (aheap_t *root, ppath_t *new)
{
    aheap_t *h;
    ppath_t *pp;
    
    if (! root) {
	h = (aheap_t *) listelem_alloc (sizeof(aheap_t));
	h->ppath = new;
	h->left = h->right = NULL;
	h->nl = h->nr = 0;
	return h;
    }

    /* Root already exists; if new node better, replace root with it */
    pp = root->ppath;
    if (pp->tscr < new->tscr) {
	root->ppath = new;
	new = pp;
    }

    /* Insert new or old (replaced) node in right or left subtree; keep them balanced */
    if (root->nl > root->nr) {
	root->right = aheap_insert (root->right, new);
	root->nr++;
    } else {
	root->left = aheap_insert (root->left, new);
	root->nl++;
    }

    return root;
}


/**
 * Pop the top element off the heap and return root of adjust heap.
 * Root must be non-NULL when this function to be called.
 */
static aheap_t *aheap_pop (aheap_t *root)
{
    aheap_t *l, *r;

    /* Propagate best value from below into root, if any */
    l = root->left;
    r = root->right;
    if (! l) {
	if (! r) {
	    listelem_free ((char *) root, sizeof(aheap_t));
	    return NULL;
	} else {
	    root->ppath = r->ppath;
	    root->right = aheap_pop (r);
	    root->nr--;
	}
    } else {
	if ((! r) || (l->ppath->tscr >= r->ppath->tscr)) {
	    root->ppath = l->ppath;
	    root->left = aheap_pop (l);
	    root->nl--;
	} else {
	    root->ppath = r->ppath;
	    root->right = aheap_pop (r);
	    root->nr--;
	}
    }

    return root;
}


/**
 * Check if pplist already contains a better (better pscr) path identical to the
 * extension of lmhist by node.  Return 1 if true, 0 if false.  Also, if false, but
 * an inferior path did exist, mark it as pruned.
 */
static int32 ppath_dup (ppath_t *hlist, ppath_t *lmhist, dagnode_t *node,
			uint32 hval, int32 pscr)
{
    ppath_t *h1, *h2;
    
    /* Compare each entry in hlist to new, proposed path */
    for (; hlist; hlist = hlist->hashnext) {
	if ((hlist->dagnode != node) || (hlist->histhash != hval))
	    continue;
	
	for (h1 = hlist->lmhist, h2 = lmhist; h1 && h2; h1 = h1->lmhist, h2 = h2->lmhist) {
	    if ((h1 == h2) ||	/* Histories converged; identical */
		(dict_basewid (dict, h1->dagnode->wid) != dict_basewid (dict, h2->dagnode->wid)))
		break;
	}

	if (h1 == h2) {	    /* Identical history already exists */
	    if (hlist->pscr >= pscr)	/* Existing history is superior */
		return 1;
	    else {
		/*
		 * New path is better; prune existing one.  There may be other paths
		 * in the list as well, but all of them must have been pruned by
		 * hlist or others later in the list!
		 */
		hlist->pruned = 1;
		return 0;
	    }
	}
    }

    return 0;
}


/**
 * Create a new ppath node for dagnode reached from top via link l.  Assign the
 * proper scores for the new, extended path and insert it in the sorted heap of
 * ppath nodes.  But first check if it's a duplicate of an existing ppath but has an
 * inferior score, in which case do not insert.
 */
static void ppath_insert (ppath_t *top, daglink_t *l, int32 pscr, int32 tscr, int32 lscr)
{
    ppath_t *pp, *lmhist;
    uint32 h, hmod;
    s3wid_t w;
    
    /* Extend path score; Add acoustic and LM scores for link */
    pscr = top->pscr + l->ascr + lscr;
    
    /*
     * Check if extended path would be a duplicate one with an inferior score.
     * First, find hash value for new node.
     */
    lmhist = dict_filler_word(dict, top->dagnode->wid) ? top->lmhist : top;
    w = lmhist->dagnode->wid;
    h = lmhist->histhash - w + dict_basewid (dict, w);
    h = (h >> 5) | (h << 27);	/* Rotate right 5 bits */
    h += l->node->wid;
    hmod = h % HISTHASH_MOD;
    
    /* If new node would be an inferior duplicate, skip creating it */
    if (ppath_dup (hash_list[hmod], lmhist, l->node, h, pscr))
	return;

    /* Add heuristic score from END OF l until end of utterance */
    tscr = pscr + l->hscr;

    /* Initialize new partial path node */
    pp = (ppath_t *) listelem_alloc (sizeof(ppath_t));

    pp->dagnode = l->node;
    pp->hist = top;
    pp->lmhist = lmhist;
    pp->lscr = lscr;
    pp->pscr = pscr;
    pp->tscr = tscr;
    pp->histhash = h;
    pp->hashnext = hash_list[hmod];
    hash_list[hmod] = pp;
    pp->pruned = 0;

    pp->next = ppath_list;
    ppath_list = pp;
    
    heap_root = aheap_insert (heap_root, pp);
    
    n_ppath++;
}


static int32 ppath_free ( void )
{
    ppath_t *pp;
    int32 n;
    
    n = 0;
    while (ppath_list) {
	pp = ppath_list->next;
	listelem_free ((char *) ppath_list, sizeof(ppath_t));
	ppath_list = pp;
	n++;
    }
    
    return n;
}


static void ppath_seg_write (FILE *fp, ppath_t *pp, int32 ascr)
{
    int32 lscr_base;
    
    if (pp->hist)
	ppath_seg_write (fp, pp->hist, pp->pscr - pp->hist->pscr - pp->lscr);

    lscr_base = pp->hist ? lm_rawscore (lmset->cur_lm, pp->lscr, 1.0) : 0;

    fprintf (fp, " %d %d %d %s",
	     pp->dagnode->sf, ascr, lscr_base, dict_wordstr (dict, pp->dagnode->wid));
}


static void nbest_hyp_write (FILE *fp, ppath_t *top, int32 pscr, int32 nfr)
{
    int32 lscr, lscr_base;
    ppath_t *pp;
    
    lscr_base = 0;
    for (lscr = 0, pp = top; pp; lscr += pp->lscr, pp = pp->hist) {
	if (pp->hist)
	    lscr_base += lm_rawscore (lmset->cur_lm, pp->lscr, 1.0);
	else
	    assert (pp->lscr == 0);
    }

    fprintf (fp, "T %d A %d L %d", pscr, pscr - lscr, lscr_base);

    ppath_seg_write (fp, top, pscr - top->pscr);

    fprintf (fp, " %d\n", nfr);
    fflush (fp);
}


#if 0
static ppath_dump (ppath_t *p)
{
    printf ("PPATH:\n");
    for (; p; p = p->hist) {
	printf ("pscr= %11d, lscr= %9d, tscr= %11d, hash= %11u, pruned= %d, sf= %5d, %s\n",
		p->pscr, p->lscr, p->tscr, p->histhash, p->pruned,
		p->dagnode->sf, dict_wordstr (dict, p->dagnode->wid));
    }
}
#endif

void nbest_search (char *filename, char *uttid)
{
    FILE *fp;
    float32 f32arg;
    float64 f64arg;
    int32 nbest_max, n_pop, n_exp, n_hyp, n_pp;
    int32 besthyp, worsthyp, besttscr;
    ppath_t *top, *pp;
    dagnode_t *d;
    daglink_t *l;
    int32 lscr, pscr, tscr;
    s3wid_t bw0, bw1, bw2;
    int32 i, k;
    int32 ispipe;
    int32 ppathdebug;
    lm_t *lm;
    
    lm=lmset->cur_lm;

    
    /* Create Nbest file and write header comments */
    if ((fp = fopen_comp (filename, "w", &ispipe)) == NULL) {
	E_ERROR("fopen_comp (%s,w) failed\n", filename);
	fp = stdout;
    }
    fprintf (fp, "# %s\n", uttid);
    fprintf (fp, "# frames %d\n", dag.nfrm);
    f32arg = *((float32 *) cmd_ln_access ("-logbase"));
    fprintf (fp, "# logbase %e\n", f32arg);
    f32arg = *((float32 *) cmd_ln_access ("-lw"));
    fprintf (fp, "# langwt %e\n", f32arg);
    f32arg = *((float32 *) cmd_ln_access ("-wip"));
    fprintf (fp, "# inspen %e\n", f32arg);
    f64arg = *((float64 *) cmd_ln_access ("-beam"));
    fprintf (fp, "# beam %e\n", f64arg);
    ppathdebug = *((int32 *) cmd_ln_access ("-ppathdebug"));
    
    assert (heap_root == NULL);
    assert (ppath_list == NULL);
    
    /*
     * Set limit on max LM ops allowed after which utterance is aborted.
     * Limit is lesser of absolute max and per frame max.
     */
    maxlmop = *((int32 *) cmd_ln_access ("-maxlmop"));
    k = *((int32 *) cmd_ln_access ("-maxlpf"));
    k *= dag.nfrm;
    if (maxlmop > k)
	maxlmop = k;
    lmop = 0;
    
    /* Set limit on max #ppaths allocated before aborting utterance */
    maxppath = *((int32 *) cmd_ln_access ("-maxppath"));
    n_ppath = 0;
    
    for (i = 0; i < HISTHASH_MOD; i++)
	hash_list[i] = NULL;
    
    /* Insert start node into heap and into list of nodes-by-frame */
    pp = (ppath_t *) listelem_alloc (sizeof(ppath_t));

    pp->dagnode = dag.entry.node;
    pp->hist = NULL;
    pp->lmhist = NULL;
    pp->lscr = 0;
    pp->pscr = 0;
    pp->tscr = 0;	/* HACK!! Not really used as it is popped off rightaway */
    pp->histhash = pp->dagnode->wid;
    pp->hashnext = NULL;
    pp->pruned = 0;
    
    pp->next = NULL;
    ppath_list = pp;

    /* Insert into heap of partial paths to be expanded */
    heap_root = aheap_insert (heap_root, pp);
    
    /* Insert at head of (empty) list of ppaths with same hashmod value */
    hash_list[pp->histhash % HISTHASH_MOD] = pp;
    
    /* Astar-search */
    n_hyp = n_pop = n_exp = n_pp = 0;
    nbest_max = *((int32 *) cmd_ln_access ("-nbest"));
    besthyp = besttscr = (int32)0x80000000;
    worsthyp = (int32)0x7fffffff;
    
    while ((n_hyp < nbest_max) && heap_root) {
	/* Extract top node from heap */
	top = heap_root->ppath;
	heap_root = aheap_pop (heap_root);
	
	n_pop++;
	
	if (top->pruned)
	    continue;
	
	if (top->dagnode == dag.exit.node) {	/* Complete hypotheses; output */
	    nbest_hyp_write (fp, top, top->pscr + dag.exit.ascr, dag.nfrm);
	    n_hyp++;
	    if (besthyp < top->pscr)
		besthyp = top->pscr;
	    if (worsthyp > top->pscr)
		worsthyp = top->pscr;
	    
	    continue;
	}
	
	/* Find two word (trigram) history beginning at this node */
	pp = (dict_filler_word (dict,top->dagnode->wid)) ? top->lmhist : top;
	if (pp) {
	    bw1 = dict_basewid(dict, pp->dagnode->wid);
	    pp = pp->lmhist;
	    bw0 = pp ? dict_basewid(dict, pp->dagnode->wid) : BAD_S3WID;
	} else
	    bw0 = bw1 = BAD_S3WID;
	
	/* Expand to successors of top (i.e. via each link leaving top) */
	d = top->dagnode;
	for (l = d->succlist; l; l = l->next) {
	    assert (l->node->reachable && (! l->is_filler_bypass));

	    /* Obtain LM score for link */
	    bw2 = dict_basewid (dict, l->node->wid);
	    lscr = (dict_filler_word (dict,bw2)) ? fillpen(fpen, bw2) : lm_tg_score (lm, lm->dict2lmwid[bw0], lm->dict2lmwid[bw1], lm->dict2lmwid[bw2],bw2);

	    if (lmop++ > maxlmop) {
		E_ERROR("%s: Max LM ops (%d) exceeded\n", uttid, maxlmop);
		break;
	    }
	    
	    /* Obtain partial path score and hypothesized total utt score */
	    pscr = top->pscr + l->ascr + lscr;
	    tscr = pscr + l->hscr;

	    if (ppathdebug) {
		printf ("pscr= %11d, tscr= %11d, sf= %5d, %s%s\n",
			pscr, tscr, l->node->sf, dict_wordstr(dict, l->node->wid),
			(tscr-beam >= besttscr) ? "" : " (pruned)");
	    }
	    
	    /* Insert extended path if within beam of best so far */
	    if (tscr - beam >= besttscr) {
		ppath_insert (top, l, pscr, tscr, lscr);
		if (n_ppath > maxppath) {
		    E_ERROR("%s: Max PPATH limit (%d) exceeded\n", uttid, maxppath);
		    break;
		}
		
		if (tscr > besttscr)
		    besttscr = tscr;
	    }
	}
	if (l)	/* Above loop was aborted */
	    break;
	
	n_exp++;
    }

    fprintf (fp, "End; best %d worst %d diff %d beam %d\n",
	     besthyp + dag.exit.ascr, worsthyp + dag.exit.ascr, worsthyp - besthyp, beam);
    fclose_comp (fp, ispipe);
    if (n_hyp <= 0) {
	unlink (filename);
	E_ERROR("%s: A* search failed\n", uttid);
    }
    
    /* Free partial path nodes and any unprocessed heap */
    while (heap_root)
	heap_root = aheap_pop(heap_root);

    n_pp = ppath_free ();

    printf ("CTR(%s): %5d frm %4d hyp %6d pop %6d exp %8d pp\n",
	    uttid, dag.nfrm, n_hyp, n_pop, n_exp, n_pp);
}

void nbest_init ( void )
{
    float64 *f64arg;
    float32 lw, wip;
    int32 fudge;

    fudge = *((int32 *) cmd_ln_access ("-dagfudge"));
    if ((fudge < 0) || (fudge > 2))
	E_FATAL("Bad -dagfudge argument: %d, must be in range 0..2\n", fudge);

    lw = *((float32 *) cmd_ln_access("-lw"));
    wip = *((float32 *) cmd_ln_access("-wip"));

    f64arg = (float64 *) cmd_ln_access ("-beam");
    beam = logs3 (*f64arg);
    E_INFO("beam= %d\n", beam);
    
    /* Initialize DAG and nbest search structures */
    dag.list = NULL;

    heap_root = NULL;
    ppath_list = NULL;
    hash_list = (ppath_t **) ckd_calloc (HISTHASH_MOD, sizeof(ppath_t *));
}
