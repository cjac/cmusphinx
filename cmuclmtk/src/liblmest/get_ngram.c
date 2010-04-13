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


/* Returns zero if we have reached eof */

#include <stdio.h>
#include <stdarg.h>
#include "general.h"   // from libs
#include "ngram.h"

/** 
    20060322 ARCHAN. 
    In both ascii and binary mode, for N-gram
    The program is trying to read a file in this format. 
    <ind1> <ind2> <ind3> ...  <indN> <count>    
 */
int get_ngram(FILE *id_ngram_fp, ngram *ng, flag ascii) {  

  int i;

  if (ascii) {
    for (i=0;i<=ng->n-1;i++) {

      if (fscanf(id_ngram_fp,"%d",&ng->id_array[i]) != 1) 
	{
	  if (rr_feof(id_ngram_fp)) 
	    return 0;

	  quit(-1,"Error reading from id_ngram file.\n");
	}
    }
    if (fscanf(id_ngram_fp,"%d",&ng->count) != 1) {
      if (rr_feof(id_ngram_fp)) 
	return 0;

      quit(-1,"Error reading from id_ngram file.2\n");
    }
  }else {

    /* Read in array of ids */
    /* Read in one at a time - slower, but spares us having to think 
       about byte-orders. */
    
    for (i=0;i<=ng->n-1;i++) {
      if (rr_feof(id_ngram_fp)) 
	return 0;

      rr_fread((char*) &ng->id_array[i],sizeof(id__t),1,id_ngram_fp,
	       "from id_ngram file",0);
    }

    /* Read in count */
    if (rr_feof(id_ngram_fp)) 
      return 0;

    rr_fread((char*) &ng->count,sizeof(count_t),1,id_ngram_fp,
	     "count from id_ngram file",0);

  }

  return 1;
  
}



