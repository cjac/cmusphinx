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
 * lextree.h -- 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified some functions to be able to deal with HMMs with any number
 * 		of states.
 * 
 * 07-Jul-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lextree_node_t.ci and lextree_ci_active().
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_LEXTREE_H_
#define _S3_LEXTREE_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <libutil/libutil.h>
#include "kbcore.h"
#include "hmm.h"
#include "vithist.h"
#include "ascr.h"


/*
 * A lextree can be built for a specific history (e.g., for all bigram successors of a given
 * word or trigram successors of a word-pair in a given LM).  The history provides a set of left
 * context CIphones (if the final history word has multiple pronunciations; and there is always
 * <sil>).
 * A lextree is usually a set of trees, one for each distinct root model for the given set of
 * words.  Furthermore, the root node of each tree can itself actually be a SET of nodes,
 * required by the different left contexts.  If there is no history (i.e., the unigram lextree),
 * any CIphone is a potential left-context.  But this can explode the number of root nodes.
 * So, the root nodes of the unigram lextree use COMPOSITE models (see dict2pid.h), merging
 * different contexts into one.  Similarly, the right context (at the leaves of any lextree) is
 * always unknown.  So, all leaf nodes also use composite models.
 * Lextrees are formed by sharing as much of the HMM models as possible (based on senone-seq ID),
 * before having to diverge.  But the leaf nodes are always distinct for each word.
 * Finally, each node has a (language model) probability, given its history.  It is the max. of
 * the LM probability of all the words reachable from that node.  (Strictly speaking, it should
 * be their sum instead of max, but practically it makes little difference.)
 */


/*
 * One node in a lextree.
 */
typedef struct {
    hmm_t hmm;		/* HMM states */
    glist_t children;	/* Its data.ptr are children (lextree_node_t *) */
    int32 wid;		/* Dictionary word-ID if a leaf node; BAD_S3WID otherwise */
    int32 prob;		/* LM probability of this node (of all words leading from this node) */
    int32 ssid;		/* Senone-sequence ID (or composite state-seq ID if composite) */
    s3ssid_t *ssid_lc;	/* Array of ssid's (composite or not) for each left context CIphone;
			   READ-ONLY structure */
    s3cipid_t ci;	/* CIphone id for this node */
    int8 composite;	/* Whether it is a composite model (merging many left/right contexts) */
    s3frmid_t frame;	/* Frame in which this node was last active; <0 if inactive */
} lextree_node_t;

/* Access macros; not meant for arbitrary use */
#define lextree_node_wid(n)		((n)->wid)
#define lextree_node_prob(n)		((n)->prob)
#define lextree_node_ssid(n)		((n)->ssid)
#define lextree_node_composite(n)	((n)->composite)
#define lextree_node_frame(n)		((n)->frame)


/*
 * Root nodes of the lextree valid for a given left context CIphone.
 */
typedef struct {
    s3cipid_t lc;	/* Left context CIphone */
    glist_t root;	/* Its data.ptr are the root nodes (lextree_node_t *) of interest; subset
			   of the entire lextree roots */
} lextree_lcroot_t;

/*
 * Entire lextree.
 */
typedef struct {
    int32 type;		/* For use by other modules; NOT maintained here.  For example:
			   N-gram type; 0: unigram lextree, 1: 2g, 2: 3g lextree... */
    glist_t root;	/* The entire set of root nodes (lextree_node_t) for this lextree */
    lextree_lcroot_t *lcroot;	/* Lists of subsets of root nodes; a list for each left context;
				   NULL if n_lc == 0 (i.e., no specific left context) */
    int32 n_lc;		/* No. of separate left contexts being maintained, if any */
    int32 n_node;	/* No. of nodes in this lextree */
    lextree_node_t **active;		/* Nodes active in any frame */
    lextree_node_t **next_active;	/* Like active, but temporary space for constructing the
					   active list for the next frame using the current */
    int32 n_active;		/* No. of nodes active in current frame */
    int32 n_next_active;	/* No. of nodes active in current frame */
    
    int32 best;		/* Best HMM state score in current frame (for pruning) */
    int32 wbest;	/* Best wordexit HMM state score in current frame (for pruning) */
} lextree_t;

/* Access macros; not meant for arbitrary usage */
#define lextree_type(l)			((l)->type)
#define lextree_root(l)			((l)->root)
#define lextree_lcroot(l)		((l)->lcroot)
#define lextree_n_lc(l)			((l)->n_lc)
#define lextree_n_node(l)		((l)->n_node)
#define lextree_active(l)		((l)->active)
#define lextree_next_active(l)		((l)->next_active)
#define lextree_n_active(l)		((l)->n_active)
#define lextree_n_next_active(l)	((l)->n_next_active)


/*
 * Build a lexical tree for the set of words specified in wordprob[] (with their
 * associated LM probabilities).  wordprob[] must contain EXACTLY the set of words for
 * which the lextree is to be built, i.e, including alternatives and excluding OOVs.
 * Return value: Pointer to lextree_t structure representing entire lextree.
 */
lextree_t *
lextree_build (kbcore_t *kbc,		/* In: All the necessary knowledge bases */
	       wordprob_t *wordprob,	/* In: Words in the tree and their (LM) probabilities */
	       int32 n_word,		/* In: Size of the wordprob[] array */
	       s3cipid_t *lc);		/* In: BAD_S3CIPID terminated array of left context
					   CIphones, or NULL if no specific left context */

/* Free a lextree that was created by lextree_build */
void lextree_free (lextree_t *lextree);


/*
 * Reset the entire lextree (to the inactive state).  I.e., mark each HMM node as inactive,
 * (with lextree_node_t.frame = -1), and the active list size to 0.
 */
void lextree_utt_end (lextree_t *l, kbcore_t *kbc);


/*
 * Enter root nodes of lextree for given left-context, with given incoming score/history.
 */
void lextree_enter (lextree_t *lextree,	/* In/Out: Lextree being entered */
		    s3cipid_t lc,	/* In: Left-context if any (can be BAD_S3CIPID) */
		    int32 frame,	/* In: Frame from which being activated (for the next) */
		    int32 inscore,	/* In: Incoming score */
		    int32 inhist,	/* In: Incoming history */
		    int32 thresh);	/* In: Pruning threshold; incoming scores below this
					   threshold will not enter successfully */

/*
 * Swap the active and next_active lists of the given lextree.  (Usually done at the end of
 * each frame: from the current active list, we've built the next_active list for the next
 * frame, and finally need to make the latter the current active list.)
 */
void lextree_active_swap (lextree_t *lextree);


/*
 * Marks the active ssid and composite ssids in the given lextree.  Caller must allocate ssid[]
 * and comssid[].  Caller also responsible for clearing them before calling this function.
 */
void lextree_ssid_active (lextree_t *lextree,	/* In: lextree->active is scanned */
			  int32 *ssid,		/* In/Out: ssid[s] is set to non-0 if senone
						   sequence ID s is active */
			  int32 *comssid);	/* In/Out: comssid[s] is set to non-0 if
						   composite senone sequence ID s is active */

/*
 * For each active lextree node, mark the corresponding CI phone as active.
 */
void lextree_ci_active (lextree_t *lextree,	/* In: Lextree being traversed */
			bitvec_t ci_active);	/* In/Out: Active/inactive flag for ciphones */

/*
 * Evaluate the active HMMs in the given lextree, using the current frame senone scores.
 * Return value: The best HMM state score as a result.
 */
int32 lextree_hmm_eval (lextree_t *lextree,	/* In/Out: Lextree with HMMs to be evaluated */
			kbcore_t *kbc,	/* In: */
			ascr_t *ascr,	/* In: Senone scores (primary and composite) */
			int32 f,	/* In: Frame in which being invoked */
			FILE *fp);	/* In: If not-NULL, dump HMM state (for debugging) */

/*
 * Propagate HMMs in the given lextree through to the start of the next frame.  Called after
 * HMM state scores have been updated.  Marks those with "good" scores as active for the next
 * frame, and propagates HMM exit scores through to successor HMM entry states.
 */
void lextree_hmm_propagate (lextree_t *lextree,	/* In/Out: Propagate scores across HMMs in
						   this lextree */
			    kbcore_t *kbc,	/* In: Core knowledge bases */
			    vithist_t *vh,	/* In/Out: Viterbi history structure to be
						   updated with word exits */
			    int32 cf,		/* In: Current frame */
			    int32 th,		/* In: General (HMM survival) pruning thresh */
			    int32 pth,		/* In: Phone transition pruning threshold */
			    int32 wth,		/* In: Word exit pruning threshold */
                            int32 *phn_heur_list,  /* Added on 20040227 by JSHERWAN for phoneme lookahead */
			    int32 heur_beam, /* Added on 20040228 by JSHERWAN for phoneme lookahead heuristic */
			    int32 heur_type);   /*Heuristic type, used it to by-pass the code if heur_type is 0 */

/*
 * In order to use histogram pruning, get a histogram of the bestscore of each active HMM in
 * the given lextree.  For a given HMM, its bin is determined by:
 * 	(bestscr - hmm->bestscore) / bw.
 * Invoked right after all active HMMs are evaluated.
 */
void lextree_hmm_histbin (lextree_t *lextree,	/* In: Its active HMM bestscores are used */
			  int32 bestscr,	/* In: Overall bestscore in current frame */
			  int32 *bin,		/* In/Out: The histogram bins; caller allocates
						 * this array */
			  int32 nbin,		/* In: Size of bin[] */
			  int32 bw);		/* In: Bin width; i.e., score range per bin */

/* For debugging */
void lextree_dump (lextree_t *lextree, dict_t *dict, FILE *fp);


#ifdef __cplusplus
}
#endif

#endif
