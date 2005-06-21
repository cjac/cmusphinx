 /* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights
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
/*************************************************
 * CMU CALO Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * 11-Jul-2004  Ziad Al Bawab (ziada@cs.cmu.edu) at Carnegie Mellon University
 * Created
 * HISTORY
 * $Log$
 * Revision 1.5  2005/06/21  21:12:05  arthchan2003
 * Added some bogus comments to endptr.h
 * 
 * Revision 1.4  2005/06/21 21:07:28  arthchan2003
 * Added  keyword.
 *
 * Revision 1.2  2005/06/15 06:48:54  archan
 * Sphinx3 to s3.generic: 1, updated the endptr and classify 's code, 2, also added
 *
 */

#include "fe.h"
#include "classify.h"

/** \file endptr.h
    \brief Wrapper of the end-pointer. 
 */
#ifdef __cplusplus
 extern "C" {
#endif 

#ifndef __END_POINTER__
#define __END_POINTER__

/** Struct to hold the end-pointer parameters */

typedef struct{
  int     status;                 /** current status of recording */
  int     leader;                 /** pointer to the start of the utterance to be passed to decoder*/
  int     spbegin;                /** pointer to the start of speech */
  int     spend;                  /** pointer to the end of speech */
  int     trailer;                /** pointer to the end of the utterance passed to the decoder */
  int 	utt_counter;		/** to count the number of utterances in a meeting */
  
  int     PAD_F_BEFORE;           /** to pad this much of frames before spbegin */
  int     PAD_F_AFTER;            /** to pad this much of frames after spend   */
  int     UTT_F_START;            /** to announce STAT_SPEECH after this much of speech frames */
  int     UTT_F_END;              /** to end an utterance after this much of non-speech frames */
  int     UTT_F_CANCEL;           /** to cancel an utterance after this much of non-speech frames */

} endpointer_t;


   /** Status of the recording */

#define STAT_OTHER      0               /** non-speech period, noise, silence, or secondary speech */
#define STAT_BEGIN      1               /** beginning of speech period, starts with first speech frame */
#define STAT_SPEECH     2               /** inter-speech period, once */
#define STAT_END        3               /** end of speech period */
#define STAT_CANCEL     4               /** non-speech frames while an utterances is in STAT_BEGIN*/

/* End-Pointing parameters */

#define PAD_T_BEFORE    0.15    /** to pad this much of seconds before spbegin*/
#define PAD_T_AFTER     0.2    /** to pad this much of seconds after spend */
#define UTT_T_START     0.08    /** to announce STAT_SPEECH after this much of speech seconds */
#define UTT_T_END       0.30    /** to end an utterance after this much of non-speech seconds */
#define UTT_T_CANCEL    0.05    /** to cancel an utterance after this much of non-speech seconds */

   /*#define PAD_T_AFTER     0.15    // to pad this much of seconds after spend
     #define UTT_T_END       0.20    // to end an utterance after this much of non-speech seconds 
     #define UTT_T_CANCEL    0.08    // to cancel an utterance after this much of non-speech seconds */


/** Initialize the end pointer */
   endpointer_t * endpointer_initialize(fe_t *FE /**< A FE structure */
				     );

/** Free the end pointer */
   void endpointer_free(endpointer_t *ENDPTR /**< An end pointer structure */
		     );

   void endpointer_update_stat (endpointer_t *ENDPTR, /**< An end pointer structure */
				fe_t *FE,  /**< A FE structure */
				class_t *CLASSW,  /**< A CLASSW structure */
				int class /**< The class*/
			     );

/** Convert frames 2 seconds for beginning frame.*/
   float endptr_frame2secs_beg (fe_t *FE, /**<A FE structure */
				int frame  /**< The frame*/
			     );

/** Convert frames 2 seconds for ending frame. */
float endptr_frame2secs_end (fe_t *FE, /**<A FE structure */
			     int frame /**< The frame */
			     );

#endif /*__END_POINTER__*/

#ifdef __cplusplus
 }
#endif 

