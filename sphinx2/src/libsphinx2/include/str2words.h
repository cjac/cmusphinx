/*
 * str2words.h -- Convert a line to an array of words
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
 * 07-Apr-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _LIBUTIL_STR2WORDS_H_
#define _LIBUTIL_STR2WORDS_H_

/*
 * Convert a line to an array of "words", based on whitespace separators.  A word
 * is a string with no whitespace chars in it.
 * Note that the string line is modified as a result: NULL chars are placed after
 * every word in the line.
 * Return value: No. of words found; -1 if no. of words in line exceeds n_wptr.
 */
int32 str2words (char *line,	/* In: line to be parsed */
		 char **wptr,	/* In/Out: Array of pointers to words found in line.
				   The array must be allocated by the caller */
		 int32 n_wptr);	/* In: Size of wptr array */

#endif
