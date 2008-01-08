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


/*  If the file's extension is "vocab_ht" (or the file's name is "vocab_ht"),
       it is assumed to be a pre-compiled vocabulary hash table.  Otherwise
       it is assumed to be a regular .vocab file.
*/

/* Edited by Philip Clarkson, March 1997 to prevent compilation warnings */


#include <stdio.h>
#include <string.h>
#include "general.h"
#include "sih.h"

/**
  If the file extension is "vocab_ht", then read value from file. 
  get the vocab from vocab hash table.  Do we stil support this?

  If not, read it as a word list, then read the wlist into array. 
 */
void read_voc(char *filename, 
	      int verbosity,   
	      sih_t *p_vocab_ht, 
	      char ***p_vocab, 
	      vocab_sz_t *p_vocab_size
	      )
/* p_vocab==NULL means: build only a hash table */
{
  char *pperiod;
  vocab_sz_t   vocab_size;

  pperiod = strrchr(filename,'.');
  if (pperiod==NULL) pperiod = filename-1;
  if (strcmp(pperiod+1,"vocab_ht")==0) { 	     /* file == hash_table */
     FILE *fp=rr_iopen(filename);
     sih_val_read_from_file(p_vocab_ht, fp, filename, verbosity);
     rr_iclose(fp);
     vocab_size = p_vocab_ht->nentries;
     if (p_vocab!=NULL) {
        get_vocab_from_vocab_ht(p_vocab_ht, vocab_size, verbosity, p_vocab);
	*p_vocab[0] = salloc("<UNK>");
     }
  }else {					     /* file == vocab(ascii) */
     read_wlist_into_siht(filename, verbosity, p_vocab_ht, &vocab_size);
     if (p_vocab!=NULL) {
        read_wlist_into_array(filename, verbosity, p_vocab, &vocab_size);
        *p_vocab[0] = salloc("<UNK>");
     }
  }

  if (p_vocab_size)
    *p_vocab_size = vocab_size;
  
}


/* derive the vocab from the vocab hash table */

void get_vocab_from_vocab_ht(sih_t *ht, vocab_sz_t vocab_size, int verbosity, char ***p_vocab)
{
  static char rname[]="get_vocab_fm_ht";
  char   **wlist;
  vocab_sz_t islot, wordid;

  wlist = (char **) rr_malloc((vocab_size+1)*sizeof(char *));

  for (islot=0; islot<ht->nslots; islot++) {
     wordid = (vocab_sz_t) ht->slots[islot].intval;
     if (wordid>0) wlist[wordid] = ht->slots[islot].string;
  }

  /*  fprintf(stderr,"vocab_size %llu\n",(int)vocab_size);*/
  /* Still some problems in using vocab_sz_t in the for loop
     If not, the wordid will actually go up to more than vocab_size. 
   */

  for (wordid=1; wordid<=(int)vocab_size; wordid++){
    /*    fprintf(stderr,"wordid %d\n",wordid);
	  fflush(stderr);*/
    if (wlist[wordid]==NULL) 
      quit(-1,"%s ERROR: the hash table does not contain wordid %d, \n",
	       rname, (int)wordid);
    else{
      /*      fprintf(stderr,"%s ERROR: the hash table does contain wordid %ld wordstr %s, \n",
	       rname, wordid, wlist[wordid]);
	       fflush(stderr);*/
    }
  }

  if (verbosity) fprintf(stderr,
     "%s: vocabulary was constructed from the vocab hash table\n",rname);
  *p_vocab = wlist;
}
