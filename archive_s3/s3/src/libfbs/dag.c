/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <s3.h>

#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "logs3.h"


/*
 * DAG structure representation of word lattice.  A unique <wordid,startframe> is a node.
 * Edges are formed if permitted by time adjacency.  (See comment before dag_build.)
 */
typedef struct dagnode_s {
    s3wid_t wid;
    int32 seqid;			/* Running sequence no. for identification */
    s3frmid_t sf;			/* Start frame for this occurrence of wid */
    s3frmid_t fef, lef;			/* First and last end frames */
    struct dagnode_s *alloc_next;	/* Next in linear list of allocated nodes */
    struct daglink_s *succlist;		/* List of successor nodes (adjacent in time) */
    struct daglink_s *predlist;		/* List of preceding nodes (adjacent in time) */
} dagnode_t;

/* A DAG node can have several successor or predecessor nodes, each represented by a link */
typedef struct daglink_s {
    dagnode_t *node;		/* Target of link (source determined by dagnode_t.succlist
				   or dagnode_t.predlist) */
    dagnode_t *src;		/* Source node of link */
    struct daglink_s *next;	/* Next in same dagnode_t.succlist or dagnode_t.predlist */
    struct daglink_s *history;	/* Previous link along best path (for traceback) */
    struct daglink_s *bypass;	/* If this links A->B, bypassing A->fillnode->B, then
				   bypass is ptr to fillnode->B */
    int32 ascr;			/* Acoustic score for segment of source node ending just
				   before the end point of this link.  (Actually this gets
				   corrupted because of filler node deletion.) */
    int32 lscr;			/* LM score to the SUCCESSOR node */
    int32 pscr;			/* Best path score to root beginning with this link */
    s3frmid_t ef;		/* End time for this link.  Should be 1 before the start
				   time of destination node (or source node for reverse
				   links), but gets corrupted because of filler deletion */
    uint8 pscr_valid;		/* Flag to avoid evaluating the same path multiple times */
} daglink_t;

/* Summary of DAG structure information */
typedef struct {
    dagnode_t *list;		/* Linear list of nodes allocated */
    daglink_t entry;		/* Entering (<s>,0) */
    daglink_t exit;		/* Exiting (</s>,finalframe) */
    s3wid_t orig_exitwid;	/* If original exit node is not a filler word */
    int32 nfrm;
    int32 nlink;
} dag_t;
static dag_t dag;

static s3wid_t startwid;	/* Begin silence */
static s3wid_t finishwid;	/* End silence */

static dict_t *dict;		/* The dictionary */

static hyp_t *hyp = NULL;	/* The final recognition result */

static int32 maxlmop;		/* Max LM ops allowed before utterance aborted */
static int32 lmop;		/* #LM ops actually made */
static int32 maxedge;		/* Max #edges in DAG allowed before utterance aborted */


/* Get rid of old hyp, if any */
static void hyp_free ( void )
{
    hyp_t *tmphyp;
    
    while (hyp) {
	tmphyp = hyp->next;
	listelem_free ((char *)hyp, sizeof(hyp_t));
	hyp = tmphyp;
    }
}


static int32 filler_word (s3wid_t w)
{
    if ((w == startwid) || (w == finishwid))
	return 0;
    if ((w >= dict->filler_start) && (w <= dict->filler_end))
	return 1;
    return 0;
}


void dag_init ( void )
{
    dict = dict_getdict ();

    /* Some key word ids */
    startwid = dict_wordid (START_WORD);
    finishwid = dict_wordid (FINISH_WORD);
    if ((NOT_WID(startwid)) || (NOT_WID(finishwid)))
	E_FATAL("%s or %s missing from dictionary\n", START_WORD, FINISH_WORD);

    /* Initialize DAG structure */
    dag.list = NULL;

    /* Set limit on max DAG edges allowed after which utterance is aborted */
    maxedge = *((int32 *) cmd_ln_access ("-maxedge"));
}


/*
 * Link two DAG nodes with the given arguments
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
static int32 dag_link (dagnode_t *pd, dagnode_t *d, int32 ascr, int32 ef, daglink_t *byp)
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

    dag.nlink++;

    return (dag.nlink > maxedge) ? -1 : 0;
}


static daglink_t *find_succlink (dagnode_t *src, dagnode_t *dst)
{
    daglink_t *l;

    for (l = src->succlist; l && (l->node != dst); l = l->next);
    return l;
}


static daglink_t *find_predlink (dagnode_t *src, dagnode_t *dst)
{
    daglink_t *l;

    for (l = src->predlist; l && (l->node != dst); l = l->next);
    return l;
}


/*
 * Like dag_link but check if link already exists.  If so, replace if new score better.
 * Return value: 0 if successful, -1 if maxedge limit exceeded.
 */
static int32 dag_update_link (dagnode_t *pd, dagnode_t *d, int32 ascr,
			      int32 ef, daglink_t *byp)
{
    daglink_t *l, *r;
    
    l = find_succlink (pd, d);

    if (! l)
	return (dag_link (pd, d, ascr, ef, byp));

    if (l->ascr < ascr) {
	r = find_predlink (d, pd);

	assert (r && (r->ascr == l->ascr));
	l->ascr = r->ascr = ascr;
	l->ef = r->ef = ef;
	l->bypass = r->bypass = byp;
    }
    
    return 0;
}


static int32 dag_param_read (FILE *fp, char *param, int32 *lineno)
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
    int32 ascr;

    for (d = dag.list; d; d = d->alloc_next) {
	if (! filler_word (d->wid))
	    continue;
	
	/* Replace each link TO d with links to d's successors */
	for (plink = d->predlist; plink; plink = plink->next) {
	    pnode = plink->node;
	    ascr = plink->ascr;
	    ascr += fillpen(dict_basewid (d->wid));
	    
	    /* Link this predecessor of d to successors of d */
	    for (slink = d->succlist; slink; slink = slink->next) {
		snode = slink->node;

		/* Link only to non-filler successors; fillers have been eliminated */
		if (! filler_word (snode->wid)) {
		    /* Update because a link may already exist */
		    if (dag_update_link (pnode, snode,
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
int32 dag_load (char *file)
{
    FILE *fp;
    char line[1024], wd[1024];
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
    
    dag.list = NULL;
    dag.nlink = 0;
    
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

	w = dict_wordid (wd);
	if (NOT_WID(w)) {
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
	if ((sscanf (line, "%s", wd, &k) == 1) && (strcmp (wd, "Edges") == 0))
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
	if ((pd == dag.entry.node) || (d == dag.exit.node) ||
	    ((d->lef - d->fef >= min_ef_range-1) && (pd->lef - pd->fef >= min_ef_range-1))) {
	    if (dag_link (pd, d, ascr, d->sf-1, NULL) < 0) {
		E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, maxedge);
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
		assert (d->wid == startwid);
	    else if ((d == dag.exit.node) || (d->lef - d->fef >= min_ef_range-1)) {
		/* Link from all end points == d->sf-1 to d */
		for (l = frm2lat[d->sf-1]; l < frm2lat[d->sf]; l++) {
		    pd = lat[l].node;		/* Predecessor DAG node */
		    if (pd->wid == finishwid)
			continue;

		    if ((pd == dag.entry.node) || (pd->lef - pd->fef >= min_ef_range-1)) {
			dag_link (pd, d, lat[l].ascr, d->sf-1, NULL);
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
		    dag_link (pd, d, lat[l].ascr, d->sf-1, NULL);
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
		    dag_link (pd, d, lat[l].ascr, d->sf-1, NULL);
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
    if (filler_word(dag.exit.node->wid))
	dag.exit.node->wid = finishwid;
    
    /* Add links bypassing filler nodes */
    if (dag_remove_filler_nodes () < 0) {
	E_ERROR ("%s: maxedge limit (%d) exceeded\n", file, maxedge);
	return -1;
    }
    
    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link (NULL, dag.entry.node, 0, -1, NULL);
    
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


int32 dag_destroy ( void )
{
    dagnode_t *d, *nd;
    daglink_t *l, *nl;
    
    for (d = dag.list; d; d = nd) {
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

    dag.list = NULL;

    return 0;
}


/*
 * Recursive step in dag_search:  best backward path from src to root beginning with l.
 * Return value: 0 if successful, -1 otherwise.
 */
static int32 dag_bestpath (daglink_t *l,	/* Backward link! */
			   dagnode_t *src)	/* Source node for backward link l */
{
    dagnode_t *d, *pd;
    daglink_t *pl;
    int32 lscr, score;
    
    assert (! l->pscr_valid);
    
    if ((d = l->node) == NULL) {
	/* If no destination at end of l, src is root node.  Recursion termination */
	assert (dict_basewid(src->wid) == startwid);
	l->lscr = 0;
	l->pscr = 0;
	l->pscr_valid = 1;
	l->history = NULL;
	
	return 0;
    }
    
    /* Search all predecessor links of l */
    for (pl = d->predlist; pl; pl = pl->next) {
	pd = pl->node;
	if (pd && filler_word (pd->wid))	/* Skip filler node */
	    continue;

	/* Evaluate best path along pl if not yet evaluated (recursive step) */
	if (! pl->pscr_valid)
	    if (dag_bestpath (pl, d) < 0)
		return -1;
	
	/* Accumulated path score along pl->l */
	if (pl->pscr > (int32)0x80000000) {
	    score = pl->pscr + l->ascr;
	    if (score > l->pscr) {	/* rkm: Added 20-Nov-1996 */
		if (pd)
		    lscr = lm_tg_score (dict_basewid(pd->wid),
					dict_basewid(d->wid),
					dict_basewid(src->wid));
		else
		    lscr = lm_bg_score (dict_basewid(d->wid), dict_basewid(src->wid));
		score += lscr;

		if (lmop++ >= maxlmop)
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
	    dict_wordstr (d->wid), d->sf,
	    dict_wordstr (src->wid), src->sf,
	    l->pscr);
#endif

    l->pscr_valid = 1;

    return 0;
}


/*
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
static hyp_t *dag_backtrace (daglink_t *l)
{
    hyp_t *h, *hhead, *htail;
    int32 pscr;
    dagnode_t *src, *dst;
    daglink_t *bl, *hist;
    
    hyp = NULL;
    
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
	    h = (hyp_t *) listelem_alloc (sizeof(hyp_t));
	    h->wid = l->node->wid;
	    h->word = dict_wordstr (h->wid);
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
		h = (hyp_t *) listelem_alloc (sizeof(hyp_t));
		h->wid = src->wid;
		h->word = dict_wordstr (h->wid);
		h->sf = src->sf;

		if (hhead)
		    h->lscr = fillpen (dict_basewid (src->wid));
		
		if (l->bypass) {
		    dst = l->bypass->src;
		    assert (filler_word (dst->wid));
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


static int32 dag_chk_linkscr (dag_t *dag)
{
    dagnode_t *d;
    daglink_t *l;
    
    for (d = dag->list; d; d = d->alloc_next) {
	for (l = d->succlist; l; l = l->next) {
	    if (l->ascr >= 0)
		return -1;
	}
    }

    return 0;
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
hyp_t *dag_search (char *utt)
{
    daglink_t *l, *bestl;
    dagnode_t *d, *final;
    int32 bestscore;
    int32 k;
    
    /* Free any old hypothesis */
    hyp_free ();
    
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

    /* Find the backward link from the final DAG node that has the best path to root */
    final = dag.exit.node;
    bestscore = (int32) 0x80000000;
    bestl = NULL;

    /* Check that all edge scores are -ve; error if not so. */
    if (dag_chk_linkscr (&dag) < 0)
	return NULL;

    for (l = final->predlist; l; l = l->next) {
	d = l->node;
	if (! filler_word (d->wid)) {	/* Ignore filler node */
	    /* Best path to root beginning with l */
	    if (dag_bestpath (l, final) < 0) {
		E_ERROR("%s: Max LM ops (%d) exceeded\n", utt, maxlmop);
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
    l = &(dag.exit);
    l->history = bestl;
    l->pscr += bestl->pscr + l->ascr;
    l->ef = dag.nfrm - 1;
    
    /* Backtrack through DAG for best path */
    hyp = dag_backtrace (l);
    
    return (hyp);
}
