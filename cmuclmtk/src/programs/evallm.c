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
  $Log: evallm.c,v $
  Revision 1.6  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.5  2006/04/15 20:59:59  archan
  Added Arthur Toth's n-gram based sentence generator to evallm.  Arthur has given me a very strong precaution to use the code.  So, I will also make sure the current version will only be used by a few.

  Revision 1.4  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */

#define MAX_ARGS 200

#include "../liblmest/evallm.h"
#include "../libs/pc_general.h"
#include "../libs/general.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void help_message(){
   fprintf(stderr,"evallm : Evaluate a language model.\n");
   fprintf(stderr,"Usage : evallm [ -binary .binlm | \n");
   fprintf(stderr,"                 -arpa .arpa [ -context .ccs ] ]\n");
}

void evallm_command_help_message()
{
  printf("The user may specify one of the following commands: \n");
  printf("\n");
  printf(" - perplexity\n");
  printf("\n");
  printf("Computes the perplexity of a given text. May optionally specify words\n");
  printf("from which to force back-off.\n");
  printf("\n");
  printf("Syntax: \n");
  printf("\n");
  printf("perplexity -text .text\n");
  printf("         [ -probs .fprobs ]\n");
  printf("         [ -oovs .oov_file ]\n");
  printf("         [ -annotate .annotation_file ]         \n");
  printf("         [ -backoff_from_unk_inc | -backoff_from_unk_exc ]\n");
  printf("         [ -backoff_from_ccs_inc | -backoff_from_ccs_exc ] \n");
  printf("         [ -backoff_from_list .fblist ]\n");
  printf("         [ -include_unks ]\n");
  printf("\n");
  printf(" - validate\n");
  printf("       \n");
  printf("Calculate the sum of the probabilities of all the words in the\n");
  printf("vocabulary given the context specified by the user.\n");
  printf("\n");
  printf("Syntax: \n");
  printf("\n");
  printf("validate [ -backoff_from_unk -backoff_from_ccs |\n");
  printf("           -backoff_from_list .fblist ]\n");
  printf("         [ -forced_backoff_inc | -forced_back_off_exc ]      \n");
  printf("           word1 word2 ... word_(n-1)\n");
  printf("\n");
  printf("Where n is the n in n-gram. \n");
  printf("\n");
  printf(" - generate\n");
  printf("       \n");
  printf("Calculate the sum of the probabilities of all the words in the\n");
  printf("vocabulary given the context specified by the user.\n");
  printf("\n");
  printf("Syntax: \n");
  printf("\n");
  printf("generate -seed seed_of_random_generator -size size_of_file -text output text file \n");
  printf("\n");
  printf(" - help\n");
  printf("\n");
  printf("Displays this help message.\n");
  printf("\n");
  printf("Syntax: \n");
  printf("\n");
  printf("help\n");
  printf("\n");
  printf(" - quit\n");
  printf("\n");
  printf("Exits the program.\n");
  printf("\n");
  printf("Syntax: \n");
  printf("\n");
  printf("quit\n");	
}

int main (int argc, char **argv) {

  ng_t ng;
  arpa_lm_t arpa_ng;
  char input_string[500];
  int num_of_args;
  char *args[MAX_ARGS];
  char *lm_filename_arpa;
  char *lm_filename_binary;
  flag told_to_quit;
  flag inconsistant_parameters;
  flag backoff_from_unk_inc;
  flag backoff_from_unk_exc;
  flag backoff_from_ccs_inc;
  flag backoff_from_ccs_exc;
  flag arpa_lm;
  flag binary_lm;
  flag include_unks;
  char *fb_list_filename;
  char *probs_stream_filename;
  char *annotation_filename;
  char *text_stream_filename;
  char *oov_filename;
  char *ccs_filename;
  int generate_size;
  int random_seed;
  double log_base;
  char wlist_entry[1024];
  char current_cc[200];
  vocab_sz_t current_cc_id;
  FILE *context_cues_fp;
  int n;

  /* Process command line */

  report_version(&argc,argv);

  if (pc_flagarg(&argc, argv,"-help") || 
      argc == 1 || 
      (strcmp(argv[1],"-binary") && strcmp(argv[1],"-arpa"))) {
    help_message();
    exit(1);
  }

  lm_filename_arpa = salloc(pc_stringarg(&argc, argv,"-arpa",""));

  if (strcmp(lm_filename_arpa,""))
    arpa_lm = 1;
  else
    arpa_lm = 0;

  lm_filename_binary = salloc(pc_stringarg(&argc, argv,"-binary",""));

  if (strcmp(lm_filename_binary,""))
    binary_lm = 1;
  else
    binary_lm = 0;

  if (arpa_lm && binary_lm)
    quit(-1,"Error : Can't use both -arpa and -binary flags.\n");
  
  if (!arpa_lm && !binary_lm)
    quit(-1,"Error : Must specify either a binary or an arpa format language model.\n");

  ccs_filename = salloc(pc_stringarg(&argc, argv,"-context",""));

  if (binary_lm && strcmp(ccs_filename,""))
    fprintf(stderr,"Warning - context cues file not needed with binary language model file.\nWill ignore it.\n");

  pc_report_unk_args(&argc,argv,2);
 
  /* Load language model */

  if (arpa_lm) {
    fprintf(stderr,"Reading in language model from file %s\n",
	    lm_filename_arpa);
    load_arpa_lm(&arpa_ng,lm_filename_arpa);
  }else {
    fprintf(stderr,"Reading in language model from file %s\n",
	    lm_filename_binary);
    load_lm(&ng,lm_filename_binary); 
  }

  fprintf(stderr,"\nDone.\n");

  n=arpa_lm?
    arpa_ng.n:
    ng.n;

  if (arpa_lm) {
    arpa_ng.context_cue = 
      (flag *) rr_calloc(arpa_ng.table_sizes[0],sizeof(flag));    
    arpa_ng.no_of_ccs = 0;
    if (strcmp(ccs_filename,"")) {
      context_cues_fp = rr_iopen(ccs_filename);
      while (fgets (wlist_entry, sizeof (wlist_entry),context_cues_fp)) {
	if (strncmp(wlist_entry,"##",2)==0) continue;
	sscanf (wlist_entry, "%s ",current_cc);

	warn_on_wrong_vocab_comments(wlist_entry);
	
	if (sih_lookup(arpa_ng.vocab_ht,current_cc,&current_cc_id) == 0)
	  quit(-1,"Error : %s in the context cues file does not appear in the vocabulary.\n",current_cc);
	
	arpa_ng.context_cue[(unsigned short) current_cc_id] = 1;
	arpa_ng.no_of_ccs++;
	fprintf(stderr,"Context cue word : %s id = %d\n",current_cc,current_cc_id);
      }
      rr_iclose(context_cues_fp);
    }
  }

  /* Process commands */
  
  told_to_quit = 0;
  num_of_args = 0;

  while (!feof(stdin) && !told_to_quit) {
    printf("evallm : \n");
    fgets(input_string, sizeof(input_string), stdin);
    if(strlen(input_string) < sizeof(input_string)-1)
      input_string[strlen(input_string)-1] = '\0'; //chop new-line
    else 
      quit(1, "evallm input exceeds size of input buffer");

    if (!feof(stdin)) {
      parse_comline(input_string,&num_of_args,args);

      log_base = pc_doublearg(&num_of_args,args,"-log_base",10.0);

      backoff_from_unk_inc = pc_flagarg(&num_of_args,args,"-backoff_from_unk_inc");
      backoff_from_ccs_inc = pc_flagarg(&num_of_args,args,"-backoff_from_ccs_inc");
      backoff_from_unk_exc = pc_flagarg(&num_of_args,args,"-backoff_from_unk_exc");
      backoff_from_ccs_exc = pc_flagarg(&num_of_args,args,"-backoff_from_ccs_exc");
      include_unks = pc_flagarg(&num_of_args,args,"-include_unks");
      fb_list_filename = salloc(pc_stringarg(&num_of_args,args,"-backoff_from_list",""));
    
      text_stream_filename = 
	salloc(pc_stringarg(&num_of_args,args,"-text",""));
      probs_stream_filename = 
	salloc(pc_stringarg(&num_of_args,args,"-probs",""));
      annotation_filename = 
	salloc(pc_stringarg(&num_of_args,args,"-annotate",""));
      oov_filename = salloc(pc_stringarg(&num_of_args,args,"-oovs",""));

      generate_size = pc_intarg(&num_of_args,args,"-size",10000);
      random_seed = pc_intarg(&num_of_args,args,"-seed",-1);

      inconsistant_parameters = 0;
    
      if (backoff_from_unk_inc && backoff_from_unk_exc) {
	fprintf(stderr,"Error : Cannot specify both exclusive and inclusive forced backoff.\n");
	fprintf(stderr,"Use only one of -backoff_from_unk_exc and -backoff_from_unk_inc\n");
	inconsistant_parameters = 1;
      }

      if (backoff_from_ccs_inc && backoff_from_ccs_exc) {
	fprintf(stderr,"Error : Cannot specify both exclusive and inclusive forced backoff.\n");
	fprintf(stderr,"Use only one of -backoff_from_ccs_exc and -backoff_from_ccs_inc\n");
	inconsistant_parameters = 1;
      }

      if (num_of_args > 0) {      
	if (!inconsistant_parameters) {
	  if (!strcmp(args[0],"perplexity")) {
	    compute_perplexity(&ng,
			       &arpa_ng,
			       text_stream_filename,
			       probs_stream_filename,
			       annotation_filename,
			       oov_filename,
			       fb_list_filename,
			       backoff_from_unk_inc,
			       backoff_from_unk_exc,
			       backoff_from_ccs_inc,
			       backoff_from_ccs_exc,
			       arpa_lm,
			       include_unks,
			       log_base);
	  }else
	    /* do perplexity sentence by sentence [20090612] (air) */
	    if (!strcmp(args[0],"uttperp")) {
	      FILE *uttfh,*tempfh;
	      char utt[4096]; /* live dangerously... */
	      char tmpfil[128];
	      if ((uttfh = fopen(text_stream_filename,"r")) == NULL) {
		printf("Error: can't open %s\n",text_stream_filename);
		exit(1);
	      }
	      strcpy(tmpfil,tempnam(NULL,"uttperp_"));
	      while ( ! feof(uttfh) ) {
		fscanf(uttfh,"%[^\n]\n",utt);
		tempfh = fopen(tmpfil,"w");
		fprintf(tempfh,"%s\n",utt);
		fclose(tempfh);
		compute_perplexity(&ng,
				   &arpa_ng,
				   tmpfil,  /* text_stream_filename, */
				   probs_stream_filename,
				   annotation_filename,
				   oov_filename,
				   fb_list_filename,
				   backoff_from_unk_inc,
				   backoff_from_unk_exc,
				   backoff_from_ccs_inc,
				   backoff_from_ccs_exc,
				   arpa_lm,
				   include_unks,
				   log_base);
	      }
	      fclose(uttfh);
	      // unlink(tmpfil);
	  }else if(!strcmp(args[0],"validate")){
	    if (num_of_args != n) 
	      fprintf(stderr,"Error : must specify %d words of context.\n",n-1);
	    else {  /* Assume last n-1 parameters form context */
	      validate(&ng,
		       &arpa_ng,
		       &(args[num_of_args-n+1]),
		       backoff_from_unk_inc,
		       backoff_from_unk_exc,
		       backoff_from_ccs_inc,
		       backoff_from_ccs_exc,
		       arpa_lm,
		       fb_list_filename);
	    }
	  }else if (!strcmp(args[0],"stats")){
	    if (arpa_lm)
	      display_arpa_stats(&arpa_ng);
	    else
	      display_stats(&ng);
	  }else if (!strcmp(args[0],"generate")) {

	    if(arpa_lm)
	      generate_words(NULL,&arpa_ng,generate_size,random_seed,text_stream_filename);
	    else
	      generate_words(&ng,NULL,generate_size,random_seed,text_stream_filename);

	  }else if (!strcmp(args[0],"quit"))
	    told_to_quit=1;
	  else if (!strcmp(args[0],"help"))
	    evallm_command_help_message();
	  else
	    fprintf(stderr,"Unknown command : %s\nType \'help\'\n",
		    args[0]);
	}
      }
    }    
  }

  fprintf(stderr,"evallm : Done.\n");

  exit(0);
  
}
