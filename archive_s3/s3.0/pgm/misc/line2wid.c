/*
 * line2wid.c -- Convert a ref or hyp line (word string sequence) to an array of
 * 		word ids.
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
 * 27-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <main/dict.h>


int32 is_uttid (char *word, char *uttid)
{
    int32 k;
    
    k = strlen(word);
    if ((word[0] == '(') && (word[k-1] == ')')) {
	word[k-1] = '\0';
	strcpy (uttid, word+1);
	word[k-1] = ')';
	return 1;
    }
    return 0;
}


int32 line2wid (dict_t *dict, char *line, s3wid_t *wid, int32 max_n_wid, int32 add_oov,
		char *uttid)
{
    char *lp, word[1024];
    int32 n, k;
    s3wid_t w;
    s3cipid_t ci[1];
    
    uttid[0] = '\0';
    ci[0] = (s3cipid_t) 0;
    
    lp = line;
    n = 0;
    while (sscanf (lp, "%s%n", word, &k) == 1) {
	lp += k;

	if (n >= max_n_wid)
	    return -n;
	
	if (is_uttid (word, uttid))
	    break;
	
	wid[n] = dict_wordid (dict, word);	/* Up to caller to handle BAD_WIDs */
	if (NOT_WID(wid[n])) {
	    /* OOV word */
	    if (add_oov) {
		E_INFO("Adding %s to dictionary\n", word);
		wid[n] = dict_add_word (dict, word, NULL, 0);
		if (NOT_WID(wid[n]))
		    E_FATAL("dict_add_word(%s) failed for line: %s\n", word, line);
	    } else
		E_FATAL("Unknown word (%s) in line: %s\n", word, line);
	}
	
	n++;
    }
    
    if (sscanf (lp, "%s", word) == 1)	/* Check that line really ended */
	E_WARN("Nonempty data ignored after uttid(%s) in line: %s\n", uttid, line);
    
    return n;
}
