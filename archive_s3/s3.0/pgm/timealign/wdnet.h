/*
 * wdnet.h -- Wordnet for alignment
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 28-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _TIMEALIGN_WDNET_H_
#define _TIMEALIGN_WDNET_H_


#include <libutil/libutil.h>
#include <libmain/dict.h>
#include <libmain/wnet.h>


/*
 * Build a graph word nodes for the given word transcript wd[].  Allow alternative
 * pronunciations for each word, compound forms of words, and optional intermediate
 * silence and filler noise words, as specified by the additional arguments.
 * However, a START_WORD is always inserted at the head, and a FINISH_WORD at the
 * end of the word net.
 * Return value: list of word nodes (wnode_t *).
 */
glist_t wdnet_build (dict_t *dict,	/* In: Dictionary to convert words to IDs */
		     char **wd,		/* In: Words in input transcript */
		     int32 nwd,		/* In: No. of words in wd[] */
		     int32 noalt,	/* In: If noalt, do not try alternative
					   pronunciations, and do not try compound words */
		     int32 nosil,	/* In: If nosil, do not insert optional
					   SILENCE_WORD between words */
		     int32 nonoise,	/* In: If nonoise, do not insert optional
					   non-silence noise words between words */
		     wnode_t **start,	/* Out: Return ptr to a START_WORD node inserted
					   at the head of the wordnet */
		     wnode_t **end);	/* Out: Return ptr to a FINISH_WORD node inserted
					   at the tail of the wordnet */

/*
 * Free the given word net graph.
 */
void wdnet_free (glist_t wdnet);


#endif
