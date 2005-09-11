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
 *
 * $Log$
 * Revision 1.10.4.4  2005/09/11  02:58:10  arthchan2003
 * remove most dag-related functions except dag_build. Use latticehist_t insteads of loosed arrays.
 * 
 * Revision 1.10.4.3  2005/09/07 23:40:06  arthchan2003
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
 * Revision 1.10.4.2  2005/08/02 21:12:45  arthchan2003
 * Changed senlist from 8-bit to 32-bit. It will be compatible to the setting of ascr's sen_active.
 *
 * Revision 1.10.4.1  2005/07/15 07:50:33  arthchan2003
 * Remove hmm computation and context building code from flat_fwd.c.
 *
 *
 */

#ifndef _LIBFBS_FWD_H_
#define _LIBFBS_FWD_H_

/* Added by BHIKSHA; Fix for 3 state hmms? 
#define ANYHMMTOPO	1
 End modification by BHIKSHA */


/** \file flat_fwd.h
   \brief (Currently not opened to public) Header for forward search for flat lexicon

 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 */


/** 
 * \struct backoff_t
 *
 * Backoff node when backing off all the way to unigrams.  Since each
 * word exits with #ciphones different scores (for so many different
 * right contexts), a separate node exists for each context.
 */
typedef struct {
    s3latid_t latid;	/**< History entry */
    int32 score;	/**< Acoustic + backed off LM score */
    s3cipid_t lc;	/**< Last ciphone of history entry, to be used as left context upon
			   entering a new word. */
} backoff_t;


/**
 * \struct word_ugprob_t
 *
 * Unigrams re-organized for faster unigram word transitions.  Words
 * partitioned by their first CI phone and ordered in descending
 * unigram probability within each partition.
 */
typedef struct word_ugprob_s {
  s3wid_t wid;        /**< Word ID */
  int32 ugprob;     /**< Unigram probability */
  struct word_ugprob_s *next;   /**< Nex unigram probability*/
} word_ugprob_t;

/**
 * \struct fwd_dbg_t 
 *
 * Structure for debugging flat forward search. 
 */

/* Debugging */
typedef struct {
  s3wid_t trace_wid;	/**< Word to be traced; for debugging */
  int32 word_dump_sf;	/**< Start frame for words to be dumped for debugging */
  int32 word_dump_ef;	/**< End frame for words to be dumped for debugging */
  int32 hmm_dump_sf;	/**< Start frame for HMMs to be dumped for debugging */
  int32 hmm_dump_ef;	/**< End frame for HMMs to be dumped for debugging */
} fwd_dbg_t ;



/**
 * Initialization of flat forward search 
 */
void fwd_init (mdef_t* _mdef,  /**< A model definition */
	       tmat_t* _tmat,  /**< A transition matrix */
	       dict_t* _dict,  /**< A dictionary */
	       lm_t *_lm       /**< An LM */
	       );

/**
 * Initialization of flat forward search 
 */
void fwd_free();

/**
 * Start of flat foward search 
 */ 
void fwd_start_utt (char *id /**< ID of an utterance */
		    );

/** 
 * Make the search to go forward for one frame. 
 * @return best score of this frame. 
 */
int32 fwd_frame (int32 *senscr /**< An array of senone score */
		 ); 

/**
 * Find the active senone list. 
 */
void fwd_sen_active (int32 *senlist, int32 n_sen);

/**
 * End of flat foward search 
 * @return searching hypothesis. 
 */ 

srch_hyp_t *fwd_end_utt ( void );

/**
   Dump timing. 
 */

void fwd_timing_dump (float64 tot /**< Total */
		      );

/**
 * The flat forward search version of DAG search. Very similar to what
 * one could find in s3dag_dag_search. 
 */

srch_hyp_t *s3flat_fwd_dag_search (char *utt /**< utterance id */
				   );

/**
 * Build a DAG from the lattice: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * dagnodes_list = linear list of DAG nodes allocated, ordered such that nodes earlier
 * in the list can follow nodes later in the list, but not vice versa:  Let two DAG
 * nodes d1 and d2 have start times sf1 and sf2, and end time ranges [fef1..lef1] and
 * [fef2..lef2] respectively.  If d1 appears later than d2 in dag.list, then
 * fef2 >= fef1, because d2 showed up later in the word lattice.  If there is a DAG
 * edge from d1 to d2, then sf1 > fef2.  But fef2 >= fef1, so sf1 > fef1.  Reductio ad
 * absurdum.
 */

int32 dag_build ( void );

/** Dump dag in s3.0 format
 * A function that can dump a dag given a lattice_t structure. 
 */
void s3flat_fwd_dag_dump (char *dir,  /**< The output directory */
			   int32 onlynodes, /**< Dump only nodes of the DAG*/
			   char *id    /**< Sentence ID*/
			   );

#endif

