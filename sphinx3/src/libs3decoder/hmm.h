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
 * hmm.h -- HMM data structure.
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
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified hmm_t.state to be a run-time array instead of a compile-time
 * 		one.  Modified compile-time 3 and 5-state versions of hmm_vit_eval
 * 		into hmm_vit_eval_3st and hmm_vit_eval_5st, to allow run-time selection.
 * 		Removed hmm_init().
 * 
 * 08-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added HMM_SKIPARCS compile-time option and hmm_init().
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started, based on an earlier version.
 */


#ifndef _S3_HMM_H_
#define _S3_HMM_H_

#include <libutil/libutil.h>
#include "s3types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * An individual HMM among the HMM search space.  An HMM with N emitting states consists
 * of N+2 internal states including the non-emitting entry (in) and exit (out) states.
 * For compatibility with Sphinx-II, we assume that the initial or entry state can only
 * transition to state 0, and the transition matrix is n_emit_state x (n_emit_state+1),
 * where the extra destination dimension correponds to the final or exit state.
 */

/*
 * NOTE: For efficiency, this version is hardwired for two possible HMM topologies:
 * 
 * 5-state left-to-right HMMs:  (0 is the entry state and E is a non-emitting exit state;
 * the x's indicate allowed transitions between source and destination states):
 * 
 *               0   1   2   3   4   E (destination-states)
 *           0   x   x   x
 *           1       x   x   x
 *           2           x   x   x
 *           3               x   x   x
 *           4                   x   x
 *    (source-states)
 * 5-state topologies that contain a subset of the above transitions should work as well.
 * 
 * 3-state left-to-right HMMs (similar notation as the 5-state topology above):
 * 
 *               0   1   2   E (destination-states)
 *           0   x   x   x
 *           1       x   x   x
 *           2           x   x 
 *    (source-states)
 * 3-state topologies that contain a subset of the above transitions should work as well.
 */


/* A single state in the HMM */
typedef struct {
    int32 score;	/* State score (path log-likelihood) */
    int32 history;	/* History index */
} hmm_state_t;

typedef struct {
    hmm_state_t *state;	/* Per-state data for emitting states */
    hmm_state_t in;	/* Non-emitting entry state */
    hmm_state_t out;	/* Non-emitting exit state */
    int32 **tp;		/* State transition scores tp[from][to] (logs3 values) */
    int32 bestscore;	/* Best [emitting] state score in current frame (for pruning) */
} hmm_t;


/*
 * Reset the states of the HMM to the invalid or inactive condition; i.e., scores to
 * LOGPROB_ZERO and hist to undefined.
 */
void hmm_clear (hmm_t *h, int32 n_emit_state);


/*
 * Viterbi evaluation of given HMM.  (NOTE that if this module were being used for tracking
 * state segmentations, the dummy, non-emitting exit state would have to be updated separately.
 * In the Viterbi DP diagram, transitions to the exit state occur from the current time; they
 * are vertical transitions.  Hence they should be made only after the history has been logged
 * for the emitting states.  But we're not bothered with state segmentations, for now.  So, we
 * update the exit state as well.)
 * Hardwired for 5-state HMMs with topology shown above.
 * Return value: Best state score after evaluation.
 */
int32 hmm_vit_eval_5st (hmm_t *hmm,		/* In/Out: HMM being updated */
			s3senid_t *senid,	/* In: Senone ID for each HMM state */
			int32 *senscore);	/* In: Senone scores, for all senones */

/*
 * Like hmm_vit_eval_5st, but hardwired for 3-state HMMs with topology shown above.
 * Return value: Best state score after evaluation.
 */
int32 hmm_vit_eval_3st (hmm_t *hmm,		/* In/Out: HMM being updated */
			s3senid_t *senid,	/* In: Senone ID for each HMM state */
			int32 *senscore);	/* In: Senone scores, for all senones */

/* Like hmm_vit_eval, but dump HMM state and relevant senscr to fp first, for debugging */
int32 hmm_dump_vit_eval (hmm_t *hmm, int32 n_emit_state,
			 s3senid_t *senid, int32 *senscr, FILE *fp);

/* For debugging */
void hmm_dump (hmm_t *h, int32 n_emit_state, s3senid_t *senid, int32 *senscr, FILE *fp);


#ifdef __cplusplus
}
#endif

#endif
