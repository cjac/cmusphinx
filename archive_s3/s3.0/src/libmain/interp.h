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
 * interp.h -- CD-senone and CI-senone score interpolation
 * 
 * 
 * HISTORY
 * 
 * 18-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started based on original S3 implementation.
 */


#ifndef _LIBMAIN_INTERP_H_
#define _LIBMAIN_INTERP_H_


#include "s3types.h"


typedef struct {
    int32 n_sen;	/* #senones */
    struct interp_wt_s {
	int32 cd;	/* logs3(CD senone weight) */
	int32 ci;	/* logs3(1 - cd) */
    } *wt;		/* wt[i] = interpolation weight for senone i */
} interp_t;


/*
 * Read a set of CD/CI senone interpolation weights from the given file.
 * Return value: pointer to interpolation structure created.  Caller MUST NOT change its
 * contents.
 */
interp_t *interp_init (char *interpfile);	/* In: interpolation weights file */

/*
 * Interpolate a single given CD senone with the given CI senone score.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 interp_cd_ci (interp_t *ip,	/* In: Interpolation weights parameters */
		    int32 *senscr,	/* In/Out: senscr[cd] interpolated with senscr[ci] */
		    int32 cd,		/* In: see senscr above */
		    int32 ci);		/* In: see senscr above */

/*
 * Interpolate each CD senone with its corresponding CI senone score.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 interp_all (interp_t *ip,		/* In: Interpolation weights parameters */
		  int32 *senscr,	/* In/Out: senscr[cd] interpolated with
					   senscr[cd2cimap[cd]], for cd >= n_ci_sen */
		  s3senid_t *cd2cimap,	/* In: see senscr above */
		  int32 n_ci_sen);	/* In: see senscr above */

#endif
