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
#ifndef _UTT_H_
#define _UTT_H_

#include <s3types.h>
#include "kb.h"
#include "approx_cont_mgau.h"
#include "corpus.h"

#ifdef __cplusplus
extern "C" {
#endif

  /** \file utt.h
      \brief high level caller of the Viterbi algorithm
   */

#define MAXHYPLEN       1000



  /** Begin utterance */
  void utt_begin (kb_t *kb /**< A kb */
		);

    /** End utterance */
  void utt_end (kb_t *kb     /**< A kb */
		);

   /** Decoding the whole utterance */
  void utt_decode (void *data,  /**< A kb */
		 utt_res_t *ur, /**< Utterance resource structure */
		 int32 sf,  /**< Starting frame of the decoding */
		 int32 ef,  /**< Ending frame of the decoding */
		 char *uttid /**< Utterance ID */
		 );

  
  /** This function decodes a block of incoming feature vectors.
 * Feature vectors have to be computed by the calling routine.
 * The utterance level index of the last feature vector decoded
 * (before the current block) must be passed.
 * The current status of the decode is stored in the kb structure that
 * is passed in.
 */

void utt_decode_block (float ***block_feat,   /* Incoming block of featurevecs */
                       int32 block_nfeatvec, /* No. of vecs in cepblock */
                       int32 *curfrm,        /* Utterance level index of
                                                frames decoded so far */
                       kb_t *kb             /* kb structure with all model
                                                and decoder info */
		       );


#ifdef __cplusplus
}
#endif

#endif
