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


#ifndef _LIBMAIN_DICT_H_
#define _LIBMAIN_DICT_H_


#include "s3types.h"
#include "mdef.h"	/* This is still a sore point */

typedef struct {
    char *word;		/* Ascii word string */
    s3cipid_t *ciphone;	/* Pronunciation */
    int32 pronlen;	/* Pronunciation length */
    s3wid_t alt;	/* Next alternative pronunciation id, NOT_WID if none */
    s3wid_t basewid;	/* Base pronunciation id */
    int32 n_comp;	/* If this is a compound word, no. of component words; else 0.
			   (Compound words are concatenations of other words, such as
			   GOING_TO, whose pronunciation is quite different from the
			   mere concatenation of their components.) */
    s3wid_t *comp;	/* If n_comp > 0, its components */
} dictword_t;

typedef struct {
    mdef_t *mdef;	/* Model definition used for phone IDs; NULL if none used */
    hash_table_t *pht;	/* Used only if CI phones handled internally (mdef == NULL) */
    char **ciphone_str;	/* Used only if CI phones handled internally (mdef == NULL) */
    int32 n_ciphone;	/* Used only if CI phones handled internally (mdef == NULL) */
    dictword_t *word;	/* Array of entries in dictionary */
    hash_table_t *ht;	/* Hash table for mapping word strings to word ids */
    int32 max_words;	/* #Entries allocated in dict, including empty slots */
    int32 n_word;	/* #Occupied entries in dict; ie, excluding empty slots */
    int32 filler_start;	/* First filler word id (read from filler dict) */
    int32 filler_end;	/* Last filler word id (read from filler dict) */
    s3wid_t *comp_head;	/* comp_head[w] = wid of a compound word with 1st component = w;
			   comp_head[comp_head[w]] = next such compound word, and so on,
			   until we hit BAD_WID.  NULL if no compound word in dict. */
    s3wid_t startwid;	/* FOR INTERNAL-USE ONLY */
    s3wid_t finishwid;	/* FOR INTERNAL-USE ONLY */
    s3wid_t silwid;	/* FOR INTERNAL-USE ONLY */
} dict_t;


/*
 * Initialize with given main and filler dictionary files.  fillerfile can be NULL
 * (but external modules might impose their own requirements).
 * Return ptr to dict_t if successful, NULL otherwise.
 */
dict_t *dict_init (mdef_t *mdef,	/* For looking up CI phone IDs; NULL if none,
					   in which case CI phones kept internally */
		   char *dictfile,	/* Main dictionary file */
		   char *fillerfile,	/* Filler dictionary file */
		   char comp_sep);	/* Compound word separator character, or 0 if
					   no compound words */

/* Return word id for given word string if present.  Otherwise return BAD_WID */
s3wid_t dict_wordid (dict_t *d, char *word);

/*
 * Return 1 if w is a filler word, 0 if not.  A filler word is one that was read in from the
 * filler dictionary; however, sentence START and FINISH words are not filler words.
 */
int32 dict_filler_word (dict_t *d, s3wid_t w);

/*
 * Add a word with the given ciphone pronunciation list to the dictionary.
 * Return value: Result word id if successful, BAD_WID otherwise
 */
s3wid_t dict_add_word (dict_t *d, char *word, s3cipid_t *p, int32 np);

/*
 * Look for a compound word that matches the given word-id sequence.
 * Return value: Base ID of compound word if found, else BAD_WID.
 */
s3wid_t dict_wids2compwid (dict_t *d,		/* In: Dictionary to look up */
			   s3wid_t *wid,	/* In: Component words to look for */
			   int32 len);		/* In: No. of component words */

/*
 * Return value: CI phone string for the given word, phone position.
 */
const char *dict_ciphone_str (dict_t *d,	/* In: Dictionary to look up */
			      s3wid_t wid,	/* In: Component word being looked up */
			      int32 pos);	/* In: Pronunciation phone position */

/* Packaged macro access to dictionary members */
#define dict_size(d)		((d)->n_word)
#define dict_basewid(d,w)	((d)->word[w].basewid)
#define dict_wordstr(d,w)	((d)->word[w].word)
#define dict_nextalt(d,w)	((d)->word[w].alt)
#define dict_pronlen(d,w)	((d)->word[w].pronlen)
#define dict_pron(d,w,p)	((d)->word[w].ciphone[p])
#define dict_filler_start(d)	((d)->filler_start)
#define dict_filler_end(d)	((d)->filler_end)
#define dict_startwid(d)	((d)->startwid)
#define dict_finishwid(d)	((d)->finishwid)
#define dict_silwid(d)		((d)->silwid)
#define dict_first_phone(d,w)	((d)->word[w].ciphone[0])
#define dict_last_phone(d,w)	((d)->word[w].ciphone[(d)->word[w].pronlen - 1])


/* Function versions of some of the above macros; note the leading underscore. */

/*
 * Return base word id for given word id w (which may be itself).  w must be valid.
 */
s3wid_t _dict_basewid (dict_t *d, s3wid_t w);

/*
 * Return word string for given word id, which must be valid.
 */
char *_dict_wordstr (dict_t *d, s3wid_t wid);

/*
 * Return the next alternative word id for the given word id, which must be valid.
 * The returned id may be BAD_WID if there is none.
 */
s3wid_t _dict_nextalt (dict_t *d, s3wid_t wid);


#endif
