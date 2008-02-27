/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2008 Carnegie Mellon University.  All rights
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

/**
 * @file ngram_search.h N-Gram based multi-pass search ("FBS")
 */

#ifndef __NGRAM_SEARCH_H__
#define __NGRAM_SEARCH_H__

/* SphinxBase headers. */
#include <cmd_ln.h>
#include <logmath.h>
#include <ngram_model.h>
#include <listelem_alloc.h>

/* Local headers. */
#include "hmm.h"
#include "dict.h"
#include "acmod.h"

/**
 * Lexical tree node data type.
 *
 * Not the first HMM for words, which multiplex HMMs based on
 * different left contexts.  This structure is used both in the
 * dynamic HMM tree structure and in the per-word last-phone right
 * context fanout.
 */
typedef struct chan_s {
    hmm_t hmm;                  /**< Basic HMM structure.  This *must* be first in
                                   the structure because chan_t and root_chan_t are
                                   sometimes used interchangeably */
    struct chan_s *next;	/**< first descendant of this channel; or, in the
				   case of the last phone of a word, the next
				   alternative right context channel */
    struct chan_s *alt;		/**< sibling; i.e., next descendant of parent HMM */

    int32    ciphone;		/**< ciphone for this node */
    union {
	int32 penult_phn_wid;	/**< list of words whose last phone follows this one;
				   this field indicates the first of the list; the
				   rest must be built up in a separate array.  Used
				   only within HMM tree.  -1 if none */
	int32 rc_id;		/**< right-context id for last phone of words */
    } info;
} chan_t;

/**
 * Lexical tree node data type for the first phone (root) of each dynamic HMM tree
 * structure.
 *
 * Each state may have a different parent static HMM.  Most fields are
 * similar to those in chan_t.
 */
typedef struct root_chan_s {
    hmm_t hmm;                  /**< Basic HMM structure.  This *must* be first in
                                   the structure because chan_t and root_chan_t are
                                   sometimes used interchangeably. */
    chan_t *next;		/**< first descendant of this channel */

    int32  penult_phn_wid;
    int32  this_phn_wid;	/**< list of words consisting of this single phone;
				   actually the first of the list, like penult_phn_wid;
				   -1 if none */
    int32    diphone;		/**< first diphone of this node; all words rooted at this
				   node begin with this diphone */
    int32    ciphone;		/**< first ciphone of this node; all words rooted at this
				   node begin with this ciphone */
} root_chan_t;

/**
 * Back pointer table (forward pass lattice; actually a tree)
 */
typedef struct bptbl_s {
    int16    frame;		/**< start or end frame */
    uint8    valid;		/**< For absolute pruning */
    uint8    reserved;          /**< Not used */
    int32    wid;		/**< Word index */
    int32    bp;		/**< Back Pointer */
    int32    score;		/**< Score (best among all right contexts) */
    int32    s_idx;		/**< Start of BScoreStack for various right contexts*/
    int32    real_wid;		/**< wid of this or latest predecessor real word */
    int32    prev_real_wid;	/**< real word predecessor of real_wid */
    int32    r_diph;		/**< rightmost diphone of this word */
    int32    ascr;
    int32    lscr;
} bptbl_t;

/*
 * Candidates words for entering their last phones.  Cleared and rebuilt in each
 * frame.
 * NOTE: candidates can only be multi-phone, real dictionary words.
 */
typedef struct lastphn_cand_s {
    int32 wid;
    int32 score;
    int32 bp;
    int32 next;                 /* next candidate starting at the same frame */
} lastphn_cand_t;

/*
 * Since the same instance of a word (i.e., <word,start-frame>) reaches its last
 * phone several times, we can compute its best BP and LM transition score info
 * just the first time and cache it for future occurrences.  Structure for such
 * a cache.
 */
typedef struct {
    int32 sf;                   /* Start frame */
    int32 dscr;                 /* Delta-score upon entering last phone */
    int32 bp;                   /* Best BP */
} last_ltrans_t;

#define CAND_SF_ALLOCSIZE	32
typedef struct {
    int32 bp_ef;
    int32 cand;
} cand_sf_t;

/*
 * Structure for reorganizing the BP table entries in the current frame according
 * to distinct right context ci-phones.  Each entry contains the best BP entry for
 * a given right context.  Each successor word will pick up the correct entry based
 * on its first ci-phone.
 */
typedef struct bestbp_rc_s {
    int32 score;
    int32 path;                 /* BP table index corresponding to this entry */
    int32 lc;                   /* right most ci-phone of above BP entry word */
} bestbp_rc_t;

#define NO_BP		-1

/**
 * Links between DAG nodes (see latnode_t below).
 * Also used to keep scores in a bestpath search.
 */
typedef struct latlink_s {
    struct latnode_s *from;	/**< From node */
    struct latnode_s *to;	/**< To node */
    struct latlink_s *next;	/**< Next link from the same "from" node */
    struct latlink_s *best_prev;
    struct latlink_s *q_next;
    int32 link_scr;		/**< Score for from->wid (from->sf to this->ef) */
    int32 path_scr;		/**< Best path score from root of DAG */
    int32 ef;			/**< end-frame for the "from" node */
} latlink_t;

typedef struct rev_latlink_s {
    latlink_t *link;
    struct rev_latlink_s *next;
} rev_latlink_t;

/**
 * DAG nodes.
 */
typedef struct latnode_s {
    int32 wid;			/**< Dictionary word id */
    int32 fef;			/**< First end frame */
    int32 lef;			/**< Last end frame */
    int16 sf;			/**< Start frame */
    int16 reachable;		/**< From </s> or <s> */
    union {
	int32 fanin;		/**< #nodes with links to this node */
	int32 rem_score;	/**< Estimated best score from node.sf to end */
    } info;
    latlink_t *links;		/**< Links out of this node */
    rev_latlink_t *revlinks;	/**< Reverse links (for housekeeping purposes only) */
    struct latnode_s *next;	/**< Next node (for housekeeping purposes only) */
} latnode_t;

/**
 * Various statistics for profiling.
 */
typedef struct ngram_search_stats_s {
    int32 n_phone_eval;
    int32 n_root_chan_eval;
    int32 n_nonroot_chan_eval;
    int32 n_last_chan_eval;
    int32 n_word_lastchan_eval;
    int32 n_lastphn_cand_utt;
    int32 n_fwdflat_chan;
    int32 n_fwdflat_words;
    int32 n_fwdflat_word_transition;
    int32 n_senone_active_utt;
} ngram_search_stats_t;

/**
 * N-Gram search module structure.
 */
struct ngram_search_s {
    cmd_ln_t *config;      /**< Configuration. */
    acmod_t *acmod;        /**< Acoustic model. */
    dict_t *dict;          /**< Pronunciation dictionary. */
    ngram_model_t *lmset;  /**< Set of language models. */
    hmm_context_t *hmmctx; /**< HMM context. */
    char *hyp_str;         /**< Current hypothesis string. */

    /* Allocators */
    listelem_alloc_t *chan_alloc; /**< For chan_t */
    listelem_alloc_t *root_chan_alloc; /**< For root_chan_t */
    listelem_alloc_t *latnode_alloc; /**< For latnode_t */

    /**
     * Search structure of HMM instances.
     *
     * The word triphone sequences (HMM instances) are transformed
     * into tree structures, one tree per unique left triphone in the
     * entire dictionary (actually diphone, since its left context
     * varies dyamically during the search process).  The entire set
     * of trees of channels is allocated once and for all during
     * initialization (since dynamic management of active CHANs is
     * time consuming), with one exception: the last phones of words,
     * that need multiple right context modelling, are not maintained
     * in this static structure since there are too many of them and
     * few are active at any time.  Instead they are maintained as
     * linked lists of CHANs, one list per word, and each CHAN in this
     * set is allocated only on demand and freed if inactive.
     */
    root_chan_t *root_chan;  /**< Roots of search tree. */
    int32 n_root_chan_alloc; /**< Number of root_chan allocated */
    int32 n_root_chan;       /**< Number of valid root_chan */
    int32 n_nonroot_chan;    /**< Number of valid non-root channels */
    int32 max_nonroot_chan;  /**< Maximum possible number of non-root channels */
    int32 *first_phone_rchan_map;    /* map 1st (left) diphone to root-chan index */
    root_chan_t *all_rhmm;   /**< Root HMMs for single-phone words */

    /**
     * Channels associated with a given word (only used for right
     * contexts and single-phone words in fwdtree search)
     */
    chan_t **word_chan;
    bitvec_t *word_active;      /**< array of active flags for all words. */

    /*
     * Some words have special meanings so we track them here.
     */
    int32 start_wid;
    int32 finish_wid;
    int32 silence_wid;

    /**
     * Each node in the HMM tree structure may point to a set of words
     * whose last phone would follow that node in the tree structure
     * (but is not included in the tree structure for reasons
     * explained above).  The channel node points to one word in this
     * set of words.  The remaining words are linked through
     * homophone_set[].
     * 
     * Single-phone words are not represented in the HMM tree; they
     * are kept in word_chan.
     *
     * Specifically, homophone_set[w] = wid of next word in the same
     * set as w.
     */
    int32 *homophone_set;
    int32 *single_phone_wid; /**< list of single-phone word ids */
    int32 n_1ph_words;       /**< #single phone words in dict (total) */
    int32 n_1ph_LMwords;     /**< #single phone dict words also in LM;
                                these come first in single_phone_wid */
    /**
     * Array of active channels for current and next frame.
     *
     * In any frame, only some HMM tree nodes are active.
     * active_chan_list[f mod 2] = list of nonroot channels in the HMM
     * tree active in frame f.
     */
    chan_t ***active_chan_list;
    int32 n_active_chan[2];  /**< #entries in active_chan_list */
    /**
     * Array of active multi-phone words for current and next frame.
     *
     * Similarly to active_chan_list, active_word_list[f mod 2] = list
     * of word ids for which active channels exist in word_chan in
     * frame f.
     *
     * Statically allocated single-phone words are always active and
     * should not appear in this list.
     */
    int32 **active_word_list;
    int32 n_active_word[2];  /**< #entries in active_word_list */

    /*
     * FIXME: Document all of these bits.
     */
    lastphn_cand_t *lastphn_cand;
    int32 n_lastphn_cand;
    last_ltrans_t *last_ltrans;      /* one per word */
    int32 cand_sf_alloc;
    cand_sf_t *cand_sf;
    bestbp_rc_t *bestbp_rc;

    bptbl_t *bp_table;       /* Forward pass lattice */
    int32 bpidx;             /* First free BPTable entry */
    int32 bp_table_size;
    int32 *bscore_stack;     /* Score stack for all possible right contexts */
    int32 bss_head;          /* First free BScoreStack entry */
    int32 bscore_stack_size;

    int32 n_frame_alloc;
    int32 *bp_table_idx; /* First BPTable entry for each frame */
    int32 *word_lat_idx; /* BPTable index for any word in current frame;
                            cleared before each frame */

    int32 *zeroPermTab; /**< Null right context table, just an array of
                           n_ciphone zeros (!!) */

    /*
     * Flat lexicon (2nd pass) search stuff.
     */
    latnode_t **frm_wordlist;
    int32 *fwdflat_wordlist;
    bitvec_t *expand_word_flag;
    int32 *expand_word_list;
    int32 n_expand_words;
    int32 min_ef_width;
    int32 max_sf_win;

    ascr_t best_score; /**< Best Viterbi path score. */
    ascr_t last_phone_best_score; /**< Best Viterbi path score for last phone. */
    int32 renormalized;
 
    ngram_search_stats_t st; /**< Various statistics for profiling. */

    /* A collection of beam widths. */
    int32 beam;
    int32 dynamic_beam;
    int32 pbeam;
    int32 wbeam;
    int32 lpbeam;
    int32 lponlybeam;
    int32 fwdflatbeam;
    int32 fwdflatwbeam;
    int32 fillpen;
    int32 silpen;
    int32 wip;
    int32 nwpen;
    int32 pip;
    int32 maxwpf;
    int32 maxhmmpf;
};
typedef struct ngram_search_s ngram_search_t;

/**
 * Initialize the N-Gram search module.
 */
ngram_search_t *ngram_search_init(cmd_ln_t *config,
                                  acmod_t *acmod,
                                  dict_t *dict);

/**
 * Finalize the N-Gram search module.
 */
void ngram_search_free(ngram_search_t *ngs);

/**
 * Record the current frame's index in the backpointer table.
 *
 * @return the current backpointer index.
 */
int ngram_search_mark_bptable(ngram_search_t *ngs, int frame_idx);

/**
 * Allocate last phone channels for all possible right contexts for word w.
 */
void ngram_search_alloc_all_rc(ngram_search_t *ngs, int32 w);

/**
 * Allocate last phone channels for all possible right contexts for word w.
 */
void ngram_search_free_all_rc(ngram_search_t *ngs, int32 w);

/**
 * Find the best word exit for the current frame in the backpointer table.
 *
 * @return the backpointer index of the best word exit.
 */
int ngram_search_find_exit(ngram_search_t *ngs, int frame_idx, ascr_t *out_best_score);

/**
 * Backtrace from a given backpointer index to obtain a word hypothesis.
 *
 * @return a <strong>read-only</strong> string with the best hypothesis.
 */
char const *ngram_search_hyp(ngram_search_t *ngs, int bpidx);

#endif /* __NGRAM_SEARCH_H__ */