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
 * HISTORY
 * $Log: idngram2lm.c,v $
 * Revision 1.12  2006/06/19 21:02:08  archan
 * Changed license from the original research-only license to the BSD license.
 *
 * Revision 1.11  2006/04/15 17:07:49  archan
 * Added comments for thirty two-bits hash table issues.  Changed %lld back to %ld.
 *
 * Revision 1.10  2006/04/15 02:41:53  archan
 * Changes the initial size of the vocab_ht.
 *
 * Revision 1.9  2006/04/13 17:36:37  archan
 * 0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)
 *
 * Revision 1.8  2006/04/03 00:02:34  archan
 * Used disc_meth,  used specific data structure. It is still very convoluted
 *
 * Revision 1.7  2006/03/31 04:24:08  archan
 * Changed style, tied initialization of discounting method, use warn_on_wrong_vocab_comments.
 *
 */

#include "../liblmest/toolkit.h"
#include "../liblmest/ngram.h"
#include "../libs/pc_general.h"
#include "../liblmest/idngram2lm.h"
#include "../liblmest/disc_meth.h"
#include "../libs/sih.h"
#include "../libs/general.h"
#include "../libs/ac_parsetext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void help_message()
{
    fprintf(stderr,"idngram2lm : Convert an idngram file to a language model file.\n");
    fprintf(stderr,"Usage : \n");
    fprintf(stderr,"idngram2lm -idngram .idngram\n");
    fprintf(stderr,"           -vocab .vocab\n");
    fprintf(stderr,"           -arpa .arpa | -binary .binlm\n");
    fprintf(stderr,"         [ -context .ccs ]\n");
    fprintf(stderr,"         [ -calc_mem | -buffer 100 | -spec_num y ... z ]\n");
    fprintf(stderr,"         [ -vocab_type 1 ]\n");
    fprintf(stderr,"         [ -oov_fraction 0.5 ]\n");
    fprintf(stderr,"         [ -two_byte_bo_weights   \n              [ -min_bo_weight nnnnn] [ -max_bo_weight nnnnn] [ -out_of_range_bo_weights] ]\n");
    fprintf(stderr,"         [ -four_byte_counts ]\n");
    fprintf(stderr,"         [ -linear | -absolute | -good_turing | -witten_bell ]\n");
    fprintf(stderr,"         [ -disc_ranges 1 7 7 ]\n");
    fprintf(stderr,"         [ -cutoffs 0 ... 0 ]\n");
    fprintf(stderr,"         [ -min_unicount 0 ]\n");
    fprintf(stderr,"         [ -zeroton_fraction ]\n");
    fprintf(stderr,"         [ -ascii_input | -bin_input ]\n");
    fprintf(stderr,"         [ -n 3 ]  \n");
    fprintf(stderr,"         [ -verbosity %d ]\n",DEFAULT_VERBOSITY);

}

ng_t * init_ng(
	     int* argc,
	     char **argv,
	     int verbosity
	     )
{
  int i;
  ng_t* ng;
  
  ng=(ng_t*) rr_calloc(1,sizeof(ng_t));

  ng->disc_meth=NULL;
  /* -n */
  ng->n = pc_intarg(argc, argv,"-n",DEFAULT_N); 

  if (ng->n<1) 
    quit(-1,"Error: Value of n must be larger than zero.\n");

  /* -cutoffs */
  ng->cutoffs = (cutoff_t *) pc_shortarrayarg(argc, argv, "-cutoffs",ng->n-1,ng->n-1);

  if (ng->cutoffs == NULL) 
    ng->cutoffs = (cutoff_t *) rr_calloc((ng->n-1)+1,sizeof(cutoff_t)); /* +1 for the sake of the correction in writing in write_lms.c */

  for (i=0;i<=ng->n-3;i++) {
    if (ng->cutoffs[i+1] < ng->cutoffs[i]) {
      quit(-1,"Error - cutoffs for (n+1)-gram must be greater than or equal to those for \nn-gram. You have %d-gram cutoff = %d > %d-gram cutoff = %d.\n",i+2,ng->cutoffs[i],i+3,ng->cutoffs[i+1]);
    }
  }

  /* -min_unicount */
  ng->min_unicount = pc_intarg(argc, argv, "-min_unicount",0);

  /* -idngram */
  ng->id_gram_filename = salloc(pc_stringarg(argc, argv,"-idngram",""));

  if (!strcmp(ng->id_gram_filename,""))
    quit(-1,"Error: id ngram file not specified. Use the -idngram flag.\n");

  /* -arpa & -bin */
  ng->arpa_filename = salloc(pc_stringarg(argc, argv,"-arpa",""));
  ng->bin_filename = salloc(pc_stringarg(argc, argv,"-binary",""));
  
  ng->write_arpa = strcmp("",ng->arpa_filename);
  ng->write_bin = strcmp("",ng->bin_filename);
  
  if (!(ng->write_arpa || ng->write_bin)) 
    quit(-1,"Error : must specify either an arpa, or a binary output file.\n");

  ng->count_table_size = DEFAULT_COUNT_TABLE_SIZE;

  /* -vocab */
  ng->vocab_filename = salloc(pc_stringarg(argc,argv,"-vocab",""));
 
  if (!strcmp("",ng->vocab_filename))
    quit(-1,"Error : vocabulary file not specified. Use the -vocab option.\n");  

  /* -context */
  ng->context_cues_filename = salloc(pc_stringarg(argc,argv,"-context",""));

  ng->context_set = strcmp("", ng->context_cues_filename);

  /* -vocab_type */
  ng->vocab_type = pc_intarg(argc,argv,"-vocab_type",1);
  
  /* -oov_fraction */
  ng->oov_fraction = pc_doublearg(argc, argv,"-oov_fraction",-1.0);


  if (ng->oov_fraction == -1.0)
    ng->oov_fraction=DEFAULT_OOV_FRACTION;
  else {
    if (ng->vocab_type != 2)
      pc_message(verbosity,1,"Warning : OOV fraction specified, but will not be used, since vocab type is not 2.\n");
  }

  if (ng->vocab_type == 0) 
    ng->first_id = 1;
  else
    ng->first_id = 0;

  /* Allow both "min_alpha" etc and "min_bo_weight" etc as valid
     syntax. The "bo_weight" form is preferred, but the "alpha" form is
     maintained as it was present in version 2.00 */

  ng->min_alpha = pc_doublearg(argc,argv,"-min_alpha",DEFAULT_MIN_ALPHA);
  ng->max_alpha = pc_doublearg(argc,argv,"-max_alpha",DEFAULT_MAX_ALPHA);
  ng->out_of_range_alphas = pc_intarg(argc,argv,"-out_of_range_alphas",
				      DEFAULT_OUT_OF_RANGE_ALPHAS);

  ng->min_alpha = pc_doublearg(argc,argv,"-min_bo_weight",ng->min_alpha);
  ng->max_alpha = pc_doublearg(argc,argv,"-max_bo_weight",ng->max_alpha);
  ng->out_of_range_alphas = pc_intarg(argc,argv,"-out_of_range_bo_weights",
				      ng->out_of_range_alphas);
  
  if (ng->min_alpha >= ng->max_alpha)
    quit(-1,"Error : Minimum of alpha range must be less than the maximum.\n");


  init_ng_disc_method(ng,
		      pc_flagarg(argc, argv,"-linear"),
		      pc_flagarg(argc,argv,"-absolute"),
		      pc_flagarg(argc,argv,"-witten_bell"),
		      pc_flagarg(argc,argv,"-good_turing"));		      
		      
  ng->disc_range = (unsigned short *) pc_shortarrayarg(argc, argv, "-disc_ranges",ng->n,ng->n);

  ng->disc_range_set = (ng->disc_range != NULL);

  if (ng->discounting_method == GOOD_TURING) {
    if (!ng->disc_range_set) {
      ng->disc_range = (unsigned short *) rr_malloc(sizeof(unsigned short) * ng->n);
      ng->disc_range[0] = DEFAULT_DISC_RANGE_1;
      for (i=1;i<=ng->n-1;i++) 
	ng->disc_range[i] = DEFAULT_DISC_RANGE_REST;
    }
    ng->fof_size = (fof_sz_t *) rr_malloc(sizeof(fof_sz_t) * ng->n);
    for (i=0;i<=ng->n-1;i++) 
      ng->fof_size[i] = ng->disc_range[i]+1;

  }else {
    if (ng->disc_range_set) 
      pc_message(verbosity,2,"Warning : discount ranges specified will be ignored, since they only apply\nto Good Turing discounting.\n");
  }

  ng->four_byte_alphas = !(pc_flagarg(argc, argv, "-two_byte_alphas") || 
			   pc_flagarg(argc, argv, "-two_byte_bo_weights"));

  ng->four_byte_counts = pc_flagarg(argc, argv, "-four_byte_counts");
  if(ng->four_byte_counts){
      pc_message(verbosity,2,"Using Four byte counts.\n");
  }

  ng->zeroton_fraction = pc_doublearg(argc,argv,"-zeroton_fraction",1.0);

  /* Attempt to open all the files that we will need for input and
     output. It is better to do it here than to spend a few hours of
     CPU processing id-gram counts, only to find that the output path
     is invalid. */

  ng->id_gram_fp = rr_iopen(ng->id_gram_filename);

  /* Vocab is read by Roni's function which does the file opening for
     us, so no need to do it here. Don't need to worry about time
     being lost if file doesn't exist, since vocab is first thing to
     be read anyway. */

  if (ng->context_set)
    ng->context_cues_fp = rr_iopen(ng->context_cues_filename);

  if (ng->write_arpa)
    ng->arpa_fp = rr_oopen(ng->arpa_filename);

  if (ng->write_bin) 
    ng->bin_fp = rr_oopen(ng->bin_filename);

  return ng;
}
	
void init_ng_table_size(ng_t *ng, 
		        flag mem_alloc_method, 
		        flag is_ascii,
		        int verbosity,
		        int buffer_size
		        )
{
  int middle_size;
  int end_size;

  if (ng->n>1) {
    switch(mem_alloc_method) {

    case TWO_PASSES: 
      ng->table_sizes = (table_size_t *) rr_calloc(ng->n,sizeof(table_size_t));
      pc_message(verbosity,2,"Calculating memory requirement.\n");
      calc_mem_req(ng,is_ascii);
      break;
    case BUFFER:
      ng->table_sizes = (table_size_t *) rr_malloc(ng->n*sizeof(table_size_t));
      middle_size = sizeof(count_ind_t) + sizeof(bo_weight_t) + 
	sizeof(index__t) + sizeof(id__t);
      end_size = sizeof(count_ind_t) + sizeof(id__t);

      if (ng->four_byte_alphas)
	middle_size += 2;

      if (ng->four_byte_counts) {
	middle_size += 2;
	end_size += 2;
      }   

      guess_mem(buffer_size,
		middle_size,
		end_size,
		ng->n,
		ng->table_sizes,
		verbosity);
      break;
    case SPECIFIED:
      break;
    }  
  }else{
    ng->table_sizes = (table_size_t *) rr_calloc(1,sizeof(table_size_t));
  }

  ng->table_sizes[0] = ng->vocab_size+1;

}		

int init_alloc_method(ng_t *ng,
		      int* argc,
		      char **argv,
		      int* buffer_size)
{
  int mem_alloc_method;
  int i;

  mem_alloc_method = 0;
  
  if (pc_flagarg(argc, argv,"-calc_mem")) 
    mem_alloc_method = TWO_PASSES;  

  *buffer_size = pc_intarg(argc, argv,"-buffer",-1);

  if (*buffer_size != -1) {
    if (mem_alloc_method != 0)
      quit(-1,"Assigned two contradictory methods of memory allocation.\n Use one of -calc_mem, -buffer, or -spec_num.\n");

    mem_alloc_method = BUFFER;
  }

  ng->table_sizes = pc_intarrayarg(argc, argv, "-spec_num",ng->n-1,ng->n);

  if (ng->table_sizes != NULL) {
    if (mem_alloc_method != 0) {
      quit(-1,"Assigned two contradictory methods of memory allocation.\n Use one of -calc_mem, -guess, or -spec_num.\n");
    }
    mem_alloc_method = SPECIFIED;
    for (i=ng->n-1;i>=1;i--)
      ng->table_sizes[i] = ng->table_sizes[i-1];
  }

  if (mem_alloc_method == 0) {
    mem_alloc_method = BUFFER;
    *buffer_size = STD_MEM;
  }

  return mem_alloc_method;
}

flag set_lmformat(flag ascii_flag,
		  flag bin_flag,
		  ng_t *ng
		  )
{
  flag is_ascii ;
  flag is_ascii_fn;
  flag is_bin_fn;

  is_ascii =0;  
  is_ascii_fn=!strcmp(&ng->id_gram_filename[strlen(ng->id_gram_filename)-6],".ascii");
  is_bin_fn=!strcmp(&ng->id_gram_filename[strlen(ng->id_gram_filename)-4],".bin");

  if (ascii_flag || is_ascii_fn) 
    is_ascii = 1;
  
  if (ascii_flag && bin_flag) 
    quit(-1,"Error : Specify only one of -bin_input and -ascii_input\n");    
  
  if (ascii_flag && is_bin_fn) 
    quit(-1,"Error : -ascii_input flag specified, but input file has .bin extention.\n");
  
  if (bin_flag && is_ascii_fn ) 
    quit(-1,"Error : -bin_input flag specified, but input file has .ascii extention.\n");

  return is_ascii;
}

void report_param(int verbosity, ng_t *ng, 
		  flag is_ascii,
		  flag mem_alloc_method,
		  int buffer_size
		  )
{
  int i;
  pc_message(verbosity,2,"  n : %d\n",ng->n);
  pc_message(verbosity,2,"  Input file : %s",ng->id_gram_filename);
  if (is_ascii) 
    pc_message(verbosity,2,"     (ascii format)\n");
  else 
    pc_message(verbosity,2,"     (binary format)\n");

  pc_message(verbosity,2,"  Output files :\n");

  if (ng->write_arpa) 
    pc_message(verbosity,2,"     ARPA format   : %s\n",ng->arpa_filename);
  if (ng->write_bin) 
    pc_message(verbosity,2,"     Binary format : %s\n",ng->bin_filename);

  pc_message(verbosity,2,"  Vocabulary file : %s\n",ng->vocab_filename);
  if (ng->context_set) 
    pc_message(verbosity,2,"  Context cues file : %s\n",ng->context_cues_filename);

  pc_message(verbosity,2,"  Cutoffs :\n     ");
  for (i=0;i<=ng->n-2;i++) 
    pc_message(verbosity,2,"%d-gram : %d     ",i+2,ng->cutoffs[i]);

  pc_message(verbosity,2,"\n");

  switch (ng->vocab_type) {
  case CLOSED_VOCAB:
    pc_message(verbosity,2,"  Vocabulary type : Closed\n");
    break;
  case OPEN_VOCAB_1:
    pc_message(verbosity,2,"  Vocabulary type : Open - type 1\n");
    break;
  case OPEN_VOCAB_2:
    pc_message(verbosity,2,"  Vocabulary type : Open - type 2\n");
    pc_message(verbosity,2,"     OOV fraction = %g\n",ng->oov_fraction);
    break;
  }
  pc_message(verbosity,2,"  Minimum unigram count : %d\n",ng->min_unicount);
  pc_message(verbosity,2,"  Zeroton fraction : %g\n",ng->zeroton_fraction);
  if (ng->four_byte_counts) 
    pc_message(verbosity,2,"  Counts will be stored in four bytes.\n");
  else {
    pc_message(verbosity,2,"  Counts will be stored in two bytes.\n");
    pc_message(verbosity,2,"  Count table size : %d\n",ng->count_table_size);
  }

  pc_message(verbosity,2,"  Discounting method : ");
  NG_DISC_METH(ng)->verbose_method(ng, verbosity);

  pc_message(verbosity,2,"  Memory allocation for tree structure : \n");
  switch(mem_alloc_method) {
  case TWO_PASSES:
    pc_message(verbosity,2,"     Perform a preliminary pass over the id n-gram file to determine \n     the amount of memory to allocate\n");
    break;
  case BUFFER:
    pc_message(verbosity,2,"     Allocate %d MB of memory, shared equally between all n-gram tables.\n",buffer_size);
    break;
  case SPECIFIED:
    pc_message(verbosity,2,"     Memory requirement specified.\n          ");
    for (i=0;i<=ng->n-2;i++)
      pc_message(verbosity,2,"%d-gram : %d     ",i+2,ng->table_sizes[i+1]);
    pc_message(verbosity,2,"\n");
    break;
  }
  pc_message(verbosity,2,"  Back-off weight storage : \n");

  if (ng->four_byte_alphas) 
    pc_message(verbosity,2,"     Back-off weights will be stored in four bytes.\n");
  else {
    pc_message(verbosity,2,"     Back-off weights will be stored in two bytes.\n");
    pc_message(verbosity,2,"        Minimum back-off weight : %g\n",ng->min_alpha);
    pc_message(verbosity,2,"        Maximum back-off weight : %g\n",ng->max_alpha);
    pc_message(verbosity,2,"        Maximum number of out of range back-off weights : %d\n",ng->out_of_range_alphas);
  }

}

void read_vocab(ng_t* ng, int verbosity)
{
  vocab_sz_t test_cc_id;
  vocab_sz_t current_cc_id;
  char current_cc[200];
  char wlist_entry[1024];

  pc_message(verbosity,2,"Reading vocabulary.\n");

  /* Don't change the parameter of sih_create, because it will change
     the binary layout of the .binlm file */

  ng->vocab_ht =
    sih_create(1000,0.5,2.0,1);

  read_voc(ng->vocab_filename,verbosity,ng->vocab_ht,&ng->vocab,&(ng->vocab_size));
  
  /* Determine which of the vocabulary words are context cues */

  ng->no_of_ccs = 0;
  ng->context_cue = (flag *) rr_calloc(ng->vocab_size+1,sizeof(flag));

  if (ng->context_set) {
    /* This should be tied to l889 to l894 in lm_combine.c
     */
    while (fgets (wlist_entry, sizeof (wlist_entry),ng->context_cues_fp)) {
      if (strncmp(wlist_entry,"##",2)==0) continue;
      sscanf (wlist_entry, "%s ",current_cc);
      warn_on_wrong_vocab_comments(wlist_entry);

      if (sih_lookup(ng->vocab_ht,current_cc,&current_cc_id) == 0) 
	pc_message(verbosity,1,"Warning : %s in the context cues file does not appear in the vocabulary.\n",current_cc);
      else {
	ng->context_cue[(unsigned short) current_cc_id] = 1;
	pc_message(verbosity,2,"Context cue word : %s id = %d\n",current_cc,current_cc_id);
	ng->no_of_ccs++;
      }
    }
    rr_iclose(ng->context_cues_fp);
  }

  if ((sih_lookup(ng->vocab_ht,"<s>",&test_cc_id) != 0)) 
    if (ng->context_cue[(unsigned short) test_cc_id] == 0) 
      fprintf(stderr,"WARNING: <s> appears as a vocabulary item, but is not labelled as a\ncontext cue.\n");

  if ((sih_lookup(ng->vocab_ht,"<p>",&test_cc_id) != 0)) 
    if (ng->context_cue[(unsigned short) test_cc_id] == 0) 
      fprintf(stderr,"WARNING: <p> appears as a vocabulary item, but is not labelled as a\ncontext cue.\n");

  if ((sih_lookup(ng->vocab_ht,"<art>",&test_cc_id) != 0)) 
    if (ng->context_cue[(unsigned short) test_cc_id] == 0) 
      fprintf(stderr,"WARNING: <art> appears as a vocabulary item, but is not labelled as a\ncontext cue.\n");

}

int main(int argc, char **argv) {

  int i,j;
  ng_t* ng;
  int verbosity;
  int mem_alloc_method; /* Method used to decide how much memory to 
			   allocate for count tables */
  int buffer_size;
  flag is_ascii;
  ngram current_ngram;
  ngram previous_ngram;
  count_t *ng_count; /* Array indicating the number of occurrances of 
			   the current 1-gram, 2-gram, ... ,n-gram 
			   Size depends on #define in general.h
			*/  
  int nlines;
  int pos_of_novelty;
  int prev_id1;
  flag contains_unks;
  int mem_alloced;

  flag displayed_oov_warning; /** Display OOV warning 
			       */

  /*  ------------------  Process command line --------------------- */

  report_version(&argc,argv);

  if (argc == 1 || pc_flagarg(&argc, argv,"-help")) {    
    /* Display help message */    
    help_message();
    exit(1);
  }

  verbosity = pc_intarg(&argc, argv,"-verbosity",DEFAULT_VERBOSITY);

  /* Initialization */
  {
    ng=init_ng(
	    &argc,
	    argv,
	    verbosity
	    );
    
    mem_alloc_method = init_alloc_method(ng, &argc, argv, &buffer_size);
    
    if (!strcmp(ng->id_gram_filename,"-") && mem_alloc_method == TWO_PASSES)
      quit(-1,"Error: If idngram is read from stdin, then cannot use -calc_mem option.\n");
    
    is_ascii = set_lmformat(pc_flagarg(&argc,argv,"-ascii_input"),
			    pc_flagarg(&argc,argv,"-bin_input"),
			    ng);  

    /* Report parameters */
    report_param(verbosity,ng,
		 is_ascii, mem_alloc_method, buffer_size);

    pc_report_unk_args(&argc,argv,verbosity);

  }

  /* --------------- Read in the vocabulary -------------- */
  read_vocab(ng,verbosity);
       		     
  /* --------------- Allocate space for the table_size array --------- */
  init_ng_table_size(ng, 
		     mem_alloc_method,
		     is_ascii,
		     verbosity,
		     buffer_size
		     );

  /* ----------- Allocate memory for tree structure -------------- */

  ng->count = NULL;
  ng->count4 = NULL;
  ng->marg_counts = NULL;
  ng->marg_counts4 = NULL;
  ng->count_table = NULL;

  ng->count = (count_ind_t **) rr_malloc(sizeof(count_ind_t *)*ng->n);
  ng->count4 = (count_t **) rr_malloc(sizeof(count_t *)*ng->n);    
  ng->count_table = (count_t **) rr_malloc(sizeof(count_t *)*ng->n);

  if (ng->four_byte_counts) {
    ng->marg_counts4 = (count_t *) rr_calloc(sizeof(count_t), ng->table_sizes[0]);

  }else {
    for (i=0;i<=ng->n-1;i++) 
      ng->count_table[i] = (count_t *) rr_calloc(ng->count_table_size+1,
						sizeof(count_t));

    ng->marg_counts = (count_ind_t *) rr_calloc(sizeof(count_ind_t),ng->table_sizes[0]);
    fprintf(stderr, "table_size %d\n",ng->table_sizes[0]);
    fflush(stderr);
  }

  ng->word_id = (id__t **) rr_malloc(sizeof(id__t *)*ng->n);

  if (ng->four_byte_alphas) {
    ng->bo_weight4 = (four_byte_t **) rr_malloc(sizeof(four_byte_t *)*ng->n);
    ng->bo_weight4[0] = (four_byte_t *) rr_malloc(sizeof(four_byte_t)*
						ng->table_sizes[0]);
  }else {
    ng->bo_weight = (bo_weight_t **) rr_malloc(sizeof(bo_weight_t *)*ng->n);
    ng->bo_weight[0] = (bo_weight_t *) rr_malloc(sizeof(bo_weight_t)*
						ng->table_sizes[0]);
  }

  ng->ind = (index__t **)  rr_malloc(sizeof(index__t *)*ng->n);

  /* First table */
  if (ng->four_byte_counts) 
    ng->count4[0] = (count_t *) rr_calloc(ng->table_sizes[0],sizeof(count_t));
  else 
    ng->count[0] = (count_ind_t *) rr_calloc(ng->table_sizes[0],sizeof(count_ind_t));

  ng->uni_probs = (uni_probs_t *) rr_malloc(sizeof(uni_probs_t)*
					   ng->table_sizes[0]);
  ng->uni_log_probs = (uni_probs_t *) rr_malloc(sizeof(uni_probs_t)*
					       ng->table_sizes[0]);

  if (ng->n >=2) 
    ng->ind[0] = (index__t *) rr_calloc(ng->table_sizes[0],sizeof(index__t));

  for (i=1;i<=ng->n-2;i++) {    
    ng->word_id[i] = (id__t *) rr_malloc(sizeof(id__t)*ng->table_sizes[i]);

    if (ng->four_byte_counts) 
      ng->count4[i] = (count_t *) rr_malloc(sizeof(count_t)*ng->table_sizes[i]);
    else 
      ng->count[i] = (count_ind_t *) rr_malloc(sizeof(count_ind_t)*ng->table_sizes[i]);

    if (ng->four_byte_alphas) 
      ng->bo_weight4[i] = (four_byte_t *) rr_malloc(sizeof(four_byte_t)*ng->table_sizes[i]);
    else 
      ng->bo_weight[i] = (bo_weight_t *) rr_malloc(sizeof(bo_weight_t)*ng->table_sizes[i]);
    
    ng->ind[i] = (index__t *) rr_malloc(sizeof(index__t)*ng->table_sizes[i]);

    mem_alloced = sizeof(count_ind_t) + sizeof(bo_weight_t) + 
		sizeof(index__t) + sizeof(id__t);
    
    if (ng->four_byte_alphas) 
      mem_alloced += 4;
   
    mem_alloced *= ng->table_sizes[i];
    
    pc_message(verbosity,2,"Allocated %d bytes to table for %d-grams.\n",
	       mem_alloced,i+1);
    
  }

  ng->word_id[ng->n-1] = (id__t *) 
    rr_malloc(sizeof(id__t)*ng->table_sizes[ng->n-1]);

  if (ng->four_byte_counts) 
    ng->count4[ng->n-1] = (count_t *) rr_malloc(sizeof(count_t)*ng->table_sizes[ng->n-1]);    
  else 
    ng->count[ng->n-1] = (count_ind_t *) rr_malloc(sizeof(count_ind_t)*ng->table_sizes[ng->n-1]);

  pc_message(verbosity,2,"Allocated (%d+%d) bytes to table for %d-grams.\n",
	     ng->four_byte_counts?sizeof(count_t):sizeof(count_ind_t),
	     sizeof(id__t)*ng->table_sizes[ng->n-1],ng->n);
  
  /* Allocate memory for table for first-byte of indices */

  ng_allocate_ptr_table(ng,NULL,0);

  /* Allocate memory for alpha array */

  ng->alpha_array = (double *) rr_malloc(sizeof(double)*ng->out_of_range_alphas);
  ng->size_of_alpha_array = 0;

  /* Allocate memory for frequency of frequency information */

  ng->freq_of_freq = (fof_t **) rr_malloc(sizeof(fof_t *)*ng->n);

  NG_DISC_METH(ng)->allocate_freq_of_freq(ng);

  /* Read n-grams into the tree */
  pc_message(verbosity,2,"Processing id n-gram file.\n");
  pc_message(verbosity,2,"20,000 n-grams processed for each \".\", 1,000,000 for each line.\n");

  /* Allocate space for ngrams id arrays */

  current_ngram.id_array = (id__t *) rr_calloc(ng->n,sizeof(id__t));
  previous_ngram.id_array = (id__t *) rr_calloc(ng->n,sizeof(id__t));
  current_ngram.n = ng->n;
  previous_ngram.n = ng->n;
  
  ng->num_kgrams = (ngram_sz_t *) rr_calloc(ng->n,sizeof(ngram_sz_t));
  ng_count = (count_t *) rr_calloc(ng->n,sizeof(count_t));
  nlines = 1;
  ng->n_unigrams = 0;

  /* Process first n-gram */  
  get_ngram(ng->id_gram_fp,&current_ngram,is_ascii);
  contains_unks = ngram_chk_contains_unks(&current_ngram,ng->n);

  /* Skip over any unknown words.  They will come first, because <UNK>
     always has a word ID of zero. */
  while (ng->vocab_type == CLOSED_VOCAB && contains_unks){
    /* Stop looking if there are no more N-Grams.  Of course, this
       means training will fail, since there are no unigrams. */
    if (get_ngram(ng->id_gram_fp,&current_ngram,is_ascii) == 0)
      break;
    contains_unks = ngram_chk_contains_unks(&current_ngram,ng->n);
  }

  for (i=0;i<=ng->n-2;i++) {
    ng->ind[i][0] = new_index(0,ng->ptr_table[i],&(ng->ptr_table_size[i]),0);
    ng->word_id[i+1][0] = current_ngram.id_array[i+1];
    ng->num_kgrams[i+1]++;
    ng_count[i] = current_ngram.count;
  }

  ng_count[0] = current_ngram.count;

  NG_DISC_METH(ng)->update_freq_of_freq(ng,ng->n-1,current_ngram.count);

  store_normal_count(ng,0,current_ngram.count,ng->n-1);

  if (current_ngram.count <= ng->cutoffs[ng->n-2]) 
    ng->num_kgrams[ng->n-1]--;

  ngram_copy(&previous_ngram,&current_ngram,ng->n);

  prev_id1 = current_ngram.id_array[0];
    
  displayed_oov_warning = 0;

  while (!rr_feof(ng->id_gram_fp)) {

    if (get_ngram(ng->id_gram_fp,&current_ngram,is_ascii)) {

      if (ng->vocab_type == CLOSED_VOCAB)
	contains_unks=ngram_chk_contains_unks(&current_ngram,ng->n);
    
      if (!contains_unks || ng->vocab_type != CLOSED_VOCAB) {

	/* Test for where this ngram differs from last - do we have an
	   out-of-order ngram? */
	pos_of_novelty = ngram_find_pos_of_novelty(&current_ngram,&previous_ngram,ng->n,nlines);
    
	nlines++; 
	show_idngram_nlines(nlines, verbosity);
    
	/* Add new n-gram as soon as it is encountered */
	/* If all of the positions 2,3,...,n of the n-gram are context
	   cues then ignore the n-gram. */
    
	if (ng->n > 1) {
	  NG_DISC_METH(ng)->update_freq_of_freq(ng,ng->n-1,current_ngram.count);
	        
	  store_normal_count(ng,ng->num_kgrams[ng->n-1],current_ngram.count,ng->n-1);
	  
	  ng->word_id[ng->n-1][ng->num_kgrams[ng->n-1]] = current_ngram.id_array[ng->n-1];
	  ng->num_kgrams[ng->n-1]++;	  
	  
	  if (ng->num_kgrams[ng->n-1] >= ng->table_sizes[ng->n-1])
	    quit(-1,"\nMore than %d %d-grams needed to be stored. Rerun with a higher table size.\n",ng->table_sizes[ng->n-1],ng->n);
	}
	/* Deal with new 2,3,...,(n-1)-grams */
      
	for (i=ng->n-2;i>=MAX(1,pos_of_novelty);i--) {

	  NG_DISC_METH(ng)->update_freq_of_freq(ng,i,ng_count[i]);
	  
	  if (ng_count[i] <= ng->cutoffs[i-1]) 
	    ng->num_kgrams[i]--;
	  else
	    store_normal_count(ng,ng->num_kgrams[i]-1,ng_count[i],i);

	  ng_count[i] = current_ngram.count;
	  ng->word_id[i][ng->num_kgrams[i]] = current_ngram.id_array[i];
	  ng->ind[i][ng->num_kgrams[i]] = new_index(ng->num_kgrams[i+1]-1,
						    ng->ptr_table[i],
						    &(ng->ptr_table_size[i]),
						    ng->num_kgrams[i]);
	  ng->num_kgrams[i]++;
	
	  if (ng->num_kgrams[i] >= ng->table_sizes[i])
	    quit(-1,"More than %d %d-grams needed to be stored. Rerun with a higher table size.\n",ng->table_sizes[i],i+1);	  
	}
      
	for (i=0;i<=pos_of_novelty-1;i++) 
	  ng_count[i] += current_ngram.count;
      
	/* Deal with new 1-grams */
      
	if (pos_of_novelty == 0) {
	  if (ng->n>1) {
	    for (i = prev_id1 + 1; i <= current_ngram.id_array[0]; i++) {
	      ng->ind[0][i] = new_index(ng->num_kgrams[1]-1,
				       ng->ptr_table[0],
				       &(ng->ptr_table_size[0]),
				       i);
	    }
	    prev_id1 = current_ngram.id_array[0];
	  }

	  NG_DISC_METH(ng)->update_freq_of_freq(ng,0,ng_count[0]);

	  if (!ng->context_cue[previous_ngram.id_array[0]]) {
	    ng->n_unigrams += ng_count[0];
	    store_normal_count(ng,previous_ngram.id_array[0],ng_count[0],0);
	  }

	  store_marginal_count(ng,previous_ngram.id_array[0],ng_count[0],0);
		      
	  ng_count[0] = current_ngram.count;
	}

	if (current_ngram.count <= ng->cutoffs[ng->n-2]) 
	  ng->num_kgrams[ng->n-1]--;

	ngram_copy(&previous_ngram,&current_ngram,ng->n);

      }else {
	if (!displayed_oov_warning){
	  pc_message(verbosity,2,"Warning : id n-gram stream contains OOV's (n-grams will be ignored).\n");
	  displayed_oov_warning = 1;
	}
      }
    }
  }

  rr_iclose(ng->id_gram_fp);

  for (i=ng->n-2;i>=1;i--) {

    NG_DISC_METH(ng)->update_freq_of_freq(ng,i,ng_count[i]);

    if (ng_count[i] <= ng->cutoffs[i-1]) 
      ng->num_kgrams[i]--;
    else 
      store_normal_count(ng,ng->num_kgrams[i]-1,ng_count[i],i);
      
  }
  
  NG_DISC_METH(ng)->update_freq_of_freq(ng,0,ng_count[0]);

  if (!ng->context_cue[current_ngram.id_array[0]]) {
    ng->n_unigrams += ng_count[0];
    store_normal_count(ng,current_ngram.id_array[0],ng_count[0],0);
  }

  store_marginal_count(ng,current_ngram.id_array[0],ng_count[0],0);

  if (ng->n>1) {
    for (i=current_ngram.id_array[0]+1;i<=ng->vocab_size;i++)
      ng->ind[0][i] = new_index(ng->num_kgrams[1],
				ng->ptr_table[0],
				&(ng->ptr_table_size[0]),
				current_ngram.id_array[0]);
  }

  /* The idngram reading is completed at this point */
  pc_message(verbosity,2,"\n");

  /* Impose a minimum unigram count, if required */

  if (ng->min_unicount > 0) {

    int nchanged= 0;

    for (i=ng->first_id;i<=ng->vocab_size;i++) {
      if ((return_count(ng->four_byte_counts,
			ng->count_table[0],
			ng->count[0],
			ng->count4[0],
			i) < ng->min_unicount) && !ng->context_cue[i]) {

	/* There was a bug in V2's switch.  Look at segment for ABSOLUTE */
	NG_DISC_METH(ng)->reduce_ug_freq_of_freq(ng,i);
	ng->n_unigrams += (ng->min_unicount - ng->count[0][i]);
	store_normal_count(ng,i,ng->min_unicount,0);
	nchanged++;
      }
    }

    if (nchanged > 0) 
      pc_message(verbosity,2,
		 "Unigram counts of %d words were bumped up to %d.\n",
		 nchanged,ng->min_unicount);
  }

  /* Count zeroton information for unigrams */

  ng->freq_of_freq[0][0] = 0;
  
  for (i=ng->first_id;i<=ng->vocab_size;i++) {
    if (return_count(ng->four_byte_counts,
		     ng->count_table[0],
		     ng->count[0],
		     ng->count4[0],
		     i) == 0) {
      ng->freq_of_freq[0][0]++;
    }
  }  

  if (ng->discounting_method == GOOD_TURING) {
    for (i=0;i<=ng->n-1;i++) 
      for (j=1;j<=ng->fof_size[i];j++) 
	pc_message(verbosity,3,"fof[%d][%d] = %d\n",i,j,ng->freq_of_freq[i][j]);
  }

  pc_message(verbosity,2,"Calculating discounted counts.\n");

  NG_DISC_METH(ng)->compute_discount_aux(ng, verbosity);
     
  /* Smooth unigram distribution, to give some mass to zerotons */     
  compute_unigram(ng,verbosity);

  /* Increment Contexts if using Good-Turing discounting-> No need otherwise,
     since all values are discounted anyway. */

  if (ng->discounting_method == GOOD_TURING) {
    pc_message(verbosity,2,"Incrementing contexts...\n");  

    for (i=ng->n-1;i>=1;i--) 
      increment_context(ng,i,verbosity);      
  }

  /* Calculate back-off weights */

  pc_message(verbosity,2,"Calculating back-off weights...\n");

  for (i=1;i<=ng->n-1;i++) 
    compute_back_off(ng,i,verbosity);

  if (!ng->four_byte_alphas) 
    pc_message(verbosity,3,"Number of out of range alphas = %d\n",
	       ng->size_of_alpha_array);

  /* Write out LM */

  pc_message(verbosity,2,"Writing out language model...\n");

  if (ng->write_arpa)
    write_arpa_lm(ng,verbosity);

  if (ng->write_bin) 
    write_bin_lm(ng,verbosity);

  pc_message(verbosity,0,"idngram2lm : Done.\n");

  return 0;    
}
