/*
 * hyp.h -- Hypothesis structures.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 27-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_HYP_H_
#define _LIBMAIN_HYP_H_


typedef struct {
    int32 id;		/* Could be s3wid_t, s3cipid_t, etc.  To be interpreted only by
			   the client. */
    int32 sf, ef;	/* Start/end frames, inclusive, for this segment */
    int32 ascr;		/* Segment acoustic score */
    int32 lscr;		/* LM score for transition to this segment (if applicable) */
    int32 scr;		/* Total segment score (ascr+lscr) */
} hyp_t;


/*
 * Log the given hypothesis to the given file.  (Kind of a hack!  Might not be the right
 * interface for all situations.)
 */
void hyp_log (FILE *fp,		/* In/Out: File to log to */
	      glist_t hyplist,	/* In: glist of hyp nodes */
	      char *(*func)(void *kb, int32 id),
	   			/* In: Function that provides a string name for the ID in
				   Viterbi history nodes. */
	      void *kb);	/* In: Auxiliary data structure, if any, needed by the
				   above func */

/*
 * Free the given glist of hyp nodes, including the hyp nodes themselves.
 */
void hyp_myfree (glist_t hyplist);


#endif
