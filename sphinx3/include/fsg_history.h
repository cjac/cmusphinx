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
 * fsg_history.h -- FSG Viterbi decode history
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
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:10:18  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Adaptation of Sphinx 2's FSG search into Sphinx 3
 *
 * Revision 1.1.2.5  2005/07/24 19:34:46  arthchan2003
 * Removed search_hyp_t, used srch_hyp_t instead
 *
 * Revision 1.1.2.4  2005/07/24 01:34:54  arthchan2003
 * Mode 2 is basically running. Still need to fix function such as resulting and build the correct utterance ID
 *
 * Revision 1.1.2.3  2005/07/13 18:39:47  arthchan2003
 * (For Fun) Remove the hmm_t hack. Consider each s2 global functions one-by-one and replace them by sphinx 3's macro.  There are 8 minor HACKs where functions need to be removed temporarily.  Also, there are three major hacks. 1,  there are no concept of "phone" in sphinx3 dict_t, there is only ciphone. That is to say we need to build it ourselves. 2, sphinx2 dict_t will be a bunch of left and right context tables.  This is currently bypass. 3, the fsg routine is using fsg_hmm_t which is just a duplication of CHAN_T in sphinx2, I will guess using hmm_evaluate should be a good replacement.  But I haven't figure it out yet.
 *
 * Revision 1.1.2.2  2005/06/28 07:01:20  arthchan2003
 * General fix of fsg routines to make a prototype of fsg_init and fsg_read. Not completed.  The number of empty functions in fsg_search is now decreased from 35 to 30.
 *
 * Revision 1.1.2.1  2005/06/27 05:26:29  arthchan2003
 * Sphinx 2 fsg mainpulation routines.  Compiled with faked functions.  Currently fended off from users.
 *
 * Revision 1.1  2004/07/16 00:57:12  egouvea
 * Added Ravi's implementation of FSG support.
 *
 * Revision 1.7  2004/07/07 22:30:35  rkm
 * *** empty log message ***
 *
 * Revision 1.6  2004/07/07 13:56:33  rkm
 * Added reporting of (acoustic score - best senone score)/frame
 *
 * Revision 1.5  2004/06/25 14:49:08  rkm
 * Optimized size of history table and speed of word transitions by maintaining only best scoring word exits at each state
 *
 * Revision 1.4  2004/06/23 20:32:16  rkm
 * *** empty log message ***
 *
 * Revision 1.3  2004/05/27 15:16:08  rkm
 * *** empty log message ***
 *
 * 
 * 25-Feb-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started, based on S3.3 version.
 */


#ifndef __S2_FSG_HISTORY_H__
#define __S2_FSG_HISTORY_H__


#include <stdio.h>

#include <s3types.h>
#include <blkarray_list.h>
#include <fsg_psubtree.h>
#include <word_fsg.h>
#include <search.h>
#include <dict.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

/*
 * The Viterbi history structure.  This is a tree, with the root at the
 * FSG start state, at frame 0, with a null predecessor.
 */

/*
 * A single Viterbi history entry
 */
typedef struct fsg_hist_entry_s {
    word_fsglink_t *fsglink;	/* Link taken result in this entry */
    int32 frame;			/* Ending frame for this entry */
    int32 score;			/* Total path score at the end of this
					   transition */
    int32 pred; 			/* Predecessor entry; -1 if none */
    int32 lc;			/* Left context provided by this entry to
                                   succeeding words */
    fsg_pnode_ctxt_t rc;		/* Possible right contexts to which this entry
					   applies */
} fsg_hist_entry_t;

/* Access macros */
#define fsg_hist_entry_fsglink(v)	((v)->fsglink)
#define fsg_hist_entry_frame(v)		((v)->frame)
#define fsg_hist_entry_score(v)		((v)->score)
#define fsg_hist_entry_pred(v)		((v)->pred)
#define fsg_hist_entry_lc(v)		((v)->lc)
#define fsg_hist_entry_rc(v)		((v)->rc)


/*
 * The entire tree of history entries (fsg_history_t.entries).
 * Optimization: In a given frame, there may be several history entries, with
 * the same left and right phonetic context, terminating in a particular state.
 * Only the best scoring one of these needs to be saved, since everything else
 * will be pruned according to the Viterbi algorithm.  frame_entries is used
 * temporarily in each frame to determine these best scoring entries in that
 * frame.  Only the ones not pruned are transferred to entries at the end of
 * the frame.  However, null transitions are a problem since they create
 * entries that depend on entries created in the CURRENT frame.  Hence, this
 * pruning is done in two stages: first for the non-null transitions, and then
 * for the null transitions alone.  (This solution is sub-optimal, and can be
 * improved with a little more work.  SMOP.)
 * Why is frame_entries a list?  Each entry has a unique terminating state,
 * and has a unique lc CIphone.  But it has a SET of rc CIphones.
 * frame_entries[s][lc] is an ordered list of entries created in the current
 * frame, terminating in state s, and with left context lc.  The list is in
 * descending order of path score.  When a new entry with (s,lc) arrives,
 * its position in the list is determined.  Then its rc set is modified by
 * subtracting the union of the rc's of all its predecessors (i.e., better
 * scoring entries).  If the resulting rc set is empty, the entry is discarded.
 * Otherwise, it is inserted, and the rc sets of all downstream entries in the
 * list are updated by subtracting the new entry's rc.  If any of them becomes
 * empty, it is also discarded.
 * As mentioned earlier, this procedure is applied in two stages, for the
 * non-null transitions, and the null transitions, separately.
 */
typedef struct fsg_history_s {
    word_fsg_t *fsg;		/* The FSG for which this object applies */
    blkarray_list_t *entries;	/* A list of history table entries; the root
                                   entry is the first element of the list */
    glist_t **frame_entries;

    /*Added by Arthur at 20050627*/
    int32 n_ciphone;
} fsg_history_t;


/*
 * One-time intialization: Allocate and return an initially empty history
 * module.
 */
fsg_history_t *fsg_history_init (word_fsg_t *fsg,int32 n_ciphone);

void fsg_history_free (fsg_history_t *);

void fsg_history_utt_start (fsg_history_t *);


void fsg_history_utt_end (fsg_history_t *);


/*
 * Create a history entry recording the completion of the given FSG
 * transition, at the end of the given frame, with the given score, and
 * the given predecessor history entry.
 * The entry is initially temporary, and may be superseded by another
 * with a higher score.  The surviving entries must be transferred to
 * the main history table, via fsg_history_end_frame().
 */
void fsg_history_entry_add (fsg_history_t *,
			    word_fsglink_t *,	/* FSG transition */
			    int32 frame,
			    int32 score,
			    int32 pred,
			    int32 lc,
			    fsg_pnode_ctxt_t rc);

/*
 * Transfer the surviving history entries for this frame into the permanent
 * history table.  This function can be called several times during a frame.
 * Each time, the entries surviving so far are transferred, and the temporary
 * lists cleared.  This feature is used to handle the entries due to non-null
 * transitions and null transitions separately.
 */
void fsg_history_end_frame (fsg_history_t *);


/* Clear the hitory table */
void fsg_history_reset (fsg_history_t *);


/* Return the number of valid entries in the given history table */
int32 fsg_history_n_entries (fsg_history_t *h);


/*
 * Viterbi backtrace.
 */
glist_t fsg_history_backtrace (fsg_history_t *);


/*
 * Dump the Viterbi history data to the given file (mainly for debugging).
 */
void fsg_history_dump (fsg_history_t *vh, char const *uttid, FILE *fp, dict_t *dict);


/*
 * Return a ptr to the history entry for the given ID; NULL if there is no
 * such entry.
 */
fsg_hist_entry_t *fsg_history_entry_get(fsg_history_t *, int32 id);


/*
 * Switch the FSG associated with the given history module.  Should be done
 * when the history list is empty.  If not empty, the list is cleared.
 */
void fsg_history_set_fsg (fsg_history_t *, word_fsg_t *);


/* Free the given Viterbi search history object */
void fsg_history_free (fsg_history_t *h);


/*
 * Extract and fill out a srch_hyp_t structure from the given history entry
 * (index).  Caller must allocate hyp.
 * Note: Must not be called with index <= 0.
 * Note: (hyp->wid < 0) iff the entry corresponds to a null transition.
 * Note: hyp->{conf,latden,phone_perp} have dummy values filled in.
 * Note: hyp->next is unaffected.
 * Return value: -1 if index <= 0 (error; hyp is not filled in);
 * the return value is +ve if it is a valid, real (non-dummy) entry.
 */
int32 fsg_history_entry_hyp_extract (fsg_history_t *h, int32 index,
				     srch_hyp_t *hyp, dict_t *dict);

#ifdef __cplusplus
}
#endif


#endif
