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
 * 28-Jul-04    ARCHAN (archan@cs.cmu.edu at Carnegie Mellon Unversity 
 *              First incorporate it from s3 code base. 
 *
 * $Log$
 * Revision 1.13  2005/10/05  00:31:14  dhdfu
 * Make int8 be explicitly signed (signedness of 'char' is
 * architecture-dependent).  Then make a bunch of things use uint8 where
 * signedness is unimportant, because on the architecture where 'char' is
 * unsigned, it is that way for a reason (signed chars are slower).
 * 
 * Revision 1.12  2005/06/21 22:41:32  arthchan2003
 * Log. 1, Removal of several functions of dag_t, 2, removal of static variable stardwid, finishwid and silwid. They are now all handled by dict.  3, Use the lmset interface (lmset_init). Currently it still doesn't support class-based LM.
 *
 * Revision 1.6  2005/06/19 03:58:16  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.5  2005/06/18 18:17:50  archan
 * Update decode_anytopo such that it also used the lmset interface. Notice it still doesn't support multiple LMs and class-based LM at this point
 *
 * Revision 1.4  2005/06/03 06:45:28  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.3  2005/06/03 05:46:19  archan
 * Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 * Revision 1.2  2005/05/26 22:03:06  archan
 * Add support for backtracking without assuming silence </s> has to be the last word.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:00  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.11  2005/02/09 05:59:30  arthchan2003
 * Sychronize the -option names in slow and faster decoders.  This makes many peopple's lives easier. Also update command-line. make test-full is done.
 *
 * Revision 1.10  2004/12/27 19:46:19  arthchan2003
 * 1, Add perf-std to Makefile.am , developers can type make perf-std as the standard performance test target. This only works in CMU. 2, Fix warning in flat_fwd.[ch], 3, Apply Yitao's change in cmd_ln.c . 4, 2,3 are standard regression tested.
 *
 * Revision 1.9  2004/12/23 21:00:51  arthchan2003
 * 1, Fixed problems in the code of -cepext, 2, Enabled the generic HMM computation routine flat_fwd.c. This is the key problem of the decode_anytopo.
 *
 * Revision 1.8  2004/12/06 10:52:00  arthchan2003
 * Enable doxygen documentation in libs3decoder
 *
 * Revision 1.7  2004/12/05 12:01:30  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.6  2004/11/16 05:13:18  arthchan2003
 * 1, s3cipid_t is upgraded to int16 because we need that, I already check that there are no magic code using 8-bit s3cipid_t
 * 2, Refactor the ep code and put a lot of stuffs into fe.c (should be renamed to something else.
 * 3, Check-in codes of wave2feat and cepview. (cepview will not dump core but Evandro will kill me)
 * 4, Make the same command line frontends for decode, align, dag, astar, allphone, decode_anytopo and ep . Allow the use a file to configure the application.
 * 5, Make changes in test such that test-allphone becomes a repeatability test.
 * 6, cepview, wave2feat and decode_anytopo will not be installed in 3.5 RCIII
 * (Known bugs after this commit)
 * 1, decode_anytopo has strange bugs in some situations that it cannot find the end of the lattice. This is urgent.
 * 2, default argument file's mechanism is not yet supported, we need to fix it.
 * 3, the bug discovered by SonicFoundry is still not fixed.
 *
 * Revision 1.2  2004/11/14 07:00:08  arthchan2003
 * 1, Finally, a version of working flat decoder is completed. It is not compiled in the standard compilation yet because there are two many warnings. 2, eliminate the statics variables in  fe_sigproc.c
 *
 * Revision 1.2  2002/12/03 23:02:38  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 *
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
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
 * 		very poor state scores and flooring them to S3_LOGPROB_ZERO.  Otherwise,
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

#define ANY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include <s3types.h>
#include "mdef.h"
#include "tmat.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "logs3.h"
#include "search.h"

#include "programs/s3_dag.h"

#include "dag.h"
#include "flat_fwd.h"


/** \file flat_fwd.c 
    \brief Implementation of forward search in a flat lexicon. 
 */

/**
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
static xwdpid_t **lcpid;
static xwdpid_t **rcpid;
static xwdpid_t **lrcpid;

static int32 n_backoff_ci;	/* #Triphone instances backed off to ciphones */
static uint8 *word_start_ci;
static uint8 *word_end_ci;
static whmm_t **whmm;


/**
 * First, the within word triphone models.  wwpid[w] = list of triphone pronunciations
 * for word w.
 * Since left and right extremes require cross-word modelling (see below), wwpid[w][0]
 * and wwpid[w][pronlen-1] contain no information and shouldn't be touched.
 */
static s3pid_t **wwpid;

    
/**
 * Word lattice for recording decoded hypotheses.
 * 
 * lattice[i] = entry for a word ending at a particular frame.  There can be at most one
 * entry for a word in a given frame.
 * NOTE: lattice array allocated statically.  Need a more graceful way to grow without
 * such an arbitrary internal limit.
 */
typedef struct lattice_s {
    s3wid_t   wid;	/**< Decoded word */
    s3frmid_t frm;	/**< End frame for this entry */
    s3latid_t history;	/**< Index of predecessor lattice_t entry */
    int32     score;	/**< Best path score upto the end of this entry */
    int32    *rcscore;	/**< Individual path scores for different right context ciphones */
    dagnode_t *dagnode;	/**< DAG node representing this entry */
} lattice_t;
static lattice_t *lattice;
static int32 lat_alloc;		/** #lattice entries allocated */
static int32 n_lat_entry;	/** #lattice entries used at any point */
#define LAT_ALLOC_INCR		32768

#define LATID2SF(l)	(IS_S3LATID(lattice[l].history) ? \
			 lattice[lattice[l].history].frm + 1 : 0)

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
 * Input lattice file format:  Each line contains a single <word> <startframe> info.  The
 * line may contain other info following these two fields; these are ignored.  Empty lines
 * and lines beginning with a # char in the first column (ie, comment lines) are ignored.
 */
static char *word_cand_dir;	/** Directory containing candidate words files.  If NULL,
				   full search performed for entire run */
static char *latfile_ext;	/** Complete word candidate filename for an utterance formed
				   by word_cand_dir/<uttid>.latfile_ext */
static int32 word_cand_win;	/** In frame f, candidate words in input lattice from frames
				   [(f - word_cand_win) .. (f + word_cand_win)] will be
				   the actual candidates to be started(entered) */
typedef struct word_cand_s {
    s3wid_t wid;		/** A particular candidate word starting in a given frame */
    struct word_cand_s *next;	/** Next candidate starting in same frame; NULL if none */
} word_cand_t;
static word_cand_t **word_cand;	/** Word candidates for each frame.  (NOTE!! Another array
				   with a hard limit on its size.) */
static int32 n_word_cand;	/** #candidate entries in word_cand for current utterance.
				   If <= 0; full search performed for current utterance */


/** Various search-related parameters */
static int32 beam;		/** General beamwidth */
static int32 wordbeam;		/** Beam for exiting a word */

static int32 phone_penalty;	/** Applied for each phone transition */

static int32 n_state = 0;
static int32 final_state;

tmat_t *tmat;		/** HMM transition probabilities matrices */

dict_t *dict;		/** The dictionary */
fillpen_t *fpen;        /** Filler penalty */
lmset_t *lmset;         /** The language model set */
lm_t *lm;               /** NOT NICE: This is a pointer for current lm */
mdef_t *mdef;


dag_t dag;              /** The dag used by decode_anytopo.c */

#if 0
lm_t   *lm;		/** The currently active language model */
s3lmwid_t *dict2lmwid;	/** Mapping from decoding dictionary wid's to lm ones.  They may not be the same! */
#endif


static char *uttid = NULL;	/** Utterance id; for error reporting */
static int32 n_frm;		/** Current frame being searched within utt */
static s3latid_t *frm_latstart;	/** frm_latstart[f] = first lattice entry in frame f */

static srch_hyp_t *hyp = NULL;	/** The final recognition result */
static int32 renormalized;	/** Whether scores had to be renormalized in current utt */

/* Debugging */
static s3wid_t trace_wid;	/** Word to be traced; for debugging */
static int32 word_dump_sf;	/** Start frame for words to be dumped for debugging */
static int32 hmm_dump_sf;	/** Start frame for HMMs to be dumped for debugging */

/* Event count statistics */

pctr_t ctr_mpx_whmm;
pctr_t ctr_nonmpx_whmm;
pctr_t ctr_latentry;


static ptmr_t tm_hmmeval;
static ptmr_t tm_hmmtrans;
static ptmr_t tm_wdtrans;




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



/**
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


/** Temporary array used during the creation of lexical triphones lists */
static s3pid_t *tmp_xwdpid = NULL;


/**
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



/**
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



/**
 * Given base b for a single-phone word, build context cross-word triphones map
 * for all left and right context ciphones.
 */
static void build_lrcpid (s3cipid_t b)
{
    s3cipid_t rc, lc;
    
    for (lc = 0; lc < mdef->n_ciphone; lc++) {
	lrcpid[b][lc].pid = (s3pid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3pid_t));
	lrcpid[b][lc].cimap = (s3cipid_t *) ckd_calloc (mdef->n_ciphone,sizeof(s3cipid_t));
							
	
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



/**
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


/**
 * Build cross-word triphones map for the entire dictionary.
 */
static void build_xwdpid_map ( void )
{
    s3wid_t w;
    int32 pronlen;
    s3cipid_t b, lc, rc;
    
    E_INFO ("Building cross-word triphones\n");
    
    word_start_ci = (uint8 *) ckd_calloc (mdef->n_ciphone, sizeof(int8));
    word_end_ci = (uint8 *) ckd_calloc (mdef->n_ciphone, sizeof(int8));

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
		 dict_wordstr (dict,lattice[i].wid));
    }
    fflush (fp);
}


/**
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
    tmp_pid=NULL;
    
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
	h->score[s] = S3_LOGPROB_ZERO;
	h->history[s] = BAD_S3LATID;
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
	if (NOT_S3PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", senscr[mdef->phone[p].state[s]]);
    }
    printf ("\n");
    
    printf ("\ttpself:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_S3PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s]);
    }
    printf ("\n");
    
    printf ("\ttpnext:");
    for (s = 0; s < n_state-1; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_S3PID(p))
	    printf (" %12s", "--");
	else
	    printf (" %12d", tmat->tp[mdef->phone[p].tmat][s][s+1]);
    }
    printf ("\n");
    
    printf ("\ttpskip:");
    for (s = 0; s < n_state-2; s++) {
	p = (h->pos > 0) ? *(h->pid) : h->pid[s];
	if (NOT_S3PID(p))
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


/**
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
		if (tmat->tp[i][from][to] > S3_LOGPROB_ZERO)
		    E_FATAL("HMM transition matrix not upper triangular\n");
    }
}


/** For partial evaluation of incoming state score (prev state score + senone score) */
static int32 *st_sen_scr;

#define ANYHMMTOPO 1
#if (! ANYHMMTOPO)
/**
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
    
    if ((s0 = h->score[0] + senscr[senp[0]]) < S3_LOGPROB_ZERO)
	s0 = S3_LOGPROB_ZERO;
    if ((s1 = h->score[1] + senscr[senp[1]]) < S3_LOGPROB_ZERO)
	s1 = S3_LOGPROB_ZERO;
    if ((s2 = h->score[2] + senscr[senp[2]]) < S3_LOGPROB_ZERO)
	s2 = S3_LOGPROB_ZERO;
    if ((s3 = h->score[3] + senscr[senp[3]]) < S3_LOGPROB_ZERO)
	s3 = S3_LOGPROB_ZERO;
    if ((s4 = h->score[4] + senscr[senp[4]]) < S3_LOGPROB_ZERO)
	s4 = S3_LOGPROB_ZERO;
    
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
    if ((s0 = h->score[0] + senscr[senp[0]]) < S3_LOGPROB_ZERO)
	s0 = S3_LOGPROB_ZERO;
    tp0 = tmat->tp[mdef->phone[p0].tmat][0];	/* HACK!! See eval_nonmpx_whmm */

    if (p1 != p0) {
	senp = mdef->phone[p1].state;
	tp1 = tmat->tp[mdef->phone[p1].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp1 = tp0;
    if ((s1 = h->score[1] + senscr[senp[1]]) < S3_LOGPROB_ZERO)
	s1 = S3_LOGPROB_ZERO;

    if (p2 != p1) {
	senp = mdef->phone[p2].state;
	tp2 = tmat->tp[mdef->phone[p2].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp2 = tp1;
    if ((s2 = h->score[2] + senscr[senp[2]]) < S3_LOGPROB_ZERO)
	s2 = S3_LOGPROB_ZERO;

    if (p3 != p2) {
	senp = mdef->phone[p3].state;
	tp3 = tmat->tp[mdef->phone[p3].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp3 = tp2;
    if ((s3 = h->score[3] + senscr[senp[3]]) < S3_LOGPROB_ZERO)
	s3 = S3_LOGPROB_ZERO;

    if (p4 != p3) {
	senp = mdef->phone[p4].state;
	tp4 = tmat->tp[mdef->phone[p4].tmat][0];/* HACK!! See eval_nonmpx_whmm */
    } else
	tp4 = tp3;
    if ((s4 = h->score[4] + senscr[senp[4]]) < S3_LOGPROB_ZERO)
	s4 = S3_LOGPROB_ZERO;
    
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

/**
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
	if ((st_sen_scr[from] = h->score[from] + senscr[sen[from]]) < S3_LOGPROB_ZERO)
	    st_sen_scr[from] = S3_LOGPROB_ZERO;
    }
    
    /* Evaluate final-state first, which does not have a self-transition */
    to = final_state;
    scr = S3_LOGPROB_ZERO;
    bestfrom = -1;
    for (from = to-1; from >= 0; --from) {
	if ((tp[from][to] > S3_LOGPROB_ZERO) &&
	    ((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
	    scr = newscr;
	    bestfrom = from;
	}
    }
    h->score[to] = scr;
    if (bestfrom >= 0)
	h->history[to] = h->history[bestfrom];

    bestscr = scr;

    /* Evaluate all other states, which might have self-transitions */
    for (to = final_state-1; to >= 0; --to) {
	/* Score from self-transition, if any */
	scr = (tp[to][to] > S3_LOGPROB_ZERO) ? st_sen_scr[to] + tp[to][to] : S3_LOGPROB_ZERO;

	/* Scores from transitions from other states */
	bestfrom = -1;
	for (from = to-1; from >= 0; --from) {
	    if ((tp[from][to] > S3_LOGPROB_ZERO) &&
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


/** Like eval_nonmpx_whmm, except there's a different pid associated with each state */
static void eval_mpx_whmm (s3wid_t w, whmm_t *h, int32 *senscr)
{
    s3pid_t pid, prevpid;
    s3senid_t *senp;
    int32 **tp;
    int32 to, from, bestfrom;
    int32 newscr, scr, bestscr;
    
    senp=NULL;
    tp=NULL;
    /* Compute previous state-score + observation output prob for each state */
    prevpid = BAD_S3PID;
    for (from = n_state-2; from >= 0; --from) {
	if ((pid = h->pid[from]) != prevpid) {
	    senp = mdef->phone[pid].state;
	    prevpid = pid;
	}

	if ((st_sen_scr[from] = h->score[from] + senscr[senp[from]]) < S3_LOGPROB_ZERO)
	    st_sen_scr[from] = S3_LOGPROB_ZERO;
    }

    /* Evaluate final-state first, which does not have a self-transition */
    to = final_state;
    scr = S3_LOGPROB_ZERO;
    bestfrom = -1;
    prevpid = BAD_S3PID;
    for (from = to-1; from >= 0; --from) {
	if ((pid = h->pid[from]) != prevpid) {
	    tp = tmat->tp[mdef->phone[pid].tmat];
	    prevpid = pid;
	}

	if ((tp[from][to] > S3_LOGPROB_ZERO) &&
	    ((newscr = st_sen_scr[from] + tp[from][to]) > scr)) {
	    scr = newscr;
	    bestfrom = from;
	}
    }
    h->score[to] = scr;
    if (bestfrom >= 0) {
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
	scr = (tp[to][to] > S3_LOGPROB_ZERO) ? st_sen_scr[to] + tp[to][to] : S3_LOGPROB_ZERO;

	/* Scores from transitions from other states */
	bestfrom = -1;
	for (from = to-1; from >= 0; --from) {
	    if ((pid = h->pid[from]) != prevpid) {
		tp = tmat->tp[mdef->phone[pid].tmat];
		prevpid = pid;
	    }
	    
	    if ((tp[from][to] > S3_LOGPROB_ZERO) &&
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
    
    best = S3_LOGPROB_ZERO;
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

    pctr_increment (ctr_mpx_whmm, n_mpx);
    pctr_increment (ctr_nonmpx_whmm, n_nonmpx);
    
    return best;
}


/**
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
	    lattice[n_lat_entry].rcscore[rc] = S3_LOGPROB_ZERO;

	n_lat_entry++;
    }

    /* Slight BUG here: each rc can have its own history, but only the best retained!! */
    if (lattice[n_lat_entry-1].score < h->score[final_state]) {
	lattice[n_lat_entry-1].score = h->score[final_state];
	lattice[n_lat_entry-1].history = h->history[final_state];
    }
    lattice[n_lat_entry-1].rcscore[h->rc] = h->score[final_state];
}


/**
 * Transition from hmm h into the next appropriate one for word w.
 * Threshold check for incoming score already completed.
 * The next HMM may be the last triphone for the word w, in which case, instantiate
 * multiple instances corresponding cross-word triphone modelling for all right context
 * ciphones.
 */
static void whmm_transition (int32 w, whmm_t *h)
{
    int32 lastpos, npid, nf;
    whmm_t *nexth, *prevh;
    s3cipid_t rc;
    s3pid_t *pid;

    lastpos = dict->word[w].pronlen - 1;
    nf = n_frm+1;
    
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
	if (h->score[final_state] > nexth->score[0]) {
	    nexth->score[0] = h->score[final_state];
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
	    if (h->score[final_state] > nexth->score[0]) {
		nexth->score[0] = h->score[final_state];
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


/**
 * Get the last two non-filler, non-silence lattice words w0 and w1 (base word-ids),
 * starting from l.  w1 is later than w0.  At least w1 must exist; w0 may not.
 */
static void two_word_history (s3latid_t l, s3wid_t *w0, s3wid_t *w1)
{
    s3latid_t l0, l1;
    l0=0;
    
    for (l1 = l; dict_filler_word(dict, lattice[l1].wid); l1 = lattice[l1].history);

/* BHIKSHA HACK - PERMIT MULTIPLE PRONS FOR <s> */
if (l1 != -1) 
    for (l0 = lattice[l1].history; 
         (IS_S3LATID(l0)) && (dict_filler_word(dict,lattice[l0].wid));
	 l0 = lattice[l0].history);

/* BHIKSHA HACK - PERMIT MULTIPLE PRONS FOR <s> */
if (l1 == -1) *w1 = 0; else
    *w1 = dict_basewid(dict, lattice[l1].wid);
if (l1 == -1) *w0 = BAD_S3WID; else
    *w0 = (NOT_S3LATID(l0)) ? BAD_S3WID : dict_basewid(dict,lattice[l0].wid);
}


/**
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



/**
 * Backoff node when backing off all the way to unigrams.  Since each word exits with
 * #ciphones different scores (for so many different right contexts), a separate node
 * exists for each context.
 */
typedef struct {
    s3latid_t latid;	/** History entry */
    int32 score;	/** Acoustic + backed off LM score */
    s3cipid_t lc;	/** Last ciphone of history entry, to be used as left context upon
			   entering a new word. */
} backoff_t;
static backoff_t *ug_backoff, *filler_backoff;
static uint8 *tg_trans_done;	/** If tg_trans_done[w] TRUE, trigram transition to w
				   occurred for a given history, and backoff bigram
				   transition from same history should be avoided */
static int32 *rcscore = NULL;	/** rc scores uncompacted; one entry/rc-ciphone */
static s3wid_t *word_cand_cf;	/** BAD_S3WID terminated array of candidate words for word
				   transition in current frame (if using input word
				   lattices to restrict search). */

/**
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


/**
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

    word_cand_cf[dict->startwid] = 0;	/* Never hypothesized (except at beginning) */
    for (w = dict->filler_start; w <= dict->filler_end; w++)
	word_cand_cf[w] = 0;	/* Handled separately */
    word_cand_cf[dict->finishwid] = 1;	/* Always a candidate */

    n = 0;
    for (w = 0; w < dict->n_word; w++)
	if (word_cand_cf[w])
	    word_cand_cf[n++] = w;
    word_cand_cf[n] = BAD_S3WID;
}

/** Transition for one word. 
 */
static void word_trans (int32 thresh)
{
    s3latid_t l;	/* lattice entry index */
    s3cipid_t *rcmap, rc, lc;
    s3wid_t w, bw0, bw1, nextwid;
    tg_t *tgptr;
    bg_t *bgptr;
    int32 bowt, acc_bowt, newscore;
    int32 n_tg, n_bg;
    int32 cand, lscr;
    int32 lat_start;
    
    /*    int32 tempi, temp_j;*/

    lat_start = frm_latstart[n_frm];
    
    for (rc = 0; rc < mdef->n_ciphone; rc++) {
	ug_backoff[rc].score = S3_LOGPROB_ZERO;
	filler_backoff[rc].score = S3_LOGPROB_ZERO;
    }
    
    if (n_word_cand > 0)
	build_word_cand_cf (n_frm);
    
    /* Trigram/Bigram word transitions from words just exited */
    for (l = lat_start; l < n_lat_entry; l++) {
	w = lattice[l].wid;
	
	if (w == dict->finishwid)
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
	    if ((IS_S3WID(bw0)) && ((n_tg = lm_tglist (lm,
						       lm->dict2lmwid[dict_basewid(dict,bw0)], 
						       lm->dict2lmwid[dict_basewid(dict,bw1)], &tgptr, &bowt)) > 0)) {
		/* Transition to trigram followers of bw0, bw1, if any */
		for (; n_tg > 0; --n_tg, tgptr++) {
		    /* Transition to all alternative pronunciations for trigram follower */
		    nextwid = LM_DICTWID(lm, tgptr->wid);
		    
		    if (IS_S3WID(nextwid) && (nextwid != dict->startwid)) {
			for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
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
	    if ((n_bg = lm_bglist (lm,
				   lm->dict2lmwid[dict_basewid(dict,bw1)], 
				   &bgptr, 
				   &bowt)) > 0) {
		/* Transition to bigram followers of bw1, if any */
		for (; n_bg > 0; --n_bg, bgptr++) {
		    /* Transition to all alternative pronunciations for bigram follower */
		    nextwid = LM_DICTWID (lm, bgptr->wid);
		    
		    if (IS_S3WID(nextwid) &&
			(! tg_trans_done[nextwid]) &&	/* TG transition already done */
			(nextwid != dict->startwid)) {	/* No transition to <s> */
			for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
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
		if (rcscore[rc] <= S3_LOGPROB_ZERO)
		    continue;

		if (rcscore[rc]+acc_bowt+phone_penalty > ug_backoff[rc].score) {
		    ug_backoff[rc].score = rcscore[rc]+acc_bowt+phone_penalty;
		    ug_backoff[rc].latid = l;
		    ug_backoff[rc].lc = lc;
		}
	    }
	} else {
	    /* Transition to words in word_cand_cf */
	    for (cand = 0; IS_S3WID(word_cand_cf[cand]); cand++) {
		nextwid = word_cand_cf[cand];
		
		lscr = lm_tg_score (lm,
				    lm->dict2lmwid[dict_basewid(dict,bw0)],
				    lm->dict2lmwid[dict_basewid(dict,bw1)],
				    lm->dict2lmwid[nextwid],
				    nextwid);

		for (w = nextwid; IS_S3WID(w); w = dict->word[w].alt) {
		    newscore = rcscore[dict->word[w].ciphone[0]] + lscr + phone_penalty;
		    
		    if (newscore >= thresh)
			word_enter (w, newscore, l, lc);
		}
	    }
	}
	
	/* Update filler backoff node */
	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    if (rcscore[rc] <= S3_LOGPROB_ZERO)
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
	    for (w = ugptr->dictwid; IS_S3WID(w); w = dict->word[w].alt) {
		if (w == dict->startwid)
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
	if ((w == dict->startwid) || (w == dict->finishwid))
	    continue;
	
	rc = dict->word[w].ciphone[0];
	if (filler_backoff[rc].score > S3_LOGPROB_ZERO) {
	    newscore = filler_backoff[rc].score + fillpen(fpen,dict_basewid(dict,w));
	    if (newscore >= thresh)
		word_enter (w, newscore, filler_backoff[rc].latid, filler_backoff[rc].lc);
	}
    }

    /* Free rcscore here, if necessary to conserve memory space */
}


/** Initialize the forward search.
 */

void fwd_init (mdef_t* _mdef, tmat_t* _tmat, dict_t* _dict,lm_t *_lm)
{
    float64 *f64arg;
    float32 *f32arg;
    char *tmpstr;

    E_INFO ("Forward Viterbi Initialization\n");
    
    mdef = _mdef;
    tmat = _tmat;
    dict = _dict;
    lm   = _lm;
    
    assert (mdef && tmat && dict && lm);

    /* HMM states information */
    n_state = mdef->n_emit_state + 1;
    final_state = n_state - 1;

    /* Variables for speeding up whmm evaluation */
    st_sen_scr = (int32 *) ckd_calloc (n_state-1, sizeof(int32));


    /* Beam widths and penalties */
    f64arg = (float64 *) cmd_ln_access ("-beam");
    beam = logs3 (*f64arg);

    f64arg = (float64 *) cmd_ln_access ("-wbeam");
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

    /* Space for first lattice entry in each frame (+ terminating sentinel) */
    frm_latstart = (s3latid_t *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(s3latid_t));

    /* Allocate timers and counters for statistics gathering */
    
    pctr_new(ctr_mpx_whmm,"mpx");
    pctr_new(ctr_nonmpx_whmm,"~mpx");
    pctr_new(ctr_latentry,"lat");

    /* Word to be traced in detail */
    if ((tmpstr = (char *) cmd_ln_access ("-tracewhmm")) != NULL) {
	trace_wid = dict_wordid (dict,tmpstr);
	if (NOT_S3WID(trace_wid))
	    E_ERROR("%s not in dictionary; cannot be traced\n", tmpstr);
    } else
	trace_wid = BAD_S3WID;

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
	n_ug = lm_uglist (lm,&ugptr);
	for (; n_ug > 0; --n_ug, ugptr++) {
	    if ((w = ugptr->dictwid) == dict->startwid)
		continue;

	    ugprob = LM_UGPROB(lm, ugptr);

	    for (; IS_S3WID(w); w = dict->word[w].alt) {
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
    dag_init(&dag);
}


static int32 word_cand_load (FILE *fp)
{
    char line[1024], word[1024];
    int32 i, k, n, nn, sf, seqno, lineno;
    s3wid_t w;
    word_cand_t *candp;
    
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

	if ((k = sscanf (line, "%d %s %d", &seqno, word, &sf)) != 3) {
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
	
	w = dict_wordid (dict,word);
	if (NOT_S3WID(w)) {
	    E_ERROR("%s: Unknown word in lattice: %s; ignored\n", uttid, word);
	    continue;
	}
	w = dict_basewid(dict,w);
	
	/* Check node not already present; avoid duplicates */
	for (candp = word_cand[sf]; candp && (candp->wid != w); candp = candp->next);
	if (candp)
	    continue;
	
	candp = (word_cand_t *) listelem_alloc (sizeof(word_cand_t));
	candp->wid = w;
	candp->next = word_cand[sf];
	word_cand[sf] = candp;

	n++;
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
    int32 w, l, ispipe;
    char str[1024];
    FILE *fp;

    uttid = ckd_salloc (id);    
    ptmr_reset (&tm_hmmeval);
    ptmr_reset (&tm_hmmtrans);
    ptmr_reset (&tm_wdtrans);
   

    
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
    
    /* Enter all pronunciations of startwid (begin silence) */
    n_frm = -1;	/* Since word_enter transitions to "NEXT" frame */
    for (w = dict->startwid; IS_S3WID(w); w = dict->word[w].alt)
	word_enter (w, 0, BAD_S3LATID,
		    dict->word[dict->silwid].ciphone[dict->word[dict->silwid].pronlen-1]);
    n_frm = 0;

    renormalized = 0;
}


void fwd_sen_active (uint8 *senlist, int32 n_sen)
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


/** Do forward search for one frame.
 */
int32 fwd_frame (int32 *senscr)
{
    int32 bestscr;	/* Best state score for any whmm evaluated in this frame */
    int32 whmm_thresh;	/* Threshold for any whmm to stay alive in search */
    int32 word_thresh;	/* Threshold for a word-final whmm to succeed */
    whmm_t *h;
    
    ptmr_start (&tm_hmmeval);
    bestscr = whmm_eval (senscr);
    ptmr_stop (&tm_hmmeval);

    whmm_thresh = bestscr + beam;
    word_thresh = bestscr + wordbeam;
    
    /* Dump bestscore and pruning thresholds if any detailed tracing specified */
    if ((hmm_dump_sf < n_frm) || (word_dump_sf < n_frm) ||
	(IS_S3WID(trace_wid) && whmm[trace_wid])) {
	printf ("[%4d]: >>>> bestscore= %11d, whmm-thresh= %11d, word-thresh= %11d\n",
		n_frm, bestscr, whmm_thresh, word_thresh);
    }
    
    /* Dump all active HMMs or words, if indicated */
    if (hmm_dump_sf < n_frm)
	dump_all_whmm (senscr);
    else if (word_dump_sf < n_frm)
	dump_all_word ();
    
    /* Trace active HMMs for specified word, if any */
    if (IS_S3WID(trace_wid)) {
	for (h = whmm[trace_wid]; h; h = h->next)
	    dump_whmm (trace_wid, h, senscr);
    }

    ptmr_start (&tm_hmmtrans);
    frm_latstart[n_frm] = n_lat_entry;
    whmm_exit (whmm_thresh, word_thresh);
    ptmr_stop (&tm_hmmtrans);

    ptmr_start (&tm_wdtrans);
    if (frm_latstart[n_frm] < n_lat_entry)
	word_trans (whmm_thresh);
    ptmr_stop (&tm_wdtrans);
    
    if (bestscr < RENORM_THRESH) {
	E_INFO("Frame %d: bestscore= %d; renormalizing\n", n_frm, bestscr);
	whmm_renorm (bestscr);
    }
    
    n_frm++;

    return bestscr;
}


/**
 * Find path score for lattice entry l for the given right context word.
 * If context word is BAD_S3WID it's a wild card; return the best path score.
 */
static int32 lat_pscr_rc (s3latid_t l, s3wid_t w_rc)
{
    s3cipid_t *rcmap, rc;
    
    if ((NOT_S3WID(w_rc)) || (! lattice[l].rcscore))
	return lattice[l].score;
    
    rcmap = get_rc_cimap (lattice[l].wid);
    rc = dict->word[w_rc].ciphone[0];
    return (lattice[l].rcscore[rcmap[rc]]);
}


/**
 * Find LM score for transition into lattice entry l.
 */
static int32 lat_seg_lscr (s3latid_t l)
{
    s3wid_t bw0, bw1, bw2;
    int32 lscr, bowt, bo_lscr;
    tg_t *tgptr;
    bg_t *bgptr;
    
    bw2 = dict_basewid (dict,lattice[l].wid);

    if (dict_filler_word (dict,bw2))
	return (fillpen(fpen,bw2));
    
    if (NOT_S3LATID(lattice[l].history)) {
	assert (bw2 == dict->startwid);
	return 0;
    }
    
    two_word_history (lattice[l].history, &bw0, &bw1);
    lscr = lm_tg_score (lm, 
			lm->dict2lmwid[dict_basewid(dict,bw0)], 
			lm->dict2lmwid[dict_basewid(dict,bw1)], 
			lm->dict2lmwid[bw2],
			bw2);
    if (n_word_cand > 0)
	return lscr;

    /* Correction for backoff cpase if that scores better (see word_trans) */
    bo_lscr = 0;
    if ((IS_S3WID(bw0)) && (lm_tglist (lm,
				       lm->dict2lmwid[dict_basewid(dict,bw0)], 
				       lm->dict2lmwid[dict_basewid(dict,bw1)],
				       &tgptr, &bowt) > 0))
	bo_lscr = bowt;
    if (lm_bglist (lm, lm->dict2lmwid[dict_basewid(dict,bw1)], &bgptr, &bowt) > 0)
	bo_lscr += bowt;
    bo_lscr += lm_ug_score (lm,lm->dict2lmwid[dict_basewid(dict,bw2)], dict_basewid(dict,bw2));

    return ((lscr > bo_lscr) ? lscr : bo_lscr);
}


/**
 * Find acoustic and LM score for segmentation corresponding to lattice entry l with
 * the given right context word.
 */
static void lat_seg_ascr_lscr (s3latid_t l, s3wid_t w_rc, int32 *ascr, int32 *lscr)
{
    int32 start_score, end_score;

    /* Score with which l ended with right context = w_rc */
    if ((end_score = lat_pscr_rc (l, w_rc)) <= S3_LOGPROB_ZERO) {
	*ascr = *lscr = S3_LOGPROB_ZERO;
	return;
    }
    
    /* Score with which l was begun */
    start_score = IS_S3LATID(lattice[l].history) ?
	lat_pscr_rc (lattice[l].history, lattice[l].wid) : 0;

    /* LM score for the transition into l */
    *lscr = lat_seg_lscr (l);
    *ascr = end_score - start_score - *lscr;
}


static srch_hyp_t *lattice_backtrace (s3latid_t l, s3wid_t w_rc)
{
    srch_hyp_t *h, *prevh;

    if (IS_S3LATID(l)) {
	prevh = lattice_backtrace (lattice[l].history, lattice[l].wid);
	
	h = (srch_hyp_t *) listelem_alloc (sizeof(srch_hyp_t));
	if (! prevh)
	    hyp = h;
	else
	    prevh->next = h;
	h->next = NULL;
	
	h->wid = lattice[l].wid;
	h->word = dict_wordstr(dict,h->wid);
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
    
    bestl=BAD_S3LATID;

    if(cmd_ln_int32("-bt_wsil")){

      /* Find lattice entry in last frame for FINISH_WORD */
      for (l = frm_latstart[n_frm-1]; l < n_lat_entry; l++){
	if (dict_basewid(dict,lattice[l].wid) == dict->finishwid)
	  break;
      }
      
      if (l < n_lat_entry) {
	/* FINISH_WORD entry found; backtrack to obtain best Viterbi path */
	return (l);
      }
    
      /* Find last available lattice entry with best ending score */
      E_WARN("When %s is used as final word, %s: Search didn't end in %s\n", dict_wordstr(dict,dict->finishwid), uttid, dict_wordstr(dict,dict->finishwid));
    }


    bestscore = S3_LOGPROB_ZERO;
    for (f = n_frm-1; (f >= 0) && (bestscore <= S3_LOGPROB_ZERO); --f) {
	for (l = frm_latstart[f]; l < frm_latstart[f+1]; l++) {
	    if ((lattice[l].wid != dict->startwid) && (bestscore < lattice[l].score)) {
		bestscore = lattice[l].score;
		bestl = l;
	    }
	}
    }
    assert(! NOT_S3LATID(l));
    return ((f >= 0) ? bestl : BAD_S3LATID);
}


srch_hyp_t *fwd_end_utt ( void )
{
    whmm_t *h, *nexth;
    s3wid_t w;
    s3latid_t l;
    frm_latstart[n_frm] = n_lat_entry;	/* Add sentinel */
    pctr_increment (ctr_latentry, n_lat_entry);
    
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
    hyp_free (hyp);
    
    /* Check if bptable should be dumped (for debugging) */
    if (cmd_ln_int32 ("-bptbldump")){
      E_INFO("Dumping the whole lattice\n");
      lattice_dump (stdout);
    }

    /* Backtrack through lattice for Viterbi result */
    l = lat_final_entry ();
    if (NOT_S3LATID(l)){
      E_INFO("lattice ID: %d\n",l);
      E_ERROR("%s: NO RECOGNITION\n", uttid);
    }
    else
	lattice_backtrace (l, BAD_S3WID);	/* BAD_S3WID => Any right context */

    /* Note final lattice entry in DAG structure */
    dag.latfinal = l;

    return hyp;
}


void fwd_timing_dump (float64 tot)
{
    printf ("[H %6.2fx %2d%%]",
	    tm_hmmeval.t_cpu * 100.0 / n_frm, (int32)(tm_hmmeval.t_cpu * 100.0 / tot));
    printf ("[XH %6.2fx %2d%%]",
	    tm_hmmtrans.t_cpu * 100.0 / n_frm, (int32)(tm_hmmtrans.t_cpu * 100.0 / tot));
    printf ("[XW %6.2fx %2d%%]",
	    tm_wdtrans.t_cpu * 100.0 / n_frm, (int32)(tm_wdtrans.t_cpu * 100.0 / tot));
}

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
    if (NOT_S3LATID(latfinal))
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
    
    /* Find initial node.  (BUG HERE: There may be > 1 initial node for multiple <s>) */
    for (d = dag.list; d; d = d->alloc_next) {
	if ((dict_basewid(dict,d->wid) == dict->startwid) && (d->sf == 0))
	    break;
    }
    assert (d);
    dag.root = d;

    /* Build DAG edges: between nodes satisfying time adjacency */
    for (d = dag.list; d; d = d->alloc_next) {
	/* Skip links to this node if too short lived */
	if ((d != lattice[latfinal].dagnode) && (d->lef - d->fef < min_ef_range-1))
	    continue;
	
	if (d->sf == 0) 
{}
/*
	    assert (d->wid == dict->startwid);	*/ /* No predecessors to this */
	else {
	    /* Link from all end points == d->sf-1 to d */
	    for (l = frm_latstart[d->sf-1]; l < frm_latstart[d->sf]; l++) {
		pd = lattice[l].dagnode;	/* Predecessor DAG node */
		
		/* Skip predecessor node under following conditions */
		if (pd->wid == dict->finishwid)	/* BUG: alternative prons for </s>?? */
		    continue;
		if ((pd != dag.root) && (pd->lef - pd->fef < min_ef_range-1))
		    continue;
		
		/*
		 * Find acoustic score for link from pd to d (for lattice entry l
		 * with pd as right context).
		 */
		lat_seg_ascr_lscr (l, d->wid, &ascr, &lscr);
		if (ascr > S3_LOGPROB_ZERO)
		    dag_link (&dag, pd, d, ascr, d->sf-1, NULL);
	    }
	}
    }
    
    dag.filler_removed = 0;
    dag.fudged = 0;

    return 0;
}


/**
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

	    if ((pd->wid != dict->finishwid) &&
		(pd->fef == d->sf) &&
		(pd->lef - pd->fef >= min_ef_range-1)) {
		lat_seg_ascr_lscr (l, BAD_S3WID, &ascr, &lscr);
		dag_link (&dag, pd, d, ascr, d->sf-1, NULL);
	    }
	}
	
	if (fudge < 2)
	    continue;
	
	/* Links to d from nodes that first ended just BEYOND when d started */
	for (l = frm_latstart[d->sf+1]; l < frm_latstart[d->sf+2]; l++) {
	    pd = lattice[l].dagnode;		/* Predecessor DAG node */

	    if ((pd->wid != dict->finishwid) &&
		(pd->fef == d->sf+1) &&
		(pd->lef - pd->fef >= min_ef_range-1)) {
		lat_seg_ascr_lscr (l, BAD_S3WID, &ascr, &lscr);
		dag_link (&dag, pd, d, ascr, d->sf-1, NULL);
	    }
	}
    }
}


/**
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
    if (dict_filler_word (dict,lattice[latfinal].wid))
	lattice[latfinal].dagnode->wid = dict->finishwid;

    /*
     * Remove filler nodes.  In principle, successors can be fillers and the process
     * must be repeated.  But removing fillers in the order in which they appear in
     * dag.list ensures that succeeding fillers have already been eliminated.
     */
    for (d = dag.list; d; d = d->alloc_next) {
	if (! dict_filler_word (dict,d->wid))
	    continue;
	
	/* Replace each link TO d with links to d's successors */
	for (plink = d->predlist; plink; plink = plink->next) {
	    pnode = plink->node;
	    ascr = plink->ascr;
	    ascr += lwf * fillpen(fpen,dict_basewid (dict,d->wid));
	    
	    /* Link this predecessor of d to successors of d */
	    for (slink = d->succlist; slink; slink = slink->next) {
		snode = slink->node;

		/* Link only to non-filler successors; fillers have been eliminated */
		if (! dict_filler_word (dict,snode->wid)) {
		    /* Update because a link may already exist */
		    dag_update_link (&dag, pnode, snode, ascr + slink->ascr, plink->ef, slink);
		}
	    }
	}
    }

    /* Attach a dummy predecessor link from <<s>,0> to nowhere */
    dag_link (&dag,NULL, dag.root, 0, -1, NULL);
    
    /* Attach a dummy predecessor link from nowhere into final DAG node */
    plink = &(dag.final);
    plink->node = lattice[latfinal].dagnode;
    plink->src = NULL;
    lat_seg_ascr_lscr (latfinal, BAD_S3WID, &(plink->ascr), &lscr);
    plink->pscr = (int32)0x80000000;
    plink->lscr = 0;
    plink->bypass = NULL;
    plink->history = NULL;
    plink->ef = lattice[latfinal].frm;
    plink->next = NULL;
}


/**
 * Final global best path through DAG constructed from the word lattice.
 * Assumes that the DAG has already been constructed and is consistent with the word
 * lattice.
 * The search uses a recursive algorithm to find the best (reverse) path from the final
 * DAG node to the root:  The best path from any node (beginning with a particular link L)
 * depends on a similar best path for all links leaving the endpoint of L.  (This is
 * sufficient to handle trigram LMs.)
 */
srch_hyp_t *s3flat_fwd_dag_search (char *utt)
{
    daglink_t *l, *bestl;
    dagnode_t *d, *final;
    int32 bestscore;
    float32 *f32arg;
    float64 lwf;
    int32 fudge, min_ef_range;
    int32 k;

    if (renormalized) {
	E_ERROR("Scores renormalized during forward Viterbi; cannot run bestpath\n");
	return NULL;
    }
    
    f32arg = (float32 *) cmd_ln_access ("-bestpathlw");
    lwf = f32arg ? ((*f32arg) / *((float32 *) cmd_ln_access ("-lw"))) : 1.0;
    
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
    
    dag.maxlmop = *((int32 *) cmd_ln_access ("-maxlmop"));
    k = *((int32 *) cmd_ln_access ("-maxlpf"));
    k *= dag.nfrm;
    if (dag.maxlmop > k)
	dag.maxlmop = k;
    dag.lmop = 0;

    /* Find the backward link from the final DAG node that has the best path to root */
    final = lattice[dag.latfinal].dagnode;
    bestscore = (int32) 0x80000000;
    bestl = NULL;

    /* Check that all edge scores are -ve; error if not so. */
    if (dag_chk_linkscr (&dag) < 0){
      E_ERROR("Some edges are not negative\n");
      return NULL;
    }
    
    for (l = final->predlist; l; l = l->next) {
	d = l->node;
	if (! dict_filler_word (dict,d->wid)) {	/* Ignore filler node */
	    dag_bestpath (&dag,l, final, lwf,dict,lm,lm->dict2lmwid);	/* Best path to root beginning with l */

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
    hyp_free (hyp);
    hyp = dag_backtrace (hyp, l, lwf, dict, fpen);
    
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

    fprintf (fp, "# -dict %s\n", (char *) cmd_ln_access ("-dict"));
    if (cmd_ln_access ("-fdict"))
	fprintf (fp, "# -fdict %s\n", (char *) cmd_ln_access ("-fdict"));
    fprintf (fp, "# -lm %s\n", (char *) cmd_ln_access ("-lm"));
    fprintf (fp, "# -mdef %s\n", (char *) cmd_ln_access ("-mdef"));
    fprintf (fp, "# -senmgau %s\n", (char *) cmd_ln_access ("-senmgau"));
    fprintf (fp, "# -mean %s\n", (char *) cmd_ln_access ("-mean"));
    fprintf (fp, "# -var %s\n", (char *) cmd_ln_access ("-var"));
    fprintf (fp, "# -mixw %s\n", (char *) cmd_ln_access ("-mixw"));
    fprintf (fp, "# -tmat %s\n", (char *) cmd_ln_access ("-tmat"));
    fprintf (fp, "# -min_endfr %d\n", *((int32 *) cmd_ln_access ("-min_endfr")));
    fprintf (fp, "#\n");
    
    fprintf (fp, "Frames %d\n", n_frm);
    fprintf (fp, "#\n");
    
    for (i = 0, d = dag.list; d; d = d->alloc_next, i++);
    fprintf (fp, "Nodes %d (NODEID WORD STARTFRAME FIRST-ENDFRAME LAST-ENDFRAME)\n", i);
    for (i = 0, d = dag.list; d; d = d->alloc_next, i++) {
	d->seqid = i;
	fprintf (fp, "%d %s %d %d %d\n", i, dict_wordstr(dict,d->wid), d->sf, d->fef, d->lef);
    }
    fprintf (fp, "#\n");

    fprintf (fp, "Initial %d\nFinal %d\n", initial->seqid, final->seqid);
    fprintf (fp, "#\n");
    
    /* Best score (i.e., regardless of Right Context) for word segments in word lattice */
    fprintf (fp, "BestSegAscr %d (NODEID ENDFRAME ASCORE)\n", n_lat_entry);
    if (! onlynodes) {
	for (i = 0; i < n_lat_entry; i++) {
	    lat_seg_ascr_lscr (i, BAD_S3WID, &ascr, &lscr);
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
