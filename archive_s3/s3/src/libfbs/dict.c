/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * dict.c -- The word lexicon and pronunciations.
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
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed dict_init to return dict_t *.
 * 
 * 01-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>

#include "s3types.h"
#include "dict.h"
#include "mdef.h"


static dict_t *d = NULL;	/* The dictionary */
static mdef_t *mdef;

static char *delim = " \t\n";


s3wid_t dict_add_word (char *word, s3cipid_t *p, int32 np)
{
    int32 i, w, len;
    dictword_t *wordp;
    s3wid_t newwid;
    
    if (d->n_word >= d->max_words) {
	E_ERROR("Dictionary full; add(%s) failed\n", word);
	return (BAD_WID);
    }
    
    wordp = d->word + d->n_word;
    wordp->word = (char *) ckd_salloc (word);
    
    /* Associate word string with d->n_word in hash table */
    if (hash_enter (d->ht, wordp->word, d->n_word) < 0) {
	ckd_free (wordp->word);
	return (BAD_WID);
    }

    /* Fill in word entry */
    wordp->ciphone = (s3cipid_t *) ckd_malloc (np * sizeof(s3cipid_t));
    memcpy (wordp->ciphone, p, np*sizeof(s3cipid_t));
    wordp->pronlen = np;
    wordp->alt = BAD_WID;		/* The default */
    wordp->basewid = d->n_word;		/* The default */
    
    /* Determine base/alt wids */
    len = strlen(word);
    if (word[len-1] == ')') {
	for (i = len-1; (i > 0) && (word[i] != '('); --i);
	
	if (i > 0) {
	    /* Found a word of the form <baseword>(...); find base word id */
	    word[i] = '\0';
	    if (hash_lookup (d->ht, word, &w) < 0) {
		word[i] = '(';
		E_FATAL("Missing base word for: %s\n", word);
	    }
	    word[i] = '(';
	    
	    /* Link into alt list */
	    wordp->basewid = w;
	    wordp->alt = d->word[w].alt;
	    d->word[w].alt = d->n_word;
	}
    }
    
    newwid = d->n_word++;
    
    return (newwid);
}


static int32 dict_read (FILE *fp)
{
    char line[1024], *lp, *word, *pron, tmp;
    s3cipid_t p[1024];
    int32 np, wlen, plen, lineno;

    lineno = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
        lineno++;
	if (line[0] == '#')	/* Comment line */
	    continue;
	
	if ((wlen = nextword (line, delim, &word, &tmp)) < 0)	/* Empty line */
	    continue;
	lp = word + wlen;
	word[wlen] = tmp;
	
	for (np = 0; (plen = nextword (lp, delim, &pron, &tmp)) > 0; np++) {
	    lp = pron + plen;

	    p[np] = mdef_ciphone_id (mdef, pron);
	    if (NOT_CIPID(p[np])) {
		E_ERROR("Line %d: Bad ciphone: %s\n", lineno, pron);
		np = 0;
		break;
	    }
	    
	    pron[plen] = tmp;		/* Restore original delimiter */
	}
	
	if (np == 0)
	    E_ERROR("Line %d: No pronunciation for %s; ignored\n", lineno, word);
	else {
	    word[wlen] = '\0';
	    if (dict_add_word (word, p, np) < 0)
		E_ERROR("Line %d: dict_add_word (%s) failed; ignored\n",
			lineno, word);
	}
    }

    return 0;
}


dict_t *dict_init (char *dictfile, char *fillerfile)
{
    FILE *fp, *fp2;
    int32 n ;
    char line[1024];

    E_INFO("Reading dictionary\n");
    
    assert (! d);
    mdef = mdef_getmdef ();
    assert (mdef);

    /*
     * First obtain #words in dictionary (for hash table allocation).
     * Reason: The PC NT system doesn't like to grow memory gradually.  Better to allocate
     * all the required memory in one go.
     */
    if ((fp = fopen(dictfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", dictfile);
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] != '#')
	    n++;
    }
    rewind (fp);

    if (fillerfile) {
	if ((fp2 = fopen(fillerfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", fillerfile);

	while (fgets (line, sizeof(line), fp2) != NULL) {
	    if (line[0] != '#')
		n++;
	}
	rewind (fp2);
    }
    
    /*
     * Allocate dict entries.  HACK!!  Allow extra (128) entries for words not in file.
     * (E.g., <sil>, <s>, </s> which might be added explicitly and not in either file.)
     */
    d = (dict_t *) ckd_calloc (1, sizeof(dict_t));
    d->word = (dictword_t *) ckd_calloc (n+128, sizeof(dictword_t));
    d->max_words = n + 128;
    d->n_word = 0;
    
    /* Obtain hash table of desired minimum size */
    d->ht = hash_new ("dict", d->max_words);

    /* Digest main dictionary file */
    E_INFO("Reading main dictionary: %s\n", dictfile);
    dict_read (fp);
    fclose (fp);
    E_INFO("%d words read\n", d->n_word);

    /* Now the filler dictionary file, if it exists */
    d->filler_start = d->n_word;
    if (fillerfile) {
        E_INFO("Reading filler dictionary: %s\n", fillerfile);
	dict_read (fp2);
	fclose (fp2);
	E_INFO("%d words read\n", d->n_word - d->filler_start);
    }
    d->filler_end = d->n_word-1;

    return d;
}


dict_t *dict_getdict ( void )
{
    assert (d);

    return d;
}


s3wid_t dict_basewid (s3wid_t w)
{
    assert (d);
    assert ((w >= 0) && (w < d->n_word));
    
    return (d->word[w].basewid);
}


s3wid_t dict_wordid (char *word)
{
    int32 w;
    
    assert (d);
    assert (word);
    
    if (hash_lookup (d->ht, word, &w) < 0)
	return (BAD_WID);
    return ((s3wid_t) w);
}


char *dict_wordstr (s3wid_t wid)
{
    assert (d);
    assert (IS_WID(wid) && (wid < d->n_word));
    
    return (d->word[wid].word);
}


int32 dict_size ( void )
{
    assert (d);
    
    return (d->n_word);
}


int32 dict_filler_word (s3wid_t w)
{
    assert (d);
    assert ((w >= 0) && (w < d->n_word));
    
    return ((w >= d->filler_start) && (w <= d->filler_end)) ? TRUE : FALSE;
}
