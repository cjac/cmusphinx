/*
 * filename.c -- File and path name operations.
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
 * 30-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "filename.h"


/* Strip off all leading pathname components */
void path2basename (char *path, char *base)
{
    int32 i, l;
    
    l = strlen(path);
    for (i = l-1; (i >= 0) && (path[i] != '/'); --i);
    strcpy (base, path+i+1);
}


/* Strip off the shortest trailing .xyz suffix */
void strip_fileext (char *path, char *root)
{
    int32 i, l;
    
    l = strlen(path);
    for (i = l-1; (i >= 0) && (path[i] != '.'); --i);
    if (i < 0)
	strcpy (root, path);	/* Didn't find a . */
    else {
	path[i] = '\0';
	strcpy (root, path);
	path[i] = '.';
    }
}
