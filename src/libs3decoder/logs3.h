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
 * logs3.h -- log(base-S3) module.
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
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added log_to_logs3_factor(), and logs3_to_p().
 * 
 * 05-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _S3_LOGS3_H_
#define _S3_LOGS3_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <libutil/libutil.h>


/*
 * In evaluating HMM models, probability values are often kept in log domain,
 * to avoid overflow.  Furthermore, to enable these logprob values to be held
 * in int32 variables without significant loss of precision, a logbase of
 * (1+epsilon), epsilon<<1, is used.  This module maintains this logbase, and
 * other functions to support it.
 */


/* Initialize module with a new base (> 1.0).  Returns 0 if successful, -1 otherwise. */
int32 logs3_init (float64 base);

/* Given logs3p, logs3q (ie, log-S3base of p and q), return logs3(p+q) */
int32 logs3_add (int32 logs3p, int32 logs3q);

/* Given p, return logs3(p) */
int32 logs3 (float64 p);

/* Given log(p), return logs3(p) */
int32 log_to_logs3 (float64 logp);

/* Return the multiplication factor used for converting log values to logs3 */
float64 log_to_logs3_factor( void );

/* Given logs3(p), return log(p) */
float64 logs3_to_log (int32 logs3p);

/* Given logs3(p), return p */
float64 logs3_to_p (int32 logs3p);

/* Given log10(p), return logs3(p) */
int32 log10_to_logs3 (float64 log10p);

/* RAH, free the add_tbl if still allocated */
void logs_free();

#ifdef __cplusplus
}
#endif


#endif
