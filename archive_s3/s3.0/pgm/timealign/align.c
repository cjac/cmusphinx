/*
 * align.c -- Time alignment module.
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
 * 11-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

/*
 * The main time-alignment routines.  Actually, this module is quite generic since it is 
 * basically performing a Viterbi search over a generic phone net.  The only thing specific to
 * time-alignment is the amount of backtrace information gathered during the search: at the
 * finest granularity of states traversed per frame.  So with a little effort we could perhaps
 * build a generic "pnet_search" out of this module...
 * 
 * The 32-bit IDs in the Viterbi history constructed by the search are built as follows:
 *     20-bit word-id,
 *      8-bit phone-position within word,
 *      4-bit state-position within phone.
 * This places somewhat smaller limits on the dictionary and HMM sizes than might be otherwise
 * available, but it is still quite sufficient for the foreseeable future.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include <libio/libio.h>
#include <libmain/hyp.h>
#include "align.h"


/*
 * Convert a <word-id, phone-position, state-position> triple to a Viterbi history ID.
 *   ppos is the phone position within the word pronunciation.
 *   spos is the state position within the phone HMM.
 */
#define VITHIST_WIDSIZE		20	/* #Bits for word-id */
#define VITHIST_PPOSSIZE	 8	/* #Bits for phone-position */
#define VITHIST_SPOSSIZE	 4	/* #Bits for state-position */
#define VITHIST_ID(wid,ppos,spos)	( ((wid)<<12) | ((ppos)<<4) | (spos) )
/* Extract the word-id from a Viterbi history ID */
#define VITHIST_ID2WID(vid)		( ((vid) >> 12) & 0x000fffff )
/* Extract the phone-position from a Viterbi history ID */
#define VITHIST_ID2PPOS(vid)		( ((vid) >> 4) & 0x000000ff )
/* Extract the state-position from a Viterbi history ID */
#define VITHIST_ID2SPOS(vid)		( (vid) & 0x0000000f )


int32 align_start (kb_t *kb, char *uttid)
{
    gnode_t *gn;
    pnode_t *pn;
    plink_t *pl;
    
    kb->pactive = NULL;
    kb->hist = NULL;
    
    if (dict_size(kb->dict) >= (1 << VITHIST_WIDSIZE))
	E_FATAL("Cannot handle dictionary size >= %d\n", (1 << VITHIST_WIDSIZE));
    if (mdef_n_ciphone(kb->mdef) >= (1 << VITHIST_PPOSSIZE))
	E_FATAL("Cannot handle #ciphones >= %d\n", (1 << VITHIST_PPOSSIZE));
    if (mdef_n_emit_state(kb->mdef) >= (1 << VITHIST_SPOSSIZE))
	E_FATAL("Cannot handle #HMMstates >= %d\n", (1 << VITHIST_SPOSSIZE));

    /* Initialize all HMMs to the inactive state */
    for (gn = kb->pnet; gn; gn = gn->next) {
	pn = (pnode_t *) gnode_ptr(gn);
	hmm_clear (kb->mdef, &(pn->hmm));
    }

    /* Activate the initial HMMs, and mark active senones */
    for (gn = kb->pstart->succ; gn; gn = gn->next) {
	pl = (plink_t *) gnode_ptr(gn);
	pn = pl->dst;
	
	assert (pn->hmm.active == -1);	/* Check against multiple activation */
	hmm_enter (&(pn->hmm), 0, 0, NULL, 0);
	kb->pactive = glist_add_ptr (kb->pactive, (void *)pn);
    }
    
    /* Mark the active senones */
    pnet_set_senactive (kb->mdef, kb->pactive, kb->am->sen_active, kb->am->sen->n_sen);
    
    return 0;
}


int32 align_frame (kb_t *kb, int32 beam, int32 frm, char *uttid)
{
    gnode_t *gn, *gn2;
    pnode_t *pn, *pn2;
    int32 bestscore, hmmscore, thresh;
    glist_t cur_active, next_active;
    hmm_t *hmm, *hmm2;
    plink_t *pl;
    int32 i;
    int32 vid;
    
    cur_active = kb->pactive;
    
    /* Step the active HMMs through this frame */
    bestscore = (int32)0x80000000;
    for (gn = cur_active; gn; gn = gn->next) {
	pn = (pnode_t *) gnode_ptr(gn);
	assert (pn->hmm.active == frm);
	
	hmmscore = hmm_vit_eval (kb->mdef, kb->tmat, &(pn->hmm), kb->am->senscr);
	
	if (bestscore < hmmscore)
	    bestscore = hmmscore;
    }
    if (bestscore <= LOGPROB_ZERO) {
	E_ERROR("%s: Best HMM score (%d) underflowed at frame %d\n", uttid, bestscore, frm);
	glist_free (cur_active);
	kb->pactive = NULL;
	return -1;
    }
    if (bestscore > 0)
	E_WARN("%s: Best HMM score > 0 (%d) at frame %d; overflow??\n", uttid, bestscore, frm);
    
    /* Note current state scores; propagate scores across HMMs; prune poor HMMs */
    thresh = bestscore + beam;
    next_active = NULL;
    
    for (gn = cur_active; gn; gn = gn->next) {
	pn = (pnode_t *) gnode_ptr(gn);
	hmm = &(pn->hmm);
	
	if (hmm->bestscore < thresh) {
	    if (hmm->active <= frm)	/* Not activated for the next frame */
		hmm_clear (kb->mdef, hmm);
	} else {
	    /* Note Viterbi history for each state */
	    for (i = 0; i < kb->mdef->n_emit_state; i++) {
		if (hmm->state[i].score >= thresh) {
		    vid = VITHIST_ID(pn->wid, pn->pos, i);
		    kb->hist = vithist_append (kb->hist, vid, frm, hmm->state[i].score,
					       hmm->state[i].hist, NULL);
		    /*
		     * Update the exit history with the history node just created
		     * (for propagation).
		     */
		    hmm->state[i].hist = (vithist_t *) gnode_ptr(kb->hist);
		}
	    }

	    /* Transition to exit state, and note its Viterbi history */
	    hmm_vit_eval_exit (kb->mdef, kb->tmat, &(pn->hmm));
	    if (hmm->out.score >= thresh) {
		vid = VITHIST_ID(pn->wid, pn->pos, kb->mdef->n_emit_state);
		kb->hist = vithist_append (kb->hist, vid, frm, hmm->out.score,
					   hmm->out.hist, NULL);
		/*
		 * Update the exit history with the history node just created
		 * (for propagation).
		 */
		hmm->out.hist = (vithist_t *) gnode_ptr(kb->hist);
	    }
	    
	    /* Activate for next frame if not already activated */
	    if (hmm->active <= frm) {
		next_active = glist_add_ptr (next_active, (void *)pn);
		hmm->active = frm+1;
	    }
	    
	    /* Transition to successor pnodes if exited */
	    if (hmm->out.score >= thresh) {
		for (gn2 = pn->succ; gn2; gn2 = gn2->next) {
		    pl = (plink_t *) gnode_ptr(gn2);
		    pn2 = pl->dst;
		    hmm2 = &(pn2->hmm);
		    
		    if (hmm_vit_trans (hmm, hmm2, frm+1))
			next_active = glist_add_ptr (next_active, (void *)pn2);
		}
	    }
	}
    }

    glist_free (cur_active);
    kb->pactive = next_active;
    
    /* Mark the active senones */
    pnet_set_senactive (kb->mdef, next_active, kb->am->sen_active, kb->am->sen->n_sen);
    
    return 0;
}


void align_hypfree (glist_t hyp)
{
    glist_myfree (hyp, sizeof(hyp_t));
}


glist_t align_end (kb_t *kb, int32 frm, char *uttid)
{
    hmm_t *hmm;
    glist_t hyplist;
    
    glist_free (kb->pactive);
    kb->pactive = NULL;
    
    hmm = &(kb->pend->hmm);
    if (hmm->active == frm)
	hyplist = vithist_backtrace (hmm->in.hist, kb->am->senscale);
    else
	hyplist = NULL;
    
    glist_myfree (kb->hist, sizeof(vithist_t));
    kb->hist = NULL;
    
    return hyplist;
}


void align_write_s2stseg (kb_t *kb, glist_t hyp, char *file, char *uttid)
{
    FILE *fp;
    static int32 byterev = -1;	/* Whether to byte reverse output data */
    int32 k;
    gnode_t *gn;
    hyp_t *h;
    int32 wid, ppos, spos, phonebegin;
    int16 s2_info;
    
    if (byterev < 0) {
	if ((byterev = host_endian()) < 0) {
	    E_ERROR("Couldn't figure out host byte ordering, using native mode\n");
	    byterev = 0;
	}
    }
    
    /* Write #frames; this should be the end-frame of the last hypnode +1  */
    if ((gn = glist_tail (hyp)) == NULL) {
	E_ERROR("%s: Empty hypothesis\n", uttid);
	return;
    }
    h = (hyp_t *) gnode_ptr(gn);
    k = h->ef + 1;
    
    if ((fp = fopen(file, "wb")) == NULL) {
	E_ERROR("%s: fopen(%s,wb) failed\n", uttid, file);
	return;
    }
    if (byterev)
	SWAP_INT32(&k);
    if (fwrite (&k, sizeof(int32), 1, fp) != 1)
	goto write_error;
    
    /* Write state info for each frame */
    phonebegin = 1;
    for (gn = hyp; gn; gn = gn->next) {
	h = (hyp_t *) gnode_ptr(gn);
	wid = VITHIST_ID2WID(h->id);
	ppos = VITHIST_ID2PPOS(h->id);
	spos = VITHIST_ID2SPOS(h->id);
	
	if (spos == kb->mdef->n_emit_state)
	    phonebegin = 1;
	else {
	    s2_info = dict_pron(kb->dict, wid, ppos) * kb->mdef->n_emit_state + spos;
	    if (phonebegin)
		s2_info |= 0x8000;
	    if (byterev)
		SWAP_INT16(&s2_info);
	    
	    if (fwrite (&s2_info, sizeof(int16), 1, fp) != 1)
		goto write_error;
	    
	    phonebegin = 0;
	}
    }
    
    fclose (fp);
    return;
    
write_error:
    E_ERROR("%s: fwrite(%s) failed\n", uttid, file);
    fclose (fp);
}


glist_t align_st2wdseg (kb_t *kb, glist_t stseg)
{
    glist_t wdseg;
    int32 wid, ppos, spos, wordbegin;
    gnode_t *gn;
    hyp_t *h, *wh;
    
    wdseg = NULL;
    wordbegin = 1;
    for (gn = stseg; gn; gn = gn->next) {
	h = (hyp_t *) gnode_ptr(gn);
	wid = VITHIST_ID2WID(h->id);
	ppos = VITHIST_ID2PPOS(h->id);
	spos = VITHIST_ID2SPOS(h->id);
	
	if (wordbegin) {
	    wh = (hyp_t *) mymalloc (sizeof(hyp_t));
	    wh->id = wid;
	    wh->sf = h->sf;
	    wh->ascr = h->ascr;
	    wh->lscr = 0;
	    
	    wdseg = glist_add_ptr (wdseg, (void *) wh);
	    
	    wordbegin = 0;
	} else {
	    assert (wh->id == wid);
	    wh->ascr += h->ascr;
	}
	
	/* If at non-emitting state of the final phone for this word, end of word */
	if ((spos == kb->mdef->n_emit_state) && ((ppos+1) == dict_pronlen(kb->dict,wid))) {
	    wh->ef = h->ef;
	    wh->scr = wh->ascr;
	    wordbegin = 1;
	}
    }
    assert (wordbegin);
    
    /* The word segmentation list is actually reversed; get it right */
    wdseg = glist_reverse (wdseg);
    
    return wdseg;
}


void align_write_wdseg (kb_t *kb, glist_t wdseg, char *file, char *uttid)
{
    FILE *fp;
    int32 uttscore;
    gnode_t *gn;
    hyp_t *h;
    
    if ((fp = fopen(file, "w")) == NULL) {
	E_ERROR("%s: fopen(%s,w) failed; writing wdseg to stdout\n", uttid, file);
	fp = stdout;
    }
    
    if (fp == stdout)
	fprintf (fp, "WD:%s>", uttid);
    fprintf (fp, "\t%5s %5s %10s %s\n", "SFrm", "EFrm", "SegAScr", "Word");
    
    uttscore = 0;
    for (gn = wdseg; gn; gn = gn->next) {
	h = (hyp_t *) gnode_ptr(gn);
	
	if (fp == stdout)
	    fprintf (fp, "wd:%s>", uttid);
	fprintf (fp, "\t%5d %5d %10d %s\n",
		 h->sf, h->ef, h->ascr, dict_wordstr (kb->dict, (s3wid_t)h->id));
	
	uttscore += h->ascr;
    }
    
    if (fp == stdout)
	fprintf (fp, "WD:%s>", uttid);
    fprintf (fp, " Total score: %11d\n", uttscore);
    
    if (fp != stdout)
	fclose (fp);
    else {
	fprintf (fp, "\n");
	fflush (fp);
    }
}


void align_write_outsent (kb_t *kb, glist_t wdseg, FILE *fp, char *uttid)
{
    gnode_t *gn;
    hyp_t *h;
    
    for (gn = wdseg; gn; gn = gn->next) {
	h = (hyp_t *) gnode_ptr(gn);
	fprintf (fp, "%s ", dict_wordstr (kb->dict, (s3wid_t)h->id));
    }
    fprintf (fp, "(%s)\n", uttid);
    fflush (fp);
}
