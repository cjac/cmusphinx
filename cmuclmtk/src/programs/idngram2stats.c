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


/*
  $Log: idngram2stats.c,v $
  Revision 1.6  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.5  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */

#include <stdlib.h>
#include <stdio.h>
#include "../libs/pc_general.h"
#include "../liblmest/toolkit.h"
#include "../liblmest/idngram2lm.h"
#include "../liblmest/stats.h"
#include "../libs/general.h"

int get_ngram(FILE *id_ngram_fp, ngram *ng, flag ascii);

void help_message()
{
    fprintf(stderr,"indngram2stats : Report statistics for an id n-gram file.\n");
    fprintf(stderr,"Usage : idngram2stats [ -n 3 ] \n");
    fprintf(stderr,"                      [ -fof_size 50 ]\n");
    fprintf(stderr,"                      [ -verbosity %d ]\n",
	    DEFAULT_VERBOSITY);
    fprintf(stderr,"                      [ -ascii_input ] \n");
    fprintf(stderr,"                      < .idngram > .stats\n");
}

int main (int argc, char **argv) {

  flag first_ngram;
  int n;
  fof_sz_t fof_size;
  flag is_ascii;
  int verbosity;
  fof_t **fof_array;
  ngram_sz_t *num_kgrams;
  ngram current_ngram;
  ngram previous_ngram;
  count_t *ng_count;
  int pos_of_novelty;
  int nlines;
  int i;

  report_version(&argc,argv);

  if (argc == 1 || pc_flagarg(&argc, argv,"-help")) {
    help_message();
    exit(1);
  }

  is_ascii = pc_flagarg(&argc, argv,"-ascii_input");
  n = pc_intarg(&argc, argv,"-n",3);
  fof_size = pc_intarg(&argc, argv,"-fof_size",50);
  verbosity = pc_intarg(&argc, argv,"-verbosity",DEFAULT_VERBOSITY);

  pc_report_unk_args(&argc,argv,verbosity);

  pc_message(verbosity,2,"n        = %d\n",n);
  pc_message(verbosity,2,"fof_size = %d\n",fof_size);

  current_ngram.n = n;
  previous_ngram.n = n;
  pos_of_novelty = n;
  
  fof_array = (fof_t **) rr_malloc(sizeof(fof_t *) * (n-1));
  for (i=0;i<=n-2;i++) 
    fof_array[i] = (fof_t *) rr_calloc(fof_size+1,sizeof(fof_t));

  num_kgrams = (ngram_sz_t *) rr_calloc(n-1,sizeof(ngram_sz_t));
  ng_count = (count_t *) rr_calloc(n-1,sizeof(count_t));

  current_ngram.id_array = (id__t *) rr_calloc(n,sizeof(id__t));
  previous_ngram.id_array = (id__t *) rr_calloc(n,sizeof(id__t));

  pc_message(verbosity,2,"Processing id n-gram file.\n");
  pc_message(verbosity,2,"20,000 n-grams processed for each \".\", 1,000,000 for each line.\n");

  nlines = 0;
  first_ngram = 1;
  
  while (!rr_feof(stdin)) {
    
    if (!first_ngram)
      ngram_copy(&previous_ngram,&current_ngram,n);

    if (get_ngram(stdin,&current_ngram,is_ascii)) {

      nlines++;
      show_idngram_nlines(nlines, verbosity);
    
      /* Test for where this ngram differs from last - do we have an
	 out-of-order ngram? */
    
      if (!first_ngram)
        pos_of_novelty = ngram_find_pos_of_novelty(&current_ngram,&previous_ngram,n,nlines);
      else
        pos_of_novelty = 0;

      /* Add new N-gram */
     
      num_kgrams[n-2]++;
      if (current_ngram.count <= fof_size) 
	fof_array[n-2][current_ngram.count]++;

      if (!first_ngram) {
	for (i=n-2;i>=MAX(1,pos_of_novelty);i--) {
	  num_kgrams[i-1]++;
	  if (ng_count[i-1] <= fof_size) 
	    fof_array[i-1][ng_count[i-1]]++;
	  
	  ng_count[i-1] = current_ngram.count;
	}
      } else {
	for (i=n-2;i>=MAX(1,pos_of_novelty);i--) 
	  ng_count[i-1] = current_ngram.count;
      }
	
      for (i=0;i<=pos_of_novelty-2;i++) 
	ng_count[i] += current_ngram.count;
	
      if (first_ngram)
        first_ngram = 0;
    }
  }

  /* Process last ngram */

  for (i=n-2;i>=MAX(1,pos_of_novelty);i--) {
    num_kgrams[i-1]++;
    if (ng_count[i-1] <= fof_size) {
      fof_array[i-1][ng_count[i-1]]++;
    }
    ng_count[i-1] = current_ngram.count;
  }
  
  for (i=0;i<=pos_of_novelty-2;i++)
    ng_count[i] += current_ngram.count;

  display_fof_array(num_kgrams,fof_array,fof_size,stderr, n);

  pc_message(verbosity,0,"idngram2stats : Done.\n");

  exit(0);
  
}
