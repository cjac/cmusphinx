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


#include "evallm.h"

void display_fof_array(ngram_sz_t *num_kgrams, 
		       fof_t **fof_array,
		       fof_sz_t fof_size,
		       FILE* fp,
		       int n
		       )
{
  int i;
  fof_sz_t j;
  fof_t t;
  i=0;
  t=0;
  j=0;

  for (i=0;i<=n-2;i++) {
    fprintf(fp,"\n%d-grams occurring:\tN times\t\t> N times\tSug. -spec_num value\n",i+2);
    fprintf(fp,"%7d\t\t\t\t\t\t%7lld\t\t%7d\n",0,num_kgrams[i],((int)((double)num_kgrams[i]*1.01))+10);
    t = num_kgrams[i];
    for (j=1;j<=fof_size;j++) {
      t -= fof_array[i][j];
      fprintf(fp,"%7d\t\t\t\t%7d\t\t%7d\t\t%7d\n",j,
	      fof_array[i][j],t,((int)(t*1.01))+10);
    }
  }

}	       

void display_discounting_method(ng_t *ng, FILE* fp)
{
  int i,j;
  switch (ng->discounting_method) {
  case GOOD_TURING:
    fprintf(fp,"Good-Turing discounting was applied.\n");
    for (i=1;i<=ng->n;i++) {
      fprintf(fp,"%d-gram frequency of frequency : ",i);
      for (j=1;j<=ng->fof_size[i-1]-1;j++)
	fprintf(fp,"%d ",ng->freq_of_freq[i-1][j]);
      fprintf(fp,"\n");
    }
    for (i=1;i<=ng->n;i++) {
      fprintf(fp,"%d-gram discounting ratios : ",i);
      for (j=1;j<=ng->disc_range[i-1];j++)
	fprintf(fp,"%.2f ",ng->gt_disc_ratio[i-1][j]);
      fprintf(fp,"\n");
    }
    break;
  case LINEAR:
    fprintf(fp,"Linear discounting was applied.\n");
    for (i=1;i<=ng->n;i++)
      fprintf(fp,"%d-gram discounting ratio : %g\n",i,ng->lin_disc_ratio[i-1]);
    break;
  case ABSOLUTE:
    fprintf(fp,"Absolute discounting was applied.\n");
    for (i=1;i<=ng->n;i++)
      fprintf(fp,"%d-gram discounting constant : %g\n",i,ng->abs_disc_const[i-1]);
    break;
  case WITTEN_BELL:
    fprintf(fp,"Witten Bell discounting was applied.\n");
    break;
  }
}

void display_vocabtype(int vocab_type, double oov_fraction, FILE *fp)
{
  if (vocab_type == CLOSED_VOCAB) {
    fprintf(fp,"This is a CLOSED-vocabulary model\n");
    fprintf(fp,"  (OOVs eliminated from training data and are forbidden in test data)\n");
  }else {
    if (vocab_type == OPEN_VOCAB_1) {
      fprintf(fp,"This is an OPEN-vocabulary model (type 1)\n");
      fprintf(fp,"  (OOVs were mapped to UNK, which is treated as any other vocabulary word)\n");
    }else {
      if (vocab_type == OPEN_VOCAB_2) {
	fprintf(fp,"This is an OPEN-vocabulary model (type 2)\n");

	if(oov_fraction >= 0)
	  fprintf(fp,"  (%.2f of the unigram discount mass was allocated to OOVs)\n",oov_fraction); 

      }
    }
  }

}

void display_stats(ng_t *ng) {
  
  int i;

  fprintf(stderr,"This is a %hu-gram language model, based on a vocabulary of %lld words,\n",ng->n,ng->vocab_size);
  fprintf(stderr,"  which begins \"%s\", \"%s\", \"%s\"...\n",ng->vocab[1],ng->vocab[2],ng->vocab[3]);

  if (ng->no_of_ccs == 1) 
    fprintf(stderr,"There is 1 context cue.");
  else 
    fprintf(stderr,"There are %d context cues.\n",ng->no_of_ccs);

  if (ng->no_of_ccs > 0 && ng->no_of_ccs < 10) {
    if (ng->no_of_ccs == 1) 
      fprintf(stderr,"This is : ");
    else 
      fprintf(stderr,"These are : ");

    for (i=ng->first_id;i<=(int)ng->vocab_size;i++) {
      if (ng->context_cue[i])
	fprintf(stderr,"\"%s\" ",ng->vocab[i]);
    }
    fprintf(stderr,"\n");
  }

  display_vocabtype(ng->vocab_type,ng->oov_fraction,stderr);

  if (ng->four_byte_alphas) 
    fprintf(stderr,"The back-off weights are stored in four bytes.\n");
  else 
    fprintf(stderr,"The back-off weights are stored in two bytes.\n");
  
  for (i=2;i<=ng->n;i++)
    fprintf(stderr,"The %d-gram component was based on %d %d-grams.\n",i,(int)ng->num_kgrams[i-1],i);

  display_discounting_method(ng,stderr);

}


void display_arpa_stats(arpa_lm_t *arpa_ng) {

  int i;

  fprintf(stderr,"This is a %d-gram language model, based on a vocabulary of %d words,\n",arpa_ng->n,(int)arpa_ng->vocab_size);
  fprintf(stderr,"  which begins \"%s\", \"%s\", \"%s\"...\n",
	  arpa_ng->vocab[1],arpa_ng->vocab[2],arpa_ng->vocab[3]);
  
  if (arpa_ng->no_of_ccs == 1) 
    fprintf(stderr,"There is 1 context cue.");
  else 
    fprintf(stderr,"There are %d context cues.\n",arpa_ng->no_of_ccs);

  if (arpa_ng->no_of_ccs > 0 && arpa_ng->no_of_ccs < 10) {
    if (arpa_ng->no_of_ccs == 1)
      fprintf(stderr,"This is : ");
    else
      fprintf(stderr,"These are : ");

    for (i=arpa_ng->first_id;i<=(int)arpa_ng->vocab_size;i++) {
      if (arpa_ng->context_cue[i]) 
	fprintf(stderr,"\"%s\" ",arpa_ng->vocab[i]);
    }
    fprintf(stderr,"\n");
  }

  display_vocabtype(arpa_ng->vocab_type,-1,stderr);

  for (i=2;i<=arpa_ng->n;i++) {
    fprintf(stderr,"The %d-gram component was based on %d %d-grams.\n",i,
	    (int)arpa_ng->num_kgrams[i-1],i);
  }

}
