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


/* Stuff that is common to both text2idngram and wngram2idngram */


int n; /* Declare it globally, so doesn't need to be passed
		     as a parameter to compare_ngrams. which might
		     cause qsort to choke */
#include "../liblmest/ngram.h"
#include "../liblmest/stats.h"

int compare_ngrams3(const void *ngram1,
		   const void *ngram2
		   ) {

  int i;

  wordid_t *ngram_pointer1;
  wordid_t *ngram_pointer2;

  ngram_pointer1 = (wordid_t *) ngram1;
  ngram_pointer2 = (wordid_t *) ngram2;

  for (i=0;i<n;i++) {
    if (ngram_pointer1[i]<ngram_pointer2[i]) 
      return(1);
    else {
      if (ngram_pointer1[i]>ngram_pointer2[i]) 
	return(-1);
    }
  }

  return(0);

}



void merge_tempfiles (int start_file, 
		      int end_file, 
		      char *temp_file_root,
		      char *temp_file_ext,
		      int max_files,
		      FILE *outfile,
		      flag write_ascii,
		      int fof_size) {

  FILE *new_temp_file;
  char temp_string[1000];
  char *new_temp_filename;
  
  FILE **temp_file;
  char **temp_filename;
  wordid_t **current_ngram;
  wordid_t *smallest_ngram;
  wordid_t *previous_ngram;

  int *current_ngram_count;
  flag *finished;
  flag all_finished;
  int temp_count;
  int i,j;
  flag first_ngram;
  fof_t **fof_array;
  ngram_sz_t *num_kgrams;
  int *ng_count;
  int pos_of_novelty;
  
  pos_of_novelty = n; /* Simply for warning-free compilation */
  num_kgrams = (ngram_sz_t *) rr_calloc(n-1,sizeof(ngram_sz_t));
  ng_count = (int *) rr_calloc(n-1,sizeof(int));
  first_ngram = 1;
  
  previous_ngram = (wordid_t *) rr_calloc(n,sizeof(wordid_t));
  temp_file = (FILE **) rr_malloc(sizeof(FILE *) * (end_file-start_file+1));
  temp_filename = (char **) rr_malloc(sizeof(char *) * 
				      (end_file-start_file+1));

  /* should change to 2d array*/
  current_ngram = (wordid_t **) rr_malloc(sizeof(wordid_t *) * 
						(end_file-start_file+1));
  for (i=0;i<=end_file-start_file;i++) 
    current_ngram[i] = (wordid_t *) rr_malloc(sizeof(wordid_t)*n);

  current_ngram_count = (int *) rr_malloc(sizeof(int)*(end_file-start_file+1));

  finished = (flag *) rr_malloc(sizeof(flag)*(end_file-start_file+1));
  smallest_ngram = (wordid_t *) rr_malloc(sizeof(wordid_t)*n);

  /* should change to 2d array*/
  fof_array = (fof_t **) rr_malloc(sizeof(fof_t *)*(n-1));
  for (i=0;i<=n-2;i++) 
    fof_array[i] = (fof_t *) rr_calloc(fof_size+1,sizeof(fof_t));

  if (end_file-start_file+1 > max_files) {
    sprintf(temp_string,"%s%hu%s",temp_file_root,
	    end_file+1,temp_file_ext);
    new_temp_filename = salloc(temp_string);
    new_temp_file = rr_oopen(new_temp_filename);
    merge_tempfiles(start_file,start_file+max_files-1,
		    temp_file_root,temp_file_ext,max_files,
		    new_temp_file,write_ascii,0);
    merge_tempfiles(start_file+max_files,end_file+1,
		    temp_file_root,temp_file_ext,max_files,
		    outfile,write_ascii,0);
  }else {

    /* Open all the temp files for reading */
    for (i=0;i<=end_file-start_file;i++) {
      sprintf(temp_string,"%s%hu%s",temp_file_root,
	      i+start_file,temp_file_ext);
      temp_filename[i] = salloc(temp_string);
      temp_file[i] = rr_iopen(temp_filename[i]);
    }
    
    /* Now go through the files simultaneously, and write out the appropriate
       ngram counts to the output file. */

    for (i=end_file-start_file;i>=0;i--) {
      finished[i] = 0;
      if (!rr_feof(temp_file[i])) {
	for (j=0;j<=n-1;j++) {
	  rr_fread((char*) &current_ngram[i][j], sizeof(wordid_t),1,
		   temp_file[i],"temporary n-gram ids",0);
	}    
	rr_fread((char*) &current_ngram_count[i], sizeof(int),1,
		 temp_file[i],"temporary n-gram counts",0);
      }
    }
    
    all_finished = 0;

    while (!all_finished) {

      /* Find the smallest current ngram */
      for (i=0;i<=n-1;i++) 
	smallest_ngram[i] = MAX_WORDID;

      for (i=0;i<=end_file-start_file;i++) {
	if (!finished[i]) {
	  if (compare_ngrams3(smallest_ngram,current_ngram[i]) < 0) {
	    for (j=0;j<n;j++)
	      smallest_ngram[j] = current_ngram[i][j];
	  }
	}
      }

#if MAX_VOCAB_SIZE < 65535
      /* This check is well-meaning but completely useless since
	 smallest_ngram[i] by definition cannot contain any value
	 greater than MAX_VOCAB_SIZE (dhuggins@cs, 2006-03) */
      for (i=0;i<=n-1;i++) {
	if (smallest_ngram[i] > MAX_VOCAB_SIZE) {
	  quit(-1,"Error : Temporary files corrupted, invalid n-gram found.\n");
	}
      }
#endif
	  
      /* For each of the files that are currently holding this ngram,
	 add its count to the temporary count, and read in a new ngram
	 from the files. */

      temp_count = 0;

      for (i=0;i<=end_file-start_file;i++) {
	if (!finished[i]) {
	  if (compare_ngrams3(smallest_ngram,current_ngram[i]) == 0) {
	    temp_count = temp_count + current_ngram_count[i];
	    if (!rr_feof(temp_file[i])) {
	      for (j=0;j<=n-1;j++) {
		rr_fread((char*) &current_ngram[i][j],sizeof(wordid_t),1,
			 temp_file[i],"temporary n-gram ids",0);
	      }
	      rr_fread((char*)&current_ngram_count[i],sizeof(int),1,
		       temp_file[i],"temporary n-gram count",0);
	    }else {
	      finished[i] = 1;
	      all_finished = 1;
	      for (j=0;j<=end_file-start_file;j++) {
		if (!finished[j]) 
		  all_finished = 0;
	      }
	    }
	  }
	}
      }
      
      if (write_ascii) {
	for (i=0;i<=n-1;i++) {

	  if (fprintf(outfile,"%d ",smallest_ngram[i]) < 0) 
	    {
	      quit(-1,"Write error encountered while attempting to merge temporary files.\nAborting, but keeping temporary files.\n");
	    }
	}
	if (fprintf(outfile,"%d\n",temp_count) < 0)  
	  quit(-1,"Write error encountered while attempting to merge temporary files.\nAborting, but keeping temporary files.\n");

      }else {
	for (i=0;i<=n-1;i++) {
	  rr_fwrite((char*)&smallest_ngram[i],sizeof(wordid_t),1,
		    outfile,"n-gram ids");
	}
	rr_fwrite((char*)&temp_count,sizeof(count_t),1,outfile,"n-gram counts");		   
      }

      if (fof_size > 0 && n>1) { /* Add stuff to fof arrays */
	
	/* Code from idngram2stats */	
	pos_of_novelty = n;
	for (i=0;i<=n-1;i++) {
	  if (smallest_ngram[i] > previous_ngram[i]) {
	    pos_of_novelty = i;
	    i=n;
	  }
	}
	  
	/* Add new N-gram */
	  
	num_kgrams[n-2]++;
	if (temp_count <= fof_size)
	  fof_array[n-2][temp_count]++;

	if (!first_ngram) {
	  for (i=n-2;i>=MAX(1,pos_of_novelty);i--) {
	    num_kgrams[i-1]++;
	    if (ng_count[i-1] <= fof_size) {
	      fof_array[i-1][ng_count[i-1]]++;
	    }
	    ng_count[i-1] = temp_count;
	  }
	}else {
	  for (i=n-2;i>=MAX(1,pos_of_novelty);i--) {
	    ng_count[i-1] = temp_count;
	  }
	  first_ngram = 0;
	}
	  
	for (i=0;i<=pos_of_novelty-2;i++)
	  ng_count[i] += temp_count;

	for (i=0;i<=n-1;i++)
	  previous_ngram[i]=smallest_ngram[i];

      }
    }
    
    for (i=0;i<=end_file-start_file;i++) {
      fclose(temp_file[i]);
      remove(temp_filename[i]); 
    }
    
  }    

  if (fof_size > 0 && n>1) { /* Display fof arrays */

    /* Process last ngram */
    for (i=n-2;i>=MAX(1,pos_of_novelty);i--) {
      num_kgrams[i-1]++;
      if (ng_count[i-1] <= fof_size)
	fof_array[i-1][ng_count[i-1]]++;

      ng_count[i-1] = temp_count;
    }
    
    for (i=0;i<=pos_of_novelty-2;i++)
      ng_count[i] += temp_count;

    display_fof_array(num_kgrams,fof_array,fof_size,stderr, n);

  }

}



