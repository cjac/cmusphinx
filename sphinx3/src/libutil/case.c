/*
 * case.c -- Upper/lower case conversion routines
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
 * 18-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added strcmp_nocase.  Moved UPPER_CASE and LOWER_CASE definitions to .h.
 * 
 * 16-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#include "case.h"


void lcase(register char *cp)
{
    for (; *cp; cp++)
	*cp = LOWER_CASE(*cp);
}


void ucase(register char *cp)
{
    for (; *cp; cp++)
	*cp = UPPER_CASE(*cp);
}


int32 strcmp_nocase (const char *str1, const char *str2)
{
    char c1, c2;
    
    for (;;) {
	c1 = *(str1++);
	c1 = UPPER_CASE(c1);
	c2 = *(str2++);
	c2 = UPPER_CASE(c2);
	if (c1 != c2)
	    return (c1-c2);
	if (c1 == '\0')
	    return 0;
    }
}
