/*
 * proncount.c -- Extract pronunciations for words
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
 * Input lines of the form (sorted):
 *     ASKED AE S T 1
 *     ASKED AE S T 1
 *     ASKED AE S T 2
 *     ASKED AE S T 4
 *     ASKED AE S T AX 1
 *     ASKED AE S T AX DD 1
 *     ASKED AE S TD 1
 *     ASKED AE S TD 1
 * Output sums up counts for distinct pronunciations.
 */


#include <libutil/libutil.h>


static int32 cmp_wptr (char **p1, char **p2, int32 n)
{
    int32 i;
    
    for (i = 0; (i < n) && (strcmp (p1[i], p2[i]) == 0); i++);
    return (i < n) ? -1 : 0;
}


#define MAX_WORDS	4092

main (int32 argc, char *argv[])
{
    char line[16384], **wptr, **prev_wptr;
    int32 i, j, k, n, lineno, count, prev_n, total_count;
    
    if (argc > 1) {
	E_INFO("Usage: %s < <pron.count>\n", argv[0]);
	exit(0);
    }
    
    prev_wptr = (char **) ckd_calloc (1024, sizeof(char *));
    prev_n = -1;
    total_count = -1;
    
    wptr = (char **) ckd_calloc (MAX_WORDS, sizeof(char *));

    lineno = 0;
    while (fgets (line, sizeof(line), stdin) != NULL) {
	lineno++;
	
	if ((n = str2words (line, wptr, MAX_WORDS)) < 0)
	    E_FATAL("str2words(%s) failed; increase %d(?)\n", line, MAX_WORDS);
	if (n == 0) continue;
	
	/* Read last (count) field */
	if (sscanf (wptr[n-1], "%d", &count) != 1)
	    E_FATAL("Last field not a count: %s\n", wptr[n-1]);
	if (count == 0)
	    break;
	--n;	/* Omit count field */
	
	/* Check if same as previous line (except for count field) */
	if ((n == prev_n) && (cmp_wptr (wptr, prev_wptr, n) == 0))
	    total_count += count;
	else {
	    if (total_count > 0) {
		printf ("%5d", total_count);
		printf ("\t%s\t", prev_wptr[0]);
		for (i = 1; i < prev_n; i++)
		    printf (" %s", prev_wptr[i]);
		printf ("\n");
	    }
	    total_count = count;
	    
	    for (i = 0; i < prev_n; i++)
		ckd_free (prev_wptr[i]);
	    for (i = 0; i < n; i++)
		prev_wptr[i] = (char *) ckd_salloc (wptr[i]);
	    prev_n = n;
	}
    }

    if (total_count > 0) {
	printf ("%5d", total_count);
	printf ("\t%s\t", prev_wptr[0]);
	for (i = 1; i < prev_n; i++)
	    printf (" %s", prev_wptr[i]);
	printf ("\n");
    }
}
