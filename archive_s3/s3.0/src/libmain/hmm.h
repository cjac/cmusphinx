/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * hmm.h -- HMM search structure.
 *
 *
 * HISTORY
 * 
 * 24-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added hmm_vit_trans_comp().
 * 
 * 16-Jul-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_HMM_H_
#define _LIBMAIN_HMM_H_


#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "vithist.h"


/*
 * An individual HMM among the HMM search space.  An HMM with N emitting states consists
 * of N+2 internal states including the non-emitting entry (in) and exit (out) states.
 * For compatibility with Sphinx-II, we assume that the initial or entry state can only
 * transition to state 0, and the transition matrix is n_emit_state x (n_emit_state+1),
 * where the extra destination dimension correponds to the final or exit state.
 */

typedef struct {	/* A single state in the HMM */
    int32 score;	/* State score (path log-likelihood) */
    int32 data;		/* Any auxiliary data */
    vithist_t *hist;	/* Viterbi search history */
} hmm_state_t;

typedef struct {	/* The HMM */
    hmm_state_t *state;	/* Per-state data for emitting states */
    hmm_state_t in;	/* Non-emitting entry state */
    hmm_state_t out;	/* Non-emitting exit state */
    s3pid_t pid;	/* Triphone ID */
    int32 bestscore;	/* Best [emitting] state score in current frame (for pruning) */
    s3frmid_t active;	/* Frame in which HMM was most recently active */
} hmm_t;


/*
 * Reset the states of the HMM to the invalid or inactive condition; i.e., scores to
 * LOGPROB_ZERO and hist to NULL.  h->*.data values are left undefined.
 */
void hmm_clear (mdef_t *m, hmm_t *h);


/*
 * Viterbi evaluation of the given HMM through one frame.  Emitting states are updated.
 * NOTE: The dummy, non-emitting exit state is updated separately.  In the Viterbi DP
 * diagram, transitions to the exit state occur from the current time (are vertical).
 * Hence they should be made only after the history has been logged for the emitting
 * states.
 * Return value: The best state score after the update.
 */
int32 hmm_vit_eval (mdef_t *m,		/* In: HMM model definition */
		    tmat_t *tmat,	/* In: HMM topology transition matrices */
		    hmm_t *h,		/* In/Out: HMM to be evaluated; states updated */
		    int32 *senscore);	/* In: Senone scores */

/*
 * Update the score for the (non-emitting) exit state in the given HMM.
 */
void hmm_vit_eval_exit (mdef_t *m,	/* In: HMM model definition */
			tmat_t *tmat,	/* In: HMM topology transition matrices */
			hmm_t *h);	/* In/Out: HMM to be evaluated; states updated */

/*
 * Enter into the non-emitting initial state of the HMM with the given parameters,
 * and activate the HMM with the given frame number.
 */
void hmm_enter (hmm_t *h,		/* In/Out: HMM to be updated */
		int32 score,		/* In: New score entering into the HMM */
		int32 data,		/* In: New user-defined data */
		vithist_t *hist,	/* In: New Viterbi history */
		s3frmid_t f);		/* In: Active in this frame */

/*
 * Like hmm_enter, but transitions from the dummy exit state of src to the dummy entry
 * state of dst, iff dst->in.score < src->out.score.  The active status of the target
 * is updated to the given frame if the transition is taken.
 * Return value: TRUE iff transition taken and the active status actually changed (i.e.,
 * if the status was already == frm, return value would be 0).
 */
int32 hmm_vit_trans (hmm_t *src,	/* In: Source HMM for the transition */
		     hmm_t *dst,	/* In/Out: Destination HMM */
		     int32 frm);	/* In: Frame for which dst is being activated */

/*
 * Like hmm_vit_comp, but incoming hmm components provided individually, rather than in an
 * hmm_t structure.
 */
int32 hmm_vit_trans_comp (int32 score,		/* In: Incoming score */
			  vithist_t *hist,	/* In: Incoming Viterbi history */
			  int32 data,		/* In: Incoming auxiliary data */
			  hmm_t *dst,		/* In/Out: Destination HMM */
			  int32 frm);		/* In: Frame for which dst is being activated */

/* For debugging */
void hmm_dump (FILE *fp, mdef_t *m, hmm_t *h);


#endif
