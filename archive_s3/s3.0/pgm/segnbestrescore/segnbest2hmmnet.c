/*
 * segnbest2hmmnet.c -- Create a complete triphone HMM network from a segmented
 * 			N-best list file.
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
 * 26-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libfeat/libfeat.h>

#include "dict.h"
#include "gauden.h"
#include "senone.h"
#include "tmat.h"
#include "lm.h"
#include "fillpen.h"
#include "hmm.h"
#include "misc.h"
#include "cmn.h"
#include "agc.h"


#define MAXWDS		4096
#define MAX_UNIQ_HIST	4096


typedef struct {
    mdef_t *mdef;
    dict_t *dict;
    lm_t *lm;
    fillpen_t *fillpen;
    gauden_t *gauden;
    senone_t *sen;
    tmat_t *tmat;
    s3lmwid_t *dict2lmwid;
    am_eval_t *am;
} kb_t;


/*
 * The search graph (DAG) consists of word nodes and links.  The following structures
 * represent these two types of entities.
 */
typedef struct wnode_s {
    s3wid_t wid;
    struct wnode_s *lmbase;	/* For bypassing "filler" nodes that are transparent to
				   the LM.  If this is a non-filler node, lmbase points to
				   itself, else to the nearest non-filler predecessor */
    glist_t succ;		/* List of successor word nodes */
    struct wnode_s *pred;	/* Predecessor node for the purpose of figuring out the
				   LM history and phonetic context.  Even though there may
				   actually be several predecessors, all of them present
				   the same history.  So we keep just one predecessor
				   link */
    glist_t p0list;		/* List of initial phones for this word node.  (It is a
				   list because there may be many right context instances
				   for a single-phone word. */
} wnode_t;

typedef struct wlink_s {
    wnode_t *dst;
} wlink_t;
static int32 n_wnode, n_wlink;


static void wnode_free (void *data)
{
    glist_myfree (((wnode_t *)data)->succ, sizeof(wlink_t));
    glist_free (((wnode_t *)data)->p0list);
}


static void wnet_free (glist_t wdag)
{
    glist_apply (wdag, wnode_free);
    glist_myfree (wdag, sizeof(wnode_t));
}


static void link_wnodes (wnode_t *src, wnode_t *dst)
{
    wlink_t *wl;
    
    wl = (wlink_t *) mymalloc (sizeof(wlink_t));
    wl->dst = dst;

    src->succ = glist_add (src->succ, (void *)wl);
    n_wlink++;
}


/* Convert a word-id array to a wnode_t list */
static glist_t seg2wnodelist (glist_t dag, s3wid_t *wid, int32 nwid)
{
    wnode_t *wn, *prev;
    int32 i;
    
    assert (nwid > 0);

    prev = NULL;
    for (i = 0; i < nwid; i++) {
	wn = (wnode_t *) mymalloc (sizeof(wnode_t));

	wn->wid = wid[i];
	wn->lmbase = NULL;	/* Undefined at the moment */
	wn->succ = NULL;
	wn->pred = prev;

	dag = glist_add (dag, (void *)wn);
	
	if (prev)
	    link_wnodes (prev, wn);
	prev = wn;
    }

    return dag;
}


/*
 * Convert rwid[] into hist[]:
 * rwid[w] = Input list of nodes that end in wid w and end the currnet segment.
 * hist[i] = Output list of nodes ending the current segment that present
 * 	the SAME HISTORY to the next segment.
 * Return value: #valid entries in hist[].
 */
static int32 build_hist (glist_t *hist, glist_t *rwid, dict_t *d)
{
    int32 n_hist;
    int32 w, i;
    gnode_t *gn;
    wnode_t *wn;
    s3wid_t bw;
    static int32 *wid2hist = NULL;
    
    n_hist = 0;
    if (! wid2hist)
	wid2hist = (int32 *) ckd_calloc (d->n_word, sizeof(int32));
    
    for (w = 0; w < d->n_word; w++) {
	if (! rwid[w])
	    continue;
	
	if (! dict_filler_word (d, w)) {
	    hist[n_hist++] = rwid[w];
	} else {
	    for (i = 0; i < d->n_word; i++)
		wid2hist[i] = -1;
	    
	    for (gn = rwid[w]; gn; gn = gn->next) {
		wn = (wnode_t *)(gn->data);
		assert (wn->lmbase != wn);

		if (! wn->lmbase) {
		    /* Must be dummy root node */
		    assert (wn->wid == d->silwid);
		    assert (! wn->pred);
		    bw = wn->wid;
		} else
		    bw = dict_basewid(d, wn->lmbase->wid);

		if (wid2hist[bw] < 0)
		    wid2hist[bw] = n_hist++;
		
		hist[wid2hist[bw]] = glist_add (hist[wid2hist[bw]], (void *)wn);
	    }

	    glist_free (rwid[w]);
	}

	rwid[w] = NULL;
    }
	    
    return n_hist;
}


/*
 * Build a wordnet from a segmented nbest file.  The segmented N-best file contains
 * a number of time-segments for an utterance.  For each segment, there are a number
 * of alternative (Nbest) segment-hypotheses, including a right-context word for each
 * hypothesis.  This context is ignored.  Therefore, there may be duplicate hypotheses
 * in a given segment, which have to be ignored.  Each segment has a number of possible
 * unique histories, determined by the previous segment(s).  Since we use triphone
 * acoustic models and N-gram language models, the entire history to the beginning of
 * the utterance is not relevant.  Rather, a unique history is decided by the rightmost
 * base phoneme of the complete history, and (N-2) most recent base words.  (For now,
 * N=3; i.e. trigram LMs.)
 * Each hypothesis in the segment is linked to every hypothesis in the previous segment,
 * as follows:  A single copy of the hypothesis can be shared by (or linked to) more
 * than one hypothesis in the previous segment, iff the latter result in the same
 * history.
 */
static glist_t segnbest2wordnet (kb_t *kb, char *file)
{
    FILE *fp;
    dict_t *dict;
    char line[32768];
    int32 ntok, nwid;
    char *wstr[MAXWDS];
    s3wid_t wid[MAXWDS];
    wnode_t *root;		/* Root node of word DAG */
    wnode_t *seghead, *segtail;	/* Head/tail nodes of a segment hypothesis instance */
    glist_t dag;		/* List of nodes in entire search DAG */
    glist_t *lwid, *rwid;	/* lwid[w] = list of nodes that begin with wid w, and
				   rwid[w] = nodes that end with w in current segment */
    glist_t *hist;		/* hist[i] = list of ending nodes in previous segment
				   that present the SAME HISTORY to the current segment */
    int32 n_hist;		/* #Currently valid entries in hist[] above */
    int32 segsf, prev_segsf;
    int32 n_seghyp;
    gnode_t *gn;
    wnode_t *wn;
    int32 i, j;
    
    E_INFO("Loading %s\n", file);
    
    if ((fp = fopen(file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return NULL;
    }
    
    /* Skip header */
    if (fgets (line, sizeof(line), fp) == NULL) {
	E_ERROR("Empty file: %s\n", file);
	fclose (fp);
	return NULL;
    }
    
    dict = kb->dict;
    
    lwid = (glist_t *) ckd_calloc (dict->n_word, sizeof(glist_t));
    rwid = (glist_t *) ckd_calloc (dict->n_word, sizeof(glist_t));
    hist = (glist_t *) ckd_calloc (MAX_UNIQ_HIST, sizeof(glist_t));

    prev_segsf = -1;
    n_hist = 0;
    n_wnode = 0;
    n_wlink = 0;
    n_seghyp = 0;
    
    /* Create a DAG with a single root node with a SILENCE_WORD */
    root = (wnode_t *) mymalloc (sizeof(wnode_t));
    root->wid = dict->silwid;
    root->lmbase = NULL;
    root->succ = NULL;	/* Empty list of links from this node */
    root->pred = NULL;
    dag = glist_add (NULL, (void *) root);
    rwid[dict->silwid] = glist_add (NULL, (void *) root);
    n_wnode++;
    
    while (fgets (line, sizeof(line), fp) != NULL) {
	if ((ntok = str2words (line, wstr, MAXWDS)) < 0)
	    E_FATAL("Increase MAXWDS (from %d)\n", MAXWDS);	/* Too many tokens */
	
	if ((ntok == 1) && (strcmp (wstr[0], "End") == 0))
	    break;
	
	assert ((ntok >= 5) && (ntok & 0x1));
	
	if (sscanf (wstr[1], "%d", &segsf) != 1)
	    E_FATAL("Bad startframe value: %s\n", wstr[1]);

	/*
	 * Convert words in line to word-ids.
	 * Format: score sf word sf word ... sf right-context-word; skip the context word.
	 */
	for (i = 2, nwid = 0; i < ntok-2; i += 2, nwid++) {
	    wid[nwid] = dict_wordid (dict, wstr[i]);
	    if (NOT_WID(wid[nwid]))
		E_FATAL("Unknown word: %s\n", wstr[i]);
	}
	
	if (segsf == prev_segsf) {
	    /*
	     * Check if this word sequence has been seen before in this segment
	     * (i.e., check lists beginning with wid[0]).  If seen, skip it.
	     */
	    for (gn = lwid[wid[0]]; gn; gn = gn->next) {
		wn = (wnode_t *) (gn->data);
		for (j = 0; (j < nwid) && wn && (wid[j] == wn->wid); j++) {
		    wn = (wn->succ) ? ((wlink_t *) (wn->succ->data))->dst : NULL;
		}
		if ((! wn) && (j >= nwid))
		    break;
	    }
	    if (gn)		/* Duplicate segment; skip it */
		continue;
	} else {
	    /*
	     * Current segment ended, start a new one.
	     * First, forget about the history list for the previous segment.
	     */
	    for (i = 0; i < n_hist; i++) {
		glist_free (hist[i]);
		hist[i] = NULL;
	    }
	    
	    /* Build history list from rwid in the current segment, and clear rwid */
	    n_hist = build_hist (hist, rwid, dict);

	    /* Clear lwid */
	    for (i = 0; i < dict->n_word; i++) {
		glist_free (lwid[i]);
		lwid[i] = NULL;
	    }
	    
#if 0
	    E_INFO("Frm %5d: %6d hyp instances\n", prev_segsf, n_seghyp);
	    E_INFO("Frm %5d: %6d unique histories\n", segsf, n_hist);
#endif
	    prev_segsf = segsf;
	    n_seghyp = 0;
	}
	
	/*
	 * Tack this new segment to the end of all possible predecessor segments.
	 * A new copy of the new segment is needed whenever the predecessor segment
	 * presents a new phonemic left context, OR a new linguistic context of
	 * (ngram-2) words.
	 * In short, for each unique history, create a separate instance of the new
	 * segment and link all predecessors with that history to the new instance.
	 */
	for (i = 0; i < n_hist; i++) {
	    dag = seg2wnodelist (dag, wid, nwid);
	    n_wnode += nwid;

	    /* Find the head and tail wnodes of the segment just created */
	    segtail = (wnode_t *)(dag->data);
	    for (j = 0, gn = dag; j < nwid-1; j++, gn = gn->next);
	    seghead = (wnode_t *)(gn->data);
	    
	    /* Successor links from all predecessors with the i-th history to seghead */
	    for (gn = hist[i]; gn; gn = gn->next)
		link_wnodes ((wnode_t *)(gn->data), seghead);

	    /*
	     * Predecessor link from seghead to ONE predecessor with this history.
	     * One link is enough since all predecessors in hist[i] have the same history.
	     */
	    seghead->pred = (wnode_t *)(hist[i]->data);
	    
	    /* Enter seghead into wl_leftwid, and the tail into wl_rightwid */
	    lwid[seghead->wid] = glist_add (lwid[seghead->wid], (void *)seghead);
	    rwid[segtail->wid] = glist_add (rwid[segtail->wid], (void *)segtail);

	    /* Update lmbase for all the new nodes */
	    for (wn = seghead; wn; ) {
		wn->lmbase = (! dict_filler_word (dict, wn->wid)) ? wn : wn->pred->lmbase;
		wn = (wn->succ) ? ((wlink_t *) (wn->succ->data))->dst : NULL;
	    }
	    
	    n_seghyp++;
	}
    }
    fclose (fp);
#if 0
    E_INFO("Frm %5d: %6d hyp instances\n", prev_segsf, n_seghyp);
#endif

    /*
     * Link all nodes to a single FINISH_WORD node.  (HACK!! This assumes that finish
     * word model is context-independent.)
     */
    wn = (wnode_t *) mymalloc (sizeof(wnode_t));
    wn->wid = dict->finishwid;
    wn->lmbase = wn;
    wn->succ = NULL;
    wn->pred = NULL;	/* No unique predecessor; THIS FIELD MUST NOT BE TOUCHED */

    dag = glist_add (dag, (void *)wn);
    n_wnode++;
    
    for (i = 0; i < dict->n_word; i++) {
	for (gn = rwid[i]; gn; gn = gn->next)
	    link_wnodes ((wnode_t *)(gn->data), wn);
    }
    
    for (i = 0; i < dict->n_word; i++) {
	glist_free (lwid[i]);
	lwid[i] = NULL;
	glist_free (rwid[i]);
	rwid[i] = NULL;
    }
    for (i = 0; i < n_hist; i++)
	glist_free (hist[i]);

    ckd_free(lwid);
    ckd_free(rwid);
    ckd_free(hist);

#if 0
    for (gn = dag; gn; gn = gn->next)
	printf ("%s\n", dict_wordstr (dict, ((wnode_t *)(gn->data))->wid));
#endif

    return dag;
}


/*
 * Phonetic equivalent representation of wordnet.  Another DAG with phone nodes and links.
 */
typedef struct pnode_s {
    s3wid_t wid;	/* Word-id; only valid on last phone of word; otherwise BAD_WID */
    s3pid_t pid;	/* Triphone id */
    hmm_t hmm;		/* Search HMM structure */
    glist_t succ;	/* List of successor phone nodes */
} pnode_t;

typedef struct plink_s {
    pnode_t *dst;
    int32 lscr;
} plink_t;
static int32 n_pnode, n_plink;


static void pnode_dump (FILE *fp, int32 frm, pnode_t *pn, kb_t *kb)
{
    char buf[128];
    
    fprintf (fp, "%5d:", frm);
    mdef_phone_str (kb->mdef, pn->pid, buf);
    fprintf (fp, "\t%s", buf);
    if (IS_WID(pn->wid))
	fprintf (fp, "\t%s", dict_wordstr(kb->dict, pn->wid));
    fprintf (fp, "\n");
    
    hmm_dump (fp, &(pn->hmm));
    
    fflush (fp);
}


static void pnode_free (void *data)
{
    glist_myfree (((pnode_t *)data)->succ, sizeof(plink_t));
}


static void pnet_free (glist_t pdag)
{
    glist_apply (pdag, pnode_free);
    glist_myfree (pdag, sizeof(pnode_t));
}


/* Allocate HMM data for each pnode */
static void pnet_hmm_alloc (kb_t *kb, glist_t pdag)
{
    gnode_t *gn;
    pnode_t *pn;
    
    for (gn = pdag; gn; gn = gn->next) {
	pn = (pnode_t *)(gn->data);
	hmm_state_alloc (&(pn->hmm));
    }
}


/* Allocate HMM data for each pnode */
static void pnet_hmm_free (kb_t *kb, glist_t pdag)
{
    gnode_t *gn;
    pnode_t *pn;
    
    for (gn = pdag; gn; gn = gn->next) {
	pn = (pnode_t *)(gn->data);
	hmm_state_free (&(pn->hmm));
    }
}


static void link_pnodes (pnode_t *src, pnode_t *dst, int32 lscr)
{
    plink_t *pl;
    
    pl = (plink_t *) mymalloc (sizeof(plink_t));
    pl->dst = dst;
    pl->lscr = lscr;
    
    src->succ = glist_add (src->succ, (void *)pl);
    n_plink++;
}


/*
 * Convert a wordnet into a phone net.  We take advantage of the fact that the nodes in
 * the wordnet DAG nodelist are in reverse topological order.  So, if two nodes w1 and w2
 * appear in that order in wdag, there may be a link from w2 to w1 but there CANNOT be
 * a link from w1 to w2.  Therefore, we can construct the phone DAG by picking off words
 * from wdag in order, transforming them into phone nodes, and linking them to the phone
 * DAG already constructed so far.
 */
static glist_t wnet2pnet (kb_t *kb, glist_t wdag)
{
    dict_t *dict;
    mdef_t *mdef;
    glist_t pdag;
    gnode_t *gwn;
    gnode_t *gwl;
    wnode_t *wn;
    wlink_t *wl;
    int32 pronlen;
    int32 n_rc;
    pnode_t **rc_pn;
    pnode_t *pn, *last_pn;
    gnode_t *p0list;
    s3cipid_t lc, rc, ci;
    int32 lscr;
    int32 i, k, p;
    
    dict = kb->dict;
    mdef = kb->mdef;
    
    pdag = NULL;
    n_pnode = 0;
    n_plink = 0;
    
    rc_pn = (pnode_t **) ckd_calloc (mdef->n_ciphone, sizeof(pnode_t *));
    
    /* Create a pnode for the final wdag node, without any context words */
    wn = (wnode_t *)(wdag->data);
    assert ((! wn->pred) && (! wn->succ));	/* HACK!! */
    assert (wn->wid == dict->finishwid);
    pronlen = dict->word[wn->wid].pronlen;
    if (pronlen != 1)
	E_FATAL("Not prepared to handle FINISH_WORD with pronunciation-length != 1\n");
    pn = (pnode_t *) mymalloc (sizeof(pnode_t));
    pn->wid = wn->wid;
    pn->succ = NULL;
    pn->pid = dict->word[wn->wid].ciphone[0];
    pn->hmm.sen = mdef->phone[pn->pid].state;
    pn->hmm.n_state = mdef->n_emit_state;
    pn->hmm.active = -1;
    n_pnode++;

    pdag = glist_add (pdag, (void *)pn);

    wn->p0list = glist_add (NULL, (void *)pn);
    
    for (gwn = wdag->next; gwn; gwn = gwn->next) {
	wn = (wnode_t *)(gwn->data);
	wn->p0list = NULL;
	
	pronlen = dict->word[wn->wid].pronlen;
	
	/* Create phone nodes right to left; last CIphone first */
	ci = dict->word[wn->wid].ciphone[pronlen-1];

	/* Left context CIphone for the last phone */
	if (pronlen > 1)
	    lc = dict->word[wn->wid].ciphone[pronlen-2];
	else {
	    if (! wn->pred)
		lc = BAD_CIPID;
	    else {
		k = dict->word[wn->pred->wid].pronlen;
		lc = dict->word[wn->pred->wid].ciphone[k-1];
	    }
	}
	
	/* Create phone nodes for each distinct right context, link to successor words */
	for (i = 0; i < mdef->n_ciphone; i++)
	    rc_pn[i] = NULL;
	n_rc = 0;
	assert (wn->succ);
	for (gwl = wn->succ; gwl; gwl = gwl->next) {
	    wl = (wlink_t *)(gwl->data);
	    rc = dict->word[wl->dst->wid].ciphone[0];

	    if (! rc_pn[rc]) {
		rc_pn[rc] = (pnode_t *) mymalloc (sizeof(pnode_t));
		rc_pn[rc]->wid = wn->wid;
		rc_pn[rc]->succ = NULL;
		rc_pn[rc]->pid = mdef_phone_id_nearest(mdef, ci, lc, rc,
						       (pronlen > 1) ?
						       WORD_POSN_END : WORD_POSN_SINGLE);
		rc_pn[rc]->hmm.sen = mdef->phone[rc_pn[rc]->pid].state;
		rc_pn[rc]->hmm.n_state = mdef->n_emit_state;
		rc_pn[rc]->hmm.active = -1;
		n_pnode++;
		n_rc++;
		
		pdag = glist_add (pdag, (void *)(rc_pn[rc]));
	    }

	    /* Find LM score for transition from wn to wl->dst */
	    if (dict_filler_word (dict, wl->dst->wid))
		lscr = fillpen (kb->fillpen, wl->dst->wid);
	    else {
		s3lmwid_t w1, w2, w3;
		wnode_t *wn1, *wn2;
		
		/* Special case for start word */
		if (dict_basewid (dict, wl->dst->wid) == dict->startwid) {
		    assert (wn->wid == dict->silwid);
		    assert (wn->pred == NULL);
		    lscr = 0;
		} else {
		    w3 = kb->dict2lmwid[wl->dst->wid];

		    wn2 = wn->lmbase;
		    assert (wn2);
		    w2 = kb->dict2lmwid[wn2->wid];

		    if ((wn1 = wn2->pred) != NULL)
			wn1 = wn1->lmbase;
		    w1 = wn1 ? kb->dict2lmwid[wn1->wid] : BAD_LMWID;
		    
		    lscr = lm_tg_score (kb->lm, w1, w2, w3);
		}
	    }
	    
	    /* Link rc_pn[rc] to all initial phone instances of wl->dst */
	    for (p0list = wl->dst->p0list; p0list; p0list = p0list->next)
		link_pnodes (rc_pn[rc], (pnode_t *)(p0list->data), lscr);
	}
	
	/* All other phones */
	for (p = pronlen-2; p >= 0; p--) {
	    ci = dict->word[wn->wid].ciphone[p];
	    rc = dict->word[wn->wid].ciphone[p+1];
	    if (p > 0)
		lc = dict->word[wn->wid].ciphone[p-1];
	    else {
		if (! wn->pred)
		    lc = BAD_CIPID;
		else {
		    k = dict->word[wn->pred->wid].pronlen;
		    lc = dict->word[wn->pred->wid].ciphone[k-1];
		}
	    }

	    pn = (pnode_t *) mymalloc (sizeof(pnode_t));
	    pn->wid = BAD_WID;
	    pn->succ = NULL;
	    pn->pid = mdef_phone_id_nearest (mdef, ci, lc, rc,
					     (p > 0) ? WORD_POSN_INTERNAL :
					     WORD_POSN_BEGIN);
	    pn->hmm.sen = mdef->phone[pn->pid].state;
	    pn->hmm.n_state = mdef->n_emit_state;
	    pn->hmm.active = -1;
	    n_pnode++;
	    
	    pdag = glist_add (pdag, (void *)pn);
	    
	    if (p == pronlen-2) {	/* Penultimate phone */
		for (i = 0; i < mdef->n_ciphone; i++)
		    if (rc_pn[i])
			link_pnodes (pn, rc_pn[i], 0);
	    } else
		link_pnodes (pn, last_pn, 0);

	    last_pn = pn;
	}

	if (pronlen > 1)
	    wn->p0list = glist_add (NULL, (void *)last_pn);
	else {
	    wn->p0list = NULL;
	    for (i = 0; i < mdef->n_ciphone; i++)
		if (rc_pn[i])
		    wn->p0list = glist_add (wn->p0list, (void *)(rc_pn[i]));
	}
#if 0
	printf ("pronlen= %3d rc= %2d %s\n", pronlen, n_rc, dict_wordstr (dict, wn->wid));
#endif
    }

    ckd_free (rc_pn);
    
    return pdag;
}


/* BUGCHECK: if the same pnode has been entered twice into the active list */
static void chk_dup_pn (pnode_t **pn, int32 n)
{
    int32 i, j;
    
    for (i = 0; i < n; i++)
	for (j = 0; j < i-1; j++)
	    if (pn[i] == pn[j])
		E_FATAL("Duplicate pnode\n");
}


static void vit_search (kb_t *kb, glist_t pdag, float32 **mfc, int32 nfr, char *uttid)
{
    am_eval_t *am;
    gnode_t *gn;
    pnode_t *pn, *npn;
    plink_t *pl;
    mdef_t *mdef;
    dict_t *dict;
    tmat_t *tmat;
    int32 **tp;
    hmm_t *hmm, *nhmm;
    pnode_t **cur_pn, **next_pn;	/* Current and next pnode active lists */
    int32 n_cur_pn, n_next_pn;
    int32 f, cf, featwin;
    int32 bestscr, hscr, newscore, beam, wordbeam, thresh, wordthresh;
    int32 *senscale;
    int32 i;
    hist_t *hist;
    glist_t hlist;	/* List of history nodes */
    glist_t hyp;	/* Word-sequence hypothesis + assoc. info for the utterance */
    
    mdef = kb->mdef;
    dict = kb->dict;
    tmat = kb->tmat;

    cmn (mfc, nfr, feat_cepsize());
    agc_max (mfc, nfr);
    
    cur_pn = (pnode_t **) ckd_calloc (n_pnode, sizeof(pnode_t *));
    next_pn = (pnode_t **) ckd_calloc (n_pnode, sizeof(pnode_t *));
    n_cur_pn = 0;
    n_next_pn = 0;
    
    /* Initialize pdag; Use the dummy initial silence word to start all initial pnodes */
    gn = pdag;
    pn = (pnode_t *)(gn->data);
    assert (pn->wid == dict->silwid);
    for (gn = pn->succ; gn; gn = gn->next) {
	pl = (plink_t *)(gn->data);
	pn = pl->dst;
	
	hmm_clear (&(pn->hmm));
	hmm_enter (&(pn->hmm), 0, 0, NULL, 0);
	
	/*
	 * HACK!! To allow for a non-existent initial silence, set all state scores to 0
	 * so that we can exit this HMM ASAP.
	 */
	for (i = 0; i < pn->hmm.n_state; i++)
	    pn->hmm.state[i].scr = 0;
	
	cur_pn[n_cur_pn++] = pn;
    }
    
    featwin = feat_window_size();
    am = kb->am;
    
    beam = logs3 (*((float64 *) cmd_ln_access ("-beam")));
    wordbeam = logs3 (*((float64 *) cmd_ln_access ("-nwbeam")));
    
    hlist = NULL;
    
    senscale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));
    
    for (f = featwin, cf = 0; f < nfr-featwin; f++, cf++) {
#if 0
	chk_dup_pn (cur_pn, n_cur_pn);
#endif
#if 0
	for (i = 0; i < n_cur_pn; i++)
	    pnode_dump (stdout, cf, cur_pn[i], kb);
#endif
	/* Mark active senones */
	bitvec_clear_all (am->sen_active, kb->sen->n_sen);
	for (i = 0; i < n_cur_pn; i++)
	    _set_sen_active (am, cur_pn[i]->hmm.sen, cur_pn[i]->hmm.n_state);
	
	senscale[cf] = _am_eval (am, mfc+f);
#if 0
	for (i = 0; i < kb->sen->n_sen; i++) {
	    if (bitvec_is_set(am->sen_active, i))
		printf ("\t%5d %12d\n", i, am->senscr[i] + senscale[cf]);
	}
#endif
	bestscr = (int32)0x80000000;
	for (i = 0; i < n_cur_pn; i++) {
	    tp = tmat->tp[mdef->phone[cur_pn[i]->pid].tmat];
	    
	    hscr = hmm_eval (&(cur_pn[i]->hmm), am->senscr, tp);
	    if (hscr > bestscr)
		bestscr = hscr;
	}
	if (bestscr <= LOGPROB_ZERO)
	    E_FATAL("Bestscore(%d) < LOGPROB_ZERO @frm %d; increase -logbase\n", bestscr);
	
	/* Figure out which HMMs remain or become active next frame */
	thresh = bestscr + beam;
	wordthresh = bestscr + wordbeam;
	n_next_pn = 0;
	for (i = 0; i < n_cur_pn; i++) {
	    pn = cur_pn[i];
	    hmm = &(pn->hmm);
	    
	    if (hmm->bestscr >= thresh) {
		/* This HMM remains active in the next frame */
		if (hmm->active < cf+1) {
		    hmm->active = cf+1;
		    next_pn[n_next_pn++] = pn;
		}
		
		/* Check if this is a final HMM for a word, and whether exited it */
		if (IS_WID(pn->wid)) {
		    if (hmm->state[hmm->n_state].scr >= wordthresh) {
			hist = (hist_t *) mymalloc (sizeof(hist_t));
			hist->wid = pn->wid;
			hist->frm = cf;
			hist->scr = hmm->state[hmm->n_state].scr;
			hist->hist = hmm->state[hmm->n_state].hist;
			hlist = glist_add (hlist, (void *)hist);
			
			/* Replace exit state with new history node, for propagation */
			hmm->state[hmm->n_state].hist = hist;
		    }
		}
		
		/* Transition to next phones */
		if (hmm->state[hmm->n_state].scr >= thresh) {
		    for (gn = pn->succ; gn; gn = gn->next) {
			pl = (plink_t *)(gn->data);
			
			newscore = hmm->state[hmm->n_state].scr + pl->lscr;
			if (newscore >= thresh) {
			    npn = pl->dst;
			    nhmm = &(npn->hmm);
			    
			    if (nhmm->active < cf)
				hmm_clear (nhmm);
			    if (nhmm->active < cf+1)
				next_pn[n_next_pn++] = npn;
			    hmm_enter (nhmm,
				       newscore,
				       hmm->state[hmm->n_state].data,
				       hmm->state[hmm->n_state].hist,
				       cf+1);
			}
		    }
		}
	    }
	}
	
	/* Swap cur_pn and next_pn for next frame */
	{
	    pnode_t **tmp;
	    
	    tmp = cur_pn;
	    cur_pn = next_pn;
	    next_pn = tmp;
	    n_cur_pn = n_next_pn;
	}
    }

    /* Backtrace through history list */
#if 0
    hist_dump (stdout, hlist, dict);
#endif
    hyp = hist_backtrace (hlist, dict->finishwid, cf-1, senscale, dict);
    _hyp_update_lscr (hyp, kb->lm, kb->fillpen, dict, kb->dict2lmwid);
    _hyp_print_detail (stdout, uttid, hyp);
    _hyp_print_base (stdout, uttid, hyp, dict);
    
    glist_myfree (hyp, sizeof(hyp_t));
    glist_myfree (hlist, sizeof(hist_t));
    
    ckd_free (cur_pn);
    ckd_free (next_pn);
    ckd_free (senscale);
}


static void process_utt (kb_t *kb,
			 char *nbestfile,
			 char *cepfile, int32 sf, int32 ef,
			 char *uttid)
{
    glist_t wdag;
    glist_t pdag;
    float32 **mfc;
    int32 nfr, featwin;
    am_eval_t *am;
    
    wdag = segnbest2wordnet (kb, nbestfile);

    if (wdag) {
	pdag = wnet2pnet (kb, wdag);
	E_INFO("%s: word DAG: %d nodes, %d links; phone DAG: %d nodes, %d links\n",
	       uttid, n_wnode, n_wlink, n_pnode, n_plink);
	wnet_free (wdag);
	lm_cache_stats_dump (kb->lm);
	lm_cache_reset (kb->lm);
	pnet_hmm_alloc (kb, pdag);
	
	am = kb->am;
	
	featwin = feat_window_size ();
	if ((nfr = s2mfc_read (cepfile, sf, ef, featwin, &mfc)) < 0)
	    E_FATAL("MFC read (%s) failed\n", uttid);
	E_INFO("%s: %d frames\n", uttid, nfr - (featwin << 1));
	
	vit_search (kb, pdag, mfc, nfr, uttid);
	
	pnet_hmm_free (kb, pdag);
	pnet_free (pdag);
    }
}


static void process_ctl (kb_t *kb, char *ctl)
{
    FILE *fp;
    char uttfile[4096];
    char uttid[4096];
    int32 sf, ef;
    char nbestfile[16384];
    char *nbestdir;
    char *cepdir;
    char cepfile[4096];
    
    if ((fp = fopen(ctl, "r")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctl);
    
    nbestdir = (char *) cmd_ln_access ("-nbestdir");
    assert (nbestdir);
    cepdir = (char *) cmd_ln_access ("-cepdir");
    
    kb->am = _am_eval_init (kb->gauden, kb->sen);
    
    while (_ctl_read (fp, uttfile, &sf, &ef, uttid) >= 0) {
	sprintf (nbestfile, "%s/%s.nbest", nbestdir, uttid);
	
	_ctl2cepfile (uttfile, cepdir, cepfile);
	
	process_utt (kb, nbestfile, cepfile, sf, ef, uttid);
    }
    
    fclose (fp);
}


static arg_t arglist[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-feat",
      ARG_STRING,
      "s3_1x39",
      "Feature vector type (s3_1x39 or s2_4x)" },
    { "-mdef",
      REQARG_STRING,
      NULL,
      "Model definition input file" },
    { "-dict",
      REQARG_STRING,
      NULL,
      "Pronunciation dictionary input file" },
    { "-fdict",
      REQARG_STRING,
      NULL,
      "Filler word pronunciation dictionary input file" },
    { "-mean",
      REQARG_STRING,
      NULL,
      "Mixture gaussian means input file" },
    { "-var",
      REQARG_STRING,
      NULL,
      "Mixture gaussian variances input file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to -var file)" },
    { "-sen2mgaumap",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixw",
      REQARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to -mixw file)" },
    { "-tmat",
      REQARG_STRING,
      NULL,
      "HMM state transition matrix input file" },
    { "-tmatfloor",
      ARG_FLOAT32,
      "0.0001",
      "HMM state transition probability floor (applied to -tmat file)" },
    { "-lm",
      REQARG_STRING,
      NULL,
      "Trigram language model .DMP file" },
    { "-lw",
      ARG_FLOAT32,
      "9.5",
      "Language weight: empirical exponent applied to LM probabilty" },
    { "-wip",
      ARG_FLOAT32,
      "0.2",
      "Word insertion penalty" },
    { "-fpen",
      ARG_STRING,
      NULL,
      "Filler penalty file (overrides silpen and fillpen defaults)" },
    { "-silpen",
      ARG_FLOAT32,
      "0.1",
      "Default probability of silence word" },
    { "-fillpen",
      ARG_FLOAT32,
      "0.05",
      "Default probability of a non-silence filler word" },
    { "-beam",
      ARG_FLOAT64,
      "1e-64",
      "Main pruning beam" },
    { "-nwbeam",
      ARG_FLOAT64,
      "1e-30",
      "Pruning beam for word exit" },
    { "-ctl",
      REQARG_STRING,
      NULL,
      "Control file listing utterances to be processed" },
    { "-nbestdir",
      ARG_STRING,
      ".",
      "Directory of segmented N-best files (for utterances in -ctl file)" },
    { "-cepdir",
      ARG_STRING,
      ".",
      "Directory of mfc files (for utterances in -ctl file)" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


main (int32 argc, char *argv[])
{
    float64 lw, wip;
    char uttid[4096];
    kb_t kb;
    
    unlimit ();
    
    cmd_ln_parse (arglist, argc, argv);
    
    logs3_init ( *((float32 *) cmd_ln_access("-logbase")) );
    feat_init ((char *) cmd_ln_access ("-feat"));
    
    kb.mdef = mdef_init ((char *) cmd_ln_access ("-mdef"));
    kb.dict = dict_init (kb.mdef,
			 (char *) cmd_ln_access ("-dict"),
			 (char *) cmd_ln_access ("-fdict"));
    kb.gauden = gauden_init ((char *) cmd_ln_access ("-mean"),
			     (char *) cmd_ln_access ("-var"),
			     *((float32 *) cmd_ln_access ("-varfloor")));
    kb.sen = senone_init ((char *) cmd_ln_access ("-mixw"),
			  (char *) cmd_ln_access ("-sen2mgaumap"),
			  *((float32 *) cmd_ln_access ("-mixwfloor")));
    kb.tmat = tmat_init ((char *) cmd_ln_access ("-tmat"),
			 *((float32 *) cmd_ln_access ("-tmatfloor")));
    
    lw = *((float32 *) cmd_ln_access("-lw"));
    wip = *((float32 *) cmd_ln_access("-wip"));
    kb.lm = lm_read ((char *) cmd_ln_access ("-lm"), lw, wip);
    kb.fillpen = fillpen_init (kb.dict, (char *) cmd_ln_access ("-fpen"), lw, wip);
    kb.dict2lmwid = _dict2lmwid (kb.dict, kb.lm);
    
    process_ctl (&kb, (char *) cmd_ln_access ("-ctl"));
}
