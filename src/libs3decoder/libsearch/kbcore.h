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
 * kbcore.h -- Structures for maintain the main models.
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
 * 11-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Removed svqpp stuff.  It doesn't work too well anyway.
 * 
 * 06-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added kb_t.svqpp_t and related handling.
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _S3_KBCORE_H_
#define _S3_KBCORE_H_

#include <libutil/libutil.h>
#include "feat.h"
#include "cont_mgau.h"
#include "mdef.h"
#include "dict.h"
#include "dict2pid.h"
#include "fillpen.h"
#include "lm.h"
#include "wid.h"
#include "tmat.h"
#include "subvq.h"
#include "gs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    feat_t *fcb;
    mdef_t *mdef;
    dict_t *dict;
    dict2pid_t *dict2pid;
    lm_t *lm;
    lmset_t *lmset;
    fillpen_t *fillpen;
    s3lmwid_t *dict2lmwid;
    mgau_model_t *mgau;
    subvq_t *svq;
    gs_t *gs;
    tmat_t *tmat;
  int32 n_lm;
  int32 n_alloclm;
} kbcore_t;


/*
 * Initialize one or more of all the major models:  pronunciation dictionary, acoustic models,
 * language models.  Several arguments are optional (e.g., pointers may be NULL); however, they
 * may not all be independently so.  The logbase argument is required (i.e. must be valid).
 * A number of consistency verifications are carried out.  It's recommended that a reasonable
 * default be provided for any real or integer argument, even if it's unused.
 * Return value: (obvious) pointer to structure encapsulating the individual data bases.
 * NOTE: If any model fails to initialize, the call fails with a FATAL ERROR.
 */
kbcore_t *kbcore_init (float64 logbase,		/* Must be specified */
		       char *feattype,
		       char *cmn,
		       char *varnorm,
		       char *agc,
		       char *mdeffile,
		       char *dictfile,
		       char *fdictfile,
		       char *compsep,		/* Must be valid if dictfile specified */
		       char *lmfile,
		       char *lmctlfile,
		       char *lmdumpdir,
		       char *fillpenfile,
		       char *senmgau,
		       float64 silprob,		/* Must be valid if lmfile/fillpenfile is
						   specified */
		       float64 fillprob,	/* Must be valid if lmfile/fillpenfile is
						   specified */
		       float64 langwt,		/* Must be valid if lmfile/fillpenfile is
						   specified. */
		       float64 inspen,		/* Must be valid if lmfile/fillpenfile is
						   specified. */
		       float64 uw,		/* Must be valid if lmfile/fillpenfile is
						   specified. */
		       char *meanfile,		/* Acoustic model... */
		       char *varfile,		/* Must be specified if meanfile specified */
		       float64 varfloor,	/* Must be valid if varfile specified */
		       char *mixwfile,		/* Must be specified if meanfile specified */
		       float64 mixwfloor,	/* Must be valid if mixwfile specified */
		       char *subvqfile,		/* Subvector quantized acoustic model
						   (quantized mean/var values), optional */
		       char *gsfile,		/* Gaussian Selection Mapping*/
		       char *tmatfile,
		       float64 tmatfloor);	/* Must be valid if tmatfile specified */

  void kbcore_free (kbcore_t *kbcore);


/* Access macros; not meant for arbitrary use */
#define kbcore_fcb(k)		((k)->fcb)
#define kbcore_mdef(k)		((k)->mdef)
#define kbcore_dict(k)		((k)->dict)
#define kbcore_dict2pid(k)	((k)->dict2pid)
#define kbcore_lm(k)		((k)->lm)
#define kbcore_fillpen(k)	((k)->fillpen)
#define kbcore_dict2lmwid(k,w)	((k)->dict2lmwid[w])
#define kbcore_mgau(k)		((k)->mgau)
#define kbcore_svq(k)		((k)->svq)
#define kbcore_gs(k)		((k)->gs)
#define kbcore_tmat(k)		((k)->tmat)
#define kbcore_lmset(k)		((k)->lmset)
#define kbcore_nlm(k)		((k)->n_lm)

#ifdef __cplusplus
}
#endif

#endif
