/*
 * search.h -- All exported search-related functions and data structures.
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
 * $Log$
 * Revision 1.1  2000/04/24  09:39:41  lenzo
 * s3 import.
 * 
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *  		Added onlynodes argument to dag_dump().
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed fwd_sen_active to flag active senones instead of building a list
 * 		of them.
 *  
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_search().
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added function fwd_sen_active() to obtain list of active senones in
 * 		current frame.
 * 
 * 04-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBFBS_SEARCH_H_
#define _LIBFBS_SEARCH_H_

typedef struct hyp_s {
    char     *word;		/* READ-ONLY item!! */
    s3wid_t   wid;
    s3frmid_t sf;
    s3frmid_t ef;
    int32     ascr;
    int32     lscr;
    int32     pscr;
    float32   conf;		/* "Confidence measure" */
    struct hyp_s *next;
} hyp_t;


/* ---------------- Forward Viterbi search related functions ---------------- */


void fwd_init ( void );

void fwd_start_utt (char *uttid);	/* In, READ-ONLY argument: utterance id */

/*
 * Called at the beginning of a frame to build a list of active senones (any senone used
 * by active HMMs in that frame.
 * Return value: #active senones in the returned list senlist[].
 */
void fwd_sen_active (int8 *senlist,	/* Out: Upon return senlist[s] TRUE iff
					   s active in current frame.
					   Caller allocates senlist[] array. */
		      int32 n_sen);	/* In: Size of senlist[] array */

/*
 * One frame of forward Viterbi beam search.
 * Return value: best path score of all the states evaluated in the frame.
 */
int32 fwd_frame (int32 *senscr);/* Step search one frame forward;
				   senscr: In: array of senone scores this frame */

hyp_t *fwd_end_utt ( void );	/* Wind up utterance and return final result */


/* ---------------- DAG related functions ---------------- */


/* DAG built from forward pass lattice */
int32 dag_build ( void );

/* Dump DAG to file for postprocessing */
int32 dag_dump (char *dir, int32 onlynodes, char *uttid);

/* Best path search through DAG from fwdvit word lattice */
hyp_t *dag_search (char *uttid);

/* Destroy DAG */
int32 dag_destroy ( void );


#endif
