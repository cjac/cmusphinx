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
  $Log: text2idngram.c,v $
  Revision 1.13  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.12  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */

#define DEFAULT_HASH_SIZE 2000000
#define DEFAULT_MAX_FILES 20
#define MAX_N 20
#define TEMP_FILE_ROOT "text2idngram.temp."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <unistd.h>
#include "../liblmest/toolkit.h"
#include "../libs/general.h"
#include "../libs/pc_general.h"
#include "../libs/ac_hash.h"
#include "../libs/ac_lmfunc_impl.h"
#include "../programs/idngram.h"

void help_message()
{
  fprintf(stderr,"text2idngram - Convert a text stream to an id n-gram stream.\n");
  fprintf(stderr,"Usage : text2idngram  -vocab .vocab \n");
  fprintf(stderr,"                    [ -buffer 100 ]\n");
  fprintf(stderr,"                    [ -hash %d ]\n",DEFAULT_HASH_SIZE);
  fprintf(stderr,"                    [ -temp %s ]\n",DEFAULT_TEMP);
  fprintf(stderr,"                    [ -files %d ]\n",DEFAULT_MAX_FILES);
  fprintf(stderr,"                    [ -gzip | -compress ]\n");
  fprintf(stderr,"                    [ -verbosity %d ]\n",DEFAULT_VERBOSITY);
  fprintf(stderr,"                    [ -n 3 ]\n");
  fprintf(stderr,"                    [ -write_ascii ]\n");
  fprintf(stderr,"                    [ -fof_size 10 ]\n");
  fprintf(stderr,"                    [ -version ]\n");
  fprintf(stderr,"                    [ -help ]\n");
}

/***************************
      MAIN FUNCTION
 ***************************/

int main(int argc, char *argv[]) {

  char *vocab_filename;
  FILE *tempfile;
  char tempfiles_directory[1000];
  int verbosity;

  int buffer_size;
  int max_files;
  int fof_size;

  wordid_t *buffer;
  char *temp_file_root;
  char *temp_file_ext;

  flag write_ascii;
  flag help_flag ;
  flag compress_flag;
  flag gzip_flag;

  /* Vocab hash table things */
  struct idngram_hash_table vocabulary;
  unsigned long M;
  unsigned long hash_size;
  unsigned int number_of_tempfiles;
  tempfile = NULL; /* Just to prevent compilation warnings. */

  report_version(&argc,argv);
  /* Process command line */

  verbosity      = pc_intarg(&argc,argv,"-verbosity",DEFAULT_VERBOSITY);
  help_flag      = pc_flagarg( &argc, argv,"-help");

  if (help_flag || argc==1) {
    fprintf(stderr,"heyhey %d argc %d\n",help_flag,argc);
    help_message();
    exit(1);
  }

  buffer_size    = pc_intarg( &argc, argv, "-buffer",STD_MEM);  
  vocab_filename = salloc(pc_stringarg( &argc, argv, "-vocab", "" ));
  hash_size      = pc_intarg( &argc, argv, "-hash",DEFAULT_HASH_SIZE);
  strcpy(tempfiles_directory,pc_stringarg( &argc, argv, "-temp",DEFAULT_TEMP));
  max_files      = pc_intarg( &argc, argv, "-files",DEFAULT_MAX_FILES);
  compress_flag  = pc_flagarg(&argc,argv,"-compress");
  gzip_flag      = pc_flagarg(&argc,argv,"-gzip");
  n              = pc_intarg( &argc, argv, "-n",DEFAULT_N);
  write_ascii    = pc_flagarg(&argc,argv,"-write_ascii");
  fof_size       = pc_intarg(&argc,argv,"-fof_size",10);

  /* the version version will be consumed in report_version */
  
  pc_message(verbosity,2,"text2idngram\n");

  
  if (!strcmp("",vocab_filename)) 
    quit(-1,"text2idngram : Error : Must specify a vocabulary file.\n");
    
  if (compress_flag) 
    temp_file_ext = salloc(".Z");
  else {
    if (gzip_flag) 
      temp_file_ext = salloc(".gz");
    else 
      temp_file_ext = salloc("");
  }

  temp_file_root = tempnam(".", TEMP_FILE_ROOT);

  pc_report_unk_args(&argc,argv,verbosity);
  
  /* If the last charactor in the directory name isn't a / then add one. */
  
  if (tempfiles_directory[strlen(tempfiles_directory)-1] != '/') 
    strcat(tempfiles_directory,"/");
  
  pc_message(verbosity,2,"Vocab                  : %s\n",vocab_filename);
  pc_message(verbosity,2,"N-gram buffer size     : %d\n",buffer_size);
  pc_message(verbosity,2,"Hash table size        : %d\n",hash_size);
  pc_message(verbosity,2,"Temp directory         : %s\n",tempfiles_directory);
  pc_message(verbosity,2,"Max open files         : %d\n",max_files);
  pc_message(verbosity,2,"FOF size               : %d\n",fof_size);  
  pc_message(verbosity,2,"n                      : %d\n",n);

  /**
     ARCHAN:
     The buffer size here is the number of wordid_t one needs in mega bytes (approximately). 
   */
  buffer_size *= (1000000/(sizeof(wordid_t)*n));

  /* Allocate memory for hash table */
  fprintf(stderr,"Initialising hash table...\n");

  M=nearest_prime(hash_size);
  new_idngram_hashtable(&vocabulary,M);

  /* Read in the vocabulary */

  read_vocab(vocab_filename,verbosity,&vocabulary,M);
  
  pc_message(verbosity,2,"Allocating memory for the n-gram buffer...\n");

  /* Read text into buffer */
  buffer=(wordid_t*) rr_malloc(n*(buffer_size+1)*sizeof(wordid_t));
  /* Read in the first ngram */

  number_of_tempfiles =  read_txt2ngram_buffer(stdin,
					       &vocabulary,
					       verbosity, 
					       buffer,
					       buffer_size, 
					       n,
					       tempfiles_directory,
					       temp_file_root,
					       temp_file_ext,
					       tempfile
					       );
  
  /* Merge the temporary files, and output the result to standard output */

  pc_message(verbosity,2,"Merging %d temporary files...\n", number_of_tempfiles);
  
  merge_tempfiles(1,
		  number_of_tempfiles,
		  temp_file_root,
		  temp_file_ext,
		  max_files,
		  tempfiles_directory,
		  stdout,
		  write_ascii,
		  fof_size); 

  pc_message(verbosity,0,"text2idngram : Done.\n");

  exit(0);
  
}

