/*
 * dictmisc.h -- Miscellaneous word dictionary related functions without requiring
 * 		libmain/dict.h.
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
 * 28-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBMISC_DICTMISC_H_
#define _LIBMISC_DICTMISC_H_


/*
 * Strip any terminating alternative pronunciation spec from the given word.  That is,
 * if word is of the form <baseword>(...), where <baseword> is non-empty, replace the
 * left-paren with a NULL char.
 * Return value: If truncated to <baseword>, its length; otherwise -1.
 */
int32 word2basestr (char *word);


#endif
