/*
 * interp.h -- CD-senone and CI-senone score interpolation
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
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
