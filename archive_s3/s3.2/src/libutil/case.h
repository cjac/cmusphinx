/*
 * case.h -- Upper/lower case conversion routines
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
 * 		Added strcmp_nocase, UPPER_CASE and LOWER_CASE definitions.
 * 
 * 16-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _LIBUTIL_CASE_H_
#define _LIBUTIL_CASE_H_


#include "prim_type.h"


/* Return upper case form for c */
#define UPPER_CASE(c)	((((c) >= 'a') && ((c) <= 'z')) ? (c-32) : c)

/* Return lower case form for c */
#define LOWER_CASE(c)	((((c) >= 'A') && ((c) <= 'Z')) ? (c+32) : c)


/* Convert str to all upper case */
void ucase(char *str);

/* Convert str to all lower case */
void lcase(char *str);

/*
 * Case insensitive string compare.  Return the usual -1, 0, +1, depending on
 * str1 <, =, > str2 (case insensitive, of course).
 */
int32 strcmp_nocase (const char *str1, const char *str2);


#endif
