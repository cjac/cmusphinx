/*
 * 
 * Jun 23, 1989 Fil Alleva, faa@cs.cmu.edu
 *      Fixed typo/bug that caused malloc to allocate a buffer that
 *      with 2 less bytes than required.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "s2types.h"
#include "strfuncs.h"

char *salloc (char const *str)
{
    int32 len = strlen(str)+1;
    char *buf;
    
    if ((buf = (char *) malloc (len)) == NULL) {
	printf ("%s(%d): malloc failed\n", __FILE__, __LINE__);
	exit(-1);
    }
	
    strcpy (buf, str);
    return (buf);
}
