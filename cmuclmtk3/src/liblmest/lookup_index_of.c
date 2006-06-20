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


/* return an index to be used for lookup of supplied value.
   If value hasn't been entered in the table yet, add it.
   Note that we first try to assign the identity mapping.
   If that's occupied, we next search from the end, till the first empty slot.
*/

/* Edited by Philip Clarkson, March 1997 to prevent compilation warnings */

#include "ngram.h"

int lookup_index_of(int *lookup_table, int lookup_table_size, int intval)
{
  int i;
  if (intval>0 && intval<lookup_table_size) {
     if (lookup_table[intval]==intval) return(intval);
     else if (lookup_table[intval]==0) {
	lookup_table[intval] = intval;
	return(intval);
     }
  }
  for (i=lookup_table_size-1; i>=0; i--) {
     if (lookup_table[i]==intval) return(i);
     if (lookup_table[i]==0) {
	lookup_table[i] = intval;
	return(i);
     }
  }
  quit(-1,"Error - more than %d entries required in the count table. \nCannot store counts in two bytes. Use the -four_byte_counts flag.\n",lookup_table_size);
  
  /* Clearly we will never get this far in the code, but the compiler
     doesn't realise this, so to stop it spewing out warnings... */

  return(0);

}
