/*
 * beam.h -- Various forms of pruning beam
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
 * 19-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_BEAM_H_
#define _S3_BEAM_H_


#include <libutil/libutil.h>


/*
 * Structure containing various beamwidth parameters.  All logs3 values; -infinite is widest,
 * 0 is narrowest.
 */
typedef struct {
    int32 subvq;	/* For selecting active mixture components based on subvq scores */
    int32 hmm;		/* For selecting active HMMs, relative to best */
    int32 ptrans;	/* For determining which HMMs transition to their successors */
    int32 word;		/* For selecting words exited, relative to best HMM score */
} beam_t;


/*
 * Create and initialize a beam_t structure, with the given parameters, converting them
 * from prob space to logs3 space.  Return value: ptr to created structure if successful,
 * NULL otherwise.
 */
beam_t *beam_init (float64 svq, float64 hmm, float64 ptr, float64 wd);


#endif
