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
 * fwd.c -- Forward Viterbi beam search
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 28-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity 
 *              First created it. 
 */

#ifndef _LIBFBS_FWD_H_
#define _LIBFBS_FWD_H_

/* Added by BHIKSHA; Fix for 3 state hmms? 
#define ANYHMMTOPO	1
 End modification by BHIKSHA */


/*
 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 */


/*
 * Triphone information for all word hmm modelling broken up into 4 cases:
 * 	within-word triphones
 * 	left-context cross-word triphones (multi-phone words)
 * 	right-context cross-word triphones (multi-phone words)
 * 	left- and right-cross-word triphones (single-phone words)
 * These 4 cases captured by the following data structures.
 */

/*
 * First, the within word triphone models.  wwpid[w] = list of triphone pronunciations
 * for word w.
 * Since left and right extremes require cross-word modelling (see below), wwpid[w][0]
 * and wwpid[w][pronlen-1] contain no information and shouldn't be touched.
 */
static s3pid_t **wwpid;

typedef struct {
    s3pid_t   *pid;	/* Pid list for all context ciphones; compressed, unique */
    s3cipid_t *cimap;	/* Index into pid[] above for each ci phone */
    int32    n_pid;	/* #Unique pid in above, compressed pid list */
} xwdpid_t;

/*
 * Word HMM instance: the basic structure searched during recognition.
 * 
 * whmm[w] = head of list of all active HMM for word w:
 * 	Will only contain active HMM in current frame.
 * 	List ordered by pronunciation position within word.
 * 	If last phone is active, right context instances ordered as in rcpid or lrcpid
 * 
 * The triphone modelled by a given whmm_t is normally obtained by looking up wwpid or
 * rcpid above, using whmm_t.pos and whmm_t.rc.  However, left context modelling, unlike
 * right context, is done not by explicit fanout but by multiplexing a single whmm_t
 * structure among all possible instantiations (for all possible left context ciphones).
 * Each state can be from a different triphone instantiation.  whmm_t.pid[] used for
 * these triphone ids.
 * (This is probably worse than no explanation.)
 */
typedef struct whmm_s {
    struct whmm_s *next;	/* Next active whmm_t for this word */
    int32     *score;		/* Per state path score */
    s3latid_t *history;		/* Per state predecessor lattice entry index */
    s3pid_t   *pid;		/* Triphone id: 1 per state if 1st phone in word,
				   otherwise single pid for entire phone */
    int32      bestscore;	/* Best among this whmm.score[] in current frame */
    int16       pos;		/* Word pronunciation position index */
    s3cipid_t  rc;		/* Right context position (only for last phone in word);
				   index into rcpid[][].pid or lrcpid[][].pid */
    int32      active;		/* Whether active in current frame */
} whmm_t;

int32 fwd_frame (int32 *senscr);
void fwd_sen_active (int8 *senlist, int32 n_sen);
void fwd_start_utt (char *id);
srch_hyp_t *fwd_end_utt ( void );
void fwd_timing_dump (float64 tot);
void fwd_init (mdef_t* _mdef, tmat_t* _tmat, dict_t* _dict,lm_t *_lm);
srch_hyp_t *s3flat_fwd_dag_search (char *utt);
#endif

