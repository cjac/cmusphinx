/*
 * allphone.c -- Allphone Viterbi decoding.
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
 * 02-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added allphone lattice output.
 * 
 * 14-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "logs3.h"
#include "s3_allphone.h"

/*
 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 */


/*
 * Phone-HMM (PHMM) structure:  Models a single unique <senone-sequence, tmat> pair.
 * Can represent several different triphones, but all with the same parent basephone.
 * (NOTE: Word-position attribute of triphone is ignored.)
 */
typedef struct phmm_s {
    s3pid_t pid;	/* Phone id (temp. during init.) */
    s3tmatid_t tmat;	/* Transition matrix id for this PHMM */
    s3cipid_t ci;	/* Parent basephone for this PHMM */
    s3frmid_t active;	/* Latest frame in which this PHMM is/was active */
    uint32 *lc;		/* Set (bit-vector) of left context phones seen for this PHMM */
    uint32 *rc;		/* Set (bit-vector) of right context phones seen for this PHMM */
    s3senid_t *sen;	/* Senone-id sequence underlying this PHMM */
    int32 *score;	/* Total path score during Viterbi decoding */
    struct history_s **hist;	/* Viterbi history (for backtrace) */
    int32 bestscore;	/* Best state score in any frame */
    int32 inscore;	/* Incoming score from predecessor PHMMs */
    struct history_s *inhist;	/* History corresponding to inscore */
    struct phmm_s *next;	/* Next unique PHMM for same parent basephone */
    struct plink_s *succlist;	/* List of predecessor PHMM nodes */
} phmm_t;
static phmm_t **ci_phmm;	/* PHMM lists (for each CI phone) */

/*
 * List of links from a PHMM node to its successors; one link per successor.
 */
typedef struct plink_s {
    phmm_t *phmm;		/* Successor PHMM node */
    struct plink_s *next;	/* Next link for parent PHMM node */
} plink_t;

/*
 * History (paths) information at any point in allphone Viterbi search.
 */
typedef struct history_s {
    phmm_t *phmm;	/* PHMM ending this path */
    int32 score;	/* Path score for this path */
    s3frmid_t ef;	/* End frame */
    struct history_s *hist;	/* Previous history entry */
    struct history_s *next;	/* Next in allocated list */
} history_t;
static history_t **frm_hist;	/* List of history nodes allocated in each frame */

extern mdef_t *mdef;		/* Model definition */
extern tmat_t *tmat;		/* Transition probability matrices */

static int32 lrc_size = 0;
static int32 curfrm;		/* Current frame */
static int32 beam;
static int32 *score_scale;	/* Score by which state scores scaled in each frame */
static phseg_t *phseg;
static int32 **tp;		/* Phone transition probabilities */
static int32 n_histnode;	/* No. of history entries */


/*
 * Find PHMM node with same senone sequence and tmat id as the given triphone.
 * Return ptr to PHMM node if found, NULL otherwise.
 */
static phmm_t *phmm_lookup (s3pid_t pid)
{
    phmm_t *p;
    phone_t *old, *new;
    
    new = &(mdef->phone[pid]);
    
    for (p = ci_phmm[(unsigned)mdef->phone[pid].ci]; p; p = p->next) {
	old = &(mdef->phone[p->pid]);
	if (old->tmat == new->tmat) {
	  if (old->ssid == new->ssid)
		return p;
	}
    }

    return NULL;
}


static void lrc_set (uint32 *vec, int32 ci)
{
    int32 i, j;
    
    assert (lrc_size > 0);
    
    /* If lc or rc not specified, set all flags */
    if (NOT_S3CIPID(ci)) {
	for (i = 0; i < lrc_size; i++)
	    vec[i] = (uint32) 0xffffffff;
    } else {
	i = (ci >> 5);
	j = ci - (i << 5);
	vec[i] |= (1 << j);
    }
}


static int32 lrc_is_set (uint32 *vec, int32 ci)
{
    int32 i, j;

    i = (ci >> 5);
    j = ci - (i << 5);
    return (vec[i] & (1 << j));
}


static int32 phmm_link ( void )
{
    s3cipid_t ci, rc;
    phmm_t *p, *p2;
    int32 *rclist;
    int32 i, n_link;
    plink_t *l;
    
    rclist = (int32 *) ckd_calloc (mdef->n_ciphone+1, sizeof(int32));
    
    /* Create successor links between PHMM nodes */
    n_link = 0;
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    /* Build rclist for p */
	    i = 0;
	    for (rc = 0; rc < mdef->n_ciphone; rc++) {
		if (lrc_is_set (p->rc, rc))
		    rclist[i++] = rc;
	    }
	    rclist[i] = BAD_S3CIPID;
	    
	    /* For each rc in rclist, transition to PHMMs for rc if left context = ci */
	    for (i = 0; IS_S3CIPID(rclist[i]); i++) {
		for (p2 = ci_phmm[rclist[i]]; p2; p2 = p2->next) {
		    if (lrc_is_set (p2->lc, ci)) {
			/* transition from p to p2 */
			l = (plink_t *) listelem_alloc (sizeof(plink_t));
			l->phmm = p2;
			l->next = p->succlist;
			p->succlist = l;
			
			n_link++;
		    }
		}
	    }
	}
    }

    ckd_free (rclist);
    
    return n_link;
}


static int32 phmm_build ( void )
{
    s3pid_t pid;
    phmm_t *p, **pid2phmm;
    s3cipid_t ci;
    int32 n_phmm, n_link;
    s3senid_t *sen;
    int32 *score;
    history_t **hist;
    uint32 *lc, *rc;
    int32 i, s;
    s3cipid_t *filler;

    E_INFO("Building PHMM net\n");
    
    ci_phmm = (phmm_t **) ckd_calloc (mdef->n_ciphone, sizeof(phmm_t *));
    pid2phmm = (phmm_t **) ckd_calloc (mdef->n_phone, sizeof(phmm_t *));

    for (lrc_size = 32; lrc_size < mdef->n_ciphone; lrc_size += 32);
    lrc_size >>= 5;

    /* For each unique ciphone/triphone entry in mdef, create a PHMM node */
    n_phmm = 0;
    for (pid = 0; pid < mdef->n_phone; pid++) {
	if ((p = phmm_lookup (pid)) == NULL) {
	    /* No previous entry; create a new one */
	    p = (phmm_t *) listelem_alloc (sizeof(phmm_t));

	    p->pid = pid;
	    p->tmat = mdef->phone[pid].tmat;
	    p->ci = mdef->phone[pid].ci;
	    p->succlist = NULL;
	    
	    p->next = ci_phmm[(unsigned)p->ci];
	    ci_phmm[(unsigned)p->ci] = p;

	    n_phmm++;
	}

	pid2phmm[pid] = p;
    }
    
    /* Fill out rest of each PHMM node */
    sen = (s3senid_t *) ckd_calloc (n_phmm * mdef->n_emit_state, sizeof(s3senid_t));
    score = (int32 *) ckd_calloc (n_phmm * (mdef->n_emit_state+1), sizeof(int32));
    hist = (history_t **) ckd_calloc (n_phmm * (mdef->n_emit_state+1),
				      sizeof(history_t *));
    lc = (uint32 *) ckd_calloc (n_phmm * lrc_size * 2, sizeof(uint32));
    rc = lc + (n_phmm * lrc_size);
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    p->sen = sen;
	    for (s = 0; s < mdef->n_emit_state; s++)
		p->sen[s] = mdef->sseq[mdef->phone[p->pid].ssid][s];
	    sen += mdef->n_emit_state;

	    p->score = score;
	    score += (mdef->n_emit_state + 1);

	    p->hist = hist;
	    hist += (mdef->n_emit_state + 1);

	    p->lc = lc;
	    lc += lrc_size;
	    
	    p->rc = rc;
	    rc += lrc_size;
	}
    }

    /* Fill out lc and rc bitmaps (remember to map all fillers to each other!!) */
    filler = (s3cipid_t *) ckd_calloc (mdef->n_ciphone + 1, sizeof(s3cipid_t));
    i = 0;
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	if (mdef->ciphone[(unsigned)ci].filler)
	    filler[i++] = ci;
    }
    filler[i] = BAD_S3CIPID;
    for (pid = 0; pid < mdef->n_phone; pid++) {
	p = pid2phmm[pid];

	if (IS_S3CIPID(mdef->phone[pid].lc) && mdef->ciphone[(unsigned)mdef->phone[pid].lc].filler) {
	    for (i = 0; IS_S3CIPID(filler[i]); i++)
		lrc_set (p->lc, filler[i]);
	} else
	    lrc_set (p->lc, mdef->phone[pid].lc);
	
	if (IS_S3CIPID(mdef->phone[pid].rc) && mdef->ciphone[(unsigned)mdef->phone[pid].rc].filler) {
	    for (i = 0; IS_S3CIPID(filler[i]); i++)
		lrc_set (p->rc, filler[i]);
	} else
	    lrc_set (p->rc, mdef->phone[pid].rc);
    }
    ckd_free (pid2phmm);
    ckd_free (filler);
    
    /* Create links between PHMM nodes */
    n_link = phmm_link ();
    
    E_INFO ("%d nodes, %d links\n", n_phmm, n_link);
    
    return 0;
}


#if 0
static void phmm_dump ( void )
{
    s3cipid_t ci, lc, rc;
    phmm_t *p;
    plink_t *l;
    
    printf ("Nodes:\n");
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    printf ("%5d\t%s", p->pid, mdef_ciphone_str (mdef, p->ci));
	    printf ("\tLC=");
	    for (lc = 0; lc < mdef->n_ciphone; lc++)
		if (lrc_is_set (p->lc, lc))
		    printf (" %s", mdef_ciphone_str (mdef, lc));
	    printf ("\tRC=");
	    for (rc = 0; rc < mdef->n_ciphone; rc++)
		if (lrc_is_set (p->rc, rc))
		    printf (" %s", mdef_ciphone_str (mdef, rc));
	    printf ("\n");
	}
    }

    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    printf ("%5d -> ", p->pid);
	    for (l = p->succlist; l; l = l->next)
		printf (" %5d", l->phmm->pid);
	    printf ("\n");
	}
    }
}
#endif

/*
 * Check model tprob matrices that they conform to upper-diagonal assumption.
 */
static void chk_tp_uppertri ( void )
{
    int32 i, n_state, from, to;
    
    n_state = mdef->n_emit_state;
    
    /* Check that each tmat is upper-triangular */
    for (i = 0; i < tmat->n_tmat; i++) {
	for (to = 0; to < n_state; to++)
	    for (from = to+1; from < n_state; from++)
		if (tmat->tp[i][from][to] > S3_LOGPROB_ZERO)
		    E_FATAL("HMM transition matrix not upper triangular\n");
    }
}


int32 allphone_start_utt (char *uttid)
{
    s3cipid_t ci;
    phmm_t *p;
    int32 s;
    
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    p->active = -1;
	    p->inscore = S3_LOGPROB_ZERO;
	    p->bestscore = S3_LOGPROB_ZERO;

	    for (s = 0; s <= mdef->n_emit_state; s++) {
		p->score[s] = S3_LOGPROB_ZERO;
		p->hist[s] = NULL;
	    }
	}
    }

    curfrm = 0;

    /* Initialize start state of the SILENCE PHMM */
    ci = mdef_ciphone_id (mdef, S3_SILENCE_CIPHONE);
    if (NOT_S3CIPID(ci))
	E_FATAL("Cannot find CI-phone %s\n", S3_SILENCE_CIPHONE);
    for (p = ci_phmm[(unsigned)ci]; p && (p->pid != ci); p = p->next);
    if (! p)
	E_FATAL("Cannot find HMM for %s\n", S3_SILENCE_CIPHONE);
    p->inscore = 0;
    p->inhist = NULL;
    p->active = curfrm;
    
    n_histnode = 0;
    
    return 0;
}


static void phmm_eval (phmm_t *p, int32 *senscr)
{
    int32 **tp;
    int32 nst, from, to, bestfrom, newscr, bestscr;
    history_t *besthist = (history_t *)0;
    
    nst = mdef->n_emit_state;
    tp = tmat->tp[p->tmat];
    
    bestscr = S3_LOGPROB_ZERO;
    
    /* Update state scores from last to first (assuming no backward transitions) */
    for (to = nst-1; to >= 0; --to) {
	/* Find best incoming score to the "to" state from predecessor states */
	bestfrom = S3_LOGPROB_ZERO;
	for (from = to; from >= 0; from--) {
	    if ((tp[from][to] > S3_LOGPROB_ZERO) && (p->score[from] > S3_LOGPROB_ZERO)) {
		newscr = p->score[from] + tp[from][to];
		if (newscr > bestfrom) {
		    bestfrom = newscr;
		    besthist = p->hist[from];
		}
	    }
	}

	/* If looking at initial state, also consider incoming score */
	if ((to == 0) && (p->inscore > bestfrom)) {
	    bestfrom = p->inscore;
	    besthist = p->inhist;
	}
	
	/* Update state score */
	if (bestfrom > S3_LOGPROB_ZERO) {
	    p->score[to] = bestfrom + senscr[p->sen[to]];
	    p->hist[to] = besthist;

	    if (p->score[to] > bestscr)
		bestscr = p->score[to];
	}
    }

    /* Update non-emitting exit state score */
    bestfrom = S3_LOGPROB_ZERO;
    to = nst;
    for (from = nst-1; from >= 0; from--) {
	if ((tp[from][to] > S3_LOGPROB_ZERO) && (p->score[from] > S3_LOGPROB_ZERO)) {
	    newscr = p->score[from] + tp[from][to];
	    if (newscr > bestfrom) {
		bestfrom = newscr;
		besthist = p->hist[from];
	    }
	}
    }
    p->score[to] = bestfrom;
    p->hist[to] = besthist;
    if (p->score[to] > bestscr)
	bestscr = p->score[to];
    
    p->bestscore = bestscr;
}


/* Evaluate active PHMMs */
static int32 phmm_eval_all (int32 *senscr)
{
    s3cipid_t ci;
    phmm_t *p;
    int32 best;
    
    best = S3_LOGPROB_ZERO;

    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    if (p->active == curfrm) {
		phmm_eval (p, senscr);
		if (p->bestscore > best)
		    best = p->bestscore;
	    }
	}
    }

    return best;
}


static void phmm_exit (int32 best)
{
    s3cipid_t ci;
    phmm_t *p;
    int32 th, nf, nst, s;
    history_t *h;
    
    th = best + beam;
    
    frm_hist[curfrm] = NULL;
    nf = curfrm+1;
    nst = mdef->n_emit_state;
    
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
	for (p = ci_phmm[(unsigned)ci]; p; p = p->next) {
	    if (p->active == curfrm) {
		if (p->bestscore >= th) {
		    /* Scale state scores to prevent underflow */
		    for (s = 0; s <= nst; s++)
			if (p->score[s] > S3_LOGPROB_ZERO)
			    p->score[s] -= best;
		    
		    /* Create lattice entry if exiting */
		    if (p->score[nst] >= beam) { /* beam, not th because scores scaled */
			h = (history_t *) listelem_alloc (sizeof(history_t));
			h->score = p->score[nst];
			h->ef = curfrm;
			h->phmm = p;
			h->hist = p->hist[nst];
			
			h->next = frm_hist[curfrm];
			frm_hist[curfrm] = h;
			
			n_histnode++;
		    }
		    
		    /* Mark PHMM active in next frame */
		    p->active = nf;
		} else {
		    /* Reset state scores */
		    for (s = 0; s <= nst; s++) {
			p->score[s] = S3_LOGPROB_ZERO;
			p->hist[s] = NULL;
		    }
		}
	    }
	    
	    /* Reset incoming score in preparation for cross-PHMM transition */
	    p->inscore = S3_LOGPROB_ZERO;
	}
    }
}


static void phmm_trans ( void )
{
    history_t *h;
    phmm_t *from, *to;
    plink_t *l;
    int32 newscore, nf;
    
    nf = curfrm+1;
    
    /* Transition from exited nodes to initial states of HMMs */
    for (h = frm_hist[curfrm]; h; h = h->next) {
	from = h->phmm;
	for (l = from->succlist; l; l = l->next) {
	    to = l->phmm;

	    newscore = h->score + tp[(unsigned)from->ci][(unsigned)to->ci];
	    if ((newscore > beam) && (newscore > to->inscore)) {
		to->inscore = newscore;
		to->inhist = h;
		to->active = nf;
	    }
	}
    }
}


int32 allphone_frame (int32 *senscr)
{
    int32 bestscr;
    
    bestscr = phmm_eval_all (senscr);
    score_scale[curfrm] = bestscr;

    phmm_exit (bestscr);
    phmm_trans ();
    
    curfrm++;
    
    return 0;
}


/* Return accumulated score scale in frame range [sf..ef] */
static int32 seg_score_scale (int32 sf, int32 ef)
{
    int32 scale, s;
    
    for (s = sf, scale = 0; s <= ef; s++, scale += score_scale[s]);
    return scale;
}


/* Phone lattice node */
typedef struct phlatnode_s {
    s3cipid_t ci;
    uint16 fef, lef;	/* First and last end frame for this node */
    struct phlatnode_s *next;
} phlatnode_t;


static void allphone_latdump (char *uttid, char *latdir)
{
    int32 f, sf, latbeam, best, thresh, nnode;
    history_t *h;
    char filename[4096];
    FILE *fp;
    float64 *f64arg;
    phlatnode_t **phlatnode, *p;
    
    f64arg = (float64 *) cmd_ln_access ("-phlatbeam");
    latbeam = logs3 (*f64arg);

    sprintf (filename, "%s/%s.phlat", latdir, uttid);
    if ((fp = fopen(filename, "w")) == NULL) {
	E_ERROR("fopen(%s,w) failed\n", filename);
	return;
    }

    phlatnode = (phlatnode_t **) ckd_calloc (curfrm+1, sizeof(phlatnode_t));
    
    for (f = 0; f < curfrm; f++) {
	/* Find best score for this frame and set pruning threshold */
	best = (int32)0x80000000;
	for (h = frm_hist[f]; h; h = h->next)
	    if (h->score > best)
		best = h->score;
	thresh = best + latbeam;
	
	for (h = frm_hist[f]; h; h = h->next) {
	    /* Skip this node if below threshold */
	    if (h->score < thresh)
		continue;
	    
	    sf = h->hist ? h->hist->ef + 1 : 0;
	    assert (h->ef == f);
	    
	    /* Find phlatnode for this <ci,sf> pair */
	    for (p = phlatnode[sf]; p && (p->ci != h->phmm->ci); p = p->next);
	    if (! p) {
		p = (phlatnode_t *) listelem_alloc (sizeof(phlatnode_t));
		p->next = phlatnode[sf];
		phlatnode[sf] = p;
		p->ci = h->phmm->ci;
		p->fef = p->lef = h->ef;
	    }
	    assert (p->lef <= h->ef);
	    p->lef = h->ef;
#if 0
	    score = h->score;
	    if (h->hist)
		score -= h->hist->score;
	    score += seg_score_scale (sf, h->ef);
	    fprintf (fp, "%4d %3d %12d %s\n",	/* startfrm endfrm ciphone */
		     sf, h->ef - sf + 1, score, mdef_ciphone_str (mdef, h->phmm->ci));
#endif
	}
    }

    /* Write phone lattice; startframe, first end frame, last end frame, ciphone */
    nnode = 0;
    for (f = 0; f <= curfrm; f++) {
	for (p = phlatnode[f]; p; p = p->next) {
	    fprintf (fp, "%4d %4d %4d %s\n", f, p->fef, p->lef,
		     mdef_ciphone_str (mdef, p->ci));
	    nnode++;
	}
    }
    E_INFO("%d phone lattice nodes written to %s\n", nnode, filename);

    /* Free phone lattice */
    for (f = 0; f <= curfrm; f++) {
	for (p = phlatnode[f]; p; p = phlatnode[f]) {
	    phlatnode[f] = p->next;
	    listelem_free ((char *)p, sizeof(phlatnode_t));
	}
    }
    ckd_free (phlatnode);
    
    fclose (fp);
}


phseg_t *allphone_end_utt (char *uttid)
{
    history_t *h, *nexth, *besth = (history_t *)0;
    int32 f, best;
    phseg_t *s, *nexts;
    char *phlatdir;
    
    /* Free old phseg, if any */
    for (s = phseg; s; s = nexts) {
	nexts = s->next;
	listelem_free ((char *)s, sizeof(phseg_t));
    }
    phseg = NULL;
    
    /* Write phone lattice if specified */
    if ((phlatdir = (char *) cmd_ln_access ("-phlatdir")) != NULL)
	allphone_latdump (uttid, phlatdir);
    
    /* Find most recent history nodes list */
    for (f = curfrm-1; (f >= 0) && (frm_hist[f] == NULL); --f);

    if (f >= 0) {
	/* Find best of the most recent history nodes */
	best = (int32) 0x80000000;
	for (h = frm_hist[f]; h; h = h->next) {
	    if (h->score > best) {
		best = h->score;
		besth = h;
	    }
	}
	
	/* Backtrace */
	for (h = besth; h; h = h->hist) {
	    s = (phseg_t *) listelem_alloc (sizeof(phseg_t));
	    s->ci = h->phmm->ci;
	    s->sf = (h->hist) ? h->hist->ef + 1 : 0;
	    s->ef = h->ef;
	    s->score = h->score;
	    if (h->hist)
		s->score -= h->hist->score;
	    s->score += seg_score_scale (s->sf, s->ef);
	    
	    s->next = phseg;
	    phseg = s;
	}
    }
    
    E_INFO("%10d history nodes created\n", n_histnode);
    
    /* Free history nodes */
    for (f = 0; f < curfrm; f++) {
	for (h = frm_hist[f]; h; h = nexth) {
	    nexth = h->next;
	    listelem_free ((char *) h, sizeof(history_t));
	}
	
	frm_hist[f] = NULL;
    }
    
    return phseg;
}


static void phone_tp_init (char *file, float64 floor, float64 wt, float64 ip)
{
    int32 i, j, ct, tot, inspen;
    FILE *fp;
    char p1[128], p2[128];
    s3cipid_t pid1, pid2;
    float64 p;
    
    tp = (int32 **) ckd_calloc_2d (mdef->n_ciphone, mdef->n_ciphone, sizeof(int32));
    inspen = logs3 (ip);
    
    if (! file) {
	for (i = 0; i < mdef->n_ciphone; i++)
	    for (j = 0; j < mdef->n_ciphone; j++)
		tp[i][j] = inspen;
	return;
    }

    for (i = 0; i < mdef->n_ciphone; i++)
	for (j = 0; j < mdef->n_ciphone; j++)
	    tp[i][j] = S3_LOGPROB_ZERO;
    
    if ((fp = fopen(file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);
    while (fscanf (fp, "%s %s %d %d", p1, p2, &ct, &tot) == 4) {
	pid1 = mdef_ciphone_id (mdef, p1);
	if (NOT_S3CIPID(pid1))
	    E_FATAL("Bad phone: %s\n", p1);
	pid2 = mdef_ciphone_id (mdef, p2);
	if (NOT_S3CIPID(pid2))
	    E_FATAL("Bad phone: %s\n", p2);
	
	if (tot > 0)
	    p = ((float64)ct)/((float64)tot);
	else
	    p = 0.0;
	if (p < floor)
	    p = floor;
	
	tp[(unsigned)pid1][(unsigned)pid2] = (int32)(logs3(p) * wt) + inspen;
    }
    
    fclose (fp);
}


int32 allphone_init ( mdef_t *mdef, tmat_t *tmat )
{
    float64 *f64arg;
    char *file;
    float64 tpfloor, ip, wt;
    
    chk_tp_uppertri ();
    
    phmm_build ();
    file = (char *)cmd_ln_access("-phonetpfn");
    if (! file)
	E_ERROR("-phonetpfn argument missing; assuming uniform transition probs\n");
    tpfloor = *((float32 *) cmd_ln_access ("-phonetpfloor"));
    ip = *((float32 *) cmd_ln_access ("-inspen"));
    wt = *((float32 *) cmd_ln_access ("-phonetpwt"));
    phone_tp_init (file, tpfloor, wt, ip);
    
    f64arg = (float64 *) cmd_ln_access ("-beam");
    beam = logs3 (*f64arg);
    E_INFO ("logs3(beam)= %d\n", beam);

    frm_hist = (history_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(history_t *));
    score_scale = (int32 *) ckd_calloc (S3_MAX_FRAMES, sizeof(int32));

    phseg = NULL;
    
    return 0;
}
