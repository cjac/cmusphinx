/*
 * libutil.h -- Collection of all other .h files in this directory; for brevity
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
 * 08-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added SLEEP_SEC macro.
 * 
 * 08-31-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _LIBUTIL_LIBUTIL_H_
#define _LIBUTIL_LIBUTIL_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef WIN32			/* RAH */
#include <unistd.h>
#endif /* RAH */
#include <math.h>

#include "prim_type.h"

#include "bitvec.h"
#include "case.h"
#include "ckd_alloc.h"
#include "cmd_ln.h"
#include "err.h"
#include "filename.h"
#include "glist.h"
#include "hash.h"
#include "heap.h"
#include "io.h"
#include "profile.h"
#include "str2words.h"
#include "unlimit.h"


#if (WIN32)
#define SLEEP_SEC(sec)	(0)			/* Why doesn't Sleep((sec)*1000) work? */
#else
#define SLEEP_SEC(sec)	sleep(sec)		/* sec must be integer */
#endif

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifndef M_PI
#define M_PI		3.1415926535897932385	/* For the pain-in-the-neck Win32 */
#endif
#define PI		M_PI


#endif
