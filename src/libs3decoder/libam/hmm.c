/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * hmm.c -- HMM Viterbi search.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Modified hmm_vit_eval_3st() to include explicit checks for
 *		tr[0][2] and tr[1][3]. Included compiler directive activated
 *		checks for int32 underflow
 *
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified hmm_t.state to be a run-time array instead of a compile-time
 * 		one.  Modified compile-time 3 and 5-state versions of hmm_vit_eval
 * 		into hmm_vit_eval_3st and hmm_vit_eval_5st, to allow run-time selection.
 * 		Removed hmm_init().
 * 
 * 11-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Bugfix in computing HMM exit state score.
 * 
 * 08-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added HMM_SKIPARCS compile-time option and hmm_init().
 * 
 * 20-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Bugfix in hmm_eval: If state1->state2 transition took place,
 * 		state1 history didn't get propagated to state2.
 * 		Also, included tp[][] in HMM evaluation.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started, based on an earlier version.
 */


#include "hmm.h"

static int32 NO_UFLOW_ADD(int32 a, int32 b);

void hmm_dump (hmm_t *hmm, int32 n_state, s3senid_t *senid, int32 *senscr, FILE *fp)
{
    int32 i;
    
    fprintf (fp, " %11d    ", hmm->in.score);
    for (i = 0; i < n_state; i++)
	fprintf (fp, " %11d", hmm->state[i].score);
    fprintf (fp, "     %11d\n", hmm->out.score);
    
    fprintf (fp, " %11d    ", hmm->in.history);
    for (i = 0; i < n_state; i++)
	fprintf (fp, " %11d", hmm->state[i].history);
    fprintf (fp, "     %11d\n", hmm->out.history);
    
    if (senid) {
	fprintf (fp, " %-11s    ", "senid");
	for (i = 0; i < n_state; i++)
	    fprintf (fp, " %11d", senid[i]);
	fprintf (fp, "\n");
	
	if (senscr) {
	    fprintf (fp, " %-11s    ", "senscr");
	    for (i = 0; i < n_state; i++)
		fprintf (fp, " %11d", senscr[senid[i]]);
	    fprintf (fp, "\n");
	}
    }
    
    fflush (fp);
}


void hmm_clear (hmm_t *h, int32 n_state)
{
    int32 i;

    h->in.score = S3_LOGPROB_ZERO;
    h->in.history = -1;
    for (i = 0; i < n_state; i++) {
	h->state[i].score = S3_LOGPROB_ZERO;
	h->state[i].history = -1;
    }
    h->out.score = S3_LOGPROB_ZERO;
    h->out.history = -1;
    
    h->bestscore = S3_LOGPROB_ZERO;
}

/* Added by ARCHAN at 20040127 , always check for underflow, it's proved to be crucial */

#define _CHECKUNDERFLOW_ 1



static int32 NO_UFLOW_ADD(int32 a, int32 b)
{
  int c;
#ifdef _CHECKUNDERFLOW_
  c= a + b;
  c= (c>0 && a <0 && b <0) ? MAX_NEG_INT32 : c;
#else
  c= a + b;
#endif 
  return c;

}


int32 hmm_vit_eval_5st (hmm_t *hmm, s3senid_t *senid, int32 *senscr)
{
    int32 s0, s1, s2, s3, s4, best, *tp;
    
    tp = hmm->tp[0];	/* Hack!!, use the knowledge that the 2-D tp is a contiguous block */
    
    /* 4 = max(2,3,4); */
    /*    s4 = hmm->state[4].score + tp[28];
    s3 = hmm->state[3].score + tp[22];
    s2 = hmm->state[2].score + tp[16];*/

    s4 = NO_UFLOW_ADD(hmm->state[4].score,tp[28]);
    s3 = NO_UFLOW_ADD(hmm->state[3].score,tp[22]);
    s2 = NO_UFLOW_ADD(hmm->state[2].score,tp[16]);

    if (s4 < s3) {
	if (s3 >= s2) {
	    s4 = s3;
	    hmm->state[4].history = hmm->state[3].history;
	} else {
	    s4 = s2;
	    hmm->state[4].history = hmm->state[2].history;
	}
    } else if (s4 < s2) {
	s4 = s2;
	hmm->state[4].history = hmm->state[2].history;
    }
    /*s4 += senscr[senid[4]];*/
    s4 = NO_UFLOW_ADD(s4,senscr[senid[4]]);
    hmm->state[4].score = s4;
    
    /* 3 = max(1,2,3); */
    /*s3 = hmm->state[3].score + tp[21];
    s2 = hmm->state[2].score + tp[15];
    s1 = hmm->state[1].score + tp[ 9];*/
    s3 = NO_UFLOW_ADD(hmm->state[3].score,tp[21]);
    s2 = NO_UFLOW_ADD(hmm->state[2].score, tp[15]);
    s1 = NO_UFLOW_ADD(hmm->state[1].score,tp[ 9]);
    if (s3 < s2) {
	if (s2 >= s1) {
	    s3 = s2;
	    hmm->state[3].history = hmm->state[2].history;
	} else {
	    s3 = s1;
	    hmm->state[3].history = hmm->state[1].history;
	}
    } else if (s3 < s1) {
	s3 = s1;
	hmm->state[3].history = hmm->state[1].history;
    }
    /*s3 += senscr[senid[3]];*/

    s3=NO_UFLOW_ADD(s3,senscr[senid[3]]);
    hmm->state[3].score = s3;
    
    best = (s4 > s3) ? s4 : s3;
    
    /* Exit state score */
    /*s4 += tp[29];
      s3 += tp[23];*/

    s4 = NO_UFLOW_ADD(s4,tp[29]);
    s3 = NO_UFLOW_ADD(s3,tp[23]);

    if (s4 < s3) {
	hmm->out.score = s3;
	hmm->out.history = hmm->state[3].history;
    } else {
	hmm->out.score = s4;
	hmm->out.history = hmm->state[4].history;
    }
    
    /* 2 = max(0,1,2); */
    /* s2 = hmm->state[2].score + tp[14];
    s1 = hmm->state[1].score + tp[ 8];
    s0 = hmm->state[0].score + tp[ 2];*/
    s2 = NO_UFLOW_ADD(hmm->state[2].score,tp[14]);
    s1 = NO_UFLOW_ADD(hmm->state[1].score,tp[ 8]);
    s0 = NO_UFLOW_ADD(hmm->state[0].score,tp[ 2]);
    if (s2 < s1) {
	if (s1 >= s0) {
	    s2 = s1;
	    hmm->state[2].history = hmm->state[1].history;
	} else {
	    s2 = s0;
	    hmm->state[2].history = hmm->state[0].history;
	}
    } else if (s2 < s0) {
	s2 = s0;
	hmm->state[2].history = hmm->state[0].history;
    }
    /*s2 += senscr[senid[2]];*/

    s2=NO_UFLOW_ADD(s2,senscr[senid[2]]);

    hmm->state[2].score = s2;
    if (best < s2)
	best = s2;
    
    /* 1 = max(0,1); */
    /*s1 = hmm->state[1].score + tp[ 7];
      s0 = hmm->state[0].score + tp[ 1];*/
    s1 = NO_UFLOW_ADD(hmm->state[1].score,tp[ 7]);
    s0 = NO_UFLOW_ADD(hmm->state[0].score,tp[ 1]);

    if (s1 < s0) {
	s1 = s0;
	hmm->state[1].history = hmm->state[0].history;
    }
    /*s1 += senscr[senid[1]];*/
    s1 = NO_UFLOW_ADD(s1,senscr[senid[1]]);
    hmm->state[1].score = s1;
    if (best < s1)
	best = s1;
    
    /* 0 = max(0,in); */
    /*s0 = hmm->state[0].score + tp[ 0];*/
    s0 = NO_UFLOW_ADD(hmm->state[0].score,tp[ 0]);
    
    if (s0 < hmm->in.score) {
	s0 = hmm->in.score;
	hmm->state[0].history = hmm->in.history;
    }
    /*s0 += senscr[senid[0]];*/
    s0 = NO_UFLOW_ADD(s0,senscr[senid[0]]);

    hmm->state[0].score = s0;
    if (best < s0)
	best = s0;
    
    hmm->in.score = S3_LOGPROB_ZERO;	/* Consumed */
    hmm->bestscore = best;
    
    return best;
}


int32 hmm_vit_eval_3st (hmm_t *hmm, s3senid_t *senid, int32 *senscr)
{
    int32 s0, s1, s2, best, *tp;

#ifdef _CHECKUNDERFLOW_
    /*    int32 st0, st1, st2, sen0, sen1, sen2;*/
#endif
    
    tp = hmm->tp[0];	/* Hack!! We know the 2-D tp is a contiguous block */
    
    /* 2 = max(0,1,2); */
    /*s2 = hmm->state[2].score + tp[10];*/
    s2 = NO_UFLOW_ADD(hmm->state[2].score,tp[10]);

    /*s1 = hmm->state[1].score + tp[ 6];*/
    s1 = NO_UFLOW_ADD(hmm->state[1].score,tp[6]);

    if (tp[2] > S3_LOGPROB_ZERO) { /* Only if skip(0->2) is allowed */
      /*s0 = hmm->state[0].score + tp[ 2]; */
        s0 = NO_UFLOW_ADD(hmm->state[0].score,tp[ 2]);

        if (s2 < s1) {
	    if (s1 >= s0) {
	        s2 = s1;
	        hmm->state[2].history = hmm->state[1].history;
	    } else {
	        s2 = s0;
	        hmm->state[2].history = hmm->state[0].history;
	    }
        } else if (s2 < s0) {
	    s2 = s0;
	    hmm->state[2].history = hmm->state[0].history;
        }
    } else {
        if (s2 < s1) {
	    s2 = s1;
	    hmm->state[2].history = hmm->state[1].history;
        } 
    } 

/* #ifdef _CHECKUNDERFLOW_ */
/*     sen2 = senscr[senid[2]]; */
/*     st2 = s2 + sen2; */
/*     s2 = (st2 > 0 && s2 < 0 && sen2 < 0) ? MAX_NEG_INT32 : st2; */
/* #else */
/*     s2 += senscr[senid[2]]; */
/* #endif  */
    s2= NO_UFLOW_ADD(s2,senscr[senid[2]]);

    hmm->state[2].score = s2;

    /* 1 = max(0,1); */
    /*    s1 = hmm->state[1].score + tp[ 5];*/
    /*s0 = hmm->state[0].score + tp[ 1];*/
    s1= NO_UFLOW_ADD(hmm->state[1].score,tp[5]);
    s0 = NO_UFLOW_ADD(hmm->state[0].score,tp[ 1]);
    
    if (s1 < s0) {
	s1 = s0;
	hmm->state[1].history = hmm->state[0].history;
    }

/* #ifdef _CHECKUNDERFLOW_ */
/*     sen1 = senscr[senid[1]]; */
/*     st1 = s1 + sen1; */
/*     s1 = (st1 > 0 && s1 < 0 && sen1 < 0) ? MAX_NEG_INT32 : st1; */
/* #else */
/*     s1 += senscr[senid[1]]; */
/* #endif  */
    s1 = NO_UFLOW_ADD(s1,senscr[senid[1]]);
    hmm->state[1].score = s1;
 
    best = (s2 > s1) ? s2 : s1;

    /* Exit state score */

/* #ifdef _CHECKUNDERFLOW_ */
/*     st2 = s2 + tp[11]; */
/*     s2 = (st2 > 0 && s2 < 0) ? MAX_NEG_INT32 : st2; */
/* #else */
/*     s2 += tp[11]; */
/* #endif */

    s2 = NO_UFLOW_ADD(s2,tp[11]);

    if (tp[7] > S3_LOGPROB_ZERO) { /* Only if skip(1->4) is allowed */
      /*  s1 += tp[ 7];*/
      s1 = NO_UFLOW_ADD(s1,tp[7]);

        if (s2 < s1) {
	    hmm->out.score = s1;
	    hmm->out.history = hmm->state[1].history;
        } else {
	    hmm->out.score = s2;
	    hmm->out.history = hmm->state[2].history;
        }
    } else {
        hmm->out.score = s2;
        hmm->out.history = hmm->state[2].history;
    }
    /* 0 = max(0,in); */
    /*s0 = hmm->state[0].score + tp[ 0];*/

    s0 = NO_UFLOW_ADD(hmm->state[0].score,tp[0]);

    if (s0 < hmm->in.score) {
	s0 = hmm->in.score;
	hmm->state[0].history = hmm->in.history;
    }

/* #ifdef _CHECKUNDERFLOW_ */
/*     sen0 = senscr[senid[0]]; */
/*     st0 = s0 + sen0; */
/*     s0 = (st0 > 0 && s0 < 0 && sen0 < 0) ? MAX_NEG_INT32 : st0; */
/* #else */
/*     s0 += senscr[senid[0]]; */
/* #endif  */
    s0 = NO_UFLOW_ADD(s0,senscr[senid[0]]);

    hmm->state[0].score = s0;

    if (best < s0)
	best = s0;

    hmm->in.score = S3_LOGPROB_ZERO;	/* Consumed */
    hmm->bestscore = best;

    return best;
}


int32 hmm_dump_vit_eval (hmm_t *hmm, int32 n_state, s3senid_t *senid, int32 *senscr, FILE *fp)
{
    int32 bs=0;
    
    if (fp)
	hmm_dump (hmm, n_state, senid, senscr, fp);
    
    if (n_state == 5)
	bs = hmm_vit_eval_5st (hmm, senid, senscr);
    else if (n_state == 3)
	bs = hmm_vit_eval_3st (hmm, senid, senscr);
    else
	E_FATAL("#States= %d unsupported\n", n_state);
    
    if (fp)
	hmm_dump (hmm, n_state, senid, senscr, fp);
    
    return bs;
}
