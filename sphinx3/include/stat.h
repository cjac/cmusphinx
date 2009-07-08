/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * stat.h -- statistics of the searching process, including timers and counters. 
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
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/22 20:01:06  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: 1, Take care of the situation when the hmm_eval =0 (but ugly). 2, Add a free function for stat_t.
 *
 * Revision 1.1.4.2  2005/07/05 06:25:40  arthchan2003
 * Fixed dox-doc.
 *
 * Revision 1.1.4.1  2005/07/03 22:56:51  arthchan2003
 * Add stat_free.
 *
 * Revision 1.1  2005/06/21 20:58:09  arthchan2003
 * Add a statistics inventory structure, it takes care of 1, counters, 2, timers. Interfaces are provided to allow convenient clearing and updating of structures
 *
 * Revision 1.6  2005/04/25 19:22:47  archan
 * Refactor out the code of rescoring from lexical tree. Potentially we want to turn off the rescoring if we need.
 *
 * Revision 1.5  2005/04/20 03:44:10  archan
 * Create functions for clear/update/report statistics.  It wraps up code which was slightly spaghatti-like in the past
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 24-Mar-2004 Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             start
 */


#ifndef _S3_STAT_
#define _S3_STAT_

#include <profile.h>
#include <s3types.h>
#include "sphinx3_export.h"

/** \file stat.h
 * \brief The wrapper structure for all statistics in sphinx 3.x
 */

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** 
    \struct stat_t
    \brief Structure to hold all statistics in Sphinx 3.x 
*/
typedef struct {
    /* All structure that measure the time and stuffs we computed */
    ptmr_t tm_sen;    /**< timer for senone computation */
    ptmr_t tm_srch;   /**< timer for search */
    ptmr_t tm_ovrhd; /**< timer for GMM computation overhead */
    ptmr_t tm;       /**< timer for the whole run time */
    
    int32 utt_hmm_eval; /**< Temporary Variable: HMM evaluated for utterance */
    int32 utt_sen_eval; /**< Temporary Variable: CD Senones evaluated for utterance */
    int32 utt_gau_eval; /**< Temporary Variable: CD Gaussians evaluated for utterance */
    int32 utt_cisen_eval; /**< Temporary Variable: CI Senones evaluated for utterance */
    int32 utt_cigau_eval; /**< Temporary Variable: CI Gaussians evaluated for utterance */
    int32 utt_wd_exit;    /**< Temporary Variable: Store the number of word exits */
 
    int32 nfr;                 /**< Temporary Variable: Number of frame of current utterance */
    
    float64 tot_sen_eval;	/**< Temporary Variable: Total CD Senones evaluated over the entire session */
    float64 tot_gau_eval;	/**< Temporary Variable: Total CD Gaussian densities evaluated over the entire session */
    float64 tot_ci_sen_eval;	/**< Temporary Variable: Total CI Senones evaluated over the entire session */
    float64 tot_ci_gau_eval;	/**< Temporary Variable: Total CI Senones evaluated over the entire session */
    float64 tot_hmm_eval;	/**< Temporary Variable: Total HMMs evaluated over the entire session */
    float64 tot_wd_exit;	/**< Temporary Variable: Total Words hypothesized over the entire session */


    int32 tot_fr;                 /**< Temporary Variable: The total number of frames that the
                                     recognizer has been
                                     recognized. Mainly for bookeeping.  */

} stat_t ;

/** Initialized the statistics structure 
    @return a statistics data structure
*/
stat_t* stat_init(void); 
  
/** Delete the memory of stat_init
 */
void stat_free(stat_t* st /**< A statistics data structure */
    );

/** Clear the utterance statistics */
void stat_clear_utt(
    stat_t* st /**< A statistics data structure */
    );
  
/** Clear the corpus statistics */
void stat_clear_corpus(
    stat_t* st /**< A statistics data structure */
    );

/** Update the corpus statistics with the utterance statistics */
void stat_update_corpus(
    stat_t* st /**< A statistics data structure */ 
    );
  
/** Report the utterance statistics */
void stat_report_utt(
    stat_t* st, /**< A statistics data structure */ 
    char * uttid /**< The utterance ID */
    );

/** Report the total statistics */
S3DECODER_EXPORT
void stat_report_corpus(
    stat_t * st /**< A statistics data structure */ 
    );


#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif

