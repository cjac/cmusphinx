/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * vithist.c -- Viterbi history
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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added vithist_free() to free allocated memory
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Added vithist_partialutt_end() to allow backtracking in
 *		the middle of an utterance
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added maxwpf handling.
 * 
 * 24-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "vithist.h"


vithist_t *vithist_init (kbcore_t *kbc, int32 wbeam, int32 bghist)
{
    vithist_t *vh;
    lm_t *lm;
    lmset_t *lmset;
    dict_t *dict;
    wordprob_t *wp;
    int i;
    /*    int n;*/
    int max=-1;

    E_INFO("Initializing Viterbi-history module\n");
    
    vh = (vithist_t *) ckd_calloc (1, sizeof(vithist_t));
    
    vh->entry = (vithist_entry_t **) ckd_calloc (VITHIST_MAXBLKS, sizeof(vithist_entry_t *));
    vh->n_entry = 0;
    
    vh->frame_start = (int32 *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(int32));
    
    vh->bestscore = (int32 *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(int32));
    vh->bestvh = (int32 *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(int32));
    
    vh->wbeam = wbeam;
    vh->bghist = bghist;
    
    lm = kbcore_lm (kbc);
    lmset =kbcore_lmset(kbc);
    dict =kbcore_dict(kbc);

    wp = (wordprob_t *) ckd_calloc (dict_size(dict), sizeof(wordprob_t));    
    if(lmset){
      /* Don't do anything. Allow it to be done in run-time */

      for(i=0;i<kbcore_nlm(kbc);i++){
	if(lm_n_ug(lmset[i].lm)>max){
	  max=lm_n_ug(lmset[i].lm);
	}
      }
      E_INFO("Allocation for viterbi history, finall size %d\n",max);
      vh->lms2vh_root = (vh_lms2vh_t **) ckd_calloc (max,sizeof(vh_lms2vh_t *));

    }else if(lm){
      vh->lms2vh_root = (vh_lms2vh_t **) ckd_calloc (lm_n_ug(lm), sizeof(vh_lms2vh_t *));
    }
    ckd_free ((void *) wp);

    vh->lwidlist = NULL;
    
    return vh;
}


/*
 * Allocate a new entry at vh->n_entry if one doesn't exist and return ptr to it.
 */
static vithist_entry_t *vithist_entry_alloc (vithist_t *vh)
{
    int32 b, l;
    vithist_entry_t *ve;
    
    b = VITHIST_ID2BLK(vh->n_entry);
    l = VITHIST_ID2BLKOFFSET (vh->n_entry);
    
    if (l == 0) {	/* Crossed block boundary; allocate a new block of vithist space */
	if (b >= VITHIST_MAXBLKS)
	    E_FATAL("Viterbi history array exhausted; increase VITHIST_MAXBLKS\n");
	
	assert (vh->entry[b] == NULL);
	
	ve = (vithist_entry_t *) ckd_calloc (VITHIST_BLKSIZE, sizeof(vithist_entry_t));
	vh->entry[b] = ve;
    } else
	ve = vh->entry[b] + l;
    
    vh->n_entry++;
    
    return ve;
}


int32 vithist_utt_begin (vithist_t *vh, kbcore_t *kbc)
{
    vithist_entry_t *ve;
    lm_t *lm;
    dict_t *dict;
    
    lm = kbcore_lm(kbc);
    dict = kbcore_dict(kbc);
    
    assert (vh->n_entry == 0);
    assert (vh->entry[0] == NULL);
    assert (vh->lwidlist == NULL);
    
    /* Create an initial dummy <s> entry.  This is the root for the utterance */
    ve = vithist_entry_alloc (vh);
    
    ve->wid = dict_startwid(dict);
    ve->sf = -1;
    ve->ef = -1;
    ve->ascr = 0;
    ve->lscr = 0;
    ve->score = 0;
    ve->pred = -1;
    ve->type = 0;
    ve->valid = 1;
    ve->lmstate.lm3g.lwid[0] = lm_startwid(lm);
    ve->lmstate.lm3g.lwid[1] = BAD_S3LMWID;
    
    vh->n_frm = 0;
    vh->frame_start[0] = 1;
    vh->bestscore[0] = MAX_NEG_INT32;
    vh->bestvh[0] = -1;
    
    return 0;
}


static int32 vh_lmstate_find (vithist_t *vh, vh_lmstate_t *lms)
{
    vh_lms2vh_t *lms2vh;
    s3lmwid_t lwid;
    gnode_t *gn;
    
    lwid = lms->lm3g.lwid[0];
    if ((lms2vh = vh->lms2vh_root[lwid]) == NULL)
	return -1;

    assert (lms2vh->state == lwid);
    
    lwid = lms->lm3g.lwid[1];
    for (gn = lms2vh->children; gn; gn = gnode_next(gn)) {
	lms2vh = (vh_lms2vh_t *) gnode_ptr (gn);
	if (lms2vh->state == lwid)
	    return lms2vh->vhid;
    }
    
    return -1;
}


/*
 * Enter a new LMstate into the current frame LMstates trees; called ONLY IF not already
 * present.
 */
static void vithist_lmstate_enter (vithist_t *vh, int32 vhid, vithist_entry_t *ve)
{
    vh_lms2vh_t *lms2vh, *child;
    s3lmwid_t lwid;
    
    lwid = ve->lmstate.lm3g.lwid[0];
    if ((lms2vh = vh->lms2vh_root[lwid]) == NULL) {
	lms2vh = (vh_lms2vh_t *) ckd_calloc (1, sizeof(vh_lms2vh_t));
	vh->lms2vh_root[lwid] = lms2vh;
	
	lms2vh->state = lwid;
	lms2vh->children = NULL;
	
	vh->lwidlist = glist_add_int32 (vh->lwidlist, (int32) lwid);
    } else {
	assert (lms2vh->state == lwid);
    }
    
    child = (vh_lms2vh_t *) ckd_calloc (1, sizeof(vh_lms2vh_t));
    child->state = ve->lmstate.lm3g.lwid[1];
    child->children = NULL;
    child->vhid = vhid;
    child->ve = ve;
    
    lms2vh->children = glist_add_ptr (lms2vh->children, (void *)child);
}


vithist_entry_t *vithist_id2entry (vithist_t *vh, int32 id)
{
    vithist_entry_t *ve;

    ve = vh->entry[VITHIST_ID2BLK(id)] + VITHIST_ID2BLKOFFSET(id);
    return ve;
}


static void vithist_enter (vithist_t *vh, kbcore_t *kbc, vithist_entry_t *tve)
{
    vithist_entry_t *ve;
    int32 vhid;
    
    /* Check if an entry with this LM state already exists in current frame */
    vhid = vh_lmstate_find (vh, &(tve->lmstate));
    if (vhid < 0) {	/* Not found; allocate new entry */
	vhid = vh->n_entry;
	ve = vithist_entry_alloc (vh);
	
	*ve = *tve;
	vithist_lmstate_enter (vh, vhid, ve);	/* Enter new vithist info into LM state tree */
    } else {
	ve = vh->entry[VITHIST_ID2BLK(vhid)] + VITHIST_ID2BLKOFFSET(vhid);
	
	if (ve->score < tve->score)
	    *ve = *tve;
    }
    
    /* Update best exit score in this frame */
    if (vh->bestscore[vh->n_frm] < tve->score) {
	vh->bestscore[vh->n_frm] = tve->score;
	vh->bestvh[vh->n_frm] = vhid;
    }
}


void vithist_rescore (vithist_t *vh, kbcore_t *kbc,
		      s3wid_t wid, int32 ef, int32 score, int32 pred, int32 type)
{
    vithist_entry_t *pve, tve;
    s3lmwid_t lwid;
    int32 se, fe;
    int32 i;
    
    assert (vh->n_frm == ef);
    if(pred == -1) { 
      E_WARN("Hmm->out.history equals to -1 with score %d, some active phone was not computed?\n",score);
      /*      exit(-1);*/
    }
 
    pve = vh->entry[VITHIST_ID2BLK(pred)] + VITHIST_ID2BLKOFFSET(pred);
    /* Create a temporary entry with all the info currently available */
    tve.wid = wid;
    tve.sf = pve->ef + 1;
    tve.ef = ef;
    tve.type = type;
    tve.valid = 1;
    tve.ascr = score - pve->score;

    if (pred == 0) {	/* Special case for the initial <s> entry */
	se = 0;
	fe = 1;
    } else {
	se = vh->frame_start[pve->ef];
	fe = vh->frame_start[pve->ef + 1];
    }
    if (dict_filler_word (kbcore_dict(kbc), wid)) {
	tve.lscr = fillpen (kbcore_fillpen(kbc), wid);
	tve.score = score + tve.lscr;
	tve.pred = pred;
	tve.lmstate.lm3g = pve->lmstate.lm3g;
	vithist_enter (vh, kbc, &tve);
    } else {

      if(kbc->lmset)
	lwid = kbc->lm->dict2lmwid[wid];
      else 
	lwid = kbcore_dict2lmwid (kbc, wid);

      /*E_INFO("lwid %d, wid %d wordstr %s\n",lwid, wid, dict_wordstr(kbc->dict,wid));*/

	tve.lmstate.lm3g.lwid[0] = lwid;
	
	for (i = se; i < fe; i++) {
	    pve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	    
	    if (pve->valid) {
		tve.lscr = lm_tg_score (kbcore_lm(kbc),
					pve->lmstate.lm3g.lwid[1],
					pve->lmstate.lm3g.lwid[0],
					lwid,
					wid);
		
		tve.score = pve->score + tve.ascr + tve.lscr;
		
		if ((tve.score - vh->wbeam) >= vh->bestscore[vh->n_frm]) {
		    tve.pred = i;
		    tve.lmstate.lm3g.lwid[1] = pve->lmstate.lm3g.lwid[0];
		    
		    vithist_enter (vh, kbc, &tve);
		}
	    }
	}
    }
}


/*
 * Garbage collect invalid entries in current frame, right after marking invalid entries.
 */
static void vithist_frame_gc (vithist_t *vh, int32 frm)
{
    vithist_entry_t *ve, *tve;
    int32 se, fe, te, bs, bv;
    int32 i, j;
    
    se = vh->frame_start[frm];
    fe = vh->n_entry - 1;
    te = se;
    
    bs = MAX_NEG_INT32;
    bv = -1;
    for (i = se; i <= fe; i++) {
	ve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	if (ve->valid) {
	    if (i != te) {	/* Move i to te */
		tve = vh->entry[VITHIST_ID2BLK(te)] + VITHIST_ID2BLKOFFSET(te);
		*tve = *ve;
	    }
	    
	    if (ve->score > bs) {
		bs = ve->score;
		bv = te;
	    }
	    
	    te++;
	}
    }
    
    assert (bs == vh->bestscore[frm]);
    vh->bestvh[frm] = bv;
    
    /* Free up entries [te..n_entry-1] */
    i = VITHIST_ID2BLK(vh->n_entry - 1);
    j = VITHIST_ID2BLK(te - 1);
    for (; i > j; --i) {
	ckd_free ((void *) vh->entry[i]);
	vh->entry[i] = NULL;
    }
    vh->n_entry = te;
}


void vithist_prune (vithist_t *vh, dict_t *dict, int32 frm,
		    int32 maxwpf, int32 maxhist, int32 beam)
{
    int32 se, fe, filler_done, th;
    vithist_entry_t *ve;
    heap_t h;
    s3wid_t *wid;
    int32 i;
    
    assert (frm >= 0);
    
    se = vh->frame_start[frm];
    fe = vh->n_entry - 1;
    
    th = vh->bestscore[frm] + beam;
    
    h = heap_new ();
    wid = (s3wid_t *) ckd_calloc (maxwpf+1, sizeof(s3wid_t));
    wid[0] = BAD_S3WID;
    
    for (i = se; i <= fe; i++) {
	ve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	heap_insert (h, (void *)ve, -(ve->score));
	ve->valid = 0;
    }
    
    /* Mark invalid entries: beyond maxwpf words and below threshold */
    filler_done = 0;
    while ((heap_pop (h, (void **)(&ve), &i) > 0) && (ve->score >= th) && (maxhist > 0)) {
	if (dict_filler_word (dict, ve->wid)) {
	    /* Major HACK!!  Keep only one best filler word entry per frame */
	    if (filler_done)
		continue;
	    filler_done = 1;
	}
	
	/* Check if this word already valid (e.g., under a different history) */
	for (i = 0; IS_S3WID(wid[i]) && (wid[i] != ve->wid); i++);
	if (NOT_S3WID(wid[i])) {
	    /* New word; keep only if <maxwpf words already entered, even if >= thresh */
	    if (maxwpf > 0) {
		wid[i] = ve->wid;
		wid[i+1] = BAD_S3WID;
		
		--maxwpf;
		--maxhist;
		ve->valid = 1;
	    }
	} else if (! vh->bghist) {
	    --maxhist;
	    ve->valid = 1;
	}
    }
    
    ckd_free ((void *) wid);
    heap_destroy (h);
    
    /* Garbage collect invalid entries */
    vithist_frame_gc (vh, frm);
}


static void vithist_lmstate_reset (vithist_t *vh)
{
    gnode_t *lgn, *gn;
    int32 i;
    vh_lms2vh_t *lms2vh, *child;
    
    for (lgn = vh->lwidlist; lgn; lgn = gnode_next(lgn)) {
	i = (int32) gnode_int32 (lgn);
	lms2vh = vh->lms2vh_root[i];
	
	for (gn = lms2vh->children; gn; gn = gnode_next(gn)) {
	    child = (vh_lms2vh_t *) gnode_ptr (gn);
	    ckd_free ((void *) child);
	}
	glist_free (lms2vh->children);
	
	ckd_free ((void *) lms2vh);
	
	vh->lms2vh_root[i] = NULL;
    }
    
    glist_free (vh->lwidlist);
    vh->lwidlist = NULL;
}


void vithist_frame_windup (vithist_t *vh, int32 frm, FILE *fp, kbcore_t *kbc)
{
    assert (vh->n_frm == frm);
    
    vh->n_frm++;
    vh->frame_start[vh->n_frm] = vh->n_entry;
    
    if (fp)
	vithist_dump (vh, frm, kbc, fp);
    
    vithist_lmstate_reset (vh);
    
    vh->bestscore[vh->n_frm] = MAX_NEG_INT32;
    vh->bestvh[vh->n_frm] = -1;
}

int32 vithist_utt_end (vithist_t *vh, kbcore_t *kbc)
{
    int32 f, i, b, l;
    int32 sv, nsv, scr, bestscore, bestvh, vhid;
    vithist_entry_t *ve, *bestve=0;
    s3lmwid_t endwid;
    lm_t *lm;
    dict_t *dict;

    /* Find last frame with entries in vithist table */
    for (f = vh->n_frm-1; f >= 0; --f) {
	sv = vh->frame_start[f];	/* First vithist entry in frame f */
	nsv = vh->frame_start[f+1];	/* First vithist entry in next frame (f+1) */
	
	if (sv < nsv)
	    break;
    }
    if (f < 0)
	return -1;
    
    if (f != vh->n_frm-1)
	E_ERROR("No word exit in frame %d, using exits from frame %d\n", vh->n_frm-1, f);
    
    /* Terminate in a final </s> node (make this optional?) */
    lm = kbcore_lm (kbc);
    endwid = lm_finishwid (lm);
    dict = kbcore_dict (kbc);
    
    bestscore = MAX_NEG_INT32;
    bestvh = -1;
    
    for (i = sv; i < nsv; i++) {
	b = VITHIST_ID2BLK (i);
	l = VITHIST_ID2BLKOFFSET (i);
	ve = vh->entry[b] + l;
	
	scr = ve->score;
	scr += lm_tg_score (lm, ve->lmstate.lm3g.lwid[1], ve->lmstate.lm3g.lwid[0], endwid,dict_finishwid(dict));
	
	if (bestscore < scr) {
	    bestscore = scr;
	    bestvh = i;
	    bestve = ve;
	}
    }
    assert (bestvh >= 0);

    
    if (f != vh->n_frm-1) {
	E_ERROR("No word exit in frame %d, using exits from frame %d\n", vh->n_frm-1, f);
	
	/* Add a dummy silwid covering the remainder of the utterance */
	assert (vh->frame_start[vh->n_frm-1] == vh->frame_start[vh->n_frm]);
	vh->n_frm -= 1;
	vithist_rescore (vh, kbc, dict_silwid (dict), vh->n_frm, bestve->score, bestvh, -1);
	vh->n_frm += 1;
	vh->frame_start[vh->n_frm] = vh->n_entry;
	
	return vithist_utt_end (vh, kbc);
    }
    
    /* Create an </s> entry */
    vhid = vh->n_entry;
    ve = vithist_entry_alloc (vh);
    
    ve->wid = dict_finishwid (dict);
    ve->sf = (bestve->ef == BAD_S3FRMID) ? 0 : bestve->ef + 1;
    ve->ef = vh->n_frm;
    ve->ascr = 0;
    ve->lscr = bestscore - bestve->score;
    ve->score = bestscore;
    ve->pred = bestvh;
    ve->type = 0;
    ve->valid = 1;
    ve->lmstate.lm3g.lwid[0] = endwid;
    ve->lmstate.lm3g.lwid[1] = ve->lmstate.lm3g.lwid[0];
    
    return vhid;
}


int32 vithist_partialutt_end (vithist_t *vh, kbcore_t *kbc)
{
    int32 f, i, b, l;
    int32 sv, nsv, scr, bestscore, bestvh;
    vithist_entry_t *ve, *bestve;
    s3lmwid_t endwid;
    lm_t *lm;
    dict_t *dict;

    /* Find last frame with entries in vithist table */
    for (f = vh->n_frm-1; f >= 0; --f) {
	sv = vh->frame_start[f];	/* First vithist entry in frame f */
	nsv = vh->frame_start[f+1];	/* First vithist entry in next frame (f+1) */
	
	if (sv < nsv)
	    break;
    }
    if (f < 0)
	return -1;
    
    if (f != vh->n_frm-1){
	E_ERROR("No word exits from in block with last frame= %d\n",vh->n_frm-1);
	return -1;
    }
    
    /* Terminate in a final </s> node (make this optional?) */
    lm = kbcore_lm (kbc);
    dict = kbcore_dict (kbc);    
    endwid = lm_finishwid (lm);
    
    bestscore = MAX_NEG_INT32;
    bestvh = -1;
    
    for (i = sv; i < nsv; i++) {
	b = VITHIST_ID2BLK (i);
	l = VITHIST_ID2BLKOFFSET (i);
	ve = vh->entry[b] + l;
	
	scr = ve->score;
	scr += lm_tg_score (lm, ve->lmstate.lm3g.lwid[1], ve->lmstate.lm3g.lwid[0], endwid, dict_finishwid(dict));
	
	if (bestscore < scr) {
	    bestscore = scr;
	    bestvh = i;
	    bestve = ve;
	}
    }

    return bestvh;
}


void vithist_utt_reset (vithist_t *vh)
{
    int32 b;
    
    vithist_lmstate_reset (vh);
    
    for (b = VITHIST_ID2BLK(vh->n_entry-1); b >= 0; --b) {
	ckd_free ((void *) vh->entry[b]);
	vh->entry[b] = NULL;
    }
    vh->n_entry = 0;
    
    vh->bestscore[0] = MAX_NEG_INT32;
    vh->bestvh[0] = -1;
}


#if 0
static void vithist_lmstate_subtree_dump (vithist_t *vh, kbcore_t *kbc,
					  vh_lmstate2vithist_t *lms2vh, int32 level, FILE *fp)
{
    gnode_t *gn;
    vh_lmstate2vithist_t *child;
    int32 i;
    lm_t *lm;
    
    lm = kbcore_lm (kbc);
    
    for (gn = lms2vh->children; gn; gn = gnode_next(gn)) {
	child = (vh_lmstate2vithist_t *) gnode_ptr (gn);
	
	for (i = 0; i < level; i++)
	    fprintf (fp, "\t");
	fprintf (fp, "\t%s -> %d\n", lm_wordstr(lm, child->state), child->vhid);
	
	vithist_lmstate_subtree_dump (vh, kbc, child, level+1, fp);
    }
}


static void vithist_lmstate_dump (vithist_t *vh, kbcore_t *kbc, FILE *fp)
{
    glist_t gl;
    gnode_t *lgn, *gn;
    int32 i;
    vh_lmstate2vithist_t *lms2vh;
    mdef_t *mdef;
    lm_t *lm;
    
    mdef = kbcore_mdef (kbc);
    lm = kbcore_lm (kbc);
    
    fprintf (fp, "LMSTATE\n");
    for (lgn = vh->lwidlist; lgn; lgn = gnode_next(lgn)) {
	i = (int32) gnode_int32 (lgn);
	
	gl = vh->lmstate_root[i];
	assert (gl);
	
	for (gn = gl; gn; gn = gnode_next(gn)) {
	    lms2vh = (vh_lmstate2vithist_t *) gnode_ptr (gn);
	    
	    fprintf (fp, "\t%s.%s -> %d\n",
		     lm_wordstr(lm, i), mdef_ciphone_str (mdef, lms2vh->state), lms2vh->vhid);
	    vithist_lmstate_subtree_dump (vh, kbc, lms2vh, 1, fp);
	}
    }
    fprintf (fp, "END_LMSTATE\n");
    fflush (fp);
}
#endif


void vithist_dump (vithist_t *vh, int32 frm, kbcore_t *kbc, FILE *fp)
{
    int32 b, l, i, j;
    dict_t *dict;
    lm_t *lm;
    vithist_entry_t *ve;
    s3lmwid_t lwid;
    int32 sf, ef;
    
    dict = kbcore_dict (kbc);
    lm = kbcore_lm (kbc);
    
    if (frm >= 0) {
	sf = frm;
	ef = frm;
	
	fprintf (fp, "VITHIST  frame %d  #entries %d\n",
		 frm, vh->frame_start[sf+1] - vh->frame_start[sf]);
    } else {
	sf = 0;
	ef = vh->n_frm - 1;
	
	fprintf (fp, "VITHIST  #frames %d  #entries %d\n", vh->n_frm, vh->n_entry);
    }
    fprintf (fp, "\t%7s %5s %5s %11s %9s %8s %7s %4s Word (LM-state)\n",
	     "Seq/Val", "SFrm", "EFrm", "PathScr", "SegAScr", "SegLScr", "Pred", "Type");
    
    for (i = sf; i <= ef; i++) {
	fprintf (fp, "%5d BS: %11d BV: %8d\n", i, vh->bestscore[i], vh->bestvh[i]);
	
	for (j = vh->frame_start[i]; j < vh->frame_start[i+1]; j++) {
	    /* This entry */
	    b = VITHIST_ID2BLK(j);
	    l = VITHIST_ID2BLKOFFSET (j);
	    ve = vh->entry[b] + l;
	    
	    fprintf (fp, "\t%c%6d %5d %5d %11d %9d %8d %7d %4d %s",
		     (ve->valid ? ' ' : '*'), j,
		     ve->sf, ve->ef, ve->score, ve->ascr, ve->lscr, ve->pred, ve->type,
		     dict_wordstr (dict, ve->wid));
	    
	    fprintf (fp, " (%s", lm_wordstr (lm, ve->lmstate.lm3g.lwid[0]));
	    lwid = ve->lmstate.lm3g.lwid[1];
	    if (IS_S3LMWID(lwid))
		fprintf (fp, ", %s", lm_wordstr (lm, lwid));
	    fprintf (fp, ")\n");
	}
	
	if (j == vh->frame_start[i])
	    fprintf (fp, "\n");
    }

#if 0    
    if (vh->lwidlist)
	vithist_lmstate_dump (vh, kbc, fp);
#endif

    fprintf (fp, "END_VITHIST\n");
    fflush (fp);
}

glist_t vithist_backtrace (vithist_t *vh, int32 id)
{
    vithist_entry_t *ve;
    int32 b, l;
    glist_t hyp;
    hyp_t *h;
    
    hyp = NULL;
    
    while (id > 0) {
	b = VITHIST_ID2BLK(id);
	l = VITHIST_ID2BLKOFFSET(id);
	ve = vh->entry[b] + l;
	
	h = (hyp_t *) ckd_calloc (1, sizeof(hyp_t));
	h->id = ve->wid;
	h->sf = ve->sf;
	h->ef = ve->ef;
	h->ascr = ve->ascr;
	h->lscr = ve->lscr;
	h->type = ve->type;
	h->vhid = id;
	
	hyp = glist_add_ptr (hyp, h);
	
	id = ve->pred;
    }
    
    return hyp;
}


typedef struct {
    s3wid_t wid;
    int32 fef, lef;
    int32 seqid;	/* Node sequence no. */
    glist_t velist;	/* Vithist entries for this dagnode */
} dagnode_t;


/*
 * Header written BEFORE this function is called.
 */
void vithist_dag_write (vithist_t *vh, glist_t hyp, dict_t *dict, int32 oldfmt, FILE *fp)
{
    glist_t *sfwid;		/* To maintain <start-frame, word-id> pair dagnodes */
    vithist_entry_t *ve, *ve2;
    gnode_t *gn, *gn2, *gn3;
    dagnode_t *dn, *dn2;
    int32 sf, ef, n_node;
    int32 f, i;
    hyp_t *h;
    
    sfwid = (glist_t *) ckd_calloc (vh->n_frm+1, sizeof(glist_t));
    
    n_node = 0;
    for (i = 0; i < vh->n_entry; i++) {	/* This range includes the dummy <s> and </s> entries */
	ve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	if (! ve->valid)
	    continue;
	
	/*
	 * The initial <s> entry (at 0) is a dummy, with start/end frame = -1.  But the old S3
	 * code treats it like a real word, so we have to reintroduce it in the dag file with
	 * a start time of 0.  And shift the start time of words starting at frame 0 up by 1.
	 * MAJOR HACK!!
	 */
	if (ve->sf <= 0) {
	    assert (ve->sf >= -1);
	    assert ((ve->ef == -1) || (ve->ef > 1));
	    
	    sf = ve->sf + 1;
	    ef = (ve->ef < 0) ? 0 : ve->ef;
	} else {
	    sf = ve->sf;
	    ef = ve->ef;
	}
	
	for (gn = sfwid[sf]; gn; gn = gnode_next(gn)) {
	    dn = (dagnode_t *) gnode_ptr(gn);
	    if (dn->wid == ve->wid)
		break;
	}
	if (! gn) {
	    dn = (dagnode_t *) ckd_calloc (1, sizeof(dagnode_t));
	    dn->wid = ve->wid;
	    dn->fef = ef;
	    dn->lef = ef;
	    dn->seqid = -1;	/* Initially all invalid, selected ones validated below */
	    dn->velist = NULL;
	    n_node++;
	    
	    sfwid[sf] = glist_add_ptr (sfwid[sf], (void *) dn);
	} else {
	    dn->lef = ef;
	}
	
	/*
	 * Check if an entry already exists under dn->velist (generated by a different
	 * LM context; retain only the best scoring one.
	 */
	for (gn = dn->velist; gn; gn = gnode_next(gn)) {
	    ve2 = (vithist_entry_t *) gnode_ptr (gn);
	    if (ve2->ef == ve->ef)
		break;
	}
	if (gn) {
	    if (ve->score > ve2->score)
		gnode_ptr(gn) = (void *)ve;
	} else
	    dn->velist = glist_add_ptr (dn->velist, (void *) ve);
    }
    
    /*
     * Validate segments with >1 end times; if only 1 end time, can be pruned.
     * But keep segments in the original hypothesis, regardless; mark them first.
     */
    for (gn = hyp; gn; gn = gnode_next(gn)) {
	h = (hyp_t *) gnode_ptr (gn);
	for (gn2 = sfwid[h->sf]; gn2; gn2 = gnode_next(gn2)) {
	    dn = (dagnode_t *) gnode_ptr (gn2);
	    if (h->id == dn->wid)
		dn->seqid = 0;	/* Do not discard (prune) this dagnode */
	}
    }
    
    /* Validate startwid and finishwid nodes */
    dn = (dagnode_t *) gnode_ptr(sfwid[0]);
    assert (dn->wid == dict_startwid(dict));
    dn->seqid = 0;
    dn = (dagnode_t *) gnode_ptr(sfwid[vh->n_frm]);
    assert (dn->wid == dict_finishwid(dict));
    dn->seqid = 0;
    
    /* Now prune dagnodes with only 1 end frame if not validated above */
    i = 0;
    for (f = vh->n_frm; f >= 0; --f) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (dagnode_t *) gnode_ptr(gn);
	    if ((dn->lef > dn->fef) || (dn->seqid >= 0))
		dn->seqid = i++;
	    else
		dn->seqid = -1;		/* Flag: discard */
	}
    }
    n_node = i;
    
    /* Write nodes info; the header should have been written before this function is called */
    fprintf (fp, "Nodes %d (NODEID WORD STARTFRAME FIRST-ENDFRAME LAST-ENDFRAME)\n", n_node);
    for (f = vh->n_frm; f >= 0; --f) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (dagnode_t *) gnode_ptr(gn);
	    if (dn->seqid >= 0) {
		fprintf (fp, "%d %s %d %d %d\n",
			 dn->seqid, dict_wordstr(dict, dn->wid), f, dn->fef, dn->lef);
	    }
	}
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "Initial %d\nFinal %d\n", n_node-1, 0);
    fprintf (fp, "#\n");
    
    fprintf (fp, "BestSegAscr 0 (NODEID ENDFRAME ASCORE)\n");
    fprintf (fp, "#\n");
    
    /* Edges */
    if (oldfmt)
	fprintf (fp, "Edges (FROM-NODEID TO-NODEID ASCORE)\n");
    else
	fprintf (fp, "Edges (FROM-NODEID ENDFRAME ASCORE)\n");
    for (f = vh->n_frm-1; f >= 0; --f) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (dagnode_t *) gnode_ptr(gn);
	    /* Look for transitions from this dagnode to later ones, if not discarded */
	    if (dn->seqid < 0)
		continue;
	    
	    for (gn2 = dn->velist; gn2; gn2 = gnode_next(gn2)) {
		ve = (vithist_entry_t *) gnode_ptr (gn2);
		
		sf = (ve->ef < 0) ? 1 : (ve->ef + 1);
		
		if (oldfmt) {
		    for (gn3 = sfwid[sf]; gn3; gn3 = gnode_next(gn3)) {
			dn2 = (dagnode_t *) gnode_ptr(gn3);
			if (dn2->seqid >= 0)
			    fprintf (fp, "%d %d %d\n", dn->seqid, dn2->seqid, ve->ascr);
		    }
		} else {
		    for (gn3 = sfwid[sf]; gn3; gn3 = gnode_next(gn3)) {
			dn2 = (dagnode_t *) gnode_ptr(gn3);
			if (dn2->seqid >= 0) {
			    fprintf (fp, "%d %d %d\n", dn->seqid, sf-1, ve->ascr);
			    break;
			}
		    }
		}
	    }
	}
    }
    fprintf (fp, "End\n");
    
    /* Free dagnodes structure */
    for (f = 0; f <= vh->n_frm; f++) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (dagnode_t *) gnode_ptr(gn);
	    
	    glist_free (dn->velist);
	    ckd_free ((void *) dn);
	}
	
	glist_free (sfwid[f]);
    }
    ckd_free ((void *) sfwid);
}

/* 
 * RAH, free memory allocated in vithist_free 
 */
void vithist_free (vithist_t *v)
{
  if (v) {
    ckd_free ((void *) v);
  }

}
