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
 * vithist.h -- Viterbi history
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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added vithist_free() to free allocated memory
 * 
 * 30-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *		Added vithist_entry_t.ascr.
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added maxwpf handling.
 * 
 * 24-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_VITHIST_H_
#define _S3_VITHIST_H_


#include <libutil/libutil.h>
#include "s3types.h"
#include "kbcore.h"
#include "hyp.h"


/*
 * LM state.  Depending on type of LM (word-ngram, class-ngram, FSG, etc.), the contents
 * of LM state will vary.  Accommodate them with a union.  For now, only trigram LM in it.
 * (Not completely thought out; some of this might have to change later.)
 */
typedef union {
    struct {
	s3lmwid_t lwid[2];	/* 2-word history; [0] is most recent */
    } lm3g;
} vh_lmstate_t;


/*
 * Viterbi history entry.
 */
typedef struct {
    s3wid_t wid;		/* Dictionary word ID; exact word that just exited */
    s3frmid_t sf, ef;		/* Start and end frames for this entry */
    int32 ascr;			/* Acoustic score for this node */
    int32 lscr;			/* LM score for this node, given its Viterbi history */
    int32 score;		/* Total path score ending here */
    int32 pred;			/* Immediate predecessor */
    int32 type;			/* >=0: regular n-gram word; <0: filler word entry */
    int32 valid;		/* Whether it should be a valid history for LM rescoring */
    vh_lmstate_t lmstate;	/* LM state */
} vithist_entry_t;

#define vithist_entry_wid(ve)	((ve)->wid)
#define vithist_entry_sf(ve)	((ve)->sf)
#define vithist_entry_ef(ve)	((ve)->ef)
#define vithist_entry_ascr(ve)	((ve)->ascr)
#define vithist_entry_lscr(ve)	((ve)->lscr)
#define vithist_entry_score(ve)	((ve)->score)
#define vithist_entry_pred(ve)	((ve)->pred)
#define vithist_entry_valid(ve)	((ve)->valid)


/* 
 * In each frame, there are several word exits.  There can be several exit instances of the
 * same word, corresponding to different LM histories.  Generally, each exit is associated
 * with an LM state.  We only need to retain the best entry for each LM state.  The following
 * structure is for this purpose.
 * For all exits in the current frame, all n-word histories (assuming an N-gram LM) ending in
 * a given word are arranged in a tree, with the most recent history word at the root.  The
 * leaves of the tree point to the (current best) vithist entry with that history in the
 * current frame.
 */
typedef struct {		/* Mapping from LM state to vithist entry */
    int32 state;		/* (Part of) the state information */
    int32 vhid;			/* Associated vithist ID (only for leaf nodes) */
    vithist_entry_t *ve;	/* Entry ptr corresponding to vhid (only for leaf nodes) */
    glist_t children;		/* Children of this node in the LM state tree; data.ptr of
				   type (vh_lms2vh_t *) */
} vh_lms2vh_t;


/*
 * Memory management of Viterbi history entries done in blocks.  Initially, one block of
 * VITHIST_BLKSIZE entries allocated.  If exhausted, another block allocated, and so on.
 * So we can have several discontiguous blocks allocated.  Entries are identified by a
 * global, running sequence no.
 */
typedef struct {
    vithist_entry_t **entry;	/* entry[i][j]= j-th entry in the i-th block allocated */
    int32 *frame_start;		/* For each frame, the first vithist ID in that frame; (the
				   last is just before the first of the next frame) */
    int32 n_entry;		/* Total #entries used (generates global seq no. or ID) */
    int32 n_frm;		/* No. of frames processed so far in this utterance */
    
    int32 bghist;		/* If TRUE (bigram-mode) only one entry/word/frame; otherwise
				   multiple entries allowed, one per distinct LM state */
    
    int32 wbeam;		/* Pruning beamwidth */
    
    int32 *bestscore;		/* Best word exit score in each frame */
    int32 *bestvh;		/* Vithist entry ID with the best exit score in each frame */
    
    vh_lms2vh_t **lms2vh_root;	/* lms2vh[w]= Root of LM states ending in w in current frame */
    glist_t lwidlist;		/* List of LM word IDs with entries in lms2vh_root */
} vithist_t;


#define VITHIST_BLKSIZE		16384	/* (1 << 14) */
#define VITHIST_MAXBLKS		256
#define VITHIST_ID2BLK(i)	((i) >> 14)
#define VITHIST_ID2BLKOFFSET(i)	((i) & 0x00003fff)	/* 14 LSB */

/* Access macros; not meant for arbitrary use */
#define vithist_n_entry(vh)		((vh)->n_entry)
#define vithist_bestscore(vh)		((vh)->bestscore)
#define vithist_bestvh(vh)		((vh)->bestvh)
#define vithist_lms2vh_root(vh,w)	((vh)->lms2vh_root[w])
#define vithist_lwidlist(vh)		((vh)->lwidlist)
#define vithist_first_entry(vh,f)	((vh)->frame_start[f])
#define vithist_last_entry(vh,f)	((vh)->frame_start[f+1] - 1)


/* One-time intialization: Allocate and return an initially empty vithist module */
vithist_t *vithist_init (kbcore_t *kbc, int32 wbeam, int32 bghist);


/*
 * Invoked at the beginning of each utterance; vithist initialized with a root <s> entry.
 * Return value: Vithist ID of the root <s> entry.
 */
int32 vithist_utt_begin (vithist_t *vh, kbcore_t *kbc);


/*
 * Invoked at the end of each utterance; append a final </s> entry that results in the best
 * path score (i.e., LM including LM transition to </s>).
 * Return the ID of the appended entry if successful, -ve if error (empty utterance).
 */
int32 vithist_utt_end (vithist_t *vh, kbcore_t *kbc);


/*
 * Invoked at the end of each block of a live decode.
 * Returns viterbi histories of partial decodes
 */
int32 vithist_partialutt_end (vithist_t *vh, kbcore_t *kbc);

/* Invoked at the end of each utterance to clear up and deallocate space */
void vithist_utt_reset (vithist_t *vh);


/*
 * Viterbi backtrace.  Return value: List of hyp_t pointer entries for the individual word
 * segments.  Caller responsible for freeing the list.
 */
glist_t vithist_backtrace (vithist_t *vh,
			   int32 id);		/* ID from which to begin backtrace */

/*
 * Return ptr to entry corresponding to the given vithist ID.
 */
vithist_entry_t *vithist_id2entry (vithist_t *vh, int32 id);


/*
 * Like vithist_enter, but LM-rescore this word exit wrt all histories that ended at the
 * same time as the given, tentative pred.  Create a new vithist entry for each predecessor
 * (but, of course, only the best for each distinct LM state will be retained; see above).
 */
void vithist_rescore (vithist_t *vh,
		      kbcore_t *kbc,
		      s3wid_t wid,
		      int32 ef,		/* In: End frame for this word instance */
		      int32 score,	/* In: Does not include LM score for this entry */
		      int32 pred,	/* In: Tentative predecessor */
		      int32 type);

/* Invoked at the end of each frame */
void vithist_frame_windup (vithist_t *vh,	/* In/Out: Vithist module to be updated */
			   int32 frm,		/* In: Frame in which being invoked */
			   FILE *fp,		/* In: If not NULL, dump vithist entries
						   this frame to the file (for debugging) */
			   kbcore_t *kbc);	/* In: Used only for dumping to fp, for
						   debugging */

/*
 * Mark up to maxwpf best words, and variants within beam of best frame score as valid,
 * and the remaining as invalid.
 */
void vithist_prune (vithist_t *vh,
		    dict_t *dict,	/* In: Dictionary, for distinguishing filler words */
		    int32 frm,		/* In: Frame in which being invoked */
		    int32 maxwpf,	/* In: Max unique words per frame to be kept valid */
		    int32 maxhist,	/* In: Max histories to maintain per frame */
		    int32 beam);	/* In: Entry score must be >= frame bestscore+beam */

/*
 * Dump the Viterbi history data to the given file (for debugging/diagnostics).
 */
void vithist_dump (vithist_t *vh,
		   int32 frm,		/* In: If >= 0, print only entries made in this frame,
					   otherwise print all entries */
		   kbcore_t *kbc,
		   FILE *fp);

/*
 * Write a word lattice file (that can be input as -inlatdir argument to old s3 decoder).
 * Note: The header must be written before this function is called.
 */
void vithist_dag_write (vithist_t *vh,	/* In: From which word segmentations are to be dumped */
			glist_t hyp,	/* In: Some word segments can be pruned; however, but
					 * those in hyp always retained */
			dict_t *dict,	/* In: Dictionary; for generating word string names */
			int32 oldfmt,	/* In: If TRUE, old format, edges: srcnode dstnode ascr;
					 * else new format, edges: srcnode endframe ascr */
			FILE *fp);	/* Out: File to be written */

#endif
