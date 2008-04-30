/* ====================================================================
 * Copyright (c) 1999-2006 Carnegie Mellon University.  All rights
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


/* Open for input a file which may potentially be Z compressed */
/* If pathanme ends in ".Z", assume the file is compressed.
   Otherwise, look for it and assume it is uncompressed.
 	      If not found, append .Z and look again, assuming compressed */
/* If pathname is "-", use stdin and assume it is uncompressed */

/* Unforgiving: quit if open() fails */
/* Also entry point for closing the associated stream */

/*****************************************************************

  Modified by Philip Clarkson 1/10/96 to allow gzipped files also. 

*****************************************************************/

/* Edited by Philip Clarkson, March 1997 to prevent compilation warnings */

#include <stdio.h>
#include <string.h>
#include "general.h"
#include "compat.h" // in win32
char  RRi_is_Z[100];

FILE *rr_iopen(char *path)
{
  static char rname[]="rr_iopen";
  FILE *fp;
  char pipe[256], is_Z;
  size_t lpath;

  if (strcmp(path,"-")==0) return(stdin);

  lpath = strlen(path);
  if (lpath > sizeof(pipe) - strlen("cat | gunzip ") - 4)
    quit(-1,"%s: pathname '%s' is too long\n",rname,path);

  if (strcmp(&path[lpath-2],".Z")==0) {
     /* popen() does not report error if file doesn't exist, so: */
     if (!rr_fexists(path)) quit(-1,"%s: file '%s' not found\n",rname,path);
     sprintf(pipe,"zcat %s",path);
     goto Z;
  }

  else if (strcmp(&path[lpath-3],".gz")==0) {
     /* popen() does not report error if file doesn't exist, so: */
     if (!rr_fexists(path)) quit(-1,"%s: file '%s' not found\n",rname,path);
     sprintf(pipe,"cat %s | gunzip",path);
     goto Z;
  }

  else if (!rr_fexists(path)) {
     sprintf(pipe,"%s.Z",path);
     /* popen() does not report error if file doesn't exist, so: */
     if (!rr_fexists(pipe)) {
       sprintf(pipe,"%s.gz",path);
       if (!rr_fexists(pipe)) {
	 quit(-1,"%s: None of '%s' '%s.Z' or '%s.gz' exist.\n",rname,path,path,path);
       }
       sprintf(pipe,"cat %s.gz | gunzip",path);
       goto Z;
     }
     sprintf(pipe,"zcat %s.Z",path);
     goto Z;
  }
  else {
     fp = rr_fopen(path,"rb");
     is_Z = 0;
     goto record;
  }

Z:
  fp = popen(pipe,"r");
  if (!fp) quit(-1,"%s: problems opening the pipe '%s' for input.\n", rname,pipe);
  is_Z = 1;

record:
  if (fileno(fp) > sizeof(RRi_is_Z)-1) quit(-1,"%s: fileno = %d is too large\n",rname,fileno(fp));
  RRi_is_Z[fileno(fp)] = is_Z;

  return(fp);
}

void *rr_iclose(FILE *fp)
{
  if (fp==stdin) return(0);
  else if (RRi_is_Z[fileno(fp)]) pclose(fp);
  else fclose(fp);

  return(0); /* Not relevant, but stops compilation warnings. */

}
