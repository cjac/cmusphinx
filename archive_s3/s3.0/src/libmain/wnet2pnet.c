/*
 * wnet2pnet.c -- Build a triphone HMM net from a given word net.
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
 * 05-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>

#include "senone.h"
#include "wnet2pnet.h"


static glist_t pnet;
static int32 n_pnode, n_plink;


/*
 * Additional data needed at each word node to build the phone net.
 */
typedef struct wnode_data_s {
    wnode_t *wnode;
    glist_t *lc;	/* lc[p] = list of initial pnode_t with left context = p */
    glist_t *rc;	/* rc[p] = list of final pnode_t with right context = p */
} wnode_data_t;


static void link_pnodes (pnode_t *src, pnode_t *dst)
{
    plink_t *pl;
    
    pl = (plink_t *) mymalloc (sizeof(plink_t));
    pl->dst = dst;
    src->succ = glist_add_ptr (src->succ, (void *)pl);
    
    n_plink++;
}


static plink_t *plink_lookup (pnode_t *src, pnode_t *dst)
{
    gnode_t *gn;
    plink_t *pl;
    
    for (gn = src->succ; gn; gn = gn->next) {
	pl = (plink_t *) gnode_ptr(gn);
	if (pl->dst == dst)
	    return pl;
    }
    
    return NULL;
}


/*
 * Lookup the list of pnodes for an HMM with the same underlying model as the given
 * pid.  Return pointer if found, NULL otherwise.
 */
static pnode_t *plist_lookup_hmm (mdef_t *mdef, glist_t plist, s3pid_t pid)
{	
    gnode_t *gn;
    pnode_t *pn;
    
    for (gn = plist; gn; gn = gnode_next(gn)) {
	pn = (pnode_t *) gnode_ptr(gn);

	if (mdef_hmm_cmp (mdef, pn->hmm.pid, pid) == 0)
	    return pn;
    }

    return NULL;
}


static pnode_t *pnode_create (mdef_t *mdef, s3pid_t pid, s3wid_t wid, int32 pos)
{
    pnode_t *pn;
    
    pn = (pnode_t *) mymalloc (sizeof(pnode_t));
    pn->wid = wid;
    pn->pos = pos;
    pn->hmm.state = (hmm_state_t *) ckd_calloc (mdef->n_emit_state, sizeof(hmm_state_t));
    pn->hmm.pid = pid;
    pn->succ = NULL;
    
    pnet = glist_add_ptr (pnet, (void *) pn);
    n_pnode++;
    
    return pn;
}


/*
 * Build HMM net for the given word.  All the internal links are set up.  But the net
 * is isolated, with no connections to any other word.  However, wnode_data->{lc,rc}
 * are updated with the HMMs at the extreme ends (left and right) so that one can later
 * complete the entire net.
 */
static void wnode2pnet_build (mdef_t *mdef, dict_t *dict, wnode_data_t *wnode_data)
{
    uint8 *lc, *rc;
    wnode_t *wn;
    gnode_t *gn, *gn2;
    wlink_t *wl;
    s3wid_t wid;
    s3pid_t pid;
    s3cipid_t ci, l, r;
    int32 pronlen;
    int32 i, j;
    glist_t p1list, p2list;
    pnode_t *pn, *prevpn;
    
    wn = wnode_data->wnode;
    
    /* Mark all the left and right context for this word node */
    lc = (uint8 *) ckd_calloc (mdef->n_ciphone, sizeof(uint8));
    rc = (uint8 *) ckd_calloc (mdef->n_ciphone, sizeof(uint8));
    
    if (! wn->pred) {		/* No predecessor, pick SILENCE_CIPHONE as left context */
	lc[mdef->sil] = 1;
    } else {
	for (gn = wn->pred; gn; gn = gn->next) {
	    wl = (wlink_t *) gnode_ptr(gn);
	    wid = wl->dst->wid;	/* Predecessor word id */
	    lc[dict_last_phone(dict, wid)] = 1;
	}
    }
    
    if (! wn->succ) {		/* No successor, use SILENCE_CIPHONE as right context */
	rc[mdef->sil] = 1;
    } else {
	for (gn = wn->succ; gn; gn = gn->next) {
	    wl = (wlink_t *) gnode_ptr(gn);
	    wid = wl->dst->wid;
	    rc[dict_first_phone(dict, wid)] = 1;
	}
    }
    
    wnode_data->lc = (glist_t *) ckd_calloc (mdef->n_ciphone, sizeof(glist_t));
    wnode_data->rc = (glist_t *) ckd_calloc (mdef->n_ciphone, sizeof(glist_t));
    
    /* Create phone net for wn->wid */
    wid = wn->wid;
    if ((pronlen = dict_pronlen (dict, wid)) > 1) {
	/* Multi-phone pronunciation; initial phone position, expand left contexts */
	p1list = NULL;	/* List of distinct HMMs allocated for leftmost phone */
	ci = dict_pron(dict, wid, 0);
	r = dict_pron(dict, wid, 1);
	for (i = 0; i < mdef->n_ciphone; i++) {
	    if (! lc[i])
		continue;
	    
	    pid = mdef_phone_id_nearest (mdef, ci, (s3cipid_t)i, r, WORD_POSN_BEGIN);
	    assert (IS_PID(pid));
	    
	    if ((pn = plist_lookup_hmm (mdef, p1list, pid)) == NULL) {
		pn = pnode_create (mdef, pid, wid, 0);
		p1list = glist_add_ptr (p1list, (void *) pn);
	    }
	    
	    wnode_data->lc[i] = glist_add_ptr (wnode_data->lc[i], (void *) pn);
	}

	/* Intermediate phones; known left/right contexts */
	for (i = 1; i < pronlen-1; i++) {
	    p2list = NULL;
	    
	    /* Create HMM node for phone position i */
	    ci = dict_pron (dict, wid, i);
	    l = dict_pron (dict, wid, i-1);
	    r = dict_pron (dict, wid, i+1);

	    pid = mdef_phone_id_nearest (mdef, ci, l, r, WORD_POSN_INTERNAL);
	    assert (IS_PID(pid));
	    
	    pn = pnode_create (mdef, pid, wid, i);
	    p2list = glist_add_ptr (p2list, (void *) pn);
	    
	    /* Link from previous nodes */
	    for (gn = p1list; gn; gn = gn->next) {
		prevpn = (pnode_t *) gnode_ptr(gn);
		link_pnodes (prevpn, pn);
	    }

	    glist_free (p1list);
	    p1list = p2list;
	}
	
	/* Final phone position; expand right context (for the ones present) */
	p2list = NULL;
	ci = dict_pron(dict, wid, pronlen-1);
	l = dict_pron(dict, wid, pronlen-2);
	for (i = 0; i < mdef->n_ciphone; i++) {
	    if (! rc[i])
		continue;
	    
	    pid = mdef_phone_id_nearest (mdef, ci, l, (s3cipid_t)i, WORD_POSN_END);
	    assert (IS_PID(pid));
	    
	    if ((pn = plist_lookup_hmm (mdef, p2list, pid)) == NULL) {
		pn = pnode_create (mdef, pid, wid, pronlen-1);
		p2list = glist_add_ptr (p2list, (void *) pn);
	    }
	    
	    wnode_data->rc[i] = glist_add_ptr (wnode_data->rc[i], (void *) pn);
	}
	/* Link from previous nodes */
	for (gn = p1list; gn; gn = gn->next) {
	    prevpn = (pnode_t *) gnode_ptr(gn);
	    for (gn2 = p2list; gn2; gn2 = gn2->next) {
		pn = (pnode_t *) gnode_ptr(gn2);
		link_pnodes (prevpn, pn);
	    }
	}
	
	glist_free (p1list);
	glist_free (p2list);
    } else {
	/* Single-phone word; handle left/right contexts simultaneously */
	p1list = NULL;
	ci = dict_pron(dict, wid, 0);
	for (i = 0; i < mdef->n_ciphone; i++) {
	    if (! lc[i])
		continue;
	    
	    for (j = 0; j < mdef->n_ciphone; j++) {
		if (! rc[j])
		    continue;
		
		pid = mdef_phone_id_nearest (mdef, ci, (s3cipid_t)i, (s3cipid_t)j,
					     WORD_POSN_SINGLE);
		assert (IS_PID(pid));
		
		if ((pn = plist_lookup_hmm (mdef, p1list, pid)) == NULL) {
		    pn = pnode_create (mdef, pid, wid, 0);
		    p1list = glist_add_ptr (p1list, (void *) pn);
		}
		
		wnode_data->lc[i] = glist_add_ptr (wnode_data->lc[i], (void *) pn);
		wnode_data->rc[j] = glist_add_ptr (wnode_data->rc[j], (void *) pn);
	    }
	}
	
	glist_free (p1list);
    }
    
    ckd_free (lc);
    ckd_free (rc);
}


/*
 * Build cross-word HMM links, taking phonetic context into account:
 * Let the last CIphone in src = l (i.e., the left context for dst), and
 * the first CIphone in dst = r (i.e., the right phonetic context for src).  Then,
 * create links from the HMMs in the glist src->rc[r] to those in dst->lc[l].  But,
 * avoid creating duplicate links, since several entries in a glist may share the
 * same HMM.
 */
static void link_wnodes (dict_t *dict, wnode_data_t *src, wnode_data_t *dst)
{
    s3cipid_t l, r;
    wnode_t *wn;
    gnode_t *gn1, *gn2;
    pnode_t *pn1, *pn2;
    
    /* Find the last phone for the source node (the left context for the destination) */
    wn = src->wnode;
    l = dict_pron (dict, wn->wid, dict_pronlen(dict, wn->wid) - 1);
    
    /* Find the first phone for the dest. node (the right context for the source) */
    wn = dst->wnode;
    r = dict_pron (dict, wn->wid, 0);
    
    /* Link each HMM in src->rc[r] to each in dst->lc[l] */
    for (gn1 = src->rc[r]; gn1; gn1 = gn1->next) {
	pn1 = (pnode_t *) gnode_ptr(gn1);
	
	for (gn2 = dst->lc[l]; gn2; gn2 = gn2->next) {
	    pn2 = (pnode_t *) gnode_ptr(gn2);
	    
	    /* Check if a link already exists */
	    if (! plink_lookup (pn1, pn2))
		link_pnodes (pn1, pn2);
	}
    }
}


/*
 * Convert a wordnet into a phone net.
 */
glist_t wnet2pnet (mdef_t *mdef, dict_t *dict, glist_t wnet,
		   wnode_t *wstart, wnode_t *wend,	/* In: Dummy start/end anchors */
		   pnode_t **pstart, pnode_t **pend)	/* Out: Dummy start/end anchors */
{
    gnode_t *gn, *gn2;
    wnode_t *wn;
    wlink_t *wl;
    int32 n, i, j;
    wnode_data_t *wnode_data;
    pnode_t *pn, *pn2;
    
    if (NOT_CIPID(mdef->sil))
	E_FATAL("%s not defined\n", SILENCE_CIPHONE);
    
    pnet = NULL;
    n_pnode = 0;
    n_plink = 0;

    /* Allocate wnode_data_t prior to building the phone net */
    n = glist_count (wnet) - 2;		/* Skip wstart and wend */
    if (n <= 0) {
	E_ERROR("Empty word net\n");
	return NULL;
    }
    wnode_data = (wnode_data_t *) ckd_calloc (n, sizeof(wnode_data_t));
    
    for (gn = wnet, i = 0; gn; gn = gn->next) {
	wn = (wnode_t *) gnode_ptr(gn);
	if ((wn == wstart) || (wn == wend))
	    continue;		/* Skip the dummy start/end nodes */
	
	wn->data = i;
	
	wnode_data[i].wnode = wn;
	
	wnode2pnet_build (mdef, dict, wnode_data+i);
	i++;
    }
    assert (i == n);
    
    /* Create links between the pnodes created for each word above */
    for (i = 0; i < n; i++) {
	wn = wnode_data[i].wnode;
	
	for (gn = wn->succ; gn; gn = gn->next) {
	    wl = (wlink_t *) gnode_ptr(gn);
	    if ((wnode_t *)wl->dst != wend)
		link_wnodes (dict, wnode_data+i, wnode_data+(wl->dst->data));
	}
    }
    
    /* Add dummy pnode at the beginning of the net */
    pn = pnode_create (mdef, mdef->sil, BAD_WID, 0);
    /* Link it to initial phones of all successors of wstart */
    for (gn = wstart->succ; gn; gn = gn->next) {
	wl = (wlink_t *) gnode_ptr(gn);
	i = wl->dst->data;
	
	for (j = 0; j < mdef->n_ciphone; j++) {
	    for (gn2 = wnode_data[i].lc[j]; gn2; gn2 = gn2->next) {
		pn2 = (pnode_t *) gnode_ptr(gn2);
		if (! plink_lookup (pn, pn2))
		    link_pnodes (pn, pn2);
	    }
	}
    }
    *pstart = pn;
    
    /* Add dummy pnode at the end of the net */
    pn = pnode_create (mdef, mdef->sil, BAD_WID, 0);
    /* Link from the final phones of all predecessors of wend to pn */
    for (gn = wend->pred; gn; gn = gn->next) {
	wl = (wlink_t *) gnode_ptr(gn);
	i = wl->dst->data;
	
	for (j = 0; j < mdef->n_ciphone; j++) {
	    for (gn2 = wnode_data[i].rc[j]; gn2; gn2 = gn2->next) {
		pn2 = (pnode_t *) gnode_ptr(gn2);
		if (! plink_lookup (pn2, pn))
		    link_pnodes (pn2, pn);
	    }
	}
    }
    *pend = pn;
    
    /* Free working data */
    for (i = 0; i < n; i++) {
	for (j = 0; j < mdef->n_ciphone; j++) {
	    glist_free (wnode_data[i].lc[j]);
	    glist_free (wnode_data[i].rc[j]);
	}
	
	ckd_free (wnode_data[i].lc);
	ckd_free (wnode_data[i].rc);
    }
    ckd_free (wnode_data);
    
    E_INFO("%d pnodes, %d plinks\n", n_pnode, n_plink);
    
    return pnet;
}


static void pnode_free (void *data)
{
    pnode_t *pn;
    
    pn = (pnode_t *) data;
    ckd_free (pn->hmm.state);
    glist_myfree (pn->succ, sizeof(plink_t));
}


void pnet_free (glist_t pnet)
{
    glist_apply_ptr (pnet, pnode_free);
    glist_myfree (pnet, sizeof(pnode_t));
}


void pnet_set_senactive (mdef_t *m, glist_t plist, bitvec_t active, int32 n_sen)
{
    gnode_t *gn;
    pnode_t *pn;
    
    bitvec_clear_all (active, n_sen);
    for (gn = plist; gn; gn = gnode_next(gn)) {
	pn = (pnode_t *) gnode_ptr(gn);
	senone_set_active (active, m->phone[pn->hmm.pid].state, m->n_emit_state);
    }
}


/* Used only by the following dump routines */
static mdef_t *tmpmdef;
static dict_t *tmpdict;

static void pnet_dump_pnode (void *data)
{
    pnode_t *pn;
    char buf[4096];
    
    pn = (pnode_t *) data;
    mdef_phone_str (tmpmdef, pn->hmm.pid, buf);
    if (IS_WID(pn->wid))
	printf ("%s.%d.%s\n", dict_wordstr(tmpdict, pn->wid), pn->pos, buf);
    else
	printf ("%s.%d.%s\n", "<>", pn->pos, buf);	/* Dummy node */
    fflush (stdout);
}


static void pnet_dump_plink (void *data)
{
    pnode_t *pn;
    plink_t *pl;
    gnode_t *gn;
    
    pnet_dump_pnode (data);
    pn = (pnode_t *) data;
    for (gn = pn->succ; gn; gn = gn->next) {
	pl = (plink_t *) gnode_ptr(gn);
	printf ("\t\t-> ");
	pnet_dump_pnode (pl->dst);
    }
    fflush (stdout);
}


void pnet_dump (mdef_t *m, dict_t *d, glist_t pnet)
{
    tmpdict = d;
    tmpmdef = m;
    
    E_INFO("pnodes:\n");
    glist_apply_ptr (pnet, pnet_dump_pnode);
    E_INFO("plinks:\n");
    glist_apply_ptr (pnet, pnet_dump_plink);
}
