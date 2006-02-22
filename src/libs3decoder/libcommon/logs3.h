/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * $Log$
 * Revision 1.10  2006/02/22  20:31:53  arthchan2003
 * Add clarification comment on logs3.h
 * 
 * Revision 1.9.4.1  2006/01/16 19:51:19  arthchan2003
 * Added a function to convert Sphinx 3 log to log 10.
 *
 * Revision 1.9  2005/06/21 20:46:54  arthchan2003
 * 1, Added a report flag in logs3_init, 2, Fixed doxygen documentation, 3, Add the $ keyword.
 *
 * Revision 1.6  2005/06/13 04:02:57  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.5  2005/05/27 01:15:44  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added log_to_logs3_factor(), and logs3_to_p().
 * 
 * 05-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _S3_LOGS3_H_
#define _S3_LOGS3_H_

#include <s3types.h>

/** \file logs3.h
 * \brief s3 log table implementation. 
 *
 * In evaluating HMM models, probability values are often kept in log domain,
 * to avoid overflow.  Furthermore, to enable these logprob values to be held
 * in int32 variables without significant loss of precision, a logbase of
 * (1+epsilon), epsilon<<1, is used.  This module maintains this logbase, and
 * other functions to support it.
 *
 *
 */
#ifdef __cplusplus
extern "C" {
#endif


#define LOGS3_SUCCESS 1;
#define LOGS3_FAILURE 0;

  /** Initialize module with a new base (> 1.0).  Returns 0 if successful, -1 otherwise. */
  int32 logs3_init (float64 base, /**< The log base of the module */
		    int32 bReport,  /**< Whether progress should be reported */
		    int32 bLogTable /**< Whether log table should be used or not */
		    );

  /** Given logs3p, logs3q (ie, log-S3base of p and q), return logs3(p+q) */
  int32 logs3_add (int32 logs3p, /**< The first number in s3 log */
		   int32 logs3q  /**< The second number in s3 log */
		   );

  /** Given p, return logs3(p) */
  int32 logs3 (float64 p   /**< The first number in float */
	       );

  /** Given log(p), return logs3(p) */
  int32 log_to_logs3 (float64 logp /**< The number in log domain */
		      );

  /** Return the multiplication factor used for converting log values to logs3 */
  float64 log_to_logs3_factor( void );

  /** Given logs3(p), return log(p) */
  float64 logs3_to_log (int32 logs3p    /**< A number in log s3 domain*/
		      );

  /** Given logs3(p), return log10(p) */
  float64 logs3_to_log10 (int32 logs3p    /**< A number in log s3 domain*/
			  );

  /** Given logs3(p), return p */
  float64 logs3_to_p (int32 logs3p  /**< A number in log s3 domain*/
		      );

  /** Given log10(p), return logs3(p) */
  int32 log10_to_logs3 (float64 log10p /**< A number in log 10 domain */
			);

  /* RAH, free the add_tbl if still allocated */
  /** Free the add_tbl if still allocated */
  void logs_free();

  /** Report the parameters for s3 log table */
  void logs3_report();
  
  /** Return the base of s3. */
  float64 logs3_base();
  
  /** Compute log 10 in base of s3 */
  float64 logs3_10base(); 

#ifdef __cplusplus
}
#endif


#endif
