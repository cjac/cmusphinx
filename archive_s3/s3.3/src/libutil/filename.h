/*
 * filename.h -- File and path name operations.
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
 * 30-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBUTIL_FILENAME_H_
#define _LIBUTIL_FILENAME_H_


#include "prim_type.h"


/*
 * Strip off leading path components from the given path and copy the base into base.
 * Caller must have allocated base.
 */
void path2basename (char *path, char *base);


/*
 * Strip off the smallest trailing file-extension suffix and copy the rest into the
 * given root argument.  Caller must have allocated root.
 */
void strip_fileext (char *file, char *root);


#endif
