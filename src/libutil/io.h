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
 * io.h -- Packaged I/O routines.
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
 * 08-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added stat_mtime().
 * 
 * 11-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added _myfopen() and myfopen macro.
 * 
 * 05-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBUTIL_IO_H_
#define _LIBUTIL_IO_H_

#include <stdio.h>
#include <sys/stat.h>
#include "prim_type.h"

/** \file io.h
 * \brief file IO related operations.  
 *
 * Custom fopen with error checking is implemented. fopen_comp can
 * open a file with .z, .Z, .gz or .GZ extension
 *  
 * \Warning Usage of stat_retry will results in 100s of waiting time
 * if the file doesn't exist.  
*/

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * Like fopen, but use popen and zcat if it is determined that "file" is compressed
   * (i.e., has a .z, .Z, .gz, or .GZ extension).
   */
FILE *fopen_comp (char *file,		/* In: File to be opened */
		  char *mode,		/* In: "r" or "w", as with normal fopen */
		  int32 *ispipe);	/* Out: On return *ispipe is TRUE iff file
					   was opened via a pipe */

  /**
   * Close a file opened using fopen_comp.
   */
void fclose_comp (FILE *fp,		/* In: File pointer to be closed */
		  int32 ispipe);	/* In: ispipe argument that was returned by the
					   corresponding fopen_comp() call */

  /**
   * Open a file for reading, but if file not present try to open compressed version (if
   * file is uncompressed, and vice versa).
   */
FILE *fopen_compchk (char *file,	/* In: File to be opened */
		     int32 *ispipe);	/* Out: On return *ispipe is TRUE iff file
					   was opened via a pipe */

  /**
   * Wrapper around fopen to check for failure and E_FATAL if failed.
   */
FILE *_myfopen(char *file, char *mode,
	       char *pgm, int32 line);	/* In: __FILE__, __LINE__ from where called */
#define myfopen(file,mode)	_myfopen((file),(mode),__FILE__,__LINE__)


  /**
   * NFS file reads seem to fail now and then.  Use the following functions in place of
   * the regular fread.  It retries failed freads several times and quits only if all of
   * them fail.  Be aware, however, that even normal failures such as attempting to read
   * beyond EOF will trigger such retries, wasting about a minute in retries.
   * Arguments identical to regular fread.
   */
int32 fread_retry(void *pointer, int32 size, int32 num_items, FILE *stream);


  /**
   * Like fread_retry, but for stat.  Arguments identical to regular stat.
   * Return value: 0 if successful, -1 if stat failed several attempts.
   */
int32 stat_retry (char *file, struct stat *statbuf);

  /**
   * Return time of last modification for the given file, or -1 if stat fails.
   */

int32 stat_mtime (char *file);

#ifdef __cplusplus
}
#endif

#endif
