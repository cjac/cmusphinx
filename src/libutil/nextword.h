/*
 * nextword.h -- Yet another "next word from a string" package.
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
 * 21-Oct-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _LIBUTIL_NEXTWORD_H_
#define _LIBUTIL_NEXTWORD_H_


#include "prim_type.h"

/*
 * Yet another attempt at a clean "next-word-in-string" function.  See arguments below.
 * Return value: length of word if one was found, otherwise -1.
 */
int32
nextword (char *line,		/* In: String being searched for next word */
	  char *delim,		/* In: A word, if found, must be delimited at either
				   end by a character from this string (or at the end
				   by the NULL char) */
	  char **word,		/* Out: *word = ptr within line to beginning of first
				   word, if found.  Delimiter at the end of word replaced
				   with the NULL char. */
	  char *delimfound);	/* Out: *delimfound = original delimiter found at the end
				   of the word.  (This way, the caller can restore the
				   delimiter, preserving the original string.) */

#endif
