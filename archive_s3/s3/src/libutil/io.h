/*
 * io.h -- Packaged I/O routines.
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
 * 05-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBUTIL_IO_H_
#define _LIBUTIL_IO_H_


#include <stdio.h>
#include <sys/stat.h>

#include "prim_type.h"


/*
 * Like fopen, but use popen and zcat if it is determined that "file" is compressed
 * (i.e., has a .z, .Z, .gz, or .GZ extension).
 */
FILE *fopen_comp (char *file,		/* In: File to be opened */
		  char *mode,		/* In: "r" or "w", as with normal fopen */
		  int32 *ispipe);	/* Out: On return *ispipe is TRUE iff file
					   was opened via a pipe */

/*
 * Close a file opened using fopen_comp.
 */
void fclose_comp (FILE *fp,		/* In: File pointer to be closed */
		  int32 ispipe);	/* In: ispipe argument that was returned by the
					   corresponding fopen_comp() call */

/*
 * Open a file for reading, but if file not present try to open compressed version (if
 * file is uncompressed, and vice versa).
 */
FILE *fopen_compchk (char *file,	/* In: File to be opened */
		     int32 *ispipe);	/* Out: On return *ispipe is TRUE iff file
					   was opened via a pipe */

/*
 * NFS file reads seem to fail now and then.  Use the following functions in place of
 * the regular fread.  It retries failed freads several times and quits only if all of
 * them fail.  Be aware, however, that even normal failures such as attempting to read
 * beyond EOF will trigger such retries, wasting about a minute in retries.
 * Arguments identical to regular fread.
 */
int32 fread_retry(void *pointer, int32 size, int32 num_items, FILE *stream);


/*
 * Like fread_retry, but for stat.  Arguments identical to regular stat.
 * Return value: 0 if successful, -1 if stat failed several attempts.
 */
int32 stat_retry (char *file, struct stat *statbuf);


#endif
