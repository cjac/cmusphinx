/*
 * wdnet.c -- Build wordnet for alignment
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
 * 28-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include <libmain/wnet2pnet.h>
#include "wdnet.h"


static glist_t wdnet;		/* List of nodes in the word graph */
static int32 n_wnode, n_wlink;


static wnode_t *wnode_create (s3wid_t wid)
{
    wnode_t *wn;
    
    wn = (wnode_t *) mymalloc (sizeof(wnode_t));
    wn->wid = wid;
    wn->succ = NULL;
    wn->pred = NULL;
    wdnet = glist_add_ptr (wdnet, (void *) wn);

    n_wnode++;
    
    return wn;
}


static void link_wnodes (wnode_t *src, wnode_t *dst)
{
    wlink_t *l;
    
    l = (wlink_t *) mymalloc (sizeof(wlink_t));
    l->dst = dst;
    src->succ = glist_add_ptr (src->succ, (void *) l);

    l = (wlink_t *) mymalloc (sizeof(wlink_t));
    l->dst = src;
    dst->pred = glist_add_ptr (dst->pred, (void *) l);

    n_wlink++;
}


static void fillernet_create (int32 nwd, s3wid_t *fwid, int32 n_fwid,
			      glist_t *pred, glist_t *succ)
{
    wnode_t **wn;
    int32 i, j, k;
    gnode_t *gn;

    wn = (wnode_t **) ckd_calloc (n_fwid, sizeof(wnode_t *));

    for (i = 0; i < nwd-1; i++) {
	/* Allocate fillernet nodes */
	for (j = 0; j < n_fwid; j++)
	    wn[j] = wnode_create (fwid[j]);
	
	/* Fully connected fillernet */
	for (j = 0; j < n_fwid; j++)
	    for (k = 0; k < n_fwid; k++)
		link_wnodes (wn[j], wn[k]);

	/* Insert fillernet between words i and i+1 */
	for (gn = pred[i+1]; gn; gn = gn->next)
	    for (j = 0; j < n_fwid; j++)
		link_wnodes ((wnode_t *) gnode_ptr(gn), wn[j]);
	for (gn = succ[i]; gn; gn = gn->next)
	    for (j = 0; j < n_fwid; j++)
		link_wnodes (wn[j], (wnode_t *) gnode_ptr(gn));
    }

    ckd_free (wn);
}


glist_t wdnet_build (dict_t *dict, char **wd, int32 nwd,
		     int32 noalt, int32 nosil, int32 nonoise,
		     wnode_t **start, wnode_t **end)
{
    s3wid_t *wid;	/* IDs for wd[] + surrounding START and FINISH words */
    glist_t *pred;	/* pred[i] = list of nodes in the word net that are predecessors
			   to the given word at position i */
    glist_t *succ;	/* succ[i] = list of nodes in the word net that are successors
			   to the given word at position i */
    s3wid_t *fwid;	/* Optional fillers wids to be inserted, if any */
    int32 n_fwid;	/* #fillers */
    int32 missing;
    s3wid_t w;
    int32 i, j;
    wnode_t *wn;
    gnode_t *gn;
    
    if (nwd == 0) {
	E_ERROR("Empty word sequence argument to wdseq2net()\n");
	return NULL;
    }
    
    n_wnode = 0;
    n_wlink = 0;
    
    /*
     * We always add an initial START_WORD node and a final FINISH_WORD node, even though
     * they may not take part in the search (i.e., if nosil is TRUE).
     */
    wid = (s3wid_t *) ckd_calloc (nwd+2, sizeof(s3wid_t));

    wid[0] = dict_wordid (dict, START_WORD);
    wid[nwd+1] = dict_wordid (dict, FINISH_WORD);
    if (NOT_WID(wid[0]) || NOT_WID(wid[nwd+1]))
	E_FATAL("%s and/or %s not in dictionary\n", START_WORD, FINISH_WORD);

    missing = 0;
    for (i = 0; i < nwd; i++) {
	wid[i+1] = dict_wordid (dict, wd[i]);
	if (NOT_WID(wid[i+1])) {
	    E_ERROR("%s not in dictionary\n", wd[i]);
	    missing = 1;
	} else {
	    if (! noalt)
		wid[i+1] = dict_basewid (dict, wid[i+1]);
	}
    }
    if (missing) {
	ckd_free (wid);
	return NULL;
    }
    
    nwd += 2;
    
    pred = (glist_t *) ckd_calloc (nwd, sizeof(glist_t));
    succ = (glist_t *) ckd_calloc (nwd, sizeof(glist_t));
    wdnet = NULL;
    
    /* Create and add START_WORD node to pred[1] (assume just 1 pronunciation for it) */
    wn = wnode_create (wid[0]);
    pred[1] = glist_add_ptr (pred[1], wn);
    *start = wn;
    
    /* Add each word (except START_WORD and FINISH_WORD) to net */
    for (i = 1; i < nwd-1; i++) {
	if (! noalt) {
	    /* Add every alternative pronunciation for wid[i] to net */
	    for (w = wid[i]; IS_WID(w); w = dict_nextalt(dict, w)) {
		/* Add w to net */
		wn = wnode_create (w);
		pred[i+1] = glist_add_ptr (pred[i+1], wn);
		succ[i-1] = glist_add_ptr (succ[i-1], wn);

		for (gn = pred[i]; gn; gn = gn->next)
		    link_wnodes ((wnode_t *) gnode_ptr(gn), wn);
	    }
	} else {
	    /* Add wid[i] to net */
	    wn = wnode_create (wid[i]);
	    pred[i+1] = glist_add_ptr (pred[i+1], wn);
	    succ[i-1] = glist_add_ptr (succ[i-1], wn);
	    
	    for (gn = pred[i]; gn; gn = gn->next)
		link_wnodes ((wnode_t *) gnode_ptr(gn), wn);
	}
    }
    
    /* Add FINISH_WORD node to succ[nwd-2] (assume just 1 pronunciation for it) */
    wn = wnode_create (wid[nwd-1]);
    succ[nwd-2] = glist_add_ptr (succ[nwd-2], wn);
    for (gn = pred[nwd-1]; gn; gn = gn->next)
	link_wnodes ((wnode_t *) gnode_ptr(gn), wn);
    *end = wn;
    
    /* Add compound words, if any and if allowed */
    if (! noalt) {
	for (i = 1; i < nwd-2; i++) {
	    for (j = i+1; j < nwd-1; j++) {
		/* See if wid[i]..wid[j] is a compound word in the dictionary */
		for (w = dict_wids2compwid (dict, wid+i, j-i+1);
		     IS_WID(w);
		     w = dict_nextalt(dict, w)) {
		    /* Insert compound word w in word net */
		    wn = wnode_create (w);
		    pred[j+1] = glist_add_ptr (pred[j+1], wn);
		    succ[i-1] = glist_add_ptr (succ[i-1], wn);
		    
		    for (gn = pred[i]; gn; gn = gn->next)
			link_wnodes ((wnode_t *) gnode_ptr(gn), wn);
		    for (gn = succ[j]; gn; gn = gn->next)
			link_wnodes (wn, (wnode_t *) gnode_ptr(gn));
		}
	    }
	}
    }
    
    /* Add the optional filler words to the wordnet, if any and if allowed */
    i = dict_filler_start (dict);
    j = dict_filler_end (dict);
    n_fwid = 0;
    fwid = NULL;
    if (i <= j) {
	/* First build the list of filler words */
	fwid = (s3wid_t *) ckd_calloc (j-i+1, sizeof(s3wid_t));
	for (; i <= j; i++) {
	    if (dict_filler_word (dict, i)) {	/* To exclude START and FINISH words */
		w = dict_basewid (dict, i);
		if (w == dict_silwid(dict)) {
		    if (! nosil)
			fwid[n_fwid++] = i;
		} else {
		    if (! nonoise)
			fwid[n_fwid++] = i;
		}
	    }
	}
    }
    if (n_fwid > 0)
	fillernet_create (nwd, fwid, n_fwid, pred, succ);

    ckd_free (wid);
    if (fwid)
	ckd_free (fwid);
    for (i = 0; i < nwd; i++) {
	glist_free (pred[i]);
	glist_free (succ[i]);
    }
    ckd_free(pred);
    ckd_free(succ);
    
    E_INFO("%d wnodes, %d wlinks\n", n_wnode, n_wlink);
    
    return wdnet;
}


static void wlink_free (void *data)
{
    wnode_t *wn;
    
    wn = (wnode_t *) data;
    glist_myfree (wn->succ, sizeof(wlink_t));
    glist_myfree (wn->pred, sizeof(wlink_t));
}


void wdnet_free (glist_t wdnet)
{
    glist_apply_ptr (wdnet, wlink_free);
    glist_myfree (wdnet, sizeof(wnode_t));
}


#if (_WDNET_TEST_)
static dict_t *tmpdict;

static void wdnet_dump_wnode (void *data)
{
    wnode_t *wn;
    
    wn = (wnode_t *)data;
    printf ("%s\n", dict_wordstr(tmpdict, wn->wid));
}


static void wdnet_dump_wlink (void *data)
{
    wnode_t *wn;
    gnode_t *gn;
    wlink_t *wl;
    
    wdnet_dump_wnode (data);
    wn = (wnode_t *) data;
    for (gn = wn->succ; gn; gn = gn->next) {
	wl = (wlink_t *) gnode_ptr(gn);
	printf ("\t\t -> ");
	wdnet_dump_wnode (wl->dst);
    }
}


static void wdnet_dump (dict_t *d, glist_t wdnet)
{
    tmpdict = d;
    
    E_INFO("wnodes:\n");
    glist_apply_ptr (wdnet, wdnet_dump_wnode);
    E_INFO("wlinks:\n");
    glist_apply_ptr (wdnet, wdnet_dump_wlink);
}


main (int32 argc, char *argv[])
{
    mdef_t *m;
    dict_t *d;
    char line[16384];
    char *wp[4096];
    int32 nwd;
    glist_t wnet;
    wnode_t *wstart, *wend;
    int32 noalt, nosil, nonoise;
    int32 n;
    glist_t pnet;
    pnode_t *pstart, *pend;
    
    if (argc != 4)
	E_FATAL("Usage: %s mdef dict fillerdict\n");
    
    m = mdef_init (argv[1]);
    d = dict_init (m, argv[2], argv[3], '_');
    
    wnet = NULL;
    for (;;) {
	printf ("noalt, nosil, nonoise, utt> ");
	fgets (line, sizeof(line), stdin);
	if (sscanf (line, "%d %d %d%n", &noalt, &nosil, &nonoise, &n) != 3) {
	    E_ERROR("Bad line: %s\n", line);
	    continue;
	}
	
	nwd = str2words (line+n, wp, 4095);
	
	if (wnet)
	    wdnet_free (wnet);
	
	wnet = wdnet_build (d, wp, nwd, noalt, nosil, nonoise, &wstart, &wend);
	if (wnet) {
	    wdnet_dump (d, wnet);
	    
	    pnet = wnet2pnet (m, d, wnet, wstart, wend, &pstart, &pend);
	    pnet_dump (m, d, pnet);
	    pnet_free (pnet);
	}
    }
    
    if (wnet)
	wdnet_free (wnet);
}
#endif
