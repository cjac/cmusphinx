/*
 * line2words.h -- Convert a line to an array of words
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


#ifndef _LINE2WORDS_H_
#define _LINE2WORDS_H_


#include "prim_type.h"


/*
 * Convert a line to an array of "words", based on whitespace separators.  A word
 * is a string with no whitespace chars in it.
 * Note that the string line is modified as a result: NULL chars are placed after
 * every word in the line.
 * Return value: No. of words found.
 */
int32 line2words (char *line,		/* In: line to be parsed */
		  char ***wptr);	/* Out: *ptr = Array of pointers to words found
					   in line.  The array is allocated by this
					   routine and should be freed by the caller
					   (using ckd_free()). */

#endif
