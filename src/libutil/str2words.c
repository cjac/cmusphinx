/*
 * str2words.c -- Convert a string to an array of words
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 21-Oct-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "str2words.h"


int32 str2words (char *line, char **ptr, int32 max_ptr)
{
    int32 i, n;
    
    n = 0;	/* #words found so far */
    i = 0;	/* For scanning through the input string */
    for (;;) {
	/* Skip whitespace before next word */
	for (; line[i] && (isspace(line[i])); i++);
	if (! line[i])
	    break;
	
	if (n >= max_ptr) {
	    /*
	     * Pointer array size insufficient.  Restore NULL chars inserted so far
	     * to space chars.  Not a perfect restoration, but better than nothing.
	     */
	    for (; i >= 0; --i)
		if (line[i] == '\0')
		    line[i] = ' ';
	    
	    return -1;
	}
	
	/* Scan to end of word */
	ptr[n++] = line+i;
	for (; line[i] && (! isspace(line[i])); i++);
	if (! line[i])
	    break;
	line[i++] = '\0';
    }
    
    return n;
}
