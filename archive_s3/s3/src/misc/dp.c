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
 * dp.c -- DP alignment core routines.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 07-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include "dp.h"


#define MAX_UTT_LEN 1024


/*
 * Backtrace through DP alignment; print alignment.
 * Return value: #hyp.
 */
static int32 dp_backtrace (FILE *fp,
			   dict_t *dict, s3wid_t oovbegin,
			   dpnode_t **dparray,
			   dagnode_t **node_array, int32 r,
			   dagnode_t *ref, int32 nref,
			   dpnode_t *ret)
{
    int32 i, j, k, nhyp, pr, pc;
    s3wid_t hyp[MAX_UTT_LEN];
    int32 ref2hyp[MAX_UTT_LEN];
    char word[4096];
    int32 state, erreg_ref, erreg_hyp;

    /* Backtrace best path and map ref words to hyp words wherever they match */
    for (i = 0; i < nref; i++)
	ref2hyp[i] = -1;

    k = MAX_UTT_LEN;
    for (i = r, j = (nref<<1) - 1; i >= 0; i = pr, j = pc) {
	if ((! dict_filler_word(dict, node_array[i]->wid)) || (i == r)) {
	    --k;
	    if (k < 0)
		E_FATAL("Increase MAX_UTT_LEN (from %d)\n", MAX_UTT_LEN);
	    
	    hyp[k] = node_array[i]->wid;
	    
	    if ((j & 0x1) && (ref[j>>1].wid == hyp[k]))
		ref2hyp[j>>1] = k;
	}

	pr = dparray[i][j].pr;
	pc = dparray[i][j].pc;
    }

    /* Shift hyp and ref2hyp to head of array */
    if (k > 0) {
	for (i = k; i < MAX_UTT_LEN; i++)
	    hyp[i-k] = hyp[i];
	for (i = 0; i < nref; i++)
	    ref2hyp[i] = ref2hyp[i] - k;
	nhyp = MAX_UTT_LEN - k;
    }

#if 0
    for (i = 0; i < nhyp; i++)
	printf ("%s ", dict_wordstr(dict, hyp[i]));
    printf ("\n");
    for (i = 0; i < nref; i++)
	printf ("%s %d ", dict_wordstr (dict, ref[i].wid), ref2hyp[i]);
    printf ("\n");
#endif

    assert (hyp[nhyp-1] == ref[nref-1].wid);
    /* assert (hyp[nhyp-1] == silwid); */
    assert (ref2hyp[nref-1] == nhyp-1);

    /* Output alignment */
    state = 0;		/* 0 => currently in a correct region, !0 => in error region */
    j = -1;		/* Last hyp entry output so far */
    ret->c = ret->e = 0;
    for (i = 0; i < nref; i++) {
	if (ref2hyp[i] >= 0) {
	    assert (ref2hyp[i] >= j+1);

	    if ((state != 0) || (ref2hyp[i] != j+1)) {	/* Intervening error hyp */
		if (state == 0) {
		    fprintf (fp, "[[ ");
		    erreg_ref = 0;
		    erreg_hyp = 0;
		}
		
		fprintf (fp, "=> ");
		for (j++; j < ref2hyp[i]; j++) {
		    strcpy (word, dict_wordstr (dict, hyp[j]));
		    ucase (word);
		    fprintf (fp, "%s ", word);
		    erreg_hyp++;
		}
		
		ret->e += ((erreg_ref >= erreg_hyp) ? erreg_ref : erreg_hyp);
		
		fprintf (fp, "]] ");
	    }
	    
	    /* Print correct word (if not [the terminating sentinel] filler) */
	    if (! dict_filler_word(dict, ref[i].wid)) {
		strcpy (word, dict_wordstr (dict, ref[i].wid));
		lcase (word);
		fprintf (fp, "%s ", word);
		(ret->c)++;
	    }

	    j = ref2hyp[i];
	    state = 0;
	} else {
	    if (state == 0) {	/* correct -> error */
		fprintf (fp, "[[ ");
		erreg_ref = 0;
		erreg_hyp = 0;
	    }
	    
	    strcpy (word, dict_wordstr (dict, ref[i].wid));
	    ucase (word);
	    if (ref[i].wid >= oovbegin)
		fprintf (fp, "%s(oov) ", word);
	    else
		fprintf (fp, "%s ", word);

	    erreg_ref++;
	    
	    state = 1;
	}
    }

    return nhyp-1;	/* -1 to get rid of sentinel silwid */
}


#define _DP_DEBUG_	0

#if (_DP_DEBUG_)
static void dpnode_dump (dpnode_t **dparray, int32 i, int32 j, s3wid_t ref, dagnode_t *d)
{
    printf ("[%4d][%4d].e,c,pr,pc = %3d %3d %3d %3d",
	    i, j, dparray[i][j].e, dparray[i][j].c, dparray[i][j].pr, dparray[i][j].pc);
    printf ("\t%s\t%s.%d\n", dict_wordstr(dict, ref), dict_wordstr(dict, d->wid), d->sf);
}
#else
#define dpnode_dump(d,i,j,r,h)
#endif


static int32 sub_time_err (dagnode_t *h, dagnode_t *r, int32 pos)
{
    int32 k;
    
    k = h->sf - r[pos].sf;
    if (k < 0)
	k = -k;
    
    return k;
}


static int32 ins_time_err (dagnode_t *h, dagnode_t *r, int32 pos)
{
    int32 j, k;
    
    k = h->sf - r[pos].sf;
    if (k < 0)
	k = -k;

    if (pos > 0) {
	j = h->sf - r[pos-1].sf;
	if (j < 0)
	    j = -j;
	
	if (k > j)
	    k = j;
    }
    
    return k;
}


/*
 * DP align DAG with ref, return #errors/#correct and #hyp (in *nhyp).  Note that
 * filler words do not get insertion errors.
 * Return value: dpnode_t with #error/#correct.
 */
dpnode_t dp (char *uttid, dict_t *dict, s3wid_t oovbegin,
	     dagnode_t *ref, int32 nref,
	     dag_t *dag, int32 *nhyp,
	     int32 use_time,
	     int32 backtrace)
{
    int32 f, i, j, k, jj, pr;
    int32 nnode;
    dpnode_t **dparray, retval;
    dagnode_t *d, *pd, **node_array;
    daglink_t *pl;
    int32 bestpr, bestpc, minerr;
    int32 sub, ins, err, time_err;

    /* Initialize return values */
    retval.e = nref-1;
    retval.c = 0;
    *nhyp = 0;

    /* Mark nodes from which final node is reachable */
    dag_reachable_bwd (dag->exit.dst);
    if (! dag->entry.dst->reachable) {
	E_ERROR("%s: DAG disconnected\n", uttid);
	return retval;
    }
    dag_reachable_fwd (dag->entry.dst);
    assert (dag->exit.dst->reachable == 2);
    
    /* Count and IDentify reachable nodes; convert word-IDs to base IDs */
    nnode = 0;
    node_array = (dagnode_t **) ckd_calloc (dag->nnode, sizeof(dagnode_t *));
    for (f = 0; f < dag->nfrm; f++) {
	for (d = dag->node_sf[f]; d; d = d->next) {
	    if (d->reachable == 2) {
		d->wid = dict_basewid (dict, d->wid);
		d->seqid = nnode;
		node_array[nnode] = d;
		nnode++;
	    } else
		d->seqid = -1;
	}
    }
    if (nnode > 32760)
	E_FATAL("Too many nodes: %d, cannot identify with 16 bits\n", nnode);
    E_INFO ("%s: %d frames, %d nodes (%d reachable), %d links\n",
	    uttid, dag->nfrm, dag->nnode, nnode, dag->nlink);
    
    /* Allocate and initialize DP array */
    dparray = (dpnode_t **) ckd_calloc_2d (nnode, nref<<1, sizeof(dpnode_t));
    for (i = 0; i < nnode; i++) {
	for (j = 0, jj = 0; j < nref; j++, jj += 2) {
	    dparray[i][jj].e = (int32)0x7fffffff;
	    dparray[i][jj+1].e = (int32)0x7fffffff;
	}
    }
    
    /* Initialize first row of DP-array */
    d = node_array[0];
    assert (d == dag->entry.dst);
    ins = dict_filler_word(dict, d->wid) ? 0 : 1;
    /* Place this word at all possible locations w.r.t. reference string */
    for (j = 0, jj = 0; j < nref; j++, jj += 2) {
	/* Insertion column first */
	time_err = use_time ? ins_time_err(d, ref, j) : 0;	/* Ins before j */
	dparray[0][jj].e = ins + j + time_err;
	dparray[0][jj].c = 0;
	dparray[0][jj].pr = -1;
	dparray[0][jj].pc = -1;
	dpnode_dump(dparray, 0, jj, ref[j].wid, d);
	
	/* Substitution column */
	sub = (d->wid != ref[j].wid);
	time_err = use_time ? sub_time_err(d, ref, j) : 0;	/* Sub at j */
	dparray[0][jj+1].e = sub + j + time_err;
	dparray[0][jj+1].c = 1 - sub;
	dparray[0][jj+1].pr = -1;
	dparray[0][jj+1].pc = -1;
	dpnode_dump(dparray, 0, jj+1, ref[j].wid, d);
    }
    
    /* Processing remaining words in DAG */
    for (i = 1; i < nnode; i++) {
	d = node_array[i];
	assert (d->predlist);
	
	/* Whether this word can cause an insertion error */
	ins = dict_filler_word(dict, d->wid) ? 0 : 1;

	/* Place this word at all possible locations w.r.t. reference string */
	for (j = 0; j < nref; j++) {
	    /* Insertion column first */
	    jj = j << 1;

	    time_err = use_time ? ins_time_err(d, ref, j) : 0;
	    
	    /* From all possible predecessor nodes */
	    minerr = (int32)0x7fffffff;
	    for (pl = d->predlist; pl; pl = pl->next) {
		pd = pl->dst;
		if (pd->reachable != 2)
		    continue;
		
		pr = pd->seqid;	/* Predecessor row */
		assert ((pr >= 0) && (pr < i));
		
		/* Transition [pr][jj] -> [i][jj] and [pr][jj-1] -> [i][jj] */
		if ((jj == 0) || (dparray[pr][jj].e < dparray[pr][jj-1].e) ||
		    ((dparray[pr][jj].e == dparray[pr][jj-1].e) &&
		     (dparray[pr][jj].c > dparray[pr][jj-1].c)))
		    k = jj;
		else
		    k = jj-1;

		err = dparray[pr][k].e + ins + time_err;
		if ((err < minerr) ||
		    ((err == minerr) &&
		     (dparray[bestpr][bestpc].c < dparray[pr][k].c))) {
		    minerr = err;
		    bestpr = pr;
		    bestpc = k;
		}
	    }

	    /* Transitions [pr][jj-2..0] -> [i][jj] (via [i][jj-1]) */
	    if (jj >= 2) {
		err = dparray[i][jj-1].e + ins + 1 + time_err;	/* 1 deletion */
		err -= (d->wid != ref[j-1].wid);	/* Remove sub error at [jj-1] */

		/* Get rid of time error from jj-1, if using time */
		err -= (use_time ? sub_time_err(d, ref, j-1) : 0);

		pr = dparray[i][jj-1].pr;
		k = dparray[i][jj-1].pc;
		
		if ((err < minerr) ||
		    ((err == minerr) &&
		     (dparray[bestpr][bestpc].c < dparray[pr][k].c))) {
		    minerr = err;
		    bestpr = pr;
		    bestpc = k;
		}
	    }

	    /* Update dparray[i][jj] with best incoming transition */
	    dparray[i][jj].e = minerr;
	    dparray[i][jj].c = dparray[bestpr][bestpc].c;
	    dparray[i][jj].pr = bestpr;
	    dparray[i][jj].pc = bestpc;
	    dpnode_dump(dparray, i, jj, ref[j].wid, d);
	    
	    /* Substitution column next; simply move up from [i][jj-1] */
	    minerr = dparray[i][jj].e - ins;	/* Remove ins error at [i][jj] */
	    sub = (d->wid != ref[j].wid);	/* Add sub error at j */
	    minerr += sub;

	    /* Add time error for jj and remove for jj-1 */
	    minerr += use_time ? sub_time_err(d, ref, j) : 0;
	    minerr -= use_time ? ins_time_err(d, ref, j) : 0;

	    jj++;

	    /* Update dparray[i][jj] with best incoming transition */
	    dparray[i][jj].e = minerr;
	    dparray[i][jj].c = dparray[i][jj-1].c + (1 - sub);
	    dparray[i][jj].pr = dparray[i][jj-1].pr;
	    dparray[i][jj].pc = dparray[i][jj-1].pc;
	    dpnode_dump(dparray, i, jj, ref[j].wid, d);
	}
    }
    
    /* Backtrace */
    if (backtrace) {
	*nhyp = dp_backtrace (stdout, dict, oovbegin,
			      dparray, node_array, dag->exit.dst->seqid,
			      ref, nref, &retval);
	
	if (! use_time) {
	    assert (retval.c == dparray[dag->exit.dst->seqid][(nref<<1)-2].c);
	    assert (retval.e == dparray[dag->exit.dst->seqid][(nref<<1)-2].e);
	}
    } else {
	retval.c = dparray[dag->exit.dst->seqid][(nref<<1)-2].c;
	retval.e = dparray[dag->exit.dst->seqid][(nref<<1)-2].e;
    }
    
    ckd_free (node_array);
    ckd_free_2d ((void **) dparray);

    return retval;
}
