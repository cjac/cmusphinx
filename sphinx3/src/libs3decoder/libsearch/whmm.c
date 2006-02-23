/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * whmm.c -- hmm structure that is used by sphinx 3.0 decode_anytopo (and perhaps
 * the fsg search as well)
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 14-Jul-05    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity 
 *              First created it. 
 *
 * $Log$
 * Revision 1.2  2006/02/23  05:07:53  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: split whmm's routine.
 * 
 * Revision 1.1.2.4  2005/09/07 23:40:06  arthchan2003
 * Several Bug Fixes and Enhancements to the flat-lexicon
 * 1, Fixed Dox-doc.
 * 2, Add -worddumpef and -hmmdumpef in parrallel to -worddumpsf and
 * -hmmdumpsf. Usage is trivial. a structure called fwd_dbg_t now wrapped
 * up all these loose parameters.  Methods of fwd_dbg are implemented.
 * 3, word_ugprob is now initialized by init_word_ugprob
 * 4, Full-triphone expansion is implemented. User can change this
 * behavior by specifying -multiplex_multi and -multiplex_single. The
 * former turn on multiplex triphone for word-begin for multi-phone word.
 * The latter do that for single-phone word. Turning off both could
 * tremendously increase computation.
 * 5, Word expansions of possible right contexts now records independent
 * history.  The behavior in the past was to use only one history for a
 * word.
 *
 * Revision 1.1.2.3  2005/07/24 01:42:58  arthchan2003
 * Added whmm_alloc_light, that will by-pass and not use any internal list inside whmm.c
 *
 * Revision 1.1.2.2  2005/07/17 05:57:25  arthchan2003
 * 1, Removed wid from the argument list of eval_*_whmm, 2, Allow  allocation of whmm_alloc to be more flexible.
 *
 * Revision 1.1.2.1  2005/07/15 07:48:32  arthchan2003
 * split the hmm (whmm_t) and context building process (ctxt_table_t) from the the flat_fwd.c
 *
 *
 */

#include <whmm.h>
/**
 * There are two sets of whmm freelists.  whmm_freelist[0] for word-initial HMMs
 * that need a separate HMM id every state, and whmm_freelist[1] for non-word-initial
 * HMMs that don't need that.
 */
static whmm_t *whmm_freelist[2] = {NULL, NULL};
/** For partial evaluation of incoming state score (prev state score + senone score) */
int32 *st_sen_scr;

void whmm_free (whmm_t *h)
{
    int32 k;
    
    /*    k = (h->pos == 0 && multiplex) ? 0 : 1;*/
    k = h->type;
    h->next = whmm_freelist[k];
    whmm_freelist[k] = h;
}


whmm_t *whmm_alloc_light (int32 n_state)
{
  whmm_t *h;
  h=(whmm_t*) ckd_calloc(1,sizeof(whmm_t));
  
  h->history=(s3latid_t *)ckd_calloc(n_state,sizeof(s3latid_t));
  h->score=(int32* )ckd_calloc(n_state,sizeof(int32));
  return h;
}


whmm_t *whmm_alloc (int32 pos, int32 n_state, int32 alloc_size, int32 multiplex)
{
    whmm_t *h;
    int32 k, i, n, s;
    int32 *tmp_scr;
    s3latid_t *tmp_latid;
    s3pid_t *tmp_pid;
    tmp_pid=NULL;
    
    k = (IS_MULTIPLEX(pos,multiplex)) ? MULTIPLEX_TYPE : NONMULTIPLEX_TYPE;
    
    if (! whmm_freelist[k]) {
#if 0
	n = 16000/sizeof(whmm_t);	/* HACK!!  Hardwired allocation size */
#endif
	n = alloc_size/sizeof(whmm_t);	

	whmm_freelist[k] = h = (whmm_t *) ckd_calloc (n, sizeof(whmm_t));
	h->next=NULL;
	tmp_scr = (int32 *) ckd_calloc (n_state * n, sizeof(int32));
	tmp_latid = (s3latid_t *) ckd_calloc (n_state * n, sizeof(s3latid_t));

	if (IS_MULTIPLEX(pos,multiplex))
	  tmp_pid = (s3pid_t *) ckd_calloc (n_state * n, sizeof(s3pid_t));

	for (i = 0; i < n; i++) {
	    h[i].next = &(h[i+1]);

	    h[i].score = tmp_scr;
	    tmp_scr += n_state;
	    
	    h[i].history = tmp_latid;
	    tmp_latid += n_state;

	    /* Allocate pid iff first phone position (for multiplexed left contexts) */
	    if (IS_MULTIPLEX(pos,multiplex)) {
		h[i].pid = tmp_pid;
		tmp_pid += n_state;
	    }
	}
	h[n-1].next = NULL;
    }
    
    h = whmm_freelist[k];
    whmm_freelist[k] = h->next;
    
    for (s = 0; s < n_state; s++) {
	h->score[s] = S3_LOGPROB_ZERO;
	h->history[s] = BAD_S3LATID;
    }
    h->pos = pos;
    h->type = k;

    assert((h->type==MULTIPLEX_TYPE)==IS_MULTIPLEX(h->pos,multiplex));
    return (h);
}

void dump_whmm (s3wid_t w, whmm_t *h, int32 *senscr, tmat_t *tmat, int32 n_frame, int32 n_state,dict_t *dict, mdef_t *mdef)
{
    int32 s;
    s3pid_t p;
    
    printf ("[%4d]", n_frame);
    printf (" [%s]", dict->word[w].word);

    printf (" pos= %d, lc=%d, rc= %d, bestscore= %d multiplex %s\n",
	    h->pos, h->lc, h->rc, h->bestscore, h->type==MULTIPLEX_TYPE ? "yes" : "no");
    
    printf ("\tscore: ");
    for (s = 0; s < n_state; s++)
	printf (" %12d", h->score[s]);
    printf ("\n");
    
    printf ("\thist:  ");
    for (s = 0; s < n_state; s++)
	printf (" %12d", h->history[s]);
    printf ("\n");
    fflush(stdout);

    if(senscr){
      printf ("\tsenscr:");
      for (s = 0; s < n_state-1; s++) {
	p = (h->type==MULTIPLEX_TYPE) ? h->pid[s] : *(h->pid) ;
	if (NOT_S3PID(p))
	  printf (" %12s", "--");
	else
	  printf (" %12d", senscr[mdef->phone[p].state[s]]);
      }
      printf ("\n");
    }
    
    printf ("\ttpself:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->type==MULTIPLEX_TYPE) ? h->pid[s] : *(h->pid)  ;
	if (NOT_S3PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s]);
    }
    printf ("\n");
    
    printf ("\ttpnext:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->type==MULTIPLEX_TYPE) ? h->pid[s] : *(h->pid) ;
	if (NOT_S3PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s+1]);
    }
    printf ("\n");
    
    printf ("\ttpskip:");
    for (s = 0; s < n_state-2; s++) {
      p = (h->type==MULTIPLEX_TYPE) ? h->pid[s] : *(h->pid);
	if (NOT_S3PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s+2]);
    }
    printf ("\n");
    
    
    if (h->type==MULTIPLEX_TYPE) {
	printf ("\tpid:   ");
	for (s = 0; s < n_state-1; s++)
	    printf (" %12d", h->pid[s]);
	printf ("\n");
    }
}



#define ANYHMMTOPO 1
#if (! ANYHMMTOPO)
/**
 * Like the general eval_nonmpx_whmm and eval_mpx_whmm below, but hardwired for
 * the Sphinx-II 5-state Bakis topology.
 */
void eval_nonmpx_whmm (whmm_t *h, int32 *senscr, tmat_t *tmat, mdef_t *mdef, int32 n_state)
{
    register int32 s0, s1, s2, s3, s4;
    register int32 scr, newscr1, newscr2, bestscr;
    register int32 *tp;
    s3pid_t p;
    s3senid_t *senp;

    p = *(h->pid);
    senp = mdef->phone[p].state;
    tp = tmat->tp[mdef->phone[p].tmat][0];	/* HACK!! Assumes tp 2-D data allocated
						   contiguously */
    
    if ((s0 = h->score[0] + senscr[senp[0]]) < S3_LOGPROB_ZERO)
	s0 = S3_LOGPROB_ZERO;
    if ((s1 = h->score[1] + senscr[senp[1]]) < S3_LOGPROB_ZERO)
	s1 = S3_LOGPROB_ZERO;
    if ((s2 = h->score[2] + senscr[senp[2]]) < S3_LOGPROB_ZERO)
	s2 = S3_LOGPROB_ZERO;
    if ((s3 = h->score[3] + senscr[senp[3]]) < S3_LOGPROB_ZERO)
	s3 = S3_LOGPROB_ZERO;
    if ((s4 = h->score[4] + senscr[senp[4]]) < S3_LOGPROB_ZERO)
	s4 = S3_LOGPROB_ZERO;
    
    newscr1 = s4 + tp[29]; /* [4][5] */
    newscr2 = s3 + tp[23]; /* [3][5] */
    if (newscr1 > newscr2) {
	h->score[5] = bestscr = newscr1;
	h->history[5] = h->history[4];
    } else {
	h->score[5] = bestscr = newscr2;
	h->history[5] = h->history[3];
    }

    scr     = s4 + tp[28]; /* [4][4] */
    newscr1 = s3 + tp[22]; /* [3][4] */
    newscr2 = s2 + tp[16]; /* [2][4] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[4] = newscr2;
	    h->history[4] = h->history[2];
	} else
	    h->score[4] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[4] = newscr1;
	    h->history[4] = h->history[3];
	} else
	    h->score[4] = scr;
    }
    if (bestscr < h->score[4])
	bestscr = h->score[4];
    
    scr     = s3 + tp[21]; /* [3][3] */
    newscr1 = s2 + tp[15]; /* [2][3] */
    newscr2 = s1 + tp[9];  /* [1][3] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[3] = newscr2;
	    h->history[3] = h->history[1];
	} else
	    h->score[3] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[3] = newscr1;
	    h->history[3] = h->history[2];
	} else
	    h->score[3] = scr;
    }
    if (bestscr < h->score[3])
	bestscr = h->score[3];
    
    scr     = s2 + tp[14]; /* [2][2] */
    newscr1 = s1 + tp[8];  /* [1][2] */
    newscr2 = s0 + tp[2];  /* [0][2] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[2] = newscr2;
	    h->history[2] = h->history[0];
	} else
	    h->score[2] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[2] = newscr1;
	    h->history[2] = h->history[1];
	} else
	    h->score[2] = scr;
    }
    if (bestscr < h->score[2])
	bestscr = h->score[2];
    
    scr     = s1 + tp[7]; /* [1][1] */
    newscr1 = s0 + tp[1]; /* [0][1] */
    if (newscr1 > scr) {
	h->score[1] = newscr1;
	h->history[1] = h->history[0];
    } else
	h->score[1] = scr;
    if (bestscr < h->score[1])
	bestscr = h->score[1];
    
    h->score[0] = scr = s0 + tp[0];	/* [0][0] */
    if (bestscr < scr)
	bestscr = scr;
    
    h->bestscore = bestscr;
}


void eval_mpx_whmm (s3wid_t w, whmm_t *h, int32 *senscr,tmat_t *tmat, mdef_t *mdef, int32 n_state)
{
    register int32 s0, s1, s2, s3, s4;
    register int32 *tp0, *tp1, *tp2, *tp3, *tp4;
    register int32 scr, newscr1, newscr2, bestscr;
    s3senid_t *senp;
    s3pid_t p0, p1, p2, p3, p4;

    p0 = h->pid[0];
    p1 = h->pid[1];
    p2 = h->pid[2];
    p3 = h->pid[3];
    p4 = h->pid[4];

    senp = mdef->phone[p0].state;
    if ((s0 = h->score[0] + senscr[senp[0]]) < S3_LOGPROB_ZERO)
	s0 = S3_LOGPROB_ZERO;
    tp0 = tmat->tp[mdef->phone[p0].tmat][0];	/* HACK!! See eval_nonmpx_whmm */

    if (p1 != p0) {
	senp = mdef->phone[p1].state;
	tp1 = tmat->tp[mdef->phone[p1].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp1 = tp0;
    if ((s1 = h->score[1] + senscr[senp[1]]) < S3_LOGPROB_ZERO)
	s1 = S3_LOGPROB_ZERO;

    if (p2 != p1) {
	senp = mdef->phone[p2].state;
	tp2 = tmat->tp[mdef->phone[p2].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp2 = tp1;
    if ((s2 = h->score[2] + senscr[senp[2]]) < S3_LOGPROB_ZERO)
	s2 = S3_LOGPROB_ZERO;

    if (p3 != p2) {
	senp = mdef->phone[p3].state;
	tp3 = tmat->tp[mdef->phone[p3].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp3 = tp2;
    if ((s3 = h->score[3] + senscr[senp[3]]) < S3_LOGPROB_ZERO)
	s3 = S3_LOGPROB_ZERO;

    if (p4 != p3) {
	senp = mdef->phone[p4].state;
	tp4 = tmat->tp[mdef->phone[p4].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp4 = tp3;
    if ((s4 = h->score[4] + senscr[senp[4]]) < S3_LOGPROB_ZERO)
	s4 = S3_LOGPROB_ZERO;
    
    newscr1 = s4 + tp4[29]; /* [4][5] */
    newscr2 = s3 + tp3[23]; /* [3][5] */
    if (newscr1 > newscr2) {
	h->score[5] = bestscr = newscr1;
	h->history[5] = h->history[4];
    } else {
	h->score[5] = bestscr = newscr2;
	h->history[5] = h->history[3];
    }

    scr     = s4 + tp4[28]; /* [4][4] */
    newscr1 = s3 + tp3[22]; /* [3][4] */
    newscr2 = s2 + tp2[16]; /* [2][4] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[4] = newscr2;
	    h->history[4] = h->history[2];
	    h->pid[4] = h->pid[2];
	} else
	    h->score[4] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[4] = newscr1;
	    h->history[4] = h->history[3];
	    h->pid[4] = h->pid[3];
	} else
	    h->score[4] = scr;
    }
    if (bestscr < h->score[4])
	bestscr = h->score[4];
    
    scr     = s3 + tp3[21]; /* [3][3] */
    newscr1 = s2 + tp2[15]; /* [2][3] */
    newscr2 = s1 + tp1[9];  /* [1][3] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[3] = newscr2;
	    h->history[3] = h->history[1];
	    h->pid[3] = h->pid[1];
	} else
	    h->score[3] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[3] = newscr1;
	    h->history[3] = h->history[2];
	    h->pid[3] = h->pid[2];
	} else
	    h->score[3] = scr;
    }
    if (bestscr < h->score[3])
	bestscr = h->score[3];
    
    scr     = s2 + tp2[14]; /* [2][2] */
    newscr1 = s1 + tp1[8];  /* [1][2] */
    newscr2 = s0 + tp0[2];  /* [0][2] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[2] = newscr2;
	    h->history[2] = h->history[0];
	    h->pid[2] = h->pid[0];
	} else
	    h->score[2] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[2] = newscr1;
	    h->history[2] = h->history[1];
	    h->pid[2] = h->pid[1];
	} else
	    h->score[2] = scr;
    }
    if (bestscr < h->score[2])
	bestscr = h->score[2];
    
    scr     = s1 + tp1[7]; /* [1][1] */
    newscr1 = s0 + tp0[1]; /* [0][1] */
    if (newscr1 > scr) {
	h->score[1] = newscr1;
	h->history[1] = h->history[0];
	h->pid[1] = h->pid[0];
    } else
	h->score[1] = scr;
    if (bestscr < h->score[1])
	bestscr = h->score[1];
    
    h->score[0] = scr = s0 + tp0[0]; /* [0][0] */
    if (bestscr < scr)
	bestscr = scr;
    
    h->bestscore = bestscr;
}

#else

/**
 * Evaluate non-multiplexed word HMM (ie, the entire whmm really represents one
 * phone rather than each state representing a potentially different phone.
 */
void eval_nonmpx_whmm (whmm_t *h, int32 *senscr,tmat_t *tmat, mdef_t *mdef, int32 n_state)
{
    s3pid_t pid;
    s3senid_t *sen;
    int32 **tp;
    int32 to, from, bestfrom;
    int32 newscr, scr, bestscr;
    int32 final_state;


    final_state = n_state - 1;

    pid = *(h->pid);
    
    sen = mdef->phone[pid].state; /*This gives a state-to-senone mapping*/
    tp = tmat->tp[mdef->phone[pid].tmat];

    /* Compute previous state-score + observation output prob for each state */
    for (from = n_state-2; from >= 0; --from) {
	if ((st_sen_scr[from] = h->score[from] + senscr[sen[from]]) < S3_LOGPROB_ZERO)
	    st_sen_scr[from] = S3_LOGPROB_ZERO;
    }
    
    /* Evaluate final-state first, which does not have a self-transition */
    to = final_state;
    scr = S3_LOGPROB_ZERO;
    bestfrom = -1;
    for (from = to-1; from >= 0; --from) {
	if ((tp[from][to] > S3_LOGPROB_ZERO) &&
	    ((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
	    scr = newscr;
	    bestfrom = from;
	}
    }
    h->score[to] = scr;
    if (bestfrom >= 0)
	h->history[to] = h->history[bestfrom];

    bestscr = scr;

    /* Evaluate all other states, which might have self-transitions */
    for (to = final_state-1; to >= 0; --to) {
	/* Score from self-transition, if any */
	scr = (tp[to][to] > S3_LOGPROB_ZERO) ? st_sen_scr[to] + tp[to][to] : S3_LOGPROB_ZERO;

	/* Scores from transitions from other states */
	bestfrom = -1;
	for (from = to-1; from >= 0; --from) {
	    if ((tp[from][to] > S3_LOGPROB_ZERO) &&
		((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
		scr = newscr;
		bestfrom = from;
	    }
	}

	/* Update new result for state to */
	h->score[to] = scr;
	if (bestfrom >= 0)
	    h->history[to] = h->history[bestfrom];

	if (bestscr < scr)
	    bestscr = scr;
    }

    h->bestscore = bestscr;
}


/** Like eval_nonmpx_whmm, except there's a different pid associated with each state */
void eval_mpx_whmm (whmm_t *h, int32 *senscr,tmat_t *tmat, mdef_t *mdef,int32 n_state)
{
    s3pid_t pid, prevpid;
    s3senid_t *senp;
    int32 **tp;
    int32 to, from, bestfrom;
    int32 newscr, scr, bestscr;
    int32 final_state;

    final_state = n_state - 1;
    
    senp=NULL;
    tp=NULL;
    /* Compute previous state-score + observation output prob for each state */
    prevpid = BAD_S3PID;
    for (from = n_state-2; from >= 0; --from) {
	if ((pid = h->pid[from]) != prevpid) {
	    senp = mdef->phone[pid].state; /*This gives a state-to-senone mapping*/
	    prevpid = pid;
	}

	if ((st_sen_scr[from] = h->score[from] + senscr[senp[from]]) < S3_LOGPROB_ZERO)
	    st_sen_scr[from] = S3_LOGPROB_ZERO;
    }

    /* Evaluate final-state first, which does not have a self-transition */
    to = final_state;
    scr = S3_LOGPROB_ZERO;
    bestfrom = -1;
    prevpid = BAD_S3PID;
    for (from = to-1; from >= 0; --from) {
	if ((pid = h->pid[from]) != prevpid) {
	    tp = tmat->tp[mdef->phone[pid].tmat];
	    prevpid = pid;
	}

	if ((tp[from][to] > S3_LOGPROB_ZERO) &&
	    ((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
	    scr = newscr;
	    bestfrom = from;
	}
    }
    h->score[to] = scr;
    if (bestfrom >= 0) {
	h->history[to] = h->history[bestfrom];
	h->pid[to] = h->pid[bestfrom];
    }
    
    bestscr = scr;

    /* Evaluate all other states, which might have self-transitions */
    for (to = final_state-1; to >= 0; --to) {
	/* Score from self-transition, if any */
	if ((pid = h->pid[to]) != prevpid) {
	    tp = tmat->tp[mdef->phone[pid].tmat];
	    prevpid = pid;
	}
	scr = (tp[to][to] > S3_LOGPROB_ZERO) ? st_sen_scr[to] + tp[to][to] : S3_LOGPROB_ZERO;

	/* Scores from transitions from other states */
	bestfrom = -1;
	for (from = to-1; from >= 0; --from) {
	    if ((pid = h->pid[from]) != prevpid) {
		tp = tmat->tp[mdef->phone[pid].tmat];
		prevpid = pid;
	    }
	    
	    if ((tp[from][to] > S3_LOGPROB_ZERO) &&
		((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
		scr = newscr;
		bestfrom = from;
	    }
	}

	/* Update new result for state to */
	h->score[to] = scr;
	if (bestfrom >= 0) {
	    h->history[to] = h->history[bestfrom];
	    h->pid[to] = h->pid[bestfrom];
	}
	
	if (bestscr < scr)
	    bestscr = scr;
    }

    h->bestscore = bestscr;
}
#endif
