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
 * dict.h -- Pronunciation dictionary structures
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.10  2006/02/22 20:55:06  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH:
 *
 * 1, Added Letter-to-sound LTS rule, dict_init will only specify
 * d->lts_rules to be true if the useLTS is specified.  Only if
 * d->lts_rules is specified, the LTS logic will be used. The code safe
 * guarded the case when a phone in mdef doesn't appear in LTS, in that
 * case, the code will force exit.
 *
 * 2, The LTS logic is only used as a reserved measure.  By default, it
 * is not turned on.  See also the comment in kbcore.c and the default
 * parameters in revision 1.3 cmdln_macro.h . We added it because we have
 * this functionality in SphinxTrain.
 *
 * Revision 1.9.4.4  2005/10/07 18:58:04  arthchan2003
 * Added macro for getting second last phone for a word.
 *
 * Revision 1.9.4.3  2005/09/25 19:12:09  arthchan2003
 * Added optional LTS support for the dictionary.
 *
 * Revision 1.9.4.2  2005/09/18 01:15:45  arthchan2003
 * Add one doxy-doc in dict.h
 *
 * Revision 1.9.4.1  2005/07/05 06:55:26  arthchan2003
 * Fixed dox-doc.
 *
 * Revision 1.9  2005/06/21 21:04:36  arthchan2003
 * 1, Introduced a reporting routine. 2, Fixed doyxgen documentation, 3, Added  keyword.
 *
 * Revision 1.5  2005/06/13 04:02:57  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 19-Apr-01    Ricky Houghton, added code for freeing memory that is allocated internally.
 * 
 * 23-Apr-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Made usage of mdef optional.  If no mdef is specified while loading
 *		a dictionary, it maintains the needed CI phone information internally.
 * 		Added dict_ciphone_str().
 * 
 * 02-Jul-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added startwid, finishwid, silwid to dict_t structure.
 * 
 * 07-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created from previous Sphinx-3 version.
 */


#ifndef _S3_DICT_H_
#define _S3_DICT_H_

/** \file dict.h
 * \brief Operations on dictionary. 
 */
#include <hash_table.h>
#include <s3types.h>
#include "mdef.h"	/* This is still a sore point; dict should be independent of mdef */
#include "lts.h" 

#define DICT_INC_SZ 4096
#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** 
    \struct dictword_t
    \brief a structure for one dictionary word. 
*/
typedef struct {
    char *word;		/**< Ascii word string */
    s3cipid_t *ciphone;	/**< Pronunciation */
    int32 pronlen;	/**< Pronunciation length */
    s3wid_t alt;	/**< Next alternative pronunciation id, NOT_S3WID if none */
    s3wid_t basewid;	/**< Base pronunciation id */
    int32 n_comp;	/**< If this is a compound word, no. of component words; else 0.
			   (Compound words are concatenations of other words, such as
			   GOING_TO, whose pronunciation is quite different from the
			   mere concatenation of their components.) */
    s3wid_t *comp;	/**< If n_comp > 0, its components */
} dictword_t;

/** 
    \struct dict_t
    \brief a structure for a dictionary. 
*/

typedef struct {
    mdef_t *mdef;	/**< Model definition used for phone IDs; NULL if none used */
    hash_table_t *pht;	/**< Used only if CI phones handled internally (mdef == NULL) */
    char **ciphone_str;	/**< Used only if CI phones handled internally (mdef == NULL) */
    int32 n_ciphone;	/**< Used only if CI phones handled internally (mdef == NULL) */
    dictword_t *word;	/**< Array of entries in dictionary */
    hash_table_t *ht;	/**< Hash table for mapping word strings to word ids */
    int32 max_words;	/**< #Entries allocated in dict, including empty slots */
    int32 n_word;	/**< #Occupied entries in dict; ie, excluding empty slots */
    int32 filler_start;	/**< First filler word id (read from filler dict) */
    int32 filler_end;	/**< Last filler word id (read from filler dict) */
    s3wid_t *comp_head;	/**< comp_head[w] = wid of a compound word with 1st component = w;
                           comp_head[comp_head[w]] = next such compound word, and so on,
                           until we hit BAD_S3WID.  NULL if no compound word in dict. */
    s3wid_t startwid;	/**< FOR INTERNAL-USE ONLY */
    s3wid_t finishwid;	/**< FOR INTERNAL-USE ONLY */
    s3wid_t silwid;	/**< FOR INTERNAL-USE ONLY */
  
    lts_t *lts_rules;     /**< The LTS rules */
} dict_t;


/**
 * Initialize with given main and filler dictionary files.  fillerfile can be NULL
 * (but external modules might impose their own requirements).
 * Return ptr to dict_t if successful, NULL otherwise.
 */
S3DECODER_EXPORT
dict_t *dict_init (mdef_t *mdef,	/**< For looking up CI phone IDs; NULL if none,
					   in which case CI phones kept internally */
		   const char *dictfile,	/**< Main dictionary file */
		   const char *fillerfile,	/**< Filler dictionary file */
		   const char comp_sep,	/**< Compound word separator character, or 0 if
					   no compound words */
		   int useLTS,          /**< Whether to use letter-to-sound rules */
		   int breport          /**< Whether we should report the progress */
    );

/** Return word id for given word string if present.  Otherwise return BAD_S3WID */
S3DECODER_EXPORT
s3wid_t dict_wordid (dict_t *d, const char *word);

/**
 * Return 1 if w is a filler word, 0 if not.  A filler word is one that was read in from the
 * filler dictionary; however, sentence START and FINISH words are not filler words.
 */
S3DECODER_EXPORT
int32 dict_filler_word (dict_t *d,  /**< The dictionary structure */
			s3wid_t w     /**< The The word */
    );

/**
 * Add a word with the given ciphone pronunciation list to the dictionary.
 * Return value: Result word id if successful, BAD_S3WID otherwise
 */
s3wid_t dict_add_word (dict_t *d,  /**< The dictionary structure */
		       char *word, /**< The word */
		       s3cipid_t *p, 
		       int32 np
    );

/**
 * Look for a compound word that matches the given word-id sequence.
 * Return value: Base ID of compound word if found, else BAD_S3WID.
 */
s3wid_t dict_wids2compwid (dict_t *d,		/**< In: Dictionary to look up */
			   s3wid_t *wid,	/**< In: Component words to look for */
			   int32 len		/**< In: No. of component words */
    );

/**
 * Return value: CI phone string for the given word, phone position.
 */
const char *dict_ciphone_str (dict_t *d,	/**< In: Dictionary to look up */
			      s3wid_t wid,	/**< In: Component word being looked up */
			      int32 pos   	/**< In: Pronunciation phone position */
    );

/** Packaged macro access to dictionary members */
#define dict_size(d)		((d)->n_word)
#define dict_basewid(d,w)	((d)->word[w].basewid)
#define dict_wordstr(d,w)	((d)->word[w].word)
#define dict_nextalt(d,w)	((d)->word[w].alt)
#define dict_pronlen(d,w)	((d)->word[w].pronlen) 
#define dict_pron(d,w,p)	((d)->word[w].ciphone[p]) /**< The CI phones of the word w at position p */
#define dict_filler_start(d)	((d)->filler_start)
#define dict_filler_end(d)	((d)->filler_end)
#define dict_startwid(d)	((d)->startwid)
#define dict_finishwid(d)	((d)->finishwid)
#define dict_silwid(d)		((d)->silwid)
#define dict_first_phone(d,w)	((d)->word[w].ciphone[0])
#define dict_second_last_phone(d,w)	((d)->word[w].ciphone[(d)->word[w].pronlen - 2])
#define dict_last_phone(d,w)	((d)->word[w].ciphone[(d)->word[w].pronlen - 1])

/* Hard-coded special words */
#define S3_START_WORD		"<s>"
#define S3_FINISH_WORD		"</s>"
#define S3_SILENCE_WORD		"<sil>"
#define S3_UNKNOWN_WORD		"<UNK>"

/* Function versions of some of the above macros; note the leading underscore. */

/**
 * Return base word id for given word id w (which may be itself).  w must be valid.
 */
s3wid_t _dict_basewid (dict_t *d, s3wid_t w);

/**
 * Return word string for given word id, which must be valid.
 */
char *_dict_wordstr (dict_t *d, s3wid_t wid);

/**
 * Return the next alternative word id for the given word id, which must be valid.
 * The returned id may be BAD_S3WID if there is none.
 */
s3wid_t _dict_nextalt (dict_t *d, s3wid_t wid);

/**
 * If the given word contains a trailing "(....)" (i.e., a Sphinx-II style alternative
 * pronunciation specification), strip that trailing portion from it.  Note that the given
 * string is modified.
 * Return value: If string was modified, the character position at which the original string
 * was truncated; otherwise -1.
 */
int32 dict_word2basestr (char *word);

/* RAH, free memory allocated for the dictionary */
/** Free memory allocated for the dictionary */
S3DECODER_EXPORT
void dict_free (dict_t *d);

/** Report a diciontary structure */
void dict_report(dict_t *d /**< A dictionary structure */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
