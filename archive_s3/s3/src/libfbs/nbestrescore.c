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
 * nbestrescore.c -- Alpha (forward algorithm) rescoring of N-best lists.
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
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started, based on align.c.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <s3.h>
#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "logs3.h"
#include "search.h"


/*
 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 */


static dict_t *dict;		/* The dictionary */
static mdef_t *mdef;		/* Model definition */
static tmat_t *tmat;		/* Transition probability matrices */

static s3cipid_t silphone;

static int32 n_frm;		/* Current frame in current utterance */
static int32 beam;		/* Pruning beamwidth */
static int32 *score_scale;	/* Score by which state scores scaled in each frame */

typedef struct snode_s {
    s3senid_t senid;		/* Senone id for this state */
    s3frmid_t active;		/* Last frame in which active */
    int32 score;		/* Path score (normalized) */
    struct slink_s *predlist;	/* Predecessor snodes list */
    struct snode_s *next;	/* Next in allocated list (for memory management) */
} snode_t;

typedef struct slink_s {
    int32 tp;			/* Transition probability */
    snode_t *node;		/* Predecessor node at the end of the link */
    struct slink_s *next;	/* Next link for same parent snode */
} slink_t;

typedef struct senthmm_s {
    snode_t *snodelist;		/* Actual sentence HMM states for the utterance;
				   snodelist is actually reversed; i.e. nodes earlier
				   in the list can succeed those later in the list, but
				   not vice versa */
    snode_t *first_active, *last_active;
				/* First and last active ones FOUND IN snodelist; i.e.,
				   last_active is actually earlier in the HMM topology */
    snode_t *entry, *exit;	/* Dummy states for entering and exiting sentence HMM */
    struct senthmm_s *next;	/* Next sentence HMM (in the order of the Nbest list) */
} senthmm_t;
static senthmm_t *senthmm_head, *senthmm_tail;


static void link_snodes (snode_t *src, snode_t *dst, int32 prob)
{
    slink_t *l;
    
    l = (slink_t *) listelem_alloc (sizeof(slink_t));
    l->node = src;
    l->tp = prob;
    l->next = dst->predlist;
    dst->predlist = l;
}


/*
 * Append states of phone pid to sentence HMM.  The pid is inserted just before the
 * exit state of the sentence HMM.
 */
static void append_phone (s3pid_t pid,		/* Triphone to append */
			  senthmm_t *senthmm)	/* Sentence HMM be appended to */
{
    int32 n_state;
    int32 st, pst;
    snode_t *snode;
    s3senid_t *senid;
    int32 **tp;
    static snode_t **phone_snode = NULL;	/* States for this phone being added */
    
    /* A few checks */
    if ((pid < mdef->n_ciphone) && (! mdef->ciphone[pid].filler))
	E_WARN ("Using CIphone %s\n", mdef_ciphone_str (mdef, pid));
    
    n_state = mdef->n_emit_state;

    /* One time allocation */
    if (! phone_snode)
	phone_snode = (snode_t **) ckd_calloc (n_state, sizeof(snode_t *));

    /* Allocate state nodes for this pid */
    for (st = 0; st < n_state; st++)
	phone_snode[st] = (snode_t *) listelem_alloc (sizeof(snode_t));
    
    senid = mdef->phone[pid].state;
    tp = tmat->tp[mdef->phone[pid].tmat];
    
    /* Fill in state node parameters */
    for (st = 0; st < n_state; st++) {
	snode = phone_snode[st];

	snode->senid = senid[st];
	snode->active = -1;
	snode->score = LOGPROB_ZERO;
	snode->next = senthmm->snodelist;
	senthmm->snodelist = snode;
	
	/*
	 * Initialize predecessor list for state.  The start state for this pid gets
	 * the predlist for the exit state since this pid is inserted just before the
	 * exit state.
	 */
	snode->predlist = (st == 0) ? senthmm->exit->predlist : NULL;
	
	/* Predecessor states; assumes Bakis topo (no backward link) */
	for (pst = 0; pst <= st; pst++) {
	    if (tp[pst][st] > LOGPROB_ZERO)
		link_snodes (phone_snode[pst], snode, tp[pst][st]);
	}
    }

    /* Link exit state to exiting arcs from this phone */
    senthmm->exit->predlist = NULL;
    for (pst = 0; pst < st; pst++) {
	if (tp[pst][st] > LOGPROB_ZERO)
	    link_snodes (phone_snode[pst], senthmm->exit, tp[pst][st]);
    }
}


/*
 * Append word w to end of partial sentence HMM.
 */
static void append_word (s3wid_t w,		/* Word to be appended */
			 senthmm_t *senthmm,	/* Sentence HMM be appended to */
			 s3cipid_t lc,		/* Left context for w */
			 s3cipid_t rc)		/* Right context for w */
{
    int32 pronlen, pron;
    s3pid_t pid;
    
    if ((pronlen = dict->word[w].pronlen) == 1) {
	/* Single phone case */
	pid = mdef_phone_id_nearest (mdef, dict->word[w].ciphone[0], lc, rc,
				     WORD_POSN_SINGLE);
	
	append_phone (pid, senthmm);
    } else {
	/* Multi-phone case; do first phone first */
	assert (pronlen > 1);
	
	pid = mdef_phone_id_nearest (mdef,
				     dict->word[w].ciphone[0],
				     lc,
				     dict->word[w].ciphone[1],
				     WORD_POSN_BEGIN);

	append_phone (pid, senthmm);

	/* Word internal phones */
	for (pron = 1; pron < pronlen-1; pron++) {
	    pid = mdef_phone_id_nearest (mdef,
					 dict->word[w].ciphone[pron],
					 dict->word[w].ciphone[pron-1],
					 dict->word[w].ciphone[pron+1],
					 WORD_POSN_INTERNAL);
	
	    append_phone (pid, senthmm);
	}

	/* Final phone */
	pid = mdef_phone_id_nearest (mdef,
				     dict->word[w].ciphone[pronlen-1],
				     dict->word[w].ciphone[pronlen-2],
				     rc,
				     WORD_POSN_END);
	
	append_phone (pid, senthmm);
    }
}


/*
 * Create sentence HMM for the given word list.
 */
static senthmm_t *senthmm_build (hyp_t *hyp)
{
    senthmm_t *senthmm;
    s3cipid_t lc, rc;
    hyp_t *h;
    int32 pronlen;
    
    assert (hyp);
    
    senthmm = (senthmm_t *) listelem_alloc (sizeof(senthmm_t));
    
    senthmm->snodelist = NULL;
    senthmm->entry = (snode_t *) listelem_alloc (sizeof(snode_t));
    senthmm->exit = (snode_t *) listelem_alloc (sizeof(snode_t));
    senthmm->next = NULL;

    senthmm->entry->score = 0;
    senthmm->entry->predlist = NULL;
    senthmm->entry->next = NULL;
    senthmm->entry->senid = BAD_SENID;
    
    senthmm->exit->score = 0;
    senthmm->exit->predlist = NULL;
    senthmm->exit->next = NULL;
    senthmm->exit->senid = BAD_SENID;
    
    /* Initially NULL transition from entry to exit with logprob = 0 */
    link_snodes (senthmm->entry, senthmm->exit, 0);

    lc = silphone;
    for (h = hyp; h->next; h = h->next) {
	rc = dict->word[h->next->wid].ciphone[0];
	append_word (h->wid, senthmm, lc, rc);
	pronlen = dict->word[h->wid].pronlen;
	lc = dict->word[h->wid].ciphone[pronlen-1];
    }
    
    append_word (h->wid, senthmm, lc, silphone);
    
    return senthmm;
}


static void slinks_free (slink_t *l)
{
    slink_t *tmpl;
    
    for (; l; l = tmpl) {
	tmpl = l->next;
	listelem_free ((char *) l, sizeof(slink_t));
    }
}


static void senthmm_destroy ( void )
{
    senthmm_t *tmpsent;
    snode_t *snode, *tmpsnode;
    
    for (; senthmm_head; senthmm_head = tmpsent) {
	tmpsent = senthmm_head->next;
	
	for (snode = senthmm_head->snodelist; snode; snode = tmpsnode) {
	    slinks_free (snode->predlist);
	    tmpsnode = snode->next;
	    listelem_free ((char *)snode, sizeof(snode_t));
	}
	
	slinks_free (senthmm_head->entry->predlist);
	listelem_free ((char *) senthmm_head->entry, sizeof(snode_t));

	slinks_free (senthmm_head->exit->predlist);
	listelem_free ((char *) senthmm_head->exit, sizeof(snode_t));
	
	listelem_free ((char *) senthmm_head, sizeof(senthmm_t));
    }
}


/*
 * hyplist does not contain valid word id fields; only word strings.
 */
int32 alpha_start_utt (hyp_t **hyplist, int32 nhyp, char *uttid)
{
    int32 i;
    hyp_t *h;
    senthmm_t *senthmm;
    snode_t *s, *p;
    
    if (senthmm_head)
	senthmm_destroy ();
    
    senthmm_head = senthmm_tail = NULL;
    
    for (i = 0; i < nhyp; i++) {
	for (h = hyplist[i]; h; h = h->next) {
	    h->wid = dict_wordid (h->word);
	    if (NOT_WID(h->wid))
		E_FATAL("%s: Unknown word: %s\n", uttid, h->word);
	}

	if ((senthmm = senthmm_build (hyplist[i])) == NULL) {
	    E_ERROR("%s: Cannot build sentence HMM #%d\n", uttid, i);
	    return -1;
	}
	
	if (! senthmm_head)
	    senthmm_head = senthmm;
	else
	    senthmm_tail->next = senthmm;
	senthmm_tail = senthmm;
    }

    n_frm = 0;

    /* Set all state score to LOGPROB_ZERO, and initial state in each hyp to active */
    for (senthmm = senthmm_head; senthmm; senthmm = senthmm->next) {
	for (s = senthmm->snodelist; s; s = s->next) {
	    s->score = LOGPROB_ZERO;
	    if (s->next)
		s->active = -1;
	    else {
		s->active = n_frm;
		senthmm->first_active = senthmm->last_active = s;
	    }
	}
	senthmm->entry->score = 0;
	senthmm->entry->active = n_frm;
    }
    
    return 0;
}


int32 alpha_sen_active (s3senid_t *sen_active, int32 nsen)
{
    senthmm_t *sent;
    snode_t *s;
    
    memset (sen_active, 0, nsen * sizeof(s3senid_t));
    
    for (sent = senthmm_head; sent; sent = sent->next) {
	for (s = sent->first_active; s; s = s->next) {
	    if (s->active == n_frm)
		sen_active[s->senid] = 1;
	    if (s == sent->last_active)
		break;
	}
    }

    return 0;
}


int32 alpha_frame (char *uttid, int32 *senscr)
{
    senthmm_t *sent;
    snode_t *s, *p;
    slink_t *l;
    int32 score, best, th;
    int32 i;
    snode_t *first, *last;
    
    /* Update state scores for each senthmm */
    for (i = 0, sent = senthmm_head; sent; sent = sent->next, i++) {
	best = (int32) 0x80000000;
	
	for (s = sent->first_active; s; s = s->next) {
	    if (s->active != n_frm)
		continue;
	    
	    /* Sum of (predecessor state score * tp) (in logspace) */
	    score = LOGPROB_ZERO;
	    for (l = s->predlist; l; l = l->next) {
		p = l->node;
		if ((p->active == n_frm) && (p->score > LOGPROB_ZERO))
		    score = logs3_add (l->tp + p->score, score);
	    }
	    
	    /* Add in current senone score */
	    s->score = score + senscr[s->senid];
	    
	    if (best < s->score)
		best = s->score;
	    
	    if (s == sent->last_active)
		break;
	}

	if (best < LOGPROB_ZERO)
	    E_FATAL("%s: Best score (hyp %d) < LOGPROB_ZERO: %d\n", uttid, i, best);

	/* Prune and activate/deactivate states; mark new first and last active states */
	th = best + beam;
	first = NULL;
	last = NULL;
	for (s = sent->snodelist; s; s = s->next) {	/* Note: NOT first_active */
	    if ((s->active == n_frm) && (s->score >= th)) {
		s->active = n_frm+1;
		if (! first)
		    first = s;
		last = s;
	    } else {
		for (l = s->predlist; l; l = l->next) {
		    p = l->node;
		    if ((p->active == n_frm) && (p->score >= th)) {
			s->active = n_frm+1;
			if (! first)
			    first = s;
			last = s;

			break;
		    }
		}

		if (! l)
		    s->score = LOGPROB_ZERO;
	    }

	    if (s == sent->last_active)	/* Nothing earlier can become active */
		break;
	}

	sent->first_active = first;
	sent->last_active = last;
    }
    
    n_frm++;

    return 0;
}


int32 alpha_end_utt (char *outdir, char *uttid)
{
    senthmm_t *sent;
    slink_t *l;
    snode_t *p;
    int32 i, score;
    FILE *fp;
    char file[1024];
    
    sprintf (file, "%s/%s.alpha", outdir, uttid);
    if ((fp = fopen(file, "w")) == NULL) {
	E_ERROR("fopen(%s,w) failed\n");
	return -1;
    }
    
    fprintf (fp, "# %s\n", uttid);
    fprintf (fp, "# %e\n", *((float32 *) cmd_ln_access ("-logbase")));
    
    for (i = 0, sent = senthmm_head; sent; sent = sent->next, i++) {
	score = LOGPROB_ZERO;

	/* Update exit state score for each sent */
	for (l = sent->exit->predlist; l; l = l->next) {
	    p = l->node;
	    if ((p->active == n_frm) && (p->score > LOGPROB_ZERO))
		score = logs3_add (p->score + l->tp, score);
	}
	
	sent->exit->score = score;
	fprintf (fp, "%11d\n", score);
    }

    fclose (fp);

    return 0;
}


int32 alpha_init ( void )
{
    float64 *f64arg;

    mdef = mdef_getmdef ();
    dict = dict_getdict ();
    tmat = tmat_gettmat ();

    if (tmat_chk_uppertri (tmat) < 0)
	E_FATAL("tmat must be upper triangular\n");
    
    silphone = mdef_ciphone_id (mdef, SILENCE_CIPHONE);
    
    f64arg = (float64 *) cmd_ln_access ("-beam");
    beam = logs3 (*f64arg);
    E_INFO ("logs3(beam)= %d\n", beam);

    return 0;
}
