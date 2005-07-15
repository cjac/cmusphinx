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
 * whmm.h -- hmm structure that is used by sphinx 3.0 decode_anytopo (and perhaps
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
 * Revision 1.1.2.1  2005/07/15  07:48:32  arthchan2003
 * split the hmm (whmm_t) and context building process (ctxt_table_t) from the the flat_fwd.c
 * 
 *
 */

#ifndef _S3_WHMM_H_
#define _S3_WHMM_H_


#include <prim_type.h>
#include <s3types.h>
#include <tmat.h>
#include <dict.h>

/** 
 *  \file whmm.h
 *  \brief Word hmm instance that is used by sphinx 3.0 decode_anytopo search. 
 */

/**
 * \struct whmm_t
 * \brief Word HMM instance: the basic structure searched during recognition.
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
    struct whmm_s *next;	/**< Next active whmm_t for this word */
    int32     *score;		/**< Per state path score */
    s3latid_t *history;		/**< Per state predecessor lattice entry index */
    s3pid_t   *pid;		/**< Triphone id: 1 per state if 1st phone in word,
				   otherwise single pid for entire phone */
    int32      bestscore;	/**< Best among this whmm.score[] in current frame */
    int16      pos;		/**< Word pronunciation position index */
    s3cipid_t  rc;		/**< Right context position (only for last phone in word);
				   index into rcpid[][].pid or lrcpid[][].pid */
    int32      active;		/**< Whether active in current frame */
} whmm_t;

/** Free a whmm */
void whmm_free (whmm_t *h /**< a whmm */
		);

/** Allocate a whmm 
 */
whmm_t *whmm_alloc (int32 pos,  /**< position of the hmm */
		    int32 nstate /**< number of state of the hmm*/
		    );


/** Dump one hmm
 */
void dump_whmm (s3wid_t w,  /**< a word id */
		whmm_t *h,  /**< a hmm */
		int32 *senscr, /**< Senone score */
		tmat_t *tmat,  /**< Transition matrix */
		int32 n_frame, /**< The frame number */
		int32 n_state,  /**< The number of state */
		dict_t *dict,  /**< The dictionary */
		mdef_t *mdef   /**< A model definition */
		);


/**
   Evaluate non-multiplex whmm . (In Sphinx 3.0, word-internal and the word end)
 */
void eval_nonmpx_whmm (s3wid_t w, /**< the word ID */
		       whmm_t *h, /**< the whmm */
		       int32 *senscr, /**< Senone score array */
		       tmat_t *tmat,  /**< tmat*/
		       mdef_t *mdef,   /**< a model definition */
		       int32 n_state  /**< Number of state */
		       );

/**
   Evaluate multiplex whmm . (In Sphinx 3.0, the word begin)
 */

void eval_mpx_whmm (s3wid_t w, /**< the word ID */
		    whmm_t *h, /**< the whmm*/
		    int32 *senscr, /**< Senone score array */
		    tmat_t *tmat, /**< tmat*/
		    mdef_t *mdef,   /**< a model definition */
		    int32 n_state  /**< Number of state */
		    );

#endif /* _S3_WHMM_H_ */
