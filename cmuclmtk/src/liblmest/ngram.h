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


/* Type and function definitions for general n_gram models */

#ifndef _NGRAM_H_
#define _NGRAM_H_

#include "sih.h"
#include "general.h"

#include "toolkit.h"

#define DEFAULT_OOV_FRACTION 0.5
#define DEFAULT_DISC_RANGE_1 1
#define DEFAULT_DISC_RANGE_REST 7
#define DEFAULT_MIN_ALPHA -3.2
#define DEFAULT_MAX_ALPHA 2.5
#define DEFAULT_OUT_OF_RANGE_ALPHAS 10000

#define GOOD_TURING 1
#define ABSOLUTE 2
#define LINEAR 3
#define WITTEN_BELL 4

#define SPECIFIED 1
#define BUFFER 2
#define TWO_PASSES 3

#define CLOSED_VOCAB 0
#define OPEN_VOCAB_1 1
#define OPEN_VOCAB_2 2

/** This defines the size of id__t and wordid_t is defined in
    general.h.  In most of the case, we expect changing the type of
    wordid_t.  Of course, that depends on how disciplined of the
    programmers
*/
typedef wordid_t id__t; /* Double underscore, since id_t is
			   already defined on some platforms */
typedef wordid_t index__t; 

typedef int count_t;   /* The count as read in, rather than its index 
			  in the count table. */

#if 0

typedef int count_ind_t; /* The count's index in the count 
				       table. */

#endif


typedef unsigned short count_ind_t; /* The count's index in the count 
				       table. */

/*
#endif
*/


typedef unsigned short bo_weight_t;
typedef float four_byte_t;

typedef unsigned short cutoff_t;
typedef int table_size_t;
typedef double disc_val_t;
typedef double uni_probs_t;
typedef int ptr_tab_t;
typedef unsigned short ptr_tab_sz_t;
typedef int fof_t;

typedef struct {
  unsigned short n;
  id__t          *id_array;
  count_t        count;
} ngram;

typedef struct {
  unsigned short count_table_size;
  int            *counts_array;
} count_table_t;

typedef struct {

  /* Language model type */
  unsigned short n;                /**< n=3 for trigram, n=4 for 4-gram etc. */
  int            version;

  /* Vocabulary stuff */
  sih_t          *vocab_ht;      /**< Vocabulary hash table */
  vocab_sz_t      vocab_size;     /**< Vocabulary size */
  char           **vocab;        /**< Array of vocabulary words */
  unsigned short no_of_ccs;      /**< Number of context cues */

  /* Tree */
  table_size_t   *table_sizes;   /**< Pointer to table size array */
  id__t          **word_id;      /**< Pointer to array of id lists */
  count_ind_t    **count;        /**< Pointer to array of count lists 
				    (actually indices in a count table) */
  count_ind_t    *marg_counts;   /**< Array of marginal counts for the 
				    unigrams. The normal unigram counts
				    differ in that context cues have
				    zero counts there, but not here */
  count_t         **count4;       /**< Alternative method of storing the counts,
				    using 4 bytes. Not normally allocated */
  count_t         *marg_counts4;  /**< Ditto */
  bo_weight_t    **bo_weight;    /**< Pointer to array of back-off weights */
  four_byte_t    **bo_weight4;   /**< Pointer to array of 4 byte
				    back_off weights. Only one of
				    these arrays will be allocated */
  index__t       **ind;          /**< Pointer to array of index lists */  

  /* Two-byte alpha stuff */
  double         min_alpha;      /**< The minimum alpha in the table */
  double         max_alpha;      /**< The maximum alpha in the table */
  unsigned short out_of_range_alphas;  /**< The maximum number of out of range 
					  alphas that we are going to allow. */
  double         *alpha_array;
  unsigned short size_of_alpha_array;

  /* Count table */
  count_ind_t    count_table_size; /**< Have same size for each count table */
  count_t        **count_table;    /**< Pointer to array of count tables */

  /* Index lookup tables */

  ptr_tab_t      **ptr_table;     /**< Pointer to the tables used for compact 
				       representation of the indices */
  ptr_tab_sz_t    *ptr_table_size; /**< Pointer to array of pointer tables */

  /* Discounting and cutoffs - note: some of these may not used,
     depending on the discounting techinque used. */

  unsigned short discounting_method;     /**< See #define stuff at the top of 
					    this file */
  flag disc_range_set;  /**< Discouting range */

  cutoff_t       *cutoffs;               /**< Array of cutoffs */
  fof_t           **freq_of_freq;         /**< Array of frequency of frequency 
					    information  */
  fof_sz_t     *fof_size;              /**< The sizes of the above arrays */
  unsigned short *disc_range;            /**< Pointer to array of discounting 
					    ranges - typically will be 
					    fof_size - 1, but can be reduced
					    further if stats are anomolous */
  disc_val_t     **gt_disc_ratio;        /**< The discounted values of the 
					    counts */
  disc_val_t     *lin_disc_ratio;        /**< The linear discounting ratio */
  double         *abs_disc_const;        /**< The constant required for
					    absolute discounting */

  /* Unigram statistics */

  uni_probs_t    *uni_probs;             /**< Probs for each unigram */
  uni_probs_t    *uni_log_probs;         /**< Log probs for each unigram */
  flag           *context_cue;           /**< True if word with this id is
					    a context cue */
  ngram_sz_t     n_unigrams;             /**< Total number of unigrams in
					    the training data */
  count_t        min_unicount;           /**< Count to which infrequent unigrams
					    will be bumped up */
  ngram_sz_t     *num_kgrams;     /**< Array indicating how many 
				     2-grams, ... ,n-grams, have been 
				     processed so far */

  /* Input files */

  char           *id_gram_filename;  /**< The filename of the id-gram file */
  FILE           *id_gram_fp;        /**< The file pointer of the id-gram file */
  char           *vocab_filename;    /**< The filename of the vocabulary file */
  char           *context_cues_filename; /**< The filename of the context cues 
					    file */
  flag           context_set;            /**< Whether a context is set */
  FILE           *context_cues_fp;       /**< The file pointer of the context 
					    cues file */

  /* Output files */

  flag           write_arpa;      /**< True if the language model is to be 
				     written out in arpa format */
  char           *arpa_filename;  /**< The filaname of the arpa format LM */
  FILE           *arpa_fp;        /**< The file of the arpa format LM */
  flag           write_bin;       /**< True if the language model is to be 
				     written out in binary format */
  char           *bin_filename;   /**< The filaname of the bin format LM */
  FILE           *bin_fp;         /**< The file of the bin format LM */


  /* Misc */

  unsigned short vocab_type;      /**< see #define stuff at the top */

  unsigned short first_id;        /**< 0 if we have open vocab, 1 if we have
				     a closed vocab. */

  /* Once the tree has been constructed, the tables are indexed from 0
     to (num_kgrams[i]-1). */

  /* 1-gram tables are indexed from 0 to ng.vocab_size. */

  double         zeroton_fraction; /**< cap on prob(zeroton) as fraction of 
				      P(singleton) */
  double         oov_fraction;
  flag           four_byte_alphas;
  flag           four_byte_counts;

  void*    disc_meth;  /**< The discounting method object */

} ng_t;

/* Type specification for forced back-off list */

typedef struct {
  flag backed_off;
  flag inclusive;
} fb_info;

typedef float bo_t;
typedef float prob_t;

/* Type specification for arpa_lm type */

typedef struct {
  double ug_weight;
  double bg_weight;
  double tg_weight;
} weight_t;

/**
   \type arpa_lm_t 
   A severely duplicated data structure. 
 */
typedef struct {

  unsigned short n;                /* n=3 for trigram, n=4 for 4-gram etc. */

  /* Vocabulary stuff */

  sih_t          *vocab_ht;      /**< Vocabulary hash table */
  vocab_sz_t     vocab_size;     /**< Vocabulary size */
  char           **vocab;        /**< Array of vocabulary words */
  flag           *context_cue;   /**< True if word with this id is
				    a context cue */
  int            no_of_ccs;      /**< The number of context cues in the LM */

  /**< Tree */

  table_size_t   *table_sizes;   /**< Pointer to table size array */
  id__t          **word_id;      /**< Pointer to array of id lists */
  bo_t           **bo_weight;    /**< Pointer to array of back-off weights */
  prob_t         **probs;        /**< Pointer to array of probabilities */
  index__t       **ind;          /**< Pointer to array of index lists */

  /* Index lookup tables */

  ptr_tab_t      **ptr_table;     /**< Pointer to the tables used for compact 
				     representation of the indices */
  ptr_tab_sz_t   *ptr_table_size; /**< Pointer to array of pointer tables */

  /* Misc */
  ngram_sz_t     *num_kgrams;     /**< Array indicating how many 
				     2-grams, ... ,n-grams, have been 
				     processed so far */
  unsigned short vocab_type;      /**< see #define stuff at the top */
  unsigned short first_id;        /**< 0 if we have open vocab, 1 if we have
				     a closed vocab. */

  int		 **bucket_no;	  /**< Pointer to array of interpolatiion bucket no*/
  int            num_buckets;
  weight_t       *weights;
} arpa_lm_t;

/**
   Write the copyright section of ARPA format
 */

void write_arpa_copyright(FILE *fp, 
			  int N, 
			  wordid_t vocab_size, 
			  char* firstwd, 
			  char* secondwd, 
			  char* thirdwd
			  );
/**
   Write the format section of the ARPA format
 */

void write_arpa_format(FILE *fp,  /**< file stream pointer  */
		       int n
		       );


/**
   Write the two headers of the ARPA format in case 
 */
void write_arpa_headers(FILE *fp,  /**< file stream pointer */
			const char* header1,  /**< header 1 */
			const char * header2  /**< header 2 */
			);
/**
   Write the num of grams 
 */
void write_arpa_num_grams(FILE* fp, /**< file stream pointer */
			  ng_t *ng,  /**< ngram with discounting structure */
			  arpa_lm_t *arpa_ng, /**< arpa_ngram*/
			  flag is_arpa   /**< Is in arpa mode */
			  );

/**
   Initialize the discounting method 
 */
void init_ng_disc_method(ng_t* ng,    /**< ngram */   
			 flag linear, /**< Is it linear discounting? */
			 flag absolute, /**< Is it abolute discounting? */
			 flag witten_bell, /**< Is it Witten Bell? */
			 flag good_turing  /**< Is it Good Turing? */
			 );

/**
   Reallocate the pointer table 
 */
void ng_allocate_ptr_table(ng_t * ng, /**< ng_t with binary format stuffs */
			   arpa_lm_t *arpa_ng,  /** arpa_lm_t with only ng stuffs */
			   flag is_arpa
			   );

void ng_allocate_vocab_ht(ng_t *ng, /**< ng_t  with binary format stuffs */
			  arpa_lm_t *arpa_ng,  /**< arpa_lm_t */
			  flag is_arpa
			  );

flag ngram_chk_contains_unks(ngram *gm, int N);
void ngram_copy(ngram *tgt, ngram *src,int N);

/**
   return back-off weight for an N-gram
 */
double ng_double_alpha(ng_t *ng, /**< ng */
		       int N,    /**< The N in N-gram */
		       int i     /**< index in ng */
		       );

/*
  set back-off weight for an N-gram
 */
void ng_short_alpha(ng_t *ng, /**< ng */
		    double alpha, /**< The value used */
		    int N,     /**< The N in N-gram */
		    int i      /**< The N-gram id */
		    );

/**
   An attempt to tie the allocation routine together, it only works
   for arpa_lm. So the problem is still there are two damn data structures
 */
void ng_arpa_lm_alloc_struct(
			     arpa_lm_t *arpa_ng /**<arpa_lm_t */
			     );

#endif


