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

#include "ac_lmfunc_impl.h"
#include "ac_hash.h"
#include "ac_parsetext.h"
#include "./liblmest/idngram2lm.h"

/* Start Implementation of text2wfreq */

int text2wfreq_impl(FILE* infp, FILE* outfp, int init_nwords, int verbosity)
{
  int hash_size, scanrc;
  struct hash_table vocab;
  char word[MAX_STRING_LENGTH];

  hash_size = nearest_prime( init_nwords );
  new_hashtable( &vocab, hash_size );
  while( (scanrc = fscanf(infp, "%500s", word )) == 1 ) {
    if ( strlen( word ) >= MAX_STRING_LENGTH ) {
      pc_message(verbosity,1,"text2wfreq : WARNING: word too long, will be split: %s...\n",word);
    }
    if (strlen(word)) {
      update( &vocab, word ,verbosity);
    }
  }
  if ( scanrc != EOF ) {
    quit(-1,"Error reading input\n");
  }
  print( outfp, &vocab );
  return 0;
}

/* Start Implementation of wfreq2vocab */

typedef struct {
  char *word;
  int count;
} word_rec;

static int sort_by_count(const void *rec1,const void *rec2) {

  word_rec *r1;
  word_rec *r2;
  
  r1 = (word_rec *) rec1;
  r2 = (word_rec *) rec2;

  return(r2->count-r1->count);

}		  

static int sort_alpha(const void *rec1,const void *rec2) {
 
  word_rec *r1;
  word_rec *r2;
 
  char *s1;
  char *s2;
  
  r1 = (word_rec *) rec1;
  r2 = (word_rec *) rec2;
  
  s1 = r1->word;
  s2 = r2->word;

  return (strcmp(s1,s2));
 
}

/* To make this function less dependent on input stream, just pull records out and create an interface for it
 */
int wfreq2vocab_impl(FILE* ifp, FILE* ofp, int cutoff, int vocab_size, int num_recs, int verbosity)
{
  flag gt_set;
  flag top_set;
  int current_rec;
  int num_above_threshold;
  int num_to_output;
  int i;
  word_rec *records;
  char temp_word[750];

  gt_set = (cutoff != -1);
  top_set = (vocab_size != -1);
  if(cutoff==-1) cutoff=0;
  if(vocab_size==-1) vocab_size=0;

  if (gt_set && top_set) 
    quit(-1,"wfreq2vocab : Error : Can't use both the -top and the -gt options.\n");

  if (!gt_set && !top_set) 
    vocab_size = 20000;

  if (gt_set) 
    pc_message(verbosity,2,"wfreq2vocab : Will generate a vocabulary containing all words which\n              occurred more that %d times. Reading wfreq stream from stdin...\n",cutoff);
  else 
    pc_message(verbosity,2,"wfreq2vocab : Will generate a vocabulary containing the most\n              frequent %d words. Reading wfreq stream from stdin...\n",vocab_size);

  current_rec = 0;
  num_above_threshold = 0;

  records = (word_rec *) rr_malloc(sizeof(word_rec)*num_recs);

  while (!rr_feof(ifp)) {
    if (fscanf(ifp, "%s %d",temp_word,&(records[current_rec].count)) != 2) {
      if (!rr_feof(ifp)) 
	quit(-1,"Error reading unigram counts from standard input.\n");

    }else {
      records[current_rec].word = salloc(temp_word);
      if (gt_set && records[current_rec].count > cutoff) 
	num_above_threshold++;

      current_rec++;
    }

    if(current_rec > num_recs ){
      quit2(-1,"The number of records %d reach the user-defined limit %d, consider to increase the number of records by -records\n",current_rec,num_recs);
    }
  }

  /* Sort records in descending order of count */

  qsort((void*) records,(size_t) current_rec, sizeof(word_rec),sort_by_count);

  if (gt_set) 
    num_to_output = num_above_threshold;
  else 
    num_to_output = vocab_size;

  if (current_rec<num_to_output) 
    num_to_output = current_rec;

  /* Now sort the relevant records alphabetically */

  qsort((void*) records,(size_t) num_to_output, sizeof(word_rec),sort_alpha);

  if (gt_set) 
    pc_message(verbosity,2,"Size of vocabulary = %d\n",num_to_output);

#ifndef THIRTYTWOBITS  
  if (num_to_output>MAX_UNIGRAM) {
    pc_message(verbosity,1,"Warning : Vocab size exceeds %d. This might cause problems with \n",MAX_UNIGRAM);
    pc_message(verbosity,1,"other tools, since word id's are stored in 2 bytes.\n");
  }
#endif

  if (num_to_output == 0) 
    pc_message(verbosity,1,"Warning : Vocab size = 0.\n");
  /* Print the vocab to stdout */
  
  printf("## Vocab generated by v2 of the CMU-Cambridge Statistcal\n");
  printf("## Language Modeling toolkit.\n");
  printf("##\n");
  printf("## Includes %d words ",num_to_output);
  printf("##\n");

  for (i=0;i<=num_to_output-1;i++) 
    fprintf(ofp,"%s\n",records[i].word);

  pc_message(verbosity,0,"wfreq2vocab : Done.\n");

  return 0;
}

int read_vocab(char* vocab_filename, 
	       int verbosity,
	       struct idngram_hash_table* vocabulary,
	       int M
	       )
{
  FILE *vocab_file;
  int vocab_size;
  char temp_word[MAX_WORD_LENGTH];
  char temp_word2[MAX_WORD_LENGTH];

  vocab_size = 0;
  vocab_file = rr_iopen(vocab_filename);

  pc_message(verbosity,2,"Reading vocabulary... \n");

  while (fgets (temp_word, sizeof(temp_word),vocab_file)) {
    if (strncmp(temp_word,"##",2)==0) continue;
    sscanf (temp_word, "%s ",temp_word2);

    /*    printf("hey hey %s %d\n ", temp_word2, idngram_hash(temp_word2,M));*/

    /* Check for repeated words in the vocabulary */    
    if (index2(vocabulary,temp_word2) != 0)
      warn_on_repeated_words(temp_word2);

    warn_on_wrong_vocab_comments(temp_word);
    vocab_size++;
    /*    printf("%s %d\n ", temp_word2, idngram_hash(temp_word2,M));*/

    add_to_idngram_hashtable(vocabulary,idngram_hash(temp_word2,M),temp_word2,vocab_size);
    if(vocab_size == M){
      quit(-1, "Number of entries reached the size of the hash.  Run the program again with a larger has size -hash \n");
    }
  }

  if (vocab_size > MAX_VOCAB_SIZE)    
    fprintf(stderr,"text2idngram : vocab_size %d\n is larger than %d\n",vocab_size,MAX_VOCAB_SIZE);

  return 0;
}

static int ng; /* Static variable to be used in qsort */

int compare_ngrams(const void *ngram1,
		   const void *ngram2
		   ) {

  int i;

  wordid_t *ngram_pointer1;
  wordid_t *ngram_pointer2;

  ngram_pointer1 = (wordid_t *) ngram1;
  ngram_pointer2 = (wordid_t *) ngram2;

  for (i=0;i<=ng-1;i++) {
    if (ngram_pointer1[i]<ngram_pointer2[i]) 
      return(-1);
    else {
      if (ngram_pointer1[i]>ngram_pointer2[i]) 
	return(1);
    }
  }

  return(0);

}

void add_to_buffer(wordid_t word_index,
		   int ypos,
		   int xpos, 
		   wordid_t *buffer) 
{
  buffer[(ng*ypos)+xpos] = word_index;
}

wordid_t buffer_contents(int ypos,
			  int xpos, 
			  wordid_t *buffer) 
{
  return (buffer[(ng*ypos)+xpos]);
}

int get_word( FILE *fp , char *word ) {

  /* read word from stream, checking for read errors and EOF */

  int nread;
  int rt_val;

  rt_val = 0;

  nread = fscanf( fp,MAX_WORD_LENGTH_FORMAT, word );

  if ( nread == 1 )
    rt_val = 1;
  else {
    if ( nread != EOF ) 
      quit(-1, "Error reading file" );
  }
  return(rt_val);
}

/*
  @return number_of_tempfiles
 */
int  read_txt2ngram_buffer(FILE* infp, 
			   struct idngram_hash_table *vocabulary, 
			   int32 verbosity,
			   wordid_t *buffer,
			   int buffer_size,
			   unsigned int n,
			   char* tempfiles_directory,
			   char* temp_file_root,
			   char* temp_file_ext,
			   FILE* temp_file
			   )
{
  /* Read text into buffer */
  char temp_word[MAX_WORD_LENGTH];
  int position_in_buffer;
  int number_of_tempfiles;
  int i,j;
  wordid_t *placeholder;
  wordid_t *temp_ngram;
  int temp_count;

#if 1
  int tmpval;
  int k;
#endif

  temp_ngram  = (wordid_t *) rr_malloc(sizeof(wordid_t)*n);
  placeholder = (wordid_t *) rr_malloc(sizeof(wordid_t)*n);

  ng=n;

  position_in_buffer = 0;
  number_of_tempfiles = 0;

  for (i=0;i<=n-1;i++) {
    get_word(infp,temp_word);
    /*    fprintf(stderr,"%s \n",temp_word);
	  fprintf(stderr,"%d \n",index2(vocabulary,temp_word));*/
    fflush(stderr);

    add_to_buffer(index2(vocabulary,temp_word),0,i,buffer);
  }

  while (!rr_feof(infp)) {
    /* Fill up the buffer */
    pc_message(verbosity,2,"Reading text into the n-gram buffer...\n");
    pc_message(verbosity,2,"20,000 n-grams processed for each \".\", 1,000,000 for each line.\n");

    while ((position_in_buffer<buffer_size) && (!rr_feof(infp))) {
      position_in_buffer++;
      show_idngram_nlines(position_in_buffer,verbosity);

      for (i=1;i<=n-1;i++) 
	add_to_buffer(buffer_contents(position_in_buffer-1,i,buffer),
		      position_in_buffer,i-1,buffer);
      
      if (get_word(infp,temp_word) == 1) {
	/*	fprintf(stderr,"%s \n",temp_word);
	fprintf(stderr,"%d \n",index2(vocabulary,temp_word));
	fflush(stderr);*/

	add_to_buffer(index2(vocabulary,temp_word),position_in_buffer,
		      n-1,buffer);
      }
    }

    for (i=0;i<=n-1;i++) 
      placeholder[i] = buffer_contents(position_in_buffer,i,buffer);

    /* Sort buffer */
    
    pc_message(verbosity,2,"\nSorting n-grams...\n");    
    
    qsort((void*) buffer,(size_t) position_in_buffer,n*sizeof(wordid_t),compare_ngrams);

    /* Output the buffer to temporary BINARY file */    
    number_of_tempfiles++;

    sprintf(temp_word,"%s%s%hu%s",tempfiles_directory,temp_file_root,
	    number_of_tempfiles,temp_file_ext);

    pc_message(verbosity,2,"Writing sorted n-grams to temporary file %s\n",
	       temp_word);

    temp_file = rr_oopen(temp_word);

    for (i=0;i<=n-1;i++) {
      temp_ngram[i] = buffer_contents(0,i,buffer);
#if MAX_VOCAB_SIZE < 65535
      /* This check is well-meaning but completely useless since
	 buffer_contents() can never return something greater than
	 MAX_VOCAB_SIZE (dhuggins@cs, 2006-03) */
      if (temp_ngram[i] > MAX_VOCAB_SIZE)
	quit(-1,"Invalid trigram in buffer.\nAborting");
#endif
    }
    temp_count = 1;

    for (i=1;i<=position_in_buffer;i++) {

      tmpval=compare_ngrams(temp_ngram,&buffer[i*n]);

      /*      for(k=0;k<=n-1;k++){
	fprintf(stderr, "tmpval: %d k %d, temp_ngram %d, &buffer[i*n] %d\n",tmpval, k, temp_ngram[k], (&buffer[i*n])[k]);
	}*/

      if (!compare_ngrams(temp_ngram,&buffer[i*n])) 
	temp_count++;
      else {
	/*	printf("Have been here?\n");*/
	for (j=0;j<=n-1;j++) {
	  rr_fwrite((char*) &temp_ngram[j],sizeof(wordid_t),1,
		    temp_file,"temporary n-gram ids");
	  temp_ngram[j] = buffer_contents(i,j,buffer);
	}
	rr_fwrite((char*)&temp_count,sizeof(int),1,temp_file,
		  "temporary n-gram counts");

	/*	for(j=0 ; j<=n-1;j++)
	  fprintf(stderr,"%d ",temp_ngram[j]);
	  fprintf(stderr,"%d\n",temp_count);*/

	temp_count = 1;
      }
    }
    
    rr_oclose(temp_file);

    for (i=0;i<=n-1;i++) 
      add_to_buffer(placeholder[i],0,i,buffer);

    position_in_buffer = 0;

  }

  return number_of_tempfiles;
}

