/*
 * dp.h -- DP alignment core routines.
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
 * 07-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _DP_H_
#define _DP_H_


#include <libutil/libutil.h>
#include <main/s3types.h>
#include <main/dict.h>
#include "dag.h"


typedef struct dpnode_s {
    int32 e, c;		/* Total #error and #correct in best path to this node */
    int16 pr, pc;	/* Predecessor (row(seqid) and col(refword)) */
} dpnode_t;


/*
 * General DP alignment of a DAG with a linear reference "string".
 * Both the reference array ref and the hypothesis DAG are assumed to terminate in the
 * same filler word (silence).
 * Return value: dpnode_t structure with e and c values filled in.  Also, *nhyp.
 * Note that filler words do not get insertion errors.
 */
dpnode_t dp (char *uttid,
	     dict_t *dict,
	     s3wid_t oovbegin,
	     dagnode_t *ref,
	     int32 nref,
	     dag_t *dag,
	     int32 *nhyp,
	     int32 use_time,
	     int32 backtrace);

#endif
