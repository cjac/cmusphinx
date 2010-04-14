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
  $Log: lm_combine.c,v $
  Revision 1.7  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.6  2006/06/01 05:09:07  archan
  Fixed the formatting of printf

  Revision 1.5  2006/05/31 05:45:33  archan
  Show more messages in lm_combine.

  Revision 1.4  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */
/* history
     12/29/2005 (by David Huggins-Daines): fix memory overrun in increase_pos()
     04/20/2005 (by Ananlada Chotimongkol): fix bug in idscmp() when combine n-grams from 2 LMs
     05/02/2005 (by Ananlada Chotimongkol): fix bug in interpolated backoff weights
     intialize the value to zero first in combine_lm()
     : comment out probability correction in calc_interpolated_prob()
     modified by Moss some time in 2000
*/

#include "../liblmest/toolkit.h"
#include "../liblmest/ngram.h"
#include "../liblmest/evallm.h"
#include "../liblmest/idngram2lm.h"
#include "../libs/pc_general.h"
#include "../libs/sih.h"
#include "../libs/general.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../liblmest/miscella.h"
#include "../libs/ac_parsetext.h"

#define MAX_LINE	1024
#define MAX_WORD	1024
#define MAX_K	20
#define MAX_HEADER 16384

#ifndef MIN_LOG
#define MIN_LOG	-99
#endif /* MIN_LOG */
#define MAX_LOG 99.999

#ifdef VERY_VERBOSE
#define dprintf(x) printf x
#else
#define dprintf(x)
#endif

typedef struct {
	int k;		//browse k-grams
	arpa_lm_t* lm;
	int pos[MAX_K];
	int end[MAX_K];
}	TBROWSE;


typedef struct {
	int k;		//browse k-grams
	arpa_lm_t* lm1;
	arpa_lm_t* lm2;
	TBROWSE br1;
	TBROWSE br2;
	id__t id1[MAX_K];
	id__t id2[MAX_K];
	int over1;
	int over2;
}	TBROWSE_UNION;

double safe_log10(double x) { return log10(x);}

void load_weights(double* w1, double* w2,char* file)
{
  FILE* fp;
  fp=rr_iopen(file);
	
  if (fscanf(fp,"%*s%lf",w1)!=1) 
    quit(-1,"Error in reading weight file\n");
  
  if (fscanf(fp,"%*s%lf",w2)!=1) 
    quit(-1,"Error in reading weight file\n");
	
  rr_iclose(fp);
}

void begin_browse(arpa_lm_t* lm,int k,TBROWSE *browse_st)
{
  int i;
  
  browse_st->lm=lm;
  browse_st->k=k;
  
  /* Find the first i-gram with i+1-grams for each i */
  for (i=0;i<k-1;i++) {
    browse_st->pos[i]=-1;
    do {
      browse_st->pos[i]++;
      /* dhuggins@cs: Unlikely that we'll get to the
	 end of the i-grams, but this is here for
	 correctness' sake. */
      if ( (i==0 && browse_st->pos[i]==lm->vocab_size) || 
	   (browse_st->pos[i]==lm->num_kgrams[i]-1) ) {
	browse_st->end[i]=lm->num_kgrams[i+1]-1;
      }else {
	browse_st->end[i]=get_full_index(lm->ind[i][browse_st->pos[i]+1],
					 lm->ptr_table[i],
					 lm->ptr_table_size[i],
					 browse_st->pos[i]+1)-1;
      }
    } while (browse_st->end[i]==-1);
  }
  /* Start with the first k-gram (i.e. first word in position k) */
  browse_st->pos[k-1]=0;
}


int get_next_word(id__t* id,TBROWSE *br)
{
  if (br->pos[0]<br->lm->table_sizes[0]) {
    id[0]=br->pos[0];
    br->pos[0]++;
    return 1;
  }
  return 0;
}

void increase_pos(int* pos,int* end,int k,arpa_lm_t* lm)
{
	int i;

	/* Go to next word in position k */
	pos[k-1]++;
	dprintf(("increase_pos(%x,%d): pos[%d]=%d table_size=%d\n",
		 (int)(long)lm&0xff, k, k-1, pos[k-1], lm->table_sizes[k-1]));

	/* If we are at the end of the k-grams for position k, then
	   step back through previous positions to find the next
	   k-gram. */
	for (i=k-2; i>=0 && pos[i+1] > end[i]; i--) {
		/* Keep iterating through position i until we find
		 * another one with k-grams */
		do {
			pos[i]++;
			dprintf(("increase_pos(%x,%d): pos[%d]=%d end[%d]=%d table_size=%d\n",
				 (int)(long)lm&0xff, k, i, pos[i], i, end[i],
				 lm->table_sizes[i]));
			/* dhuggins@cs: don't run off the end of the
			 * list of pos[i]! */
			if ((i==0 && pos[i]>=lm->vocab_size) || 
			    (pos[i]>=lm->num_kgrams[i]-1) ) {
				end[i]=lm->num_kgrams[i+1]-1;
				dprintf(("at end of %d-grams pos[%d]=%d\n",
					 i, i, pos[i]));
				dprintf(("setting last end[%d]=%d\n",
					 i, end[i]));
				/* dhuggins@cs: don't run off the end
				 * of the list of pos[i]! */
				break;
			}
			else {
				end[i]=get_full_index(lm->ind[i][pos[i]+1],
					lm->ptr_table[i],
					lm->ptr_table_size[i],
					pos[i]+1)-1;
				dprintf(("setting get_full_index end[%d]=%d\n",
					 i, end[i]));
			}
		} while (pos[i+1] > end[i]);
	}
}

int get_next_ngram(id__t* id,TBROWSE* br)
{
	int i,k;
	
	k=br->k;
	
	/* deal with unigram */
	if (k==1) return get_next_word(id,br);
	
	dprintf(("get_next_ngram(%x,%d): ",
		 (int)(long)br->lm&0xff,k));

	/* deal with k gram */
	/* If we have reached the last word for position k, we are
	 * done. */
	if (br->pos[k-1]>=br->lm->table_sizes[k-1]) {
		dprintf(("no more %d-grams\n", k));
		return 0;
	}

	for (i = 0; i < k; ++i)
		dprintf(("%d ", br->pos[i]));
	/* Get word IDs */
	id[0]=br->pos[0];
	dprintf(("%s ", br->lm->vocab[id[0]]));
	for (i=1;i<k;i++) {
		id[i]=br->lm->word_id[i][br->pos[i]];
		dprintf(("%s ", br->lm->vocab[id[i]]));
	}
	dprintf(("\n"));
	increase_pos(br->pos,br->end,k,br->lm);
	return 1;
}

/* comment out Wei's code, there is a problem when lm1 and lm2 both have <UNK>
this idscmp will always return 0 if the first elements of the ngrams are both <UNK> (id[0]==0)
without checking further words in the ngram
this will cause get_next_ngram_union to take only the ngram from lm1
which then will cause either 1) Error : Same 3-gram occurs twice in ARPA format LM or
2) Error - Repeated 2-gram in ARPA format language model
int idscmp (id__t* id1, id__t* id2, int k, char** voc1, char** voc2)
{
	int i;
	int ret;
	
	for (i=0;i<k;i++) {
		if (id1[i]==0 && id2[i]==0) return 0;
		if (id1[i]==0) return -1;
		if (id2[i]==0) return 1;
		if (ret=strcmp(voc1[id1[i]],voc2[id2[i]])) return ret;
	}
	
	return 0;
}
*/

/*bug fixed version by Moss (April 20, 2005)
  4 cases:
	1) if both are <UNK> always compare
	2) if only lm1 has <UNK> use the ngram from lm1
	3) if only lm2 has <UNK> use the ngram from lm2
	3) for non <UNK> case always compare
*/
int idscmp (id__t* id1, id__t* id2, int k, char** voc1, char** voc2)
{
  int i;
  int ret;
	
  for (i=0;i<k;i++) {
    if (id1[i]==0 && id2[i]==0) {
      if ((ret=strcmp(voc1[id1[i]],voc2[id2[i]]))) return ret;
    }else if (id1[i]==0)
      return -1;
    else if(id2[i]==0) 
      return 1;
    else{
      if ((ret=strcmp(voc1[id1[i]],voc2[id2[i]]))) 
	return ret;
    }
  }
	
  return 0;
}


void ids2words(char** words,id__t* id, int k, char** voc)
{
  int i;	
  for (i=0;i<k;i++) 
    strcpy(words[i],voc[id[i]]);
}

void begin_browse_union(arpa_lm_t *lm1,arpa_lm_t *lm2,int k,TBROWSE_UNION *browse_st)
{
	browse_st->k=k;
	browse_st->lm1=lm1;
	browse_st->lm2=lm2;
	begin_browse(lm1,k,&browse_st->br1);
	begin_browse(lm2,k,&browse_st->br2);
	browse_st->over1=!get_next_ngram(browse_st->id1,&browse_st->br1);
	browse_st->over2=!get_next_ngram(browse_st->id2,&browse_st->br2);
}

int get_next_ngram_union(char** words,TBROWSE_UNION* bru)
{
	int k;
	
	k=bru->k;
	
	if (!bru->over1 && !bru->over2) {
		int cmp = idscmp(bru->id1,bru->id2,k,bru->lm1->vocab,bru->lm2->vocab);
		if (cmp == 0) {
			ids2words (words,bru->id1,k,bru->lm1->vocab);
			bru->over1=!get_next_ngram(bru->id1,&bru->br1);
			bru->over2=!get_next_ngram(bru->id2,&bru->br2);
		}
		else if (cmp < 0) {
			ids2words (words,bru->id1,k,bru->lm1->vocab);
			bru->over1=!get_next_ngram(bru->id1,&bru->br1);
		}
		else if (cmp > 0) {
			ids2words (words,bru->id2,k,bru->lm2->vocab);
			bru->over2=!get_next_ngram(bru->id2,&bru->br2);
		}
		return 1;
	}
	if (!bru->over1) {
		ids2words (words,bru->id1,k,bru->lm1->vocab);
		bru->over1=!get_next_ngram(bru->id1,&bru->br1);
		return 1;
	}
	if (!bru->over2) {
		ids2words (words,bru->id2,k,bru->lm2->vocab);
		bru->over2=!get_next_ngram(bru->id2,&bru->br2);
		return 1;
	}
	return 0;
}



void calc_merged_ngram_num(arpa_lm_t *arpa_lm, arpa_lm_t *lm1,arpa_lm_t *lm2)
{
	TBROWSE_UNION browse_st;
	int k,num;
	char** words;
	
	words=(char**)NewArray(MAX_K,MAX_WORD,sizeof(char));
	
	if (lm1->n != lm2->n) {
		quit(-1,"Error - Two language model have different n: n1=%d, n2=%d",lm1->n,lm2->n);
	}
	
	arpa_lm->n=lm1->n;
	
	for (k=1;k<=arpa_lm->n;k++) {
		
		begin_browse_union(lm1,lm2,k,&browse_st);
		num=0;
		while (get_next_ngram_union(words,&browse_st)) num++;
		arpa_lm->table_sizes[k-1]=num;
	}
	
	DeleteArray(words);
}

void combine_lm(arpa_lm_t *arpa_lm, arpa_lm_t *lm1, arpa_lm_t *lm2)
{
	char *in_line;
	char *input_line;
	int i,j,k;
	int num_of_args;
	int pos_of_novelty;
	char *input_line_ptr_orig;
	char *word_copy;
	id__t *previous_ngram;
	id__t *current_ngram;
	vocab_sz_t temp_id;
	vocab_sz_t *pos_in_list;
	int previd;
	TBROWSE_UNION bru;
	char** words;
	
	words=(char**)NewArray(15,MAX_WORD,sizeof(char));
	
	in_line = (char *) rr_malloc(1024*sizeof(char));
	input_line = (char *) rr_malloc(1024*sizeof(char));
	
	input_line_ptr_orig = input_line;
	
	
	/* Read number of each k-gram */
	
	arpa_lm->table_sizes = (table_size_t *) rr_malloc(sizeof(table_size_t)*11);
	arpa_lm->num_kgrams = (ngram_sz_t *) rr_malloc(sizeof(ngram_sz_t)*11);
		
	calc_merged_ngram_num(arpa_lm, lm1, lm2);
	
	previous_ngram = (id__t *) rr_calloc(arpa_lm->n,sizeof(id__t));
	current_ngram = (id__t *) rr_calloc(arpa_lm->n,sizeof(id__t));

	pos_in_list = (vocab_sz_t *) rr_malloc(sizeof(vocab_sz_t) * arpa_lm->n);
	ng_arpa_lm_alloc_struct(arpa_lm);
	
	/* Process 1-grams */
	
	printf("Reading unigrams...\n");
	
	i=0;
	
	begin_browse_union(lm1,lm2,1,&bru);
	
	while (get_next_ngram_union(words,&bru)) {
	  word_copy = salloc(words[0]);
	  /* Do checks about open or closed vocab */
	  check_open_close_vocab(arpa_lm,word_copy,&i);
	}
	
	/* Process 2, ... , n-1 grams */
	
	previd = -1;
	
	for (i=2;i<=arpa_lm->n-1;i++) {
		
		printf("\nReading %d-grams...\n",i);
		
		previd = -1;
		
		j=0;
		
		for (k=0;k<=arpa_lm->n-1;k++) {
			pos_in_list[k] = 0;
		}
		
		begin_browse_union(lm1,lm2,i,&bru);
		while (get_next_ngram_union(words,&bru)) {
			
			/* Process line into all relevant temp_words */			
			num_of_args = 0;						
			
			sih_lookup(arpa_lm->vocab_ht,words[i-1],&temp_id);
			arpa_lm->word_id[i-1][j] = temp_id;
			
			show_dot(j);
			
			j++;
			if (j>arpa_lm->table_sizes[i-1]) {
				quit(-1,"Error - Header information in ARPA format language model is incorrect.\nMore than %d %d-grams needed to be stored.\n",arpa_lm->table_sizes[i-1],i);
			}
			
			/* Make sure that indexes in previous table point to 
			the right thing. */
			
			for (k=0;k<=i-1;k++) {
				previous_ngram[k] = current_ngram[k];
				sih_lookup(arpa_lm->vocab_ht,words[k],&temp_id);
				if (temp_id == 0 && strcmp(words[k],"<UNK>")) {
					quit(-1,"Error - found unknown word in n-gram file : %s\n",
						words[k]);
				}
				current_ngram[k] = temp_id;
			}
			
			/* Find position of novelty */
			
			/*bug fixed, for the first ngram, pos_of novelty should be 0 - Wei Xu*/
			if (j==1) pos_of_novelty=0;
			else {
				pos_of_novelty = i;
				
				for (k=0;k<=i-1;k++) {
					if (current_ngram[k] > previous_ngram[k]) {
						pos_of_novelty = k;
						k = arpa_lm->n;
					}
					else {
						if ((current_ngram[k] > previous_ngram[k]) && (j > 0)) {
							quit(-1,"Error : n-grams are not correctly ordered.\n");
						}
					}
				}
			}
			
			if (pos_of_novelty == i && j != 1)
			  quit(-1,"Error - Repeated %d-gram in ARPA format language model.\n",
			       i);
			
			if (pos_of_novelty != i-1) {
				if (i==2) {
					/* Deal with unigram pointers */
					
					for (k = previd + 1; k <= current_ngram[0]; k++) {
						arpa_lm->ind[0][k] = new_index(j-1,
							arpa_lm->ptr_table[0],
							&(arpa_lm->ptr_table_size[0]),
							k);
					}
					previd = current_ngram[0];
				}else {
					
					for (k=pos_of_novelty;k<=i-2;k++) {
						if (k == 0) {
							pos_in_list[0] = current_ngram[0];
						}
						else {
							pos_in_list[k] = 
								MIN(get_full_index(arpa_lm->ind[k-1][pos_in_list[k-1]],
								arpa_lm->ptr_table[k-1],   
								arpa_lm->ptr_table_size[k-1],   
								pos_in_list[k-1]),pos_in_list[k]);
							while (arpa_lm->word_id[k][pos_in_list[k]] < 
								current_ngram[k]) {
								pos_in_list[k]++;
							}
						}
					}
					for (k = previd + 1; k <= pos_in_list[i-2]; k++) {
						arpa_lm->ind[i-2][k] = 
							new_index(j-1,
							arpa_lm->ptr_table[i-2],
							&(arpa_lm->ptr_table_size[i-2]),
							k);
					}
					previd = pos_in_list[i-2];	    
				}
			}
		}
	
		/* Now need to tidy up pointers for bottom section of unigrams */
	
		for (k = previd + 1; k <= arpa_lm->vocab_size; k++) {
			arpa_lm->ind[0][k] = new_index(arpa_lm->num_kgrams[1],
				arpa_lm->ptr_table[0],
				&(arpa_lm->ptr_table_size[0]),
				k);
		}      
	
	}
  
	printf("\nReading %d-grams...\n",arpa_lm->n);
	
	j = 0;
	previd = 0;
	
	arpa_lm->ind[arpa_lm->n-2][0] = 0;
	
	for (k=0;k<=arpa_lm->n-1;k++) {
		/* bug fixed by Wei Xu : this is a serious bug*/
		pos_in_list[k] = 0;
		//    pos_in_list[0] = 0;
	}
	
	begin_browse_union(lm1,lm2,arpa_lm->n,&bru);
	while (get_next_ngram_union(words,&bru)) {

	  show_dot(j);
	  
		sih_lookup(arpa_lm->vocab_ht,words[arpa_lm->n-1],&temp_id);
		
		arpa_lm->word_id[arpa_lm->n-1][j] = temp_id;
		
		j++;
		
		for (k=0;k<=arpa_lm->n-1;k++) {
			previous_ngram[k] = current_ngram[k];
			sih_lookup(arpa_lm->vocab_ht,words[k],&temp_id);
			if (temp_id == 0 && strcmp(words[k],"<UNK>")) {
				quit(-1,"Error - found unknown word in n-gram file : %s\n",
					words[k]);
			}
			current_ngram[k] = temp_id;
		}
		
		/* Find position of novelty */
		
		/*bug fixed, for the first ngram, pos_of novelty should be 0 - Wei Xu*/
		if (j==1) pos_of_novelty=0;
		else {
			pos_of_novelty = arpa_lm->n+1;
			
			for (k=0;k<=arpa_lm->n-1;k++) {
				if (current_ngram[k] > previous_ngram[k]) {
					pos_of_novelty = k;
					k = arpa_lm->n;
				}else {
					if ((current_ngram[k] > previous_ngram[k]) && (j>0)) {
						quit(-1,"Error : n-grams are not correctly ordered.\n");
					}
				}
			}
		}
		
		if ( pos_of_novelty == arpa_lm->n+1 && j != 1 ) {
			quit(-1,"Error : Same %d-gram occurs twice in ARPA format LM.\n",
				arpa_lm->n);
		}
		
		if (pos_of_novelty != arpa_lm->n-1) {
			
			for (k=pos_of_novelty;k<=arpa_lm->n-2;k++) {
				if (k == 0) {
					pos_in_list[0] = current_ngram[0];
				}else {
					pos_in_list[k] = 
						MAX(get_full_index(arpa_lm->ind[k-1][pos_in_list[k-1]],
						arpa_lm->ptr_table[k-1],   
						arpa_lm->ptr_table_size[k-1],   
						pos_in_list[k-1]),pos_in_list[k]);
					while (arpa_lm->word_id[k][pos_in_list[k]] < 
						current_ngram[k]) {
						pos_in_list[k]++;
					}
				}
			}
			for (k = previd + 1; k <= pos_in_list[arpa_lm->n-2]; k++) {
				arpa_lm->ind[arpa_lm->n-2][k] = 
					new_index(j-1,
					arpa_lm->ptr_table[arpa_lm->n-2],
					&(arpa_lm->ptr_table_size[arpa_lm->n-2]),
					k);
			}
			previd = pos_in_list[arpa_lm->n-2];
		}
		
		if (j>arpa_lm->table_sizes[arpa_lm->n-1]) {
			quit(-1,"Error - Header information in ARPA format language model is incorrect.\nMore than %d %d-grams needed to be stored.\n",arpa_lm->table_sizes[arpa_lm->n-1],arpa_lm->n-1);
		}
	}
	
	
	
	/* Tidy up */
	
	
	free(previous_ngram);
	free(current_ngram);
	free(in_line);
	free(input_line);
	DeleteArray(words);
  
}

 
double calc_prob(char** words,int k,arpa_lm_t* lm,fb_info* fb_list)
{
  id__t id[MAX_K];
  int bo_case,acl;
  vocab_sz_t index;
  int i;
  
  for (i=0;i<k;i++) {
    sih_lookup(lm->vocab_ht,words[i],&index);
    id[i]=index;
  }
  
  return calc_prob_of(id[k-1],id,k-1,NULL,lm,fb_list,&bo_case,&acl,TRUE);
}
void calc_interpolated_prob(arpa_lm_t *lm, arpa_lm_t *lm1, arpa_lm_t *lm2,fb_info* fb1,fb_info* fb2,double w1,double w2)
{
	TBROWSE br;
	id__t id[MAX_K];
	char** words;
	int k,j;
	double p1,p2,p;

	words=(char**)NewArray(MAX_K,MAX_WORD,sizeof(char));

	for (k=1;k<=lm->n;k++) {
		begin_browse(lm,k,&br);
		j=0;
		printf("\nProcessing %d-gram\n",k);
		while (get_next_ngram(id,&br)) {
		  j++;
		  show_dot(j);
			ids2words(words,id,k,lm->vocab);
			p1=calc_prob(words,k,lm1,fb1);
			p2=calc_prob(words,k,lm2,fb2);
			p=w1*p1+w2*p2;
			lm->probs[k-1][br.pos[k-1]-1]=log10(p);
			/* comment out this probability correction
			if(lm->probs[k-1][br.pos[k-1]-1]==0.0){
				lm->probs[k-1][br.pos[k-1]-1]=-0.00001;
			}
			*/

		}
	}

	DeleteArray(words);
}

void check_prob(arpa_lm_t *lm, arpa_lm_t *lm1, arpa_lm_t *lm2,fb_info* fb1,fb_info* fb2,double w1,double w2)
{
  TBROWSE br;
  id__t id[MAX_K];
  char** words;
  int j,k;
  double p1,p2,p,s1,s2;
  int bo_pos = 0; /* for a quiet compile */
  
  words=(char**)NewArray(MAX_K,MAX_WORD,sizeof(char));

  for (k=1;k<=lm->n;k++) {
    begin_browse(lm,k,&br);
    if (k>=2) bo_pos=br.pos[k-2];
    s1=s2=0;
    j=0;

    printf("\nProcessing %d-gram\n",k);
    while (get_next_ngram(id,&br)) {
      ids2words(words,id,k,lm->vocab);
      p1=calc_prob(words,k,lm1,fb1);
      p2=calc_prob(words,k,lm2,fb2);

      j++;
      show_dot(j);
      s1+=p1;
      s2+=p2;
      if ((k>=2 && br.pos[k-2]!=bo_pos) || (k==1 && br.pos[0]==lm->num_kgrams[0])) {
	s1=s2=0;
      }
      p=w1*p1+w2*p2;
      lm->probs[k-1][br.pos[k-1]-1]=log10(p);
    }
  }

  DeleteArray(words);
}

void calc_backoff_weight(arpa_lm_t* lm,fb_info* fb_list)
{
	int k;
	TBROWSE br;
	id__t id[MAX_K];
	int bo_case,acl;
	double p,q;
	int bo_pos;

	char** words;
	words=(char**)NewArray(MAX_K,MAX_WORD,sizeof(char));

	lm->bo_weight[0][0]=0.0;		//for <UNK>

	for (k=2;k<=lm->n;k++) {
		begin_browse(lm,k,&br);
		p=q=0;
		bo_pos=br.pos[k-2];
		while (get_next_ngram(id,&br)) {
			ids2words(words,id,k,lm->vocab);
			p+=calc_prob_of(id[k-1],id,k-1,NULL,lm,fb_list,&bo_case,&acl,TRUE);
			q+=calc_prob_of(id[k-1],&id[1],k-2,NULL,lm,fb_list,&bo_case,&acl,TRUE);
			if (br.pos[k-2]!=bo_pos) {
				if (p>=1) {
					lm->bo_weight[k-2][bo_pos]=MIN_LOG;
				}else if (q>=1) {
					/* printf ("Warning: sum of low order prob is 1"); */
					lm->bo_weight[k-2][bo_pos]=MAX_LOG;
				}else {
					lm->bo_weight[k-2][bo_pos]=safe_log10((1-p)/(1-q));
				}
				p=q=0;
				bo_pos=br.pos[k-2];
			}
		}
	}
}

void load_context_cue(arpa_lm_t* lm, char* ccs_filename)
{
  FILE* context_cues_fp;
  char wlist_entry[1024];
  char current_cc[200];
  vocab_sz_t current_cc_id;

  lm->context_cue = (flag *) rr_calloc(lm->table_sizes[0],sizeof(flag));    
  lm->no_of_ccs = 0;
  if (strcmp(ccs_filename,"")) {
    context_cues_fp = rr_iopen(ccs_filename);
    while (fgets (wlist_entry, sizeof (wlist_entry),context_cues_fp)) {
      if (strncmp(wlist_entry,"##",2)==0) continue;
      sscanf (wlist_entry, "%s ",current_cc);
      warn_on_wrong_vocab_comments(wlist_entry);
	
      if (sih_lookup(lm->vocab_ht,current_cc,&current_cc_id) == 0)
	quit(-1,"Error : %s in the context cues file does not appear in the vocabulary.\n",current_cc);
      
      lm->context_cue[(unsigned short) current_cc_id] = 1;
      lm->no_of_ccs++;
      fprintf(stderr,"Context cue word : %s id = %lld\n",current_cc,current_cc_id);
    }
    rr_iclose(context_cues_fp);
  }
}

/**
   Very similar to write_lms except that it use arpa_lm_t but not
   lm_t.  Headers of the two files are also written. 
 */
void write_interpolated_lm(arpa_lm_t *ng, const char* arpa_filename, const char* header1, const char* header2, int verbosity) 
{		
  int i;
  int j;
  FILE* fp;
  TBROWSE br;
  id__t id[MAX_K];
  char** words;
  
  words=(char**)NewArray(MAX_K,MAX_WORD,sizeof(char));
  if (words==NULL) {
    Error ("Cannot allocate memory");
    return;
  }
  
  if ((fp=fopen(arpa_filename,"w"))==NULL) {
    Error ("Cannot open file to write arpa lm file.");
    return ;
  }
	
  /* HEADER */
  
  pc_message(verbosity,1,"ARPA-style %d-gram will be written to %s\n",ng->n,arpa_filename);
  
  write_arpa_copyright(fp,ng->n,(int) ng->vocab_size,ng->vocab[1],ng->vocab[2],ng->vocab[3]);
  write_arpa_format(fp,ng->n);	
  write_arpa_headers(fp, header1, header2);
  write_arpa_num_grams(fp,NULL,ng,1);
 
  /* Print 1-gram, ... n-gram info. */
  
  for (i=0;i<=ng->n-1;i++) {    
    /* Print out the (i+1)-gram */		
    write_arpa_k_gram_header(fp,i+1);
    
    begin_browse(ng,i+1,&br);
    
    /* Go through the n-gram list in order */
    
    while (get_next_ngram(id,&br)) {
      fprintf(fp,"%.4f ",ng->probs[i][br.pos[i]-1]);
      for (j=0;j<=i;j++)
	fprintf(fp,"%s ",ng->vocab[id[j]]);

      if (i <= ng->n-2)
	fprintf(fp,"\t%.4f\n",ng->bo_weight[i][br.pos[i]-1]);
      else
	fprintf(fp,"\n");
    }
  }	
	
  fprintf(fp,"\n\\end\\\n");

  fclose(fp);
	
  DeleteArray(words);
} 

void recalc_oov_prob(arpa_lm_t* lm,arpa_lm_t* lm1,arpa_lm_t* lm2)
{
  lm1->probs[0][0]=lm1->probs[0][0]-log10(lm->vocab_size-lm1->vocab_size+1);
  lm2->probs[0][0]=lm2->probs[0][0]-log10(lm->vocab_size-lm2->vocab_size+1);
}

void help_message()
{
  fprintf(stderr,"lm_combine : Combine two language models.\n");
  fprintf(stderr,"Usage : lm_combine -lm1 lmfile1\n");
  fprintf(stderr,"                   -lm2 lmfile2\n");
  fprintf(stderr,"                   -weight .wt\n");
  fprintf(stderr,"                   -lm newlmfile\n");
  fprintf(stderr,"                   [ -context .ccs ]\n");
  fprintf(stderr,"                   [ -forced_backoff .fblist]\n");
  fprintf(stderr,"                   [ -backoff_from_unk_inc | -backoff_from_unk_exc ]\n");
  fprintf(stderr,"                   [ -backoff_from_ccs_inc | -backoff_from_ccs_exc ]\n");
  exit(1);

}

int main(int argc,char** argv)
{
	arpa_lm_t arpa_lm,lm1,lm2;
	char header1[MAX_HEADER];
	char header2[MAX_HEADER];
	flag backoff_from_unk_inc,backoff_from_unk_exc,backoff_from_ccs_inc,backoff_from_ccs_exc;
	char *lmfile1,*lmfile2,*newlmfile, *wtfile;
	char *fb_list_filename,*ccs_filename;
	fb_info *fb1,*fb2,*fb;
	double w1,w2;
	
	if (pc_flagarg(&argc, argv,"-help") || argc == 1) {
	  help_message();
	  exit(1);
	}
	
	lmfile1 = salloc(pc_stringarg(&argc, argv,"-lm1",""));
	if (0 == strcmp(lmfile1, "")) {
		fprintf(stderr, "ERROR: Please specify a first input file with -lm1.\n");
		help_message();
	}
	lmfile2 = salloc(pc_stringarg(&argc, argv,"-lm2",""));
	if (0 == strcmp(lmfile2, "")) {
		fprintf(stderr, "ERROR: Please specify a second input file with -lm2.\n");
		help_message();
	}
	newlmfile = salloc(pc_stringarg(&argc, argv,"-lm",""));
	if (0 == strcmp(newlmfile, "")) {
		fprintf(stderr, "ERROR: Please specify a destination file with -lm.\n");
		help_message();
	}
	fb_list_filename = salloc(pc_stringarg(&argc, argv,"-forced_backoff",""));
	wtfile= salloc(pc_stringarg(&argc, argv,"-weight",""));
	if (0 == strcmp(wtfile, "")) {
		fprintf(stderr, "ERROR: Please specify a weights file with -weight.\n");
		help_message();
	}
	ccs_filename= salloc(pc_stringarg(&argc, argv,"-context",""));

	backoff_from_unk_inc = pc_flagarg(&argc,argv,"-backoff_from_unk_inc");
	backoff_from_ccs_inc = pc_flagarg(&argc,argv,"-backoff_from_ccs_inc");
	backoff_from_unk_exc = pc_flagarg(&argc,argv,"-backoff_from_unk_exc");
	backoff_from_ccs_exc = pc_flagarg(&argc,argv,"-backoff_from_ccs_exc");
  
	robust_load_arpa_lm(&lm1,lmfile1,header1,MAX_HEADER);
	robust_load_arpa_lm(&lm2,lmfile2,header2,MAX_HEADER);
	
	load_weights(&w1,&w2,wtfile);

	printf ("\ncombine lms\n");
	combine_lm(&arpa_lm,&lm1,&lm2);

	printf ("\nloading context cues.\n");
	load_context_cue(&arpa_lm,ccs_filename);
	load_context_cue(&lm1,ccs_filename);
	load_context_cue(&lm2,ccs_filename);

	fb=gen_fb_list(arpa_lm.vocab_ht,
		arpa_lm.vocab_size,
		arpa_lm.vocab,
		arpa_lm.context_cue,
		backoff_from_unk_inc,
		backoff_from_unk_exc,
		backoff_from_ccs_inc,
		backoff_from_ccs_exc,
		fb_list_filename);

	fb1=gen_fb_list(lm1.vocab_ht,
		lm1.vocab_size,
		lm1.vocab,
		lm1.context_cue,
		backoff_from_unk_inc,
		backoff_from_unk_exc,
		backoff_from_ccs_inc,
		backoff_from_ccs_exc,
		fb_list_filename);

	fb2=gen_fb_list(lm2.vocab_ht,
		lm2.vocab_size,
		lm2.vocab,
		lm2.context_cue,
		backoff_from_unk_inc,
		backoff_from_unk_exc,
		backoff_from_ccs_inc,
		backoff_from_ccs_exc,
		fb_list_filename);
	
	printf ("\nrecaculate oov probabilities.\n");
	recalc_oov_prob(&arpa_lm,&lm1,&lm2);

	printf ("\ncheck probabilities\n");
	check_prob(&arpa_lm,&lm1,&lm2,fb1,fb2,w1,w2);

	printf ("\ncalculate interpolated probabilities\n");
	calc_interpolated_prob(&arpa_lm,&lm1,&lm2,fb1,fb2,w1,w2);

	printf ("\ncalculate backoff weights\n");
	calc_backoff_weight(&arpa_lm,fb);

	printf ("\nwrite interpolated lm\n");
	write_interpolated_lm(&arpa_lm,newlmfile,header1,header2,2);

	printf ("\nfinished\n");

	return 0;
}
