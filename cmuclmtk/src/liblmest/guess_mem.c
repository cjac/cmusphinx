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


/* Rewritten 6 May 1997. No longer try to guess based on filesize and
cutoffs. Merely allocate an equal amount of memory to each of the
tables for 2,3,...,n-grams. Total memory allocation should equal
STD_MEM. This is far from optimal, but is at least robust. */

#include <stdio.h>
#include "toolkit.h"
#include "ngram.h"
#include "general.h"   // from libs
#include "pc_general.h"   // from libs

void guess_mem(int total_mem,
	       int middle_size,   /* Size of 2,3,(n-1)-gram records */
	       int end_size,      /* Size of n-gram record */
	       int n,
	       table_size_t *table_sizes,
	       int verbosity) 
{
  ngram_sz_t *num_kgrams, i;

  num_kgrams = (ngram_sz_t *) rr_malloc(sizeof(ngram_sz_t)*(n-1));  
  /* First decide on file size (making allowances for compressed files) */

  for (i=0;i<=n-3;i++) 
    num_kgrams[i] = total_mem*1000000/(middle_size*(n-1));

  num_kgrams[n-2] = total_mem*1000000/(end_size*(n-1));

  for (i=0;i<=n-2;i++) {
    table_sizes[i+1] = num_kgrams[i];
    pc_message(verbosity,2,"Allocated space for %d %d-grams.\n",
	       table_sizes[i+1],i+2);
  }
}


