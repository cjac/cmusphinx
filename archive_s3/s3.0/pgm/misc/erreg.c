/*
 * erreg.c -- Extract error regions
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 01-May-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>


#define MAX_WORDS	4092

main (int32 argc, char *argv[])
{
    char line[16384], **wptr;
    int32 i, n, lineno;
    
    if (argc > 1) {
	E_INFO("Usage: %s < <filtered-dp-output>\n", argv[0]);
	exit(0);
    }
    
    wptr = (char **) ckd_calloc (MAX_WORDS, sizeof(char *));
    
    lineno = 0;
    while (fgets (line, sizeof(line), stdin) != NULL) {
	lineno++;
	
	if ((n = str2words (line, wptr, MAX_WORDS)) < 0)
	    E_FATAL("str2words(%s) failed; increase %d(?)\n", line, MAX_WORDS);
	
	for (i = 0; (i < n) && (strcmp (wptr[i], "<<") != 0); i++);
	if (i >= n)
	    E_FATAL("Bad line (#%d)\n", lineno);
	n = i-1;
	
	for (i = 0; i < n; i++) {
	    if (strcmp (wptr[i], "[[") == 0) {
		printf ("%s\t", wptr[n]);
		
		if (i > 0)
		    printf ("%s", wptr[i-1]);
		else
		    printf ("<s>");
		
		for (; (i < n) && (strcmp (wptr[i], "]]") != 0); i++)
		    printf (" %s", wptr[i]);
		assert (i < n);
		printf (" ]]");
		
		if (i < n-1)
		    printf (" %s", wptr[i+1]);
		else
		    printf (" </s>");

		printf ("\n");
	    }
	}
    }
}
