/*
 * line2words.c -- Convert a line to an array of words
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "line2words.h"
#include "ckd_alloc.h"
#include "err.h"
#include <ctype.h>


#define MAX_LINE2WORDS		1024


int32 line2words (char *line, char ***ptr_out)
{
    char *w, *d, **ptr;
    int32 i, n;
    
    ptr = (char **) ckd_calloc (MAX_LINE2WORDS, sizeof(char *));
    
    n = 0;
    i = 0;
    for (;;) {
	/* Skip whitespace before word */
	for (; line[i] && (isspace(line[i])); i++);
	if (! line[i])
	    break;
	
	if (n >= MAX_LINE2WORDS-2)
	    E_FATAL("Increase MAX_LINE2WORDS\n");
	
	ptr[n++] = line+i;
	for (; line[i] && (! isspace(line[i])); i++);
	if (! line[i])
	    break;
	line[i++] = '\0';
    }
    
    ptr[n] = NULL;
    *ptr_out = ptr;
    
    return n;
}
