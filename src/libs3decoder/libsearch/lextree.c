/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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


static lextree_node_t *lextree_node_alloc (int32 wid, int32 prob,
					   int32 comp, int32 ssid, int32 n_state, int32 ci)
{
    lextree_node_t *ln;
    
    ln = (lextree_node_t *) mymalloc (sizeof(lextree_node_t));
    ln->children = NULL;
    ln->wid = wid;
    ln->prob = prob;
    ln->ssid = ssid;
    ln->ci = (s3cipid_t) ci;
    ln->composite = comp;
    ln->frame = -1;
    ln->hmm.state = (hmm_state_t *) ckd_calloc (n_state, sizeof(hmm_state_t));
    
    hmm_clear (&(ln->hmm), n_state);
    
    return ln;
}


lextree_t *lextree_build (kbcore_t *kbc, wordprob_t *wordprob, int32 n_word, s3cipid_t *lc)
{
    mdef_t *mdef;
    dict_t *dict;
    tmat_t *tmat;
    dict2pid_t *d2p;
    s3ssid_t *ldiph_lc;
    lextree_t *lextree;
    lextree_lcroot_t *lcroot;
    int32 n_lc, n_node, n_ci, n_sseq, pronlen, ssid, prob, ci, rc, wid, np, n_st;
    lextree_node_t *ln=0, **parent, **ssid2ln;
    gnode_t *gn=0;
    bitvec_t *ssid_lc;
    int32 i, j, k, p;
    
    mdef = kbc->mdef;
    dict = kbc->dict;
    tmat = kbc->tmat;
    d2p = kbc->dict2pid;
    n_ci = mdef_n_ciphone (mdef);
    n_sseq = mdef_n_sseq (mdef);
    n_st = mdef_n_emit_state (mdef);
    
    lextree = (lextree_t *) ckd_calloc (1, sizeof(lextree_t));
    lextree->root = NULL;
    
    /* Table mapping from root level ssid to lexnode (temporary) */
    ssid2ln = (lextree_node_t **) ckd_calloc (n_sseq, sizeof(lextree_node_t *));
    
    /* ssid_lc[ssid] = bitvec indicating which lc's this (root) ssid is entered under */
    ssid_lc = (bitvec_t *) ckd_calloc (n_sseq, sizeof(bitvec_t));
    for (i = 0; i < n_sseq; i++)
	ssid_lc[i] = bitvec_alloc (n_ci);
    
    n_node = 0;
    
    /* Create top-level structures pointing to (shared) lextrees for each left context */
    n_lc = 0;
    lcroot = NULL;
    if (! lc) {
	lextree->n_lc = 0;
	lextree->lcroot = NULL;
	
	parent = (lextree_node_t **) ckd_calloc (1, sizeof(lextree_node_t *));
    } else {
	for (n_lc = 0; IS_S3CIPID(lc[n_lc]); n_lc++);
	assert (n_lc > 0);
	
	lextree->n_lc = n_lc;
	lcroot = (lextree_lcroot_t *) ckd_calloc (n_lc, sizeof(lextree_lcroot_t));
	lextree->lcroot = lcroot;
	
	for (i = 0; i < n_lc; i++) {
	    lcroot[i].lc = lc[i];
	    lcroot[i].root = NULL;
	}
	
	parent = (lextree_node_t **) ckd_calloc (n_lc, sizeof(lextree_node_t *));
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
	    if (! lc) {
		ln = lextree_node_alloc (wid, prob, 1, d2p->internal[wid][0], n_st,
					 dict_pron(dict, wid, 0));
		ln->hmm.tp = tmat->tp[mdef_pid2tmatid (mdef, ci)];	/* Assuming CI tmat!! */
		
		lextree->root = glist_add_ptr (lextree->root, (void *) ln);
		n_node++;
	    } else {
		np = 0;
		for (j = 0; j < n_lc; j++) {
		    ssid = d2p->single_lc[ci][(int)lc[j]];
		    
		    /* Check if this ssid already allocated for another lc */
		    for (k = 0; (k < np) && (parent[k]->ssid != ssid); k++);
		    if (k >= np) {	/* Not found; allocate new node */
			ln = lextree_node_alloc (wid, prob, 1, ssid, n_st, ci);
			ln->hmm.tp = tmat->tp[mdef_pid2tmatid (mdef, ci)];
			
			lextree->root = glist_add_ptr (lextree->root, (void *) ln);
			n_node++;
			
			lcroot[j].root = glist_add_ptr (lcroot[j].root, (void *) ln);
			parent[np++] = ln;
		    } else {	/* Already exists; link to lcroot[j] */
			lcroot[j].root = glist_add_ptr (lcroot[j].root, (void *)parent[k]);
		    }
		}
	    }
	} else {
	    assert (pronlen > 1);
	    
	    /* Multi-phone word; allocate root node(s) first, if not already present */
	    if (! lc) {
		ssid = d2p->internal[wid][0];
		ci = dict_pron(dict, wid, 0);
		
		/* Check if this ssid already allocated for another word */
		for (gn = lextree->root; gn; gn = gnode_next(gn)) {
		    ln = (lextree_node_t *) gnode_ptr (gn);
		    if ((ln->ssid == ssid) && ln->composite && NOT_S3WID(ln->wid))
			break;
		}
		if (! gn) {
		    ln = lextree_node_alloc (BAD_S3WID, prob, 1, ssid, n_st, ci);
		    ln->hmm.tp = tmat->tp[mdef_pid2tmatid (mdef, ci)];
		    
		    lextree->root = glist_add_ptr (lextree->root, (void *) ln);
		    n_node++;
		} else {
		    if (ln->prob < prob)
			ln->prob = prob;
		}
		parent[0] = ln;
		np = 1;
	    } else {
		ci = dict_pron(dict, wid, 0);
		rc = dict_pron(dict, wid, 1);
		ldiph_lc = d2p->ldiph_lc[ci][rc];
		
		np = 0;
		for (j = 0; j < n_lc; j++) {
		    ssid = ldiph_lc[(int)lc[j]];
		    
		    /* Check if ssid already allocated */
		    ln = ssid2ln[ssid];
		    if (! ln) {
			ln = lextree_node_alloc (BAD_S3WID, prob, 0, ssid, n_st, ci);
			ln->hmm.tp = tmat->tp[mdef_pid2tmatid (mdef, ci)];
			lextree->root = glist_add_ptr (lextree->root, (void *) ln);
			n_node++;
			
			ssid2ln[ssid] = ln;
		    } else if (ln->prob < prob)
			ln->prob = prob;
		    
		    /* Check if lexnode already entered under lcroot[lc] */
		    if (bitvec_is_clear (ssid_lc[ssid], lc[j])) {
			lcroot[j].root = glist_add_ptr (lcroot[j].root, (void *) ln);
			bitvec_set (ssid_lc[ssid], lc[j]);
		    }
		    
		    /* Add to parent_list if not already there */
		    for (k = 0; (k < np) && (parent[k]->ssid != ssid); k++);
		    if (k >= np)
			parent[np++] = ln;
		}
	    }
	    
	    /* Rest of the pronunciation except the final one */
	    for (p = 1; p < pronlen-1; p++) {
		ssid = d2p->internal[wid][p];
		ci = dict_pron(dict, wid, p);
		
		/* Check for ssid under each parent (#parents(np) > 1 only when p==1) */
		for (j = 0; j < np; j++) {
		    for (gn = parent[j]->children; gn; gn = gnode_next(gn)) {
			ln = (lextree_node_t *) gnode_ptr(gn);
			
			if ((ln->ssid == ssid) && (! ln->composite)) {
			    assert (NOT_S3WID(ln->wid));
			    break;
			}
		    }
		    if (gn)
			break;
		}
		
		if (! gn) {	/* Not found under any parent; allocate new node */
		    ln = lextree_node_alloc (BAD_S3WID, prob, 0, ssid, n_st, ci);
		    ln->hmm.tp = tmat->tp[mdef_pid2tmatid (mdef, ci)];
		    
		    for (j = 0; j < np; j++)
			parent[j]->children = glist_add_ptr (parent[j]->children, (void *)ln);
		    n_node++;
		} else {	/* Already exists under parent[j] */
		    if (ln->prob < prob)
			ln->prob = prob;
		    
		    k = j;
		    
		    /* Child was not found under parent[0..k-1]; add */
		    for (j = 0; j < k; j++)
			parent[j]->children = glist_add_ptr(parent[j]->children, (void *)ln);
		    
		    /* Parents beyond k have not been checked; add if not present */
		    for (j = k+1; j < np; j++) {
			if (! glist_chkdup_ptr (parent[j]->children, (void *)ln))
			    parent[j]->children = glist_add_ptr(parent[j]->children, (void *)ln);
		    }
		}
		
		parent[0] = ln;
		np = 1;
	    }
	    
	    /* Final (leaf) node, no sharing */
	    ssid = d2p->internal[wid][p];
	    ci = dict_pron(dict, wid, p);
	    ln = lextree_node_alloc (wid, prob, 1, ssid, n_st, ci);
	    ln->hmm.tp = tmat->tp[mdef_pid2tmatid (mdef, ci)];
	    
	    for (j = 0; j < np; j++)
		parent[j]->children = glist_add_ptr (parent[j]->children, (void *)ln);
	    n_node++;
	}
    }
    
    lextree->n_node = n_node;
    
    lextree->active = (lextree_node_t **) ckd_calloc (n_node, sizeof(lextree_node_t *));
    lextree->next_active = (lextree_node_t **) ckd_calloc (n_node, sizeof(lextree_node_t *));
    lextree->n_active = 0;
    lextree->n_next_active = 0;
    
    ckd_free ((void *) ssid2ln);
    for (i = 0; i < n_sseq; i++)
	bitvec_free (ssid_lc[i]);
    ckd_free ((void *) ssid_lc);
    ckd_free (parent);
    
    return lextree;
}


static int32 lextree_subtree_free (lextree_node_t *ln, int32 level)
{
    gnode_t *gn;
    lextree_node_t *ln2;
    int32 k;
    
    k = 0;

    /* Free subtrees below this node */
    for (gn = ln->children; gn; gn = gnode_next(gn)) {
	ln2 = (lextree_node_t *) gnode_ptr (gn);
	k += lextree_subtree_free (ln2, level+1);
    }
    glist_free (ln->children);
    ln->children = NULL;
    
    /* Free this node, but for level-1 nodes only if reference count drops to 0 */
    if ((level != 1) || (--ln->ssid == 0)) {
	myfree ((void *) ln, sizeof(lextree_node_t));
	k++;
    }
    
    return k;
}


/*
 * This is a bit tricky because of the replication of root nodes for different left-contexts.
 * A node just below the root can have more than one parent.  Use reference counts to know how
 * many parents refer to such a node.  Use the lextree_node_t.ssid field for such counts.
 */
void lextree_free (lextree_t *lextree)
{
    gnode_t *gn, *gn2;
    lextree_node_t *ln, *ln2;
    int32 i, k;
    
    if (lextree->n_lc > 0) {
	for (i = 0; i < lextree->n_lc; i++)
	    glist_free (lextree->lcroot[i].root);
	
	ckd_free (lextree->lcroot);
    }
    
    /* Build reference counts for level-1 nodes (nodes just below root) */
    for (gn = lextree->root; gn; gn = gnode_next(gn)) {
	ln = (lextree_node_t *) gnode_ptr (gn);
	for (gn2 = ln->children; gn2; gn2 = gnode_next(gn2)) {
	    ln2 = (lextree_node_t *) gnode_ptr (gn2);
	    if (ln2->composite >= 0) {	/* First visit to this node */
		ln2->composite = -1;
		ln2->ssid = 1;		/* Ref count = 1 */
	    } else
		ln2->ssid++;		/* Increment ref count */
	}
    }
    
    /* Free lextree */
    k = 0;
    for (gn = lextree->root; gn; gn = gnode_next(gn)) {
	ln = (lextree_node_t *) gnode_ptr (gn);
	k += lextree_subtree_free (ln, 0);
    }
    glist_free (lextree->root);
    
    ckd_free ((void *) lextree->active);
    ckd_free ((void *) lextree->next_active);
    
    if (k != lextree->n_node)
	E_ERROR("#Nodes allocated(%d) != #nodes freed(%d)\n", lextree->n_node, k);
    
    ckd_free (lextree);
}


void lextree_ci_active (lextree_t *lextree, bitvec_t ci_active)
{
    lextree_node_t **list, *ln;
    int32 i;
    
    list = lextree->active;
    
    for (i = 0; i < lextree->n_active; i++) {
	ln = list[i];
	bitvec_set (ci_active, ln->ci);
    }
}


void lextree_ssid_active (lextree_t *lextree, int32 *ssid, int32 *comssid)
{
    lextree_node_t **list, *ln;
    int32 i;
    
    list = lextree->active;
    
    for (i = 0; i < lextree->n_active; i++) {
	ln = list[i];
	if (ln->composite)
	    comssid[ln->ssid] = 1;
	else
	    ssid[ln->ssid] = 1;
    }
}


void lextree_utt_end (lextree_t *l, kbcore_t *kbc)
{
    mdef_t *mdef;
    lextree_node_t *ln;
    int32 i;
    
    mdef = kbcore_mdef (kbc);
    
    for (i = 0; i < l->n_active; i++) {	/* The inactive ones should already be reset */
	ln = l->active[i];
	
	ln->frame = -1;
	hmm_clear (&(ln->hmm), mdef_n_emit_state(mdef));
    }

    l->n_active = 0;
    l->n_next_active = 0;
}


static void lextree_node_print (lextree_node_t *ln, dict_t *dict, FILE *fp)
{
    fprintf (fp, "wid(%d)pr(%d)com(%d)ss(%d)", ln->wid, ln->prob, ln->composite, ln->ssid);
    if (IS_S3WID(ln->wid))
	fprintf (fp, "%s", dict_wordstr(dict, ln->wid));
    fprintf (fp, "\n");
}


static void lextree_subtree_print (lextree_node_t *ln, int32 level, dict_t *dict, FILE *fp)
{
    int32 i;
    gnode_t *gn;
    
    for (i = 0; i < level; i++)
	fprintf (fp, "    ");
    lextree_node_print (ln, dict, fp);
    
    for (gn = ln->children; gn; gn = gnode_next(gn)) {
	ln = (lextree_node_t *) gnode_ptr (gn);
    	lextree_subtree_print (ln, level+1, dict, fp);
    }
}


void lextree_dump (lextree_t *lextree, dict_t *dict, FILE *fp)
{
    gnode_t *gn;
    lextree_node_t *ln;
    int32 i;
    
    for (gn = lextree->root; gn; gn = gnode_next(gn)) {
	ln = (lextree_node_t *) gnode_ptr (gn);
    	lextree_subtree_print (ln, 0, dict, fp);
    }
    
    if (lextree->n_lc > 0) {
	for (i = 0; i < lextree->n_lc; i++) {
	    fprintf (fp, "lcroot %d\n", lextree->lcroot[i].lc);
	    for (gn = lextree->lcroot[i].root; gn; gn = gnode_next(gn)) {
		ln = (lextree_node_t *) gnode_ptr(gn);
		lextree_node_print (ln, dict, fp);
	    }
	}
    }
}


void lextree_enter (lextree_t *lextree, s3cipid_t lc, int32 cf,
		    int32 inscore, int32 inhist, int32 thresh)
{
    glist_t root;
    gnode_t *gn;
    lextree_node_t *ln;
    int32 nf, scr;
    int32 i, n;
    hmm_t *hmm;
    
    nf = cf+1;
    
    /* Locate root nodes list */
    if (lextree->n_lc == 0) {
	assert (NOT_S3CIPID(lc));
	root = lextree->root;
    } else {
	for (i = 0; (i < lextree->n_lc) && (lextree->lcroot[i].lc != lc); i++);
	assert (i < lextree->n_lc);
	
	root = lextree->lcroot[i].root;
    }
    
    /* Enter root nodes */
    n = lextree->n_next_active;
    for (gn = root; gn; gn = gnode_next(gn)) {
	ln = (lextree_node_t *) gnode_ptr (gn);
	
	hmm = &(ln->hmm);
	
	scr = inscore + ln->prob;
	if ((scr >= thresh) && (hmm->in.score < scr)) {
	    hmm->in.score = scr;
	    hmm->in.history = inhist;
	    
	    if (ln->frame != nf) {
		ln->frame = nf;
		lextree->next_active[n++] = ln;
	    }
	} /* else it is activated separately */
    }
    lextree->n_next_active = n;
}


void lextree_active_swap (lextree_t *lextree)
{
    lextree_node_t **t;
    
    t = lextree->active;
    lextree->active = lextree->next_active;
    lextree->next_active = t;
    lextree->n_active = lextree->n_next_active;
    lextree->n_next_active = 0;
}


int32 lextree_hmm_eval (lextree_t *lextree, kbcore_t *kbc, ascr_t *ascr, int32 frm, FILE *fp)
{
    int32 best, wbest, n_st;
    int32 i, k;
    lextree_node_t **list, *ln;
    mdef_t *mdef;
    dict2pid_t *d2p;
    
    mdef = kbc->mdef;
    d2p = kbc->dict2pid;
    n_st = mdef_n_emit_state (mdef);
    
    list = lextree->active;
    best = MAX_NEG_INT32;
    wbest = MAX_NEG_INT32;
    
    if (fp) {
	for (i = 0; i < lextree->n_active; i++) {
	    ln = list[i];
	    assert (ln->frame == frm);
	    
	    lextree_node_print (ln, kbc->dict, fp);
	    
	    if (! ln->composite)
		k = hmm_dump_vit_eval (&(ln->hmm), n_st,
				       mdef->sseq[ln->ssid], ascr->sen, fp);
	    else
		k = hmm_dump_vit_eval (&(ln->hmm), n_st,
				       d2p->comsseq[ln->ssid], ascr->comsen, fp);
	    
	    if (best < k)
		best = k;
	    
	    if (IS_S3WID(ln->wid)) {
		if (wbest < k)
		    wbest = k;
	    }
	}
    } else {
	if (n_st == 3) {
	    for (i = 0; i < lextree->n_active; i++) {
		ln = list[i];
		assert (ln->frame == frm);
		
		if (! ln->composite)
		  {

		    k = hmm_vit_eval_3st (&(ln->hmm), mdef->sseq[ln->ssid], ascr->sen);
		  }
		else
		  {
		    k = hmm_vit_eval_3st (&(ln->hmm), d2p->comsseq[ln->ssid], ascr->comsen);

		  }
		if (best < k)
		    best = k;
		
		if (IS_S3WID(ln->wid)) {
		    if (wbest < k)
			wbest = k;
		}
	    }
	} else if (n_st == 5) {
	    for (i = 0; i < lextree->n_active; i++) {
		ln = list[i];
		assert (ln->frame == frm);
		
		if (! ln->composite)
		    k = hmm_vit_eval_5st (&(ln->hmm), mdef->sseq[ln->ssid], ascr->sen);
		else
		    k = hmm_vit_eval_5st (&(ln->hmm), d2p->comsseq[ln->ssid], ascr->comsen);
		
		if (best < k)
		    best = k;
		
		if (IS_S3WID(ln->wid)) {
		    if (wbest < k)
			wbest = k;
		}
	    }
	} else
	    E_FATAL("#State= %d unsupported\n", n_st);
    }
    
    lextree->best = best;
    lextree->wbest = wbest;
    
    if (fp) {
	fprintf (fp, "Fr %d  #active %d  best %d  wbest %d\n",
		 frm, lextree->n_active, best, wbest);
	fflush (fp);
    }
    
    return best;
}


void lextree_hmm_histbin (lextree_t *lextree, int32 bestscr, int32 *bin, int32 nbin, int32 bw)
{
    lextree_node_t **list, *ln;
    hmm_t *hmm;
    int32 i, k;
    glist_t *binln;
    gnode_t *gn;
    
    binln = (glist_t *) ckd_calloc (nbin, sizeof(glist_t));
    
    list = lextree->active;
    
    for (i = 0; i < lextree->n_active; i++) {
	ln = list[i];
	hmm = &(ln->hmm);
	
	k = (bestscr - hmm->bestscore) / bw;
	if (k >= nbin)
	    k = nbin-1;
	assert (k >= 0);
	
	bin[k]++;
	binln[k] = glist_add_ptr (binln[k], (void *) ln);
    }
    
    /* Reorder the active lexnodes in APPROXIMATELY descending scores */
    k = 0;
    for (i = 0; i < nbin; i++) {
	for (gn = binln[i]; gn; gn = gnode_next(gn)) {
	    ln = (lextree_node_t *) gnode_ptr (gn);
	    list[k++] = ln;
	}
	glist_free (binln[i]);
    }
    assert (k == lextree->n_active);
    
    ckd_free ((void *) binln);
}


void lextree_hmm_propagate (lextree_t *lextree, kbcore_t *kbc, vithist_t *vh,
			    int32 cf, int32 th, int32 pth, int32 wth,int32 *phn_heur_list,int32 heur_beam,int32 heur_type)
{
    mdef_t *mdef;
    int32 nf, newscore, newHeurScore;
    lextree_node_t **list, *ln, *ln2;
    hmm_t *hmm, *hmm2;
    gnode_t *gn;
    int32 i, n;

    /* Code for heursitic score */
    static int32 maxNewHeurScore=MAX_NEG_INT32;
    static int32 lastfrm=-1;
    int32 hth;

    mdef = kbcore_mdef(kbc);
    
    nf = cf+1;
    
    list = lextree->active;
    
    n = lextree->n_next_active;
    assert (n == 0);
    for (i = 0; i < lextree->n_active; i++) {
	ln = list[i];
	hmm = &(ln->hmm);
	
	if (ln->frame < nf) {
	    if (hmm->bestscore >= th) {		/* Active in next frm */
		ln->frame = nf;
		lextree->next_active[n++] = ln;
	    } else {				/* Deactivate */
	      ln->frame = -1;
		hmm_clear (hmm, mdef_n_emit_state(mdef));
	    }
	}
	
	if (NOT_S3WID(ln->wid)) {		/* Not a leaf node */
#if 0
	    if (((cf % 3) == 0) || (hmm->out.score < pth))
		continue;			/* HMM exit score not good enough */
#else 
	    if (hmm->out.score < pth)
		continue;			/* HMM exit score not good enough */
#endif
	    if(heur_type >0){
	      if (cf!=lastfrm) {
		lastfrm=cf;
		maxNewHeurScore=MAX_NEG_INT32;
	      }
	      for (gn = ln->children; gn; gn = gnode_next(gn)) {
		ln2 = gnode_ptr(gn);
                
		newHeurScore = hmm->out.score + (ln2->prob - ln->prob) + phn_heur_list[(int32)ln2->ci];
		if (maxNewHeurScore < newHeurScore)  maxNewHeurScore = newHeurScore;
	      }
	      hth = maxNewHeurScore + heur_beam;
	    }

	    /* Transition to each child */
	    for (gn = ln->children; gn; gn = gnode_next(gn)) {
		ln2 = gnode_ptr(gn);
		hmm2 = &(ln2->hmm);
		
		newscore = hmm->out.score + (ln2->prob - ln->prob);
		newHeurScore = newscore + phn_heur_list[(int32)ln2->ci];

#if 0
                E_INFO("Newscore %d, Heurscore %d, hth %d, CI phone ID %d, CI phone Str %s\n",newscore, newHeurScore, hth, ln2->ci,mdef_ciphone_str(mdef,ln2->ci));
#endif

 		if (((heur_type==0)||                          /*If the heuristic type is 0, 
								by-pass heuristic score OR */
		     (heur_type>0 && newHeurScore >= hth)) &&  /*If the heuristic type is other 
								and if the heur score is within threshold*/
		    (newscore >= th) &&                       /*If the score is smaller than the
								phone score, prune away*/
		    (hmm2->in.score < newscore)               /*Just the Viterbi Update */
		   ) {
		    hmm2->in.score = newscore;
		    hmm2->in.history = hmm->out.history;
	    
		    if (ln2->frame != nf) {
			ln2->frame = nf;
			lextree->next_active[n++] = ln2;
		    }
		}
	    }
	} else {			/* Leaf node; word exit */
	    if (hmm->out.score < wth)
		continue;		/* Word exit score not good enough */

	    if(hmm->out.history==-1)
	      E_ERROR("Hmm->out.history equals to -1 with score %d and active idx %d, lextree->type\n",hmm->out.score,i,lextree->type);

	    /* Rescore the LM prob for this word wrt all possible predecessors */
	    vithist_rescore (vh, kbc, ln->wid, cf,
			     hmm->out.score - ln->prob, hmm->out.history, lextree->type);
	}
    }
    
    lextree->n_next_active = n;
}
