/* ====================================================================
 * Copyright (c) 1999-2003 Carnegie Mellon University.  All rights
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
#include <libutil/libutil.h>
#include <libs3decoder/lm.h>
#include <libs3decoder/logs3.h>
#include <programs/cmd_ln_args.h>

#define MAX_NGRAMS 5100
#define MAX_STRLEN 100

int read_ngrams(char *ngrams_file, char *ngrams[], int max_lines);
void print_ngrams(char *ngrams[], int nlines);


int main(int argc, char *argv[])
{
    char *lm_file;
    char *args_file;
    char *ngrams_file;

    char *ngrams[MAX_NGRAMS];

    float64 lw, wip, uw, logbase;

    int n;

    lm_t *lm;

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

    logs3_init(logbase);
    
    /* initialize the language model */
    lm = lm_read(lm_file, lw, wip, uw);

    /* read in all the N-grams */
    n = read_ngrams(ngrams_file, ngrams, MAX_NGRAMS);
    /* print_ngrams(ngrams, n); */

    
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
int read_ngrams(char *ngrams_file, char *ngrams[], int max_lines)
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
            ngrams[n] = (char *) calloc(length, sizeof(char));
            strncpy(ngrams[n], line_read, length-1);
            n++;
        } else {
            break;
        }
    }

    return n;
}


/**
 * Prints an array of N-grams.
 *
 * args:
 * ngrams - the N-gram array to print
 * nlines - the number of N-grams in the array
 */
void print_ngrams(char *ngrams[], int nlines)
{
    int i;
    
    for (i = 0; i < nlines; i++) {
        printf("%s\n", ngrams[i]);
    }
}
