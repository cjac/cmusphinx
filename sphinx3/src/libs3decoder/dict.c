/*
 * dict.c -- Pronunciation dictionary.
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
 * 		Added startwid, finishwid, silwid to dict_t.  Modified dict_filler_word
 * 		to check for start and finishwid.
 * 
 * 07-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created from previous Sphinx-3 version.
 */


#include "dict.h"


#define DELIM	" \t\n"		/* Set of field separator characters */
#define DEFAULT_NUM_PHONE	(MAX_S3CIPID+1)


static s3cipid_t dict_ciphone_id (dict_t *d, char *str)
{
    int32 id;
    
    if (d->mdef)
	return mdef_ciphone_id (d->mdef, str);
    else {
	if (hash_lookup (d->pht, str, &id) < 0) {
	    id = (d->n_ciphone)++;
	    
	    if (id >= MAX_S3CIPID)
		E_FATAL("Too many CIphones in dictionary; increase MAX_S3CIPID\n");
	    d->ciphone_str[id] = (char *) ckd_salloc(str);
	    
	    if (hash_enter (d->pht, d->ciphone_str[id], id) != id)
		E_FATAL("hash_enter(local-phonetable, %s) failed\n", str);
	}
	return id;
    }
}


const char *dict_ciphone_str (dict_t *d, s3wid_t wid, int32 pos)
{
    assert (d != NULL);
    assert ((wid >= 0) && (wid < d->n_word));
    assert ((pos >= 0) && (pos < d->word[wid].pronlen));
    
    if (d->mdef)
	return mdef_ciphone_str (d->mdef, d->word[wid].ciphone[pos]);
    else
	return (d->ciphone_str[(int)d->word[wid].ciphone[pos]]);
}


s3wid_t dict_add_word (dict_t *d, char *word, s3cipid_t *p, int32 np)
{
    int32 w, len;
    dictword_t *wordp;
    s3wid_t newwid;
    
    if (d->n_word >= d->max_words) {
	E_ERROR("Dictionary full; add(%s) failed\n", word);
	return (BAD_S3WID);
    }
    
    wordp = d->word + d->n_word;
    wordp->word = (char *) ckd_salloc (word);
    
    /* Associate word string with d->n_word in hash table */
    if (hash_enter (d->ht, wordp->word, d->n_word) != d->n_word) {
	ckd_free (wordp->word);
	return (BAD_S3WID);
    }

    /* Fill in word entry, and set defaults */
    if (p && (np > 0)) {
	wordp->ciphone = (s3cipid_t *) ckd_malloc (np * sizeof(s3cipid_t));
	memcpy (wordp->ciphone, p, np*sizeof(s3cipid_t));
	wordp->pronlen = np;
    } else {
	wordp->ciphone = NULL;
	wordp->pronlen = 0;
    }
    wordp->alt = BAD_S3WID;
    wordp->basewid = d->n_word;
    wordp->n_comp = 0;
    wordp->comp = NULL;
    
    /* Determine base/alt wids */
    if ((len = dict_word2basestr (word)) > 0) {
	/* Truncated to a baseword string; find its ID */
	if (hash_lookup (d->ht, word, &w) < 0) {
	    word[len] = '(';	/* Get back the original word */
	    E_FATAL("Missing base word for: %s\n", word);
	} else
	    word[len] = '(';	/* Get back the original word */
	
	/* Link into alt list */
	wordp->basewid = w;
	wordp->alt = d->word[w].alt;
	d->word[w].alt = d->n_word;
    }
    
    newwid = d->n_word++;
    
    return (newwid);
}


static int32 dict_read (FILE *fp, dict_t *d)
{
    char line[16384], **wptr;
    s3cipid_t p[4096];
    int32 lineno, nwd;
    s3wid_t w;
    int32 i, maxwd;
    
    maxwd = 4092;
    wptr = (char **) ckd_calloc (maxwd, sizeof(char *));
    
    lineno = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
        lineno++;
	if (line[0] == '#')	/* Comment line */
	    continue;
	
	if ((nwd = str2words (line, wptr, maxwd)) < 0)
	    E_FATAL("str2words(%s) failed; Increase maxwd from %d\n", line, maxwd);

	if (nwd == 0)	    /* Empty line */
	    continue;
	/* wptr[0] is the word-string and wptr[1..nwd-1] the pronunciation sequence */
	if (nwd == 1) {
	    E_ERROR("Line %d: No pronunciation for word %s; ignored\n", lineno, wptr[0]);
	    continue;
	}
	
	/* Convert pronunciation string to CI-phone-ids */
	for (i = 1; i < nwd; i++) {
	    p[i-1] = dict_ciphone_id (d, wptr[i]);
	    if (NOT_S3CIPID(p[i-1])) {
		E_ERROR("Line %d: Bad ciphone: %s; word %s ignored\n",
			lineno, wptr[i], wptr[0]);
		break;
	    }
	}
	
	if (i == nwd) {	/* All CI-phones successfully converted to IDs */
	    w = dict_add_word (d, wptr[0], p, nwd-1);
	    if (NOT_S3WID(w))
		E_ERROR("Line %d: dict_add_word (%s) failed (duplicate?); ignored\n",
			lineno, wptr[0]);
	}
    }

    ckd_free (wptr);
    
    return 0;
}


static s3wid_t *dict_comp_head (dict_t *d)
{
    int32 w;
    s3wid_t *comp_head;
    
    comp_head = (s3wid_t *) ckd_calloc (d->n_word, sizeof(s3wid_t));
    
    for (w = 0; w < d->n_word; w++)
	comp_head[w] = BAD_S3WID;
    for (w = 0; w < d->n_word; w++) {
	if (d->word[w].n_comp > 0) {
	    comp_head[w] = comp_head[d->word[w].comp[0]];
	    comp_head[d->word[w].comp[0]] = w;
	}
    }

    return comp_head;
}


/*
 * Scan the dictionary for compound words.  This function should be called just after
 * loading the dictionary.  For the moment, compound words in a compound word are
 * assumed to be separated by the given sep character, (underscore in the CMU dict).
 * Return value: #compound words found in dictionary.
 */
static int32 dict_build_comp (dict_t *d,
			      char sep)		/* Separator character */
{
    char wd[4096];
    int32 w, cwid;
    dictword_t *wordp;
    int32 nc;		/* # compound words in dictionary */
    int32 i, j, l, n;
    
    nc = 0;
    for (w = 0; w < d->n_word; w++) {
	wordp = d->word + dict_basewid(d, w);
	strcpy (wd, wordp->word);
	l = strlen(wd);
	if ((wd[0] == sep) || (wd[l-1] == sep))
	    E_FATAL("Bad compound word %s: leading or trailing separator\n", wordp->word);
	
	/* Count no. of components in this word */
	n = 1;
	for (i = 1; i < l-1; i++)	/* 0 and l-1 already checked above */
	    if (wd[i] == sep)
		n++;
	if (n == 1)
	    continue;		/* Not a compound word */
	nc++;
	
	if ((w == d->startwid) || (w == d->finishwid) || dict_filler_word (d, w))
	    E_FATAL("Compound special/filler word (%s) not allowed\n", wordp->word);
	
	/* Allocate and fill in component word info */
	wordp->n_comp = n;
	wordp->comp = (s3wid_t *) ckd_calloc (n, sizeof(s3wid_t));
	
	/* Parse word string into components */
	n = 0;
	for (i = 0; i < l; i++) {
	    for (j = i; (i < l) && (wd[i] != sep); i++);
	    if (j == i)
		E_FATAL("Bad compound word %s: successive separators\n", wordp->word);
	    
	    wd[i] = '\0';
	    cwid = dict_wordid (d, wd+j);
	    if (NOT_S3WID(cwid))
		E_FATAL("Component word %s of %s not in dictionary\n", wd+j, wordp->word);
	    wordp->comp[n] = cwid;
	    n++;
	}
    }
    
    if (nc > 0)
	d->comp_head = dict_comp_head (d);
    
    return nc;
}


dict_t *dict_init (mdef_t *mdef, char *dictfile, char *fillerfile, char comp_sep)
{
    FILE *fp, *fp2;
    int32 n ;
    char line[1024];
    dict_t *d;
    
    if (! dictfile)
	E_FATAL("No dictionary file\n");

    /*
     * First obtain #words in dictionary (for hash table allocation).
     * Reason: The PC NT system doesn't like to grow memory gradually.  Better to allocate
     * all the required memory in one go.
     */
    if ((fp = fopen(dictfile, "r")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,r) failed\n", dictfile);
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] != '#')
	    n++;
    }
    rewind (fp);

    fp2 = NULL;
    if (fillerfile) {
	if ((fp2 = fopen(fillerfile, "r")) == NULL)
	    E_FATAL_SYSTEM("fopen(%s,r) failed\n", fillerfile);

	while (fgets (line, sizeof(line), fp2) != NULL) {
	    if (line[0] != '#')
		n++;
	}
	rewind (fp2);
    }
    
    /*
     * Allocate dict entries.  HACK!!  Allow some extra entries for words not in file.
     * Also check for type size restrictions.
     */
    d = (dict_t *) ckd_calloc (1, sizeof(dict_t));
    d->max_words = (n+1024 < MAX_S3WID) ? n+1024 : MAX_S3WID;
    if (n >= MAX_S3WID)
	E_FATAL("#Words in dictionaries (%d) exceeds limit (%d)\n", n, MAX_S3WID);
    
    d->word = (dictword_t *) ckd_calloc (d->max_words, sizeof(dictword_t));
    d->n_word = 0;
    d->mdef = mdef;
    if (mdef) {
	d->pht = NULL;
	d->ciphone_str = NULL;
    } else {
	d->pht = hash_new (DEFAULT_NUM_PHONE, 1 /* No case */);
	d->ciphone_str = (char **) ckd_calloc (DEFAULT_NUM_PHONE, sizeof(char *));
    }
    d->n_ciphone = 0;
    
    /* Create new hash table for word strings; case-insensitive word strings */
    d->ht = hash_new (d->max_words, 1 /* no-case */);

    /* Initialize with no compound words */
    d->comp_head = NULL;
    
    /* Digest main dictionary file */
    E_INFO("Reading main dictionary: %s\n", dictfile);
    dict_read (fp, d);
    fclose (fp);
    E_INFO("%d words read\n", d->n_word);

    /* Now the filler dictionary file, if it exists */
    d->filler_start = d->n_word;
    if (fillerfile) {
        E_INFO("Reading filler dictionary: %s\n", fillerfile);
	dict_read (fp2, d);
	fclose (fp2);
	E_INFO("%d words read\n", d->n_word - d->filler_start);
    }
    d->filler_end = d->n_word-1;

    /* Initialize distinguished word-ids */
    d->startwid = dict_wordid (d, S3_START_WORD);
    d->finishwid = dict_wordid (d, S3_FINISH_WORD);
    d->silwid = dict_wordid (d, S3_SILENCE_WORD);
#if 0
    if (NOT_S3WID(d->startwid))
	E_WARN("%s not in dictionary\n", S3_START_WORD);
    if (NOT_S3WID(d->finishwid))
	E_WARN("%s not in dictionary\n", S3_FINISH_WORD);
    if (NOT_S3WID(d->silwid))
	E_WARN("%s not in dictionary\n", S3_SILENCE_WORD);
#endif

    /* Identify compound words if indicated */
    if (comp_sep) {
	E_INFO("Building compound words (separator = '%c')\n", comp_sep);
	n = dict_build_comp (d, comp_sep);
	E_INFO("%d compound words\n", n);
    }
    
    return d;
}


s3wid_t dict_wordid (dict_t *d, char *word)
{
    int32 w;
    
    assert (d);
    assert (word);
    
    if (hash_lookup (d->ht, word, &w) < 0)
	return (BAD_S3WID);
    return ((s3wid_t) w);
}


s3wid_t _dict_basewid (dict_t *d, s3wid_t w)
{
    assert (d);
    assert ((w >= 0) && (w < d->n_word));
    
    return (d->word[w].basewid);
}


char *_dict_wordstr (dict_t *d, s3wid_t wid)
{
    assert (d);
    assert (IS_S3WID(wid) && (wid < d->n_word));
    
    return (d->word[wid].word);
}


s3wid_t _dict_nextalt (dict_t *d, s3wid_t wid)
{
    assert (d);
    assert (IS_S3WID(wid) && (wid < d->n_word));
    
    return (d->word[wid].alt);
}


int32 dict_filler_word (dict_t *d, s3wid_t w)
{
    assert (d);
    assert ((w >= 0) && (w < d->n_word));
    
    w = dict_basewid(d, w);
    if ((w == d->startwid) || (w == d->finishwid))
	return 0;
    if ((w >= d->filler_start) && (w <= d->filler_end))
	return 1;
    return 0;
}


s3wid_t dict_wids2compwid (dict_t *d, s3wid_t *wid, int32 len)
{
    s3wid_t w;
    int32 i;
    
    if (! d->comp_head)
	return BAD_S3WID;
    
    assert (len > 1);
    
    for (w = d->comp_head[wid[0]]; IS_S3WID(w); w = d->comp_head[w]) {
	/* w is a compound word beginning with wid[0]; check if rest matches */
	assert (d->word[w].n_comp > 1);
	assert (d->word[w].comp[0] == wid[0]);
	
	if (d->word[w].n_comp == len) {
	    for (i = 0; (i < len) && (d->word[w].comp[i] == wid[i]); i++);
	    if (i == len)
		return (dict_basewid(d, w));
	}
    }

    return BAD_S3WID;
}


int32 dict_word2basestr (char *word)
{
    int32 i, len;
    
    len = strlen(word);
    if (word[len-1] == ')') {
	for (i = len-2; (i > 0) && (word[i] != '('); --i);
	
	if (i > 0) {
	    /* The word is of the form <baseword>(...); strip from left-paren */
	    word[i] = '\0';
	    return i;
	}
    }

    return -1;
}


#if (_DICT_TEST_)
main (int32 argc, char *argv[])
{
    mdef_t *m;
    dict_t *d;
    char wd[1024];
    s3wid_t wid;
    int32 p;
    
    if (argc < 3)
	E_FATAL("Usage: %s {mdeffile | NULL} dict [fillerdict]\n", argv[0]);
    
    m = (strcmp (argv[1], "NULL") != 0) ? mdef_init (argv[1]) : NULL;
    d = dict_init (m, argv[2], ((argc > 3) ? argv[3] : NULL), '_');
    
    for (;;) {
	printf ("word> ");
	scanf ("%s", wd);
	
	wid = dict_wordid (d, wd);
	if (NOT_S3WID(wid))
	    E_ERROR("Unknown word\n");
	else {
	    for (wid = dict_basewid(d, wid); IS_S3WID(wid); wid = d->word[wid].alt) {
		printf ("%s\t", dict_wordstr(d, wid));
		for (p = 0; p < d->word[wid].pronlen; p++)
		    printf (" %s", dict_ciphone_str (d, wid, p));
		printf ("\n");
	    }
	}
    }
}
#endif
