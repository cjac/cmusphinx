/*
 * rc.c -- Right context transformations
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
 * 08-Apr-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


/*
 * Input lines of the form:
 *   354 .PERIOD                  => (TD)  p  ih  r  iy  ax  dd   ()
 *     9 IN                       => (S)   [[ AX  => IX  ]] n   (F)
 *     6 HELP PAY                 => (AX)  hh  eh  l  [[ => PD  ]] p  ey   (F)
 * Output last phone transformations only.
 */

#include <libutil/libutil.h>


#define MAX_WORDS	4092

main (int32 argc, char *argv[])
{
    char line[16384], **wptr;
    int32 i, n, k;
    
    if (argc > 1) {
	E_INFO("Usage: %s < <result-of-pronerralign>\n", argv[0]);
	exit(0);
    }
    
    wptr = (char **) ckd_calloc (MAX_WORDS, sizeof(char *));
    
    while (fgets (line, sizeof(line), stdin) != NULL) {
	if ((n = str2words (line, wptr, MAX_WORDS)) < 0)
	    E_FATAL("str2words(%s) failed; increase %d(?)\n", line, MAX_WORDS);
	
	/* Read first (count) field */
	if (n == 0) continue;
	if (sscanf (wptr[0], "%d", &k) != 1)
	    E_FATAL("First field not a count: %s\n", wptr[0]);
	
	/* Find => separator after word list */
	for (i = 0; (i < n) && (strcmp (wptr[i], "=>") != 0); i++);
	i++;		/* Hopefully at (lc) */

	/* Must have at least: (lc) p1 p2 (rc) */
	if (n-i <= 3)
	    continue;	
	assert (i > 2);
	assert (wptr[i][0] == '(');	/* (lc) */
	assert (wptr[n-1][0] == '(');	/* (rc) */

	if ((strcmp (wptr[n-2], "]]") != 0) && (strcmp (wptr[n-3], "]]") != 0)) {
	    /* No error */
	    printf ("%6d %-5s %-5s %-5s          %s\n",
		    k, wptr[n-3], wptr[n-2], wptr[n-1], wptr[1]);
	} else if (strcmp (wptr[n-2], "]]") == 0) {
	    /*
	     * Last phone got transformed.  Look for:
	     *     (lc) p [[ => ee ]] (rc),
	     *     (lc) p [[ ee => ]] (rc), or
	     *     (lc) p [[ pp => ee ]] (rc)
	     */
	    if (n-i <= 6)
		continue;	/* Not enough fields */
	    
	    if ((strcmp (wptr[n-4], "=>") == 0) &&
		(strcmp (wptr[n-5], "[[") == 0) &&
		(strcmp (wptr[n-6], "]]") != 0)) {
		printf ("%6d %-5s %-5s %-5s => %-5s %s\n",
			k, wptr[n-6], wptr[n-3], wptr[n-1], "--", wptr[1]);
	    } else if ((strcmp (wptr[n-3], "=>") == 0) &&
		       (strcmp (wptr[n-5], "[[") == 0) &&
		       (strcmp (wptr[n-6], "]]") != 0)) {
		printf ("%6d %-5s %-5s %-5s => %-5s %s\n",
			k, wptr[n-6], "--", wptr[n-1], wptr[n-4], wptr[1]);
	    } else if ((strcmp (wptr[n-4], "=>") == 0) &&
		       (strcmp (wptr[n-6], "[[") == 0) &&
		       (strcmp (wptr[n-7], "]]") != 0) &&
		       (n-i > 7)) {
		printf ("%6d %-5s %-5s %-5s => %-5s %s\n",
			k, wptr[n-7], wptr[n-3], wptr[n-1], wptr[n-5], wptr[1]);
	    }
	}
    }
}
