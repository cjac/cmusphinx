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
 * fsg_search.h -- Search structures for FSM decoding.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2004/07/16  00:57:12  egouvea
 * Added Ravi's implementation of FSG support.
 * 
 * Revision 1.4  2004/07/07 13:56:33  rkm
 * Added reporting of (acoustic score - best senone score)/frame
 *
 * Revision 1.3  2004/06/22 15:36:12  rkm
 * Added partial result handling in FSG mode
 *
 * Revision 1.2  2004/05/27 14:22:57  rkm
 * FSG cross-word triphones completed (but for single-phone words)
 *
 * Revision 1.1.1.1  2004/03/01 14:30:31  rkm
 *
 *
 * Revision 1.6  2004/02/27 16:15:13  rkm
 * Added FSG switching
 *
 * Revision 1.5  2004/02/27 15:05:21  rkm
 * *** empty log message ***
 *
 * Revision 1.4  2004/02/26 14:48:20  rkm
 * *** empty log message ***
 *
 * Revision 1.3  2004/02/26 01:14:48  rkm
 * *** empty log message ***
 *
 * Revision 1.2  2004/02/24 18:13:05  rkm
 * Added NULL transition handling
 *
 * Revision 1.1  2004/02/23 15:53:45  rkm
 * Renamed from fst to fsg
 *
 * Revision 1.1  2004/02/19 21:16:54  rkm
 * Added fsg_search.{c,h}
 *
 * 
 * 18-Feb-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */


#ifndef __S2_FSG_SEARCH_H__
#define __S2_FSG_SEARCH_H__


#include <glist.h>
#include <fsg_lextree.h>
#include <fsg_history.h>
#include <fbs.h>


typedef struct fsg_search_s {
  glist_t fsglist;		/* List of all FSGs loaded */
  
  word_fsg_t *fsg;		/* Currently active FSG; NULL if none.  One
				   must be made active before starting FSG
				   decoding */
  fsg_lextree_t *lextree;	/* Lextree structure for the currently
				   active FSG */
  fsg_history_t *history;	/* For storing the Viterbi search history */
  
  glist_t pnode_active;		/* Those active in this frame */
  glist_t pnode_active_next;	/* Those activated for the next frame */
  
  int32 beam;			/* Global threshold */
  int32 pbeam;			/* Threshold for phone transition */
  int32 wbeam;			/* Threshold for word exit */
  
  int32 frame;			/* Current frame */

  int32 bestscore;		/* For beam pruning */
  int32 bpidx_start;		/* First history entry index this frame */
  
  search_hyp_t *hyp;		/* Search hypothesis */
  int32 ascr, lscr;		/* Total acoustic and lm score for utt */
  
  int32 n_hmm_eval;		/* Total HMMs evaluated this utt */
  
  int32 state;			/* Whether IDLE or BUSY */
} fsg_search_t;


/* Access macros */
#define fsg_search_frame(s)	((s)->frame)


/*
 * Create, initialize and return a search module for the given FSM.
 * If no FSG is given (i.e., the argument is NULL), a search structure is
 * still created.  If an FSG is provided, it is made the currently active
 * FSG.
 */
fsg_search_t *fsg_search_init (word_fsg_t *);


/*
 * Lookup the FSG associated with the given name and return it, or NULL if
 * no match found.
 */
word_fsg_t *fsg_search_fsgname_to_fsg (fsg_search_t *, char *name);


/*
 * Add the given FSG to the collection of FSGs known to this search object.
 * The given fsg is simply added to the collection.  It is not automatically
 * made the currently active one.
 * The name of the new FSG must not match any of the existing ones.  If so,
 * FALSE is returned.  If successfully added, TRUE is returned.
 */
boolean fsg_search_add_fsg (fsg_search_t *, word_fsg_t *);

/*
 * Delete the given FSG from the known collection.  Free the FSG itself,
 * and if it was the currently active FSG, also free the associated search
 * structures and leave the current FSG undefined.
 */
boolean fsg_search_del_fsg (fsg_search_t *, word_fsg_t *);

/* Like fsg_search_del_fsg(), but identifies the FSG by its name */
boolean fsg_search_del_fsg_byname (fsg_search_t *, char *name);


/*
 * Switch to a new FSG (identified by its string name).  Must not be invoked
 * when search is busy (ie, in the midst of an utterance.  That's an error
 * and FALSE is returned.  If successful, returns TRUE.
 */
boolean fsg_search_set_current_fsg (fsg_search_t *, char *);


/*
 * Deallocate search structure.
 */
void fsg_search_free (fsg_search_t *);


/*
 * Prepare the FSG search structure for beginning decoding of the next
 * utterance.
 */
void fsg_search_utt_start (fsg_search_t *);


/*
 * Windup and clean the FSG search structure after utterance.  Fill in the
 * results of search: fsg_search_t.{hyp,ascr,lscr,frame}.  (But some fields
 * of hyp are left unfilled for now: conf, latden, phone_perp.)
 */
void fsg_search_utt_end (fsg_search_t *);


/*
 * Step one frame forward through the Viterbi search.
 */
void fsg_search_frame_fwd (fsg_search_t *);


/*
 * Compute the partial or final Viterbi backtrace result.  (The result can
 * be retrieved using the API functions seach_result or search_get_hyp().)
 * If "check_fsg_final_state" is TRUE, the backtrace starts from the best
 * history entry ending in the final state (if it exists).  Otherwise it
 * starts from the best entry, regardless of the terminating state (usually
 * used for partial results).
 */
void fsg_search_history_backtrace (fsg_search_t *search,
				   boolean check_fsg_final_state);

#endif
