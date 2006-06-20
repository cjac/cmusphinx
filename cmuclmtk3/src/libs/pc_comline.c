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




#include <string.h>
#include <stdlib.h>
#include "../libs/general.h"
#include "pc_general.h"
#include "../liblmest/toolkit.h"

/* update the command line argument sequence */
void updateArgs( int *pargc, char **argv, int rm_cnt ) 
{
  int i ;                      
  /* update the argument count */
  (*pargc)-- ;
 
  /* update the command line */
  for( i = rm_cnt ; i < *pargc ; i++ ) argv[i] = argv[i+1] ;
}

int pc_flagarg(int *argc, char **argv, char *flag) {

  int i;
  
  for(i = 1; i < *argc; i++){
    if (!strcmp(argv[i], flag)) {
      updateArgs(argc, argv, i);
      return(1);
    }
  }
  return(0);
}

char *pc_stringarg(int *argc, char **argv, char *flag, char *value) {
  
  int i;
  
  for(i = 1; i < *argc -1; i++){
    if (!strcmp(argv[i], flag)) {
      value = argv[i+1];
      updateArgs(argc, argv, i+1);
      updateArgs(argc, argv, i);
      return(value);
    }
  }
  return(value);
}

int pc_intarg(int *argc, char **argv, char *flag, int value) {

  int i;

  for(i = 1; i < *argc - 1; i++){
    if (!strcmp(argv[i], flag)) {
      value = atoi(argv[i+1]);
      updateArgs(argc, argv, i+1);
      updateArgs(argc, argv, i);
      return(value);
    }
  }
  return(value);
}

double pc_doublearg(int *argc, char **argv, char *flag, double value) {

  int i;

  for(i = 1; i < *argc -1; i++){
    if (!strcmp(argv[i], flag)) {
      value = atof(argv[i+1]);
      updateArgs(argc, argv, i+1);
      updateArgs(argc, argv, i);
      return(value);
    }
  }
  return(value);
}
  
short *pc_shortarrayarg(int *argc, char **argv, char *flag, int elements, int size) {
 
  short *array;
  int i,j;

  array = NULL;

  if (size < elements) {
    quit(-1,"pc_shortarrayarg Error : Size of array is less than number of elements\nto be read.\n");
  }

  for(i = 1; i < *argc-elements; i++){
    if (!strcmp(argv[i], flag)) {

      array = (short *) rr_malloc(size * sizeof(int));

      for (j=0;j<=elements-1;j++)
	array[j] = atoi(argv[i+1+j]);

      for (j=i+elements;j>=i;j--)
	updateArgs(argc, argv, j);

      return(array);
    }
  }

  return(array);
}

int *pc_intarrayarg(int *argc, char **argv, char *flag, int elements, int size) {
 
  int *array;
  int i,j;

  array = NULL;

  if (size < elements) {
    quit(-1,"pc_shortarrayarg Error : Size of array is less than number of elements\nto be read.\n");
  }

  for(i = 1; i < *argc-elements; i++){
    if (!strcmp(argv[i], flag)) {

      array = (int *) rr_malloc(size * sizeof(int));

      for (j=0;j<=elements-1;j++)
	array[j] = atoi(argv[i+1+j]);
      
      for (j=i+elements;j>=i;j--)
	updateArgs(argc, argv, j);

      return(array);
    }
  }

  return(array);
}

void pc_report_unk_args(int *argc, char **argv, int verbosity) {
 
  int i;
 
  if (*argc > 1) {
    fprintf(stderr,"Error : Unknown (or unprocessed) command line options:\n");
    for (i = 1; i< *argc; i++) 
      fprintf(stderr,"%s ",argv[i]);

    quit(-1,"\nRerun with the -help option for more information.\n");
  }
}

void report_version(int *argc, char **argv) {

  if (pc_flagarg(argc,argv,"-version")) 
    quit(-1,"%s from the %s\n",argv[0],CMU_SLM_VERSION);    
}

