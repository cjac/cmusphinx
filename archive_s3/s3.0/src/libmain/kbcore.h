/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * 
 * HISTORY
 * 
 * 28-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

#ifndef _LIBMAIN_KBCORE_H_
#define _LIBMAIN_KBCORE_H_


#include <libutil/libutil.h>
#include <libfeat/libfeat.h>

#include "mdef.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "gauden.h"
#include "senone.h"
#include "tmat.h"
#include "wid.h"


typedef struct {
    feat_t *fcb;
    mdef_t *mdef;
    dict_t *dict;
    lm_t *lm;
    fillpen_t *fillpen;
    tmat_t *tmat;
    s3lmwid_t *dict2lmwid;
    gauden_t *gau;
    senone_t *sen;
} kbcore_t;


/*
 * Initialize one or more of all the major models:  pronunciation dictionary, acoustic models,
 * language models.  Several arguments are optional (e.g., pointers may be NULL); however, they
 * may not all be independently so.  The logbase argument is required (i.e. must be valid).
 * A number of consistency verifications are carried out.  It's recommended that a reasonable
 * default be provided for any real or integer argument, even if it's unused.
 * Return value: obvious.
 * NOTE: If any model fails to initialize, the call fails with a FATAL ERROR.
 */
kbcore_t *kbcore_init (float64 logbase,		/* Must be specified */
		       char *feattype,
		       char *mdeffile,
		       char *dictfile,
		       char *fdictfile,
		       char *compsep,		/* Must be valid if dictfile specified */
		       char *lmfile,
		       char *fillpenfile,
		       float64 silprob,		/* Must be valid if lmfile/fillpenfile is
						   specified */
		       float64 fillprob,	/* Must be valid if lmfile/fillpenfile is
						   specified */
		       float64 langwt,		/* Must be valid if lmfile/fillpenfile is
						   specified. */
		       float64 inspen,		/* Must be valid if lmfile/fillpenfile is
						   specified. */
		       char *meanfile,
		       char *varfile,		/* Must be specified if meanfile specified */
		       float64 varfloor,	/* Must be valid if meanfile specified */
		       char *sen2mgau,		/* Must be specified if mixwfile specified */
		       char *mixwfile,
		       float64 mixwfloor,	/* Must be valid if mixwfile specified */
		       char *tmatfile,
		       float64 tmatfloor);	/* Must be valid if tmatfile specified */

#endif
