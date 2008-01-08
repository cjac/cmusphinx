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


/* Edited by Philip Clarkson, March 1997 to prevent compilation warnings */


#include <stdio.h>
#include <string.h>
#include "general.h"
#include "ac_parsetext.h"

/* allocate an lagre enough array and read in a list of words (first word on each line)
   Leave entry no. 0 empty.
*/

void read_wlist_into_array(char *wlist_filename, int verbosity,  char ***p_wlist, int *p_n_wlist)
{
  static char rname[]="read_wlist_into_array";
  FILE   *wlist_fp = rr_iopen(wlist_filename);
  char   **wlist;
  int    n_wlist, c, lastc, entry_no;
  char   wlist_entry[1024], word[256];

  lastc = '\0';
  n_wlist = 0;
  while ((c=getc(wlist_fp)) != EOF) {
     if (c == '\n') n_wlist++;
     lastc = c;
  }
  if (lastc != '\n') 
    quit(-1,"%s: no newline at end of %s\n",rname,wlist_filename);

  rr_iclose(wlist_fp);
  wlist_fp = rr_iopen(wlist_filename);

  wlist = (char **) rr_malloc((n_wlist+1)*sizeof(char *));
  entry_no = 0;

  while (fgets (wlist_entry, sizeof (wlist_entry), wlist_fp)) {
     if (strncmp(wlist_entry,"##",2)==0) continue;
     /* warn ARPA sites who may have comments starting with a single '#' */
     sscanf (wlist_entry, "%s ", word);
     
     warn_on_wrong_vocab_comments(wlist_entry);
     wlist[++entry_no] = salloc(word);
  }
  rr_iclose(wlist_fp);
  if (verbosity) fprintf(stderr,"%s: a list of %d words was read from \"%s\".\n",
	  	 		 rname,entry_no,wlist_filename);
  *p_wlist = wlist;
  *p_n_wlist = entry_no;
}


