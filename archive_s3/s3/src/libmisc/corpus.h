/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * corpus.h -- Corpus-file related misc functions.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 25-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _LIBMISC_CORPUS_H_
#define _LIBMISC_CORPUS_H_


#include <libutil/libutil.h>


/*
 * Structure for a corpus: essentially a set of strings each associated with a
 * unique ID.  (Such as a reference sentence file, hypothesis file, and various
 * control files.)
 */
typedef struct {
    hash_t ht;		/* Hash table for IDs */
    int32 n;		/* #IDs (and corresponding argument strings) in the corpus */
    char **str;		/* The argument strings */
} corpus_t;


/*
 * Load a corpus from the given file and return it.
 * Each line is a separate entry in the corpus.  Blank lines and lines beginning with a
 * hash character (#) are skipped.  The ID is the FIRST word in a line.
 * Return value: Ptr to corpus if successfully loaded (FATAL_ERROR if any error).
 */
corpus_t *corpus_load_headid (char *file);

/*
 * Similar to corpus_load_headid, but the ID is at the END of each line, in parentheses.
 */
corpus_t *corpus_load_tailid (char *file);


/*
 * Lookup the given corpus for the given ID and return the associated string.
 * Return NULL if ID not found.
 */
char *corpus_lookup (corpus_t *corp, char *id);


#endif
