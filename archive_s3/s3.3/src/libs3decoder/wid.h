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

#ifndef _S3_WID_H_
#define _S3_WID_H_


#include <libutil/libutil.h>
#include "dict.h"
#include "lm.h"


/*
 * Create mappings between dictionary and LM word-IDs.  In short:
 *    - An array of s3lmwid_t entries (map[]) is created; where map[i] is the LM word-ID for
 * 	the dictionary word-ID i.  Mappings are created for the alternative pronunciations as
 * 	well.  For words not in the LM, the corresponding entries are BAD_S3LMWID.
 *    - Similarly, lm->ug[u].dictwid is assigned the dictionary word id for unigram word u.
 * Return value: The map[] array built as described above.
 */
s3lmwid_t *wid_dict_lm_map (dict_t *dict,	/* In: Dictionary */
			    lm_t *lm);		/* In/Out: LM; lm->ug[].dictwid values are
						   updated. */

/*
 * Augment the given wordprob array with alternative pronunciations from the dictionary.
 * Return value: #entries in the augmented wordprob array (including the original ones).
 */
int32 wid_wordprob2alt (dict_t *dict,	/* In: Dictionary */
			wordprob_t *wp,	/* In/Out: Input wordprob array, to be augmented with
					   alternative pronunciations for the entries that
					   already exist in it.  Caller must have allocated
					   this array. */
			int32 n);	/* In: #Input entries in the wordprob array */

#endif
