/*
 * logs3.h -- log(base-S3) module.
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
 * 05-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _LIBFBS_LOGS3_H_
#define _LIBFBS_LOGS3_H_


/*
 * In evaluating HMM models, probability values are often kept in log domain,
 * to avoid overflow.  Furthermore, to enable these logprob values to be held
 * in int32 variables without significant loss of precision, a logbase of
 * (1+epsilon), epsilon<<1, is used.  This module maintains this logbase, and
 * other functions to support it.
 */


#include <libutil/prim_type.h>


/* Initialize module with a new base (> 1.0).  Returns 0 if successful, -1 otherwise. */
int32 logs3_init (float64 base);

/* Given logs3p, logs3q (ie, log-S3base of p and q), return logs3(p+q) */
int32 logs3_add (int32 logs3p, int32 logs3q);

/* Given p, return logs3(p) */
int32 logs3 (float64 p);

/* Given log(p), return logs3(p) */
int32 log_to_logs3 (float64 logp);

/* Given log(p), return logs3(p) */
float64 logs3_to_log (int32 logs3p);

/* Given log10(p), return logs3(p) */
int32 log10_to_logs3 (float64 log10p);


#endif
