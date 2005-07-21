/* $Header$
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * lm_3g.c -- Darpa Trigram LM module.
 *
 * HISTORY
 * 
 * 12-Feb-2000	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Adapted from original version, for standalone compilation and running.
 * 
 * 03-Apr-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added lm3g_raw_score() and lm_t.invlw.
 * 		Changed a number of function names from lm_... to lm3g_...
 * 
 * 09-Jan-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		BUGFIX: Added check for lmp->unigrams[i].wid in lm_set_current().
 * 
 * 06-Dec-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Changed function name lmname_to_lm() to lm_name2lm().
 * 
 * 06-Dec-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Changed function name get_current_lm to lm_get_current.
 * 		Changed check for already existing word in lm_add_word, and added
 * 		condition to updating dictwid_map.
 * 
 * 01-Jul-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Removed LM cache and replaced with find_bg and find_tg within the main
 * 		bigrams and trigram structures.  No loss of speed and uses less memory.
 * 
 * 24-Jun-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Fixed a number of memory leaks while deleting an LM.  Added the global
 * 		dictwid_map, and allocated it once and never freed.  Made sure lm_cache
 * 		is created only once.
 * 
 * 14-Jun-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Modified lm_read to return 0 on success, and to delete any existing LM
 * 		with the new LM name (instead of reporting error and exiting).
 * 		Added backslash option in building filenames (for PC compatibility).
 * 
 * $Log$
 * Revision 1.2  2005/07/21  19:42:47  egouvea
 * Cleaned up code so it compiles with MS Visual C++. Added MS VC++
 * support files (.dsp and .dsw).
 * 
 * Revision 1.1  2002/11/11 17:42:40  egouvea
 * Initial import of lm3g2dmp from Ravi's files.
 *
 * Revision 1.1.1.1  2000/02/28 18:34:43  rkm
 * Imported Sources
 *
 * Revision 8.9  94/10/11  12:36:28  rkm
 * Changed lm_tg_score to call lm_bg_score if no trigrams present or
 * the first word is invalid.
 * 
 * Revision 8.8  94/07/29  11:54:23  rkm
 * Renamed lmSetParameters to lm_set_param and moved it into lm_add().
 * Added functions lm_init_oov() to create initial list of OOVs,
 * and lm_add_word() to add new OOV at run time.
 * 
 * Revision 8.7  94/05/19  14:19:59  rkm
 * Rewrote LM cache code for greater efficiency.
 * 
 * Revision 8.6  94/05/10  10:47:58  rkm
 * Added lm_add() and lm_set_param() functions, for dynamically adding a new
 * in-memory LM to the set of available LMs.
 * 
 * Revision 8.5  94/04/22  13:53:27  rkm
 * Added query_lm_cache_lines() to allow run-time spec of #cache lines.
 * 
 * Revision 8.4  94/04/14  15:08:46  rkm
 * Added function lm_delete() to delete a named LM and reclaim space.
 * 
 * Revision 8.3  94/04/14  14:40:27  rkm
 * Minor changes.
 * 
 * Revision 8.1  94/02/15  15:09:22  rkm
 * Derived from v7.  Includes multiple LMs for grammar switching.
 * 
 * Revision 6.13  94/02/11  13:14:45  rkm
 * Added bigram and trigram multi-line caches, and functions, for v7.
 * Replaced sequential search in wstr2wid() with hash_lookup().
 * 
 * Revision 6.12  94/01/07  10:56:16  rkm
 * Corrected bug relating to input file format.
 * 
 * Revision 6.11  93/12/17  13:14:52  rkm
 * *** empty log message ***
 * 
 * Revision 6.10  93/12/03  17:09:59  rkm
 * Added ability to handle bigram-only dump files.
 * Added <s> </s> bigram -> MIN_PROB.
 * Added timestamp to dump files.
 * 
 * Revision 6.9  93/12/01  12:29:55  rkm
 * Added ability to handle LM files containing only bigrams.
 * Excluded start_sym from interpolation of unigram prob with uniform prob.
 * 
 * 
 * 93/10/21 rkm@cs.cmu.edu
 * Added <c.h>
 * 
 * Revision 6.6  93/10/19  18:58:10  rkm
 * Added code to change bigram-prob(<s>,<s>) to very low value.  The
 * Darpa LM file contains a spurious value to be ignored.
 * Fixed bug that dumps one trigram entry too many.
 * 
 * Revision 6.5  93/10/15  15:00:14  rkm
 * 
 * Revision 6.4  93/10/13  16:56:04  rkm
 * Added LM cache line stats.
 * Added bg_only option for lm_read parameter list
 * (but not yet implemented).
 * Changed proc name ilm_LOG_prob_of to lm3g_prob, to avoid conflict
 * with Roni's ILM function of the same name.
 * 
 * Revision 6.3  93/10/09  17:01:55  rkm
 * M K Ravishankar (rkm@cs) at Carnegie Mellon
 * Cleaned up handling precompiled binary 3g LM file,
 * Added SWAP option for HP platforms.
 * 
 * Revision 6.2  93/10/06  11:08:15  rkm
 * M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * Darpa Trigram LM module.  Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "primtype.h"
#include "hash.h"
#include "lm_3g.h"
#include "CM_macros.h"
#include "err.h"


#define NO_WORD	-1

static char *dumpdir;

static char   *start_sym = "<s>";
static char   *end_sym = "</s>";
static char   *darpa_hdr = "Darpa Trigram LM";

/* The currently active LM */
static lm_t *lmp;

/* Run-time determined versions of compile-time constants BG_SEG_SZ and LOG_BG_SEG_SZ */
static int32 bg_seg_sz;
static int32 log_bg_seg_sz;

/* HACK!! Re-define these macros from lm_3g.h to make them run-time constants */
#undef TSEG_BASE
#undef FIRST_TG
#undef LAST_TG
#define TSEG_BASE(m,b)		((m)->tseg_base[(b)>>log_bg_seg_sz])
#define FIRST_TG(m,b)		(TSEG_BASE((m),(b))+((m)->bigrams[b].trigrams))
#define LAST_TG(m,b)		(FIRST_TG((m),(b)+1)-1)

/* Single dictionary->LM wid map for the currently active LM */
static int32 *dictwid_map = NULL;

static int32 lm3g_dump ();

extern char *salloc();
extern char *listelem_alloc ();
extern void  listelem_free ();

/* Words in LM; used only for building internal LM from LM file */
static char **word_str;

#define MIN_PROB_F		-99.0

#define MAX_SORTED_ENTRIES	65534

/*
 * Bigram probs and bo-wts, and trigram probs are kept in separate tables
 * rather than within the bigram_t and trigram_t structures.  These tables
 * hold unique prob and bo-wt values, and can be < 64K long (see lm_3g.h).
 * The following tree structure is used to construct these tables of unique
 * values.  Whenever a new value is read from the LM file, the sorted tree
 * structure is searched to see if the value already exists, and inserted
 * if not found.
 */
typedef struct sorted_entry_s {
    log_t val;		/* value being kept in this node */
    u_int16 lower;	/* index of another entry.  All descendants down
			   this path have their val < this node's val.
			   0 => no son exists (0 is root index) */
    u_int16 higher;	/* index of another entry.  All descendants down
			   this path have their val > this node's val
			   0 => no son exists (0 is root index) */
} sorted_entry_t;

/*
 * The sorted list.  list is a (64K long) array.  The first entry is the
 * root of the tree and is created during initialization.
 */
typedef struct {
    sorted_entry_t *list;
    int32 free;		/* first free element in list */
} sorted_list_t;

/* Arrays of unique bigram probs and bo-wts, and trigram probs */
static sorted_list_t sorted_prob2;
static sorted_list_t sorted_bo_wt2;
static sorted_list_t sorted_prob3;

/*
 * Initialize sorted list with the 0-th entry = MIN_PROB_F, which may be needed
 * to replace spurious values in the Darpa LM file.
 */
static void init_sorted_list (l)
    sorted_list_t *l;
{
    l->list =
	(sorted_entry_t *) CM_calloc (MAX_SORTED_ENTRIES, sizeof (sorted_entry_t));
    l->list[0].val.f = MIN_PROB_F;
    l->list[0].lower = 0;
    l->list[0].higher = 0;
    l->free = 1;
}

static void free_sorted_list (l)
    sorted_list_t *l;
{
    free (l->list);
}

static log_t *vals_in_sorted_list (l)
    sorted_list_t *l;
{
    log_t *vals;
    int32 i;
    
    vals = (log_t *) CM_calloc (l->free, sizeof (log_t));
    for (i = 0; i < l->free; i++)
	vals[i].f = l->list[i].val.f;
    return (vals);
}

static int32 sorted_id (l, val)
    sorted_list_t *l;
    float *val;
{
    int32 i = 0;
    
    for (;;) {
	if (*val == l->list[i].val.f)
	    return (i);
	if (*val < l->list[i].val.f) {
	    if (l->list[i].lower == 0) {
		if (l->free >= MAX_SORTED_ENTRIES)
		    E_FATAL("sorted list overflow\n");
		l->list[i].lower = l->free;
		(l->free)++;
		i = l->list[i].lower;
		l->list[i].val.f = *val;
		return (i);
	    } else
		i = l->list[i].lower;
	} else {
	    if (l->list[i].higher == 0) {
		if (l->free >= MAX_SORTED_ENTRIES)
		    E_FATAL("sorted list overflow\n");
		l->list[i].higher = l->free;
		(l->free)++;
		i = l->list[i].higher;
		l->list[i].val.f = *val;
		return (i);
	    } else
		i = l->list[i].higher;
	}
    }
}


/*
 * allocate, initialize and return pointer to an array of unigram entries.
 */
static unigram_t *NewUnigramTable (n_ug)
    int32 n_ug;
{
    unigram_t *table;
    int32 i;

    table = (unigram_t *) CM_calloc (n_ug, sizeof (unigram_t));
    for (i = 0; i < n_ug; i++) {
	table[i].wid = NO_WORD;
	table[i].prob1.f = -99.0;
	table[i].bo_wt1.f = -99.0;
    }
    return table;
}

/*
 * returns a pointer to a new language model record.  The size is passed in
 * as a parameter.
 */
lm_t *
NewModel (n_ug, n_bg, n_tg, n_dict)
    int32 n_ug;
    int32 n_bg;
    int32 n_tg;
    int32 n_dict;
{
    lm_t *model;
    int32 i;

    model = (lm_t *) CM_calloc (1, sizeof (lm_t));

    /*
     * Allocate one extra unigram and bigram entry: sentinels to terminate
     * followers (bigrams and trigrams, respectively) of previous entry.
     */
    model->unigrams	= NewUnigramTable (n_ug+1);
    model->bigrams	= (bigram_t *) CM_calloc (n_bg+1, sizeof (bigram_t));
    if (n_tg > 0)
	model->trigrams	= (trigram_t *) CM_calloc (n_tg, sizeof (trigram_t));

    /* Allocate dictwid_map only once; shared among all LMs; valid only for current LM */
    if (dictwid_map == NULL)
	dictwid_map = (int32 *) CM_calloc (n_dict, sizeof (int32));
    model->dictwid_map = dictwid_map;
    
    for (i = 0; i < n_dict; i++)
	model->dictwid_map[i] = 0x80000000;	/* an illegal index into unigrams */
    
    if (n_tg > 0) {
	model->tseg_base = (int32 *) CM_calloc ((n_bg+1)/bg_seg_sz+1, sizeof (int32));
#if 0
	E_INFO("%8d = tseg_base entries allocated\n", (n_bg+1)/bg_seg_sz+1);
#endif
    }

    model->max_ucount = model->ucount = n_ug;
    model->bcount = n_bg;
    model->tcount = n_tg;
    model->dict_size = n_dict;
    
    model->HT.size = 0;
    model->HT.inuse = 0;
    model->HT.tab = NULL;

    return model;
}


static int32 wstr2wid (lm_t *model, char *w)
{
    caddr_t val;
    
    if (hash_lookup (&(model->HT), w, &val) != 0)
	return NO_WORD;
    return ((int32) val);
}


/*
 * Read and return #unigrams, #bigrams, #trigrams as stated in input file.
 */
static void ReadNgramCounts (fp, n_ug, n_bg, n_tg)
    FILE *fp;
    int32 *n_ug, *n_bg, *n_tg;		/* return the info here */
{
    char string[256];
    int32 ngram, ngram_cnt;
    
    /* skip file until past the '\data\' marker */
    do
	fgets (string, sizeof (string), fp);
    while ( (strcmp (string, "\\data\\\n") != 0) && (! feof (fp)) );

    if (strcmp (string, "\\data\\\n") != 0)
	E_FATAL("No \\data\\ mark in LM file\n");

    *n_ug = *n_bg = *n_tg = 0;
    while (fgets (string, sizeof (string), fp) != NULL) {
	if (sscanf (string, "ngram %d=%d", &ngram, &ngram_cnt) != 2)
	    break;
	switch (ngram) {
	case 1: *n_ug = ngram_cnt;
	    break;
	case 2: *n_bg = ngram_cnt;
	    break;
	case 3: *n_tg = ngram_cnt;
	    break;
	default:
	    E_FATAL("Unknown ngram (%d)\n", ngram);
	    break;
	}
    }
    
    /* Position file to just after the unigrams header '\1-grams:\' */
    while ( (strcmp (string, "\\1-grams:\n") != 0) && (! feof (fp)) )
	fgets (string, sizeof (string), fp);
    
    /* Check counts;  NOTE: #trigrams *CAN* be 0 */
    if ((*n_ug <= 0) || (*n_bg <= 0) || (*n_tg < 0))
	E_FATAL("Bad or missing ngram count\n");
}

/*
 * Read in the unigrams from given file into the LM structure model.  On
 * entry to this procedure, the file pointer is positioned just after the
 * header line '\1-grams:'.
 */
static void ReadUnigrams (fp, model)
    FILE *fp;		/* input file */
    lm_t *model;	/* to be filled in */
{
    char string[256];
    char name[128];
    int32 wcnt;
    float p1, bo_wt;
    double ignored_prob = 0.0;
    
    E_INFO("Reading unigrams\n");
    fflush (stdout);

    wcnt = 0;
    while ((fgets (string, sizeof(string), fp) != NULL) &&
	   (strcmp (string, "\\2-grams:\n") != 0))
    {
	if (sscanf (string, "%f %s %f", &p1, name, &bo_wt) != 3) {
	    if (string[0] != '\n')
		E_WARN("Format error; unigram ignored: %s", string);
	    continue;
	}
	
	if (wcnt >= model->ucount)
	    E_FATAL("Too many unigrams\n");

	/* Associate name with word id */
	word_str[wcnt] = (char *) salloc (name);
	hash_add (&(model->HT), word_str[wcnt], (caddr_t) wcnt);
	model->unigrams[wcnt].prob1.f = p1;
	model->unigrams[wcnt].bo_wt1.f = bo_wt;

	model->unigrams[wcnt].wid = wcnt;
	wcnt++;
    }

    if (model->ucount != wcnt) {
	E_WARN("lm_t.ucount(%d) != #unigrams read(%d)\n", model->ucount, wcnt);
	model->ucount = wcnt;
    }
}


/*
 * Read bigrams from given file into given model structure.  File may be arpabo
 * or arpabo-id format, depending on idfmt = 0 or 1.
 */
static void ReadBigrams (FILE *fp, lm_t *model, int32 idfmt)
{
    char string[1024], word1[256], word2[256];
    int32 w1, w2, prev_w1, bgcount, p;
    bigram_t *bgptr;
    float p2, bo_wt;
    int32 n_fld, n;
    
    E_INFO("Reading bigrams\n");
    fflush (stdout);
    
    bgcount = 0;
    bgptr = model->bigrams;
    prev_w1 = -1;
    n_fld = (model->tcount > 0) ? 4 : 3;

    bo_wt = 0.0;
    while (fgets (string, sizeof(string), fp) != NULL) {
	if (! idfmt)
	    n = sscanf (string, "%f %s %s %f", &p2, word1, word2, &bo_wt);
	else
	    n = sscanf (string, "%f %d %d %f", &p2, &w1, &w2, &bo_wt);
	if (n < n_fld) {
	    if (string[0] != '\n')
		break;
	    continue;
	}

	if (! idfmt) {
	    if ((w1 = wstr2wid (model, word1)) == NO_WORD)
		E_FATAL("Unknown word: %s\n", word1);
	    if ((w2 = wstr2wid (model, word2)) == NO_WORD)
		E_FATAL("Unknown word: %s\n", word2);
	} else {
	    if ((w1 >= model->ucount) || (w2 >= model->ucount) || (w1 < 0) || (w2 < 0))
		E_FATAL("Bad bigram: %s", string);
	}
	
	/* HACK!! to quantize probs to 4 decimal digits */
	p = (int) (p2*10000);
	p2 = (float) (p*0.0001);
	p = (int) (bo_wt*10000);
	bo_wt = (float) (p*0.0001);

	if (bgcount >= model->bcount)
	    E_FATAL("Too many bigrams\n");
	
	bgptr->wid = w2;
	bgptr->prob2 = sorted_id (&sorted_prob2, &p2);
	if (model->tcount > 0)
	    bgptr->bo_wt2 = sorted_id (&sorted_bo_wt2, &bo_wt);

	if (w1 != prev_w1) {
	    if (w1 < prev_w1)
		E_FATAL("Bigrams not in unigram order\n");
	    
	    for (prev_w1++; prev_w1 <= w1; prev_w1++)
		model->unigrams[prev_w1].bigrams = bgcount;
	    prev_w1 = w1;
	}
	
	bgcount++;
	bgptr++;

	if ((bgcount & 0x0000ffff) == 0) {
	    printf (".");
	    fflush (stdout);
	}
    }
    if ((strcmp (string, "\\end\\\n") != 0) && (strcmp (string, "\\3-grams:\n") != 0))
	E_FATAL("Bad bigram: %s\n", string);
    
    for (prev_w1++; prev_w1 <= model->ucount; prev_w1++)
	model->unigrams[prev_w1].bigrams = bgcount;
}


/*
 * Very similar to ReadBigrams.
 */
static void ReadTrigrams (FILE *fp, lm_t *model, int32 idfmt)
{
    char string[1024], word1[256], word2[256], word3[256];
    int32 i, n, w1, w2, w3, prev_w1, prev_w2, tgcount, prev_bg, bg, endbg, p;
    int32 seg, prev_seg, prev_seg_lastbg, tgoff;
    trigram_t *tgptr;
    bigram_t *bgptr;
    float p3;
    
    E_INFO("Reading trigrams\n");
    fflush (stdout);
    
    tgcount = 0;
    tgptr = model->trigrams;
    prev_w1 = -1;
    prev_w2 = -1;
    prev_bg = -1;
    prev_seg = -1;

    while (fgets (string, sizeof(string), fp) != NULL) {
	if (! idfmt)
	    n = sscanf (string, "%f %s %s %s", &p3, word1, word2, word3);
	else
	    n = sscanf (string, "%f %d %d %d", &p3, &w1, &w2, &w3);
	if (n != 4) {
	    if (string[0] != '\n')
		break;
	    continue;
	}

	if (! idfmt) {
	    if ((w1 = wstr2wid (model, word1)) == NO_WORD)
		E_FATAL("Unknown word: %s\n", word1);
	    if ((w2 = wstr2wid (model, word2)) == NO_WORD)
		E_FATAL("Unknown word: %s\n", word2);
	    if ((w3 = wstr2wid (model, word3)) == NO_WORD)
		E_FATAL("Unknown word: %s\n", word3);
	} else {
	    if ((w1 >= model->ucount) || (w2 >= model->ucount) || (w3 >= model->ucount) ||
		(w1 < 0) || (w2 < 0) || (w3 < 0))
		E_FATAL("Bad trigram: %s", string);
	}
	
	/* HACK!! to quantize probs to 4 decimal digits */
	p = (int) (p3*10000);
	p3 = (float) (p*0.0001);

	if (tgcount >= model->tcount)
	    E_FATAL("Too many trigrams\n");
	
	tgptr->wid = w3;
	tgptr->prob3 = sorted_id (&sorted_prob3, &p3);

	if ((w1 != prev_w1) || (w2 != prev_w2)) {
	    /* Trigram for a new bigram; update tg info for all previous bigrams */
	    if ((w1 < prev_w1) || ((w1 == prev_w1) && (w2 < prev_w2)))
		E_FATAL("Trigrams not in bigram order\n");
	    
	    bg = (w1 != prev_w1) ? model->unigrams[w1].bigrams : prev_bg+1;
	    endbg = model->unigrams[w1+1].bigrams;
	    bgptr = model->bigrams + bg;
	    for (; (bg < endbg) && (bgptr->wid != w2); bg++, bgptr++);
	    if (bg >= endbg)
		E_FATAL("Missing bigram for trigram: %s", string);

	    /* bg = bigram entry index for <w1,w2>.  Update tseg_base */
	    seg = bg >> log_bg_seg_sz;
	    for (i = prev_seg+1; i <= seg; i++)
		model->tseg_base[i] = tgcount;

	    /* Update trigrams pointers for all bigrams until bg */
	    if (prev_seg < seg) {
		if (prev_seg >= 0) {
		    tgoff = tgcount - model->tseg_base[prev_seg];
		    if (tgoff > 65535)
			E_FATAL("Offset from tseg_base > 65535; reduce log2(bg_seg_sz)\n");
		}
		
		prev_seg_lastbg = ((prev_seg+1) << log_bg_seg_sz) - 1;
		bgptr = model->bigrams + prev_bg;
		for (++prev_bg, ++bgptr; prev_bg <= prev_seg_lastbg; prev_bg++, bgptr++)
		    bgptr->trigrams = tgoff;
		
		for (; prev_bg <= bg; prev_bg++, bgptr++)
		    bgptr->trigrams = 0;
	    } else {
		tgoff = tgcount - model->tseg_base[prev_seg];
		if (tgoff > 65535)
		    E_FATAL("Offset from tseg_base > 65535; reduce log2(bg_seg_sz)\n");
		
		bgptr = model->bigrams + prev_bg;
		for (++prev_bg, ++bgptr; prev_bg <= bg; prev_bg++, bgptr++)
		    bgptr->trigrams = tgoff;
	    }
	    
	    prev_w1 = w1;
	    prev_w2 = w2;
	    prev_bg = bg;
	    prev_seg = seg;
	}
	
	tgcount++;
	tgptr++;

	if ((tgcount & 0x0000ffff) == 0) {
	    printf (".");
	    fflush (stdout);
	}
    }
    if (strcmp (string, "\\end\\\n") != 0)
	E_FATAL("Bad trigram: %s\n", string);
    
    for (prev_bg++; prev_bg <= model->bcount; prev_bg++) {
	if ((prev_bg & (bg_seg_sz-1)) == 0)
	    model->tseg_base[prev_bg >> log_bg_seg_sz] = tgcount;
	if ((tgcount - model->tseg_base[prev_bg >> log_bg_seg_sz]) > 65535)
	    E_FATAL("Offset from tseg_base > 65535; reduce log2(bg_seg_sz)\n");
	model->bigrams[prev_bg].trigrams =
	    tgcount - model->tseg_base[prev_bg >> log_bg_seg_sz];
    }
}

static FILE *lm_file_open (filename, usepipe)
    char *filename;
    int32 usepipe;
{
    char command[1024];
    FILE *fp;
    
    if (usepipe) {
#ifdef WIN32
	sprintf (command, "D:\\compress\\gzip.exe -d -c %s", filename);
	if ((fp = _popen (command, "r")) == NULL)
	    E_FATAL("Cannot popen %s\n", command);
#else
	sprintf (command, "zcat %s", filename);
	if ((fp = popen (command, "r")) == NULL)
	    E_FATAL("Cannot popen %s\n", command);
#endif
    }
    else {
        fp = CM_fopen (filename, "r");
    }
    return (fp);
}

/*
 * Reads in a trigram language model from the given file.
 */
int32 lm_read (char *filename, char *lmname)
{
    lm_t *model;
    FILE *fp = NULL;
    int32 usingPipe = 0;
    int32 n_unigram;
    int32 n_bigram;
    int32 n_trigram;
    int32 dict_size;
    int32 i, k;
    char dumpfile[4096];
    struct stat statbuf;
    int32 idfmt;

    E_INFO("Reading LM file %s (name \"%s\")\n", filename, lmname);
    
    /* Check if a compressed file */
    k = strlen(filename);
#ifdef WIN32
    usingPipe = (k > 3) &&
	((strcmp (filename+k-3, ".gz") == 0) || (strcmp (filename+k-3, ".GZ") == 0));
#else
    usingPipe = (k > 2) &&
	((strcmp (filename+k-2, ".Z") == 0) || (strcmp (filename+k-2, ".z") == 0));
#endif
    /* Check if an .arpabo-id format file; More HACK!! Hardwired check for -id */
    if (usingPipe)
	k -= 2;
    idfmt = ((k > 3) && (strncmp (filename+k-3, "-id", 3) == 0));

    fp = lm_file_open (filename, usingPipe);
    if (stat(filename, &statbuf) < 0)
	E_FATAL("stat(%s) failed\n", filename);

    /* Read #unigrams, #bigrams, #trigrams from file */
    ReadNgramCounts (fp, &n_unigram, &n_bigram, &n_trigram);
    E_INFO("ngrams 1=%d, 2=%d, 3=%d\n", n_unigram, n_bigram, n_trigram);
    
    /* Determine dictionary size (for dict-wid -> LM-wid map) */
    dict_size = n_unigram;
    if (dict_size >= 65535)
	E_FATAL("#dict-words(%d) > 65534\n", dict_size);
    
    /* Allocate space for LM, including initial OOVs and placeholders; initialize it */
    model = lmp = NewModel (n_unigram, n_bigram, n_trigram, dict_size);
    word_str = (char **) CM_calloc (n_unigram, sizeof (char *));
    
    /* Create name for binary dump form of Darpa LM file */
#if (! WIN32)
    for (i = strlen(filename)-1; (i >= 0) && (filename[i] != '/'); --i);
#else
    for (i = strlen(filename)-1;
	 (i >= 0) && (filename[i] != '\\') && (filename[i] != '/'); --i);
#endif
    i++;
    
    /* form dumpfilename */
    sprintf (dumpfile, "%s/%s.DMP", dumpdir, filename+i);
    
    ReadUnigrams (fp, model);
    E_INFO("%8d = #unigrams created\n", model->ucount);
    
    init_sorted_list (&sorted_prob2);
    if (model->tcount > 0)
	init_sorted_list (&sorted_bo_wt2);
    
    ReadBigrams (fp, model, idfmt);
    
    model->bcount = FIRST_BG(model,model->ucount);
    model->n_prob2 = sorted_prob2.free;
    model->prob2  = vals_in_sorted_list (&sorted_prob2);
    free_sorted_list (&sorted_prob2);
    E_INFO("%8d = #bigrams created\n", model->bcount);
    E_INFO("%8d = #prob2 entries\n", model->n_prob2);
    
    if (model->tcount > 0) {
	/* Create trigram bo-wts array */
	model->n_bo_wt2 = sorted_bo_wt2.free;
	model->bo_wt2 = vals_in_sorted_list (&sorted_bo_wt2);
	free_sorted_list (&sorted_bo_wt2);
	E_INFO("%8d = #bo_wt2 entries\n", model->n_bo_wt2);
	
	init_sorted_list (&sorted_prob3);
	
	ReadTrigrams (fp, model, idfmt);
	
	model->tcount = FIRST_TG(model,model->bcount);
	model->n_prob3 = sorted_prob3.free;
	model->prob3  = vals_in_sorted_list (&sorted_prob3);
	E_INFO("%8d = #trigrams created\n", model->tcount);
	E_INFO("%8d = #prob3 entries\n", model->n_prob3);
	
	free_sorted_list (&sorted_prob3);
    }
    
    /* Dump binary form of LM file */
    lm3g_dump (dumpfile, model, filename, (int32)statbuf.st_mtime);
    
    if (usingPipe) {
#ifdef WIN32
	_pclose (fp);
#else
	pclose (fp);
#endif
    } else
	fclose (fp);
    fflush (stdout);
    fflush (stderr);
    
    return 0;
}


#if (__BIG_ENDIAN__)

#define SWAPW(x)	x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define SWAPL(x)	x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
    			      (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )

#else

#define SWAPW(x)
#define SWAPL(x)

#endif


static fwrite_int32 (fp, val)
    FILE *fp;
    int32 val;
{
    SWAPL(val);
    fwrite (&val, sizeof(int32), 1, fp);
}

static fwrite_ug (fp, ug)
    FILE *fp;
    unigram_t *ug;
{
    unigram_t tmp_ug = *ug;
    
    SWAPL(tmp_ug.wid);
    SWAPL(tmp_ug.prob1.l);
    SWAPL(tmp_ug.bo_wt1.l);
    SWAPL(tmp_ug.bigrams);
    fwrite (&tmp_ug, sizeof(unigram_t), 1, fp);
}

static fwrite_bg (fp, bg)
    FILE *fp;
    bigram_t *bg;
{
    bigram_t tmp_bg = *bg;
    
    SWAPW(tmp_bg.wid);
    SWAPW(tmp_bg.prob2);
    SWAPW(tmp_bg.bo_wt2);
    SWAPW(tmp_bg.trigrams);
    fwrite (&tmp_bg, sizeof(bigram_t), 1, fp);
}

static fwrite_tg (fp, tg)
    FILE *fp;
    trigram_t *tg;
{
    trigram_t tmp_tg = *tg;
    
    SWAPW(tmp_tg.wid);
    SWAPW(tmp_tg.prob3);
    fwrite (&tmp_tg, sizeof(trigram_t), 1, fp);
}


static char *fmtdesc[] = {
    "BEGIN FILE FORMAT DESCRIPTION",
    "Header string length (int32) and string (including trailing 0)",
    "Original LM filename string-length (int32) and filename (including trailing 0)",
    "(int32) version number (present iff value <= 0)",
    "(int32) original LM file modification timestamp (iff version# present)",
    "(int32) string-length and string (including trailing 0) (iff version# present)",
    "... previous entry continued any number of times (iff version# present)",
    "(int32) 0 (terminating sequence of strings) (iff version# present)",
    "(int32) log_bg_seg_sz (present iff different from default value of LOG_BG_SEG_SZ)",
    "(int32) lm_t.ucount (must be > 0)",
    "(int32) lm_t.bcount",
    "(int32) lm_t.tcount",
    "lm_t.ucount+1 unigrams (including sentinel)",
    "lm_t.bcount+1 bigrams (including sentinel)",
    "lm_t.tcount trigrams (present iff lm_t.tcount > 0)",
    "(int32) lm_t.n_prob2",
    "(int32) lm_t.prob2[]",
    "(int32) lm_t.n_bo_wt2 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.bo_wt2[] (present iff lm_t.tcount > 0)",
    "(int32) lm_t.n_prob3 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.prob3[] (present iff lm_t.tcount > 0)",
    "(int32) (lm_t.bcount+1)/bg_seg_sz+1 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.tseg_base[] (present iff lm_t.tcount > 0)",
    "(int32) Sum(all word string-lengths, including trailing 0 for each)",
    "All word strings (including trailing 0 for each)",
    "END FILE FORMAT DESCRIPTION",
    NULL,
};

/*
 * Dump internal LM to file.  Format described above.
 * Remember to swap bytes if necessary.
 */
static int32 lm3g_dump (file, model, lmfile, mtime)
    char *file;		/* output file */
    lm_t *model;
    char *lmfile;	/* original Darpa LM filename */
    int32 mtime;	/* lmfile last mod time */
{
    int32 i, k;
    FILE *fp;

    E_INFO("Dumping LM to %s\n", file);
    if ((fp = fopen (file, "wb")) == NULL) {
	E_ERROR("Cannot create file %s\n", file);
	return 0;
    }

    k = strlen(darpa_hdr)+1;
    fwrite_int32 (fp, k);
    fwrite (darpa_hdr, sizeof(char), k, fp);

    k = strlen(lmfile)+1;
    fwrite_int32 (fp, k);
    fwrite (lmfile, sizeof(char), k, fp);
    
    /* Write version# and LM file modification date */
    if (log_bg_seg_sz != LOG_BG_SEG_SZ)	/* Hack!! */
	fwrite_int32 (fp, -2);	/* version # */
    else
	fwrite_int32 (fp, -1);	/* version # */
    fwrite_int32 (fp, mtime);
    
    /* Write file format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	k = strlen(fmtdesc[i])+1;
	fwrite_int32 (fp, k);
	fwrite (fmtdesc[i], sizeof(char), k, fp);
    }
    fwrite_int32 (fp, 0);
    
    /* HACK!! Write only if different from previous version */
    if (log_bg_seg_sz != LOG_BG_SEG_SZ)
	fwrite_int32 (fp, log_bg_seg_sz);

    fwrite_int32 (fp, model->ucount);
    fwrite_int32 (fp, model->bcount);
    fwrite_int32 (fp, model->tcount);

    for (i = 0; i <= model->ucount; i++)
	fwrite_ug (fp, &(model->unigrams[i]));
    for (i = 0; i <= model->bcount; i++)
	fwrite_bg (fp, &(model->bigrams[i]));
    for (i = 0; i < model->tcount; i++)
	fwrite_tg (fp, &(model->trigrams[i]));
    
    fwrite_int32 (fp, model->n_prob2);
    for (i = 0; i < model->n_prob2; i++)
	fwrite_int32 (fp, model->prob2[i].l);
    
    if (model->tcount > 0) {
	fwrite_int32 (fp, model->n_bo_wt2);
	for (i = 0; i < model->n_bo_wt2; i++)
	    fwrite_int32 (fp, model->bo_wt2[i].l);
	fwrite_int32 (fp, model->n_prob3);
	for (i = 0; i < model->n_prob3; i++)
	    fwrite_int32 (fp, model->prob3[i].l);
	
	k = (model->bcount+1)/bg_seg_sz + 1;
	fwrite_int32 (fp, k);
	for (i = 0; i < k; i++)
	    fwrite_int32 (fp, model->tseg_base[i]);
    }
    
    k = 0;
    for (i = 0; i < model->ucount; i++)
	k += strlen(word_str[i])+1;
    fwrite_int32 (fp, k);
    for (i = 0; i < model->ucount; i++)
	fwrite (word_str[i], sizeof(char), strlen(word_str[i])+1, fp);

    fclose (fp);
    return 0;
}


main (int32 argc, char *argv[])
{
    char *lmfile;
    
    if ((argc < 3) || (argc > 4)) {
	E_INFO("Usage: %s <LMfile> <Dump-directory> [log2(bg-seg-size); default=%d]\n",
	       argv[0], LOG_BG_SEG_SZ);
	exit(0);
    }
    
    lmfile = argv[1];
    dumpdir = argv[2];
    
    log_bg_seg_sz = LOG_BG_SEG_SZ;	/* Default */
    if ((argc > 3) && (sscanf (argv[3], "%d", &log_bg_seg_sz) != 1))
	E_FATAL("Usage: %s <LMfile> <Dump-directory> [log2(bg-seg-size); default=%d]\n",
		argv[0], LOG_BG_SEG_SZ);
    if ((log_bg_seg_sz < 1) || (log_bg_seg_sz > 15))
	E_FATAL("log2(bg-seg-size) argument must be in range 1..15\n");
    bg_seg_sz = 1 << log_bg_seg_sz;
    
    lm_read(lmfile, "");
}
