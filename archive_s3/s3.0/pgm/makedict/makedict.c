/*
 * makedict.c -- Make a dictionary for the given vocab, using the given dictionaries.
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
 * 23-Apr-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


/*
 * Vocab input file should contain one or two "words" per line.  If two words, the
 * 2nd word is used to look up the given dictionaries for pronunciations.  But once
 * pronunciations are found in one dictionary, the remaining are not searched (to
 * avoid extract duplicate pronunciations).
 * Vocab input file is assumed to contain a unique set of words, no duplicates.
 */


#include <libutil/libutil.h>
#include <libmain/dict.h>


main (int32 argc, char *argv[])
{
    dict_t **d;
    int32 i, k, p, wid;
    char line[16384], *wp[1024];
    
    if (argc < 2) {
	E_INFO("Usage: %s dictfile [dictfile ...] < vocabfile\n", argv[0]);
	exit(0);
    }
    d = (dict_t **) ckd_calloc (argc-1, sizeof(dict_t *));
    
    for (i = 1; i < argc; i++)
	d[i-1] = dict_init (NULL, argv[i], NULL, 0);
    
    while (fgets (line, sizeof(line), stdin) != NULL) {
	if ((k = str2words (line, wp, 1024)) < 0)
	    E_FATAL("Line too long: %s\n", line);
	if (k > 2)
	    E_FATAL("Vocab entry contains too many words\n");
	
	if (k == 0)
	    continue;
	if (k == 1)
	    wp[1] = wp[0];
	
	/* Look up word in each dictionary until found */
	k = 0;
	for (i = 0; (i < argc-1) && (k == 0); i++) {
	    wid = dict_wordid (d[i], wp[1]);
	    if (NOT_WID(wid))
		continue;
	    
	    for (wid = dict_basewid(d[i], wid);
		 IS_WID(wid);
		 wid = dict_nextalt(d[i], wid)) {
		k++;
		if (k == 1)
		    printf ("%s\t", wp[0]);
		else
		    printf ("%s(%d)\t", wp[0], k);
		
		for (p = 0; p < dict_pronlen(d[i], wid); p++)
		    printf (" %s", dict_ciphone_str (d[i], wid, p));
		printf ("\n");
	    }
	}
	if (k == 0)
	    E_ERROR("No pronunciation for: '%s'\n", wp[0]);
    }
}
