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
 * allpalign.c -- DP alignment of time-marked phone sequences.
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


/*
 * DP alignment of time-marked phone sequences for the purpose of counting observed
 * pronunciations.  Noise and silence phones are ignored.
 */


#include <libutil/libutil.h>

#include <s3.h>
#include <main/mdef.h>
#include <main/dict.h>

#include "dp.h"


static mdef_t *mdef;
static dict_t *dict;
static s3wid_t silwid;


#define MAX_UTT_LEN	1024


static void make_dagnode (dagnode_t *d, s3wid_t wid, int32 sf, int32 ef)
{
    d->wid = wid;
    d->seqid = 0;
    d->sf = sf;
    d->fef = ef;
    d->lef = d->fef;
    d->reachable = 0;
    d->next = NULL;
    d->predlist = NULL;
    d->succlist = NULL;
}


/*
 * Create a degenerate DAG (linear sequence of nodes) for the given hyp line.
 * The DAG contains a terminal sentinel silwid node.
 */
static dag_t *allpfile_load (char *file)
{
    int32 sf, ef, scr;
    s3wid_t w;
    char *lp, word[4096];
    dag_t *dag;
    dagnode_t *d, *pd;
    FILE *fp;

    if ((fp = fopen(file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);

    /* Build DAG from word sequence */
    dag = ckd_calloc (1, sizeof(dag_t));
    dag->node_sf = (dagnode_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(dagnode_t *));
    dag->nnode = 0;
    dag->nfrm = 0;
    dag->nlink = 0;

    fscanf (fp, "%s", word);
    assert (strcmp (word, "SFrm") == 0);
    fscanf (fp, "%s", word);
    assert (strcmp (word, "EFrm") == 0);
    fscanf (fp, "%s", word);
    assert (strcmp (word, "SegAScr") == 0);
    fscanf (fp, "%s", word);
    assert (strcmp (word, "Phone") == 0);
    
    pd = NULL;
    while (fscanf (fp, "%d %d %d %s", &sf, &ef, &scr, word) == 4) {
	/* No need to skip filler and noise words; dp alignment takes care of it */
	w = dict_wordid(dict, word);
	assert (IS_WID(w));

	/* Create DAG node for word */
	d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));
	make_dagnode (d, w, sf, ef);
	
	dag->node_sf[sf] = d;
	if (pd) {
	    dag_link (pd, d);
	    dag->nlink++;
	}
	dag->nnode++;

	pd = d;
    }
    fscanf (fp, "%s", word);
    assert (strcmp (word, "Total") == 0);
    fclose (fp);

    assert (pd);
    dag->nfrm = ef+1;
    
    dag->entry.src = NULL;
    dag->entry.dst = dag->node_sf[0];
    dag->entry.next = NULL;
    assert (dag->entry.dst);

    dag->exit.src = NULL;
    dag->exit.dst = pd;
    dag->exit.next = NULL;

    return dag;
}


static int32 refline_parse (char *line, dagnode_t *ref, char *uttid)
{
    int32 sf, nf, n, len;
    s3wid_t w;
    char *lp, word[4096];
    
    uttid[0] = '\0';
    
    lp = line;
    n = 0;
    while (sscanf (lp, "%s %d %d%n", word, &sf, &nf, &len) == 3) {
	lp += len;

	/* Skip filler and noise phones */
	w = dict_wordid(dict, word);
	if (IS_WID(w) && (! dict_filler_word(dict, w))) {
	    make_dagnode (ref+n, dict_wordid(dict, word), sf, sf+nf-1);
	    n++;
	}
    }

    /* Add final sentinel */
    make_dagnode (ref+n, silwid, sf+nf, sf+nf);
    n++;

    /* Obtain uttid */
    len = strlen(word);
    assert ((word[0] == '(') && (word[len-1] == ')'));
    word[len-1] = '\0';
    strcpy (uttid, word+1);

    return n;
}


static void process_reffile (char *reffile)
{
    FILE *rfp;
    char *allpdir, allpfile[4096], line[16384], uttid[4096];
    int32 nref, nhyp;
    dagnode_t ref[MAX_UTT_LEN];
    dag_t *dag;
    int32 i, k;
    dpnode_t retval;
    int32 tot_ref, tot_corr, tot_hyp, tot_err;
    
    if ((rfp = fopen(reffile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", reffile);

    allpdir = (char *) cmd_ln_access ("-allp");
    
    tot_err = 0;
    tot_ref = 0;
    tot_hyp = 0;
    tot_corr = 0;

    while (fgets(line, sizeof(line), rfp) != NULL) {
	if ((nref = refline_parse (line, ref, uttid)) < 0)
	    E_FATAL("Bad line in file %s: %s\n", reffile, line);
	
	sprintf (allpfile, "%s/%s.allp", allpdir, uttid);
	E_INFO("Utt: %s\n", uttid);
	
	dag = allpfile_load (allpfile);
	
	if (dag) {
	    /* Append sentinel silwid node to end of DAG */
	    dag_append_sentinel (dag, silwid);
	    
	    /* Find best path (returns #errors/#correct and updates *nhyp) */
	    retval = dp (uttid, dict, dict->n_word, ref, nref, dag, &nhyp, 1);
	    
	    dag_destroy (dag);
	}
	
	tot_ref += nref-1;
	tot_hyp += nhyp;
	tot_err += retval.e;
	tot_corr += retval.c;

	printf("(%s) << %d ref; %d hyp; %d %.1f%% corr; %d %.1f%% err >>\n",
	       uttid, nref-1, nhyp,
	       retval.c, (nref > 1) ? (retval.c * 100.0) / (nref-1) : 0.0,
	       retval.e, (nref > 1) ? (retval.e * 100.0) / (nref-1) : 0.0);
	printf("== %7d ref; %7d hyp; %7d %5.1f%% corr; %6d %5.1f%% err; %s\n",
	       tot_ref, tot_hyp,
	       tot_corr, (tot_ref > 0) ? (tot_corr * 100.0) / tot_ref : 0.0,
	       tot_err, (tot_ref > 0) ? (tot_err * 100.0) / tot_ref : 0.0,
	       uttid);
	
	fflush (stderr);
	fflush (stdout);
    }

    fclose (rfp);
    
    printf("SUMMARY: %d ref; %d hyp; %d %.3f%% corr; %d %.3f%% err\n",
	   tot_ref, tot_hyp,
	   tot_corr, (tot_ref > 0) ? (tot_corr * 100.0) / tot_ref : 0.0,
	   tot_err, (tot_ref > 0) ? (tot_err * 100.0) / tot_ref : 0.0);
}


static arg_t arglist[] = {
    { "-mdef",
      CMD_LN_STRING,
      NULL,
      "Model definition input file" },
    { "-dict",
      CMD_LN_STRING,
      NULL,
      "Pronunciation dictionary input file (pronunciations not relevant)" },
    { "-fdict",
      CMD_LN_STRING,
      NULL,
      "Filler dictionary input file (pronunciations not relevant)" },
    { "-ref",
      CMD_LN_STRING,
      NULL,
      "Reference (lsn) time-segmented input file" },
    { "-allp",
      CMD_LN_STRING,
      ".",
      "Allphone decoding input directory" },
    
    { NULL, CMD_LN_INT32, NULL, NULL }
};


main (int32 argc, char *argv[])
{
    char *reffile, *mdeffile, *dictfile, *fdictfile;
    
    if (argc == 1) {
	cmd_ln_print_help (stderr, arglist);
	exit(0);
    }
    
    cmd_ln_parse (arglist, argc, argv);
    
    if ((mdeffile = (char *) cmd_ln_access ("-mdef")) == NULL)
	E_FATAL("-mdef argument missing\n");
    if ((dictfile = (char *) cmd_ln_access ("-dict")) == NULL)
	E_FATAL("-dict argument missing\n");
    if ((fdictfile = (char *) cmd_ln_access ("-fdict")) == NULL)
	E_FATAL("-fdict argument missing\n");
    if ((reffile = (char *) cmd_ln_access ("-ref")) == NULL)
	E_FATAL("-ref argument missing\n");

    unlimit();
    
    mdef = mdef_init (mdeffile);
    dict = dict_init (mdef, dictfile, fdictfile);
    silwid = dict_wordid (dict, "<sil>");
    
    process_reffile (reffile);
    
    exit(0);
}
