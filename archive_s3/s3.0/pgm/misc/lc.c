/*
 * lc.c -- Left context transformations
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
 * 07-Apr-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


/*
 * Input lines of the form:
 *   354 .PERIOD                  => (TD)  p  ih  r  iy  ax  dd   ()
 *     9 IN                       => (S)   [[ AX  => IX  ]] n   (F)
 *     6 HELP PAY                 => (AX)  hh  eh  l  [[ => PD  ]] p  ey   (F)
 * Output 1st phone transformations only.
 */

#include <libutil/libutil.h>

static char *phonestr[] = {
    "AA",
    "AE",
    "AH",
    "AO",
    "AW",
    "AX",
    "AXR",
    "AY",
    "B",
    "BD",
    "CH",
    "D",
    "DD",
    "DH",
    "DX",
    "EH",
    "ER",
    "EY",
    "F",
    "G",
    "GD",
    "HH",
    "IH",
    "IX",
    "IY",
    "JH",
    "K",
    "KD",
    "L",
    "M",
    "N",
    "NG",
    "OW",
    "OY",
    "P",
    "PD",
    "R",
    "S",
    "SH",
    "T",
    "TD",
    "TH",
    "TS",
    "UH",
    "UW",
    "V",
    "W",
    "Y",
    "Z",
    "ZH",
    "--",	/* Dummy empty-phone */
   NULL
};


static int32 phone_str2id (char *str)
{
    int32 i;
    
    ucase (str);
    
    for (i = 0; phonestr[i] && (strcmp (phonestr[i], str) != 0); i++);
    return (phonestr[i] ? i : -1);
}


#define MAX_WORDS	4092

main (int32 argc, char *argv[])
{
    char line[16384], **wptr;
    int32 i, n, k, np;
    
    if (argc > 1) {
	E_INFO("Usage: %s < <result-of-pronerralign>\n", argv[0]);
	exit(0);
    }
    
    for (np = 0; phonestr[np]; np++);
    E_INFO("%d phones\n");
    
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
	
	if ((strcmp (wptr[i+1], "[[") != 0) && (strcmp (wptr[i+2], "[[") != 0)) {
	    /* No error */
	    printf ("%6d %-5s %-5s %-5s          %s\n",
		    k, wptr[i], wptr[i+1], wptr[i+2], wptr[1]);
	} else if (strcmp (wptr[i+1], "[[") == 0) {
	    /*
	     * First phone got transformed.  Must be:
	     *     (lc) [[ => ee ]] p2 (rc),
	     *     (lc) [[ ee => ]] p2 (rc), or
	     *     (lc) [[ pp => ee ]] p2 (rc)
	     */
	    if (n-i <= 6)
		continue;
	    
	    if ((strcmp (wptr[i+2], "=>") == 0) &&
		(strcmp (wptr[i+4], "]]") == 0) &&
		(strcmp (wptr[i+5], "[[") != 0)) {
		printf ("%6d %-5s %-5s %-5s => %-5s %s\n",
			k, wptr[i], wptr[i+3], wptr[i+5], "--", wptr[1]);
	    } else if ((strcmp (wptr[i+3], "=>") == 0) &&
		       (strcmp (wptr[i+4], "]]") == 0) &&
		       (strcmp (wptr[i+5], "[[") != 0)) {
		printf ("%6d %-5s %-5s %-5s => %-5s %s\n",
			k, wptr[i], "--", wptr[i+5], wptr[i+2], wptr[1]);
	    } else if ((strcmp (wptr[i+3], "=>") == 0) &&
		       (strcmp (wptr[i+5], "]]") == 0) &&
		       (strcmp (wptr[i+6], "[[") != 0) &&
		       (n-i > 7)) {
		printf ("%6d %-5s %-5s %-5s => %-5s %s\n",
			k, wptr[i], wptr[i+4], wptr[i+6], wptr[i+2], wptr[1]);
	    }
	}
    }
}
