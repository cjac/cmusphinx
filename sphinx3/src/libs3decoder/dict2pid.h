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
 * dict2pid.h -- Triphones for dictionary
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
 * 14-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dict2pid_comsseq2sen_active().
 * 
 * 04-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_DICT2PID_H_
#define _S3_DICT2PID_H_

#include "dict.h"


  /** \file dict2pid.h
   * \brief Building triphones for a dictionary. 
   *
   * This is one of the more complicated parts of a cross-word
   * triphone model decoder.  The first and last phones of each word
   * get their left and right contexts, respectively, from other *
   * words.  For single-phone words, both its contexts are from other
   * words, simultaneously.  As * these words are not known
   * beforehand, life gets complicated.  In this implementation, when
   * * we do not wish to distinguish between distinct contexts, we use
   * a COMPOSITE triphone (a bit * like BBN's fast-match
   * implementation), by clubbing together all possible contexts.
   * 
   * There are 3 cases:
   *   1. Internal phones, and boundary phones without any specific context, in each word.  The
   * 	boundary phones are modelled using composite phones, internal ones using ordinary phones.
   *   2. The first phone of a multi-phone word, for a specific history (i.e., in a 2g/3g/4g...
   *	tree) has known left and right contexts.  The possible left contexts are limited to the
   *	possible last phones of the history.  So it can be modelled separately, efficiently, as
   *	an ordinary triphone.
   *   3. The one phone in a single-phone word, for a specific history (i.e., in a 2g/3g/4g...
   * 	tree) has a known left context, but unknown right context.  It is modelled using a
   * 	composite triphone.
   * (Note that right contexts are always composite, left contexts are composite only in the
   * unigram tree.)
   * 
   * A composite triphone is formed as follows.  (NOTE: this assumes
   * that all CIphones/triphones have the same HMM topology,
   * specifically, no. of states.)  A composite triphone represents a
   * situation where either the left or the right context (or both)
   * for a given base phone is unknown.  That is, it represents the
   * set of all possible ordinary triphones derivable from * the
   * unkown context(s).  Let us call this set S.  It is modelled using
   * the same HMM topology * as the ordinary triphones, but with
   * COMPOSITE states.  A composite state (in a given position * in
   * the HMM state topology) is the set of states (senones) at that
   * position derived from S.
   * 
   * Actually, we generally deal with COMPOSITE SENONE-SEQUENCES rather than COMPOSITE PHONES.
   * The former are compressed forms of the latter, by virtue of state sharing among phones.
   * (See mdef.h.)  
   */

#ifdef __cplusplus
extern "C" {
#endif



typedef struct {
    s3ssid_t **internal;	/** For internal phone positions (not first, not last), the
				   ssid; for first and last positions, the composite ssid.
				   ([word][phone-position]) */
    s3ssid_t ***ldiph_lc;	/** For multi-phone words, [base][rc][lc] -> ssid; filled out for
				   word-initial base x rc combinations in current vocabulary */
    s3ssid_t **single_lc;	/** For single phone words, [base][lc] -> composite ssid; filled
				   out for single phone words in current vocabulary */
    
    s3senid_t **comstate;	/** comstate[i] = BAD_S3SENID terminated set of senone IDs in
				   the i-th composite state */
    s3senid_t **comsseq;	/** comsseq[i] = sequence of composite state IDs in i-th
				   composite phone (composite sseq). */
    int32 *comwt;		/** Weight associated with each composite state (logs3 value).
				   Final composite state score weighted by this amount */
    int32 n_comstate;		/** #Composite states */
    int32 n_comsseq;		/** #Composite senone sequences */
} dict2pid_t;

  /** Access macros; not designed for arbitrary use */
#define dict2pid_internal(d,w,p)	((d)->internal[w][p])
#define dict2pid_n_comstate(d)		((d)->n_comstate)
#define dict2pid_n_comsseq(d)		((d)->n_comsseq)


  /** Build the dict2pid structure for the given model/dictionary */
dict2pid_t *dict2pid_build (mdef_t *mdef, dict_t *dict);


  /**
 * Compute composite senone scores from ordinary senone scores (max of component senones)
 */
void dict2pid_comsenscr (dict2pid_t *d2p,
			 int32 *senscr,		/* In: Ordinary senone scores */
			 int32 *comsenscr);	/* Out: Composite senone scores */

  /** 
 * Mark active senones as indicated by the input array of composite senone-sequence active flags.
 * Caller responsible for allocating and clearing sen[] before calling this function.
 */
void dict2pid_comsseq2sen_active (dict2pid_t *d2p,
				  mdef_t *mdef,
				  int32 *comssid,	/* In: Active flag for each comssid */
				  int32 *sen);		/* In/Out: Active flags set for senones
							   indicated by the active comssid */

  /** For debugging */
void dict2pid_dump (FILE *fp, dict2pid_t *d2p, mdef_t *mdef, dict_t *dict);

#ifdef __cplusplus
}
#endif


#endif
