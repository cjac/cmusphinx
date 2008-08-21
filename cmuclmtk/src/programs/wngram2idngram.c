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
  $Log: wngram2idngram.c,v
 */

/*
  [20080222] (air) temporary filenames now generated using tempnam() 
*/

#define DEFAULT_HASH_SIZE 200000
#define DEFAULT_MAX_FILES 20
#define MAX_N 20
#define TEMP_FILE_ROOT "wngram2idngram.temp."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* #include <sys/utsname.h> */
#include <sys/types.h>
#include "../liblmest/toolkit.h"
#include "../libs/general.h"
#include "../libs/pc_general.h"
#include "../programs/idngram.h"
#include "../libs/ac_hash.h"
#include "../libs/win32compat.h"

typedef struct {
  wordid_t *word;
  int count;
} ngram_rec;

int compare_ngrams2(const void *ngram1,
		    const void *ngram2) 
{
  int temp;
  int i;
  ngram_rec *r1;
  ngram_rec *r2;

  r1 = (ngram_rec *) ngram1;
  r2 = (ngram_rec *) ngram2;
  
  temp = 0;

  for (i=0;i<n;i++) {
    if ((r1->word[i]) < (r2->word[i])) {
      temp = -1;
      break;
     }else {
      if ((r1->word[i]) > (r2->word[i])) {
	temp = 1;
	break;
      }
    }
  }

  return(temp);
}

void help_message()
{
    fprintf(stderr,"wngram2idngram - Convert a word n-gram file to an id n-gram file.\n");
    fprintf(stderr,"Usage : wngram2idngram -vocab .vocab\n");
    fprintf(stderr,"                     [ -buffer %d ] \n",STD_MEM);
    fprintf(stderr,"                     [ -hash %d ]\n",DEFAULT_HASH_SIZE);
    fprintf(stderr,"                     [ -temp %s ]\n",DEFAULT_TEMP);
    fprintf(stderr,"                     [ -files %d ]\n",DEFAULT_MAX_FILES);
    fprintf(stderr,"                     [ -gzip | -compress ]\n");
    fprintf(stderr,"                     [ -verbosity 2 ]\n");
    fprintf(stderr,"                     [ -n 3 ]\n");
    fprintf(stderr,"                     [ -write_ascii ]\n");
    fprintf(stderr,"                     [ -fof_size 10 ]\n");
    fprintf(stderr,"                     < .wngram > .idngram\n");
}

int main(int argc, char *argv[]) {

  int verbosity;
  int vocab_size;
  FILE *vocab_file;
  int buffer_size;
  flag write_ascii;
  int max_files;
  int number_of_tempfiles;
  char *vocab_filename;
  char tempfiles_directory[1000];
  char temp_word[MAX_WORD_LENGTH];
  char temp_word2[MAX_WORD_LENGTH];
  char temp_word3[MAX_WORD_LENGTH];
  flag contains_unks;
  int position_in_buffer;
  FILE *tempfile;
  FILE *non_unk_fp;
  ngram_rec *buffer;
  flag same_ngram;
  int i;
  int j;
  int fof_size;
  int size_of_rec;
  char *temp_file_root;
  char *temp_file_ext;
  char *host_name;
  /*   struct utsname uname_info; */
  int proc_id;

  /* Vocab hash table things */

  struct idngram_hash_table vocabulary;
  unsigned long hash_size;
  unsigned long M;

  wordid_t *current_ngram;
  int current_count;
  wordid_t *sort_ngram;
  int sort_count;
  
  /* Process command line */

  report_version(&argc,argv);
  
  if (argc == 1 || pc_flagarg(&argc, argv,"-help")) {    
    /* Display help message */    
    help_message();
    exit(1);
  }


  n = pc_intarg( &argc, argv, "-n",DEFAULT_N);
  hash_size = pc_intarg( &argc, argv, "-hash",DEFAULT_HASH_SIZE);
  buffer_size = pc_intarg( &argc, argv, "-buffer",STD_MEM);
  write_ascii = pc_flagarg(&argc,argv,"-write_ascii");
  verbosity = pc_intarg(&argc,argv,"-verbosity",DEFAULT_VERBOSITY);
  max_files = pc_intarg( &argc, argv, "-files",DEFAULT_MAX_FILES);
  fof_size = pc_intarg(&argc,argv,"-fof_size",10);
  vocab_filename = salloc(pc_stringarg( &argc, argv, "-vocab", "" ));
  
  if (!strcmp("",vocab_filename)) 
    quit(-1,"Error : Must specify a vocabulary file.\n");
    
  strcpy(tempfiles_directory,pc_stringarg( &argc, argv, "-temp",DEFAULT_TEMP));

  if (pc_flagarg(&argc,argv,"-compress")) 
    temp_file_ext = salloc(".Z");
  else {
    if (pc_flagarg(&argc,argv,"-gzip")) 
      temp_file_ext = salloc(".gz");
    else 
      temp_file_ext = salloc("");
  }

  temp_file_root = tempnam(tempfiles_directory,TEMP_FILE_ROOT);

  pc_report_unk_args(&argc,argv,verbosity);
  
  pc_message(verbosity,2,"Vocab           : %s\n",vocab_filename);
  pc_message(verbosity,2,"Buffer size     : %d\n",buffer_size);
  pc_message(verbosity,2,"Hash table size : %d\n",hash_size);
  pc_message(verbosity,2,"Temp directory  : %s\n",tempfiles_directory);
  pc_message(verbosity,2,"Max open files  : %d\n",max_files);
  pc_message(verbosity,2,"n               : %d\n",n);
  pc_message(verbosity,2,"FOF size               : %d\n",fof_size);  

  size_of_rec = (sizeof(wordid_t) * n) + 16 - (( n* sizeof(wordid_t)) % 16);
  buffer_size *= (1000000/((sizeof(ngram_rec) + size_of_rec)));
  fprintf(stderr,"buffer size = %d\n",buffer_size);

  /* Allocate memory for hash table */

  fprintf(stderr,"Initialising hash table...\n");

  M = nearest_prime(hash_size);

  new_idngram_hashtable(&vocabulary,M);

  /* Read in the vocabulary */

  vocab_size = 0;

  vocab_file = rr_iopen(vocab_filename);

  pc_message(verbosity,2,"Reading vocabulary...\n");

  while (fgets (temp_word, sizeof(temp_word),vocab_file)) {
    if (strncmp(temp_word,"##",2)==0) continue;
    sscanf (temp_word, "%s ",temp_word2);

    /* Check for vocabulary order */
    if (vocab_size > 0 && strcmp(temp_word2,temp_word3)<0) 
      quit(-1,"wngram2idngram : Error : Vocabulary is not alphabetically ordered.\n");

    /* Check for repeated words in the vocabulary */

    if (index2(&vocabulary,temp_word2) != 0) 
      warn_on_repeated_words(temp_word);

    warn_on_wrong_vocab_comments(temp_word);

    vocab_size++;
    
    add_to_idngram_hashtable(&vocabulary,idngram_hash(temp_word2,M),temp_word2,vocab_size);
    strcpy(temp_word3,temp_word2);
  }

  if (vocab_size > MAX_VOCAB_SIZE) 
    quit(-1,"Error : Vocabulary size exceeds maximum.\n");
  
  pc_message(verbosity,2,"Allocating memory for the buffer...\n");

  buffer=(ngram_rec *) rr_malloc((buffer_size+1)*sizeof(ngram_rec));
  
  for (i=0;i<=buffer_size;i++) 
    buffer[i].word = (wordid_t *) rr_malloc(n*sizeof(wordid_t));

  /* Open the "non-OOV" tempfile */

  sprintf(temp_word,"%s1%s",temp_file_root,temp_file_ext);
  
  non_unk_fp = rr_fopen(temp_word,"w");

  pc_message(verbosity,2,"Writing non-OOV counts to temporary file %s\n",
	     temp_word);
  number_of_tempfiles = 1;

  current_ngram = (wordid_t *) rr_malloc(n*sizeof(wordid_t));
  sort_ngram = (wordid_t *) rr_malloc(n*sizeof(wordid_t));

  /* Read text into buffer */
  position_in_buffer = 0;

  while (!rr_feof(stdin)) {
    
    for (i=0;i<=n-1;i++) {
      get_word(stdin,temp_word);
      current_ngram[i]=index2(&vocabulary,temp_word);
    }
    if (scanf("%d",&current_count) != 1) 
      if (!rr_feof(stdin)) 
	quit(-1,"Error reading n-gram count from stdin.\n");

    if (!rr_feof(stdin)) {

      contains_unks = 0;
      for (i=0;i<=n-1;i++) {
	if (!current_ngram[i]) 
	  contains_unks = 1;
      }

      if (contains_unks) {
	/* Write to buffer */

	position_in_buffer++;

	if (position_in_buffer >= buffer_size) {

	  /* Sort buffer */
	  pc_message(verbosity,2,
		     "Sorting n-grams which include an OOV word...\n");

	  qsort((void*) buffer,(size_t) position_in_buffer,
		sizeof(ngram_rec),compare_ngrams2);

	  pc_message(verbosity,2,"Done.\n");

	  /* Write buffer to temporary file */

	  number_of_tempfiles++;
	  
	  sprintf(temp_word,"%s%hu%s",temp_file_root,
		  number_of_tempfiles,temp_file_ext);
	  
	  pc_message(verbosity,2,
		     "Writing sorted OOV-counts buffer to temporary file %s\n",
		     temp_word);

	  tempfile = rr_fopen(temp_word,"w");
	  
	  for (i=0;i<=n-1;i++) 
	    sort_ngram[i] = buffer[0].word[i];

	  sort_count = buffer[0].count;

	  for (i=0;i<=position_in_buffer-2;i++) {
	    
	    same_ngram = 1;
	    for (j=n-1;j>=0;j--) {
	      if (buffer[i].word[j] != sort_ngram[j]) {
		same_ngram = 0;
		j = -1;
	      }
	    }

	    if (same_ngram) 
	      sort_count += buffer[i].count;
	    else {
	      for (j=0;j<=n-1;j++) {
		rr_fwrite((char*)&sort_ngram[j],sizeof(wordid_t),1,
			  tempfile,"temporary n-gram ids");
		sort_ngram[j] = buffer[i].word[j];
	      }
	      rr_fwrite((char*)&sort_count,sizeof(int),1,tempfile,
			"temporary n-gram counts");
	      sort_count = buffer[i].count;
	    }
	  }	    
	  for (j=0;j<=n-1;j++) 
	    rr_fwrite((char*)&sort_ngram[j],sizeof(wordid_t),1,
		      tempfile,"temporary n-gram ids");

	  rr_fwrite((char*)&sort_count,sizeof(int),1,tempfile,
		    "temporary n-gram counts");
	  rr_oclose(tempfile);
	  position_in_buffer = 1;

	}
	
	for (i=0;i<=n-1;i++) 
	  buffer[position_in_buffer-1].word[i] = current_ngram[i];

	buffer[position_in_buffer-1].count = current_count;

      }else {
	/* Write to temporary file */
	for (i=0;i<=n-1;i++) 
	  rr_fwrite((char*)&current_ngram[i],sizeof(unsigned short),1,
		    non_unk_fp,"temporary n-gram ids");

	rr_fwrite((char*)&current_count,sizeof(int),1,non_unk_fp,
		  "temporary n-gram counts");
      }
    }
  }

  if (position_in_buffer > 0) {

    /* Only do this bit if we have actually seen some OOVs */
    /* Sort final buffer */    
    pc_message(verbosity,2,"Sorting final buffer...\n");

    qsort((void*) buffer,(size_t) position_in_buffer,
	  sizeof(ngram_rec),compare_ngrams2);
    
    /* Write final buffer */
    
    number_of_tempfiles++;
  
    sprintf(temp_word,"%s%hu%s",temp_file_root,
	    number_of_tempfiles,temp_file_ext);
    
    pc_message(verbosity,2,"Writing sorted buffer to temporary file %s\n", temp_word);

    tempfile = rr_fopen(temp_word,"w");
    
    for (i=0;i<=n-1;i++) 
      sort_ngram[i] = buffer[0].word[i];

    sort_count = buffer[0].count;
    
    for (i=1;i<=position_in_buffer-1;i++) {
      
      same_ngram = 1;
      for (j=n-1;j>=0;j--) {
	if (buffer[i].word[j] != sort_ngram[j]) {
	  same_ngram = 0;
	  j = -1;
	}
      }
      
      if (same_ngram) 
	sort_count += buffer[i].count;
      else {
	for (j=0;j<=n-1;j++) {
	  rr_fwrite((char*)&sort_ngram[j],sizeof(wordid_t),1,
		    tempfile,"temporary n-gram ids");
	  sort_ngram[j] = buffer[i].word[j];
	}
	rr_fwrite((char*)&sort_count,sizeof(int),1,tempfile,
		  "temporary n-gram counts");
	sort_count = buffer[i].count;
      }
    }	    
    for (j=0;j<=n-1;j++) 
      rr_fwrite((char*)&sort_ngram[j],sizeof(wordid_t),1,
		tempfile,"temporary n-gram ids");

    rr_fwrite((char*)&sort_count,sizeof(int),1,tempfile,
	      "temporary n-gram counts");
    fclose(tempfile);
    
    /* Merge the temporary files, and output the result to standard output */

    fclose(non_unk_fp);

    pc_message(verbosity,2,"Merging temporary files...\n");
    
    merge_tempfiles(1,
		    number_of_tempfiles,
		    temp_file_root,
		    temp_file_ext,
		    max_files,
		    stdout,
		    write_ascii,
		    fof_size); 
  }else {

    /* Just write out the none OOV buffer to stdout */
    fclose(non_unk_fp);

    merge_tempfiles(1,
		    1,
		    temp_file_root,
		    temp_file_ext,
		    max_files,
		    stdout,
		    write_ascii,
		    fof_size); 
  }

  pc_message(verbosity,0,"wngram2idngram : Done.\n");

  exit(0);

}
