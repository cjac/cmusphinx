/*
 * dpalign.c -- DP alignment of various kinds.
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


/*
 * Return the next parameter value in DAG file.  If not found, return -1.
 */
static int32 dag_param_read (FILE *fp, char *param)
{
    char line[16384], wd[4096];
    int32 n;
    
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] != '#') {
	    if ((sscanf (line, "%s %d", wd, &n) == 2) && (strcmp (wd, param) == 0))
		return n;
	    return -1;
	}
    }
    return -1;
}


/*
 * Load a DAG from a file: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * Return value: ptr to DAG structure if successful; NULL otherwise.
 */
dag_t *dag_load (char *file)
{
    FILE *fp;
    dag_t *dag;
    int32 seqid, sf, fef, lef, ef;
    char line[16384], wd[4096];
    int32 i, j, k;
    dagnode_t *d, *d2, **darray;
    s3wid_t w;
    int32 fudge, min_ef_range;
    
    E_INFO("Reading DAG file: %s\n", file);
    if ((fp = fopen (file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return NULL;
    }

    dag = ckd_calloc (1, sizeof(dag_t));
    dag->node_sf = (dagnode_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(dagnode_t *));
    dag->nnode = 0;
    dag->nlink = 0;
    dag->nfrm = 0;
    
    /* Read Frames parameter */
    if ((dag->nfrm = dag_param_read (fp, "Frames")) <= 0)
	E_FATAL("%s: Frames parameter missing or invalid\n", file);
    
    /* Read Nodes parameter */
    if ((dag->nnode = dag_param_read (fp, "Nodes")) <= 0)
	E_FATAL("%s: Nodes parameter missing or invalid\n", file);
    
    /* Read nodes */
    darray = (dagnode_t **) ckd_calloc (dag->nnode, sizeof(dagnode_t *));
    for (i = 0; i < dag->nnode; i++) {
	if (fgets (line, sizeof(line), fp) == NULL)
	    E_FATAL("%s: Premature EOF\n", file);
	
	if ((k = sscanf (line, "%d %s %d %d %d", &seqid, wd, &sf, &fef, &lef)) != 5)
	    E_FATAL("%s: Bad line: %s\n", file, line);
	if ((sf < 0) || (sf >= dag->nfrm) ||
	    (fef < 0) || ( fef >= dag->nfrm) ||
	    (lef < 0) || ( lef >= dag->nfrm))
	    E_FATAL("%s: Bad frame info: %s\n", file, line);
	
	w = dict_wordid (dict, wd);
	if (NOT_WID(w))
	    E_FATAL("%s: Unknown word: %s\n", file, line);
	
	if (seqid != i)
	    E_FATAL("%s: Seqno error: %s\n", file, line);
	
	d = (dagnode_t *) mymalloc (sizeof(dagnode_t));
	darray[i] = d;
	
	d->wid = w;
	d->seqid = seqid;
	d->reachable = 0;
	d->sf = sf;
	d->fef = fef;
	d->lef = lef;
	d->succlist = NULL;
	d->predlist = NULL;
	d->next = dag->node_sf[sf];
	dag->node_sf[sf] = d;
    }

    /* Read initial node ID */
    if (((k = dag_param_read (fp, "Initial")) < 0) || (k >= dag->nnode))
	E_FATAL("%s: Initial node parameter missing or invalid\n", file);
    dag->entry.src = NULL;
    dag->entry.dst = darray[k];
    dag->entry.next = NULL;

    /* Read final node ID */
    if (((k = dag_param_read (fp, "Final")) < 0) || (k >= dag->nnode))
	E_FATAL("%s: Final node parameter missing or invalid\n", file);
    dag->exit.src = NULL;
    dag->exit.dst = darray[k];
    dag->exit.next = NULL;
    
    ckd_free (darray);	/* That's all I need darray for??? */

    /* Read bestsegscore entries; just to make sure all nodes have been read */
    if ((k = dag_param_read (fp, "BestSegAscr")) < 0)
	E_FATAL("%s: BestSegAscr parameter missing\n", file);
    fclose (fp);
    
    /*
     * Build edges based on time-adjacency.
     * min_ef_range = min. endframes that a node must persist for it to be not ignored.
     * fudge = #frames to be fudged around word begin times
     */
    min_ef_range = *((int32 *) cmd_ln_access ("-min_endfr"));
    fudge = *((int32 *) cmd_ln_access ("-dagfudge"));
    if (min_ef_range <= 0)
	E_FATAL("Bad min_endfr argument: %d\n", min_ef_range);
    if ((fudge < 0) || (fudge > 2))
	E_FATAL("Bad dagfudge argument: %d\n", fudge);

    dag->nlink = 0;
    for (sf = 0; sf < dag->nfrm; sf++) {
	for (d = dag->node_sf[sf]; d; d = d->next) {
	    if ((d->lef - d->fef < min_ef_range - 1) && (d != dag->entry.dst))
		continue;
	    if (d->wid == finishwid)
		continue;
	    
	    for (ef = d->fef - fudge + 1; ef <= d->lef + 1; ef++) {
		for (d2 = dag->node_sf[ef]; d2; d2 = d2->next) {
		    if ((d2->lef - d2->fef < min_ef_range - 1) && (d2 != dag->exit.dst))
			continue;
		    
		    dag_link (d, d2);
		    dag->nlink++;
		}
	    }
	}
    }
    
    return dag;
}


static void wid2dagnode (dagnode_t *d, int32 id, s3wid_t wid)
{
    /* Turn it into a dagnode */
    d->wid = wid;
    d->seqid = id;
    d->reachable = 0;
    d->sf = id;
    d->fef = id;
    d->lef = id;
    d->next = NULL;
    d->succlist = NULL;
    d->predlist = NULL;
}


/*
 * Create a degenerate DAG (linear sequence of nodes) for the given hyp line.
 * The DAG contains a terminal sentinel silwid node.
 */
static dag_t *hypline2dag (char *ref_uttid, char *line)
{
    char junk1[4096], junk2[4096], uttid[4096];
    s3wid_t wid[MAX_UTT_LEN];
    int32 i, n;
    dag_t *dag;
    dagnode_t *d;
    
    if ((n = line2wid (dict, line, wid, MAX_UTT_LEN-1, 0, uttid)) < 0)
	E_FATAL("Error in parsing hyp line: %s\n", line);
    
    /* Verify uttid with ref_uttid */
    if (strcmp (uttid, ref_uttid) != 0) {
	strcpy (junk1, uttid);
	ucase (junk1);
	strcpy (junk2, ref_uttid);
	ucase (junk2);
	if (strcmp (junk1, junk2) != 0)
	    E_ERROR("Uttid mismatch: %s(ref), %s(hyp)\n", ref_uttid, uttid);
    }

    /* Build DAG from word sequence */
    dag = ckd_calloc (1, sizeof(dag_t));
    dag->node_sf = (dagnode_t **) ckd_calloc (S3_MAX_FRAMES, sizeof(dagnode_t *));
    dag->nnode = 0;
    dag->nfrm = 0;
    dag->nlink = 0;

    for (i = 0; i < n; i++) {
	if ((NOT_WID(wid[i])) || (wid[i] >= oovbegin))
	    E_FATAL("%s: Unknown word in line: %s\n", uttid, line);
	
	/* Create DAG node for word */
	d = (dagnode_t *) mymalloc (sizeof(dagnode_t));
	wid2dagnode (d, i, wid[i]);
	
	dag->node_sf[i] = d;
	if (i > 0) {
	    dag_link (dag->node_sf[i-1], d);
	    dag->nlink++;
	}
	
	dag->nnode++;
    }
    dag->nfrm = dag->nnode;
    
    dag->entry.src = NULL;
    dag->entry.dst = dag->node_sf[0];
    dag->entry.next = NULL;

    dag->exit.src = NULL;
    dag->exit.dst = dag->node_sf[dag->nnode - 1];
    dag->exit.next = NULL;
    
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

	wid2dagnode (ref+i, i, wid[i]);
    }

    return n;
}


static void process_reffile (char *reffile)
{
    FILE *rfp, *hfp;
    char line[16384], uttid[4096], file[4096], lc_uttid[4096];
    int32 i, k;
    dagnode_t ref[MAX_UTT_LEN];
    int32 nref, noov, nhyp;
    int32 tot_err, tot_ref, tot_corr, tot_oov, tot_hyp;
    dag_t *dag;
    dpnode_t retval;
    ptmr_t *tm;
    char *latdir, *hypfile;
    
    if ((rfp = fopen(reffile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", reffile);

    latdir = (char *) cmd_ln_access ("-latdir");
    hypfile = (char *) cmd_ln_access ("-hyp");
    
    if ((! latdir) && (! hypfile))
	E_FATAL("Both -latdir and -hyp arguments missing\n");
    if (latdir && hypfile)
	E_FATAL("-latdir and -hyp arguments are mutually exclusive\n");

    hfp = NULL;
    if (hypfile) {
	if ((hfp = fopen(hypfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", hypfile);
    }
    
    tot_err = 0;
    tot_ref = 0;
    tot_hyp = 0;
    tot_corr = 0;
    tot_oov = 0;
    
    tm = (ptmr_t *) ckd_calloc (1, sizeof(ptmr_t));
    
    while (fgets(line, sizeof(line), rfp) != NULL) {
	ptmr_reset (tm);
	ptmr_start (tm);

	if ((nref = refline2wds (line, ref, &noov, uttid)) < 0)
	    E_FATAL("Bad line in file %s: %s\n", reffile, line);
	
	/* Read lattice or hypfile, whichever is specified */
	if (latdir) {
	    sprintf (file, "%s/%s.lat", latdir, uttid);
	    
	    dag = dag_load (file);
	    if (! dag) {
		/* Try lower casing uttid */
		strcpy (lc_uttid, uttid);
		lcase (lc_uttid);
		sprintf (file, "%s/%s.lat", latdir, lc_uttid);
		dag = dag_load (file);
	    }
	} else {
	    if (fgets(line, sizeof(line), hfp) == NULL)
		E_FATAL("Premature EOF(%s) at uttid %s\n", hypfile, uttid);

	    dag = hypline2dag (uttid, line);
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
	
	ptmr_stop (tm);
	
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
    if (hfp)
	fclose (hfp);
    
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
      NULL,
      "Input lattice directory (-latdir/-hyp mutually exclusive)" },
    { "-hyp",
      CMD_LN_STRING,
      NULL,
      "Input hypothesis file (-latdir/-hyp mutually exclusive)" },
    { "-min_endfr",
      CMD_LN_INT32,
      "3",
      "Min. endframes that a node must persist in order for it to be not ignored" },
    { "-dagfudge",
      CMD_LN_INT32,
      "2",
      "#Frames fudged between a node's startframe and its precessor's earliest endframe" },
    
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
