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
 * The triphone modelled by a given whmm_t is normally obtained by
 * looking up wwpid or rcpid above, using whmm_t.pos and whmm_t.rc.
 * However, left context modelling, unlike right context, is done not
 * by explicit fanout but by multiplexing a single whmm_t structure
 * among all possible instantiations (for all possible left context
 * ciphones).  Each state can be from a different triphone
 * instantiation.  whmm_t.pid[] used for these triphone ids.  
 * (This is probably worse than no explanation.)
 *
 * ARCHAN : Augmented at 20050824
 * 
 * This is how this data structure works and admittedly it is fairly
 * tricky.  When whmm_t is used for normal triphones. (i.e. When we
 * try to model word-internal triphones or any triphone which is
 * full-blown), then one whmm_t will be one such triphone.  In that
 * case, the pointer pid will *point to* (not equal to) one single
 * triphone ID. There is nothing tricky about that. 
 *
 * However, when a multiplexed triphone is used, then the situation
 * will be tricky.  So what is multiplexing in the first place?
 * 
 * It is a technique mainly for **initial phone** and we are
 * considering all **left contexts**.  So just imagine, we are
 * entering a phone from multiple active word-ends. Now, if the
 * triphone is implemented in full, then for each word-end, one will
 * need to allocate one whmm_t to store the context for the word-end.
 *
 * Multiplex triphones though, doesn't require that, when multiple
 * word-ends are entered, only the best pid will be stored in the
 * **first state**.  Another interesting thing is that the best pid
 * will be propagated through the HMM, just like state backtracker or
 * the score. 
 *
 * Notice the state-based pid is, as it's named, state-based.
 * Therefore, when an hmm is in the multiplexed mode, pid will be used
 * as an **array of pid** instead of a **pointer of pid**.  That is
 * why they were handled pretty differently in general in the code
 * (such as flat_fwd.c and ctxt_table.c)
 *
 * Some historical note, at the beginning, this structure only
 * considered right-context, I have changed it to handle left-context
 * as well. However, as I have started to use the flag -multiplex to
 * control the behavior of the search.  I still keep the convention of
 * above.  I will say that is more a pragmatic measure rather a
 * correct measure. 
 */
typedef struct whmm_s {
  int32     *score;		/**< Per state path score */
    int32      bestscore;	/**< Best among this whmm.score[] in current frame */

    s3latid_t *history;		/**< Per state predecessor lattice entry index */

    s3pid_t   *pid;		/**< Triphone id: 1 per state if 1st
				   phone in word, (When it is
				   multiplexed) otherwise single pid
				   for entire phone 
				   
				   When use as one senone,  do pid=*(h->pid)
				   When use as multiple phone, do pid=h->pid[0] for state 0
				*/

    
    int16      pos;		/**< Word pronunciation position index */

    s3cipid_t  rc;		/**< Right context position (only for last phone in word);
				   index into rcpid[][].pid or lrcpid[][].pid */

    s3cipid_t  lc;		/**< Left context position (only for first phone in word);
				     index into lcpid[][].pid or lrcpid[][].pid */

    int32      active;		/**< Whether active in current frame */

  int32 type;  /**< 0 <- multiplex, 1 <- non-multiplex*/
    struct whmm_s *next;	/**< Next active whmm_t for this word */
} whmm_t;


#define whmm_hmmpid(hmm,pid)  *(hmm->pid)

/* Macro for multiplex list or value, notice that, legacy causes the
   answer of IS_MULTIPLEX and the type to be different. 
   Of course, (h->type==MULTPLEX_TYPE) should always be equal to 
   IS_MULTIPLEX(h->pos,mul);  That is a would be a good sanity check. 
  */
#define IS_MULTIPLEX(pos,mul) (pos==0&&mul)
#define MULTIPLEX_TYPE 0
#define NONMULTIPLEX_TYPE 1 


/** Free a whmm */
void whmm_free (whmm_t *h /**< a whmm */
		);

/** Allocate a whmm 
    If pos =0 and multiplex=1, then a list of pid will also be allocated for each state. 
    else pid will just be a pointer of pid. 
 */
whmm_t *whmm_alloc (int32 pos,  /**< position of the hmm */
		    int32 nstate, /**< number of state of the hmm*/
		    int32 alloc_size,  /**< Allocation size , alloc_size/size_of(whmm_t) will be allocated as HMMs that
					 need a separate HMM id every state. 
				       */
		    int32 multiplex   /**< Is it multiplexed?*/
		    );

/** 
    A lesser version of whmm_alloc, whmm_allloc_scores is only
    responsible to allocate memory in score and history.  It also
    won't use the internal structure of whmm to handle multiplexed and
    non-multiplexed triphone. 
 */
whmm_t* whmm_alloc_light(int32 nstate /**< number of state of the hmm*/
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
void eval_nonmpx_whmm (
		       whmm_t *h, /**< the whmm */
		       int32 *senscr, /**< Senone score array */
		       tmat_t *tmat,  /**< tmat*/
		       mdef_t *mdef,   /**< a model definition */
		       int32 n_state  /**< Number of state */
		       );

/**
   Evaluate multiplex whmm . (In Sphinx 3.0, the word begin)
 */

void eval_mpx_whmm (
		    whmm_t *h, /**< the whmm*/
		    int32 *senscr, /**< Senone score array */
		    tmat_t *tmat, /**< tmat*/
		    mdef_t *mdef,   /**< a model definition */
		    int32 n_state  /**< Number of state */
		    );

#endif /* _S3_WHMM_H_ */
