/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * main_dp.c -- DP alignment of word strings
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
 * $Log$
 * Revision 1.2  2006/02/24  03:49:19  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Added application dp.
 * 
 * Revision 1.1.2.1  2006/01/16 20:23:20  arthchan2003
 * Added a routine to compute confidence scores and do string matching.
 *
 *
 * 28-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added maximization of #correct when #errors same.
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3types.h>

#define MAX_HYP_LEN	600
#define DP_HASH_SIZE 100000

typedef struct node_s {
    int16 i, d, s, t;	/* Total ins, del, sub (and their sum) in best path to this node */
    int16 c;		/* #correct (#hyp - #(sub + ins)) */
    int16 pred;		/* Predecessor (col in previous row) */
} node_t;


static arg_t arg[] = {
  { "-reffile",
    ARG_STRING,
    NULL,
    "Reference transcription file"},
  { "-hypfile",
    ARG_STRING,
    NULL,
    "Hypothesis transcription file"},
  { "-seg",
    ARG_INT32,
    "1",
    "Boolean value specified whether to use the old transcription format. "},
  { "-d",
    ARG_INT32,
    "0",
    "A debug flag to specified whether the scores for dp algorithm will be dumped. "},    
  { "-logfn",
    ARG_STRING,
    NULL,
    "Log file (default stdout/stderr)" },

  { NULL, ARG_INT32,  NULL, NULL }
};



static hash_table_t *dict_ht;
static char **word;
static int32 n_word_alloc;
static int32 n_word;
static int32 silwid;

static node_t *node_alloc;

static char uttid[256];

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


static int32 word2id (char *w)
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


static void dp_init ( void )
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

static int32 n_corr, n_err, n_sub, n_ins, n_del;

static int32 dp (int32 *ref, int32 nref, int32 *hyp, int32 *hyp_sf, int32 nhyp)
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


static int32 refline2wds (char *line, int32 *wid)
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


#if 0
static int32 strcmp_nocase (char *s1, char *s2)
{
    char c1, c2;
    
    for (; *s1 && *s2; s1++, s2++) {
	c1 = *s1;
	c2 = *s2;

	if ((c1 >= 'a') && (c1 <= 'z'))
	    c1 -= 32;
	if ((c2 >= 'a') && (c2 <= 'z'))
	    c2 -= 32;
	
	if (c1 != c2)
	    return (c1 - c2);
    }

    return (*s1 ? 1 : (*s2 ? -1 : 0));
}
#endif


/*
 * Parses hypotheses in FWDXCT format (see decoder log output).
 */
static int32 hypline2wds (char *line, int32 *wid, int32 *sf, int32 oldfmt)
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


int main (int32 argc, char *argv[])
{
    char line[32768], *reffile, *hypfile;
    int32 ref[MAX_HYP_LEN], hyp[MAX_HYP_LEN], nref, nhyp, hyp_sf[MAX_HYP_LEN];
    int32 tot_err, tot_corr, tot_ref, tot_hyp, tot_sub, tot_ins, tot_del;
    FILE *rfp, *hfp;
    int32 oldfmt;
    

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc,argv,"default.arg",arg);

    reffile = cmd_ln_str("-reffile");
    hypfile = cmd_ln_str("-hypfile");

    /*argv[1];
    hypfile = argv[2];
    oldfmt = ((argc > 3) && (strcmp (argv[3], "-seg") == 0)) ? 0 : 1;*/

    oldfmt = cmd_ln_int32("-seg");
    

    if ((rfp = fopen (reffile, "r")) == NULL)
	E_FATAL("fopen(%s,r) (reffile) failed\n", reffile);
    if (strcmp (hypfile, "-") != 0) {
	if ((hfp = fopen (hypfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) (hypfile) failed\n", hypfile);
    } else
	hfp = stdin;
    
    dp_init ();
    
    tot_err = tot_corr = tot_ref = tot_hyp = tot_sub = tot_ins = tot_del = 0;
    while (fgets (line, sizeof(line), rfp) != NULL) {
	if ((nref = refline2wds (line, ref)) < 0)
	    E_FATAL("Bad line in file %s: %s\n", reffile, line);
	ref[nref++] = silwid;

	if (fgets (line, sizeof(line), hfp) == NULL)
	    E_FATAL("Premature EOF(%s)\n", hypfile);
	if ((nhyp = hypline2wds (line, hyp, hyp_sf, oldfmt)) < 0)
	    E_FATAL("Bad line in file %s: %s\n", hypfile, line);
	hyp[nhyp++] = silwid;

	if (oldfmt)
	    tot_err += dp (ref, nref, hyp, NULL, nhyp);
	else
	    tot_err += dp (ref, nref, hyp, hyp_sf, nhyp);
	tot_ref += nref-1;
	tot_hyp += nhyp-1;
	tot_corr += n_corr;
	tot_sub += n_sub;
	tot_ins += n_ins;
	tot_del += n_del;
	
	printf ("==== %7d ref, %7d hyp, %7d err, %8.3f%% (%7d sub, %7d ins, %7d del); %7d corr, %8.3f%%, %s ====\n",
		tot_ref, tot_hyp, tot_err, (tot_err*100.0/tot_ref),
		tot_sub, tot_ins, tot_del,
		tot_corr, (tot_corr*100.0/tot_ref),
		uttid);
    }

    if (tot_ref > 0)
	printf ("==== SUMMARY: %d ref, %d hyp, %d err, %.3f%% (%d sub, %d ins, %d del); %d corr, %.3f%% ====\n",
		tot_ref, tot_hyp, tot_err, (tot_err*100.0/tot_ref),
		tot_sub, tot_ins, tot_del,
		tot_corr, (tot_corr*100.0/tot_ref));
    else
	printf ("==== SUMMARY: %d ref, %d hyp, %d err (%d sub, %d ins, %d del); %d corr ====\n",
		tot_ref, tot_hyp, tot_err, tot_sub, tot_ins, tot_del, tot_corr);

  cmd_ln_appl_exit();

    return 0 ;
}


