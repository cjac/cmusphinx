/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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

/** \file lm_ng.c
	\brief Darpa N-gram LM module (adapted to Sphinx 3)
*/

/*
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: lm_ng.c,v $
 * 2008/10/20  N. Coetmeur, supervised by Y. Esteve
 * When reading a LM file without trigrams, read it as a trigram LM
 *
 * 2008/08/26  N. Coetmeur, supervised by Y. Esteve
 * In function ReadNgrams: set back-off weight at 0.0 if not written in
 * language model
 *
 * 2008/06/27  N. Coetmeur, supervised by Y. Esteve
 * Adjust comments for compatibility with Doxygen 1.5.6
 *
 * 2008/06/17  N. Coetmeur, supervised by Y. Esteve
 * Adapt LM functions for working with N-grams for each N value
 * (i.e for a trigram file N=3, for a quadrigram file N=4 and so on...).
 * - ReadUnigrams, ReadBigrams and ReadTrigrams became ReadNgrams and work for
 *  any N value (given in parameter)
 * - lm_write_arpa_unigram, lm_write_arpa_bigram and lm_write_arpa_trigram
 *  became lm_write_arpa_Ngram and work for any N value (given in parameter)
 * - Macros TSEG_BASE(m,  b), FIRST_TG(m,  b), FIRST_TG32(m,  b) are
 *  renamed NSEG_BASE(m,n,b), FIRST_NG(m,n,b), FIRST_NG32(m,n,b) and
 *  work for the given N value (N>=3)
 * - The part of txtheader table string describing probabilities is dynamically
 *  written in the lm_write_arpa_header function depending on the max N value in
 *  the LM file
 * - New functions:
 *   > lm_write_arpa_Ngram_rec is the recursive part of the lm_write_arpa_Ngram
 *    function for trail the N levels of the LM file
 *   > SearchNgram: return a N-grams (N>=3) with the given word indexes
 *
 * Revision 1.3  2006/03/01 20:05:09  arthchan2003
 * Pretty format the LM dumping and make numbers in 4 decimals only.
 *
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert LM to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.4  2006/01/16 19:58:25  arthchan2003
 * Small change to make function public.
 *
 * Revision 1.1.2.3  2005/11/17 06:21:05  arthchan2003
 * 1, Change all lm_write_arpa_* functions' s3wid_t to s3lmwid_t.
 * 2, Make the writing code to be changed more easily.
 *
 * Revision 1.1.2.2  2005/07/28 20:04:20  dhdfu
 * Make LM writing not crash for bigram LM
 *
 * Revision 1.1.2.1  2005/07/13 01:39:55  arthchan2003
 * Added lm_3g.c, which take cares of reading and writing of LM into the lm_t
 * (3-gram specific LM structure.)
 *
 */

#include "lm.h"
#include "bio.h"
#include "logs3.h"
#include "wid.h"


/** \name Quantization values */
/*\{*/
#define QUANTIZATION_MULTIPLIER 10000
#define QUANTIZATION_DIVISOR 0.0001
/*\}*/

/**
	\brief Maximum values in a sorted list
 
   20060321 ARCHAN: Why MAX_SORTED_ENTRIES = 200000?

   Generally, MAX_SORTED_ENTRIES relates the quantization schemes of
   the log probabilities values and back-off weights.  In the
   Sphinx/CMUCamLMtk case.  This is usually represented as
   -x.xxxx. That is to say maximally 100000 could be involved.  It is
   also possible that value such as -99.0 and other special values is
   involved.   Therefore, doubling the size is reasonable measure. 

   When we use better precision, say 5 decimal places, we should
   revise the validity of the above schemes. 
 */
#define MAX_SORTED_ENTRIES 200000


/** \name Macros */
/*\{*/
#define FIRST_BG(m,u)     ((m)->ug[u].firstbg)	/**< Return the first bigram
												 corresponding to the word index
												 'u' in language model 'm' */
#define FIRST_NG(m,n,b)   (NSEG_BASE((m),(n),(b))+((m)->ng[n-2][b].firstnng))
					 /**< Return the first 'n'-gram corresponding to ('n'-1)gram
									  index 'b' in language model 'm', 'n'>=3 */
#define FIRST_NG32(m,n,b) (NSEG_BASE((m),(n),(b))+((m)->ng32[n-2][b].firstnng))
				 /**< Return the first 32-bits 'n'-gram corresponding to 32-bits
						  ('n'-1)gram index 'b' in language model 'm', 'n'>=3 */
#define NSEG_BASE(m,n,b)  ((m)->ng_segbase[n-1][(b)>>((m)->log_ng_seg_sz[n-2])])
				  /**< Return the first 'n'-gram corresponding to the segment of
					  the ('n'-1)gram index 'b' in language model 'm', 'n'>=3 */
/*\}*/


/** \brief Header written in Arpa text files */
static char const *txtheader[] = {
 "############################################################################",
 "# Copyright (c) 1999-2004 Carnegie Mellon University. All rights reserved. #",
 "############################################################################",
 "#==========================================================================#",
 "#=============  This file was produced by the CMU Sphinx 3.X  =============#",
 "#==========================================================================#",
 "############################################################################",
 "This file is in the ARPA-standard format introduced by Doug Paul.",
 NULL
};


/**
  \brief Search the word ID corresponding to a word string
	  \return Word ID
*/
static int32
wstr2wid	(lm_t *	model,	/**< In: language model */
			 char *	w		/**< In: pre-allocated word string */
			 )
{
    void *val;

    if (hash_table_lookup(model->HT, w, &val) != 0)
        return NO_WORD;
    return ((int32)(long)val);
}

/**
  \brief Initialize sorted list
	  
  Initialize with the 0-th entry = MIN_PROB_F, which may be needed
  to replace spurious values in the Darpa LM file.
*/
static void
init_sorted_list	(sorted_list_t * l	/**< In/Out: a sorted list */
					 )
{
    l->list =
        (sorted_entry_t *) ckd_calloc(MAX_SORTED_ENTRIES,
                                      sizeof(sorted_entry_t));
    l->list[0].val.f = MIN_PROB_F;
    l->list[0].lower = 0;
    l->list[0].higher = 0;
    l->free = 1;
}

/**
  \brief Free a sorted list 
*/
static void
free_sorted_list	(sorted_list_t * l	/**< In/Out: a sorted list */
					 )
{
    free(l->list);
}

/**
  \brief Get the values in a sorted list
	  \return Table of values
*/
static lmlog_t *
vals_in_sorted_list	(sorted_list_t * l /**< In: a sorted list */
					 )
{
    lmlog_t *vals;
    int32 i;

    vals = (lmlog_t *) ckd_calloc(l->free, sizeof(lmlog_t));
    for (i = 0; i < l->free; i++)
        vals[i].f = l->list[i].val.f;
    return (vals);
}

/**
  \brief Store a value in a sorted list
	  \return Index of inserted value
*/
static int32
sorted_id	(sorted_list_t 	*	l,	/**< In: a sorted list */
			 float 			*	val	/**< In: value to insert */
			 )
{
    int32 i = 0;

    for (;;) {
        if (*val == l->list[i].val.f)
            return (i);
        if (*val < l->list[i].val.f) {
            if (l->list[i].lower == 0) {

                if (l->free >= MAX_SORTED_ENTRIES)
                    E_INFO("sorted list overflow\n");

                l->list[i].lower = l->free;
                (l->free)++;
                i = l->list[i].lower;
                l->list[i].val.f = *val;
                return (i);
            }
            else
                i = l->list[i].lower;
        }
        else {
            if (l->list[i].higher == 0) {

                if (l->free >= MAX_SORTED_ENTRIES)
                    E_INFO("sorted list overflow\n");

                l->list[i].higher = l->free;
                (l->free)++;
                i = l->list[i].higher;
                l->list[i].val.f = *val;
                return (i);
            }
            else
                i = l->list[i].higher;
        }
    }
}

/**
  \brief Read and return number of unigrams...N-grams as stated in input file
 
  @warning The internal buffer has size 256. 
 
  @return LM_NO_DATA_MARK if there is "\\data\\" mark. LM_UNKNOWN_NG if an
  unknown K of K-gram appears.  LM_BAD_LM_COUNT if a bad LM counts. LM_SUCCESS
  if the whole reading succeeds. 
*/
static int
ReadNgramCounts	(FILE 	* fp,		/**< In: file stream */
				 int32 ** n_ng, 	/**< Out: table of N-grams read */
				 uint32 * max_ng	/**< Out: N-gram level */
				 )
{
    char string[256];
    int32 ngram=0, ngram_cnt=0;
	uint32 N;
	void *ptr;

	
    /* skip file until past the '\data\' marker */

	do {
        fgets(string, sizeof(string), fp);
    } while ((strcmp(string, "\\data\\\n") != 0) && (!feof(fp)));

    if (strcmp(string, "\\data\\\n") != 0) {

        E_WARN("No \\data\\ mark in LM file\n");
        return LM_NO_DATA_MARK;
    }


	/* allocate memory for trigrams count */
	(*n_ng) = (int32 *) ckd_calloc(3, sizeof(int32));
	(*max_ng) = 3;

	/* read N-grams count */
	while (fgets(string, sizeof(string), fp) != NULL) {
		if (sscanf(string, "ngram %d=%d", &ngram, &ngram_cnt) != 2)
			break;
		
		if (ngram == ((*max_ng)+1))
		{ /* next N-gram level : reallocate n_ng */
			ptr = ckd_realloc(*n_ng, ngram * sizeof(int32));
			if (ptr != NULL) {
				(*n_ng) = (int32 *) ptr;
				(*max_ng) = ngram;
			}
		}

		if ((ngram > 0) && (ngram <= (*max_ng)) && ((*n_ng) != NULL))
			(*n_ng)[ngram-1] = ngram_cnt;

		if ((ngram <= 0) && (ngram > (*max_ng))) {
			E_WARN("Unknown N-gram (%d)\n", ngram);
			return LM_UNKNOWN_NG;
		}
	}

	/* Position file to just after the unigrams header '\1-grams:\' */
	while ((strcmp(string, "\\1-grams:\n") != 0) && (!feof(fp)))
		fgets(string, sizeof(string), fp);

	
    /* Check counts;  NOTE: number of (N>=3)grams *CAN* be 0 */
	
	if (    ((*max_ng) < 2)   || ((*n_ng) == NULL)
		 || ((*n_ng)[0] <= 0) || ((*n_ng)[1] <= 0)) {
        E_WARN("Bad or missing N-gram count\n");
		return LM_BAD_LM_COUNT;
    }

	for (N = 3; N <= (*max_ng); N++)
		if ((*n_ng)[N-1] < 0) {
			E_WARN("Bad or missing N-gram count\n");
			return LM_BAD_LM_COUNT;
		}

	return LM_SUCCESS;
}

/**
   \brief Allocate a new unigram table.

   Initially all dictionary words are defined as NO_WORD,
   the probabilities and back-off weights are -99. 
   
   \return a pointer of unigram if succeed, NULL if failed
*/
ug_t *
NewUnigramTable	(int32 n_ug	/**< In: number of unigrams */
				 )
{
    ug_t *table;
    int32 i;
    table = NULL;

    table = (ug_t *) ckd_calloc(n_ug, sizeof(ug_t));
    if (table == NULL) {
        E_WARN("Fail to allocate the unigram table\n");
        return NULL;
    }
    for (i = 0; i < n_ug; i++) {
        table[i].dictwid = NO_WORD;
        table[i].prob.f = -99.0;
        table[i].bowt.f = -99.0;
    }
    return table;
}

/**
   \brief Allocate a new model

   That includes the tables of unigram, bigram and more. 

	\note The n_ng table in input must be allocate with a a ckd_alloc function
	and not free after the call of this function.

	\return New allocated LM
*/
static lm_t *
NewModel	(uint32		max_ng,		/**< In: maximum N-gram level */
			 int32 *	n_ng,		/**< In: allocated table of N-grams numbers
															 for each N level */
			 int32 		lminmemory,	/**< In: is language model must be in memory
																	   or not */
			 int32 		version		/**< In: version number */
			 )
{
    lm_t *model;
	uint32 N;


	/* Allocate memory for LM */

	if ((model = (lm_t *) ckd_calloc(1, sizeof(lm_t))) == NULL)
		return NULL;

	lm_null_struct(model);

	model->n_ng = n_ng;

    if (lm_init_n_ng (model, max_ng, n_ng[0], version, lminmemory) == LM_FAIL) {
        E_ERROR("Error in allocating memory for LM with max Ng level\n");
        lm_free(model);
        return NULL;
    }


	/*
     * Allocate one extra N-gram entry: sentinels to terminate
     * followers ((N+1)grams) of previous entry.
     */

	model->ug = NewUnigramTable(n_ng[0] + 1);

	if (model->is32bits) {
		for (N = 2; N <= max_ng; N++)
			if (n_ng[N-1] > 0)
				model->ng32[N-1]
					= (ng32_t *) ckd_calloc(n_ng[N-1] + 1, sizeof(ng32_t));
	}
	else {
		for (N = 2; N <= max_ng; N++)
			if (n_ng[N-1] > 0)
				model->ng[N-1]
					= (ng_t *) ckd_calloc(n_ng[N-1] + 1, sizeof(ng_t));
	}


	for (N = 3; N <= max_ng; N++)
		if (n_ng[N-1] > 0) {
			model->ng_segbase[N-1] =
				(int32 *) ckd_calloc((n_ng[N-2] + 1) / NG_SEG_SZ + 1,
									 sizeof(int32));
			
			E_INFO("%8d = %dseg_base entries allocated\n",
				   (n_ng[N-2] + 1) / NG_SEG_SZ + 1, N);
			
		}
		
    model->max_ug = n_ng[0];
	
    model->HT = hash_table_new(model->n_ng[0], HASH_CASE_YES);
	/* Default value for log_ng_seg_sz */
	for (N = 2; N <= max_ng; N++)
		model->log_ng_seg_sz[N-1] = LOG2_NG_SEG_SZ;

    return model;
}

/**
  \brief Search for a N-grams (N >= 2)
 
  Written for being called by ReadNgrams function.
 
  @return LM_NO_MINUS_1GRAM if the corresponding N-gram couldn't be found.
  LM_SUCCESS when the N-gram is found. 
*/
static int
SearchNgram (lm_t *	model,	/**< In: LM model */
			 uint32 Ne,		/**< In: (Ne)gram level to found */
			 uint32 Ns,		/**< In: (Ns)gram level to start search
															(Ns<=Ne && Ns>=2) */
			 char *	string,	/**< In: The complete (Ne+1)gram line
								  (with the Ne first words) from an Arpa file */
			 int32 	w	[], /**< In: Ne index of words */
			 int32 	ng	[]	/**< In/Out: (Ne)gram offset, ng[Ns-1] is the
							 input value indicating the point to start search */
			 )
{
	int32 seg, endng;
	uint32 i;
	ng_t *ngptr;
	ng32_t *ngptr32;

	for (i = Ns; i <= Ne; i++) {
		if (model->is32bits) {
			ngptr32 = model->ng32[i-1] + ng[i-1];
			
			if (i == 2)
				endng = model->ug[w[0] + 1].firstbg;
			else
				endng = (ng[i-2]+1 < model->n_ng[i-2]) ?
					(model->ng32[i-2][ng[i-2]+1].firstnng
					 + model->ng_segbase[i-1][ (ng[i-2]+1) >> LOG2_NG_SEG_SZ])
					: model->n_ng[i-1];

			for (; (ng[i-1] < endng) && (ngptr32->wid != w[i-1])
				 ; ng[i-1]++, ngptr32++);

			/* prepare the next loop (if any) */
			if (i < Ne) {
				seg = ng[i-1] >> LOG2_NG_SEG_SZ;
				ng[i] = ngptr32->firstnng + model->ng_segbase[i][seg];
			}
		}
		else {
			ngptr = model->ng[i-1] + ng[i-1];
			
			if (i == 2)
				endng = model->ug[w[0] + 1].firstbg;
			else
				endng = (ng[i-2]+1 < model->n_ng[i-2]) ?
					(model->ng[i-2][ng[i-2]+1].firstnng
					 + model->ng_segbase[i-1][ (ng[i-2]+1) >> LOG2_NG_SEG_SZ])
					: model->n_ng[i-1];

			for (; (ng[i-1] < endng) && (ngptr->wid != w[i-1])
				 ; ng[i-1]++, ngptr++);

			/* prepare the next loop (if any) */
			if (i < Ne) {
				seg = ng[i-1] >> LOG2_NG_SEG_SZ;
				ng[i] = ngptr->firstnng + model->ng_segbase[i][seg];
			}
		}

		if (ng[i-1] >= endng) {
			E_WARN("Missing %d-gram for %d-gram: %s", i, Ne+1, string);
			return LM_NO_MINUS_1GRAM;
		}

		/* Here first (w1...wi, *...*) is found */
	}

	/* Here first (w1...wNe, *) is found */
    return LM_SUCCESS;
}

/**
  \brief Read N-grams from given file into given LM model structure

  On entry to this procedure, the file pointer is positioned just after
  the line "\N-grams:". For bigrams or more (N>=2), the File may be
  arpabo or arpabo-id format, depending on idfmt = 0 or 1.

  @return LM_UNKNOWN_WORDS when an unknown word couldn't be found.
  LM_BAD_NGRAM when a bad quadrigram is found. LM_NO_MINUS_1GRAM if the
  corresponding (N-1)gram of a N-gram couldn't be found. LM_OFFSET_TOO_LARGE
  when in 16 bit mode, the N-gram count of a segment is too huge.
  LM_BAD_NGRAM when a bad N-gram is found. LM_FAIL on memory allocation
  failure. LM_SUCCESS when the whole reading is ok. 
*/
static int
ReadNgrams	(FILE *	fp,		/**< In: file stream */
			 lm_t *	model,	/**< In/Out: language model */
			 uint32	N,		/**< In: N-gram level */
			 int32	idfmt	/**< In: is N-grams represented in ID format */
			 )
{
    char string[1024];
	char word[256];
	int32 *w = NULL, *prev_w = NULL, *prev_ng = NULL, *ng = NULL;
    ng_t *nmgptr, *Ngptr = NULL;
    ng32_t *nmgptr32, *Ngptr32 = NULL;
    float pn, bo_wt;
	int val_ret = LM_SUCCESS;

	int32 p, ngoff, is32bits = model->is32bits;
    int32 seg = -1, prev_seg_lastnmg, prev_seg = -1;
	uint32 i, j, l, n, test, ngcount = 0;
    s3lmwid32_t startwid, endwid;


	/* verify parameters */
	if ( (N <= 0) || (model == NULL) )
		return LM_FAIL;


	/* allocate memory */

	if (N >= 2) {
		/* for bigrams or more */
		if ((w = (int32 *) ckd_calloc(N, sizeof(int32))) == NULL) {
			val_ret = LM_FAIL;
			goto end_ReadNgrams;
		}
		if ((prev_w = (int32 *) ckd_calloc(N-1, sizeof(int32))) == NULL) {
			val_ret = LM_FAIL;
			goto end_ReadNgrams;
		}

		for (i = 1; i <= (N-1); i++) {
			if (prev_w != NULL)
				prev_w[i-1] = -1;

			if (prev_ng != NULL)
				prev_ng[i-1] = -1;
		}

		if (N >= 3) {
			/* for trigrams or more */
			if ((ng = (int32 *) ckd_calloc(N-1, sizeof(int32))) == NULL) {
				val_ret = LM_FAIL;
				goto end_ReadNgrams;
			}
			if ((prev_ng = (int32 *) ckd_calloc(N-1, sizeof(int32))) == NULL) {
				val_ret = LM_FAIL;
				goto end_ReadNgrams;
			}

			ng[1] = -1 ;
		}

		if (is32bits)
			Ngptr32 = model->ng32[N-1];
		else
			Ngptr = model->ng[N-1];
	}

	word[0] = '\0';

	
    E_INFO("Reading %d-grams\n", N);

	/* get the words and the numbers */
    while (fgets(string, sizeof(string), fp) != NULL) {
		/* get the probability value */
		sscanf(string, "%s ", word);
		n = sscanf(word, "%f", &pn);
		l = strlen(word);

		/* get the words */
		for (i = 1; (i <= N) && (n == i); i++) {
			if (sscanf(string + l, " %s", word) == 1) {
				l += (strlen(word)+1);

				if (N >= 2) {
					/* for bigrams or more */
					if (idfmt) {
						n += sscanf(word, "%d", &(w[i-1]));

						if ((w[i-1] >= model->n_ng[0]) || (w[i-1] < 0)) {
							E_WARN("Bad %d-gram: %s\n", N, string);
							val_ret = LM_BAD_NGRAM;
							goto end_ReadNgrams;
						}
					}
					else {
						n++;

						if ((w[i-1] = wstr2wid(model, word)) == NO_WORD) {
							E_WARN("Unknown word: %s\n", word);
							val_ret = LM_UNKNOWN_WORDS;
							goto end_ReadNgrams;
						}
					}
				}
				else
					n++;
			}
		}

		/* get the back-off weight value (if any) */
		if ( (n == (N+1)) && (model->max_ng > N) && (model->n_ng[N] > 0) )
			if (sscanf(string + l, " %f", &bo_wt)!=1)
				bo_wt = 0.0;

		/* verify the numbers of items */
		if (n != (N+1)) {
			/* end of N-grams or error */
			if (string[0] != '\n') {
				sprintf(word, "\\%d-grams:\n", N+1);
				if (   (strcmp(string, "\\end\\\n") != 0)
					&& (strcmp(string, word)        != 0) ) {
					if (N >= 2) {
						/* bigrams or more */
						E_WARN("Bad %d-gram: %s\n", N, string);
						val_ret = LM_BAD_NGRAM;
						goto end_ReadNgrams;
					}
					else { /* N == 1 */
						/* unigrams */
						E_WARN("Format error; unigram ignored:%s", string);
						continue;
					}
				}
				else
					/* end of N-grams part */
					break;
			}
            continue;
		}

		/* verify number of N-grams */
        if (ngcount >= model->n_ng[N-1]) {
            E_WARN("Too many %d-grams\n", N);
            val_ret = LM_TOO_MANY_NGRAM;
			goto end_ReadNgrams;
        }

		/* save values */
		if (N == 1) {
			/* for unigrams */

			/* Associate name with word id */
			/* This is again not local */
			model->wordstr[ngcount] = (char *) ckd_salloc(word);
			hash_table_enter (model->HT, model->wordstr[ngcount],
							  (void *)(long)ngcount);
			model->ug[ngcount].dictwid = ngcount;

			model->ug[ngcount].prob.f = pn;

			if ((model->max_ng > N) && (model->n_ng[N] > 0))
				model->ug[ngcount].bowt.f = bo_wt;
		}
		else { /* N >= 2 */
			/* for bigrams or more */

			/* HACK!! to quantize probabilities to 4 decimal digits */
			p = pn * QUANTIZATION_MULTIPLIER;
			pn = p * QUANTIZATION_DIVISOR;
			p = bo_wt * QUANTIZATION_MULTIPLIER;
			bo_wt = p * QUANTIZATION_DIVISOR;

			if (is32bits) {
				Ngptr32->wid = w[N-1];
				Ngptr32->probid = sorted_id(&model->sorted_probn[N-1], &pn);

				if ((model->max_ng > N) && (model->n_ng[N] > 0))
					Ngptr32->bowtid
						= sorted_id(&(model->sorted_bowtn[N-1]), &bo_wt);
			}
			else {
				Ngptr->wid = w[N-1];
				Ngptr->probid = sorted_id(&model->sorted_probn[N-1], &pn);

				if ((model->max_ng > N) && (model->n_ng[N] > 0))
					Ngptr->bowtid
						= sorted_id(&(model->sorted_bowtn[N-1]), &bo_wt);
			}

			test = 0;
			for (i = 1; (i <= (N-1)) && (!test); i++)
				if (w[i-1] != prev_w[i-1])
					test = TRUE;
			if (test) {
				/* N-gram for a new (N-1)gram;
				  update Ng info for all previous (N-1)grams */

				test = FALSE;
				for (i = 1; (i <= (N-1)) && (!test); i++) {
					if (w[i-1] < prev_w[i-1]) {
						test = TRUE;
						for (j = 1; (j < i) && test; j++)
							if (w[j-1] != prev_w[j-1])
								test = FALSE;
					}
				}
				/* for a quadrigram (N=4), the previous test is equivalent to :
				if (                                          (w1 < prev_w1)
					|| (                   (w1 == prev_w1) && (w2 < prev_w2))
					|| ((w1 == prev_w1) && (w2 == prev_w2) && (w3 < prev_w3)))*/
				if (test)
					E_INFO("%d-grams not in %d-grams order\n", N, N-1);

				if (N >= 3) {
					/* for trigrams or more */

					/* Looking for (N-1)grams pointing to the current N-gram */
					if (w[0] != prev_w[0]) { /* At least w1 has changed */
						ng[1] =  model->ug[w[0]].firstbg; 

						if ( SearchNgram (model, N-1, 2, string, w, ng)
								== LM_NO_MINUS_1GRAM) {
							val_ret = LM_NO_MINUS_1GRAM;
							goto end_ReadNgrams;
						}

						/* Here first (w1,w2,w3, *) is found */
					}
					else {
						for (i=2; (i < (N-1)) && (w[i-1] == prev_w[i-1]); i++);

						ng[i-1] = prev_ng[i-1]+1;

						if ( SearchNgram (model, N-1, i, string, w, ng)
								== LM_NO_MINUS_1GRAM ) {
							val_ret = LM_NO_MINUS_1GRAM;
							goto end_ReadNgrams;
						}

						/* Here first (w1...w(N-1), *) is found */
					}

					/* ng[N-2] = trigram entry index for <w1...w(N-1)>
					  Update nseg_base[N-1] */
					seg = ng[N-2] >> LOG2_NG_SEG_SZ;
					for (i = prev_seg + 1; i <= seg; i++)
						model->ng_segbase[N-1][i] = ngcount;

					/* Update N-grams pointers
					  for all (N-1)grams until ng[N-2] */
					if (prev_seg < seg) {
						ngoff = 0;

						if (prev_seg >= 0) {
							ngoff = ngcount - model->ng_segbase[N-1][prev_seg];

							if (!is32bits) {
								if (ngoff > LM_LEGACY_CONSTANT) {
									E_WARN (
									 "Offset %d %dgcount %d, seg_base %d from %dseg_base > %d\n"
											, ngoff, N, ngcount
											, model->ng_segbase[N-1][prev_seg]
											, N, LM_LEGACY_CONSTANT);
									val_ret = LM_OFFSET_TOO_LARGE;
									goto end_ReadNgrams;
								}
							}
						}

						prev_seg_lastnmg
							= ((prev_seg + 1) << LOG2_NG_SEG_SZ) - 1;

						if (is32bits) {
							nmgptr32 = model->ng32[N-2] + prev_ng[N-2];
							for (++(prev_ng[N-2]), ++nmgptr32;
								 prev_ng[N-2] <= prev_seg_lastnmg;
								 prev_ng[N-2]++, nmgptr32++)
								nmgptr32->firstnng = ngoff;
							for (; prev_ng[N-2] <= ng[N-2];
								 prev_ng[N-2]++, nmgptr32++)
								nmgptr32->firstnng = 0;
						}
						else {
							nmgptr = model->ng[N-2] + prev_ng[N-2];
							for (++(prev_ng[N-2]), ++nmgptr;
								 prev_ng[N-2] <= prev_seg_lastnmg;
								 prev_ng[N-2]++, nmgptr++)
								nmgptr->firstnng = ngoff;
							for (; prev_ng[N-2] <= ng[N-2];
								 prev_ng[N-2]++, nmgptr++)
								nmgptr->firstnng = 0;
						}

					}
					else {
						ngoff = ngcount - model->ng_segbase[N-1][prev_seg];

						if (!is32bits) {
							if (ngoff > LM_LEGACY_CONSTANT) {
								E_WARN (
								 "Offset %d %dgcount %d, seg_base %d from %dseg_base > %d\n"
										, ngoff, N, ngcount
										, model->ng_segbase[N-1][prev_seg]
										, N, LM_LEGACY_CONSTANT);
								val_ret = LM_OFFSET_TOO_LARGE;
								goto end_ReadNgrams;
							}
						}

						if (is32bits) {
							nmgptr32 = model->ng32[N-2] + prev_ng[N-2];
							for (++(prev_ng[N-2]), ++nmgptr32;
								 prev_ng[N-2] <= ng[N-2];
								 prev_ng[N-2]++, nmgptr32++)
								nmgptr32->firstnng = ngoff;
						}
						else {
							nmgptr = model->ng[N-2] + prev_ng[N-2];
							for (++(prev_ng[N-2]), ++nmgptr;
								 prev_ng[N-2] <= ng[N-2];
								 prev_ng[N-2]++, nmgptr++)
								nmgptr->firstnng = ngoff;
						}
					}
				}
				else /* N == 2 */
					/* for bigrams */
					for (prev_w[0]++; prev_w[0] <= w[0]; prev_w[0]++)
						model->ug[prev_w[0]].firstbg = ngcount;

				prev_w[0] = w[0];
				for (i = 2; i <= (N-1); i++) {
					prev_w[i-1] = w[i-1];
					prev_ng[i-1] = ng[i-1];
				}
				prev_seg = seg;
			}
		}

        ngcount++;

        if (is32bits)
            Ngptr32++;
        else
            Ngptr++;

        if ((ngcount & 0x0000ffff) == 0) {
            printf(".");
			fflush(stdout);
        }
    }
    if (ngcount >= 0x0000ffff)
		printf("\n");
	
	if (N >= 3) {
		/* for trigrams or more */
		for (prev_ng[N-2]++; prev_ng[N-2] <= model->n_ng[N-2]; prev_ng[N-2]++) {
			if ((prev_ng[N-2] & (NG_SEG_SZ - 1)) == 0)
				model->ng_segbase[N-1][prev_ng[N-2] >> LOG2_NG_SEG_SZ]
					= ngcount;

			if (!is32bits) {
				if ((ngcount - model->ng_segbase[N-1][prev_ng[N-2]
									>> LOG2_NG_SEG_SZ]) > LM_LEGACY_CONSTANT) {
					E_WARN (
					 "Offset %d %dgcount %d, seg_base %d from %dseg_base > %d\n"
							, model->ng_segbase[N-1][prev_seg >> LOG2_NG_SEG_SZ]
							, N, ngcount
							, model->ng_segbase[N-1][prev_seg >> LOG2_NG_SEG_SZ]
							, N, LM_LEGACY_CONSTANT);
					val_ret = LM_OFFSET_TOO_LARGE;
					goto end_ReadNgrams;
				}
			}

			if (is32bits) {
				model->ng32[N-2][prev_ng[N-2]].firstnng = ngcount
					- model->ng_segbase[N-1][prev_ng[N-2] >> LOG2_NG_SEG_SZ];
			}
			else {
				model->ng[N-2][prev_ng[N-2]].firstnng = ngcount
					- model->ng_segbase[N-1][prev_ng[N-2] >> LOG2_NG_SEG_SZ];
			}
		}
	}
	else {
		if (N == 2) {
			/* for bigrams */
			for (prev_w[0]++; prev_w[0] <= model->n_ng[0]; prev_w[0]++)
				model->ug[prev_w[0]].firstbg = ngcount;
		}
		else { /* N == 1 */
			/* for unigrams */

			if (model->n_ng[0] != ngcount) {
				E_WARN ("lm_t.n_ng[0](%d) != number of unigrams read(%d)\n",
						model->n_ng[0], ngcount);
				model->n_ng[0] = ngcount;
			}

			startwid = endwid = BAD_LMWID(model);

			for (i = 0; i < model->n_ng[0]; i++) {
				if (strcmp(model->wordstr[i], S3_START_WORD) == 0)
					startwid = i;
				else if (strcmp(model->wordstr[i], S3_FINISH_WORD) == 0)
					endwid = i;
			}

			/* Force prob(<s>) = MIN_PROB_F */
			if (IS_LMWID(model, startwid)) {
				model->ug[startwid].prob.f = MIN_PROB_F;
				model->startlwid = startwid;
			}

			/* Force bowt(</s>) = MIN_PROB_F */
			if (IS_LMWID(model, endwid)) {
				model->ug[endwid].bowt.f = MIN_PROB_F;
				model->finishlwid = endwid;
			}
		}
	}

end_ReadNgrams:
	/* free allocated memory */
	if (w != NULL)
		ckd_free(w);
	if (prev_w != NULL)
		ckd_free(prev_w);
	if (ng != NULL)
		ckd_free(ng);
	if (prev_ng != NULL)
		ckd_free(prev_ng);

	return val_ret;
}

/**
   \brief Read an Arpa-based LM file

   (200060708) At this point, it is not memory leak-free. 
   
   \return An initialized LM if everything is alright,
			NULL if something goes wrong
*/
lm_t *
lm_read_txt	(const char *	filename,       /**< In: file name*/
             int32 			lminmemory,		/**< In: whether LM is in memory */
             int32 		* 	err_no,   		/**< Out: Depends on the problem
											that LM reading encounters, it could
											be errors from -2
											(LM_OFFSET_TOO_LARGE) to -15
											(LM_CANNOT_ALLOCATE) */
             int32 			isforced32bit	/**< In: force LM to be in 32-bits
											or let the function to decide
											whether the file is in 32-bits */
			 )
{
    lm_t *model;
    FILE *fp = NULL;
    int32 usingPipe = FALSE;
    int32 *n_ngram = NULL;
    int32 idfmt = 0;
    int32 _errmsg;
	uint32 i, max_ng;
	

	E_INFO("Reading LM file %s\n", filename);

    fp = fopen_comp(filename, "r", &usingPipe);
    if (fp == NULL) {
		E_WARN("failed to read filename for LM\n");
        *err_no = LM_FILE_NOT_FOUND;
		return NULL;
    }

    _errmsg = ReadNgramCounts(fp, &n_ngram, &max_ng);
    if (_errmsg != LM_SUCCESS) {
        E_WARN("Couldnt' read the N-gram count\n");
        *err_no = _errmsg;
		
		if (n_ngram != NULL)
			ckd_free (n_ngram);
		
        return NULL;
    }

	for (i = 1; i <= max_ng; i++)
		E_INFO("%d-grams: %d\n", i, n_ngram[i-1]);
    /* HACK! This should be something provided
	  by the dictionary. What is dict_size? */

    model = NewModel(max_ng, n_ngram, lminmemory, isforced32bit ?
                     LMFORCED_TXT32VERSION : LMTXT_VERSION);
    if (model == NULL) {
        E_WARN("Cannot allocate tables for new LM\n");
        *err_no = LM_CANNOT_ALLOCATE;

		if (n_ngram != NULL)
			ckd_free (n_ngram);
		
        return NULL;
    }

    model->max_ng = 1;

    if (model->is32bits)
        E_INFO ("Is 32 bits %d, lm->version %d\n", model->is32bits,
                model->version);

    /* ARCHAN. Should do checking as well. I was lazy.
     */

    if (model->n_ng[1] > 0) {
        model->max_ng = 2;
        if (model->is32bits) {
            model->membg32 =
                (membg32_t *) ckd_calloc(model->n_ng[0], sizeof(membg32_t));

        }
        else {
            model->membg =
                (membg_t *) ckd_calloc(model->n_ng[0], sizeof(membg_t));

        }
    }

	for (i = 3; i <= max_ng; i++)
		if (model->n_ng[i-1] > 0) {
			model->max_ng = i;

			if (model->is32bits) {
				model->nginfo32[i-1] = (nginfo32_t **)
					ckd_calloc(model->n_ng[0], sizeof(nginfo32_t *));

			}
			else {
				model->nginfo[i-1] = (nginfo_t **)
					ckd_calloc(model->n_ng[0], sizeof(nginfo_t *));
			}
		}


    /* Have to put it somewhere in LM as a kind of buffer */
    model->wordstr = (char **) ckd_calloc(model->n_ng[0], sizeof(char *));

    /* control the LM dumping mechanism */

    _errmsg = ReadNgrams(fp, model, 1, idfmt);
    if (_errmsg != LM_SUCCESS) {
        *err_no = _errmsg;

		lm_free (model);

        return NULL;
    }

    E_INFO("%8d = 1-grams created\n", model->n_ng[0]);

    init_sorted_list(&(model->sorted_probn[1]));
    if ((max_ng >= 3) && (model->n_ng[2] > 0))
        init_sorted_list(&(model->sorted_bowtn[1]));

    _errmsg = ReadNgrams(fp, model, 2, idfmt);
    if (_errmsg != LM_SUCCESS) {
        *err_no = _errmsg;

		lm_free (model);

        return NULL;
    }

    model->n_ng[1] = FIRST_BG(model, model->n_ng[0]);
    model->n_ngprob[1] = model->sorted_probn[1].free;
    model->ngprob[1] = vals_in_sorted_list(&(model->sorted_probn[1]));
    free_sorted_list(&(model->sorted_probn[1]));

    E_INFO("%8d = 2-grams created\n", model->n_ng[1]);
    E_INFO("%8d = prob2 entries\n", model->n_ngprob[1]);

	for (i = 3; i <= max_ng; i++)
		if (model->n_ng[i-1] > 0) {
			/* Create i-gram back-off weights array */
			model->n_ngbowt[i-1] = model->sorted_bowtn[i-2].free;
			model->ngbowt[i-1] =
				vals_in_sorted_list(&(model->sorted_bowtn[i-2]));
			free_sorted_list(&(model->sorted_bowtn[i-2]));
			E_INFO("%8d = bowt%d entries\n", model->n_ngbowt[i-1], i-1);

			init_sorted_list(&(model->sorted_probn[i-1]));
			if ((max_ng >= (i+1)) && (model->n_ng[i] > 0))
				init_sorted_list(&(model->sorted_bowtn[i-1]));

			_errmsg = ReadNgrams(fp, model, i, idfmt);
			if (_errmsg != LM_SUCCESS) {
				*err_no = _errmsg;

				lm_free (model);

				return NULL;
			}

			model->n_ng[i-1] =
				model->is32bits ? FIRST_NG32(model, i,
											 model->n_ng[i-2]) : FIRST_NG(model,
																	 i, model->
																	 n_ng[i-2]);
			model->n_ngprob[i-1] = model->sorted_probn[i-1].free;
			model->ngprob[i-1] =
				vals_in_sorted_list(&(model->sorted_probn[i-1]));
			E_INFO("%8d = %d-grams created\n", model->n_ng[i-1], i);
			E_INFO("%8d = prob%d entries\n", model->n_ngprob[i-1], i);

			free_sorted_list(&model->sorted_probn[i-1]);
		}

    *err_no = LM_SUCCESS;
    return model;
}


/**
   \brief Write an Arpa LM header (Sphinx 3.x -specific)
*/
static void
lm_write_arpa_header(lm_t * lmp,            /**< In: language model */
					 FILE * fp              /**< In: file stream */
    )
{
    uint32 i, j;

	/* Print header */
    for (i = 0; txtheader[i] != NULL; i++)
        fprintf(fp, "%s\n", txtheader[i]);

	/* Print probabilities format */
	for (i = lmp->max_ng; i >= 2; i--) {
		switch (i)
		{
		case 2:
			fprintf (fp, "\np(wd2|wd1)      ");
			break;
		case 3:
			fprintf (fp, "\np(wd3|wd1,wd2)  ");
			break;
		default:/*i>=4*/
			fprintf (fp, "\np(wd%d|wd1...wd%d)", i, i-1);
		}
		fprintf (fp, "= if(%dgram exists)              p_%d(wd1", i, i);
		if (i == 2)
			fprintf (fp, ",wd2)\n");
		else {/*i>=3*/
			fprintf (fp, "...wd%d)\n                  else if(%dgram w1",
					 i, i-1);
			switch (i)
			{
			case 3:
				fprintf (fp, ",w2 exists)   bo_wt_2(w1,w2)*p(wd3|wd2)\n");
				break;
			case 4:
				fprintf (fp, "...w3 exists) bo_wt_3(w1...w3)*p(wd4|wd2,wd3)\n");
				break;
			default:/*i>=5*/
				fprintf (fp, "...w%d exists) bo_wt_%d(w1...w%d)", i-1, i-1, i-1);
				fprintf (fp, "*p(wd%d|wd2...wd%d)\n", i, i-1);
			}
		}
		fprintf (fp, "                  else                          ");
		switch (i)
		{
		case 2:
			fprintf (fp, "bo_wt_1(wd1)*p_1(wd2)\n");
			break;
		case 3:
			fprintf (fp, "p(wd3|w2)\n");
			break;
		case 4:
			fprintf (fp, "p(wd4|w2,w3)\n");
			break;
		default:/*i>=5*/
			fprintf (fp, "p(wd%d|w2...w%d)\n", i, i-1);
		}
	}
    fprintf (fp, "\nAll probabilities and back-off weights (bo_wt) ");
	fprintf (fp, "are given in log10 form.\n");

	/* print data formats */
	fprintf(fp, "\\\nData formats:\n");
    fprintf(fp, "\nBeginning of data mark: \\data\\\n");
    for (i = 1; i <= lmp->max_ng; i++) {
        fprintf(fp, "ngram %d=nr            number of %d-grams\n", i, i);
    }
    fprintf(fp, "\n");
    for (i = 1; i <= lmp->max_ng; i++) {
        fprintf(fp, "\\%d-grams:\n", i);
        fprintf(fp, "p_%d     ", i);
        for (j = 1; j <= i; j++) {
            fprintf(fp, "wd_%d ", j);
        }
        if (i == lmp->max_ng) {
            fprintf(fp, "\n");
        }
        else {
            fprintf(fp, "bo_wt_%d\n", i);
        }
    }

	/* Print end */
    fprintf(fp, "\n");
    fprintf(fp, "end of data mark: \\end\\\n");
    fprintf(fp, "\n");
}

/**
   \brief Write the N-gram counts for ARPA file format. 
*/
static void
lm_write_arpa_count(lm_t * lmp,            /**< In: language model */
					FILE * fp              /**< In: file stream */
    )
{
	uint32 i;

    fprintf(fp, "\\data\\\n");

    fprintf(fp, "ngram %d=%d\n", 1, lmp->n_ng[0]);

	for (i = 2; i <= lmp->max_ng; i++)
		if (lmp->n_ng[i-1])
			fprintf(fp, "ngram %d=%d\n", i, lmp->n_ng[i-1]);

    fprintf(fp, "\n");
}

/**
   \brief Write N-gram in Arpa format without the header (N >= 2)

   This recursive function is called by lm_write_arpa_Ngram function
*/
static void
lm_write_arpa_Ngram_rec	(lm_t 		 * lmp,	 /**< In: language model */
						 uint32 	   Nw,	 /**< In: (Nw)gram level of writting
													  (for a bigram, Nw=2...) */
						 uint32 	   Nc,	 /**< In: current (Nc)gram level
																 in recursion */
						 s3lmwid32_t * lw,	 /**< In: list of word IDs */
						 int32 		   b_ng, /**< In: search start */
						 int32 		   n_ng, /**< In: search end */
						 FILE 		 * fp	 /**< In: a file stream */
						 )
{
    uint32 probid, bowtid, i, j;
	int32 b_nng, n_nng;

    for (i = b_ng; i < n_ng; i++) {
		if (Nc >= Nw) {
			if (lmp->is32bits) {
				assert(lmp->ng32[Nc-1]);
				lw[Nc-1] = lmp->ng32[Nc-1][i].wid;
				probid = lmp->ng32[Nc-1][i].probid;
				bowtid = lmp->ng32[Nc-1][i].bowtid;
			}
			else {
				assert(lmp->ng[Nc-1]);
				lw[Nc-1] = (s3lmwid32_t) lmp->ng[Nc-1][i].wid;
				probid = (uint32) lmp->ng[Nc-1][i].probid;
				bowtid = lmp->ng[Nc-1][i].bowtid;
			}
			
			fprintf(fp, "%.4f ", lmp->ngprob[Nc-1][probid].f);
			
			for (j = 1; j < Nc; j++) {
				fprintf(fp, "%s", lmp->wordstr[lw[j-1]]);
				fprintf(fp, " ");
			}
			
			fprintf(fp, "%s", lmp->wordstr[lw[Nc-1]]);
			
			if ((Nc < lmp->max_ng) && (lmp->ngbowt[Nc])) {
				fprintf(fp, " ");
				fprintf(fp, "%.4f\n", lmp->ngbowt[Nc][bowtid].f);
			}
			else
				fprintf(fp, "\n");
		}
		else {
            if (lmp->is32bits) {
                assert(lmp->ng32[Nc-1]);
                b_nng =
                    lmp->ng_segbase[Nc][i >> (lmp->log_ng_seg_sz[Nc-1])] +
                    lmp->ng32[Nc-1][i].firstnng;
                n_nng =
                    lmp->ng_segbase[Nc][(i + 1) >> (lmp->log_ng_seg_sz[Nc-1])] +
                    lmp->ng32[Nc-1][i + 1].firstnng;

				lw[Nc-1] = lmp->ng32[Nc-1][i].wid;
            }
            else {
                assert(lmp->ng[Nc-1]);
                b_nng =
                    lmp->ng_segbase[Nc][i >> (lmp->log_ng_seg_sz[Nc-1])] +
                    lmp->ng[Nc-1][i].firstnng;
                n_nng =
                    lmp->ng_segbase[Nc][(i + 1) >> (lmp->log_ng_seg_sz[Nc-1])] +
                    lmp->ng[Nc-1][i + 1].firstnng;

				lw[Nc-1] = (s3lmwid32_t) lmp->ng[Nc-1][i].wid;
            }

			lm_write_arpa_Ngram_rec (lmp, Nw, Nc+1, lw, b_nng, n_nng, fp);
		}
	}
}

/**
   \brief Write N-gram in Arpa format with the header (N >= 1)
*/
static void
lm_write_arpa_Ngram(lm_t * lmp,	/**< In: language model */
					uint32 N,	/**< In: N-gram level (for a bigram, N=2...) */
					FILE * fp	/**< In: file stream */
    )
{
    int32 i, b_bg, n_bg;
	s3lmwid32_t *lw;
	
	fprintf(fp, "\\%d-grams:\n", N);

	if (N >= 2) {
		/* write bigrams or more */
		lw = (s3lmwid32_t *) ckd_calloc (N, sizeof(s3lmwid32_t));
		if (lw != NULL) {
			for (i = 0; i <= lmp->n_ng[0] - 1; i++) {
				b_bg = lmp->ug[i].firstbg;
				n_bg = lmp->ug[i + 1].firstbg;

				/* first word */
				lw[0] = i;
				
				/* call recursive function */
				lm_write_arpa_Ngram_rec (lmp, N, 2, lw, b_bg, n_bg, fp);
			}
		}
		else
			E_FATAL("Can't allocate memory for write Arpa %d-grams\n", N);
	}
	else /* N == 1 */
		/* write unigrams */
		for (i = 0; i < lmp->n_ng[0]; i++) {
			fprintf(fp, "%.4f ", lmp->ug[i].prob.f);
			fprintf(fp, "%s", lmp->wordstr[i]);
			fprintf(fp, " ");
			fprintf(fp, "%.4f\n", lmp->ug[i].bowt.f);
		}

    fprintf(fp, "\n");
}

/**
   \brief Write the end for an Arpa file format
 */
static void
lm_write_arpa_end(lm_t * lmp,            /**< In: language model */
				  FILE * fp              /**< In: file stream */
    )
{
    fprintf(fp, "\\end\\\n");
}

/**
   \brief Write a LM in Arpa file format
	  \return Error code
*/
int32
lm_write_arpa_text	(lm_t		* lmp,		/**< In: language model */
					 const char	* outputfn	/**< In: output file name */
					 )
{
    FILE *fp;
	uint32 N;
    int usingPipe;

    E_INFO("Dumping LM to %s\n", outputfn);
    if ((fp = fopen_comp(outputfn, "w", &usingPipe)) == NULL) {
        E_ERROR("Cannot create file %s\n", outputfn);
        return LM_FAIL;
    }

    lm_write_arpa_header(lmp, fp);
    lm_write_arpa_count(lmp, fp);

    lm_convert_structure(lmp, lmp->is32bits);

	for (N = 1; N <= lmp->max_ng; N++)
		if (lmp->n_ng[N-1] > 0)
			lm_write_arpa_Ngram(lmp, N, fp);

	lm_write_arpa_end(lmp, fp);

    fclose_comp(fp, usingPipe);
    return LM_SUCCESS;
}
