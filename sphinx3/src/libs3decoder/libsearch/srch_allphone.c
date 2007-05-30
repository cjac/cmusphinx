/* -*- c-basic-offset: 4 -*- */
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

/**
 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 */

#include "srch.h"
#include "srch_allphone.h"
#include "s3types.h"

/**
 * \struct phmm_t
 * \brief Phone-HMM (PHMM) structure
 *
 * Models a single unique <senone-sequence, tmat> pair.
 * Can represent several different triphones, but all with the same parent basephone.
 * (NOTE: Word-position attribute of triphone is ignored.)
 */
typedef struct phmm_s {
    s3pid_t pid;        /**< Phone id (temp. during init.) */
    s3tmatid_t tmat;    /**< Transition matrix id for this PHMM */
    s3cipid_t ci;       /**< Parent basephone for this PHMM */
    s3frmid_t active;   /**< Latest frame in which this PHMM is/was active */
    uint32 *lc;         /**< Set (bit-vector) of left context phones seen for this PHMM */
    uint32 *rc;         /**< Set (bit-vector) of right context phones seen for this PHMM */
    s3senid_t *sen;     /**< Senone-id sequence underlying this PHMM */
    int32 *score;       /**< Total path score during Viterbi decoding */
    struct history_s **hist;    /**< Viterbi history (for backtrace) */
    int32 bestscore;    /**< Best state score in any frame */
    int32 inscore;      /**< Incoming score from predecessor PHMMs */
    int32 in_tscore;    /**< Incoming transition score from predecessor PHMMs */
    struct history_s *inhist;   /**< History corresponding to inscore */
    struct phmm_s *next;        /**< Next unique PHMM for same parent basephone */
    struct plink_s *succlist;   /**< List of predecessor PHMM nodes */
} phmm_t;

/**
 * List of links from a PHMM node to its successors; one link per successor.
 */
typedef struct plink_s {
    phmm_t *phmm;               /**< Successor PHMM node */
    struct plink_s *next;       /**< Next link for parent PHMM node */
} plink_t;

/**
 * History (paths) information at any point in allphone Viterbi search.
 */
typedef struct history_s {
    phmm_t *phmm;       /**< PHMM ending this path */
    int32 score;        /**< Path score for this path */
    int32 tscore;       /**< Transition score for this path */
    s3frmid_t ef;       /**< End frame */
    struct history_s *hist;     /**< Previous history entry */
    struct history_s *next;     /**< Next in allocated list */
} history_t;

/**
 * Allphone search structure.
 */

typedef struct allphone_s {
    phmm_t **ci_phmm; /**< PHMM lists (for each CI phone) */
    history_t **frm_hist;    /**< List of history nodes allocated in each frame */
    s3lmwid32_t *ci2lmwid;   /**< Mapping from CI-phone-id to LM-word-id */

    mdef_t *mdef;	     /**< Model definition (linked from kbcore) */
    tmat_t *tmat;	     /**< Transition matrices (linked from kbcore) */
    lm_t *lm;   	     /**< Language model (linked from kbcore) */

    int32 curfrm;            /**< Current frame */
    int32 beam, pbeam, inspen;
    int32 *score_scale;      /**< Score by which state scores scaled in each frame */
    phseg_t *phseg;          /**< Phoneme segmentation. */
    int32 n_histnode;        /**< No. of history entries */
} allphone_t;

/**
 * Find PHMM node with same senone sequence and tmat id as the given triphone.
 * Return ptr to PHMM node if found, NULL otherwise.
 */
static phmm_t *
phmm_lookup(allphone_t *allp, s3pid_t pid)
{
    phmm_t *p;
    phone_t *old, *new;
    mdef_t *mdef;
    phmm_t **ci_phmm;

    mdef = allp->mdef;
    ci_phmm = allp->ci_phmm;

    new = &(mdef->phone[pid]);

    for (p = ci_phmm[(unsigned) mdef->phone[pid].ci]; p; p = p->next) {
        old = &(mdef->phone[p->pid]);
        if (old->tmat == new->tmat) {
            if (old->ssid == new->ssid)
                return p;
        }
    }

    return NULL;
}


static void
lrc_set(uint32 * vec, int32 ci, int32 lrc_size)
{
    int32 i, j;

    assert(lrc_size > 0);

    /* If lc or rc not specified, set all flags */
    if (NOT_S3CIPID(ci)) {
        for (i = 0; i < lrc_size; i++)
            vec[i] = (uint32) 0xffffffff;
    }
    else {
        i = (ci >> 5);
        j = ci - (i << 5);
        vec[i] |= (1 << j);
    }
}


static int32
lrc_is_set(uint32 * vec, int32 ci)
{
    int32 i, j;

    i = (ci >> 5);
    j = ci - (i << 5);
    return (vec[i] & (1 << j));
}


static int32
phmm_link(allphone_t *allp)
{
    s3cipid_t ci, rc;
    phmm_t *p, *p2;
    int32 *rclist;
    int32 i, n_link;
    plink_t *l;
    mdef_t *mdef;
    phmm_t **ci_phmm;

    mdef = allp->mdef;
    ci_phmm = allp->ci_phmm;

    rclist = (int32 *) ckd_calloc(mdef->n_ciphone + 1, sizeof(int32));

    /* Create successor links between PHMM nodes */
    n_link = 0;
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        for (p = ci_phmm[(unsigned) ci]; p; p = p->next) {
            /* Build rclist for p */
            i = 0;
            for (rc = 0; rc < mdef->n_ciphone; rc++) {
                if (lrc_is_set(p->rc, rc))
                    rclist[i++] = rc;
            }
            rclist[i] = BAD_S3CIPID;

            /* For each rc in rclist, transition to PHMMs for rc if left context = ci */
            for (i = 0; IS_S3CIPID(rclist[i]); i++) {
                for (p2 = ci_phmm[rclist[i]]; p2; p2 = p2->next) {
                    if (lrc_is_set(p2->lc, ci)) {
                        /* transition from p to p2 */
                        l = (plink_t *) listelem_alloc(sizeof(plink_t));
                        l->phmm = p2;
                        l->next = p->succlist;
                        p->succlist = l;

                        n_link++;
                    }
                }
            }
        }
    }

    ckd_free(rclist);

    return n_link;
}


static int32
phmm_build(allphone_t *allp)
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
    phmm_t **ci_phmm;
    int32 lrc_size;
    mdef_t *mdef;
    tmat_t *tmat;

    E_INFO("Building PHMM net\n");

    mdef = allp->mdef;
    tmat = allp->tmat;

    ci_phmm = (phmm_t **) ckd_calloc(mdef->n_ciphone, sizeof(phmm_t *));
    pid2phmm = (phmm_t **) ckd_calloc(mdef->n_phone, sizeof(phmm_t *));

    for (lrc_size = 32; lrc_size < mdef->n_ciphone; lrc_size += 32);
    lrc_size >>= 5;

    /* For each unique ciphone/triphone entry in mdef, create a PHMM node */
    n_phmm = 0;
    for (pid = 0; pid < mdef->n_phone; pid++) {
        if ((p = phmm_lookup(allp, pid)) == NULL) {
            /* No previous entry; create a new one */
            p = (phmm_t *) listelem_alloc(sizeof(phmm_t));

            p->pid = pid;
            p->tmat = mdef->phone[pid].tmat;
            p->ci = mdef->phone[pid].ci;
            p->succlist = NULL;

            p->next = ci_phmm[(unsigned) p->ci];
            ci_phmm[(unsigned) p->ci] = p;

            n_phmm++;
        }

        pid2phmm[pid] = p;
    }

    /* Fill out rest of each PHMM node */
    sen =
        (s3senid_t *) ckd_calloc(n_phmm * mdef->n_emit_state,
                                 sizeof(s3senid_t));
    score =
        (int32 *) ckd_calloc(n_phmm * (mdef->n_emit_state + 1),
                             sizeof(int32));
    hist =
        (history_t **) ckd_calloc(n_phmm * (mdef->n_emit_state + 1),
                                  sizeof(history_t *));
    lc = (uint32 *) ckd_calloc(n_phmm * lrc_size * 2, sizeof(uint32));
    rc = lc + (n_phmm * lrc_size);
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        for (p = ci_phmm[(unsigned) ci]; p; p = p->next) {
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
    filler =
        (s3cipid_t *) ckd_calloc(mdef->n_ciphone + 1, sizeof(s3cipid_t));
    i = 0;
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        if (mdef->ciphone[(unsigned) ci].filler)
            filler[i++] = ci;
    }
    filler[i] = BAD_S3CIPID;
    for (pid = 0; pid < mdef->n_phone; pid++) {
        p = pid2phmm[pid];

        if (IS_S3CIPID(mdef->phone[pid].lc)
            && mdef->ciphone[(unsigned) mdef->phone[pid].lc].filler) {
            for (i = 0; IS_S3CIPID(filler[i]); i++)
                lrc_set(p->lc, filler[i], lrc_size);
        }
        else
            lrc_set(p->lc, mdef->phone[pid].lc, lrc_size);

        if (IS_S3CIPID(mdef->phone[pid].rc)
            && mdef->ciphone[(unsigned) mdef->phone[pid].rc].filler) {
            for (i = 0; IS_S3CIPID(filler[i]); i++)
                lrc_set(p->rc, filler[i], lrc_size);
        }
        else
            lrc_set(p->rc, mdef->phone[pid].rc, lrc_size);
    }
    ckd_free(pid2phmm);
    ckd_free(filler);

    /* Create links between PHMM nodes */
    n_link = phmm_link(allp);

    E_INFO("%d nodes, %d links\n", n_phmm, n_link);

    allp->ci_phmm = ci_phmm;

    return 0;
}


#if 0
static void
phmm_dump(void)
{
    s3cipid_t ci, lc, rc;
    phmm_t *p;
    plink_t *l;

    printf("Nodes:\n");
    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        for (p = ci_phmm[(unsigned) ci]; p; p = p->next) {
            printf("%5d\t%s", p->pid, mdef_ciphone_str(mdef, p->ci));
            printf("\tLC=");
            for (lc = 0; lc < mdef->n_ciphone; lc++)
                if (lrc_is_set(p->lc, lc))
                    printf(" %s", mdef_ciphone_str(mdef, lc));
            printf("\tRC=");
            for (rc = 0; rc < mdef->n_ciphone; rc++)
                if (lrc_is_set(p->rc, rc))
                    printf(" %s", mdef_ciphone_str(mdef, rc));
            printf("\n");
        }
    }

    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        for (p = ci_phmm[(unsigned) ci]; p; p = p->next) {
            printf("%5d -> ", p->pid);
            for (l = p->succlist; l; l = l->next)
                printf(" %5d", l->phmm->pid);
            printf("\n");
        }
    }
}
#endif

static void
phmm_eval(allphone_t *allp, phmm_t * p, int32 * senscr)
{
    int32 **tp;
    int32 nst, from, to, bestfrom, newscr, bestscr;
    history_t *besthist = (history_t *) 0;
    mdef_t *mdef;
    tmat_t *tmat;

    mdef = allp->mdef;
    tmat = allp->tmat;

    nst = mdef->n_emit_state;
    tp = tmat->tp[p->tmat];

    bestscr = S3_LOGPROB_ZERO;

    /* Update state scores from last to first (assuming no backward transitions) */
    for (to = nst - 1; to >= 0; --to) {
        /* Find best incoming score to the "to" state from predecessor states */
        bestfrom = S3_LOGPROB_ZERO;
        for (from = to; from >= 0; from--) {
            if ((tp[from][to] > S3_LOGPROB_ZERO)
                && (p->score[from] > S3_LOGPROB_ZERO)) {
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
    for (from = nst - 1; from >= 0; from--) {
        if ((tp[from][to] > S3_LOGPROB_ZERO)
            && (p->score[from] > S3_LOGPROB_ZERO)) {
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


/** Evaluate active PHMMs */
static int32
phmm_eval_all(allphone_t *allp, int32 *senscr, stat_t *st)
{
    s3cipid_t ci;
    phmm_t *p;
    int32 best;
    mdef_t *mdef;
    int32 curfrm;
    phmm_t **ci_phmm;

    mdef = allp->mdef;
    curfrm = allp->curfrm;
    ci_phmm = allp->ci_phmm;

    best = S3_LOGPROB_ZERO;

    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        for (p = ci_phmm[(unsigned) ci]; p; p = p->next) {
            if (p->active == curfrm) {
		++st->utt_hmm_eval;
                phmm_eval(allp, p, senscr);
                if (p->bestscore > best)
                    best = p->bestscore;
            }
        }
    }

    return best;
}


static void
phmm_exit(allphone_t *allp, int32 best)
{
    s3cipid_t ci;
    phmm_t *p;
    int32 th, nf, nst, s;
    history_t *h;
    history_t **frm_hist;
    mdef_t *mdef;
    int32 curfrm, n_histnode;
    phmm_t **ci_phmm;

    th = best + allp->pbeam;

    frm_hist = allp->frm_hist;
    mdef = allp->mdef;
    curfrm = allp->curfrm;
    n_histnode = allp->n_histnode;
    ci_phmm = allp->ci_phmm;

    frm_hist[curfrm] = NULL;
    nf = curfrm + 1;
    nst = mdef->n_emit_state;

    for (ci = 0; ci < mdef->n_ciphone; ci++) {
        for (p = ci_phmm[(unsigned) ci]; p; p = p->next) {
            if (p->active == curfrm) {
                if (p->bestscore >= th) {
                    /* Scale state scores to prevent underflow */
                    for (s = 0; s <= nst; s++)
                        if (p->score[s] > S3_LOGPROB_ZERO)
                            p->score[s] -= best;

                    /* Create lattice entry if exiting */
                    if (p->score[nst] >= allp->pbeam) { /* pbeam, not th because scores scaled */
                        h = (history_t *)
                            listelem_alloc(sizeof(history_t));
                        h->score = p->score[nst];
                        /* FIXME: This isn't going to be the correct
                         * transition score, for reasons I don't
                         * totally understand (dhuggins@cs,
                         * 2006-02-07). */
                        h->tscore = p->in_tscore;
                        h->ef = curfrm;
                        h->phmm = p;
                        h->hist = p->hist[nst];
                        h->next = frm_hist[curfrm];
                        frm_hist[curfrm] = h;

                        n_histnode++;
                    }

                    /* Mark PHMM active in next frame */
                    p->active = nf;
                }
                else {
                    /* Reset state scores */
                    for (s = 0; s <= nst; s++) {
                        p->score[s] = S3_LOGPROB_ZERO;
                        p->hist[s] = NULL;
                    }
                }
            }

            /* Reset incoming score in preparation for cross-PHMM transition */
            p->inscore = S3_LOGPROB_ZERO;
            p->in_tscore = S3_LOGPROB_ZERO;
        }
    }
}


static void
phmm_trans(allphone_t *allp)
{
    history_t *h;
    phmm_t *from, *to;
    plink_t *l;
    int32 newscore, nf, curfrm;
    s3lmwid32_t *ci2lmwid;
    lm_t *lm;

    curfrm = allp->curfrm;
    nf = curfrm + 1;
    ci2lmwid = allp->ci2lmwid;
    lm = allp->lm;

    /* Transition from exited nodes to initial states of HMMs */
    for (h = allp->frm_hist[curfrm]; h; h = h->next) {
        from = h->phmm;
        for (l = from->succlist; l; l = l->next) {
            int32 tscore;
            to = l->phmm;

	    /* No LM, just use uniform (insertion penalty). */
	    if (lm == NULL)
		tscore = allp->inspen;
	    /* If they are not in the LM, kill this
	     * transition. */
	    else if (ci2lmwid[to->ci] == BAD_LMWID(lm))
		tscore = S3_LOGPROB_ZERO;
	    else {
		if (h->hist && h->hist->phmm) {
		    tscore = lm_tg_score(lm,
					 ci2lmwid[h->hist->phmm->ci],
					 ci2lmwid[from->ci],
					 ci2lmwid[to->ci],
					 ci2lmwid[to->ci]);
		}
		else
		    tscore = lm_bg_score(lm,
					 ci2lmwid[from->ci],
					 ci2lmwid[to->ci],
					 ci2lmwid[to->ci]);
	    }

            newscore = h->score + tscore;
            if ((newscore > allp->beam) && (newscore > to->inscore)) {
                to->inscore = newscore;
                to->in_tscore = tscore;
                to->inhist = h;
                to->active = nf;
            }
        }
    }
}

static int32
_allphone_start_utt(allphone_t *allp)
{
    s3cipid_t ci;
    phmm_t *p;
    history_t *h, *nexth;
    int32 s, f;

    /* Reset all HMMs. */
    for (ci = 0; ci < allp->mdef->n_ciphone; ci++) {
        for (p = allp->ci_phmm[(unsigned) ci]; p; p = p->next) {
            p->active = -1;
            p->inscore = S3_LOGPROB_ZERO;
            p->bestscore = S3_LOGPROB_ZERO;

            for (s = 0; s <= allp->mdef->n_emit_state; s++) {
                p->score[s] = S3_LOGPROB_ZERO;
                p->hist[s] = NULL;
            }
        }
    }

    allp->curfrm = 0;

    /* Initialize start state of the SILENCE PHMM */
    ci = mdef_ciphone_id(allp->mdef, S3_SILENCE_CIPHONE);
    if (NOT_S3CIPID(ci))
        E_FATAL("Cannot find CI-phone %s\n", S3_SILENCE_CIPHONE);
    for (p = allp->ci_phmm[(unsigned) ci]; p && (p->pid != ci); p = p->next);
    if (!p)
        E_FATAL("Cannot find HMM for %s\n", S3_SILENCE_CIPHONE);
    p->inscore = 0;
    p->inhist = NULL;
    p->active = allp->curfrm;

    /* Free history nodes, if any */
    for (f = 0; f < allp->curfrm; f++) {
        for (h = allp->frm_hist[f]; h; h = nexth) {
            nexth = h->next;
            listelem_free((char *) h, sizeof(history_t));
        }

        allp->frm_hist[f] = NULL;
    }
    allp->n_histnode = 0;

    return 0;
}

static void
allphone_log_hypseg(srch_t *s,
                    phseg_t * hypptr,   /* In: Hypothesis */
		    char *uttid,
                    int32 nfrm, /* In: #frames in utterance */
                    int32 scl)
{                               /* In: Acoustic scaling for entire utt */
    phseg_t *h;
    int32 ascr, lscr, tscr;
    kbcore_t *kbcore = s->kbc;
    FILE *fp = s->matchsegfp;

    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
        ascr += h->score;
        lscr += h->tscore;      /* FIXME: unscaled score? */
        tscr += h->score + h->tscore;
    }

    fprintf(fp, "%s S %d T %d A %d L %d", uttid, scl, tscr, ascr, lscr);

    if (!hypptr)                /* HACK!! */
        fprintf(fp, " (null)\n");
    else {
        for (h = hypptr; h; h = h->next) {
            fprintf(fp, " %d %d %d %s", h->sf, h->score, h->tscore,
                    mdef_ciphone_str(kbcore_mdef(kbcore), h->ci));
        }
        fprintf(fp, " %d\n", nfrm);
    }

    fflush(fp);
}

static void
allphone_log_hypstr(srch_t *s, phseg_t * hypptr, char *uttid)
{
    kbcore_t *kbcore = s->kbc;
    FILE *fp = s->matchfp;
    phseg_t *h;

    if (!hypptr)                /* HACK!! */
        fprintf(fp, "(null)");

    for (h = hypptr; h; h = h->next) {
        fprintf(fp, "%s ", mdef_ciphone_str(kbcore_mdef(kbcore), h->ci));
    }
    fprintf(fp, " (%s)\n", uttid);
    fflush(fp);
}


static void
write_phseg(srch_t *s, char *dir, char *uttid, phseg_t * phseg)
{
    char str[1024];
    FILE *fp = (FILE *) 0;
    int32 uttscr;
    kbcore_t *kbcore = s->kbc;

    /* Attempt to write segmentation for this utt to a separate file */
    if (dir) {
        sprintf(str, "%s/%s.allp", dir, uttid);
        E_INFO("Writing phone segmentation to: %s\n", str);
        if ((fp = fopen(str, "w")) == NULL) {
            E_ERROR("fopen(%s,w) failed\n", str);
            dir = NULL;         /* Flag to indicate fp shouldn't be closed at the end */
        }
    }

    if (!dir) {
        fp = stdout;            /* Segmentations can be directed to stdout this way */
        E_INFO("Phone segmentation (%s):\n", uttid);
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }

    fprintf(fp, "\t%5s %5s %9s %s\n", "SFrm", "EFrm", "SegAScr", "Phone");
    fflush(fp);

    uttscr = 0;
    for (; phseg; phseg = phseg->next) {
        if (!dir) {
            fprintf(fp, "ph:%s>", uttid);
            fflush(fp);
        }
        fprintf(fp, "\t%5d %5d %9d %s\n",
                phseg->sf, phseg->ef, phseg->score,
                mdef_ciphone_str(kbcore_mdef(kbcore), phseg->ci));
        fflush(fp);
        uttscr += (phseg->score);
    }

    if (!dir) {
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }
    fprintf(fp, " Total score: %11d\n", uttscr);
    fflush(fp);
    if (dir)
        fclose(fp);
    else {
        fprintf(fp, "\n");
        fflush(fp);
    }
}

/** Return accumulated score scale in frame range [sf..ef] */
static int32
seg_score_scale(allphone_t *allp, int32 sf, int32 ef)
{
    int32 scale, s;

    for (s = sf, scale = 0; s <= ef; s++, scale += allp->score_scale[s]);
    return scale;
}

static phseg_t *
allphone_backtrace(allphone_t *allp, int32 f)
{
    int32 best;
    history_t *besth, *h;
    phseg_t *phseg, *s;

    besth = NULL;
    phseg = NULL;
    if (f >= 0) {
        /* Find best of the most recent history nodes */
        best = (int32) 0x80000000;
        for (h = allp->frm_hist[f]; h; h = h->next) {
            if (h->score > best) {
                best = h->score;
                besth = h;
            }
        }

        /* Backtrace */
        for (h = besth; h; h = h->hist) {
            s = (phseg_t *) listelem_alloc(sizeof(phseg_t));
            s->ci = h->phmm->ci;
            s->sf = (h->hist) ? h->hist->ef + 1 : 0;
            s->ef = h->ef;
            s->score = h->score;
            s->tscore = h->tscore;
            if (h->hist)
                s->score -= h->hist->score;
            s->score += seg_score_scale(allp, s->sf, s->ef);

            s->next = phseg;
            phseg = s;
        }
    }

    E_INFO("%10d history nodes created\n", allp->n_histnode);
    return phseg;
}

static void
allphone_clear_phseg(allphone_t *allp)
{
    phseg_t *s, *nexts;
    for (s = allp->phseg; s; s = nexts) {
        nexts = s->next;
        listelem_free((char *) s, sizeof(phseg_t));
    }
    allp->phseg = NULL;
}

static int32
_allphone_end_utt(allphone_t *allp)
{
    int32 f;

    /* Free old phseg, if any */
    allphone_clear_phseg(allp);

    /* Find most recent history nodes list */
    for (f = allp->curfrm - 1;
	 (f >= 0) && (allp->frm_hist[f] == NULL); --f) ;

    /* Now backtrace. */
    allp->phseg = allphone_backtrace(allp, f);

    return f;
}

static int
srch_allphone_init(kb_t *kb, void *srch)
{
    allphone_t *allp;
    kbcore_t *kbc;
    srch_t *s;

    kbc = kb->kbcore;
    s = (srch_t *) srch;
    allp = ckd_calloc(1, sizeof(*allp));

    allp->mdef = kbcore_mdef(kbc);
    allp->tmat = kbcore_tmat(kbc);
    allp->lm = kbcore_lm(kbc);

    phmm_build(allp);

    /* Build mapping of CI phones to LM word IDs. */
    if (allp->lm) {
	int32 i;

	allp->ci2lmwid = ckd_calloc(allp->mdef->n_ciphone, sizeof(s3lmwid32_t));
	for (i = 0; i < allp->mdef->n_ciphone; i++)
	    allp->ci2lmwid[i] = lm_wid(allp->lm,
				       (char *) mdef_ciphone_str(allp->mdef, i));
    }
    else {
	E_WARN("-lm argument missing; doing unconstrained phone-loop decoding\n");
	allp->inspen = allp->lm->wip;
    }

    allp->beam = logs3(cmd_ln_float64("-beam"));
    E_INFO("logs3(beam)= %d\n", allp->beam);
    allp->pbeam = logs3(cmd_ln_float64("-pbeam"));
    E_INFO("logs3(pbeam)= %d\n", allp->pbeam);

    allp->frm_hist =
        (history_t **) ckd_calloc(S3_MAX_FRAMES, sizeof(history_t *));
    allp->score_scale = (int32 *) ckd_calloc(S3_MAX_FRAMES, sizeof(int32));

    allp->phseg = NULL;

    s->grh->graph_struct = allp;
    s->grh->graph_type = GRAPH_STRUCT_PHMM;

    return SRCH_SUCCESS;
}

static int
srch_allphone_uninit(void *srch)
{
    srch_t *s;
    allphone_t *allp;

    s = (srch_t *) srch;
    allp = (allphone_t *) s->grh->graph_struct;

    allphone_clear_phseg(allp);
    ckd_free(allp->ci2lmwid);
    ckd_free(allp->frm_hist);
    ckd_free(allp->score_scale);
    ckd_free(allp);

    return SRCH_SUCCESS;
}

static int
srch_allphone_begin(void *srch)
{
    srch_t *s;
    allphone_t *allp;

    s = (srch_t *) srch;
    allp = (allphone_t *) s->grh->graph_struct;

    stat_clear_utt(s->stat);
    _allphone_start_utt(allp);

    return SRCH_SUCCESS;
}

static int
srch_allphone_end(void *srch)
{
    srch_t *s;
    allphone_t *allp;
    stat_t *st;
    int32 scl, i;

    s = (srch_t *) srch;
    allp = (allphone_t *) s->grh->graph_struct;
    st = s->stat;

    s->exit_id = _allphone_end_utt(allp);

    /* Write and/or log phoneme segmentation */
    write_phseg(s, (char *) cmd_ln_access("-phsegdir"), s->uttid, allp->phseg);
    if (s->matchfp)
        allphone_log_hypstr(s, allp->phseg, s->uttid);
    if (s->matchsegfp) {
	scl = 0;
	for (i = 0; i < allp->curfrm; ++i)
	    scl += allp->score_scale[i];
        allphone_log_hypseg(s, allp->phseg, s->uttid, allp->curfrm, scl);
    }

    return SRCH_SUCCESS;
}

static int
srch_allphone_srch_one_frame_lv2(void *srch)
{
    srch_t *s;
    allphone_t *allp;
    int32 bestscr;

    s = (srch_t *) srch;
    allp = (allphone_t *) s->grh->graph_struct;

    bestscr = phmm_eval_all(allp, s->ascr->senscr, s->stat);
    allp->score_scale[allp->curfrm] = bestscr;

    phmm_exit(allp, bestscr);
    phmm_trans(allp);

    ++allp->curfrm;

    return SRCH_SUCCESS;
}

static int
srch_allphone_shift_one_cache_frame(void *srch, int32 win_efv)
{
    ascr_t *ascr;
    srch_t *s;

    s = (srch_t *) srch;
    ascr = s->ascr;

    ascr_shift_one_cache_frame(ascr, win_efv);

    return SRCH_SUCCESS;
}

static int
srch_allphone_select_active_gmm(void *srch)
{
    ascr_t *ascr;
    allphone_t *allp;
    srch_t *s;
    int32 ci, ss;
    phmm_t *p;

    s = (srch_t *) srch;
    ascr = s->ascr;
    allp = (allphone_t *) s->grh->graph_struct;

    ascr_clear_sen_active(ascr);

    for (ci = 0; ci < allp->mdef->n_ciphone; ci++) {
	for (p = allp->ci_phmm[(unsigned) ci]; p; p = p->next) {
	    if (p->active == allp->curfrm) {
		for (ss = 0; ss < allp->mdef->n_emit_state; s++) {
		    ascr->sen_active[p->sen[ss]] = 1;
		}
	    }
	}
    }

    return SRCH_SUCCESS;
}

glist_t
srch_allphone_gen_hyp(void *srch)
{
    srch_t *s;
    allphone_t *allp;
    glist_t hyp;
    phseg_t *p;

    s = (srch_t *) srch;
    allp = (allphone_t *) s->grh->graph_struct;

    if (s->exit_id == -1) { /* Search not finished */
	int32 f;

	allphone_clear_phseg(allp);
	for (f = allp->curfrm - 1;
	     (f >= 0) && (allp->frm_hist[f] == NULL); --f) ;
	allp->phseg = allphone_backtrace(allp, f);
    }

    if (allp->phseg == NULL) {
        E_WARN("Failed to retrieve phone segmentation.\n");
        return NULL;
    }

    /* Now create a glist from it */
    hyp = NULL;
    for (p = allp->phseg; p; p = p->next) {
	srch_hyp_t *h;

        h = (srch_hyp_t *) ckd_calloc(1, sizeof(srch_hyp_t));
	h->id = p->ci;
	h->sf = p->sf;
	h->ef = p->ef;
	h->ascr = p->score;
	h->lscr = p->tscore;

	/* FIXME: Will this leak memory from h? */
        hyp = glist_add_ptr(hyp, h);
    }

    return hyp;
}

/* Pointers to all functions */
srch_funcs_t srch_allphone_funcs = {
	/* init */			srch_allphone_init,
	/* uninit */			srch_allphone_uninit,
	/* utt_begin */ 		srch_allphone_begin,
	/* utt_end */   		srch_allphone_end,
	/* decode */			NULL,
	/* set_lm */			NULL,
	/* add_lm */			NULL,
	/* delete_lm */ 		NULL,

	/* gmm_compute_lv1 */		approx_ci_gmm_compute,
	/* one_srch_frame_lv1 */	NULL,
	/* hmm_compute_lv1 */		NULL,
	/* eval_beams_lv1 */		NULL,
	/* propagate_graph_ph_lv1 */	NULL,
	/* propagate_graph_wd_lv1 */	NULL,

	/* gmm_compute_lv2 */		s3_cd_gmm_compute_sen,
	/* one_srch_frame_lv2 */	srch_allphone_srch_one_frame_lv2,
	/* hmm_compute_lv2 */		NULL,
	/* eval_beams_lv2 */		NULL,
	/* propagate_graph_ph_lv2 */	NULL,
	/* propagate_graph_wd_lv2 */	NULL,

	/* rescoring */			NULL,
	/* frame_windup */		NULL,
	/* compute_heuristic */		NULL,
	/* shift_one_cache_frame */	srch_allphone_shift_one_cache_frame,
	/* select_active_gmm */		srch_allphone_select_active_gmm,

	/* gen_hyp */			srch_allphone_gen_hyp,
	/* gen_dag */			NULL,
	/* dump_vithist */		NULL,
	/* bestpath_impl */		NULL,
	/* dag_dump */			NULL,
	NULL
};
