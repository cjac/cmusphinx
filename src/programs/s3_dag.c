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
 * dag.c -- DAG search
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
 * Revision 1.9  2006/02/28  02:06:47  egouvea
 * Updated MS Visual C++ 6.0 support files. Fixed things that didn't
 * compile in Visual C++ (declarations didn't match, etc). There are
 * still some warnings, so this is not final. Also, sorted files in
 * several Makefile.am.
 * 
 * Revision 1.8  2006/01/17 20:57:53  dhdfu
 * Make sure we don't doubly-free hyp if bestpath search fails (which it sometimes does due to -min_endfr pruning out crucial nodes...)
 *
 * Revision 1.7  2005/06/22 05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.7  2005/06/20 22:20:19  archan
 * Fix non-conforming problems for Windows plot.
 *
 * Revision 1.6  2005/06/19 03:58:17  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.5  2005/06/18 03:23:13  archan
 * Change to lmset interface.
 *
 * Revision 1.4  2005/06/03 06:45:30  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.3  2005/06/03 05:46:42  archan
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

/** \file s3_dag.c
    \brief Engine for s3.0 dag
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3types.h>
#include "s3_dag.h"
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "logs3.h"
#include "dag.h"


dag_t dag;
/* Global variables : Hack! */
dict_t *dict;		/* The dictionary */
fillpen_t *fpen;         /* The fillpenalty data structure */

#if 0
lm_t *lm;                /* Global variables */
s3lmwid_t *dict2lmwid;	/* Mapping from decoding dictionary wid's to lm ones.  They may not be the same! */
#endif

lmset_t *lmset;



static srch_hyp_t *hyp = NULL;	/* The final recognition result */

void s3_dag_init (dict_t* _dict )
{
  dict = _dict;
  dag_init(&dag);
}


/*
 * Remove filler nodes from DAG by replacing each link TO a filler with links
 * to its successors.  In principle, successors can be fillers and the process
 * must be repeated.  But removing fillers in the order in which they appear in
 * dag.list ensures that succeeding fillers have already been eliminated.
 * Return value: 0 if successful; -1 if DAG maxedge limit exceeded.
 */
static int32 dag_remove_filler_nodes ( void )
{
    dagnode_t *d, *pnode, *snode;
    daglink_t *plink, *slink;
    int32 ascr=0;
    
    assert(dag.list);

    for (d = dag.list; d; d = d->alloc_next) {

	if (! dict_filler_word (dict, d->wid))
	    continue;
	

	/* Replace each link TO d with links to d's successors */
	for (plink = d->predlist; plink; plink = plink->next) {
	    pnode = plink->node;

	    ascr = plink->ascr; 
	    ascr += fillpen(fpen,dict_basewid (dict,d->wid));
	    
	    /* Link this predecessor of d to successors of d */
	    for (slink = d->succlist; slink; slink = slink->next) {
		snode = slink->node;
		/* Link only to non-filler successors; fillers have been eliminated */
		if (! dict_filler_word (dict, snode->wid)) {
		    /* Update because a link may already exist */
		    if (dag_update_link (&dag,pnode, snode,
					 ascr + slink->ascr, plink->ef, slink) < 0)
			return -1;
		}
	    }
	}
    }
    return 0;
}


/*
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
int32 s3dag_dag_load (char *file)
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

    s3wid_t finishwid;

    finishwid = dict_wordid (dict,S3_FINISH_WORD);
    
    dag.list = NULL;
    dag.nlink = 0;
    dag.nbypass =0;
    
    tail = NULL;
    darray = NULL;
    lat = NULL;
    frm2lat = NULL;
    lineno = 0;
    
    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen_compchk (file, &ispipe)) == NULL) {
	E_ERROR("fopen_compchk(%s) failed\n", file);
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
    if ((strncmp (line, "# -logbase ", 11) != 0) || (sscanf (line+11, "%f", &lb) != 1)) {
	E_WARN ("%s: Cannot find -logbase in header\n", file);
    } else {
	f32arg = *((float32 *) cmd_ln_access ("-logbase"));
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
	    E_ERROR ("Premature EOF(%s)\n", file);
	    goto load_error;
	}
	lineno++;
	
	if ((k = sscanf (line, "%d %s %d %d %d", &seqid, wd, &sf, &fef, &lef)) != 5) {
	    E_ERROR("Bad line: %s\n", line);
	    goto load_error;
	}

	w = dict_wordid (dict,wd);
	if (NOT_S3WID(w)) {
	    E_ERROR("Unknown word: %s\n", line);
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
    dag.final.node = darray[k];
    dag.final.ascr = 0;
    dag.final.next = NULL;
    dag.final.pscr_valid = 0;
    dag.final.bypass = NULL;
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
	    E_ERROR("Premature EOF(%s)\n", line);
	    goto load_error;
	}
	
	lineno++;

	if (sscanf (line, "%d %d %d", &seqid, &ef, &ascr) != 3) {
	    E_ERROR("Bad line: %s\n", line);
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
	
	if ((seqid == final) && (ef == dag.final.node->lef))
	    dag.final.ascr = ascr;
    }
    for (j++; j <= nfrm; j++)
	frm2lat[j] = k;
    
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
	if ((pd == dag.entry.node) || (d == dag.final.node) ||
	    ((d->lef - d->fef >= min_ef_range-1) && (pd->lef - pd->fef >= min_ef_range-1))) {
	    if (dag_link (&dag,pd, d, ascr, d->sf-1, NULL) < 0) {
		E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, dag.maxedge);
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
	for (d = dag.list; d; d = d->alloc_next) {
	    if (d->sf == 0)
		assert (d->wid == dict->startwid);
	    else if ((d == dag.final.node) || (d->lef - d->fef >= min_ef_range-1)) {
		/* Link from all end points == d->sf-1 to d */
		for (l = frm2lat[d->sf-1]; l < frm2lat[d->sf]; l++) {
		    pd = lat[l].node;		/* Predecessor DAG node */
		    if (pd->wid == finishwid)
			continue;

		    if ((pd == dag.entry.node) || (pd->lef - pd->fef >= min_ef_range-1)) {
			dag_link (&dag,pd, d, lat[l].ascr, d->sf-1, NULL);
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
		if ((pd->wid != finishwid) && (pd->fef == d->sf) &&
		    (pd->lef - pd->fef >= min_ef_range-1)) {
		    dag_link (&dag, pd, d, lat[l].ascr, d->sf-1, NULL);
		    k++;
		}
	    }
	    
	    if (fudge < 2)
		continue;
	    
	    /* Links to d from nodes that first ended just BEYOND when d started */
	    for (l = frm2lat[d->sf+1]; l < frm2lat[d->sf+2]; l++) {
		pd = lat[l].node;		/* Predecessor DAG node */
		if ((pd->wid != finishwid) && (pd->fef == d->sf+1) &&
		    (pd->lef - pd->fef >= min_ef_range-1)) {
		    dag_link (&dag, pd, d, lat[l].ascr, d->sf-1, NULL);
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
     * HACK!! Change dag.final.node wid to finishwid if some other filler word,
     * to avoid complications with LM scores at this point.
     */
    dag.orig_exitwid = dag.final.node->wid;
    if (dict_filler_word(dict, dag.final.node->wid))
	dag.final.node->wid = finishwid;
    
    /* Add links bypassing filler nodes */
    if (dag_remove_filler_nodes () < 0) {
	E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, dag.maxedge);
	return -1;
    }
    
    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link (&dag,NULL, dag.entry.node, 0, -1, NULL);
    
    E_INFO("%5d frames, %6d nodes, %8d edges\n", nfrm, nnode, dag.nlink);
    
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




/*
 * Final global best path through DAG constructed from the word lattice.
 * Assumes that the DAG has already been constructed and is consistent with the word
 * lattice.
 * The search uses a recursive algorithm to find the best (reverse) path from the final
 * DAG node to the root:  The best path from any node (beginning with a particular link L)
 * depends on a similar best path for all links leaving the endpoint of L.  (This is
 * sufficient to handle trigram LMs.)
 */
srch_hyp_t *s3dag_dag_search (char *utt)
{
    daglink_t *l, *bestl;
    dagnode_t *d, *final;
    int32 bestscore;
    int32 k;
    
    /* Free any old hypothesis */
    hyp_free (hyp);
    hyp = NULL;
    
    /*
     * Set limit on max LM ops allowed after which utterance is aborted.
     * Limit is lesser of absolute max and per frame max.
     */
    dag.maxlmop = *((int32 *) cmd_ln_access ("-maxlmop"));
    k = *((int32 *) cmd_ln_access ("-maxlpf"));
    k *= dag.nfrm;
    if (dag.maxlmop > k)
	dag.maxlmop = k;
    dag.lmop = 0;

    /* Find the backward link from the final DAG node that has the best path to root */
    final = dag.final.node;
    bestscore = (int32) 0x80000000;
    bestl = NULL;

    /* Check that all edge scores are -ve; error if not so. */
    if (dag_chk_linkscr (&dag) < 0){
      E_ERROR("Some edges are not negative\n");
      return NULL;
    }

    for (l = final->predlist; l; l = l->next) {

	d = l->node;
	if (! dict_filler_word (dict,d->wid)) {	/* Ignore filler node */
	    /* Best path to root beginning with l */
	    if (dag_bestpath (&dag,l, final,1,dict,lmset->cur_lm,lmset->cur_lm->dict2lmwid) < 0) {
		E_ERROR("%s: Max LM ops (%d) exceeded\n", utt, dag.maxlmop);
		bestl = NULL;
		break;
	    }

	    if (l->pscr > bestscore) {
		bestscore = l->pscr;
		bestl = l;
	    }
	}
    }

    if (! bestl) {
	E_ERROR("Bestpath search failed for %s\n", utt);
	return NULL;
    }
    
    /*
     * At this point bestl is the best (reverse) link/path leaving the final node.  But
     * this does not include the acoustic score for the final node itself.  Add it.
     */
    l = &(dag.final);
    l->history = bestl;
    l->pscr += bestl->pscr + l->ascr;
    l->ef = dag.nfrm - 1;
    
    /* Backtrack through DAG for best path */
    hyp = dag_backtrace (hyp,l,1,dict,fpen);
    assert(hyp);
    if(hyp==NULL){
      E_INFO("At this point hyp is NULL\n");
    }else{
      E_INFO("At this point hyp it is not NULL\n");
    }
    return (hyp);
}
