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
 * logs3.c -- log(base-S3) module.
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


#include "logs3.h"
#include "s3types.h"

/* RAH, 5.9.2001, Add a means of controlling whether the add table is
   used or the value is simply computed Note, if the add tables are
   not to be used, they are still generated. I'll remove this portion
   later, for now I want to make that there are no problems 
*/
static int USE_LOG3_ADD_TABLE = 1;	
static float64 F = 0;		/* Set this global variable so we don't have to keep computing it in logs3_add() */

/*
 * In evaluating HMM models, probability values are often kept in log domain,
 * to avoid overflow.  Furthermore, to enable these logprob values to be held
 * in int32 variables without significant loss of precision, a logbase of
 * (1+epsilon), epsilon<<1, is used.  This module maintains this logbase (B).
 * 
 * More important, maintaining probabilities in log domain creates a problem when
 * adding two probability values: difficult in the log domain.
 * Suppose P = Q+R (0 <= P,Q,R,Q+R <= 1), and we have to compute:
 *   logB(P), given logB(Q) and logB(R).  Assume Q >= R.
 *   Let z = logB(P), x = logB(Q), y = logB(R).
 *   Therefore, B^z = B^x + B^y = B^x(1 + B^(y-x)).
 *   Therefore, z = x + logB(1+B^(y-x)).
 * Since the latter term only depends on y-x, and log probs are kept in integer
 * variables, it can be precomputed into a table for y-x = 0, -1, -2, -3... until
 * logB(1+B^(y-x)) = (int32) 0.
 */


static float64 B, logB, invlogB, invlog10B;
static uint16 *add_tbl = NULL;	/* See discussion above */
static int32 add_tbl_size;


int32 logs3_init (float64 base)
{
    int32 i, k;
    float64 d, t, f;


    USE_LOG3_ADD_TABLE = cmd_ln_int32 ("-log3table");

    E_INFO("Initializing logbase: %e (add table: %d)\n", base,USE_LOG3_ADD_TABLE);

    if (base <= 1.0)
	E_FATAL("Illegal logbase: %e; must be > 1.0\n", base);

    if (add_tbl) {
	if (B == base)
	    E_WARN("logs3_init() already done\n");
	else
	    E_FATAL("logs3_init() already done with base %e\n", B);
    }
    
    B = base;
    logB = log(base);
    invlogB = 1.0/logB;
    invlog10B = 1.0/log10(base);

    /* Create add-table for adding probs in log domain */

    k = (int32) (log(2.0)*invlogB + 0.5);
    if (k > 65535) {
	E_ERROR("Logbase too small: %e; needs int32 addtable[]\n", base);
	return -1;
    }

    d = 1.0;
    f = 1.0/B;
    F = 1.0/B;			/* RAH 5.9.01, set this global variable so that we don't have to compute it in logs3_add() */

    /* Figure out size of add-table requried */
    for (i = 0;; i++) {
	t = log(1.0+d)*invlogB;
	k = (int32) (t + 0.5);

#if 0
	if (((i%1000) == 0) || (k == 0))
	    printf ("%10d %10d %e\n", i, k, d);
#endif

	if (k == 0)
	    break;

	d *= f;
    }

    add_tbl_size = i+1;
    add_tbl = (uint16 *) ckd_calloc (i+1, sizeof(uint16));
    
    /* Fill add-table */
    d = 1.0;
    for (i = 0;; i++) {
	t = log(1.0+d)*invlogB;
	k = (int32) (t + 0.5);

	add_tbl[i] = k;

	if (k == 0)
	    break;

	d *= f;
    }
    
    E_INFO("Log-Add table size = %d\n", add_tbl_size);

    return 0;
}


int32 logs3_add (int32 logp, int32 logq)
{
    int32 d, r;
    
    assert(add_tbl != NULL);	/* Use assert to allow use of NDEBUG for efficiency */

    if (logp > logq) {
	d = logp - logq;
	r = logp;
    } else {
	d = logq - logp;
	r = logq;
    }
    /* RAH 5.9.01 If we allow the computation of values of d beyond
       the add_tbl_size, speed degrades quickly, for that reason,
       limit the calculations to the same range as the table. This
       seems wrong. Must think more about it */
    if (d < add_tbl_size)
      {
      if (USE_LOG3_ADD_TABLE) 
	r += add_tbl[d];
      else
	/* Do we need to be checking to see if the value is too large? small? */
	r += 0.5 + (float64) (log(1.0 + pow(F,d)) * invlogB); /* RAH, 5.9.01 - compute instead of looking it up */
      }

    return r;
}


int32 logs3 (float64 p)
{
    if (! add_tbl)
	E_FATAL("logs3 module not initialized\n");
    
    if (p <= 0.0) {
	E_WARN("logs3 argument: %e; using S3_LOGPROB_ZERO\n", p);
	return S3_LOGPROB_ZERO;
    }
    
    return ((int32) (log(p) * invlogB));
}


int32 log_to_logs3 (float64 logp)
{
    if (! add_tbl)
	E_FATAL("logs3 module not initialized\n");
    
    return ((int32) (logp * invlogB));
}


float64 log_to_logs3_factor ( void )
{
    return invlogB;
}


float64 logs3_to_log (int32 logs3p)
{
    if (! add_tbl)
	E_FATAL("logs3 module not initialized\n");
    
    return ((float64)logs3p * logB);
}


float64 logs3_to_p (int32 logs3p)
{
    return (exp((float64)logs3p * logB));
}


int32 log10_to_logs3 (float64 log10p)
{
    if (! add_tbl)
	E_FATAL("logs3 module not initialized\n");
    
    return ((int32) (log10p * invlog10B));
}

void logs_free ()
{
  if (add_tbl) 
    ckd_free ((void *) add_tbl);
}


#if _LOGS3_TEST_
main (int argc, char *argv[])
{
    float64 base;
    float64 p, q;
    int32 logp, logq, logpq;

    printf ("base: ");
    scanf ("%lf", &base);
    if (logs3_init (base) < 0)
	exit (-1);

    for (;;) {
	printf ("p,q: ");

	scanf ("%lf %lf", &p, &q);
	logp = logs3 (p);
	logq = logs3 (q);

	logpq = logs3_add (logp, logq);

	printf ("logB(p,q) = %d, %d\n", logp, logq);
	printf ("logB(p+q) = %d, expected %d\n", logpq, logs3 (p+q));
    }
}
#endif
