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


/* Open a file for output. */
/* If pathanme ends in ".Z", prepare to write thru a 'compress' pipe.  */
/* If pathname is "-", prepare to write to stdout (uncompressed) */

/* Unforgiving: quit if open() fails */
/* Also entry point for closing the associated stream */

/*****************************************************************

  Modified by Philip Clarkson 1/10/96 to allow gzipped files also. 

*****************************************************************/

/* Edited by Philip Clarkson, March 1997 to prevent compilation warnings */


#include <stdio.h>
#include <string.h>
#include "general.h"

char  RRo_is_Z[100];

FILE *rr_oopen(char *path)
{
  static char rname[]="rr_oopen";
  FILE *fp;
  char pipe[256], is_Z;
  size_t lpath;

  if (strcmp(path,"-")==0) return(stdout);

  lpath = strlen(path);
  if (strcmp(&path[lpath-2],".Z")==0) {
    if (lpath > sizeof(pipe) - strlen("compress >!  ") - 4)
      quit(-1,"%s: pathname '%s' is too long\n",rname,path);
     sprintf(pipe,"compress > %s",path);
     fp = popen(pipe,"w");
     if (!fp) quit(-1,"%s: problems opening the pipe '%s' for output.\n", rname,pipe);
     is_Z = 1;
  }
  else {
    if (strcmp(&path[lpath-3],".gz")==0) {
      if (lpath > sizeof(pipe) - strlen("gzip >!  ") -4)
	quit(-1,"%s: pathname '%s' is too long\n",rname,path);
      sprintf(pipe,"gzip > %s",path);
      fp = popen(pipe,"w");
      if (!fp) quit(-1,"%s: problems opening the pipe '%s' for output.\n", rname,pipe);
      is_Z = 1;
    }
    else {
      fp = rr_fopen(path,"wb");
      is_Z = 0;
    }
  }

  if (fileno(fp) > sizeof(RRo_is_Z)-1) quit(-1,"%s: fileno = %d is too large\n",rname,fileno(fp));
  RRo_is_Z[fileno(fp)] = is_Z;

  return(fp);
}

void *rr_oclose(FILE *fp)
{
  if (fp==stdout) return(0);
  fflush(fp); 
  if (RRo_is_Z[fileno(fp)]) 
    pclose(fp);
  else
    fclose(fp);

  return(0); /* Not relevant, but stops compilation warnings. */

}
