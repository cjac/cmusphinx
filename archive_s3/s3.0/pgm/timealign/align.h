/*
 * align.h -- Time alignment module.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 11-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _TIMEALIGN_H_
#define _TIMEALIGN_H_


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>

#include <libfeat/feat.h>
#include <libmain/mdef.h>
#include <libmain/dict.h>
#include <libmain/tmat.h>
#include <libmain/am.h>
#include <libmain/wnet2pnet.h>


/* Collection of knowledge sources and working data structures for the alignment */
typedef struct {
    mdef_t *mdef;	/* HMM model definition */
    dict_t *dict;	/* Pronunciation dictionary */
    acoustic_t *am;	/* Acoustic models; gaussian density codebooks/senones */
    tmat_t *tmat;	/* HMM transition matrices */
    glist_t pnet;	/* The phone net being searched */
    pnode_t *pstart;	/* Dummy initial pnode in pnet */
    pnode_t *pend;	/* Dummy final pnode in pnet */
    glist_t pactive;	/* List of active pnodes in any frame (subset of pnodes in pnet);
			   updated at the end of each frame to reflect the active set in
			   the next frame */
    glist_t hist;	/* Viterbi history accumulated during the search */
    corpus_t *insent;	/* Input transcript corpus file */
    FILE *outsentfp;	/* Output transcript file */
} kb_t;


/*
 * Start an alignment with the given phone net.  The process starts with the successors
 * of the dummy pnode pstart.  These are the initial set of active pnodes.  Further, the
 * set of active senones is flagged in kb->am->sen_active.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 align_start (kb_t *kb,	/* In/Out: (See structure above.) */
		   char *uttid);

/*
 * Step the phone net through one frame of Viterbi evaluation.  The given list of active
 * pnodes are evaluated, pruned according to the given beamwidth, and cross-pnode
 * transitions performed.
 * Return value: 0 if successful, -1 if any error occurred.
 */
int32 align_frame (kb_t *kb,	/* In/Out: kb->am->senscr should contain senone scores
				   for this frame before calling this function.  Upon
				   return, kb->am->sen_active is updated to reflect the
				   active senones for the next frame. */
		   int32 beam,	/* In: Pruning beamwidth for determining active pnodes
				   at the end of this frame, and also to determine which
				   states are entered in the Viterbi history */
		   int32 frm,	/* In: The current frame no. */
		   char *uttid);

/*
 * End of the alignment.  Cleanup as necessary.  Backtrace through the Viterbi history to
 * find the best alignment.
 * Return value: State/frame alignment.  This is a list of hyp_t nodes with word, phone
 * position, and state position encoded in the id field.  Use other functions listed here
 * to interpret the results.  Use align_hypfree to free the structure.
 * 
 */
glist_t align_end (kb_t *kb,		/* In/Out: Working data structures (pactive and
					   hist) cleaned up */
		   int32 frm,		/* In: Total number of frames searched */
		   char *uttid);	/* In: Utterance ID */

/*
 * Generate and return a word-level segmentation from the state-level segmentation
 * returned by align_end().  This is a list of hyp_t nodes with dictionary word-id in the
 * id field.  Use align_hypfree to free the returned information.
 */
glist_t align_st2wdseg (kb_t *kb,	/* In: The usual */
			glist_t stseg);	/* In: State segmentation from which the word
					   segmentation is built up */

/*
 * Write state segmentation in Sphinx-II format (always BIG-ENDIAN):
 * Format:
 *     (int32)(No. of frames)
 *     (int16)State information for each frame, consisting of:
 * 	   (CIphone ID) * (kb->mdef->n_emit_state) + (state-ID within phone); with
 * 	   the MSB (bit-15) turned on if this is the first frame of each new phone.
 */
void align_write_s2stseg (kb_t *kb,	/* In: Reference databases */
			  glist_t stseg,/* In: State-level Viterbi hypothesis returned
					   by align_end */
			  char *file,	/* In: Output filename */
			  char *uttid);	/* In: For logging purposes */

/*
 * Write word level segmentation; arguments similar to align_write_s2stseg(), but 
 * with a word segmentation input.
 */
void align_write_wdseg (kb_t *kb, glist_t wdseg, char *file, char *uttid);


/* Write word level sentence transcript; arguments similar to align_write_wdseg() */
void align_write_outsent (kb_t *kb, glist_t wdseg, FILE *fp, char *uttid);


/*
 * Free the segmentation hyp list returned by align_end() or align_st2wdseg().
 */
void align_hypfree (glist_t hyp);


#endif
