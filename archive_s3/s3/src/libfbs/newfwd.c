/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2002/12/03  23:02:43  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 * 
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 04-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_chk_linkscr().  Added check for renormalization before bestpath.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Added lwf factoring of fillpen in dag_backtrace().
 * 
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed hardwired MIN_EF_RANGE constant into -min_endfr argument.
 * 		Added fudge edges in dag (dag_add_fudge_edges).
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added exact reporting of word sequence and scores from dag_search.
 * 		For this, added daglink_t.bypass, daglink_t.lscr, daglink_t.src, and
 * 		added bypass argument to dag_link and dag_bypass_link, and changed
 * 		dag_backtrace to find exact best path.
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *  		Added onlynodes argument to dag_dump().
 *  
 * 29-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Applied lwf to filler penalties in dag_remove_filler_nodes().
 *  
 * 28-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Corrected for backoff case of LM score in lat_seg_lscr().
 *  
 * 15-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Undid previous change: now the complete DAG is built whether the bestpath
 * 		search is to be run or not.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Replace explicit silpen and noisepen with calls to fillpen().
 *  
 * 05-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Added pscr_valid flag to daglink_t to avoid evaluating the
 * 		same path mulitple times (millions of times, in some cases).
 *  
 * 27-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		BUGFIX: Added checks in dag_bestpath and dag_search for dealing with
 * 		zero paths through DAG (caused by introduction of MIN_EF_RANGE).
 *  
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added separate language weight (-bestpathlw) for bestpath DAG search.
 * 		Added MIN_EF_RANGE to limit active nodes in DAG search.  Removed internal
 * 		finishwid nodes from DAG search.
 *  
 * 21-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added and used -bptblsize argument.
 * 		Freed rcscore entries in BP table if not running bestpath search (for
 * 		reducing memory requirement; but causes acoustic scores in dumped
 * 		lattices to be inaccurate).
 *  
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed fwd_sen_active to flag active senones instead of building a list
 * 		of them.
 * 
 * 09-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed (> thresh) to (>= thresh) in word_trans, for consistency.
 * 		Added word_ugprob structure and use in word_trans() to speed up unigram
 * 		cross-word transitions.  (Didn't help that much.)
 * 		Postponed pruning and reclaiming of inactive whmm to whmm_eval, to avoid
 * 		unnecessarily deallocating HMMs, only to allocate them again because of
 * 		an incoming transition.
 * 		Changed tp[][] indices to tp[] in 5-state specific eval_nonmpx_whmm and
 * 		eval_mpx_whmm, again to speed up whmm_eval.  (Helped a bit.)
 * 
 * 06-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Create edges in DAG iff bestpath search being done.  Reduces size of
 * 		dumped lattices, but cannot be used to run bestpath search.
 * 
 * 29-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed output lattice file to include edges and best ending scores.
 * 		Changed input lattice file format to conform to output format.
 * 
 * 26-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: </s> never becomes an active candidate if using an input lattice
 * 		to constrain search AND </s> appears in filler dictionary.
 * 
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added DAG search.
 * 
 * 02-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check (through tg_trans_done) in word_trans to avoid backing off to
 * 		bigram transition w2->w3 if trigram transition w1,w2->w3 already done.
 * 
 * 29-Mar-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added the reporting of no. of triphones mapped to ciphones.  (BUG: The
 * 		number reported is not accurate as it counts the number of such INSTANCES
 * 		for within-word triphones, but only the SET of cross-word triphones.)
 * 
 * 12-Mar-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added checks in eval_mpx_whmm and eval_nonmpx_whmm for detecting
 * 		very poor state scores and flooring them to LOGPROB_ZERO.  Otherwise,
 * 		these scores could overflow and turn +ve.
 * 
 * 26-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Fixed bug in build_wwpid (pointed out by purify) that accessed
 * 		out of bounds memory in the case of single-phone words.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added functionality to constrain search to words in given input lattices.
 * 		This mainly affects the word_trans function.
 * 		Added fwd_sen_active() function.
 * 		Added code to increase lattice[] size (realloc) when it overflows, instead
 * 		of exiting with an error message.
 * 
 * 20-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed allocation of whmm state,latid,pid vectors to block mode
 * 		allocation in whmm_alloc (suggested by Paul Placeway).
 * 
 * 10-Aug-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include <s3.h>

#include "s3types.h"
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "logs3.h"
#include "search.h"


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

/*
 * Left context mapping (for multiphone words): given the 1st base phone, b, of a word
 * and its right context, r, the triphone for any left context l =
 *     lcpid[b][r].pid[lcpid[b][r].cimap[l]].
 * 
 * Similarly, right context mapping (for multiphone words): given b and left context l,
 * the triphone for any right context r =
 *     rcpid[b][l].pid[lcpid[b][l].cimap[r]].
 * 
 * A single phone word is a combination of the above, where both l and r are unknown.
 * Triphone, given any l and r context ciphones:
 *     lrcpid[b][l].pid[lcpid[b][l].cimap[r]].
 * For simplicity, all cimap[] vectors (for all l) must be identical.  For now, this is
 * attained by avoiding any compression and letting cimap be the identity map.
 * 
 * Reason for compressing pid[] and indirect access: senone sequences for triphones not
 * distinct.  Hence, cross-word modelling fanout at word exits can be limited by fanning
 * out to only distinct ones and sharing the results among all ciphones.
 */
typedef struct {
    s3pid_t   *pid;	/* Pid list for all context ciphones; compressed, unique */
    s3cipid_t *cimap;	/* Index into pid[] above for each ci phone */
    int32    n_pid;	/* #Unique pid in above, compressed pid list */
} xwdpid_t;
static xwdpid_t **lcpid;
static xwdpid_t **rcpid;
static xwdpid_t **lrcpid;

static int32 n_backoff_ci;	/* #Triphone instances backed off to ciphones */
static int8 *word_start_ci;
static int8 *word_end_ci;


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
    int8       pos;		/* Word pronunciation position index */
    s3cipid_t  rc;		/* Right context position (only for last phone in word);
				   index into rcpid[][].pid or lrcpid[][].pid */
    int32      active;		/* Whether active in current frame */
} whmm_t;
static whmm_t **whmm;


/*
 * DAG structure representation of word lattice.  A unique <wordid,startframe> is a node.
 * Edges are formed if permitted by time adjacency.  (See comment before dag_build.)
 */
typedef struct dagnode_s {
    s3wid_t wid;
    int32 seqid;			/* Running sequence no. for identification */
    s3frmid_t sf;			/* Start frame for this occurrence of wid */
    s3frmid_t fef, lef;			/* First and last end frames */
    struct dagnode_s *alloc_next;	/* Next in linear list of allocated nodes */
    struct daglink_s *succlist;		/* List of successor nodes (adjacent in time) */
    struct daglink_s *predlist;		/* List of preceding nodes (adjacent in time) */
} dagnode_t;

/* A DAG node can have several successor/predecessor nodes, each represented by a link */
typedef struct daglink_s {
    dagnode_t *node;		/* Target of link (source determined by dagnode_t.succlist
				   or dagnode_t.predlist) */
    dagnode_t *src;		/* Source node of link */
    struct daglink_s *next;	/* Next in same dagnode_t.succlist or dagnode_t.predlist */
    struct daglink_s *history;	/* Previous link along best path (for traceback) */
    struct daglink_s *bypass;	/* If this links A->B, bypassing A->fillnode->B, then
				   bypass is ptr to fillnode->B */
    int32 ascr;			/* Acoustic score for segment of source node ending just
				   before the end point of this link.  (Actually this gets
				   corrupted because of filler node deletion.) */
    int32 lscr;			/* LM score to the SUCCESSOR node */
    int32 pscr;			/* Best path score to root beginning with this link */
    s3frmid_t ef;		/* End time for this link.  Should be 1 before the start
				   time of destination node (or source node for reverse
				   links), but gets corrupted because of filler deletion */
    uint8 pscr_valid;		/* Flag to avoid evaluating the same path multiple times */
} daglink_t;

/* Summary of DAG structure information */
typedef struct {
    dagnode_t *list;		/* Linear list of nodes allocated */
    dagnode_t *root;		/* Corresponding to (<s>,0) */
    daglink_t final;		/* Exit link from final DAG node */
    int32 filler_removed;	/* Whether filler nodes removed from DAG to help search */
    int32 fudged;		/* Whether fudge edges have been added */
    s3latid_t latfinal;		/* Lattice entry determined to be final end point */
} dag_t;
static dag_t dag;

    
/*
 * Word lattice for recording decoded hypotheses.
 * 
 * lattice[i] = entry for a word ending at a particular frame.  There can be at most one
 * entry for a word in a given frame.
 * NOTE: lattice array allocated statically.  Need a more graceful way to grow without
 * such an arbitrary internal limit.
 */
typedef struct lattice_s {
    s3wid_t   wid;	/* Decoded word */
    s3frmid_t frm;	/* End frame for this entry */
    s3latid_t history;	/* Index of predecessor lattice_t entry */
    int32     score;	/* Best path score upto the end of this entry */
    int32    *rcscore;	/* Individual path scores for different right context ciphones */
    dagnode_t *dagnode;	/* DAG node representing this entry */
} lattice_t;
static lattice_t *lattice;
static int32 lat_alloc;		/* #lattice entries allocated */
static int32 n_lat_entry;	/* #lattice entries used at any point */
#define LAT_ALLOC_INCR		32768

#define LATID2SF(l)	(IS_LATID(lattice[l].history) ? \
			 lattice[lattice[l].history].frm + 1 : 0)

/*
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
 * Input lattice file format:  Each line contains a single <word> <startframe> info.  The
 * line may contain other info following these two fields; these are ignored.  Empty lines
 * and lines beginning with a # char in the first column (ie, comment lines) are ignored.
 */
static char *word_cand_dir;	/* Directory containing candidate words files.  If NULL,
				   full search performed for entire run */
static char *latfile_ext;	/* Complete word candidate filename for an utterance formed
				   by word_cand_dir/<uttid>.latfile_ext */
static int32 word_cand_win;	/* In frame f, candidate words in input lattice from frames
				   [(f - word_cand_win) .. (f + word_cand_win)] will be
				   the actual candidates to be started(entered) */
typedef struct word_cand_s {
    s3wid_t wid;		/* A particular candidate word starting in a given frame */
    struct word_cand_s *next;	/* Next candidate starting in same frame; NULL if none */
} word_cand_t;
static word_cand_t **word_cand;	/* Word candidates for each frame.  (NOTE!! Another array
				   with a hard limit on its size.) */
static int32 n_word_cand;	/* #candidate entries in word_cand for current utterance.
				   If <= 0; full search performed for current utterance */

/*
 * Structure for "efficient" implementation of breaking up search through compound words
 * into a sequence of component words.  I.e., compound words are needed only for lexical
 * modelling; for the language model we consider them as sequences of the component
 * words.  To implement this, when we transition to the beginning of a compound word,
 * we apply the LM transition probability for only the first component.  The LM scores
 * for the remaining component words are added as we go through the phone sequence for
 * the compound word, approximately evenly spaced through the pronunciation.
 */
static s3wid_t *comp_alt;	/* If w is not a compound word, comp_alt[w] is the head
				   of the list of compound words with w as the first
				   component.  All such compound words are linked using
				   this same array. */

/* Various search-related parameters */
static int32 beam;		/* General beamwidth */
static int32 wordbeam;		/* Beam for exiting a word */

static int32 phone_penalty;	/* Applied for each phone transition */

static int32 n_state = 0;
static int32 final_state;

static s3wid_t silwid;		/* General silence word id */
static s3wid_t startwid;	/* Begin silence */
static s3wid_t finishwid;	/* End silence */

static dict_t *dict;		/* The dictionary */
static mdef_t *mdef;		/* HMM model definition parameters */
static tmat_t *tmat;		/* HMM transition probabilities matrices */
static lm_t   *lm;		/* The currently active language model */

static char *uttid = NULL;	/* Utterance id; for error reporting */
static int32 n_frm;		/* Current frame being searched within utt */
static s3latid_t *frm_latstart;	/* frm_latstart[f] = first lattice entry in frame f */

static hyp_t *hyp = NULL;	/* The final recognition result */
static int32 renormalized;	/* Whether scores had to be renormalized in current utt */

static corpus_t *lmcontext_corp;
static s3wid lmcontext_pred[2], lmcontext_succ;

/* Debugging */
static s3wid_t trace_wid;	/* Word to be traced; for debugging */
static int32 word_dump_sf;	/* Start frame for words to be dumped for debugging */
static int32 hmm_dump_sf;	/* Start frame for HMMs to be dumped for debugging */

/* Event count statistics */
static int32 ctr_mpx_whmm;
static int32 ctr_nonmpx_whmm;
static int32 ctr_latentry;
static timing_t *tm_hmmeval;
static timing_t *tm_hmmtrans;
static timing_t *tm_wdtrans;


/* Get rid of old hyp, if any */
static void hyp_free ( void )
{
    hyp_t *tmphyp;
    
    while (hyp) {
	tmphyp = hyp->next;
	listelem_free ((char *)hyp, sizeof(hyp_t));
	hyp = tmphyp;
    }
}


static int32 filler_word (s3wid_t w)
{
    if ((w == startwid) || (w == finishwid))
	return 0;
    if ((w >= dict->filler_start) && (w <= dict->filler_end))
	return 1;
    return 0;
}


#if 0
static void dump_xwdpidmap (xwdpid_t **x)
{
    s3cipid_t b, c1, c2;
    s3pid_t p;

    for (b = 0; b < mdef->n_ciphone; b++) {
	if (! x[b])
	    continue;
	
	for (c1 = 0; c1 < mdef->n_ciphone; c1++) {
	    if (! x[b][c1].cimap)
		continue;
	    
	    printf ("n_pid(%s, %s) = %d\n",
		    mdef_ciphone_str(mdef, b), mdef_ciphone_str(mdef, c1),
		    x[b][c1].n_pid);

	    for (c2 = 0; c2 < mdef->n_ciphone; c2++) {
		p = x[b][c1].pid[x[b][c1].cimap[c2]];
		printf ("  %10s %5d\n", mdef_ciphone_str(mdef, c2), p);
	    }
	}
    }
}
#endif


/*
 * Utility function for building cross-word pid maps.  Compresses cross-word pid list
 * to unique ones.
 */
static int32 xwdpid_compress (s3pid_t p, s3pid_t *pid, s3cipid_t *map, s3cipid_t ctx,
			      int32 n)
{
    s3senid_t *senmap, *prevsenmap;
    int32 s;
    s3cipid_t i;
    
    senmap = mdef->phone[p].state;
    
    for (i = 0; i < n; i++) {
	if (mdef->phone[p].tmat != mdef->phone[pid[i]].tmat)
	    continue;

	prevsenmap = mdef->phone[pid[i]].state;
	for (s = 0; (s < n_state-1) && (senmap[s] == prevsenmap[s]); s++);

	if (s == n_state-1) {
	    /* This state sequence same as a previous ones; just map to it */
	    map[ctx] = i;
	    return n;
	}
    }

    /* This state sequence different from all previous ones; allocate new entry */
    map[ctx] = n;
    pid[n] = p;
    
    return (n+1);
}


/* Temporary array used during the creation of lexical triphones lists */
static s3pid_t *tmp_xwdpid = NULL;


/*
 * Given base b, and right context rc, build left context cross-word triphones map
 * for all left context ciphones.  Compress map to unique list.
 */
static void build_lcpid (s3cipid_t b, s3cipid_t rc)
{
    s3cipid_t lc, *map;
    s3pid_t p;
    int32 n;
    
    map = (s3cipid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3cipid_t));
    
    n = 0;
    for (lc = 0; lc < mdef->n_ciphone; lc++) {
	p = mdef_phone_id_nearest (mdef, b, lc, rc, WORD_POSN_BEGIN);
	if ((! mdef->ciphone[b].filler) && word_end_ci[lc] &&
	    mdef_is_ciphone(mdef, p))
	    n_backoff_ci++;
	
	n = xwdpid_compress (p, tmp_xwdpid, map, lc, n);
    }

    /* Copy/Move to lcpid */
    lcpid[b][rc].cimap = map;
    lcpid[b][rc].n_pid = n;
    lcpid[b][rc].pid = (s3pid_t *) ckd_calloc (n, sizeof(s3pid_t));
    memcpy (lcpid[b][rc].pid, tmp_xwdpid, n*sizeof(s3pid_t));
}


/*
 * Given base b, and left context lc, build right context cross-word triphones map
 * for all right context ciphones.  Compress map to unique list.
 */
static void build_rcpid (s3cipid_t b, s3cipid_t lc)
{
    s3cipid_t rc, *map;
    s3pid_t p;
    int32 n;
    
    map = (s3cipid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3cipid_t));
    
    n = 0;
    for (rc = 0; rc < mdef->n_ciphone; rc++) {
	p = mdef_phone_id_nearest (mdef, b, lc, rc, WORD_POSN_END);
	if ((! mdef->ciphone[b].filler) && word_start_ci[rc] &&
	    mdef_is_ciphone(mdef, p))
	    n_backoff_ci++;

	n = xwdpid_compress (p, tmp_xwdpid, map, rc, n);
    }

    /* Copy/Move to rcpid */
    rcpid[b][lc].cimap = map;
    rcpid[b][lc].n_pid = n;
    rcpid[b][lc].pid = (s3pid_t *) ckd_calloc (n, sizeof(s3pid_t));
    memcpy (rcpid[b][lc].pid, tmp_xwdpid, n*sizeof(s3pid_t));
}


/*
 * Given base b for a single-phone word, build context cross-word triphones map
 * for all left and right context ciphones.
 */
static void build_lrcpid (s3cipid_t b)
{
    s3cipid_t rc, lc;
    
    for (lc = 0; lc < mdef->n_ciphone; lc++) {
	lrcpid[b][lc].pid = (s3pid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3pid_t));
	lrcpid[b][lc].cimap = (s3cipid_t *) ckd_calloc (mdef->n_ciphone,
							sizeof(s3cipid_t));
	
	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    lrcpid[b][lc].cimap[rc] = rc;
	    lrcpid[b][lc].pid[rc] = mdef_phone_id_nearest (mdef, b, lc, rc,
							   WORD_POSN_SINGLE);
	    if ((! mdef->ciphone[b].filler) &&
		word_start_ci[rc] && word_end_ci[lc] &&
		mdef_is_ciphone(mdef, lrcpid[b][lc].pid[rc]))
		n_backoff_ci++;
	}
	lrcpid[b][lc].n_pid = mdef->n_ciphone;
    }
}


/*
 * Build within-word triphones sequence for each word.  The extreme ends are not needed
 * since cross-word modelling is used for those.  (See lcpid, rcpid, lrcpid.)
 */
static void build_wwpid ( void )
{
    s3wid_t w;
    int32 pronlen, l;
    s3cipid_t b, lc, rc;
    
    E_INFO ("Building within-word triphones\n");
    
    wwpid = (s3pid_t **) ckd_calloc (dict->n_word, sizeof(s3pid_t *));
    for (w = 0; w < dict->n_word; w++) {
	pronlen = dict->word[w].pronlen;
	if (pronlen >= 3)
	    wwpid[w] = (s3pid_t *) ckd_calloc (pronlen-1, sizeof(s3pid_t));
	else
	    continue;
	
	lc = dict->word[w].ciphone[0];
	b = dict->word[w].ciphone[1];
	for (l = 1; l < pronlen-1; l++) {
	    rc = dict->word[w].ciphone[l+1];
	    wwpid[w][l] = mdef_phone_id_nearest (mdef, b, lc, rc, WORD_POSN_INTERNAL);
	    if ((! mdef->ciphone[b].filler) && mdef_is_ciphone(mdef, wwpid[w][l]))
		n_backoff_ci++;
	    
	    lc = b;
	    b = rc;
	}
#if 0
	printf ("%-25s ", dict->word[w].word);
	for (l = 1; l < pronlen-1; l++)
	    printf (" %5d", wwpid[w][l]);
	printf ("\n");
#endif
    }
}


/*
 * Build cross-word triphones map for the entire dictionary.
 */
static void build_xwdpid_map ( void )
{
    s3wid_t w;
    int32 pronlen;
    s3cipid_t b, lc, rc;
    
    E_INFO ("Building cross-word triphones\n");
    
    word_start_ci = (int8 *) ckd_calloc (mdef->n_ciphone, sizeof(int8));
    word_end_ci = (int8 *) ckd_calloc (mdef->n_ciphone, sizeof(int8));

    /* Mark word beginning and ending ciphones that occur in given dictionary */
    for (w = 0; w < dict->n_word; w++) {
	word_start_ci[dict->word[w].ciphone[0]] = 1;
	word_end_ci[dict->word[w].ciphone[dict->word[w].pronlen-1]] = 1;
    }
    
    lcpid = (xwdpid_t **) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t *));
    rcpid = (xwdpid_t **) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t *));
    lrcpid = (xwdpid_t **) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t *));

    for (w = 0; w < dict->n_word; w++) {
	pronlen = dict->word[w].pronlen;
	if (pronlen > 1) {
	    /* Multi-phone word; build rcmap and lcmap if not already present */
	    b = dict->word[w].ciphone[pronlen-1];
	    lc = dict->word[w].ciphone[pronlen-2];
	    if (! rcpid[b])
		rcpid[b] = (xwdpid_t *) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t));
	    if (! rcpid[b][lc].cimap)
		build_rcpid (b, lc);
	    
	    b = dict->word[w].ciphone[0];
	    rc = dict->word[w].ciphone[1];
	    if (! lcpid[b])
		lcpid[b] = (xwdpid_t *) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t));
	    if (! lcpid[b][rc].cimap)
		build_lcpid (b, rc);
	} else {
	    /* Single-phone word; build lrcmap if not already present */
	    b = dict->word[w].ciphone[0];
	    if (! lrcpid[b]) {
		lrcpid[b] = (xwdpid_t *) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t));
		build_lrcpid (b);
	    }
	}
    }

    ckd_free (word_start_ci);
    ckd_free (word_end_ci);

#if 0
    E_INFO ("LCXWDPID\n");
    dump_xwdpidmap (lcpid);
    
    E_INFO ("RCXWDPID\n");
    dump_xwdpidmap (rcpid);

    E_INFO ("LRCXWDPID\n");
    dump_xwdpidmap (lrcpid);
#endif
}


static s3cipid_t *get_rc_cimap (s3wid_t w)
{
    int32 pronlen;
    s3cipid_t b, lc;
    
    pronlen = dict->word[w].pronlen;
    b = dict->word[w].ciphone[pronlen-1];
    if (pronlen == 1) {
	/* No known left context.  But all cimaps (for any l) are identical; pick one */
	return (lrcpid[b][0].cimap);
    } else {
	lc = dict->word[w].ciphone[pronlen-2];
	return (rcpid[b][lc].cimap);
    }
}


static void get_rcpid (s3wid_t w, s3pid_t **pid, int32 *npid)
{
    int32 pronlen;
    s3cipid_t b, lc;
    
    pronlen = dict->word[w].pronlen;
    assert (pronlen > 1);
    
    b = dict->word[w].ciphone[pronlen-1];
    lc = dict->word[w].ciphone[pronlen-2];
    
    *pid = rcpid[b][lc].pid;
    *npid = rcpid[b][lc].n_pid;
}


static int32 get_rc_npid (s3wid_t w)
{
    int32 pronlen;
    s3cipid_t b, lc;
    
    pronlen = dict->word[w].pronlen;
    b = dict->word[w].ciphone[pronlen-1];
    if (pronlen == 1) {
	/* No known left context.  But all cimaps (for any l) are identical; pick one */
	return (lrcpid[b][0].n_pid);
    } else {
	lc = dict->word[w].ciphone[pronlen-2];
	return (rcpid[b][lc].n_pid);
    }
}


static void lattice_dump (FILE *fp)
{
    int32 i;
    
    for (i = 0; i < n_lat_entry; i++) {
	fprintf (fp, "%6d: %5d %6d %11d %s\n", i,
		 lattice[i].frm, lattice[i].history, lattice[i].score,
		 dict_wordstr (lattice[i].wid));
    }
    fflush (fp);
}


/*
 * There are two sets of whmm freelists.  whmm_freelist[0] for word-initial HMMs
 * that need a separate HMM id every state, and whmm_freelist[1] for non-word-initial
 * HMMs that don't need that.
 */
static whmm_t *whmm_freelist[2] = {NULL, NULL};


static void whmm_free (whmm_t *h)
{
    int32 k;
    
    k = (h->pos == 0) ? 0 : 1;
    
    h->next = whmm_freelist[k];
    whmm_freelist[k] = h;
}


static whmm_t *whmm_alloc (int32 pos)
{
    whmm_t *h;
    int32 k, i, n, s;
    int32 *tmp_scr;
    s3latid_t *tmp_latid;
    s3pid_t *tmp_pid;
    
    k = (pos == 0) ? 0 : 1;

    if (! whmm_freelist[k]) {
	n = 16000/sizeof(whmm_t);	/* HACK!!  Hardwired allocation size */

	whmm_freelist[k] = h = (whmm_t *) ckd_calloc (n, sizeof(whmm_t));
	tmp_scr = (int32 *) ckd_calloc (n_state * n, sizeof(int32));
	tmp_latid = (s3latid_t *) ckd_calloc (n_state * n, sizeof(s3latid_t));
	if (pos == 0)
	    tmp_pid = (s3pid_t *) ckd_calloc (n_state * n, sizeof(s3pid_t));

	for (i = 0; i < n; i++) {
	    h[i].next = &(h[i+1]);

	    h[i].score = tmp_scr;
	    tmp_scr += n_state;
	    
	    h[i].history = tmp_latid;
	    tmp_latid += n_state;

	    /* Allocate pid iff first phone position (for multiplexed left contexts) */
	    if (pos == 0) {
		h[i].pid = tmp_pid;
		tmp_pid += n_state;
	    }
	}
	h[n-1].next = NULL;
    }
    
    h = whmm_freelist[k];
    whmm_freelist[k] = h->next;
    
    for (s = 0; s < n_state; s++) {
	h->score[s] = LOGPROB_ZERO;
	h->history[s] = BAD_LATID;
    }
    h->pos = pos;
    
    return (h);
}


static void dump_whmm (s3wid_t w, whmm_t *h, int32 *senscr)
{
    int32 s;
    s3pid_t p;
    
    printf ("[%4d]", n_frm);
    printf (" [%s]", dict->word[w].word);

    printf (" pos= %d, rc= %d, bestscore= %d\n",
	    h->pos, h->rc, h->bestscore);
    
    printf ("\tscore: ");
    for (s = 0; s < n_state; s++)
	printf (" %12d", h->score[s]);
    printf ("\n");
    
    printf ("\thist:  ");
    for (s = 0; s < n_state; s++)
	printf (" %12d", h->history[s]);
    printf ("\n");
    
    printf ("\tsenscr:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", senscr[mdef->phone[p].state[s]]);
    }
    printf ("\n");
    
    printf ("\ttpself:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s]);
    }
    printf ("\n");
    
    printf ("\ttpnext:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s+1]);
    }
    printf ("\n");
    
    printf ("\ttpskip:");
    for (s = 0; s < n_state-2; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s+2]);
    }
    printf ("\n");
    
    
    if (h->pos == 0) {
	printf ("\tpid:   ");
	for (s = 0; s < n_state-1; s++)
	    printf (" %12d", h->pid[s]);
	printf ("\n");
    }
}


static void dump_all_whmm (int32 *senscr)
{
    s3wid_t w;
    whmm_t *h;
    
    for (w = 0; w < dict->n_word; w++) {
	if (whmm[w]) {
	    for (h = whmm[w]; h; h = h->next)
		dump_whmm (w, h, senscr);
	}
    }
}


static void dump_all_word ( void )
{
    s3wid_t w;
    whmm_t *h;
    int32 last, bestlast;
    
    for (w = 0; w < dict->n_word; w++) {
	if (whmm[w]) {
	    printf ("[%4d] %-24s", n_frm, dict->word[w].word);

	    last = dict->word[w].pronlen - 1;
	    bestlast = (int32) 0x80000000;
	    
	    for (h = whmm[w]; h; h = h->next) {
		if (h->pos < last)
		    printf (" %9d.%2d", -h->score[n_state-1], h->pos);
		else if (bestlast < h->score[n_state-1])
		    bestlast = h->score[n_state-1];
	    }

	    if (bestlast > (int32) 0x80000000)
		printf (" %9d.%2d", -bestlast, last);

	    printf ("\n");
	}
    }
}


/*
 * Check model tprob matrices that they conform to upper-diagonal assumption.
 */
static void chk_tp_uppertri ( void )
{
    int32 i, from, to;
    
    assert (n_state > 0);
    
    /* Check that each tmat is upper-triangular */
    for (i = 0; i < tmat->n_tmat; i++) {
	for (to = 0; to < n_state-1; to++)
	    for (from = to+1; from < n_state-1; from++)
		if (tmat->tp[i][from][to] > LOGPROB_ZERO)
		    E_FATAL("HMM transition matrix not upper triangular\n");
    }
}


/* For partial evaluation of incoming state score (prev state score + senone score) */
static int32 *st_sen_scr;


#if (! ANYHMMTOPO)
/*
 * Like the general eval_nonmpx_whmm and eval_mpx_whmm below, but hardwired for
 * the Sphinx-II 5-state Bakis topology.
 */
static void eval_nonmpx_whmm (s3wid_t w, whmm_t *h, int32 *senscr)
{
    register int32 s0, s1, s2, s3, s4;
    register int32 scr, newscr1, newscr2, bestscr;
    register int32 *tp;
    s3pid_t p;
    s3senid_t *senp;
    
    p = *(h->pid);
    senp = mdef->phone[p].state;
    tp = tmat->tp[mdef->phone[p].tmat][0];	/* HACK!! Assumes tp 2-D data allocated
						   contiguously */
    
    if ((s0 = h->score[0] + senscr[senp[0]]) < LOGPROB_ZERO)
	s0 = LOGPROB_ZERO;
    if ((s1 = h->score[1] + senscr[senp[1]]) < LOGPROB_ZERO)
	s1 = LOGPROB_ZERO;
    if ((s2 = h->score[2] + senscr[senp[2]]) < LOGPROB_ZERO)
	s2 = LOGPROB_ZERO;
    if ((s3 = h->score[3] + senscr[senp[3]]) < LOGPROB_ZERO)
	s3 = LOGPROB_ZERO;
    if ((s4 = h->score[4] + senscr[senp[4]]) < LOGPROB_ZERO)
	s4 = LOGPROB_ZERO;
    
    newscr1 = s4 + tp[29]; /* [4][5] */
    newscr2 = s3 + tp[23]; /* [3][5] */
    if (newscr1 > newscr2) {
	h->score[5] = bestscr = newscr1;
	h->history[5] = h->history[4];
    } else {
	h->score[5] = bestscr = newscr2;
	h->history[5] = h->history[3];
    }

    scr     = s4 + tp[28]; /* [4][4] */
    newscr1 = s3 + tp[22]; /* [3][4] */
    newscr2 = s2 + tp[16]; /* [2][4] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[4] = newscr2;
	    h->history[4] = h->history[2];
	} else
	    h->score[4] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[4] = newscr1;
	    h->history[4] = h->history[3];
	} else
	    h->score[4] = scr;
    }
    if (bestscr < h->score[4])
	bestscr = h->score[4];
    
    scr     = s3 + tp[21]; /* [3][3] */
    newscr1 = s2 + tp[15]; /* [2][3] */
    newscr2 = s1 + tp[9];  /* [1][3] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[3] = newscr2;
	    h->history[3] = h->history[1];
	} else
	    h->score[3] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[3] = newscr1;
	    h->history[3] = h->history[2];
	} else
	    h->score[3] = scr;
    }
    if (bestscr < h->score[3])
	bestscr = h->score[3];
    
    scr     = s2 + tp[14]; /* [2][2] */
    newscr1 = s1 + tp[8];  /* [1][2] */
    newscr2 = s0 + tp[2];  /* [0][2] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[2] = newscr2;
	    h->history[2] = h->history[0];
	} else
	    h->score[2] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[2] = newscr1;
	    h->history[2] = h->history[1];
	} else
	    h->score[2] = scr;
    }
    if (bestscr < h->score[2])
	bestscr = h->score[2];
    
    scr     = s1 + tp[7]; /* [1][1] */
    newscr1 = s0 + tp[1]; /* [0][1] */
    if (newscr1 > scr) {
	h->score[1] = newscr1;
	h->history[1] = h->history[0];
    } else
	h->score[1] = scr;
    if (bestscr < h->score[1])
	bestscr = h->score[1];
    
    h->score[0] = scr = s0 + tp[0];	/* [0][0] */
    if (bestscr < scr)
	bestscr = scr;
    
    h->bestscore = bestscr;
}


static void eval_mpx_whmm (s3wid_t w, whmm_t *h, int32 *senscr)
{
    register int32 s0, s1, s2, s3, s4;
    register int32 *tp0, *tp1, *tp2, *tp3, *tp4;
    register int32 scr, newscr1, newscr2, bestscr;
    s3senid_t *senp;
    s3pid_t p0, p1, p2, p3, p4;

    p0 = h->pid[0];
    p1 = h->pid[1];
    p2 = h->pid[2];
    p3 = h->pid[3];
    p4 = h->pid[4];

    senp = mdef->phone[p0].state;
    if ((s0 = h->score[0] + senscr[senp[0]]) < LOGPROB_ZERO)
	s0 = LOGPROB_ZERO;
    tp0 = tmat->tp[mdef->phone[p0].tmat][0];	/* HACK!! See eval_nonmpx_whmm */

    if (p1 != p0) {
	senp = mdef->phone[p1].state;
	tp1 = tmat->tp[mdef->phone[p1].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp1 = tp0;
    if ((s1 = h->score[1] + senscr[senp[1]]) < LOGPROB_ZERO)
	s1 = LOGPROB_ZERO;

    if (p2 != p1) {
	senp = mdef->phone[p2].state;
	tp2 = tmat->tp[mdef->phone[p2].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp2 = tp1;
    if ((s2 = h->score[2] + senscr[senp[2]]) < LOGPROB_ZERO)
	s2 = LOGPROB_ZERO;

    if (p3 != p2) {
	senp = mdef->phone[p3].state;
	tp3 = tmat->tp[mdef->phone[p3].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp3 = tp2;
    if ((s3 = h->score[3] + senscr[senp[3]]) < LOGPROB_ZERO)
	s3 = LOGPROB_ZERO;

    if (p4 != p3) {
	senp = mdef->phone[p4].state;
	tp4 = tmat->tp[mdef->phone[p4].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp4 = tp3;
    if ((s4 = h->score[4] + senscr[senp[4]]) < LOGPROB_ZERO)
	s4 = LOGPROB_ZERO;
    
    newscr1 = s4 + tp4[29]; /* [4][5] */
    newscr2 = s3 + tp3[23]; /* [3][5] */
    if (newscr1 > newscr2) {
	h->score[5] = bestscr = newscr1;
	h->history[5] = h->history[4];
    } else {
	h->score[5] = bestscr = newscr2;
	h->history[5] = h->history[3];
    }

    scr     = s4 + tp4[28]; /* [4][4] */
    newscr1 = s3 + tp3[22]; /* [3][4] */
    newscr2 = s2 + tp2[16]; /* [2][4] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[4] = newscr2;
	    h->history[4] = h->history[2];
	    h->pid[4] = h->pid[2];
	} else
	    h->score[4] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[4] = newscr1;
	    h->history[4] = h->history[3];
	    h->pid[4] = h->pid[3];
	} else
	    h->score[4] = scr;
    }
    if (bestscr < h->score[4])
	bestscr = h->score[4];
    
    scr     = s3 + tp3[21]; /* [3][3] */
    newscr1 = s2 + tp2[15]; /* [2][3] */
    newscr2 = s1 + tp1[9];  /* [1][3] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[3] = newscr2;
	    h->history[3] = h->history[1];
	    h->pid[3] = h->pid[1];
	} else
	    h->score[3] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[3] = newscr1;
	    h->history[3] = h->history[2];
	    h->pid[3] = h->pid[2];
	} else
	    h->score[3] = scr;
    }
    if (bestscr < h->score[3])
	bestscr = h->score[3];
    
    scr     = s2 + tp2[14]; /* [2][2] */
    newscr1 = s1 + tp1[8];  /* [1][2] */
    newscr2 = s0 + tp0[2];  /* [0][2] */
    if (newscr2 > newscr1) {
	if (newscr2 > scr) {
	    h->score[2] = newscr2;
	    h->history[2] = h->history[0];
	    h->pid[2] = h->pid[0];
	} else
	    h->score[2] = scr;
    } else {
	if (newscr1 > scr) {
	    h->score[2] = newscr1;
	    h->history[2] = h->history[1];
	    h->pid[2] = h->pid[1];
	} else
	    h->score[2] = scr;
    }
    if (bestscr < h->score[2])
	bestscr = h->score[2];
    
    scr     = s1 + tp1[7]; /* [1][1] */
    newscr1 = s0 + tp0[1]; /* [0][1] */
    if (newscr1 > scr) {
	h->score[1] = newscr1;
	h->history[1] = h->history[0];
	h->pid[1] = h->pid[0];
    } else
	h->score[1] = scr;
    if (bestscr < h->score[1])
	bestscr = h->score[1];
    
    h->score[0] = scr = s0 + tp0[0]; /* [0][0] */
    if (bestscr < scr)
	bestscr = scr;
    
    h->bestscore = bestscr;
}

#else

/*
 * Evaluate non-multiplexed word HMM (ie, the entire whmm really represents one
 * phone rather than each state representing a potentially different phone.
 */
static void eval_nonmpx_whmm (s3wid_t w, whmm_t *h, int32 *senscr)
{
    s3pid_t pid;
    s3senid_t *sen;
    int32 **tp;
    int32 to, from, bestfrom;
    int32 newscr, scr, bestscr;
    
    pid = *(h->pid);
    
    sen = mdef->phone[pid].state;
    tp = tmat->tp[mdef->phone[pid].tmat];

    /* Compute previous state-score + observation output prob for each state */
    for (from = n_state-2; from >= 0; --from) {
	if ((st_sen_scr[from] = h->score[from] + senscr[sen[from]]) < LOGPROB_ZERO)
	    st_sen_scr[from] = LOGPROB_ZERO;
    }
    
    /* Evaluate final-state first, which does not have a self-transition */
    to = final_state;
    scr = LOGPROB_ZERO;
    bestfrom = -1;
    for (from = to-1; from >= 0; --from) {
	if ((tp[from][to] > LOGPROB_ZERO) &&
	    ((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
	    scr = newscr;
	    bestfrom = from;
	}
    }
    h->score[to] = scr;
    if (bestfrom > 0)
	h->history[to] = h->history[bestfrom];

    bestscr = scr;

    /* Evaluate all other states, which might have self-transitions */
    for (to = final_state-1; to >= 0; --to) {
	/* Score from self-transition, if any */
	scr = (tp[to][to] > LOGPROB_ZERO) ? st_sen_scr[to] + tp[to][to] : LOGPROB_ZERO;

	/* Scores from transitions from other states */
	bestfrom = -1;
	for (from = to-1; from >= 0; --from) {
	    if ((tp[from][to] > LOGPROB_ZERO) &&
		((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
		scr = newscr;
		bestfrom = from;
	    }
	}

	/* Update new result for state to */
	h->score[to] = scr;
	if (bestfrom >= 0)
	    h->history[to] = h->history[bestfrom];

	if (bestscr < scr)
	    bestscr = scr;
    }

    h->bestscore = bestscr;
}


/* Like eval_nonmpx_whmm, except there's a different pid associated with each state */
static void eval_mpx_whmm (s3wid_t w, whmm_t *h, int32 *senscr)
{
    s3pid_t pid, prevpid;
    s3senid_t *senp;
    int32 **tp;
    int32 to, from, bestfrom;
    int32 newscr, scr, bestscr;
    
    /* Compute previous state-score + observation output prob for each state */
    prevpid = BAD_PID;
    for (from = n_state-2; from >= 0; --from) {
	if ((pid = h->pid[from]) != prevpid) {
	    senp = mdef->phone[pid].state;
	    prevpid = pid;
	}

	if ((st_sen_scr[from] = h->score[from] + senscr[senp[from]]) < LOGPROB_ZERO)
	    st_sen_scr[from] = LOGPROB_ZERO;
    }

    /* Evaluate final-state first, which does not have a self-transition */
    to = final_state;
    scr = LOGPROB_ZERO;
    bestfrom = -1;
    prevpid = BAD_PID;
    for (from = to-1; from >= 0; --from) {
	if ((pid = h->pid[from]) != prevpid) {
	    tp = tmat->tp[mdef->phone[pid].tmat];
	    prevpid = pid;
	}

	if ((tp[from][to] > LOGPROB_ZERO) &&
	    ((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
	    scr = newscr;
	    bestfrom = from;
	}
    }
    h->score[to] = scr;
    if (bestfrom > 0) {
	h->history[to] = h->history[bestfrom];
	h->pid[to] = h->pid[bestfrom];
    }
    
    bestscr = scr;

    /* Evaluate all other states, which might have self-transitions */
    for (to = final_state-1; to >= 0; --to) {
	/* Score from self-transition, if any */
	if ((pid = h->pid[to]) != prevpid) {
	    tp = tmat->tp[mdef->phone[pid].tmat];
	    prevpid = pid;
	}
	scr = (tp[to][to] > LOGPROB_ZERO) ? st_sen_scr[to] + tp[to][to] : LOGPROB_ZERO;

	/* Scores from transitions from other states */
	bestfrom = -1;
	for (from = to-1; from >= 0; --from) {
	    if ((pid = h->pid[from]) != prevpid) {
		tp = tmat->tp[mdef->phone[pid].tmat];
		prevpid = pid;
	    }
	    
	    if ((tp[from][to] > LOGPROB_ZERO) &&
		((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
		scr = newscr;
		bestfrom = from;
	    }
	}

	/* Update new result for state to */
	h->score[to] = scr;
	if (bestfrom >= 0) {
	    h->history[to] = h->history[bestfrom];
	    h->pid[to] = h->pid[bestfrom];
	}
	
	if (bestscr < scr)
	    bestscr = scr;
    }

    h->bestscore = bestscr;
}
#endif


static int32 whmm_eval (int32 *senscr)
{
    int32 best, cf;
    s3wid_t w;
    whmm_t *h, *nexth, *prevh;
    int32 n_mpx, n_nonmpx;
    
    best = LOGPROB_ZERO;
    n_mpx = n_nonmpx = 0;
    cf = n_frm;
    
    for (w = 0; w < dict->n_word; w++) {
	prevh = NULL;

	for (h = whmm[w]; h; h = nexth) {
	    nexth = h->next;

	    if (h->active == cf) {
		if (h->pos == 0) {
		    eval_mpx_whmm (w, h, senscr);
		    n_mpx++;
		} else {
		    eval_nonmpx_whmm (w, h, senscr);
		    n_nonmpx++;
		}

		if (best < h->bestscore)
		    best = h->bestscore;

		prevh = h;
	    } else {
		if (prevh)
		    prevh->next = nexth;
		else
		    whmm[w] = nexth;
		
		whmm_free (h);
	    }
	}
    }

    counter_increment (ctr_mpx_whmm, n_mpx);
    counter_increment (ctr_nonmpx_whmm, n_nonmpx);
    
    return best;
}


/*
 * Record a word exit in word lattice.
 * NOTE: All exits from a single word in a given frame (for different right context
 * ciphones) must occur contiguously.
 */
static void lattice_entry (s3wid_t w, int32 f, whmm_t *h)
{
    s3cipid_t rc, npid;
    
    if ((n_lat_entry <= 0) ||
	(lattice[n_lat_entry-1].wid != w) || (lattice[n_lat_entry-1].frm != f)) {
	/* New lattice entry */
	if (n_lat_entry >= lat_alloc) {
	    printf ("\n");
	    E_INFO("Lattice size(%d) exceeded; increasing to %d\n",
		   lat_alloc, lat_alloc+LAT_ALLOC_INCR);
	    lat_alloc += LAT_ALLOC_INCR;
	    lattice = ckd_realloc (lattice, lat_alloc * sizeof(lattice_t));
	}
	
	lattice[n_lat_entry].wid = w;
	lattice[n_lat_entry].frm = (s3frmid_t) f;
	lattice[n_lat_entry].score = h->score[final_state];
	lattice[n_lat_entry].history = h->history[final_state];

	/* Allocate space for different right context scores */
	npid = get_rc_npid (w);
	assert (npid > 0);

	lattice[n_lat_entry].rcscore = (int32 *) ckd_calloc (npid, sizeof(int32));
	for (rc = 0; rc < npid; rc++)
	    lattice[n_lat_entry].rcscore[rc] = LOGPROB_ZERO;

	n_lat_entry++;
    }

    /* Slight BUG here: each rc can have its own history, but only the best retained!! */
    if (lattice[n_lat_entry-1].score < h->score[final_state]) {
	lattice[n_lat_entry-1].score = h->score[final_state];
	lattice[n_lat_entry-1].history = h->history[final_state];
    }
    lattice[n_lat_entry-1].rcscore[h->rc] = h->score[final_state];
}


/*
 * Get the last two non-filler, non-silence lattice words w0 and w1 (base word-ids),
 * starting from l.  Also, break up any compound words in the history into components
 * and return only the last two.  Finally, take LM context into account if run past
 * the beginning of utt.
 */
static void two_word_history (s3latid_t l, s3wid_t *w0, s3wid_t *w1)
{
    s3latid_t l0, l1;
    int32 n;
    
    for (l1 = l;
	 IS_LATID(l1) && filler_word(lattice[l1].wid);
	 l1 = lattice[l1].history);
    
    if (NOT_LATID(l1)) {
	*w0 = lmcontext_pred[0];
	*w1 = lmcontext_pred[1];
	return;
    }

    *w1 = dict_basewid(lattice[l1].wid);
    if ((n = dict->word[*w1].n_comp) > 0) {
	*w0 = dict->word[*w1].comp[n-2].wid;
	*w1 = dict->word[*w1].comp[n-1].wid;
	return;
    }
    
    for (l0 = lattice[l1].history;
	 IS_LATID(l0) && filler_word(lattice[l0].wid);
	 l0 = lattice[l0].history);

    if (NOT_LATID(l0))
	*w0 = lmcontext_pred[1];
    else {
	*w0 = dict_basewid(lattice[l0].wid);
	if ((n = dict->word[*w0].n_comp) > 0)
	    *w0 = dict->word[*w0].comp[n-1].wid;
    }
}


/*
 * Transition from hmm h into the next appropriate one for word w.
 * Threshold check for incoming score already completed.
 * The next HMM may be the last triphone for the word w, in which case, instantiate
 * multiple instances corresponding cross-word triphone modelling for all right context
 * ciphones.
 */
static void whmm_transition (int32 w, whmm_t *h)
{
    int32 lastpos, npid, nf, lscr;
    whmm_t *nexth, *prevh;
    s3cipid_t rc;
    s3pid_t *pid;
    int32 i;
    s3wid_t bw0, bw1;
    
    lastpos = dict->word[w].pronlen - 1;
    nf = n_frm+1;
    
    /*
     * If this is a compound word, compute LM score to be added if an internal word
     * boundary is being crossed.
     */
    lscr = 0;
    if (dict->word[w].n_comp > 0) {
	assert (w != startwid);
	
	for (i = 1; i < dict->word[w].n_comp; i++) {
	    if (dict->word[w].comp[i].pronoff == h->pos+1) {
		if (i >= 2)
		    lscr = lm_tg_score (dict->word[w].comp[i-2].wid,
					dict->word[w].comp[i-1].wid,
					dict->word[w].comp[i].wid);
		else {
		    /* Find the one word history for h and obtain LM score */
		    assert (IS_LATID(h->history[final_state]));
		    two_word_history (h->history[final_state], &bw0, &bw1);
		    lscr += lm_tg_score (bw1, dict->word[w].comp[i-1].wid,
					 dict->word[w].comp[i].wid);
		}
	    }
	}
    }
    
    if (h->pos < lastpos-1) {
	/*
	 * Transition to word HMM that's not the final one in word.  First, allocate
	 * the HMM if not already present.
	 */
	if ((! h->next) || (h->next->pos != h->pos+1)) {
	    nexth = whmm_alloc (h->pos+1);

	    nexth->pid = &(wwpid[w][nexth->pos]);
	    
	    nexth->next = h->next;
	    h->next = nexth;
	}
	
	/* Transition to next HMM */
	nexth = h->next;
	if (h->score[final_state]+lscr > nexth->score[0]) {
	    nexth->score[0] = h->score[final_state]+lscr;
	    nexth->history[0] = h->history[final_state];
	    nexth->active = nf;		/* Ensure it doesn't get pruned */
	}
    } else {
	/*
	 * Transition to final HMMs in word, with full cross-word rc modelling.  Allocate
	 * all final triphone HMM instances first.
	 */
	prevh = h;
	get_rcpid (w, &pid, &npid);
	
	for (rc = 0; rc < npid; rc++) {
	    if ((! prevh->next) || (prevh->next->rc != rc)) {
		nexth = whmm_alloc (h->pos+1);

		nexth->rc = rc;
		nexth->pid = &(pid[rc]);
		
		nexth->next = prevh->next;
		prevh->next = nexth;
	    }
	    prevh = prevh->next;
	}

	/* Transition to next HMMs */
	for (rc = 0, nexth = h->next; rc < npid; rc++, nexth = nexth->next) {
	    if (h->score[final_state]+lscr > nexth->score[0]) {
		nexth->score[0] = h->score[final_state]+lscr;
		nexth->history[0] = h->history[final_state];
		nexth->active = nf;	/* Ensure it doesn't get pruned */
	    }
	}
    }
}


static void whmm_exit (int32 thresh, int32 wordthresh)
{
    s3wid_t w;
    whmm_t *h;
    int32 pronlen, nf;
    
    nf = n_frm+1;

    for (w = 0; w < dict->n_word; w++) {
	pronlen = dict->word[w].pronlen;

	for (h = whmm[w]; h; h = h->next) {
	    if (h->bestscore >= thresh) {
		if (h->pos == pronlen-1) {
		    if (h->score[final_state] >= wordthresh)
			lattice_entry (w, n_frm, h);
		} else {
		    if (h->score[final_state]+phone_penalty >= thresh)
			whmm_transition (w, h);
		}

		h->active = nf;
	    }
	}
    }
}


/*
 * Transition into a word w.  Since we transition into the first phone position, the
 * triphone model must be derived from the incoming left context ciphone.  The first
 * state of the whmm instance inherits this triphone model and propagates it along with
 * the score.
 * If the first phone is also the last (single-phone word), we must also model all
 * possible right context ciphones, by instantiating separate whmm models for each rc.
 */
static void word_enter (s3wid_t w, int32 score, s3latid_t l, s3cipid_t lc)
{
    whmm_t *h, *prevh;
    s3cipid_t b, rc;
    s3pid_t pid, *rpid;
    int32 s, npid, nf;
    
    nf = n_frm+1;
    
    b = dict->word[w].ciphone[0];

    if (dict->word[w].pronlen > 1) {	/* Multi-phone word; no right context problem */
	rc = dict->word[w].ciphone[1];
	pid = lcpid[b][rc].pid[lcpid[b][rc].cimap[lc]];

	if ((! whmm[w]) || (whmm[w]->pos != 0)) {
	    h = whmm_alloc (0);

	    for (s = 0; s < n_state; s++)
		h->pid[s] = pid;
	    
	    h->next = whmm[w];
	    whmm[w] = h;
	}

	h = whmm[w];
	if (score > h->score[0]) {
	    h->score[0] = score;
	    h->history[0] = l;
	    h->pid[0] = pid;
	    h->active = nf;
	}
    } else {
	/* Do all right contexts; first make sure all are allocated */
	prevh = NULL;
	h = whmm[w];
	npid = get_rc_npid (w);
	rpid = lrcpid[b][lc].pid;
	
	for (rc = 0; rc < npid; rc++) {
	    if ((! h) || (h->rc != rc)) {
		h = whmm_alloc (0);

		for (s = 0; s < n_state; s++)
		    h->pid[s] = rpid[rc];
		h->rc = rc;

		if (prevh) {
		    h->next = prevh->next;
		    prevh->next = h;
		} else {
		    h->next = whmm[w];
		    whmm[w] = h;
		}
	    }
	    prevh = h;
	    h = h->next;
	}
	assert (! h);
	
	/* Transition to the allocated HMMs */
	b = dict->word[w].ciphone[0];
	for (rc = 0, h = whmm[w]; rc < npid; rc++, h = h->next) {
	    if (score > h->score[0]) {
		h->score[0] = score;
		h->history[0] = l;
		h->pid[0] = rpid[rc];
		h->active = nf;
	    }
	}
    }
}


/*
 * Backoff node when backing off all the way to unigrams.  Since each word exits with
 * #ciphones different scores (for so many different right contexts), a separate node
 * exists for each context.
 */
typedef struct {
    s3latid_t latid;	/* History entry */
    int32 score;	/* Acoustic + backed off LM score */
    s3cipid_t lc;	/* Last ciphone of history entry, to be used as left context upon
			   entering a new word. */
} backoff_t;
static backoff_t *ug_backoff, *filler_backoff;
static uint8 *tg_trans_done;	/* If tg_trans_done[w] TRUE, trigram transition to w
				   occurred for a given history, and backoff bigram
				   transition from same history should be avoided */
static int32 *rcscore = NULL;	/* rc scores uncompacted; one entry/rc-ciphone */
static s3wid_t *word_cand_cf;	/* BAD_WID terminated array of candidate words for word
				   transition in current frame (if using input word
				   lattices to restrict search). */

/*
 * Unigrams re-organized for faster unigram word transitions.  Words partitioned by
 * their first CI phone and ordered in descending unigram probability within each
 * partition.
 */
typedef struct word_ugprob_s {
    s3wid_t wid;
    int32 ugprob;
    struct word_ugprob_s *next;
} word_ugprob_t;
static word_ugprob_t **word_ugprob;


/*
 * Build array of candidate words that start around the current frame (cf).
 * Note: filler words are not in this list since they are always searched (see
 * word_trans).
 */
static void build_word_cand_cf (int32 cf)
{
    int32 f, sf, ef;
    s3wid_t w, n;
    word_cand_t *candp;
    
    for (w = 0; w < dict->n_word; w++)
	word_cand_cf[w] = 0;
    
    if ((sf = cf - word_cand_win) < 0)
	sf = 0;
    if ((ef = cf + word_cand_win) >= S3_MAX_FRAMES)
	ef = S3_MAX_FRAMES-1;
    
    for (f = sf; f <= ef; f++) {
	for (candp = word_cand[f]; candp; candp = candp->next)
	    word_cand_cf[candp->wid] = 1;
    }

    word_cand_cf[startwid] = 0;	/* Never hypothesized (except at beginning) */
    for (w = dict->filler_start; w <= dict->filler_end; w++)
	word_cand_cf[w] = 0;	/* Handled separately */
    word_cand_cf[finishwid] = 1;	/* Always a candidate */

    n = 0;
    for (w = 0; w < dict->n_word; w++)
	if (word_cand_cf[w])
	    word_cand_cf[n++] = w;
    word_cand_cf[n] = BAD_WID;
}


static void word_trans (int32 thresh)
{
    s3latid_t l;	/* lattice entry index */
    s3cipid_t *rcmap, rc, lc;
    s3wid_t w, bw0, bw1, bw2, nextwid;
    tg_t *tgptr;
    bg_t *bgptr;
    ug_t *ugptr;
    int32 bowt, acc_bowt, newscore;
    int32 n_tg, n_bg, n_ug;
    int32 cand, lscr;
    int32 lat_start;
    
    lat_start = frm_latstart[n_frm];
    
    for (rc = 0; rc < mdef->n_ciphone; rc++) {
	ug_backoff[rc].score = LOGPROB_ZERO;
	filler_backoff[rc].score = LOGPROB_ZERO;
    }
    
    if (n_word_cand > 0)
	build_word_cand_cf (n_frm);
    
    /* Trigram/Bigram word transitions from words just exited */
    for (l = lat_start; l < n_lat_entry; l++) {
	w = lattice[l].wid;
	
	if (w == finishwid)
	    continue;
	
	/* Cross-word left context for new words to which we may transition */
	lc = dict->word[w].ciphone[dict->word[w].pronlen-1];
	
	/* Uncompact path scores for all right context ciphones for word just finished */
	rcmap = get_rc_cimap (w);
	for (rc = 0; rc < mdef->n_ciphone; rc++)
	    rcscore[rc] = lattice[l].rcscore[rcmap[rc]];

	/* Find the last real (non-filler, non-silence) two-word history */
	two_word_history (l, &bw0, &bw1);

	if (n_word_cand <= 0) {
	    /* Transition to all words in vocab */

	    /* Clear trigram transition flag for each word for this history */
	    memset (tg_trans_done, 0, dict->n_word*sizeof(uint8));

	    /* First, transition to trigram followers of bw0, bw1 */
	    acc_bowt = 0;
	    if ((IS_WID(bw0)) && ((n_tg = lm_tglist (bw0, bw1, &tgptr, &bowt)) > 0)) {
		/* Transition to trigram followers of bw0, bw1, if any */
		for (; n_tg > 0; --n_tg, tgptr++) {
		    /* Transition to all alternative pronunciations for trigram follower */
		    nextwid = LM_DICTWID(lm, tgptr->wid);
		    
		    if (IS_WID(nextwid) && (nextwid != startwid)) {
			for (w = nextwid; IS_WID(w); w = dict->word[w].alt) {
			    newscore = rcscore[dict->word[w].ciphone[0]] +
				LM_TGPROB (lm, tgptr) + phone_penalty;
			    
			    if (newscore >= thresh) {
				word_enter (w, newscore, l, lc);
				tg_trans_done[w] = 1;
			    }
			}

			/* Also transition to compound words that begin with nextwid */
			for (w = comp_alt[nextwid]; IS_WID(w); w = comp_alt[w]) {
			    newscore = rcscore[dict->word[w].ciphone[0]] +
				LM_TGPROB (lm, tgptr) + phone_penalty;
			    
			    if (newscore >= thresh) {
				word_enter (w, newscore, l, lc);
				tg_trans_done[w] = 1;
			    }
			}
		    }
		}
		
		acc_bowt = bowt;
	    }
	    
	    /* Transition to bigram followers of bw1 */
	    if ((IS_WID(bw1)) && ((n_bg = lm_bglist (bw1, &bgptr, &bowt)) > 0)) {
		/* Transition to bigram followers of bw1, if any */
		for (; n_bg > 0; --n_bg, bgptr++) {
		    /* Transition to all alternative pronunciations for bigram follower */
		    nextwid = LM_DICTWID (lm, bgptr->wid);
		    
		    if (IS_WID(nextwid) &&
			(! tg_trans_done[nextwid]) &&	/* TG transition already done */
			(nextwid != startwid)) {	/* No transition to <s> */
			for (w = nextwid; IS_WID(w); w = dict->word[w].alt) {
			    newscore = rcscore[dict->word[w].ciphone[0]] +
				LM_BGPROB (lm, bgptr) + acc_bowt + phone_penalty;
			    
			    if (newscore >= thresh)
				word_enter (w, newscore, l, lc);
			}

			/* Also transition to compound words that begin with nextwid */
			for (w = comp_alt[nextwid]; IS_WID(w); w = comp_alt[w]) {
			    newscore = rcscore[dict->word[w].ciphone[0]] +
				LM_BGPROB (lm, bgptr) + acc_bowt + phone_penalty;
			    
			    if (newscore >= thresh)
				word_enter (w, newscore, l, lc);
			}
		    }
		}
		
		acc_bowt += bowt;
	    }
	    
	    /* Update unigram backoff node */
	    for (rc = 0; rc < mdef->n_ciphone; rc++) {
		if (rcscore[rc] <= LOGPROB_ZERO)
		    continue;

		if (rcscore[rc]+acc_bowt+phone_penalty > ug_backoff[rc].score) {
		    ug_backoff[rc].score = rcscore[rc]+acc_bowt+phone_penalty;
		    ug_backoff[rc].latid = l;
		    ug_backoff[rc].lc = lc;
		}
	    }
	} else {
	    /* Transition to words in word_cand_cf */
	    for (cand = 0; IS_WID(word_cand_cf[cand]); cand++) {
		nextwid = word_cand_cf[cand];
		
		/* If a compound word, add LM score for only the first component */
		if (dict->word[nextwid].n_comp > 0)
		    bw2 = dict->word[nextwid].comp[0].wid;
		else
		    bw2 = nextwid;
		lscr = lm_tg_score (bw0, bw1, bw2);
		
		for (w = nextwid; IS_WID(w); w = dict->word[w].alt) {
		    newscore = rcscore[dict->word[w].ciphone[0]] + lscr + phone_penalty;
		    
		    if (newscore >= thresh)
			word_enter (w, newscore, l, lc);
		}
	    }
	}
	
	/* Update filler backoff node */
	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    if (rcscore[rc] <= LOGPROB_ZERO)
		continue;

	    if (rcscore[rc]+phone_penalty > filler_backoff[rc].score) {
		filler_backoff[rc].score = rcscore[rc]+phone_penalty;
		filler_backoff[rc].latid = l;
		filler_backoff[rc].lc = lc;
	    }
	}
    }
    
    /*
     * We have finished transitions to all tg/bg followers of all words just ended.
     * Or, if working from a lattice, transitioned to all words that may start at this
     * point as indicated by the lattice.
     */

    /* Transition to unigrams from backoff nodes (if not working from a lattice) */
    if (n_word_cand <= 0) {
#if 0
	n_ug = lm_uglist (&ugptr);
	for (; n_ug > 0; --n_ug, ugptr++) {
	    for (w = ugptr->dictwid; IS_WID(w); w = dict->word[w].alt) {
		if (w == startwid)
		    continue;
		
		rc = dict->word[w].ciphone[0];
		if (ug_backoff[rc].score >= thresh) {
		    newscore = ug_backoff[rc].score + LM_UGPROB (lm, ugptr);
		    if (newscore >= thresh)
			word_enter (w, newscore, ug_backoff[rc].latid, ug_backoff[rc].lc);
		}
	    }
	}
#else
	word_ugprob_t *wp;
	int32 rcscr;

	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    rcscr = ug_backoff[rc].score;
	    l = ug_backoff[rc].latid;
	    lc = ug_backoff[rc].lc;

	    for (wp = word_ugprob[rc]; wp; wp = wp->next) {
		newscore = rcscr + wp->ugprob;
		if (newscore < thresh)
		    break;
		word_enter (wp->wid, newscore, l, lc);
	    }
	}
#endif
    }

    /*
     * Transition to silence and filler words.  Assume alternative pronunciations
     * are all within filler_start..filler_end
     */
    for (w = dict->filler_start; w <= dict->filler_end; w++) {
	if ((w == startwid) || (w == finishwid))
	    continue;
	
	rc = dict->word[w].ciphone[0];
	if (filler_backoff[rc].score > LOGPROB_ZERO) {
	    newscore = filler_backoff[rc].score + fillpen(dict_basewid(w));
	    if (newscore >= thresh)
		word_enter (w, newscore, filler_backoff[rc].latid, filler_backoff[rc].lc);
	}
    }

    /* Free rcscore here, if necessary to conserve memory space */
}


void fwd_init ( void )
{
    float64 *f64arg;
    float32 *f32arg;
    char *tmpstr;

    E_INFO ("Forward Viterbi Initialization\n");
    
    mdef = mdef_getmdef ();
    tmat = tmat_gettmat ();
    dict = dict_getdict ();
    lm   = lm_current ();
    
    /* HMM states information */
    n_state = mdef->n_emit_state + 1;
    final_state = n_state - 1;

    /* Variables for speeding up whmm evaluation */
    st_sen_scr = (int32 *) ckd_calloc (n_state-1, sizeof(int32));

    /* Some key word ids */
    silwid = dict_wordid (SILENCE_WORD);
    startwid = dict_wordid (START_WORD);
    finishwid = dict_wordid (FINISH_WORD);
    if ((NOT_WID(silwid)) || (NOT_WID(startwid)) || (NOT_WID(finishwid)))
	E_FATAL("%s, %s, or %s missing from dictionary\n",
		SILENCE_WORD, START_WORD, FINISH_WORD);

    /* Beam widths and penalties */
    f64arg = (float64 *) cmd_ln_access ("-beam");
    beam = logs3 (*f64arg);

    f64arg = (float64 *) cmd_ln_access ("-nwbeam");
    wordbeam = logs3 (*f64arg);

    f32arg = (float32 *) cmd_ln_access ("-phonepen");
    phone_penalty = logs3 (*f32arg);

    E_INFO ("logs3(beam)= %d, logs3(nwbeam)= %d\n", beam, wordbeam);
    
    /* Allocate whmm structure */
    whmm = (whmm_t **) ckd_calloc (dict->n_word, sizeof(whmm_t *));

    /* Allocate output word lattice structure */
    lat_alloc = *((int32 *) cmd_ln_access ("-bptblsize"));
    lattice = (lattice_t *) ckd_calloc (lat_alloc, sizeof(lattice_t));
    n_lat_entry = 0;
    
    /* Build cross-word triphone models */
    tmp_xwdpid = (s3pid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3pid_t));

    n_backoff_ci = 0;
    build_wwpid ();
    E_INFO("%d within-word triphone instances mapped to CI-phones\n", n_backoff_ci);

    n_backoff_ci = 0;
    build_xwdpid_map ();
    E_INFO("%d cross-word triphones mapped to CI-phones\n", n_backoff_ci);

    ckd_free (tmp_xwdpid);

    /* Data structures needed during word transition */
    rcscore = (int32 *) ckd_calloc (mdef->n_ciphone, sizeof(int32));
    ug_backoff = (backoff_t *) ckd_calloc (mdef->n_ciphone, sizeof(backoff_t));
    filler_backoff = (backoff_t *) ckd_calloc (mdef->n_ciphone, sizeof(backoff_t));
    tg_trans_done = (uint8 *) ckd_calloc (dict->n_word, sizeof(uint8));
    
    /* Check transition matrices for upper-triangularity */
    chk_tp_uppertri ();

    /* Input candidate-word lattices information to restrict search; if any */
    word_cand_dir = (char *) cmd_ln_access ("-inlatdir");
    latfile_ext = (char *) cmd_ln_access ("-latext");
    word_cand_win = *((int32 *) cmd_ln_access ("-inlatwin"));
    if (word_cand_win < 0) {
	E_ERROR("Invalid -inlatwin argument: %d; set to 50\n", word_cand_win);
	word_cand_win = 50;
    }
    /* Allocate pointers to lists of word candidates in each frame */
    if (word_cand_dir) {
	word_cand = (word_cand_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(word_cand_t *));
	word_cand_cf = (s3wid_t *) ckd_calloc (dict->n_word+1, sizeof(s3wid_t));
    }

    /* Initialize comp_alt */
    comp_alt = (s3wid_t *) ckd_calloc (dict->n_word, sizeof(s3wid_t));
    {
	int32 w;
	
	for (w = 0; w < dict->n_word; w++)
	    comp_alt[w] = BAD_WID;
	for (w = 0; w < dict->n_word; w++) {
	    if (dict->word[w].n_comp > 0) {
		comp_alt[w] = comp_alt[dict->word[w].comp[0].wid];
		comp_alt[dict->word[w].comp[0].wid] = w;
	    }
	}
    }
    
    /* Space for first lattice entry in each frame (+ terminating sentinel) */
    frm_latstart = (s3latid_t *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(s3latid_t));

    /* Allocate timers and counters for statistics gathering */
    ctr_mpx_whmm = counter_new ("mpx");
    ctr_nonmpx_whmm = counter_new ("~mpx");
    ctr_latentry = counter_new ("lat");
    tm_hmmeval = timing_new ();
    tm_hmmtrans = timing_new ();
    tm_wdtrans = timing_new ();

    /* Word to be traced in detail */
    if ((tmpstr = (char *) cmd_ln_access ("-tracewhmm")) != NULL) {
	trace_wid = dict_wordid (tmpstr);
	if (NOT_WID(trace_wid))
	    E_ERROR("%s not in dictionary; cannot be traced\n", tmpstr);
    } else
	trace_wid = BAD_WID;

    /* Active words to be dumped for debugging after the given frame no, if any */
    tmpstr = (char *) cmd_ln_access ("-worddumpsf");
    word_dump_sf = tmpstr ? *((int32 *) tmpstr) : (int32) 0x7ffffff0;
    /* Active HMMs to be dumped for debugging after the given frame no, if any */
    tmpstr = (char *) cmd_ln_access ("-hmmdumpsf");
    hmm_dump_sf = tmpstr ? *((int32 *) tmpstr) : (int32) 0x7ffffff0;

    /* Initialize word_ugprob (assuming there is only one LM) */
    {
	s3wid_t w;
	s3cipid_t ci;
	int32 n_ug, ugprob;
	ug_t *ugptr;
	word_ugprob_t *wp, *prevwp;

	word_ugprob = (word_ugprob_t **) ckd_calloc (mdef->n_ciphone,
						     sizeof(word_ugprob_t *));
	n_ug = lm_uglist (&ugptr);
	for (; n_ug > 0; --n_ug, ugptr++) {
	    w = ugptr->dictwid;
	    if (NOT_WID(w) || (w == startwid))
		continue;
	    
	    ugprob = LM_UGPROB(lm, ugptr);

	    for (; IS_WID(w); w = dict->word[w].alt) {
		ci = dict->word[w].ciphone[0];
		prevwp = NULL;
		for (wp = word_ugprob[ci]; wp && (wp->ugprob >= ugprob); wp = wp->next)
		    prevwp = wp;
		wp = (word_ugprob_t *) listelem_alloc (sizeof(word_ugprob_t));
		wp->wid = w;
		wp->ugprob = ugprob;
		if (prevwp) {
		    wp->next = prevwp->next;
		    prevwp->next = wp;
		} else {
		    wp->next = word_ugprob[ci];
		    word_ugprob[ci] = wp;
		}
	    }

	    /* Also add compound words */
	    for (w = comp_alt[ugptr->dictwid]; IS_WID(w); w = comp_alt[w]) {
		ci = dict->word[w].ciphone[0];
		prevwp = NULL;
		for (wp = word_ugprob[ci]; wp && (wp->ugprob >= ugprob); wp = wp->next)
		    prevwp = wp;
		wp = (word_ugprob_t *) listelem_alloc (sizeof(word_ugprob_t));
		wp->wid = w;
		wp->ugprob = ugprob;
		if (prevwp) {
		    wp->next = prevwp->next;
		    prevwp->next = wp;
		} else {
		    wp->next = word_ugprob[ci];
		    word_ugprob[ci] = wp;
		}
	    }
	}
    }

    /* Initialize bestpath search related */
    dag.list = NULL;

    /* Initialize LM context, corpus */
    if ((tmpstr = (char *) cmd_ln_access ("-lmcontextfn")) != NULL)
	lmcontext_corp = corpus_load_headid (tmpstr);
    else
	lmcontext_corp = NULL;
    lmcontext_pred[0] = BAD_WID;
    lmcontext_pred[1] = startwid;
    lmcontext_succ = finishwid;
}


static int32 cand_compwd_split (char *wd, s3wid_t *wid, int32 max)
{
    int32 i, j, l, n;
    char tmp;
    s3lmwid_t lwid;
    
    l = strlen(wd);
    for (i = 1; (i < l-1) && (wd[i] != '_'); i++);
    if (i >= l-1)
	return 0;	/* Not a compound word */
    
    n = 0;
    for (i = 0; i < l; ) {
	for (j = i; (i < l) && (wd[i] != '_'); i++);
	if (j == i) {
	    E_ERROR("Badly formed compound word (%s); ignored\n", wd);
	    return 0;
	}
	
	if (n >= max) {
	    E_ERROR("Compound word (%s) has too many components; ignored\n", wd);
	    return 0;
	}
	
	tmp = wd[i];
	wd[i] = '\0';
	wid[n] = dict_wordid (wd+j);
	wd[i] = tmp;
	
	if (NOT_WID(wid[n])) {
	    E_ERROR("Component word not in dictionary; %s ignored\n", wd);
	    return 0;
	}
	
	lwid = lm_lmwid(wid[n]);
	if (NOT_LMWID(lwid)) {
	    E_ERROR("Component word not in LM; %s ignored\n", wd);
	    return 0;
	}
	
	n++;
	i++;
    }

    return n;
}


static int32 word_cand_add (s3wid_t w, int32 f)
{
    word_cand_t *candp;
    
    /* Check if node not already present; avoid duplicates */
    for (candp = word_cand[f]; candp && (candp->wid != w); candp = candp->next);
    if (candp)
	return 0;	/* Not added */
    
    candp = (word_cand_t *) listelem_alloc (sizeof(word_cand_t));
    candp->wid = w;
    candp->next = word_cand[f];
    word_cand[f] = candp;
    
    return 1;		/* Added */
}


static int32 word_cand_load (FILE *fp)
{
    char line[1024], word[1024];
    int32 i, k, n, nn, sf, fef, lef, df, f, seqno, lineno, nc;
    s3wid_t w[32];
    s3lmwid_t lwid;
    
    /* Skip past Nodes parameter */
    lineno = 0;
    nn = 0;
    word[0] = '\0';
    while (fgets (line, sizeof(line), fp) != NULL) {
	lineno++;
	if ((sscanf (line, "%s %d", word, &nn) == 2) && (strcmp (word, "Nodes") == 0))
	    break;
    }
    if ((strcmp (word, "Nodes") != 0) || (nn <= 0)) {
	E_ERROR("%s: Nodes parameter missing from input lattice\n", uttid);
	return -1;
    }

    n = 0;
    for (i = 0; i < nn; i++) {
	if (fgets (line, 1024, fp) == NULL) {
	    E_ERROR("%s: Incomplete input lattice\n", uttid);
	    return -1;
	}
	lineno++;

	if ((k = sscanf (line, "%d %s %d %d %d", &seqno, word, &sf, &fef, &lef)) != 5) {
	    E_ERROR("%s: Error in lattice, line %d: %s\n", uttid, lineno, line);
	    return -1;
	}
	if (seqno != i) {
	    E_ERROR("%s: Seq# error in lattice, line %d: %s\n", uttid, lineno, line);
	    return -1;
	}
	if ((sf < 0) || (sf >= S3_MAX_FRAMES)) {
	    E_ERROR("%s: Startframe error in lattice, line %d: %s\n", uttid, lineno, line);
	    return -1;
	}
	
	/* Ignore specific pronunciation spec; strip off trailing (...) */
	dict_basestr (word);
	w[0] = dict_wordid (word);

	/* If a compound word, split into components */
	nc = cand_compwd_split (word, w+1, 31);
	assert ((nc == 0) || (nc >= 2));
	
	/* Check if word qualifies as a candidate, given the current dictionary and LM */
	if (IS_WID(w[0])) {
	    lwid = lm_lmwid (w[0]);
	    if ( ((dict->word[w[0]].n_comp == 0) && IS_LMWID(lwid)) ||
		 ((dict->word[w[0]].n_comp > 0) && (nc > 0)) ) {
		/* I.e., if a compound word, all components are in both dict and LM */
		n += word_cand_add (w[0], sf);
	    }
	}
	
	/*
	 * Add components (if any) to candidate list.  Components spread evenly
	 * through the word duration (HACK!!).
	 */
	if (nc > 0) {
	    df = (((fef+lef+1)>>1) - sf + 1)/nc;	/* Frames between components */
	    for (k = 0, f = sf; k < nc; k++, f += df)
		n += word_cand_add (w[k+1], f);
	}
    }
    
    return n;
}


static void word_cand_free ( void )
{
    word_cand_t *candp, *next;
    int32 f;
    
    for (f = 0; f < S3_MAX_FRAMES; f++) {
	for (candp = word_cand[f]; candp; candp = next) {
	    next = candp->next;
	    listelem_free ((char *)candp, sizeof(word_cand_t));
	}

	word_cand[f] = NULL;
    }

    n_word_cand = 0;
}


/*
 * Begin forward Viterbi search of one utterance
 */
void fwd_start_utt (char *id)
{
    int32 l, ispipe;
    char str[1024];
    FILE *fp;
    
    timing_reset (tm_hmmeval);
    timing_reset (tm_hmmtrans);
    timing_reset (tm_wdtrans);
    
    if (uttid)
	ckd_free (uttid);
    uttid = ckd_salloc (id);
    
    /* Free rcscores for each lattice entry */
    for (l = 0; l < n_lat_entry; l++) {
	if (lattice[l].rcscore) {
	    ckd_free (lattice[l].rcscore);
	    lattice[l].rcscore = NULL;
	}
    }
    n_lat_entry = 0;

    /* If input lattice file containing word candidates to be searched specified; use it */
    if (word_cand_dir) {
	sprintf (str, "%s/%s.%s", word_cand_dir, id, latfile_ext);
	E_INFO("Reading input lattice: %s\n", str);
	
	if ((fp = fopen_compchk (str, &ispipe)) == NULL)
	    E_ERROR("fopen_compchk(%s) failed; running full search\n", str);
	else {
	    if ((n_word_cand = word_cand_load (fp)) <= 0) {
		E_ERROR("Bad or empty lattice file: %s; ignored\n", str);
		word_cand_free ();
	    } else
		E_INFO("%d lattice entries read\n", n_word_cand);

	    fclose_comp (fp, ispipe);
	}
    }
    
    /* If LM context available, load it */
    if (lmcontext_corp)
	lmcontext (lmcontext_corp, id, lmcontext_pred, &lmcontext_succ);
    else {
	/* See initialization in fwd_init() */
    }

    /* Start search from silwid */
    n_frm = -1;	/* Since word_enter transitions to "NEXT" frame */
    word_enter (silwid, fillpen(silwid), BAD_LATID,
		dict->word[silwid].ciphone[dict->word[silwid].pronlen-1]);
    n_frm = 0;
    
    renormalized = 0;
}


void fwd_sen_active (int8 *senlist, int32 n_sen)
{
    s3wid_t w;
    whmm_t *h;
    int32 sen, st;
    s3pid_t p;
    s3senid_t *senp;
    
    for (sen = 0; sen < n_sen; sen++)
	senlist[sen] = 0;
    
    /* Flag active senones */
    for (w = 0; w < dict->n_word; w++) {
	for (h = whmm[w]; h; h = h->next) {
	    if (h->pos == 0) {
		for (st = n_state-2; st >= 0; --st) {
		    p = h->pid[st];
		    senp = mdef->phone[p].state;
		    senlist[senp[st]] = 1;
		}
	    } else {
		p = *(h->pid);
		senp = mdef->phone[p].state;
		for (st = n_state-2; st >= 0; --st)
		    senlist[senp[st]] = 1;
	    }
	}
    }
}


static void whmm_renorm (int32 bestscr)
{
    s3wid_t w;
    whmm_t *h;
    int32 st;
    
    renormalized = 1;
    
    for (w = 0; w < dict->n_word; w++)
	for (h = whmm[w]; h; h = h->next)
	    for (st = n_state-2; st >= 0; --st)
		h->score[st] -= bestscr;
}


int32 fwd_frame (int32 *senscr)
{
    int32 bestscr;	/* Best state score for any whmm evaluated in this frame */
    int32 whmm_thresh;	/* Threshold for any whmm to stay alive in search */
    int32 word_thresh;	/* Threshold for a word-final whmm to succeed */
    whmm_t *h;
    
    timing_start (tm_hmmeval);
    bestscr = whmm_eval (senscr);
    timing_stop (tm_hmmeval);

    whmm_thresh = bestscr + beam;
    word_thresh = bestscr + wordbeam;
    
    /* Dump bestscore and pruning thresholds if any detailed tracing specified */
    if ((hmm_dump_sf < n_frm) || (word_dump_sf < n_frm) ||
	(IS_WID(trace_wid) && whmm[trace_wid])) {
	printf ("[%4d]: >>>> bestscore= %11d, whmm-thresh= %11d, word-thresh= %11d\n",
		n_frm, bestscr, whmm_thresh, word_thresh);
    }
    
    /* Dump all active HMMs or words, if indicated */
    if (hmm_dump_sf < n_frm)
	dump_all_whmm (senscr);
    else if (word_dump_sf < n_frm)
	dump_all_word ();
    
    /* Trace active HMMs for specified word, if any */
    if (IS_WID(trace_wid)) {
	for (h = whmm[trace_wid]; h; h = h->next)
	    dump_whmm (trace_wid, h, senscr);
    }

    timing_start (tm_hmmtrans);
    frm_latstart[n_frm] = n_lat_entry;
    whmm_exit (whmm_thresh, word_thresh);
    timing_stop (tm_hmmtrans);

    timing_start (tm_wdtrans);
    if (frm_latstart[n_frm] < n_lat_entry)
	word_trans (whmm_thresh);
    timing_stop (tm_wdtrans);
    
    if (bestscr < RENORM_THRESH) {
	E_INFO("Frame %d: bestscore= %d; renormalizing\n", n_frm, bestscr);
	whmm_renorm (bestscr);
    }
    
    n_frm++;

    return bestscr;
}


/*
 * Find path score for lattice entry l for the given right context word.
 * If context word is BAD_WID it's a wild card; return the best path score.
 */
static int32 lat_pscr_rc (s3latid_t l, s3wid_t w_rc)
{
    s3cipid_t *rcmap, rc;
    
    if ((NOT_WID(w_rc)) || (! lattice[l].rcscore))
	return lattice[l].score;
    
    rcmap = get_rc_cimap (lattice[l].wid);
    rc = dict->word[w_rc].ciphone[0];
    return (lattice[l].rcscore[rcmap[rc]]);
}


/*
 * Find LM score for transition into lattice entry l.
 */
static int32 lat_seg_lscr (s3latid_t l)
{
    s3wid_t bw0, bw1, bw2;
    int32 lscr, bowt, bo_lscr, t_lscr;
    tg_t *tgptr;
    bg_t *bgptr;
    
    bw2 = dict_basewid (lattice[l].wid);
    
    if (NOT_LATID(lattice[l].history))
	assert (bw2 == silwid);
    
    if (filler_word (bw2))
	return (fillpen(bw2));
    
    two_word_history (lattice[l].history, &bw0, &bw1);
    lscr = lm_tg_score (bw0, bw1, bw2);

    /* Correction for backoff case if that scores better (see word_trans) */
    if (n_word_cand > 0)
	return lscr;		/* No correction needed */
    bo_lscr = 0;
    if ((IS_WID(bw0)) && (lm_tglist (bw0, bw1, &tgptr, &bowt) > 0))
	bo_lscr = bowt;
    if ((IS_WID(bw1)) && (lm_bglist (bw1, &bgptr, &bowt) > 0))
	bo_lscr += bowt;

    if (dict->word[bw2].n_comp == 0) {
	bo_lscr += lm_ug_score (bw2);
	if (bo_lscr > lscr)
	    lscr = bo_lscr;
    } else {
	bw2 = dict->word[bw2].comp[0].wid;
	t_lscr = lm_tg_score (bw0, bw1, bw2);
	bo_lscr += lm_ug_score (bw2);
	if (bo_lscr > t_lscr)
	    lscr += (bo_lscr - t_lscr);
    }

    return lscr;
}


/*
 * Find acoustic and LM score for segmentation corresponding to lattice entry l with
 * the given right context word.
 */
static void lat_seg_ascr_lscr (s3latid_t l, s3wid_t w_rc, int32 *ascr, int32 *lscr)
{
    int32 start_score, end_score;

    /* Score with which l ended with right context = w_rc */
    if ((end_score = lat_pscr_rc (l, w_rc)) <= LOGPROB_ZERO) {
	*ascr = *lscr = LOGPROB_ZERO;
	return;
    }
    
    /* Score with which l was begun */
    start_score = IS_LATID(lattice[l].history) ?
	lat_pscr_rc (lattice[l].history, lattice[l].wid) : 0;

    /* LM score for the transition into l */
    *lscr = lat_seg_lscr (l);
    *ascr = end_score - start_score - *lscr;
}


static hyp_t *lattice_backtrace (s3latid_t l, s3wid_t w_rc)
{
    hyp_t *h, *prevh;

    if (IS_LATID(l)) {
	prevh = lattice_backtrace (lattice[l].history, lattice[l].wid);
	
	h = (hyp_t *) listelem_alloc (sizeof(hyp_t));
	if (! prevh)
	    hyp = h;
	else
	    prevh->next = h;
	h->next = NULL;
	
	h->wid = lattice[l].wid;
	h->word = dict_wordstr(h->wid);
	h->sf = prevh ? prevh->ef+1 : 0;
	h->ef = lattice[l].frm;
	h->pscr = lattice[l].score;
	lat_seg_ascr_lscr (l, w_rc, &(h->ascr), &(h->lscr));

	return h;
    } else {
	return NULL;
    }
}


static s3latid_t lat_final_entry ( void )
{
    s3latid_t l, bestl;
    int32 f, bestscore;
    
    /* Find lattice entry in last frame for FINISH_WORD */
    for (l = frm_latstart[n_frm-1]; l < n_lat_entry; l++)
	if (dict_basewid(lattice[l].wid) == finishwid)
	    break;
    if (l < n_lat_entry) {
	/* FINISH_WORD entry found; backtrack to obtain best Viterbi path */
	return (l);
    }
    
    /* Find last available lattice entry with best ending score */
    E_WARN("%s: Search didn't end in %s\n", uttid, dict_wordstr(finishwid));
    bestscore = LOGPROB_ZERO;
    for (f = n_frm-1; (f >= 0) && (bestscore <= LOGPROB_ZERO); --f) {
	for (l = frm_latstart[f]; l < frm_latstart[f+1]; l++) {
	    if ((lattice[l].wid != startwid) && (bestscore < lattice[l].score)) {
		bestscore = lattice[l].score;
		bestl = l;
	    }
	}
    }
    return ((f >= 0) ? bestl : BAD_LATID);
}


hyp_t *fwd_end_utt ( void )
{
    whmm_t *h, *nexth;
    s3wid_t w;
    s3latid_t l;

    frm_latstart[n_frm] = n_lat_entry;	/* Add sentinel */
    counter_increment (ctr_latentry, n_lat_entry);
    
    /* Free whmm search structures */
    for (w = 0; w < dict->n_word; w++) {
	for (h = whmm[w]; h; h = nexth) {
	    nexth = h->next;
	    whmm_free (h);
	}
	whmm[w] = NULL;
    }

    /* Free word candidates */
    if (n_word_cand > 0)
	word_cand_free ();

    /* Get rid of old hyp, if any */
    hyp_free ();
    
    /* Check if bptable should be dumped (for debugging) */
    {
	int32 k;
	
	k = *((int32 *) cmd_ln_access ("-bptbldump"));
	if (k)
	    lattice_dump (stdout);
    }

    /* Backtrack through lattice for Viterbi result */
    l = lat_final_entry ();
    if (NOT_LATID(l))
	E_ERROR("%s: NO RECOGNITION\n", uttid);
    else
	lattice_backtrace (l, BAD_WID);	/* BAD_WID => Any right context */

    /* Note final lattice entry in DAG structure */
    dag.latfinal = l;

    return hyp;
}


void fwd_timing_dump (float64 tot)
{
    printf ("[H %6.2fx %2d%%]",
	    tm_hmmeval->t_cpu * 100.0 / n_frm, (int32)(tm_hmmeval->t_cpu * 100.0 / tot));
    printf ("[XH %6.2fx %2d%%]",
	    tm_hmmtrans->t_cpu * 100.0 / n_frm, (int32)(tm_hmmtrans->t_cpu * 100.0 / tot));
    printf ("[XW %6.2fx %2d%%]",
	    tm_wdtrans->t_cpu * 100.0 / n_frm, (int32)(tm_wdtrans->t_cpu * 100.0 / tot));
}


static void dag_link (dagnode_t *pd, dagnode_t *d, int32 ascr, int32 ef, daglink_t *byp)
{
    daglink_t *l;
    
    /* Link d into successor list for pd */
    if (pd) {	/* Special condition for root node which doesn't have a predecessor */
	l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
	l->node = d;
	l->src = pd;
	l->bypass = byp;	/* This is a FORWARD link!! */
	l->ascr = ascr;
	l->pscr = (int32)0x80000000;
	l->pscr_valid = 0;
	l->history = NULL;
	l->ef = ef;
	l->next = pd->succlist;
	pd->succlist = l;
    }

    /* Link pd into predecessor list for d */
    l = (daglink_t *) listelem_alloc (sizeof(daglink_t));
    l->node = pd;
    l->src = d;
    l->bypass = byp;		/* This is a FORWARD link!! */
    l->ascr = ascr;
    l->pscr = (int32)0x80000000;
    l->pscr_valid = 0;
    l->history = NULL;
    l->ef = ef;
    l->next = d->predlist;
    d->predlist = l;
}


static daglink_t *find_succlink (dagnode_t *src, dagnode_t *dst)
{
    daglink_t *l;

    for (l = src->succlist; l && (l->node != dst); l = l->next);
    return l;
}


static daglink_t *find_predlink (dagnode_t *src, dagnode_t *dst)
{
    daglink_t *l;

    for (l = src->predlist; l && (l->node != dst); l = l->next);
    return l;
}


/* Like dag_link but check if link already exists.  If so, replace if new score better */
static void dag_update_link (dagnode_t *pd, dagnode_t *d, int32 ascr,
			     int32 ef, daglink_t *byp)
{
    daglink_t *l, *r;
    
    l = find_succlink (pd, d);

    if (! l)
	dag_link (pd, d, ascr, ef, byp);
    else if (l->ascr < ascr) {
	r = find_predlink (d, pd);

	assert (r && (r->ascr == l->ascr));
	l->ascr = r->ascr = ascr;
	l->ef = r->ef = ef;
	l->bypass = r->bypass = byp;
    }
}


/*
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
int32 dag_build ( void )
{
    int32 l;
    s3wid_t w;
    int32 sf;
    dagnode_t *d, *pd;
    int32 ascr, lscr;
    s3latid_t latfinal;
    int32 min_ef_range;
    
    dag.list = NULL;
    latfinal = dag.latfinal;
    if (NOT_LATID(latfinal))
	return -1;
    
    /* Min. endframes value that a node must persist for it to be not ignored */
    min_ef_range = *((int32 *) cmd_ln_access ("-min_endfr"));
    
    /* Build DAG nodes list from the lattice */
    for (l = 0; l < n_lat_entry; l++) {
	w = lattice[l].wid;
	sf = LATID2SF(l);
	
	/* Check if node <w,sf> already created */
	for (d = dag.list; d; d = d->alloc_next) {
	    if ((d->wid == w) && (d->sf == sf))
		break;
	}

	if (! d) {
	    d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));

	    d->wid = w;
	    d->sf = sf;
	    d->fef = lattice[l].frm;
	    d->succlist = NULL;
	    d->predlist = NULL;

	    d->alloc_next = dag.list;
	    dag.list = d;
	}
	d->lef = lattice[l].frm;

	lattice[l].dagnode = d;
    }
    
    /* Find initial node */
    for (d = dag.list; d && (d->sf != 0); d = d->alloc_next);
    assert (d);
    assert (d->wid == silwid);
    dag.root = d;
    
    /* Build DAG edges: between nodes satisfying time adjacency */
    for (d = dag.list; d; d = d->alloc_next) {
	/* Skip links to this node if too short lived */
	if ((d != lattice[latfinal].dagnode) && (d->lef - d->fef < min_ef_range-1))
	    continue;
	
	if (d->sf == 0)
	    assert (d->wid == startwid);	/* No predecessors to this */
	else {
	    /* Link from all end points == d->sf-1 to d */
	    for (l = frm_latstart[d->sf-1]; l < frm_latstart[d->sf]; l++) {
		pd = lattice[l].dagnode;	/* Predecessor DAG node */
		
		/* Skip predecessor node under following conditions */
		if (pd->wid == finishwid)	/* BUG: alternative prons for </s>?? */
		    continue;
		if ((pd != dag.root) && (pd->lef - pd->fef < min_ef_range-1))
		    continue;
		
		/*
		 * Find acoustic score for link from pd to d (for lattice entry l
		 * with pd as right context).
		 */
		lat_seg_ascr_lscr (l, d->wid, &ascr, &lscr);
		if (ascr > LOGPROB_ZERO)
		    dag_link (pd, d, ascr, d->sf-1, NULL);
	    }
	}
    }
    
    dag.filler_removed = 0;
    dag.fudged = 0;

    return 0;
}


/*
 * Add "fudge" edges: from node P to another Q if Q starts IN THE SAME FRAME or ONE
 * FRAME EARLIER THAN the first end frame for P.
 */
static void dag_add_fudge_edges (int32 fudge, int32 min_ef_range)
{
    dagnode_t *d, *pd;
    int32 l, ascr, lscr;
    
    assert (fudge > 0);
    
    /* Add "illegal" links that are near misses */
    for (d = dag.list; d; d = d->alloc_next) {
	if (d->lef - d->fef < min_ef_range-1)
	    continue;
	
	/* Links to d from nodes that first ended just when d started */
	for (l = frm_latstart[d->sf]; l < frm_latstart[d->sf+1]; l++) {
	    pd = lattice[l].dagnode;		/* Predecessor DAG node */

	    if ((pd->wid != finishwid) &&
		(pd->fef == d->sf) &&
		(pd->lef - pd->fef >= min_ef_range-1)) {
		lat_seg_ascr_lscr (l, BAD_WID, &ascr, &lscr);
		dag_link (pd, d, ascr, d->sf-1, NULL);
	    }
	}
	
	if (fudge < 2)
	    continue;
	
	/* Links to d from nodes that first ended just BEYOND when d started */
	for (l = frm_latstart[d->sf+1]; l < frm_latstart[d->sf+2]; l++) {
	    pd = lattice[l].dagnode;		/* Predecessor DAG node */

	    if ((pd->wid != finishwid) &&
		(pd->fef == d->sf+1) &&
		(pd->lef - pd->fef >= min_ef_range-1)) {
		lat_seg_ascr_lscr (l, BAD_WID, &ascr, &lscr);
		dag_link (pd, d, ascr, d->sf-1, NULL);
	    }
	}
    }
}


/*
 * Remove filler nodes from DAG by replacing each link TO a filler with links
 * to its successors.
 * lwf = language weight factor to be applied to LM scores.
 */
static void dag_remove_filler_nodes (float64 lwf)
{
    s3latid_t latfinal;
    dagnode_t *d, *pnode, *snode;
    daglink_t *plink, *slink;
    int32 ascr, lscr;
    
    latfinal = dag.latfinal;
    
    /* If Viterbi search terminated in filler word coerce final DAG node to FINISH_WORD */
    if (filler_word (lattice[latfinal].wid))
	lattice[latfinal].dagnode->wid = finishwid;

    /*
     * Remove filler nodes.  In principle, successors can be fillers and the process
     * must be repeated.  But removing fillers in the order in which they appear in
     * dag.list ensures that succeeding fillers have already been eliminated.
     */
    for (d = dag.list; d; d = d->alloc_next) {
	if (! filler_word (d->wid))
	    continue;
	
	/* Replace each link TO d with links to d's successors */
	for (plink = d->predlist; plink; plink = plink->next) {
	    pnode = plink->node;
	    ascr = plink->ascr;
	    ascr += lwf * fillpen(dict_basewid (d->wid));
	    
	    /* Link this predecessor of d to successors of d */
	    for (slink = d->succlist; slink; slink = slink->next) {
		snode = slink->node;

		/* Link only to non-filler successors; fillers have been eliminated */
		if (! filler_word (snode->wid)) {
		    /* Update because a link may already exist */
		    dag_update_link (pnode, snode, ascr + slink->ascr, plink->ef, slink);
		}
	    }
	}
    }

    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link (NULL, dag.root, 0, -1, NULL);
    
    /* Attach a dummy predecessor link from nowhere into final DAG node */
    plink = &(dag.final);
    plink->node = lattice[latfinal].dagnode;
    plink->src = NULL;
    lat_seg_ascr_lscr (latfinal, BAD_WID, &(plink->ascr), &lscr);
    plink->pscr = (int32)0x80000000;
    plink->lscr = 0;
    plink->bypass = NULL;
    plink->history = NULL;
    plink->ef = lattice[latfinal].frm;
    plink->next = NULL;
}


int32 dag_destroy ( void )
{
    dagnode_t *d, *nd;
    daglink_t *l, *nl;
    
    for (d = dag.list; d; d = nd) {
	nd = d->alloc_next;
	
	for (l = d->succlist; l; l = nl) {
	    nl = l->next;
	    listelem_free ((char *)l, sizeof(daglink_t));
	}
	for (l = d->predlist; l; l = nl) {
	    nl = l->next;
	    listelem_free ((char *)l, sizeof(daglink_t));
	}

	listelem_free ((char *)d, sizeof(dagnode_t));
    }

    dag.list = NULL;

    return 0;
}


/*
 * Recursive step in dag_search:  best backward path from src to root beginning with l.
 */
static void dag_bestpath (daglink_t *l,		/* Backward link! */
			  dagnode_t *src,	/* Source node for backward link l */
			  float64 lwf)		/* Language weight multiplication factor */
{
    dagnode_t *d, *pd;
    daglink_t *pl;
    int32 lscr, score;
    
    assert (! l->pscr_valid);
    
    if ((d = l->node) == NULL) {
	/* If no destination at end of l, src is root node.  Recursion termination */
	assert (dict_basewid(src->wid) == startwid);
	l->lscr = 0;
	l->pscr = 0;
	l->pscr_valid = 1;
	l->history = NULL;
	
	return;
    }
    
    /* Search all predecessor links of l */
    for (pl = d->predlist; pl; pl = pl->next) {
	pd = pl->node;
	if (pd && filler_word (pd->wid))	/* Skip filler node */
	    continue;

	/* Evaluate best path along pl if not yet evaluated (recursive step) */
	if (! pl->pscr_valid)
	    dag_bestpath (pl, d, lwf);
	
	/* Accumulated path score along pl->l */
	if (pl->pscr > (int32)0x80000000) {
	    score = pl->pscr + l->ascr;
	    if (pd)
		lscr = lwf * lm_tg_score (dict_basewid(pd->wid),
					    dict_basewid(d->wid),
					    dict_basewid(src->wid));
	    else
		lscr = lwf * lm_bg_score (dict_basewid(d->wid), dict_basewid(src->wid));
	    score += lscr;
	    
	    /* Update best path and score beginning with l */
	    if (score > l->pscr) {
		l->lscr = lscr;
		l->pscr = score;
		l->history = pl;
	    }
	}
    }

#if 0    
    printf ("%s,%d -> %s,%d = %d\n",
	    dict_wordstr (d->wid), d->sf,
	    dict_wordstr (src->wid), src->sf,
	    l->pscr);
#endif

    l->pscr_valid = 1;
}


/*
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
static hyp_t *dag_backtrace (daglink_t *l, float64 lwf)
{
    hyp_t *h, *hhead, *htail;
    int32 pscr;
    dagnode_t *src, *dst;
    daglink_t *bl, *hist;
    
    hyp = NULL;
    
    for (; l; l = hist) {
	hist = l->history;
	
	if (hyp)
	    hyp->lscr = l->lscr;	/* As lscr actually applies to successor node */
	
	if (! l->node) {
	    assert (! l->history);
	    break;
	}
	
	if (! l->bypass) {
	    /* Link did not bypass any filler node */
	    h = (hyp_t *) listelem_alloc (sizeof(hyp_t));
	    h->wid = l->node->wid;
	    h->word = dict_wordstr (h->wid);
	    h->sf = l->node->sf;
	    h->ef = l->ef;
	    h->ascr = l->ascr;
	    
	    h->next = hyp;
	    hyp = h;
	} else {
	    /* Link bypassed one or more filler nodes; restore bypassed link seq. */
	    hhead = htail = NULL;
	    
	    src = l->node;	/* Note that l is a PREDECESSOR link */
	    for (; l; l = l->bypass) {
		h = (hyp_t *) listelem_alloc (sizeof(hyp_t));
		h->wid = src->wid;
		h->word = dict_wordstr (h->wid);
		h->sf = src->sf;

		if (hhead)
		    h->lscr = lwf * fillpen (dict_basewid (src->wid));
		
		if (l->bypass) {
		    dst = l->bypass->src;
		    assert (filler_word (dst->wid));
		    bl = find_succlink (src, dst);
		    assert (bl);
		} else
		    bl = l;

		h->ef = bl->ef;
		h->ascr = bl->ascr;
		if (htail)
		    htail->next = h;
		else
		    hhead = h;
		htail = h;

		src = dst;
	    }

	    htail->next = hyp;
	    hyp = hhead;
	}
    }

    /* Compute path score for each node in hypothesis */
    pscr = 0;
    for (h = hyp; h; h = h->next) {
	pscr = pscr + h->lscr + h->ascr;
	h->pscr = pscr;
    }
    
    return hyp;
}


static int32 dag_chk_linkscr (dag_t *dag)
{
    dagnode_t *d;
    daglink_t *l;
    
    for (d = dag->list; d; d = d->alloc_next) {
	for (l = d->succlist; l; l = l->next) {
	    if (l->ascr >= 0)
		return -1;
	}
    }

    return 0;
}


/*
 * Final global best path through DAG constructed from the word lattice.
 * Assumes that the DAG has already been constructed and is consistent with the word
 * lattice.
 * The search uses a recursive algorithm to find the best (reverse) path from the final
 * DAG node to the root:  The best path from any node (beginning with a particular link L)
 * depends on a similar best path for all links leaving the endpoint of L.  (This is
 * sufficient to handle trigram LMs.)
 */
hyp_t *dag_search (char *utt)
{
    daglink_t *l, *bestl;
    dagnode_t *d, *final;
    int32 bestscore;
    float32 *f32arg;
    float64 lwf;
    int32 fudge, min_ef_range;

    if (renormalized) {
	E_ERROR("Scores renormalized during forward Viterbi; cannot run bestpath\n");
	return NULL;
    }
    
    f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
    lwf = f32arg ? ((*f32arg) / *((float32 *) cmd_ln_access ("-langwt"))) : 1.0;
    
    /* Add fudge edges to DAG if specified */
    if (! dag.fudged) {
	fudge = *((int32 *) cmd_ln_access ("-dagfudge"));
	if (fudge > 0) {
	    min_ef_range = *((int32 *) cmd_ln_access ("-min_endfr"));
	    dag_add_fudge_edges (fudge, min_ef_range);
	}
	dag.fudged = 1;
    }
    
    /* Bypass filler nodes */
    if (! dag.filler_removed) {
	dag_remove_filler_nodes (lwf);
	dag.filler_removed = 1;
    }
    
    /* Find the backward link from the final DAG node that has the best path to root */
    final = lattice[dag.latfinal].dagnode;
    bestscore = (int32) 0x80000000;
    bestl = NULL;

    /* Check that all edge scores are -ve; error if not so. */
    if (dag_chk_linkscr (&dag) < 0)
	return NULL;
    
    for (l = final->predlist; l; l = l->next) {
	d = l->node;
	if (! filler_word (d->wid)) {	/* Ignore filler node */
	    dag_bestpath (l, final, lwf);	/* Best path to root beginning with l */

	    if (l->pscr > bestscore) {
		bestscore = l->pscr;
		bestl = l;
	    }
	}
    }

    if (! bestl)
	return NULL;
    
    /*
     * At this point bestl is the best (reverse) link/path leaving the final node.  But
     * this does not include the acoustic score for the final node itself.  Add it.
     */
    l = &(dag.final);
    l->history = bestl;
    l->pscr = bestl->pscr + l->ascr;
    
    /* Backtrack through DAG for best path; but first free any old hypothesis */
    hyp_free ();
    hyp = dag_backtrace (l, lwf);
    
    return (hyp);
}


int32 dag_dump (char *dir, int32 onlynodes, char *id)
{
    int32 i;
    dagnode_t *d, *initial, *final;
    daglink_t *l;
    char filename[1024], str[1024];
    FILE *fp;
    int32 ascr, lscr;
    float32 logbase;
    int32 ispipe;
    
    initial = dag.root;
    final = lattice[dag.latfinal].dagnode;

    sprintf (filename, "%s/%s.%s", dir, id, latfile_ext);
    E_INFO("Writing lattice file: %s\n", filename);
    if ((fp = fopen_comp (filename, "w", &ispipe)) == NULL) {
	E_ERROR("fopen_comp (%s,w) failed\n", filename);
	return -1;
    }
    
    getcwd (str, sizeof(str));
    fprintf (fp, "# getcwd: %s\n", str);

    /* Print logbase first!!  Other programs look for it early in the DAG */
    logbase = *((float32 *) cmd_ln_access("-logbase"));
    fprintf (fp, "# -logbase %e\n", logbase);

    fprintf (fp, "# -dictfn %s\n", (char *) cmd_ln_access ("-dictfn"));
    if (cmd_ln_access ("-fdictfn"))
	fprintf (fp, "# -fdictfn %s\n", (char *) cmd_ln_access ("-fdictfn"));
    fprintf (fp, "# -lmfn %s\n", (char *) cmd_ln_access ("-lmfn"));
    fprintf (fp, "# -mdeffn %s\n", (char *) cmd_ln_access ("-mdeffn"));
    fprintf (fp, "# -senmgaufn %s\n", (char *) cmd_ln_access ("-senmgaufn"));
    fprintf (fp, "# -meanfn %s\n", (char *) cmd_ln_access ("-meanfn"));
    fprintf (fp, "# -varfn %s\n", (char *) cmd_ln_access ("-varfn"));
    fprintf (fp, "# -mixwfn %s\n", (char *) cmd_ln_access ("-mixwfn"));
    fprintf (fp, "# -tmatfn %s\n", (char *) cmd_ln_access ("-tmatfn"));
    fprintf (fp, "# -min_endfr %d\n", *((int32 *) cmd_ln_access ("-min_endfr")));
    fprintf (fp, "#\n");
    
    fprintf (fp, "Frames %d\n", n_frm);
    fprintf (fp, "#\n");
    
    for (i = 0, d = dag.list; d; d = d->alloc_next, i++);
    fprintf (fp, "Nodes %d (NODEID WORD STARTFRAME FIRST-ENDFRAME LAST-ENDFRAME)\n", i);
    for (i = 0, d = dag.list; d; d = d->alloc_next, i++) {
	d->seqid = i;
	fprintf (fp, "%d %s %d %d %d\n", i, dict_wordstr(d->wid), d->sf, d->fef, d->lef);
    }
    fprintf (fp, "#\n");

    fprintf (fp, "Initial %d\nFinal %d\n", initial->seqid, final->seqid);
    fprintf (fp, "#\n");
    
    /* Best score (i.e., regardless of Right Context) for word segments in word lattice */
    fprintf (fp, "BestSegAscr %d (NODEID ENDFRAME ASCORE)\n", n_lat_entry);
    if (! onlynodes) {
	for (i = 0; i < n_lat_entry; i++) {
	    lat_seg_ascr_lscr (i, BAD_WID, &ascr, &lscr);
	    fprintf (fp, "%d %d %d\n",
		     (lattice[i].dagnode)->seqid, lattice[i].frm, ascr);
	}
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "Edges (FROM-NODEID TO-NODEID ASCORE)\n");
    if (! onlynodes) {
	for (d = dag.list; d; d = d->alloc_next) {
	    for (l = d->succlist; l; l = l->next)
		fprintf (fp, "%d %d %d\n", d->seqid, l->node->seqid, l->ascr);
	}
    }
    fprintf (fp, "End\n");

    fclose_comp (fp, ispipe);

    return 0;
}
