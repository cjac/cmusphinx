/* ====================================================================
 * Copyright (c) 1995-2005 Carnegie Mellon University.  All rights
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
 * dp_linear.c -- DP alignment core routines.
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
 * 07-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#include "dp_linear.h"
static node_t *node_alloc;
static hash_table_t *dict_ht;
static int32 n_word_alloc;
static int32 n_word;
static char **word;
extern int32 silwid;
extern char uttid[256];
extern int32 n_corr, n_err, n_sub, n_ins, n_del;

static char *filler[] = {
    "<s>",
    "</s>",
    "<sil>",
    "++INHALE++",
    "++SMACK++",
    NULL,
};

typedef struct {
    char *word;
    char *map;
} filler2phnMap;

filler2phnMap fillpause[] = 
{ {"++UH++", "UH"},
  {"++AH++", "AH"},
  {"++UM++", "UM"},
  {"++EH++", "EH"},
  {NULL, NULL}
};


int32 refline2wds (char *line, int32 *wid)
{
    char *lp, word[256];
    int32 len, n, k;
    
    lp = line;
    n = 0;
    uttid[0] = '\0';
    
    while (sscanf (lp, "%s%n", word, &len) == 1) {
	lp += len;

	if (n > MAX_HYP_LEN-2)
	    E_FATAL("Increase MAX_HYP_LEN\n");
	
	k = strlen(word);
	if ((word[0] == '(') && (word[k-1] == ')')) {
	    word[k-1] = '\0';
	    strcpy (uttid, word+1);
	    break;
	}
	
	wid[n++] = word2id (word);
    }
    
    if (sscanf (lp, "%s%n", word, &len) == 1)
	return -1;
    
    return n;
}


/*
 * Parses hypotheses in FWDXCT format (see decoder log output).
 */
int32 hypline2wds (char *line, int32 *wid, int32 *sf, int32 oldfmt)
{
    char *lp, word[256], junk[256];
    int32 len, n, i, k, f;
    
    lp = line;
    n = 0;
    
    /* Skip uttid, scalefactor, acoustic and LM scores info */
    if (! oldfmt) {
	if (sscanf (lp, "%s%s%d%s%d%s%d%n", word, junk, &k, junk, &k, junk, &k, &len) != 7)
	    E_FATAL("Bad hyp line: %s\n", line);
	if (strcmp_nocase (word, uttid) != 0)
	    E_ERROR("Uttid mismatch: Ref: %s, Hyp: %s\n", uttid, word);
	lp += len;
    } else {
	/* Strip uttid from end of utterance */
	for (k = strlen (line) - 1; (k > 0) && (line[k] != ')'); --k);
	if (k <= 0)
	    E_FATAL("Cannot find ending ) in:\n%s\n", line);
	line[k] = '\0';
	for (--k; (k > 0) && (line[k] != '('); --k);
	if (k <= 0)
	    E_FATAL("Cannot find (uttid in:\n%s\n", line);
	
	if (sscanf (line+k+1, "%s", word) != 1)
	    E_FATAL("Cannot read uttid in:\n%s\n", line);
	if (strcmp_nocase (word, uttid) != 0)
	    E_ERROR("Uttid mismatch: %s(ref), %s(hyp)\n", uttid, word);
	
	line[k] = '\0';
    }
    
    for (;;) {
	if (! oldfmt) {
	    if (sscanf (lp, "%d%s%n", &f, word, &len) != 2)
		break;
	} else {
	    if (sscanf (lp, "%s%n", word, &len) != 1)
		break;
	}
	lp += len;
	
	if (strcmp (word, "(null)") == 0)
	    E_ERROR ("%s: (null) hypothesis???\n", uttid);
	
	/* Strip alternative pronunciation indication (%d) at end of word, if any */
	k = strlen(word);
	if ((k > 2) && (word[k-1] == ')')) {
	    for (--k; (k > 0) && (word[k] != '('); --k);
	    if (k > 0)
		word[k] = '\0';
	}

	/* Skip transparent filler words */
	for (i = 0; filler[i] && (strcmp (filler[i], word) != 0); i++);
	if (filler[i])
	    continue;

	/* Transform filled pause filler words */
	for (i = 0; fillpause[i].word && (strcmp (fillpause[i].word, word) != 0); i++);
	if (fillpause[i].word)
	    strcpy (word, fillpause[i].map);

	if (n > MAX_HYP_LEN-2)
	    E_FATAL("Increase MAX_HYP_LEN\n");
	
	sf[n] = f;
	wid[n] = word2id (word);

	n++;
    }
    
    return n;
}


int32 word2id (char *w)
{
    int32 wid;
    
    if (hash_lookup (dict_ht, w, &wid) < 0) {
	if (n_word >= n_word_alloc)
	    E_FATAL("Increase dictionary size\n");
	word[n_word] = ckd_salloc (w);
	hash_enter (dict_ht, word[n_word], n_word);
	wid = n_word++;
    }

    return wid;
}


void dp_init ( void )
{
    node_alloc = (node_t *) ckd_calloc (((MAX_HYP_LEN << 1) + 1) * MAX_HYP_LEN,
					sizeof(node_t));
    
    dict_ht = hash_new (DP_HASH_SIZE, HASH_CASE_YES);
    n_word_alloc = DP_HASH_SIZE;
    word = (char **) ckd_calloc (n_word_alloc, sizeof(char *));
    n_word = 0;

    silwid = word2id (" ");
}

/* For debugging */
static void dparray_dump (node_t **dparray, int32 rows, int32 cols)
{
    int32 i, j;
    
    for (i = 0; i < rows; i++) {
	printf ("%2d: ", i);
	for (j = 0; j < cols; j++)
	    printf ("  %2d,%2d,%2d,%2d,%2d",
		    dparray[i][j].t, dparray[i][j].s, dparray[i][j].i, dparray[i][j].d,
		    dparray[i][j].pred);
	printf ("\n");
    }
}




int32 dp (int32 *ref, int32 nref, int32 *hyp, int32 *hyp_sf, int32 nhyp)
{
    int32 i,j, jj, k;
    int32 err, minerr, best, succ, pred, del, sub, nextcorr;
    int32 map[MAX_HYP_LEN+1];
    int8 corr[MAX_HYP_LEN];
    char wd[256];
    node_t **dparray;
    
    
    /* Set up 2-D DP-array for the current size of hyp and ref */
    dparray = (node_t **) ckd_calloc (nhyp, sizeof(node_t *));
    dparray[0] = node_alloc;
    for (i = 1; i < nhyp; i++)
	dparray[i] = dparray[i-1] + (nref<<1);
    
    /* Initialize costs for each DP-array node */
    for (i = 0; i < nhyp; i++) {
	for (j = 0, jj = 0; j < nref; j++, jj += 2) {
	    dparray[i][jj].t = (int32)0x7fff;
	    dparray[i][jj+1].t = (int32)0x7fff;
	}
    }
    
    /* Initialize first row of DP-array */
    for (j = 0, jj = 0; j < nref; j++, jj += 2) {
	dparray[0][jj].i = 1;
	dparray[0][jj].d = j;
	dparray[0][jj].s = 0;
	dparray[0][jj].t = j+1;
	dparray[0][jj].c = 0;
	dparray[0][jj].pred = -1;
	
	dparray[0][jj+1].i = 0;
	dparray[0][jj+1].d = j;
	dparray[0][jj+1].s = (hyp[0] == ref[j]) ? 0 : 1;
	dparray[0][jj+1].t = dparray[0][jj+1].s + j;
	dparray[0][jj+1].c = 1 - dparray[0][jj+1].s;
	dparray[0][jj+1].pred = -1;
    }
    
    for (i = 1; i < nhyp; i++) {
	for (j = 0; j < nref; j++) {
	    /* Insertion column first */
	    jj = j << 1;
	    del = 0;
	    minerr = (int32)0x7fff;
	    for (k = jj; k >= 0; --k) {
		err = dparray[i-1][k].t + del + 1 /* 1 new ins error */;

		/* Minimize #errors and maximize #correct when #errors are equal */
		if (err < minerr) {
		    minerr = err;
		    best = k;
		} else if ((err == minerr) &&
			   ((dparray[i-1][best].s + dparray[i-1][best].i) >
			    (dparray[i-1][k].s + dparray[i-1][k].i))) {
		    best = k;
		}

		del += (k & 0x1);
	    }
	    dparray[i][jj].t = minerr;
	    dparray[i][jj].s = dparray[i-1][best].s;
	    dparray[i][jj].i = dparray[i-1][best].i + 1;
	    dparray[i][jj].d = dparray[i-1][best].d + ((jj-best)>>1);
	    dparray[i][jj].c = i - (dparray[i][jj].s + dparray[i][jj].i);
	    dparray[i][jj].pred = best;
	    
	    /* Substitution column next; vertical transition first */
	    jj++;
	    sub = 1;	/* This word is a substitution for a vertical transition */
	    if ((hyp[i-1] == ref[j]) && (dparray[i-1][jj].pred != jj))
		sub = 2;	/* Previous word was correct, but nullified by this */
	    minerr = dparray[i-1][jj].t + sub;
	    dparray[i][jj].t = minerr;
	    dparray[i][jj].s = dparray[i-1][jj].s + sub;
	    dparray[i][jj].i = dparray[i-1][jj].i;
	    dparray[i][jj].d = dparray[i-1][jj].d;
	    dparray[i][jj].c = i - (dparray[i][jj].s + dparray[i][jj].i);
	    dparray[i][jj].pred = jj;
	    best = jj;

	    /* Non-vertical transitions */
	    sub = (hyp[i] == ref[j]) ? 0 : 1;
	    del = 0;
	    for (k = jj-1; k >= 0; --k) {
		err = dparray[i-1][k].t + del + sub;

		if (err < minerr) {
		    minerr = err;
		    best = k;
		} else if ((err == minerr) &&
			   ((dparray[i-1][best].s + dparray[i-1][best].i) >
			    (dparray[i-1][k].s + dparray[i-1][k].i))) {
		    best = k;
		}

		del += (k & 0x1);
	    }
	    if (best != jj) {
		dparray[i][jj].t = minerr;
		dparray[i][jj].s = dparray[i-1][best].s + sub;
		dparray[i][jj].i = dparray[i-1][best].i;
		dparray[i][jj].d = dparray[i-1][best].d + ((jj-1-best)>>1);
		dparray[i][jj].c = i - (dparray[i][jj].s + dparray[i][jj].i);
		dparray[i][jj].pred = best;
	    }
	}
    }
    
    if(cmd_ln_int32("-d"))
       dparray_dump (dparray, nhyp, (nref<<1));

    best = (nref<<1) - 1;
    succ = (int32)0x7ffffff0;
    for (i = nhyp-1; i >= 0; --i) {
	pred = dparray[i][best].pred;

	map[i] = (best & 0x1) ? (best >>1) : -1;
	corr[i] =
	    ((map[i] >= 0) && (hyp[i] == ref[map[i]]) && (pred != best) && (succ != best)) ?
	    1 : 0;
	if (corr[i])
	    assert ((pred < best) && (succ > best));
	
	succ = best;
	best = pred;
    }
    map[nhyp] = nref;
    
    nextcorr = 0;
    i = 0;
    n_corr = 0;
    while (i < nhyp) {
	for (; (i < nhyp) && corr[i]; i++) {
	    if (map[i] != nextcorr) {
		/* Deleted reference words */
		assert (map[i] > nextcorr);
		printf ("[[ ");
		for (; nextcorr < map[i]; nextcorr++) {
		    strcpy (wd, word[ref[nextcorr]]);
		    ucase (wd);
		    printf ("%s ", wd);
		}
		printf ("=> ]] ");
	    }

	    strcpy (wd, word[hyp[i]]);
	    lcase (wd);
	    printf ("%s ", wd);
	    n_corr++;
	    
	    nextcorr++;
	}

	if ((i >= nhyp) && (nextcorr >= nref))
	    break;
	
	printf ("[[ ");
	for (j = i; (j < nhyp) && (! corr[j]); j++);	/* maximal incorrect seg */
	for (; nextcorr < map[j]; nextcorr++) {
	    strcpy (wd, word[ref[nextcorr]]);
	    ucase (wd);
	    printf ("%s ", wd);
	}
	
	printf ("=> ");
	if (hyp_sf) {
	    if ((i < j) || (j < nhyp))
		printf ("%d ", hyp_sf[i]);
	    else
		printf ("-1 ");
	}
	for (; i < j; i++) {
	    strcpy (wd, word[hyp[i]]);
	    ucase (wd);
	    printf ("%s ", wd);
	}
	printf ("]] ");
    }

    best = (nref<<1) - 1;
    n_corr--;	/* To get rid of dummy finishwid at the end */
    if (nref > 1) {
	printf ("<<Summary %s %d ref, %d hyp, %d err, %.2f%%, %d sub, %d ins, %d del; %d corr, %.2f%%>>\n",
		uttid, nref-1, nhyp-1,	/* -1 because of dummy finishwid at the end */
		dparray[nhyp-1][best].t, dparray[nhyp-1][best].t * 100.0 / (nref-1),
		dparray[nhyp-1][best].s, dparray[nhyp-1][best].i, dparray[nhyp-1][best].d,
		n_corr, n_corr * 100.0 / (nref-1));
    } else {
	printf ("<<Summary %s %d ref, %d hyp, %d err, %.2f%%, %d sub, %d ins, %d del; %d corr, %.2f%%>>\n",
		uttid, nref-1, nhyp-1,	/* -1 because of dummy finishwid at the end */
		dparray[nhyp-1][best].t, 100.0,
		dparray[nhyp-1][best].s, dparray[nhyp-1][best].i, dparray[nhyp-1][best].d,
		n_corr, 100.0);
    }

    /* Check (temporary debugging) */
    if (nhyp-1 - dparray[nhyp-1][best].s - dparray[nhyp-1][best].i != n_corr) {
	fprintf (stderr,
		 "%s %d ref, %d hyp, %d correct, %d errors, %d sub, %d ins, %d del\n",
		 uttid,
		 nref-1, nhyp-1, n_corr,	/* -1 for dummy finishwid at the end */
		 dparray[nhyp-1][best].t,
		 dparray[nhyp-1][best].s,
		 dparray[nhyp-1][best].i,
		 dparray[nhyp-1][best].d);
    }

    n_err = dparray[nhyp-1][best].t;
    n_sub = dparray[nhyp-1][best].s;
    n_ins = dparray[nhyp-1][best].i;
    n_del = dparray[nhyp-1][best].d;
    
    ckd_free (dparray);

    return n_err;
}
