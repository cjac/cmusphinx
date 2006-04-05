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

/* gmm_wrap.h
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.3  2006/02/23 05:38:39  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH: Added multi-stream GMM computation routine.
 *
 *
 * Revision 1.1.4.3  2005/08/02 21:31:35  arthchan2003
 * Added interface for 1, doing multi stream gmm computation with/without composite senone. 2, doing gmm computation (ms or ss optimized) with/wihout composite senone.  Haven't tested on the SCHMM on s3.x yet.  I think it will work though.
 *
 * Revision 1.1.4.2  2005/07/24 01:35:41  arthchan2003
 * Add a wrapper for computing senone score without computing composite senone score. Mainly used in mode FSG now
 *
 * Revision 1.1.4.1  2005/06/27 05:30:25  arthchan2003
 * Merge from the tip of the trunk
 *
 * Revision 1.2  2005/06/22 08:00:09  arthchan2003
 * Completed all doxygen documentation on file description for libs3decoder/libutil/libs3audio and programs.
 *
 * Revision 1.1  2005/06/21 22:48:14  arthchan2003
 * A wrapper that provide the function pointer interface of approx_cont_mgau_ci_eval  and approx_cont_mgau_frame_eval.  They are used in srch_gmm_compute_lv1  and srch_gmm_compute_lv2 respectively.  This will also be the home of other gmm computation routine. (Say the s3.0 version of GMM computation)
 *
 * Revision 1.2  2005/06/13 04:02:59  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.1  2005/04/22 04:22:37  archan
 * Add gmm_wrap, this will share code across op_mode 4 and op_mode 5. Also it also separate active senone selection into a different process.  I hope this is the final step before making the WST search works.  At the current stage, the code of mode-5 looks very much alike mode-4.  This is intended because in Prototype 4, tail sharing will be used to reduce memory.
 *
 */

/** \file gmm_wrap.h
 *  \brief Function pointer wrapper of GMM computation. 
 */
#include <s3types.h>
#include "approx_cont_mgau.h"

int32 approx_ci_gmm_compute(void *srch,  /**< a pointer to a srch_t */
			    float32 *feat,  /**< feature vector */
			    int32 cache_idx, /**< cache index */
			    int32 wav_idx    /**< frame index */
			    );

/**
   This wrapper calls the approximate GMM computation routine which
   compute the senone score.  Then the composite senone will also be
   computed. 
 */
int32 approx_cd_gmm_compute_sen_comp(
				     void *srch, /**< a pointer to a srch_t */
				     float32 **feat, /**< feature vector */
				     int32 wav_idx  /**< frame index */
				     );

/**
   This wrapper that calls the approximate GMM computation routine which 
   compute only normal senone. 
 */
int32 approx_cd_gmm_compute_sen(
				void *srch, /**< a pointer to a srch_t */
				float32 **feat, /**< feature vector #stream x #coeff*/
				int32 wav_idx  /**< frame index */
				);

/**
   This wrapper calls the multi-stream exact GMM computation routine
   which compute the senone score.  Then the composite senone will also be
   computed.
 */

int32 ms_cd_gmm_compute_sen_comp(
				 void *srch,   /**< a pointer to a srch_t */
				 float32 **feat,  /**< feature vector #stream x #coeff*/
				 int32 wav_idx    /**< frame index */
				 );

/**
   This wrapper that calls the approximate GMM computation routine which 
   compute only normal senone. 
 */

int32 ms_cd_gmm_compute_sen(
			    void *srch,   /**< a pointer to a srch_t */
			    float32 **feat,  /**< feature vector #stream x #coeff*/
			    int32 wav_idx    /**< frame index */
			    );

/**
   Depends on which data structure has been initialized, s3_cd_gmm_compute_sen_comp
   calls
   ms_cd_gmm_compute_sen_comp (if ms_mgau is initialized) or
   approx_cd_gmm_coupute_sen_compu (if _mgau is initialized)

   Then composite triphone will also be computed. 
 */

int32 s3_cd_gmm_compute_sen_comp(
				 void *srch,   /**< a pointer to a srch_t */
				 float32 **feat,  /**< feature vector #stream x #coeff*/
				 int32 wav_idx    /**< frame index */

				 );

/**
   Depends on which data structure has been initialized, s3_cd_gmm_compute_sen
   calls
   ms_cd_gmm_compute_sen (if ms_mgau is initialized) or
   approx_cd_gmm_coupute_sen (if _mgau is initialized)

   Only normal senone will be computed. 
 */


int32 s3_cd_gmm_compute_sen(
			    void *srch,   /**< a pointer to a srch_t */
			    float32 **feat,  /**< feature vector #stream x #coeff*/
			    int32 wav_idx    /**< frame index */
			    );

