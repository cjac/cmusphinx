/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
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
 * $Log: wid.h,v $
 * Revision 1.9  2005/06/21 21:05:49  arthchan2003
 * 1, Fixed doxygen documentation, 2, Added  keyword.
 *
 * Revision 1.4  2005/06/13 04:02:58  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#ifndef _S3_WID_H_
#define _S3_WID_H_

#include <s3types.h>
#include "dict.h"
#include "lm.h"

/** \file wid.h
    \brief coversion of dictionary ID to LM ID. 
*/
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif
  

/**
 * Create mappings between dictionary and LM word-IDs.  In short:
 *    - An array of s3lmwid32_t entries (map[]) is created; where map[i] is the LM word-ID for
 * 	the dictionary word-ID i.  Mappings are created for the alternative pronunciations as
 * 	well.  For words not in the LM, the corresponding entries are BAD_LMWID(lm).
 *    - Similarly, lm->ug[u].dictwid is assigned the dictionary word id for unigram word u.
 * Return value: The map[] array built as described above.
 */

s3lmwid32_t *wid_dict_lm_map (dict_t *dict,	/**< In: Dictionary */
			      lm_t *lm,           /**< In/Out: LM; lm->ug[].dictwid values are
						     updated. */
			      int32 lw            /**< In: Language Weight */
    );		

/**
 * Augment the given wordprob array with alternative pronunciations from the dictionary.
 * Return value: #entries in the augmented wordprob array (including the original ones).
 */
int32 wid_wordprob2alt (dict_t *dict,	/**< In: Dictionary */
			wordprob_t *wp,	/**< In/Out: Input wordprob array, to be augmented with
					   alternative pronunciations for the entries that
					   already exist in it.  Caller must have allocated
					   this array. */
			int32 n	/**< In: #Input entries in the wordprob array */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
