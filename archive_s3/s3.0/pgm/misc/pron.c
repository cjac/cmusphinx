/*
 * pron.c -- Extract pronunciations for words
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
 * 02-May-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


/*
 * Input lines of the form:
 *   354 .PERIOD                  => (TD)  p  ih  r  iy  ax  dd   ()
 *     9 IN                       => (S)   [[ AX  => IX  ]] n   (F)
 *     6 HELP PAY                 => (AX)  hh  eh  l  [[ => PD  ]] p  ey   (F)
 * Output observed pronunciations for words and their counts
 */


#include <libutil/libutil.h>


#define MAX_WORDS	4092

main (int32 argc, char *argv[])
{
    char line[16384], **wptr;
    int32 i, j, k, n, lineno, count;
    
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
	if (n == 0) continue;
	
	for (i = 0; i < n; i++)
	    ucase (wptr[i]);
	
	/* Read first (count) field */
	if (sscanf (wptr[0], "%d", &count) != 1)
	    E_FATAL("First field not a count: %s\n", wptr[0]);
	if (count == 0)
	    break;
	
	/* Find => separator after word list, skip if word list length > 1 */
	for (i = 0; (i < n) && (strcmp (wptr[i], "=>") != 0); i++);
	i++;		/* Hopefully at (lc) */
	if (i != 3)
	    continue;

	/* Must have at least: (lc) p1 p2 (rc); remove () from lc and rc */
	if (n-i <= 3)
	    continue;
	k = strlen(wptr[i]) - 1;
	assert ((wptr[i][0] == '(') && (wptr[i][k] == ')'));
	wptr[i][k] = '\0';
	(wptr[i])++;
	k = strlen(wptr[n-1]) - 1;
	assert ((wptr[n-1][0] == '(') && (wptr[n-1][k] == ')'));
	wptr[n-1][k] = '\0';
	(wptr[n-1])++;
	
	/* Check if there are no errors */
	for (j = i; (j < n) && (strcmp (wptr[j], "[[") != 0); j++);
	if (j >= n) {
	    for (j = i; j < n; j++)
		lcase (wptr[j]);
	} else {
	    /* Check if the only error is deletion of first geminate phone */
	    if ((strcmp (wptr[i+1], "[[") == 0) && (strcmp (wptr[i+2], "=>") == 0) &&
		(strcmp (wptr[i+4], "]]") == 0) && (strcmp (wptr[i+3], wptr[i]) == 0)) {
		for (j = i+5; (j < n) && (strcmp (wptr[j], "[[") != 0); j++);
		if (j >= n) {
		    for (j = i; j < n; j++)
			lcase (wptr[j]);
		}
	    }
	}
	
	printf ("%s", wptr[1]);		/* The word */
	
	if ((strcmp (wptr[i+1], "[[") == 0) && (strcmp (wptr[i+2], "=>") == 0) &&
	    (strcmp (wptr[i+4], "]]") == 0)) {
	    if (strcmp (wptr[i+3], wptr[i]) == 0)
		printf (" %s", wptr[i]);
	    else
		printf (" (%s)", wptr[i]);
	}
	
	/* Print observed pronunciation */
	for (i++; i < n-1; i++) {
	    if (strcmp (wptr[i], "[[") != 0)
		printf (" %s", wptr[i]);
	    else {
		for (i++; (strcmp (wptr[i], "=>") != 0); i++)
		    printf (" %s", wptr[i]);
		for (i++; (strcmp (wptr[i], "]]") != 0); i++);
	    }
	}

	/* Print count */
	printf (" %d\n", count);
	fflush (stdout);
    }
}
