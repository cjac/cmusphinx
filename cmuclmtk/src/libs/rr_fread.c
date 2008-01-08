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


/* call fread and quit if it fails 
   Also SWAP the data if read on a MIPS machine 
   Roni Rosenfeld, 9/92  */


/* Edited by Philip Clarkson, March 1997 to prevent compilation warnings */


#include <stdio.h>
#include "mips_swap.h"
#include "general.h"

void *rr_fread(char *ptr, int elsize, size_t n_elem, FILE *fp,
	       char *header, int not_more)
{
  size_t n_read, i;
  char dummychar;

  if (n_elem > 0) {
     n_read = fread(ptr,elsize,n_elem,fp);
     if (n_read != n_elem) quit(-1,
	"rr_fread: problems reading %s.  Only %d of %d elements were read\n",
	 header, n_read, n_elem);

     if (elsize == sizeof(int)) {
        for (i=0; i<n_elem; i++) {
	   SWAPWORD(ptr+(elsize*i));
	}
     }
     else if (elsize == sizeof(short)) {
        for (i=0; i<n_elem; i++) {
	   SWAPHALF(ptr+(elsize*i));
        }
     }
     else if (elsize == sizeof(double)) {
        for (i=0; i<n_elem; i++) {
	   SWAPDOUBLE(ptr+(elsize*i));
	}
     }
  }

  if (not_more) {
     if (fread(&dummychar,1,1,fp) != 0)
       quit(-1,"rr_fread: more data after %s - should not be there\n",header);
  }

  return(0); /* Not relevant, but stops compilation warnings. */

}
