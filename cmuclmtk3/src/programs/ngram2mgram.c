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
  $Log: ngram2mgram.c,v $
  Revision 1.6  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.5  2006/04/15 03:42:23  archan
  Make sure the 8-byte value is fixing.  Typecast all pointers to (char*)

  Revision 1.4  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */

/* 
   Converts a n-gram stream in to an m-gram stream, where n > m 
   Input (from the standard input) can either be :
   
   a binary id n-gram file
   an ascii id n-gram file
   a word n-gram file
   
   Output (at the standard output) will be the same format of the input.

   Note: Program is not intelligent enough to be able to tell value of n from 
   the input, so user must specify it on the command line.

*/

#define BINARY 1
#define ASCII 2
#define WORDS 3

#define NUMERIC 1
#define ALPHA 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../liblmest/toolkit.h"
#include "../liblmest/ngram.h"
#include "../libs/pc_general.h"
#include "../libs/general.h"


void checking(int n, int m)
{
  if (n <= 0)
    quit(-1,"Must specify a positive value for n. Use the -n switch.\n");

  if (m <= 0)
    quit(-1,"Must specify a positive value for m. Use the -m switch.\n");
  
  if (n<=m)
    quit(-1,"n must be greater than m.\n");

}

/***************************
      MAIN FUNCTION
 ***************************/


int main(int argc, char *argv[]) {

  int verbosity;
  int n;
  int m;
  int i;
  int input_type;
  int storage_type;
  unsigned short *current_ngram_int;
  unsigned short *previous_ngram_int;
  char **current_ngram_text;
  char **previous_ngram_text;
  int current_count;
  int running_total;
  flag same;
  flag first_one;
  flag got_to_eof;
   
  running_total = 0;

  report_version(&argc,argv);

  if (pc_flagarg( &argc, argv,"-help") || argc==1) {
    fprintf(stderr,"ngram2mgram - Convert an n-gram file to an m-gram file, where m<n\n");
    fprintf(stderr,"Usage : ngram2mgram   -n N -m M\n");
    fprintf(stderr,"                    [ -binary | -ascii | -words ]\n");
    fprintf(stderr,"                    < .ngram > .mgram\n");
    exit(1);
  }
 
  n = pc_intarg( &argc, argv,"-n",0);
  m = pc_intarg( &argc, argv,"-m",0);
  verbosity = pc_intarg(&argc,argv,"-verbosity",DEFAULT_VERBOSITY);  

  input_type = 0;
  
  if (pc_flagarg( &argc, argv,"-binary"))
    input_type = BINARY;

  if (pc_flagarg( &argc, argv,"-ascii")) {
    if (input_type != 0)
      quit(-1,"Error : more than one file format specified.\n");

    input_type = ASCII;
  }

  if (pc_flagarg( &argc, argv,"-words")) {  
    if (input_type != 0)
      quit(-1,"Error : more than one file format specified.\n");

    input_type = WORDS;
  }    

  if (input_type == 0) {
    pc_message(verbosity,2,"Warning : no input type specified. Defaulting to binary.\n");
    input_type = BINARY;
  }

  checking(n,m);

  pc_report_unk_args(&argc,argv,verbosity);

  if (input_type == BINARY || input_type == ASCII)
    storage_type = NUMERIC;
  else
    storage_type = ALPHA;

  if (storage_type == NUMERIC) {
    current_ngram_int = (unsigned short *) rr_malloc(n*sizeof(unsigned short));
    previous_ngram_int = (unsigned short *) rr_malloc(n*sizeof(unsigned short));

    /* And to prevent compiler warnings ... */

    current_ngram_text = NULL;
    previous_ngram_text = NULL;
  }else {
    current_ngram_text = (char **) rr_malloc(n*sizeof(char *));
    previous_ngram_text = (char **) rr_malloc(n*sizeof(char *));
    for (i=0;i<=n-1;i++) {
      current_ngram_text[i] = (char *) rr_malloc(MAX_WORD_LENGTH*sizeof(char));
      previous_ngram_text[i] = (char *) rr_malloc(MAX_WORD_LENGTH*sizeof(char));
    }
    /* And to prevent compiler warnings ... */
    current_ngram_int = NULL;
    previous_ngram_int = NULL;

  }

  got_to_eof = 0;
  first_one = 1;

  while (!rr_feof(stdin)) {
    /* Store previous n-gram */

    if (!first_one) {
      if (storage_type == NUMERIC) {
	for (i=0;i<=n-1;i++)
	  previous_ngram_int[i] = current_ngram_int[i];
      }else {
	for (i=0;i<=n-1;i++) 
	  strcpy(previous_ngram_text[i],current_ngram_text[i]);
      }
    }

    /* Read new n-gram */

    switch(input_type) {
    case BINARY:
      for (i=0;i<=n-1;i++) {
	rr_fread((char*) &current_ngram_int[i],sizeof(id__t),1,stdin,
		 "from id_ngrams at stdin",0);
      }
      rr_fread((char*)&current_count,sizeof(count_t),1,stdin,
	       "from id_ngrams file at stdin",0);
      break;
    case ASCII:
      for (i=0;i<=n-1;i++) {
	if (fscanf(stdin,"%hu",&current_ngram_int[i]) != 1) {
	  if (!rr_feof(stdin))
	    quit(-1,"Error reading id_ngram.\n");
	  else
	    got_to_eof = 1;
	}
      }
      if (fscanf(stdin,"%d",&current_count) != 1) {
	if (!rr_feof(stdin))
	  quit(-1,"Error reading id_ngram.\n");
	else
	  got_to_eof = 1;
      }
      break;
    case WORDS:
      for (i=0;i<=n-1;i++) {
	if (fscanf(stdin,"%s",current_ngram_text[i]) != 1) {
	  if (!rr_feof(stdin))
	    quit(-1,"Error reading id_ngram.\n");
	  else
	    got_to_eof = 1;
	}
      }
      if (fscanf(stdin,"%d",&current_count) != 1) {
	if (!rr_feof(stdin))
	  quit(-1,"Error reading id_ngram.\n");
	else
	  got_to_eof = 1;
      }
      break;
    }

    if (!got_to_eof) {

      /* Check for correct sorting */

      if (!first_one) {

	switch(storage_type) {
	case NUMERIC:
	  for (i=0;i<=n-1;i++) {
	    if (current_ngram_int[i]<previous_ngram_int[i])
	      quit(-1,"Error : ngrams not correctly sorted.\n");
	    else {
	      if (current_ngram_int[i]>previous_ngram_int[i])
		i=n;
	    }
	  }
	  break;
	case ALPHA:
	  for (i=0;i<=n-1;i++) {
	    if (strcmp(current_ngram_text[i],previous_ngram_text[i])<0)
	      quit(-1,"Error : ngrams not correctly sorted.\n");
	    else {
	      if (strcmp(current_ngram_text[i],previous_ngram_text[i])>0)
		i=n;
	    }
	  }
	  break;
	}
      }

      /* Compare this m-gram with previous m-gram */

      if (!first_one) {

	switch(storage_type) {
	case NUMERIC:
	  same = 1;
	  for (i=0;i<=m-1;i++) {
	    if (current_ngram_int[i] != previous_ngram_int[i])
	      same = 0;
	  }
	  if (same)
	    running_total += current_count;
	  else {
	    if (input_type == ASCII) {
	      for (i=0;i<=m-1;i++)
		printf("%d ",previous_ngram_int[i]);
	      printf("%d\n",running_total);
	    }else {
	      for (i=0;i<=m-1;i++)
		rr_fwrite((char*)&previous_ngram_int[i],sizeof(id__t),1,stdout,
			  "to id_ngrams at stdout");
	      rr_fwrite((char*) &running_total,sizeof(count_t),1,stdout,
			"to id n-grams at stdout");
	    }
	    running_total = current_count;
	  }
	  break;
	case ALPHA:
	  same = 1;
	  for (i=0;i<=m-1;i++) {
	    if (strcmp(current_ngram_text[i],previous_ngram_text[i]))
	      same = 0;	   
	  }
	  if (same)
	    running_total += current_count;
	  else {
	    for (i=0;i<=m-1;i++)
	      printf("%s ",previous_ngram_text[i]);

	    printf("%d\n",running_total);
	    running_total = current_count;
	  
	  }
	  break;
	}
      
      }else
	running_total = current_count;
    
      first_one = 0;
    
    }
  }

  /* Write out final m-gram */

  switch(input_type) {
  case BINARY:
    break;
  case ASCII:
    for (i=0;i<=m-1;i++)
      printf("%d ",previous_ngram_int[i]);
    printf("%d\n",running_total);
    break;
  case WORDS:
    for (i=0;i<=m-1;i++)
      printf("%s ",previous_ngram_text[i]);
    printf("%d\n",running_total);
    break;
  } 

  pc_message(verbosity,0,"ngram2mgram : Done.\n");

  exit(0);

}	  
