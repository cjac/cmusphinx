/*
 * lextree.h -- Maintaining the lexical tree.
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
 * 15-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LEXTREE_H_
#define _LEXTREE_H_


/*
 * Lextree structure used for search: actually consists of two parts--tree and flat.
 * The initial prefix, the tree, has HMM nodes shared among more than one word.  It
 * then develops into a flat structure with no such sharing.  The tree depth at which
 * this transition occurs can be controlled.  This allows us to control the point at
 * which LM scores may be applied.  Regardless of this transition point, the final
 * node (HMM) for a word is always in the flat structure.  So single phone words, for
 * example, are entirely within the flat structure.
 */


#include <libutil/libutil.h>
#include <libmain/hmm.h>
#include "kb.h"


/*
 * Each "node" in the lextree contains all the HMM/lexicon related info, as well
 * as a list (glist) of its children "nodes".  The entire lextree is a list (glist)
 * of such top-level "nodes".
 */
typedef struct {
    hmm_t hmm;
    int32 id;		/* If < 0, lextree internal node (i.e., not flat part).
			   Else, includes word-id and pronunciation position within
			   the word for this node; see definitions below. */
    glist_t children;	/* data.ptr should be (lextree_node_t *); see libutil/glist */
} lextree_node_t;


/*
 * Representing word id (wid) and pronunciation position (ppos) in node id field.
 * Note: pronunciation position is measured from the END, 0 being the final phone for
 * the word.
 */
#define	LEXTREE_NODEID(wid,ppos)	(((wid) & 0x00ffffff) | ((ppos) << 24))
#define LEXTREE_NODEID2WID(id)		((id) & 0x00ffffff)
#define LEXTREE_NODEID2PPOS(id)		(((id) >> 24) & 0x000000ff)
#define LEXTREE_NODEID_MAXWID		((int32) 0x00ffffff)
#define LEXTREE_NODEID_MAXPPOS		((int32) 0x0000007f)
#define LEXTREE_NODEID_INVALID(id)	((id) < 0)
#define LEXTREE_NODEID_VALID(id)	((id) >= 0)
#define LEXTREE_NODEID_NONE		(-1)


/*
 * Build a lexical tree for the given lexicon.  The nodes in the tree are of type
 * lextree_node_t.
 * Return value: a glist of top-level nodes in the lextree; NULL if empty or error.
 */
glist_t lextree_build(dict_t *dict,	/* In: Lexicon for which lextree is to be built */
		      mdef_t *mdef,	/* In: Model definition with phone definitions */
		      bitvec_t active,	/* In: Lexicon word-id w is included in the lextree iff
					   active[w] is TRUE.  If active is NULL, the entire
					   lexicon is active.  The caller must ensure that any
					   active word is in the LM (or is a filler word). */
		      int32 flatdepth);	/* In: Depth starting from which the generated
					   "tree" is forced to be flat; e.g., if 0,
					   the result is entirely flat. */

/*
 * Prepare for Viterbi decoding of new utterance.  Any clutter from earlier utterances must have
 * been cleared already.
 */
void lextree_vit_start (kb_t *kb,	/* In/Out: Models and search structures */
			char *uttid);	/* In: String name for new utt */

/*
 * Push Viterbi search through one frame.  Note that the active list of lextree nodes is a glist
 * given by kb->lextree_active, and it is updated for the next frame as a side-effect.
 * Return value: Best HMM state score at the end of the current frame.
 */
int32 lextree_vit_frame (kb_t *kb,	/* In/Out: Models and search structures */
			 int32 frm,	/* In: Current frame */
			 char *uttid);	/* In: String name for new utt */

/*
 * Finish the utterance.
 * Return value: Viterbi history node for the final node if successful, NULL if error.
 * (The utterance hypothesis can be extracted by backtracking via the history nodes.)
 */
vithist_t *lextree_vit_end (kb_t *kb,		/* In/Out: Models and search structures */
			    int32 frm,		/* In: #Frames in utterance */
			    char *uttid);	/* In: String name for new utt */

#endif
