/*
 * segnbesterr.c -- Segmented N-best list oracle error rate (using DP alignment).
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
 * 21-Mar-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>

#include <s3.h>
#include <main/s3types.h>
#include <main/mdef.h>
#include <main/dict.h>

#include "line2wid.h"
#include "dp.h"


/* Homophones structures */
typedef struct hom_s {
    s3wid_t w1, w2;		/* w2 and w2 are considered equivalent */
    struct hom_s *next;
} hom_t;
static hom_t *homlist;

static mdef_t *mdef;
static dict_t *dict;
static s3wid_t startwid, finishwid, silwid;
static s3wid_t oovbegin;


#define MAX_UTT_LEN	1024


static s3wid_t hom_lookup (s3wid_t w)
{
    hom_t *h;
    
    for (h = homlist; h && (h->w1 != w); h = h->next);
    return (h ? h->w2 : BAD_WID);
}


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


typedef struct taillist_s {
    dagnode_t *d;
    struct taillist_s *next;
} taillist_t;


static dagnode_t *find_dagnode (taillist_t *taillist, s3wid_t wid, int32 sf)
{
    taillist_t *tl;
    
    for (tl = taillist; tl; tl = tl->next) {
	if ((tl->d->wid == wid) && (tl->d->sf == sf))
	    return tl->d;
	if (tl->d->sf < sf)
	    break;
    }
    
    return NULL;
}


static dag_t *segnbest_load (char *file)
{
    FILE *fp;
    dag_t *dag;
    char line[4096], wd[4096], *lp;
    int32 len, scr, n, sf;
    taillist_t *taillist, *tl;
    s3wid_t wid;
    dagnode_t *d, *prevd;
    
    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen (file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return NULL;
    }

    /* Create new DAG */
    dag = ckd_calloc (1, sizeof(dag_t));
    dag->node_sf = (dagnode_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(dagnode_t *));
    dag->nnode = 0;
    dag->nlink = 0;
    dag->nfrm = 0;

    /* Add the <s>.0 node at the beginning of the DAG */
    d = (dagnode_t *) mymalloc (sizeof(dagnode_t));
    make_dagnode (d, startwid, 0, 0);
    dag->node_sf[0] = d;
    dag->nfrm = 1;
    dag->nnode = 1;
    
    dag->entry.src = NULL;
    dag->entry.dst = d;
    dag->entry.next = NULL;
    
    dag->exit.src = NULL;
    dag->exit.dst = d;
    dag->exit.next = NULL;
    
    taillist = (taillist_t *) ckd_calloc (1, sizeof(taillist_t));
    taillist->d = d;
    taillist->next = NULL;
    
    /* Read #frames */
    if (fgets (line, sizeof(line), fp) == NULL)
	E_FATAL("Premature EOF(%s)\n", file);
    if ((sscanf (line, "%d %s", &dag->nfrm, wd) != 2) || (strcmp (wd, "frames") != 0))
	E_FATAL("Bad header line in %s: %s\n", file, line);
    
    /* Read data */
    while (fgets (line, sizeof(line), fp) != NULL) {
	lp = line;
	if (sscanf (lp, "%d%n", &scr, &len) != 1) {
	    if ((strcmp (line, "End\n") != 0) && (strcmp (line, "Failed\n") != 0))
		E_FATAL("Bad line: %s\n", line);
	    break;
	}
	lp += len;
	
	if (sscanf (lp, "%d %s%n", &sf, wd, &len) != 2)
	    E_FATAL("Bad line: %s\n", line);
	lp += len;

	wid = dict_wordid (dict, wd);
	if (NOT_WID(wid))
	    E_FATAL("Bad word(%s) in line: %s\n", wd, line);
	
	prevd = find_dagnode (taillist, wid, sf);
	if (! prevd) {
	    E_ERROR("Cannot find first dagnode for: %s\n", line);
	    continue;
	}
	
	n = 0;
	while (sscanf (lp, "%d %s%n", &sf, wd, &len) == 2) {
	    lp += len;
	    
	    wid = dict_wordid (dict, wd);
	    if (NOT_WID(wid))
		E_FATAL("Bad word(%s) in line: %s\n", wd, line);

	    d = find_dagnode (taillist, wid, sf);
	    if (! d) {
		d = (dagnode_t *) mymalloc (sizeof(dagnode_t));
		make_dagnode (d, wid, sf, sf);
		d->next = dag->node_sf[sf];
		dag->node_sf[sf] = d;
		
		dag->nnode++;
	    }
	    
	    dag_link (prevd, d);
	    dag->nlink++;
	    prevd = d;
	    
	    n++;
	}
	assert (n > 0);
	
	/* Reached end of line */
	if (! find_dagnode (taillist, prevd->wid, prevd->sf)) {
	    tl = (taillist_t *) ckd_calloc (1, sizeof(taillist_t));
	    tl->d = prevd;
	    tl->next = taillist;
	    taillist = tl;
	}
    }

    fclose (fp);
    
    /* Create a dummy final node and make it the exit node */
    d = (dagnode_t *) mymalloc (sizeof(dagnode_t));
    make_dagnode (d, silwid, dag->nfrm-1, dag->nfrm-1);
    assert (dag->node_sf[dag->nfrm-1] == NULL);
    d->next = NULL;
    dag->node_sf[dag->nfrm-1] = d;
    dag->nnode++;
    for (tl = taillist; tl && (tl->d->sf == taillist->d->sf); tl = tl->next) {
	dag_link (tl->d, d);
	dag->nlink++;
    }
    dag->exit.dst = d;
    
    while (taillist) {
	tl = taillist->next;
	ckd_free (taillist);
	taillist = tl;
    }
    
    E_INFO("%s: %d frames, %d nodes, %d links\n", file, dag->nfrm, dag->nnode, dag->nlink);
#if 0
    dag_dump (dag, dict);
#endif

    return dag;
}


static int32 refline2wds (char *line, dagnode_t *ref, int32 *noov, char *uttid)
{
    int32 i, n, k;
    s3wid_t w, wid[MAX_UTT_LEN];
    
    n = 0;
    uttid[0] = '\0';
    *noov = 0;
    
    if ((n = line2wid (dict, line, wid, MAX_UTT_LEN-1, 1, uttid)) < 0)
	E_FATAL("Error in parsing ref line: %s\n", line);
    wid[n++] = silwid;
    
    for (i = 0; i < n; i++) {
	if (dict_filler_word (dict, wid[i]) && (i < n-1))
	    E_FATAL("Filler word (%s) in ref: %s\n", dict_wordstr(dict, wid[i]), line);
	
	if (wid[i] >= oovbegin) {
	    /* Perhaps one of a homophone pair */
	    w = hom_lookup (wid[i]);
	    if (IS_WID(w))
		wid[i] = w;
	    if (wid[i] >= oovbegin)
		(*noov)++;
	}

	make_dagnode (ref+i, wid[i], i, i);
    }

    return n;
}


static void process_reffile (char *reffile)
{
    FILE *rfp;
    char line[16384], uttid[4096], file[4096], lc_uttid[4096];
    int32 i, k;
    dagnode_t ref[MAX_UTT_LEN];
    int32 nref, noov, nhyp;
    int32 tot_err, tot_ref, tot_corr, tot_oov, tot_hyp;
    dag_t *dag;
    dpnode_t retval;
    timing_t *tm;
    char *latdir;
    
    if ((rfp = fopen(reffile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", reffile);

    latdir = (char *) cmd_ln_access ("-latdir");
    
    tot_err = 0;
    tot_ref = 0;
    tot_hyp = 0;
    tot_corr = 0;
    tot_oov = 0;
    
    tm = timing_new ("Utt");
    
    while (fgets(line, sizeof(line), rfp) != NULL) {
	timing_reset (tm);
	timing_start (tm);

	if ((nref = refline2wds (line, ref, &noov, uttid)) < 0)
	    E_FATAL("Bad line in file %s: %s\n", reffile, line);
	
	/* Read lattice or hypfile, whichever is specified */
	sprintf (file, "%s/%s.nbest", latdir, uttid);
	
	dag = segnbest_load (file);
	if (! dag) {
	    /* Try lower casing uttid */
	    strcpy (lc_uttid, uttid);
	    lcase (lc_uttid);
	    sprintf (file, "%s/%s.nbest", latdir, lc_uttid);
	    dag = segnbest_load (file);
	}
	
	if (dag) {
	    /* Append sentinel silwid node to end of DAG */
	    dag_append_sentinel (dag, silwid);
	    
	    /* Find best path (returns #errors/#correct and updates *nhyp) */
	    retval = dp (uttid, dict, oovbegin, ref, nref, dag, &nhyp, 0, 1);
	    
	    dag_destroy (dag);
	} else {
	    retval.c = 0;
	    retval.e = nref-1;
	    nhyp = 0;
	}
	
	timing_stop (tm);
	
	tot_ref += nref-1;
	tot_hyp += nhyp;
	tot_err += retval.e;
	tot_corr += retval.c;
	tot_oov += noov;

	printf("(%s) << %d ref; %d %.1f%% oov; %d hyp; %d %.1f%% corr; %d %.1f%% err; %.1fs CPU >>\n",
	       uttid, nref-1,
	       noov, (nref > 1) ? (noov * 100.0) / (nref-1) : 0.0,
	       nhyp,
	       retval.c, (nref > 1) ? (retval.c * 100.0) / (nref-1) : 0.0,
	       retval.e, (nref > 1) ? (retval.e * 100.0) / (nref-1) : 0.0,
	       tm->t_cpu);

	printf("== %7d ref; %5d %5.1f%% oov; %7d hyp; %7d %5.1f%% corr; %6d %5.1f%% err; %5.1fs CPU; %s\n",
	       tot_ref,
	       tot_oov, (tot_ref > 0) ? (tot_oov * 100.0) / tot_ref : 0.0,
	       tot_hyp,
	       tot_corr, (tot_ref > 0) ? (tot_corr * 100.0) / tot_ref : 0.0,
	       tot_err, (tot_ref > 0) ? (tot_err * 100.0) / tot_ref : 0.0,
	       tm->t_tot_cpu,
	       uttid);

	fflush (stderr);
	fflush (stdout);
    }
    
    fclose (rfp);
    
    printf("SUMMARY: %d ref; %d %.3f%% oov; %d hyp; %d %.3f%% corr; %d %.3f%% err; %.1fs CPU\n",
	   tot_ref,
	   tot_oov, (tot_ref > 0) ? (tot_oov * 100.0) / tot_ref : 0.0,
	   tot_hyp,
	   tot_corr, (tot_ref > 0) ? (tot_corr * 100.0) / tot_ref : 0.0,
	   tot_err, (tot_ref > 0) ? (tot_err * 100.0) / tot_ref : 0.0,
	   tm->t_tot_cpu);
}


static void homfile_load (char *file)
{
    FILE *fp;
    char line[16380], w1[4096], w2[4096];
    int32 k, n;
    s3wid_t wid1, wid2;
    s3cipid_t ci[1];
    hom_t *h;
    
    E_INFO("Reading homophones file %s\n", file);
    if ((fp = fopen(file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);
    
    ci[0] = (s3cipid_t) 0;	/* Dummy */
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if ((k = sscanf (line, "%s %s", w1, w2)) == 2) {
	    wid1 = dict_wordid (dict, w1);
	    if (NOT_WID(wid1)) {
		E_INFO("Adding %s to dictionary\n", w1);
		wid1 = dict_add_word (dict, w1, ci, 1);
		if (NOT_WID(wid1))
		    E_FATAL("dict_add_word(%s) failed\n", w1);
	    }
	    
	    wid2 = dict_wordid (dict, w2);
	    if ((NOT_WID(wid2)) || (wid2 >= oovbegin))
		E_FATAL("%s not in dictionary\n", w2);

	    h = (hom_t *) mymalloc (sizeof(hom_t));
	    h->w1 = wid1;
	    h->w2 = wid2;
	    h->next = homlist;
	    homlist = h;
	    
	    n++;
	} else
	    E_FATAL("Bad homophones line: %s\n", line);
    }
    
    E_INFO("%d homophone pairs read\n", n);
    
    fclose (fp);
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
      "Filler word pronunciation dictionary input file (pronunciations not relevant)" },
    { "-ref",
      CMD_LN_STRING,
      NULL,
      "Input reference (correct) file" },
    { "-hom",
      CMD_LN_STRING,
      NULL,
      "Homophones input file" },
    { "-sil",
      CMD_LN_STRING,
      "<sil>",
      "Silence word" },
    { "-latdir",
      CMD_LN_STRING,
      ".",
      "Input lattice directory (-latdir/-hyp mutually exclusive)" },
    
    { NULL, CMD_LN_INT32, NULL, NULL }
};


main (int32 argc, char *argv[])
{
    char *reffile, *mdeffile, *dictfile, *fdictfile, *homfile;
    
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
    if (mdef->n_ciphone <= 0)
	E_FATAL("0 CIphones in %s\n", mdeffile);
    
    dict = dict_init (mdef, dictfile, fdictfile);
    oovbegin = dict->n_word;
    
    startwid = dict_wordid (dict, "<s>");
    finishwid = dict_wordid (dict, "</s>");
    silwid = dict_wordid (dict, (char *) cmd_ln_access("-sil"));
    assert (dict_filler_word (dict, silwid));
    
    homlist = NULL;
    if ((homfile = (char *) cmd_ln_access ("-hom")) != NULL)
	homfile_load (homfile);
    
    process_reffile (reffile);
    
#if (0 && (! WIN32))
    fflush (stdout);
    fflush (stderr);
    system ("ps aguxwww | grep dpalign");
#endif

    exit(0);
}
