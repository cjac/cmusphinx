/*
 * dictmisc.c -- Miscellaneous word dictionary related functions without requiring
 * 		libmain/dict.h.
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
 * 28-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include "dictmisc.h"


int32 word2basestr (char *word)
{
    int32 i, len;
    
    len = strlen(word);
    if (word[len-1] == ')') {
	for (i = len-2; (i > 0) && (word[i] != '('); --i);
	
	if (i > 0) {
	    /* The word is of the form <baseword>(...); strip from left-paren */
	    word[i] = '\0';
	    return i;
	}
    }

    return -1;
}
