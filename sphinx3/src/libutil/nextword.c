/*
 * nextword.c -- Yet another "next word from a string" package.
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


#include "nextword.h"


int32 nextword (char *line, char *delim, char **word, char *delimfound)
{
    char *w, *d;

    /* Skip past any preceding delimiters */
    for (w = line; *w; w++) {
        for (d = delim; *d && (*d != *w); d++);
	if (! *d)
	    break;
    }
    if (! *w)
        return -1;

    *word = w;		/* Beginning of word */

    /* Skip until first delimiter char */
    for (w++; *w; w++) {
        for (d = delim; *d && (*d != *w); d++);
	if (*d)
	    break;
    }
    
    /* Replace delimiter with NULL char, but return the original first */
    *delimfound = *w;
    *w = '\0';

    return (w - *word);	/* Length of word */
}
