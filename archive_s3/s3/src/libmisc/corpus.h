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
