/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/*
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:16:14  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Add wrapper of flat_fwd.c
 *
 * Revision 1.1.2.5  2006/01/16 20:11:23  arthchan2003
 * Interfaces for 2nd stage search, now commented.
 *
 * Revision 1.1.2.4  2005/11/17 06:42:15  arthchan2003
 * Added back crossword triphone traversing timing for search. Also. for consistency with srch.c.  Some dummy code of IBM lattice conversion was added. They are now bypassed because it is not fully function.
 *
 * Revision 1.1.2.3  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.1.2.2  2005/09/18 01:45:19  arthchan2003
 * Filled in all implementation in srch_flat_fwd.[ch], like the FSG mode, it takes care of reporting itselft.
 *
 * Revision 1.1.2.1  2005/07/24 01:40:37  arthchan2003
 * (Incomplete) The implementation of flat-lexicon decoding.
 *
 *
 *
 */

/* \file srch_flat_fwd.h
 * 
 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 *
 */

#ifndef SRCH_FLT_FWD_INTERNAL
#define SRCH_FLT_FWD_INTERNAL


#include <stdio.h>

#include <sphinxbase/profile.h>
#include <lm.h>
#include <kbcore.h>
#include <vithist.h>
#include <word_ugprob.h>
#include <word_graph.h>
#include <whmm.h>
#include <hmm.h>
#include <ctxt_table.h>
#include <dict.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

/**
 * \struct fwd_dbg_t 
 *
 * Structure for debugging  search. 
 */
typedef struct {
    s3wid_t trace_wid;	/**< Word to be traced; for debugging */
    int32 word_dump_sf;	/**< Start frame for words to be dumped for debugging */
    int32 word_dump_ef;	/**< End frame for words to be dumped for debugging */
    int32 hmm_dump_sf;	/**< Start frame for HMMs to be dumped for debugging */
    int32 hmm_dump_ef;	/**< End frame for HMMs to be dumped for debugging */
} fwd_dbg_t ;


/** 
 * \struct backoff_t
 *
 * Backoff node when backing off all the way to unigrams.  Since each
 * word exits with \#ciphones different scores (for so many different
 * right contexts), a separate node exists for each context.
 */
typedef struct {
    s3latid_t latid;	/**< History entry */
    int32 score;	/**< Acoustic + backed off LM score */
    s3cipid_t lc;	/**< Last ciphone of history entry, to be used as left context upon
			   entering a new word. */
} backoff_t;

typedef struct word_cand_s {
    s3wid_t wid;		/**< A particular candidate word starting in a given frame */
    struct word_cand_s *next;	/**< Next candidate starting in same frame; NULL if none */
} word_cand_t;


/*
  \struct srch_FLAT_FWD_graph_t;
  
*/
typedef struct srch_FLAT_FWD_graph_s {

    /**
     * Structures for decoding utterances subject to given input word lattices; ie, restricting
     * the decoding to words found in the lattice.  (For speeding up the decoding process.)
     * NOTE:  This mode is optional.  If no input lattice is given, the entire vocabulary is
     * eligible during recognition.  Also, SILENCEWORD, FINISHWORD, and noisewords are always
     * eligible candidates.
     * 
     * Input lattice specifies candidate words that may start at a given frame.  In addition,
     * this forward pass can also consider words starting at a number of neighbouring frames
     * within a given window.
     * 
     * Input lattice file format:  Each line contains a single \<word\> \<startframe\> info.  The
     * line may contain other info following these two fields; these are ignored.  Empty lines
     * and lines beginning with a # char in the first column (ie, comment lines) are ignored.
     */

    /*
      FIXME! This should be used by the generic search as well. 
    */
    char const *word_cand_dir;	/**< Directory containing candidate words
				   files.  If NULL, full search performed for
				   entire run */
  
    char const *latfile_ext;	/**< Complete word candidate filename for an utterance formed
                           by word_cand_dir/\<uttid\>.latfile_ext */
    int32 word_cand_win;	/**< In frame f, candidate words in input lattice from frames
				   [(f - word_cand_win) .. (f + word_cand_win)] will be
				   the actual candidates to be started(entered) */
    word_cand_t **word_cand;	/**< Word candidates for each frame.  (NOTE!! Another array
                                   with a hard limit on its size.) */
    int32 n_word_cand;	/**< \#candidate entries in word_cand for current utterance.
                           If <= 0; full search performed for current utterance */


    /**
     * Structures for flat lexicon decoding search 
     */
    hmm_context_t *hmmctx; /**< The HMM context. */
    whmm_t **whmm;         /**< The word hmms list.  For actual search traverse */

    word_ugprob_t **word_ugprob; /**< word unigram probability */
    backoff_t *ug_backoff;       /**< Unigram backoff probability */
    backoff_t *filler_backoff;   /**< Filler probability */
    uint8 *tg_trans_done;	/**< If tg_trans_done[w] TRUE, trigram transition to w
				   occurred for a given history, and backoff bigram
				   transition from same history should be avoided */

    int32 *rcscore;	/**< rc scores uncompacted; one entry/rc-ciphone */

    s3wid_t *word_cand_cf;	/**< BAD_S3WID terminated array of candidate words for word
				   transition in current frame (if using input word
				   lattices to restrict search). */

    ctxt_table_t *ctxt;           /**< A context table. This parameter,
                                     as well as the one in word_fsg.c
                                     should all go up to srch.c. They
                                     are more generic  then the others.  */

    fwd_dbg_t *fwdDBG;            /**< Debug object of srch_flat_fwd.c */

    latticehist_t *lathist;       /**< Viterbi history (backpointer) table */

    /*
      states for the search 
    */ 
    int32 n_frm;        /**< Number of frame of this utternance */
    int32 final_state;      /**< Final state is supposed to be the last state, so it is usually equal to n_state-1*/
    int32 renormalized;	/**< Whether scores had to be renormalized in current utt */
    int32 multiplex;       /**< Whether we will use multiplexed triphones */
    int32 multiplex_singleph;       /**< Whether we will use multiplexed triphones */

    /* Event count statistics */
    pctr_t* ctr_mpx_whmm;
    pctr_t* ctr_nonmpx_whmm;
    pctr_t* ctr_latentry;

    ptmr_t tm_hmmeval;
    ptmr_t tm_hmmtrans;
    ptmr_t tm_wdtrans;

    kbcore_t* kbcore;     /**< A pointer for convenience */
} srch_FLAT_FWD_graph_t ;


/**
 * Build array of candidate words that start around the current frame (cf).
 * Note: filler words are not in this list since they are always searched (see
 * word_trans).
 */
void build_word_cand_cf (int32 cf, /**< Current frame */
			 dict_t *dict, /**< The dictionary */
			 s3wid_t* wcand_cf, /**< The array of word candidate */
			 int32 word_cand_win, /**< In frame f, candidate words in input lattice from frames
						 [(f - word_cand_win) .. (f + word_cand_win)] will be
						 the actual candidates to be started(entered) */
			 word_cand_t ** wcand

    );



/**
 * Load word candidate into a list 
 */
int32 word_cand_load (FILE *fp,  /**< An initialized for inputfile poiner */
		      word_cand_t** wcand, /**< list of word candidate */
		      dict_t *dict, /**< The dictionary*/
		      char* uttid   /**< The ID of an utterance */
    );


/**
 * Free word candidate
 */
void word_cand_free ( word_cand_t ** wcand  /**< list of word candidate to free */
    );

/** Evaluate one frame in a whmm (mostly a wrapper around hmm_vit_eval()) */
int32 whmm_eval(srch_FLAT_FWD_graph_t * fwg, int32 * senscr);

void dump_all_whmm(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm,
                   int32 n_frm, int32 * senscr);

void dump_all_word(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm);

void whmm_renorm(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm, int32 bestscr);

void whmm_transition(srch_FLAT_FWD_graph_t * fwg, whmm_t ** whmm, int32 w,
		     whmm_t * h);

void word_enter(srch_FLAT_FWD_graph_t * fwg, s3wid_t w,
                int32 score, s3latid_t l, s3cipid_t lc);

void whmm_exit(srch_FLAT_FWD_graph_t * fwg,
               whmm_t ** whmm,
               latticehist_t * lathist,
               int32 thresh, int32 wordthresh, int32 phone_penalty);


void word_trans(srch_FLAT_FWD_graph_t * fwg,
                whmm_t ** whmm,
                latticehist_t * lathist,
                int32 thresh, int32 phone_penalty);

void flat_fwd_dag_add_fudge_edges(srch_FLAT_FWD_graph_t * fwg,
                                  dag_t * dagp,
                                  int32 fudge,
                                  int32 min_ef_range,
                                  void *hist, dict_t * dict);

#ifdef __cplusplus
}
#endif


#endif /* SRCH_FLT_FWD_INTERNAL */
