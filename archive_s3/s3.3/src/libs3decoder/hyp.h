/*
 * hyp.h -- Hypothesis structures.
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
 * 01-Jun-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _S3_HYP_H_
#define _S3_HYP_H_


#include <libutil/libutil.h>


typedef struct {
    int32 id;		/* Token ID; could be s3wid_t, s3cipid_t...  Interpreted by client. */
    int32 vhid;		/* Viterbi history (lattice) ID from which this entry created */
    int32 sf, ef;	/* Start/end frames, inclusive, for this segment */
    int32 ascr;		/* Segment acoustic score */
    int32 lscr;		/* LM score for transition to this segment (if applicable) */
    int32 type;		/* Uninterpreted data; see vithist_entry_t in vithist.h */
} hyp_t;


#endif
