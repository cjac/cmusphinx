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


/*
 * A test program for the large language model module.
 * You should give this program an ARGS file, and a text file containing
 * a list of N-grams. It will output to stdout the language model
 * scores for each N-gram.
 *
 */

#include <stdio.h>
#include <string.h>
#include <lm.h>
#include <logs3.h>
#include <s3types.h>

#include "cmd_ln_args.h"
#include "metrics.h"


#define MAX_NGRAMS 5100
#define MAX_STRLEN 100

int read_ngrams(char *ngrams_file, char **ngrams, 
                s3lmwid_t *wid[], int32 nwords[], int max_lines, lm_t *lm);
int ngram2wid(char *word, int length, s3lmwid_t *w, lm_t *lm);
int score_ngram(s3lmwid_t *wid, int nwd, lm_t *lm);



int main(int argc, char *argv[])
{
    char *lm_file;
    char *args_file;
    char *ngrams_file;
    char *lmLoadTimer = "LM Load";
    char *lmLookupTimer = "LM Lookup";

    char *ngrams[MAX_NGRAMS];

    float64 lw, wip, uw, logbase;

    int i, n;
    
    int32 nwords[MAX_NGRAMS];
    int scores[MAX_NGRAMS];

    lm_t *lm;

    s3lmwid_t *wid[MAX_NGRAMS];


    if (argc < 3) {
        E_FATAL("USAGE: %s <lm_file> <args_file> <ngrams_file>\n", argv[0]);
    }

    args_file = argv[1];
    lm_file = argv[2];
    ngrams_file = argv[3];

    parse_args_file(args_file);

    lw = cmd_ln_float32("-lw");
    wip = cmd_ln_float32("-wip");
    uw = cmd_ln_float32("-uw");
    logbase = cmd_ln_float32("-logbase");

    logs3_init(logbase,1,1); /*Report progress and use log table*/

    metricsStart(lmLoadTimer);
    
    /* initialize the language model */
    /* HACK! This doesn't work for class-based LM */
    lm = lm_read(lm_file, "default",lw, wip, uw, 0, NULL,1);

    metricsStop(lmLoadTimer);

    /* read in all the N-grams */
    n = read_ngrams(ngrams_file, ngrams, wid, nwords, MAX_NGRAMS, lm);

    metricsStart(lmLookupTimer);

    /* scores the N-grams */
    for (i = 0; i < n; i++) {
        scores[i] = score_ngram(wid[i], nwords[i], lm);
    }

    metricsStop(lmLookupTimer);

    for (i = 0; i < n; i++) {
        printf("%-10d %s\n", scores[i], ngrams[i]);
    }

    printf("Bigram misses: %d \n", lm->n_bg_bo);
    printf("Trigram misses: %d \n", lm->n_tg_bo);

    fflush(stdout);

    metricsPrint();
    return 0;
}


/**
 * Reads all the N-grams in the given N-gram file into the array of strings.
 *
 * args:
 * ngrams_file - the N-gram file to read N-grams from
 * ngrams - the array of string to read N-grams into
 *
 * returns: the number of ngrams read
 */
int read_ngrams(char *ngrams_file, 
                char **ngrams, 
                s3lmwid_t *wid[], 
                int32 nwords[],
                int max_lines, 
                lm_t *lm)
{
    FILE *fp;
    char line_read[MAX_STRLEN];
    int n, length;
    
    if ((fp = fopen(ngrams_file, "r")) == NULL) {
        E_FATAL("Unable to open N-gram file %s\n", ngrams_file);
    }

    n = 0;

    /* read each line in the file into the ngrams array */
    while (fgets(line_read, MAX_STRLEN, fp) != NULL) {
        if (n < max_lines) {
            length = strlen(line_read);
            line_read[length-1] = '\0';
            ngrams[n] = (char *) ckd_calloc(length, sizeof(char));
            strncpy(ngrams[n], line_read, length-1);
            wid[n] = (s3lmwid_t *) ckd_calloc(3, sizeof(s3lmwid_t));
            nwords[n] = ngram2wid(line_read, length, wid[n], lm);
            n++;
        } else {
            break;
        }
    }

    return n;
}


/**
 * Map the given ngram string to an array of word IDs of the individual
 * words in the ngram.
 *
 * args:
 * ngram - the ngram string to map
 * length - the length of the ngram string
 * w - the word ID array
 * lm - the language model to use
 *
 * returns:
 * the number of words in the ngram string, or 0 if the string contains an
 * unknown word
 */
int ngram2wid(char *ngram, int length, s3lmwid_t *w, lm_t *lm)
{
    char *word[1024];
    int nwd;
    int i;
    
    if ((nwd = str2words(ngram, word, length)) < 0)
	E_FATAL("Increase word[] and w[] arrays size\n");
    
    for (i = 0; i < nwd; i++) {
	w[i] = lm_wid (lm, word[i]);
	if (NOT_S3LMWID(w[i])) {
	    E_ERROR("Unknown word: %s\n", word[i]);
	    return 0;
	}
    }

    return nwd;
}


/**
 * Scores the given N-gram using the given language model.
 *
 * args:
 * wid - the IDs of the sequence of words in the n-gram
 * nwd - the number of words in the n-gram
 * lm - the language model to use
 *
 * return: the language model score of the given sequence of words
 */
int score_ngram(s3lmwid_t *wid, int nwd, lm_t *lm)
{
    int32 score;
    
    score = 0;
    if (nwd == 3) {
      /* The last argument is a hack: the information there - the dict
       * ID - is never used if LM classes are not used, and classes
       * are not used in this code. Therefore, the last argument here
       * is a nop.
       */
      score = lm_tg_score(lm, wid[0], wid[1], wid[2], 0);
    } else if (nwd == 2) {
      /* Ditto.
       */
      score = lm_bg_score(lm, wid[0], wid[1], 0);
    } else if (nwd == 1) {
      /* Ditto.
       */
      score = lm_ug_score(lm, wid[0], 0);
    } else {
        printf("%d grams not supported\n", nwd);
    }
    
    return score;
}
