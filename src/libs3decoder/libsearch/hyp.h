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
 * $Log$
 * Revision 1.12  2006/02/24  12:43:18  arthchan2003
 * Fixed typedef issue of hyp_t and srch_hyp_t.
 * 
 * Revision 1.11  2006/02/23 15:13:16  arthchan2003
 * Typedef hyp_t to srch_hyp_t.  This makes libAPI compiled.
 *
 * Revision 1.10  2006/02/23 05:40:53  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH: Comment the hyp_t structure,  Now only defined srch_hyp_t struct
 *
 * Revision 1.9.4.2  2005/07/26 02:19:20  arthchan2003
 * Comment out hyp_t, change name of wid in srch_hyp_t to id.
 *
 * Revision 1.9.4.1  2005/07/22 03:38:37  arthchan2003
 * Change the code a little bit so it starts to really look like srch_hyp_t .
 *
 * Revision 1.9  2005/06/21 22:49:03  arthchan2003
 * Add  keyword.
 *
 * Revision 1.4  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 01-Jun-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _S3_HYP_H_
#define _S3_HYP_H_

#include <s3types.h>
#include "search.h"

/** \file hyp.h
 * \brief data structure for storing the hypothesis. 
 */

#ifdef __cplusplus
extern "C" {
#endif


  /*  typedef hyp_t srch_hyp_t;*/
#if 0
  /**
   * Structure to store hypothesis. 
   */
typedef struct {
    int32 id;		/* Token ID; could be s3wid_t, s3cipid_t...  Interpreted by client. */
    int32 vhid;		/* Viterbi history (lattice) ID from which this entry created */
    int32 sf;           /* Start/ frames, inclusive, for this segment */
    int32 ef;	        /* end/ frame */
    int32 ascr;		/* Segment acoustic score */
    int32 lscr;		/* LM score for transition to this segment (if applicable) */
    int32 senscale;	/* Segment acoustic score scaling factor */
    int32 type;		/* Uninterpreted data; see vithist_entry_t in vithist.h */
} hyp_t;
#endif

#ifdef __cplusplus
}
#endif

#endif
