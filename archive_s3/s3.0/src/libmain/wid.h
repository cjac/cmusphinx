/*
 * wid.c -- Mapping word-IDs between LM and dictionary.
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
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#ifndef _LIBMAIN_WID_H_
#define _LIBMAIN_WID_H_


/*
 * Create mappings between dictionary and LM word-IDs.  In short:
 *    - An array of s3lmwid_t entries (map[]) is created; where map[i] is the LM word-ID for
 * 	the dictionary word-ID i.  Mappings are created for the alternative pronunciations as
 * 	well.  For words not in the LM, the corresponding entries are BAD_LMWID.
 *    - Similarly, lm->ug[u].dictwid is assigned the dictionary word id for unigram word u.
 * Return value: The map[] array built as described above.
 */
s3lmwid_t *wid_dict_lm_map (dict_t *dict,	/* In: Dictionary */
			    lm_t *lm);		/* In/Out: LM; lm->ug[].dictwid values are
						   updated. */

#endif
