/*
 * pronerr.c -- Obtain word/phrase pronunciations from DP alignment of allphone and
 * 		correct phone sequences.
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
 * 27-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


/*
 * Inputs:
 * 	Main and filler (noise-word) pronunciation dictionaries.
 * 	.lsn file: exact, correct word sequence for each utterance.
 * 	.dp file: DP alignment of allphone recognition against correct phone sequence.
 * Output:
 * 	"Observed" pronunciations for words/phrases that had errors in DP alignment.
 * NOTES:
 * 	- Silence and noise words are ignored.
 * 	- Pure insertions between words are ignored.
 */


#include <libutil/libutil.h>

#include <s3.h>
#include <main/s3types.h>
#include <main/mdef.h>
#include <main/dict.h>

#include "line2wid.h"


static dict_t *dict;	/* The pronunciation dictionary */
static mdef_t *mdef;	/* Needed by dictionary */

typedef struct {
    int32 s, e;		/* Start and end points in allphone recognition for this word.
			   Either or both may be -ve if not known */
    int32 err;		/* Whether any error occurred INSIDE the word */
} wseg_t;

#define MAX_UTT_LEN	2040


static void pronerr_output (char *id, s3wid_t *ref, int32 nref,
			    wseg_t *wseg, s3cipid_t *ap, int8 *ap_err,
			    int32 ws, int32 we, int32 ps, int32 pe)
{
    int32 j;
    s3wid_t rcwid, lcwid;
    char str[4096];
    
    /* Word sequence for region in error */
    sprintf (str, "%s", dict_wordstr (dict, dict_basewid(dict, ref[ws])));
    for (j = ws+1; j <= we; j++) {
	strcat (str, " ");
	strcat (str, dict_wordstr (dict, dict_basewid(dict, ref[j])));
    }
    printf ("%-22s\t=>\t", str);

    /* Print left context phone */
    /*lcwid = ((wseg[ws].s < 0) && (ws > 0) && IS_WID(ref[ws-1])) ? ref[ws-1] : BAD_WID;*/
    lcwid = (ws > 0) ? ref[ws-1] : BAD_WID;
    if (IS_WID(lcwid)) {
	j = dict->word[lcwid].pronlen - 1;
	sprintf (str, "(%s)", mdef_ciphone_str (mdef, dict->word[lcwid].ciphone[j]));
    } else
	strcpy (str, "()");
    printf ("%-5s", str);
    
    /* Phone sequence for region in error */
    for (j = ps; j <= pe; j++) {
	strcpy (str, mdef_ciphone_str (mdef, ap[j]));
	if (ap_err[j])
	    ucase (str);
	else
	    lcase (str);
	
	printf (" %s", str);
    }
    
    /* Right context if ending in error */
    /* rcwid = ((wseg[we].e < 0) && IS_WID(ref[we+1])) ? ref[we+1] : BAD_WID; */
    rcwid = ref[we+1];
    if (IS_WID(rcwid))
	printf ("\t(%s)", mdef_ciphone_str (mdef, dict->word[rcwid].ciphone[0]));
    else
	printf ("\t()");

    printf (" ( %s )\n", id);
}


static void pronerr (char *id, s3wid_t *ref, int32 nref, wseg_t *wseg,
		     s3cipid_t *ap, int8 *ap_err)
{
    int32 last_word_output, last_phone_output;
    int32 i;
    
    last_word_output = -1;
    last_phone_output = -1;
    
    for (i = 0; i < nref; i++) {
	if (wseg[i].s >= 0) {
	    /* Terminate preceding error segment, if any */
	    if (last_word_output < i-1) {
		pronerr_output (id, ref, nref, wseg, ap, ap_err,
				last_word_output+1, i-1,
				last_phone_output+1, wseg[i].s-1);
	    }

	    last_word_output = i-1;
	    last_phone_output = wseg[i].s-1;
	}
	
	if (wseg[i].e >= 0) {
	    /* Terminate upto end of this segment */
	    /* if ((wseg[i].s < 0) || wseg[i].err) { */
	    if (i < nref-1) {	/* Otherwise it's the dummy sentinel at the end */
		pronerr_output (id, ref, nref, wseg, ap, ap_err,
				last_word_output+1, i,
				last_phone_output+1, wseg[i].e);
	    }
	    
	    last_word_output = i;
	    last_phone_output = wseg[i].e;
	}
    }

    fflush (stdout);
}


static wseg_t *line2wseg (char *line, s3wid_t *ref,
			  s3cipid_t *ap, int8 *ap_err, int32 aplen, char *id)
{
    char word[1024], uttid[1024], *lp;
    int32 i, k, n_hypci, n_refwd, n_refci, pronlen;
    s3cipid_t ci;
    typedef enum {CORR=0, REFERR=1, HYPERR=2} state_t;
    state_t state;
    static wseg_t *wseg = NULL;
    
    if (! wseg)
	wseg = (wseg_t *) ckd_calloc (MAX_UTT_LEN, sizeof(wseg_t));

    lp = line;
    n_hypci = n_refci = pronlen = 0;
    n_refwd = -1;
    uttid[0] = '\0';
    state = CORR;
    
    while (sscanf (lp, "%s%n", word, &k) == 1) {
	lp += k;
	
	if (is_uttid (word, uttid))
	    break;
	
	if (strcmp (word, "[[") == 0) {
	    if (state != CORR)
		E_FATAL("%s: Illegal [[\n", id);
	    state = REFERR;
	    if (n_refci < pronlen)
		wseg[n_refwd].err = 1;
	} else if (strcmp (word, "]]") == 0) {
	    if (state != HYPERR)
		E_FATAL("%s: Illegal ]]\n", id);
	    state = CORR;
	} else if (strcmp (word, "=>") == 0) {
	    if (state != REFERR)
		E_FATAL("%s: Illegal =>\n", id);
	    state = HYPERR;
	} else {
	    ci = mdef_ciphone_id (mdef, word);
	    if (NOT_CIPID(ci))
		E_FATAL("%s: Unknown CIphone %s\n", id, word);
	    
	    if (state != HYPERR) {	/* Check if matches next pron in ref word */
		if (n_refci >= pronlen) {
		    assert (n_refci == pronlen);
		    n_refwd++;
		    pronlen = dict->word[ref[n_refwd]].pronlen;
		    assert (pronlen > 0);

		    wseg[n_refwd].s = (state == CORR) ? n_hypci : -1;
		    wseg[n_refwd].e = -1;
		    wseg[n_refwd].err = 0;
		    
		    n_refci = 0;
		}
		if (NOT_WID(ref[n_refwd]))
		    E_FATAL("%s: Premature end of ref wid\n", id);

		if (dict->word[ref[n_refwd]].ciphone[n_refci] != ci)
		    E_FATAL("%s: CIphone mismatch at word %d, ciphone %d\n",
			    id, n_refwd, n_refci);
		n_refci++;
		if ((n_refci == pronlen) && (state == CORR))
		    wseg[n_refwd].e = n_hypci;

		if (state != CORR)
		    wseg[n_refwd].err = 1;
	    }
	    
	    if (state != REFERR) {
		if (n_hypci >= aplen)
		    E_FATAL("%s: Too many CIphones: >%d\n", id, aplen);
		ap[n_hypci] = ci;
		ap_err[n_hypci] = (state == CORR) ? 0 : 1;
		n_hypci++;
	    }
	}
    }
    assert (n_refci == pronlen);
    n_refwd++;
    assert (NOT_WID(ref[n_refwd]));
    wseg[n_refwd].s = wseg[n_refwd].e = n_hypci;
    wseg[n_refwd].err = 0;
    
    ap[n_hypci] = BAD_CIPID;
    ap_err[n_hypci] = 1;
    
    if (strcmp (uttid, id) != 0)
	E_FATAL("Uttid mismatch: %s expected, %s found\n", id, uttid);

#if 0
    for (i = 0; IS_WID(ref[i]); i++) {
	printf ("%s: %4d %4d %d %s\n", id, wseg[i].s, wseg[i].e, wseg[i].err,
		dict_wordstr (dict, ref[i]));
    }
#endif

    return wseg;
}


static void process_reffile (char *reffile)
{
    FILE *rfp, *afp;
    char line[16384], uttid[4096];
    int32 i, k, nref;
    s3wid_t ref[MAX_UTT_LEN];
    s3cipid_t ap[MAX_UTT_LEN];
    int8 ap_err[MAX_UTT_LEN];
    wseg_t *wseg;
    
    if ((rfp = fopen(reffile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", reffile);
    
    afp = stdin;	/* DP file read in from stdin */
    
    while (fgets(line, sizeof(line), rfp) != NULL) {
	if ((nref = line2wid (dict, line, ref, MAX_UTT_LEN-1, 0, uttid)) < 0)
	    E_FATAL("Bad line in file %s: %s\n", reffile, line);

	/* Check for unknown words; remove filler words; terminate with BAD_WID */
	k = 0;
	for (i = 0; i < nref; i++) {
	    if (NOT_WID(ref[i]))
		E_FATAL("Unknown word at position %d in line: %s\n", i, line);
	    if (! dict_filler_word (dict, ref[i]))
		ref[k++] = ref[i];
	}
	ref[k++] = BAD_WID;
	nref = k;
	
	/* Build wseg map for DP line */
	if (fgets (line, sizeof(line), afp) == NULL)
	    E_FATAL("Unexpected EOF(DP-file)\n");
	wseg = line2wseg (line, ref, ap, ap_err, MAX_UTT_LEN-1, uttid);
	
	pronerr (uttid, ref, nref, wseg, ap, ap_err);
    }
    
    fclose (rfp);
}


static arg_t arglist[] = {
    { "-mdef",
      CMD_LN_STRING,
      NULL,
      "Model definition input file" },
    { "-dict",
      CMD_LN_STRING,
      NULL,
      "Pronunciation dictionary input file" },
    { "-fdict",
      CMD_LN_STRING,
      NULL,
      "Filler word pronunciation dictionary input file" },
    { "-ref",
      CMD_LN_STRING,
      NULL,
      "Reference input file (.trans from forced alignment)" },
    
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
    if (mdef->n_ciphone <= 0)
	E_FATAL("0 CIphones in %s\n", mdeffile);
    
    dict = dict_init (mdef, dictfile, fdictfile);
    
    process_reffile (reffile);
    
#if (0 && (! WIN32))
    fflush (stdout);
    fflush (stderr);
    system ("ps aguxwww | grep dpalign");
#endif

    exit(0);
}
