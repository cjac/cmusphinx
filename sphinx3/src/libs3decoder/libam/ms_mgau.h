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
 * ms_mgau.h -- Essentially a wrapper that wrap up gauden and
 * senone. It supports multi-stream. 
 *
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * HISTORY
 * $Log$
 * Revision 1.1.2.1  2005/07/20  19:37:09  arthchan2003
 * Added a multi-stream cont_mgau (ms_mgau) which is a wrapper of both gauden and senone.  Add ms_mgau_init and model_set_mllr.  This allow eliminating 600 lines of code in decode_anytopo/align/allphone.
 * 
 *
 *
 */

/** \file ms_mgau.h
 *
 * \brief (Sphinx 3.0 specific) A module that wraps up the code of
 * gauden and senone because they are closely related.  
 *
 * At the time at Sphinx 3.1 to 3.2, Ravi has decided to rewrite only
 * single-stream part of the code into cont_mgau.[ch].  This marks the
 * beginning of historical problem of having two sets of Gaussian
 * distribution computation routine, one for single-stream and one of
 * multi-stream.
 *
 * In Sphinx 3.5, when we figure out that it is possible to allow both
 * 3.0 family of tools and 3.x family of tools to coexist.  This
 * becomes one problem we found that very hard to reconcile.  That is
 * why we currently allow two versions of the code in the code
 * base. This is likely to change in the future.
 */


#ifndef _LIBFBS_MS_CONT_MGAU_H_
#define _LIBFBS_MS_CONT_MGAU_H_

#include <ms_gauden.h>
#include <ms_senone.h>
#include <interp.h>
#include <s3types.h>
#include <prim_type.h>
#include <linklist.h>
#include <feat.h>
#include <mdef.h>


/* Lists of senones sharing each mixture Gaussian codebook */
/* \struct mgau2sen_t
   \brief a mapping from gaussian to senone
 */
typedef struct mgau2sen_s {
    s3senid_t sen;		/* Senone shared by this mixture Gaussian */
    struct mgau2sen_s *next;	/* Next entry in list for this mixture Gaussian */
} mgau2sen_t;

/** \struct ms_mgau_t
    \brief Multi-stream mixture gaussian. It is not necessary to be continr
 */

typedef struct {
  gauden_t* g;   /**< The codebook */
  senone_t* s;   /**< The senone */
  mgau2sen_t **mgau2sen; /**< Senones sharing mixture Gaussian codebooks */
  interp_t* i;   /**< The interplotion weight file */
  int32 topn;    /**< Top-n gaussian will be computed */
} ms_mgau_model_t;  

#define ms_mgau_gauden(msg) (msg->g)
#define ms_mgau_senone(msg) (msg->s)
#define ms_mgau_interp(msg) (msg->i)
#define ms_mgau_mgau2sen(msg) (msg->mgau2sen)
#define ms_mgau_topn(msg) (msg->topn)

ms_mgau_model_t* ms_mgau_init (char *meanfile,	/**< In: File containing means of mixture gaussians */
			       char *varfile,	/**< In: File containing variances of mixture gaussians */
			       float64 varfloor,	/**< In: Floor value applied to variances; e.g., 0.0001 */
			       char *mixwfile,	/**< In: File containing mixture weights */
			       float64 mixwfloor,	/**< In: Floor value for mixture weights; e.g., 0.0000001 */
			       char* senmgau,	/**< In: type of the gaussians distribution, .cont. or .semi. FIX 
						   me! This is confusing!*/
			       char* lambdafile, /**< In: Interplation file */
			       int32 topn        /**< In: Top-n gaussian will be computed */
			       );


int32 model_set_mllr(ms_mgau_model_t* msg,
		     const char *mllrfile, 
		     const char *cb2mllrfile,
		     feat_t* fcb,
		     mdef_t *mdef
		     );


#endif /* _LIBFBS_MS_CONT_MGAU_H_*/

