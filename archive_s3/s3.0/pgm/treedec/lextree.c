/*
 * lextree.c -- Maintaining the lexical tree.
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
 * 15-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include "lextree.h"


static void lextree_dump (FILE *fp, glist_t root, dict_t *dict, mdef_t *mdef, int32 ind)
{
    gnode_t *gn;
    lextree_node_t *node;
    char phonestr[128];
    int32 i;
    
    for (gn = root; gn; gn = gnode_next(gn)) {
	node = (lextree_node_t *) gnode_ptr(gn);
	if (mdef_phone_str(mdef, node->hmm.pid, phonestr) < 0)
	    E_FATAL("mdef_phone_str(%d) failed\n", node->hmm.pid);
	
	for (i = 0; i < ind; i++)
	    fprintf (fp, "\t");
	fprintf (fp, "%s", phonestr);
	if (LEXTREE_NODEID_VALID(node->id))
	    fprintf (fp, "[%s.%d]", dict_wordstr(dict, LEXTREE_NODEID2WID(node->id)),
		     LEXTREE_NODEID2PPOS(node->id));
	fprintf (fp, "\n");
	fflush (fp);

	lextree_dump (fp, node->children, dict, mdef, ind+1);
    }
}


/*
 * NOTE: This version uses only the SILENCE phone context for the beginning and end of each word.
 */
glist_t lextree_build (dict_t *dict, mdef_t *mdef, bitvec_t active, int32 flatdepth)
{
    glist_t root, subtree;
    gnode_t *gn;
    int32 w, p, l;
    s3cipid_t sil, lc, rc, ci;
    s3pid_t pid;
    lextree_node_t *node, *parent;
    hmm_t *hmm;
    int32 *n_node;
    int32 pos, maxpronlen;
    
    assert (mdef && dict);
    assert (flatdepth >= 0);
    
    sil = mdef->sil;
    if (NOT_CIPID(sil)) {
	E_ERROR("No silence phone in mdef\n");
	return NULL;
    }
    if (NOT_WID(dict->startwid) || NOT_WID(dict->finishwid)) {
	E_ERROR("START or FINISH word(%s or %s) missing from dict\n", START_WORD, FINISH_WORD);
	return NULL;
    }
    
    maxpronlen = 0;
    for (w = 0; w < dict_size(dict); w++) {
	if (maxpronlen < dict_pronlen(dict, w))
	    maxpronlen = dict_pronlen(dict, w);
    }
    n_node = (int32 *) ckd_calloc (maxpronlen+1, sizeof(int32));
    
    if (flatdepth > maxpronlen)
	flatdepth = maxpronlen;
    
    root = NULL;
    for (w = 0; w < dict_size(dict); w++) {
	/* Skip START and FINISH words; they are just dummies at the beginning/end of utt */
	if ((dict_basewid(dict, w) == dict->startwid) ||
	    (dict_basewid(dict, w) == dict->finishwid))
	    continue;
	
	/* Skip words marked as inactive */
	if (active && bitvec_is_clear (active, w))
	    continue;
	
	l = dict_pronlen(dict,w) - 1;
	if (l > flatdepth)
	    l = flatdepth;
	
	ci = dict_pron (dict, w, 0);
	lc = sil;	/* Silence left context for the beginning of the word */
	
	/*
	 * Tree portion; shared prefix portion of pronunciation.  The last phone of the word
	 * must not be in the shared tree.
	 */
	for (p = 0, subtree = root, parent = NULL; p < l; p++) {
	    rc = dict_pron (dict, w, p+1);
	    pos = (p == 0) ? WORD_POSN_BEGIN : WORD_POSN_INTERNAL;
	    pid = mdef_phone_id_nearest (mdef, ci, lc, rc, pos);
	    if (NOT_PID(pid))
		E_FATAL("mdef_phone_id(%s,%s,%s,%d) failed\n",
			mdef_ciphone_str(mdef, ci),
			mdef_ciphone_str(mdef, lc),
			mdef_ciphone_str(mdef, rc), pos);
	    
	    /* Look for an HMM in subtree list matching the current pid */
	    for (gn = subtree; gn; gn = gnode_next(gn)) {
		node = (lextree_node_t *) gnode_ptr(gn);
		if ((node->hmm.pid == pid) && LEXTREE_NODEID_INVALID(node->id))
		    break;
	    }
	    if (! gn) {
		/* Didn't find pid; create one and add to parent's children list */
		node = (lextree_node_t *) mymalloc (sizeof(lextree_node_t));
		if (! parent)
		    root = glist_add_ptr(root, (void *)node);
		else
		    parent->children = glist_add_ptr(parent->children, (void *)node);
		
		node->hmm.pid = pid;
		node->hmm.state = (hmm_state_t *) ckd_calloc (mdef->n_emit_state,
							      sizeof(hmm_state_t));
		node->id = LEXTREE_NODEID_NONE;
		node->children = NULL;
		
		n_node[p]++;
	    }
	    
	    parent = node;
	    subtree = node->children;
	    lc = ci;
	    ci = rc;
	}
	
	/* Flat portion; new nodes created for the rest of the pronunciation */
	l = dict_pronlen(dict,w) - 1;
	if ((w >= LEXTREE_NODEID_MAXWID) || (l >= LEXTREE_NODEID_MAXPPOS))
	    E_FATAL("Panic! Cannot encode wid > %d, ppos > %d in lextree node-id\n",
		    LEXTREE_NODEID_MAXWID-1, LEXTREE_NODEID_MAXPPOS-1);
	
	for (; p <= l; p++) {
	    if (p < l) {
		rc = dict_pron (dict, w, p+1);
		pos = (p == 0) ? WORD_POSN_BEGIN : WORD_POSN_INTERNAL;
	    } else {
		rc = sil;
		pos = (p == 0) ? WORD_POSN_SINGLE : WORD_POSN_END;
	    }
	    pid = mdef_phone_id_nearest (mdef, ci, lc, rc, pos);
	    if (NOT_PID(pid))
		E_FATAL("mdef_phone_id(%s,%s,%s,%d) failed\n",
			mdef_ciphone_str(mdef, ci),
			mdef_ciphone_str(mdef, lc),
			mdef_ciphone_str(mdef, rc), pos);
	    
	    node = (lextree_node_t *) mymalloc (sizeof(lextree_node_t));
	    if (! parent)
		root = glist_add_ptr(root, (void *)node);
	    else
		parent->children = glist_add_ptr(parent->children, (void *)node);
	    
	    node->hmm.pid = pid;
	    node->hmm.state = (hmm_state_t *) ckd_calloc (mdef->n_emit_state,
							  sizeof(hmm_state_t));
	    node->id = LEXTREE_NODEID (w, l-p);
	    node->children = NULL;
	    
	    n_node[p]++;
	    
	    parent = node;
	    subtree = node->children;
	    lc = ci;
	    ci = rc;
	}
    }
    
    printf ("Tree nodes/depth: ");
    for (p = 0; p <= maxpronlen; p++)
	printf (" %d(%d)", n_node[p], p);
    printf ("\n");
    fflush (stdout);
    
    ckd_free (n_node);
    
    return root;
}


/*
 * Set all the senones in the lextree to the active state in the given bitvec.
 */
static void lextree_set_senactive (kb_t *kb)
{
    gnode_t *gn;
    lextree_node_t *node;
    mdef_t *mdef;
    bitvec_t active;
    
    mdef = kb->mdef;
    active = kb->am->sen_active;

    bitvec_clear_all (active, kb->am->sen->n_sen);
    for (gn = kb->lextree_active; gn; gn = gnode_next(gn)) {
	node = (lextree_node_t *) gnode_ptr(gn);
	senone_set_active (active, mdef->phone[node->hmm.pid].state, mdef->n_emit_state);
    }

    kb->n_sen_eval += bitvec_count_set (active, kb->am->sen->n_sen);
}


/*
 * Reset the entire lextree (i.e., HMMs in it) to the inactive state.
 */
static void lextree_clear (mdef_t *mdef,
			   glist_t subtree)	/* In/Out: Subtree to be recursively reset */
{
    gnode_t *gn;
    lextree_node_t *node;
    
    for (gn = subtree; gn; gn = gnode_next(gn)) {
	node = (lextree_node_t *) gnode_ptr(gn);
	hmm_clear (mdef, &(node->hmm));
	
	lextree_clear (mdef, node->children);
    }
}


/*
 * Evaluate active HMMs in lextree.  Consider only HMM-internal transitions; i.e.,
 * not cross-HMM transitions in lextree.
 * Return value: The resulting best HMM state score.
 */
static int32 lextree_hmm_eval (kb_t *kb,
			       int32 frm)	/* In: Current frame */
{
    int32 bestscore, hmmscore;
    gnode_t *gn;
    lextree_node_t *node;
    int32 n;
    
    /* Step the active HMMs through this frame */
    bestscore = MAX_NEG_INT32;
    for (gn = kb->lextree_active, n = 0; gn; gn = gnode_next(gn), n++) {
	node = (lextree_node_t *) gnode_ptr(gn);
	assert (node->hmm.active == frm);
	
	/* Internal states first */
	hmmscore = hmm_vit_eval (kb->mdef, kb->tmat, &(node->hmm), kb->am->senscr);
	if (bestscore < hmmscore)
	    bestscore = hmmscore;
	
	/* Exit state.  (Assume that exit state score must be <= best internal score.) */
	hmm_vit_eval_exit (kb->mdef, kb->tmat, &(node->hmm));
    }

    kb->n_hmm_eval += n;
    
    return bestscore;
}


/*
 * Find the best Viterbi history and LM score for the given word.  The candidate histories are
 * all those that ended at the given frame.  Return value: LM score for the given word.
 */
static int32 besthist_lscr(kb_t *kb,		/* In: Models, search data structures */
			   s3wid_t wid,		/* In: Word whose LM score is sought */
			   int32 frm,		/* In: Frame at which the immediate predecessors
						   (i.e., candidate histories) of wid ended */
			   vithist_t **outhist)	/* Out: Updated Viterbi history for word */
{
    int32 lmwid, l1, l2, bestscore, score, bestlscr, lscr;
    gnode_t *gn;
    vithist_t *hist, *lmhist, *besthist;
    int32 n;
    
    lmwid = kb->dict2lmwid[wid];
    assert (IS_LMWID(lmwid));

    bestscore = MAX_NEG_INT32;
    for (gn = kb->vithist[frm], n = kb->wordmax; gn && (n > 0); gn = gnode_next(gn), --n) {
	/* Find two-word history and get LM score */
	hist = (vithist_t *) gnode_ptr(gn);
	lmhist = hist->lmhist;
	assert (lmhist);
	l2 = kb->dict2lmwid[lmhist->id];
	
	lmhist = lmhist->hist ? lmhist->hist->lmhist : NULL;
	l1 = lmhist ? kb->dict2lmwid[lmhist->id] : BAD_LMWID;
	
	lscr = lm_tg_score (kb->lm, l1, l2, lmwid);
	score = hist->scr + lscr;
	
	if (bestscore < score) {
	    bestscore = score;
	    bestlscr = lscr;
	    besthist = hist;
	}
    }
    
    *outhist = besthist;
    return (bestlscr);
}


/*
 * Determine which nodes remain active for next frame.  Also, evaluate cross-HMM transitions in
 * lextree, and activate successors as needed.  Four possible cases to be handled:
 *   1. tree -> tree (i.e., not yet entered the flat structure).
 * 	Viterbi transition from exit state of source to entry state of dest. HMM.
 *   2. tree -> flat.
 * 	Vit. trans. as in 1, but also compute and include LM score for current word.
 * 	(LM score may be accumulated right away, or spread over remaining HMMs for this word.)
 *   3. flat -> flat (remaining within flat part of lextree).
 * 	Vit. trans. as in 1, but if LM score spread over entire flatlex.
 *   4. flat -> empty (exiting word); enter into Viterbi history structure.
 * Filler word exits have to be handled separately.  For a given segmentation, only the one best
 * filler word needs to be maintained (assuming filler words are indistinguishable as phonetic
 * context providers; but then we're not using cross-word phonetic context anyway).  Also, filler
 * exits have to be replicated for all possible histories.
 * 
 * Replace the active list for the current frame (kb->lextree_active) with that for the next, as
 * a side-effect.
 */
static void lextree_hmm_trans (kb_t *kb,
			       int32 frm,	/* Current frame */
			       int32 bestscore)	/* Of all HMM states just evaluated */
{
    int32 th, wth, ppos, sf, lscr, origscore, newscore;
    gnode_t *gn, *gn2;
    lextree_node_t *node, *node2;
    hmm_t *hmm, *hmm2;
    glist_t next_active;
    s3wid_t wid;
    vithist_t *newhist;
    glist_t filler_exit;
    
    /* Pruning thresholds */
    th = bestscore + kb->beam;
    wth = bestscore + kb->wordbeam;
    assert (wth >= th);
    
    /* Lextree nodes active next frame */
    next_active = NULL;
    
    /* List of filler nodes exiting this frame; for special handling of filler exits */
    filler_exit = NULL;
    
    for (gn = kb->lextree_active; gn; gn = gnode_next(gn)) {
	node = (lextree_node_t *) gnode_ptr(gn);
	hmm = &(node->hmm);
	
	if (hmm->bestscore < th) {
	    if (hmm->active <= frm)		/* Not activated for next frame */
		hmm_clear (kb->mdef, hmm);
	    continue;
	}
	
	/* Activate for next frame if not already so */
	if (hmm->active <= frm) {
	    hmm->active = frm+1;
	    next_active = glist_add_ptr (next_active, (void *)node);
	}
	
	if (hmm->out.score < th)		/* Exit score not good enough */
	    continue;
	
	if (LEXTREE_NODEID_INVALID(node->id)) {	/* tree -> tree/flat */
	    assert (node->children);
	    
	    for (gn2 = node->children; gn2; gn2 = gnode_next(gn2)) {
		node2 = (lextree_node_t *) gnode_ptr(gn2);
		hmm2 = &(node2->hmm);
		
		if (LEXTREE_NODEID_VALID(node2->id)) {	    /* tree -> flat; need LM score */
		    wid = LEXTREE_NODEID2WID(node2->id);
		    
		    /* Get LM score and spread over remaining phones */
		    if (NOT_LMWID(kb->dict2lmwid[wid])) {
			assert (dict_filler_word(kb->dict, wid));
			lscr = fillpen(kb->fillpen, wid) / (LEXTREE_NODEID2PPOS(node2->id) + 1);
			newhist = hmm->out.hist;
			newscore = hmm->out.score;
		    } else {
			lscr = besthist_lscr (kb, wid, hmm->out.hist->frm, &newhist) /
			    (LEXTREE_NODEID2PPOS(node2->id) + 1);
			newscore = hmm->out.score + (newhist->scr - hmm->out.hist->scr);
		    }
		    
		    if (hmm_vit_trans_comp (newscore + lscr, newhist, lscr, hmm2, frm+1))
			next_active = glist_add_ptr(next_active, (void *)node2);
		} else {			/* still within tree -> tree */
		    if (hmm_vit_trans (hmm, hmm2, frm+1))
			next_active = glist_add_ptr(next_active, (void *)node2);
		}
	    }
	} else {
	    gn2 = node->children;
	    if (gn2) {				/* flat -> flat transition */
		assert (! gnode_next(gn2));	/* Can only have one child in lexflat */
		
		node2 = (lextree_node_t *) gnode_ptr(gn2);
		hmm2 = &(node2->hmm);
		
		if (hmm_vit_trans_comp (hmm->out.score + hmm->out.data,	/* Include LM score */
					hmm->out.hist, hmm->out.data, hmm2, frm+1)) {
		    next_active = glist_add_ptr(next_active, (void *)node2);
		}
	    } else if (hmm->out.score >= wth) {	/* Word end; note in Viterbi history */
		wid = LEXTREE_NODEID2WID(node->id);
		ppos = LEXTREE_NODEID2PPOS(node->id);
		assert (IS_WID(wid) && (ppos == 0));
		
		/* Do not note first exit of this word instance */
		sf = hmm->out.hist->frm + 1;
		if (kb->wd_last_sf[wid] < sf)
		    kb->wd_last_sf[wid] = sf;
		else {
		    if (IS_LMWID(kb->dict2lmwid[wid])) {
			kb->vithist[frm] = vithist_append (kb->vithist[frm], wid, frm,
							   hmm->out.score, hmm->out.hist, NULL);
		    } else {			/* Filler word */
			/* Check if another filler with same segmentation already found */
			for (gn2 = filler_exit; gn2; gn2 = gnode_next(gn2)) {
			    node2 = (lextree_node_t *) gnode_ptr(gn2);
			    hmm2 = &(node2->hmm);
			    
			    if (hmm->out.hist->frm == hmm2->out.hist->frm) {
				assert (hmm->out.hist == hmm2->out.hist);
				if (hmm->out.score > hmm2->out.score)
				    gnode_ptr(gn2) = (void *)node;
				break;
			    }
			}
			if (! gn2)		/* No existing filler with same segmentation */
			    filler_exit = glist_add_ptr (filler_exit, (void *)node);
			/* Filler word exits handled after all candidates have been sorted out */
		    }
		}
	    }
	}
    }
    
    /*
     * Viterbi history entries for filler word exits.  Each filler word is replicated for all
     * possible predecessors (i.e., predecessors with same end time).
     */
    for (gn = filler_exit; gn; gn = gnode_next(gn)) {
	node = (lextree_node_t *) gnode_ptr(gn);
	hmm = &(node->hmm);

	assert (LEXTREE_NODEID_VALID(node->id));
	wid = LEXTREE_NODEID2WID(node->id);
	
	/* Score with which the current (best) predecessor ended */
	origscore = hmm->out.hist->scr;
	
	for (gn2 = kb->vithist[hmm->out.hist->frm]; gn2; gn2 = gnode_next(gn2)) {
	    newhist = (vithist_t *) gnode_ptr(gn2);
	    newscore = hmm->out.score + (newhist->scr - origscore);

	    if (newscore >= wth)
		kb->vithist[frm] = vithist_append (kb->vithist[frm], wid, frm, newscore,
						   newhist, newhist->lmhist);
	}
    }
    
    glist_free (filler_exit);
    glist_free (kb->lextree_active);
    kb->lextree_active = next_active;
}


/*
 * Transition from all word exits that just occurred to all the lextree root nodes.
 * Update the active list (kb->lextree_active) for the next frame as a side-effect.
 * NOTE: Assumes that the history nodes have been sorted, best score first.
 */
static void lextree_root_trans (kb_t *kb,
				int32 frm)	/* In: Frame at which words just exited */
{
    vithist_t *hist, *best;
    gnode_t *gn;
    lextree_node_t *node;
    hmm_t *hmm;
    s3wid_t wid;
    int32 lscr;
    
    best = gnode_ptr(kb->vithist[frm]);		/* The first is the best */
    
    for (gn = kb->lextree_root; gn; gn = gnode_next(gn)) {
	node = (lextree_node_t *) gnode_ptr(gn);
	hmm = &(node->hmm);
	
	if (LEXTREE_NODEID_INVALID(node->id)) {	/* Tree internal node */
	    if (hmm_vit_trans_comp (best->scr, best, 0, hmm, frm+1))
		kb->lextree_active = glist_add_ptr (kb->lextree_active, (void *)node);
	} else {		/* Flatlex node; need to include LM score */
	    wid = LEXTREE_NODEID2WID(node->id);
	    
	    if (NOT_LMWID(kb->dict2lmwid[wid])) {
		assert (dict_filler_word (kb->dict, wid));
		lscr = fillpen (kb->fillpen, wid);
		hist = best;
	    } else {
		lscr = besthist_lscr (kb, wid, frm, &hist);
	    }
	    
	    /* Spread the LM score over the phones */
	    lscr /= (LEXTREE_NODEID2PPOS(node->id) + 1);
	    
	    if (hmm_vit_trans_comp (hist->scr + lscr, hist, lscr, hmm, frm+1))
		kb->lextree_active = glist_add_ptr (kb->lextree_active, (void *)node);
	}
    }
}


void lextree_vit_start (kb_t *kb, char *uttid)
{
    int32 w;
    
    lextree_clear(kb->mdef, kb->lextree_root);
    
    /* Initialize the Viterbi history with the start word; pretent we're at frame -1 */
    assert (kb->vithist[-1] == NULL);
    kb->vithist[-1] = vithist_append (NULL, kb->dict->startwid, -1, 0, NULL, NULL);
    
    /* Start new search with this history; pretend we're at frame -1 */
    assert (kb->lextree_active == NULL);
    lextree_root_trans (kb, -1);

    kb->n_sen_eval = 0;
    lextree_set_senactive (kb);
    
    /* Clear the most recent start time for any given word */
    for (w = 0; w < dict_size(kb->dict); w++)
	kb->wd_last_sf[w] = -1;
    
    kb->n_hmm_eval = 0;
}


int32 lextree_vit_frame (kb_t *kb, int32 frm, char *uttid)
{
    int32 bestscore;
    glist_t newlist;
    
    /* Evaluate active HMMs */
    bestscore = lextree_hmm_eval (kb, frm);
    
    /* Check for underflow/overflow problems (Hack!!) */
    if ((bestscore <= LOGPROB_ZERO) || (bestscore + kb->beam <= LOGPROB_ZERO)) {
	E_ERROR("%s: Best HMM score(%d)/pruning threshold(%d) underflow @fr %d; raise logbase\n",
		uttid, bestscore, bestscore + kb->beam, frm);
	glist_free (kb->lextree_active);
	kb->lextree_active = NULL;
	
	return MAX_NEG_INT32;
    }
    if (bestscore > 0)
	E_WARN("%s: Best HMM score >0(%d) @fr %d; overflow??\n", uttid, bestscore, frm);

    /* Cross-HMM transitions within tree, and word exits */
    lextree_hmm_trans (kb, frm, bestscore);

    /* Sort the words just exited in descending order of score */
    if (kb->vithist[frm]) {
	if ((newlist = vithist_sort (kb->vithist[frm])) == NULL)
	    E_FATAL("%s: vithist_sort failed @fr %d\n", uttid, frm);
	
	glist_free (kb->vithist[frm]);
	kb->vithist[frm] = newlist;
	
	/* Transitions to lextree root nodes from words just exited */
	lextree_root_trans (kb, frm);
    }
    
    lextree_set_senactive (kb);
    
    return bestscore;
}


vithist_t *lextree_vit_end (kb_t *kb, int32 frm, char *uttid)
{
    int32 f, lscr;
    vithist_t *hist;
    
    f = frm-1;
    if (! kb->vithist[f]) {
	/*
	 * No words exited in the utterance final frame; find the most recent entries.
	 * (Alternatively, one could force entries in the final frame.)
	 */
	for (f = frm-2; (f >= 0) && (! kb->vithist[f]); --f);
	if (f < 0) {
	    E_ERROR("%s: Empty Viterbi history\n", uttid);
	    return NULL;
	}
	E_ERROR("%s: No Viterbi history in final frame; using frame %d\n", uttid, f);
    }
    
    /* Find the best possible transition to the utterance FINISH WORD */
    lscr = besthist_lscr (kb, kb->dict->finishwid, f, &hist);
    
    /* Create dummy FINISH_WORD node in Viterbi history */
    assert (kb->vithist[frm] == NULL);
    kb->vithist[frm] = vithist_append (NULL, kb->dict->finishwid, frm, hist->scr + lscr,
				       hist, NULL);
    hist = gnode_ptr(kb->vithist[frm]);
    
    return hist;
}


#if _LEXTREE_TEST_
static void usagemsg(char *pgm)
{
    E_INFO("Usage: %s mdef dict fillerdict flatdepth\n", pgm);
    exit(0);
}


main (int32 argc, char *argv[])
{
    dict_t *dict;
    mdef_t *mdef;
    int32 flatdepth;
    glist_t root;
    
    if (argc != 5)
	usagemsg(argv[0]);
    if ((sscanf (argv[4], "%d", &flatdepth) != 1) || (flatdepth < 0))
	usagemsg(argv[0]);
    
    mdef = mdef_init (argv[1]);
    dict = dict_init (mdef, argv[2], argv[3], 0);
    
    E_INFO("Building lextree\n");
    root = lextree_build (dict, mdef, NULL, flatdepth);
    
    if (! root)
	E_FATAL("lextree_build() failed\n");
    else
	lextree_dump (stdout, root, dict, mdef, 0);
}
#endif
