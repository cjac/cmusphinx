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
 * pronerralign.c -- DP alignment of pronunciation errors and dictionary pronunciations.
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
 * Takes a pronunciation error file created by pronerr, sorted and counted,
 * each line of the form:
 *     <count> <word-sequence> => [(LC-phone)] pronunciation [(RC-phone)]
 * (Note that LC and RC-phones are optional.)
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


static dag_t *dag_add_word (dag_t *dag_in, s3wid_t wid)
{
    dag_t *dag;
    dagnode_t *prev_tail, *new_tail, *d, *pd;
    int32 pronlen, p;
    char str[4096];
    s3wid_t w;
    
    if (! dag_in) {
	dag = ckd_calloc (1, sizeof(dag_t));
	dag->node_sf = (dagnode_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(dagnode_t *));
	dag->nlink = 0;

	/* Add a dummy start word at the beginning */
	d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));
	make_dagnode (d, silwid, 0, 0);
	dag->node_sf[0] = d;
	dag->nfrm = 1;
	dag->nnode = 1;
	
	dag->entry.src = NULL;
	dag->entry.dst = d;
	dag->entry.next = NULL;
	
	dag->exit.src = NULL;
	dag->exit.dst = d;
	dag->exit.next = NULL;
    } else
	dag = dag_in;
    
    prev_tail = dag->exit.dst;    /* Becomes the head for the new word */
    
    new_tail = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));
    make_dagnode (new_tail, silwid, 0, 0);	/* Temporary sf,ef values */
    
    assert (IS_WID(wid));
    for (; IS_WID(wid); wid = dict->word[wid].alt) {
	pronlen = dict->word[wid].pronlen;
	assert (pronlen > 0);
	
	pd = prev_tail;
	for (p = 0; p < pronlen; p++) {
	    d = (dagnode_t *) listelem_alloc (sizeof(dagnode_t));
	    strcpy (str, mdef_ciphone_str (mdef, dict->word[wid].ciphone[p]));
	    strcat (str, " ");
	    w = dict_wordid (dict, str);
	    assert (IS_WID(w));
	    
	    make_dagnode (d, w, dag->nfrm, dag->nfrm);
	    dag->node_sf[dag->nfrm] = d;
	    
	    dag_link (pd, d);
	    dag->nlink++;
	    dag->nnode++;
	    dag->nfrm++;	/* Dummy */
	    pd = d;
	}
	
	dag_link (pd, new_tail);
	dag->nlink++;
    }
    
    dag->node_sf[dag->nfrm] = new_tail;
    new_tail->sf = dag->nfrm;
    new_tail->fef = dag->nfrm;
    new_tail->lef = dag->nfrm;
    dag->nnode++;
    dag->nfrm++;
    
    dag->exit.dst = new_tail;

    return dag;
}


static void process_line (char *line)
{
    int32 len, k, n_word, n_ref, n_hyp;
    char word[4096], *lp;
    dag_t *dag;
    dagnode_t ref[MAX_UTT_LEN];
    s3wid_t w;
    dpnode_t retval;
    
    lp = line;
    if (sscanf (lp, "%d%n", &k, &len) != 1)
	E_FATAL("Failed to read count: %s\n", line);
    lp += len;
    
    /* Read words */
    dag = NULL;
    n_word = 0;
    while (sscanf (lp, "%s%n", word, &len) == 1) {
	lp += len;
	
	if (strcmp (word, "=>") == 0)
	    break;
	
	dag = dag_add_word (dag, dict_wordid (dict, word));
	n_word++;
    }
    if (n_word == 0)
	return;

    dag_append_sentinel (dag, silwid);

    n_ref = 0;
    while (sscanf (lp, "%s%n", word, &len) == 1) {
	lp += len;
	
	ucase (word);
	strcat (word, " ");
	
	w = dict_wordid (dict, word);
	if (NOT_WID(w)) {
	    if ((word[0] != '(') || (word[strlen(word)-2] != ')'))
		E_FATAL("Bad phone: %s in line %s\n", word, line);
	    continue;
	}
	
	make_dagnode (&(ref[n_ref]), w, n_ref, n_ref);
	n_ref++;
    }

    if (n_ref == 0) {
	E_INFO("Skipped: %s", line);
	dag_destroy (dag);
	return;
    }
    
    /* Add final sentinel */
    make_dagnode (&(ref[n_ref]), silwid, n_ref, n_ref);
    n_ref++;
    
    retval = dp ("", dict, dict->n_word, ref, n_ref, dag, &n_hyp, 0, 0);

    if (retval.e > 0)
	printf ("%s", line);
    else
	E_INFO("Skipped: %s", line);
    
    dag_destroy (dag);
}


static void add_ciphone_dict ( void )
{
    int32 i;
    char word[1024];
    s3wid_t w;
    
    for (i = 0; i < mdef->n_ciphone; i++) {
	strcpy (word, mdef_ciphone_str (mdef, i));
	strcat (word, " ");
	w = dict_add_word (dict, word, NULL, 0);
	if (NOT_WID(w))
	    E_FATAL("Failed to add '%s' to dictionary\n", word);
    }
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
    
    { NULL, CMD_LN_INT32, NULL, NULL }
};


main (int32 argc, char *argv[])
{
    char *mdeffile, *dictfile, *fdictfile;
    char line[16380];
    
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

    unlimit();
    
    mdef = mdef_init (mdeffile);
    dict = dict_init (mdef, dictfile, fdictfile);
    silwid = dict_wordid (dict, "<sil>");
    add_ciphone_dict ();
    
    while (fgets (line, sizeof(line), stdin) != NULL)
	process_line (line);
    
    exit(0);
}
