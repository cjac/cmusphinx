/*
 * misc.c -- Misc. routines (especially I/O) needed by many S3 applications.
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
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "misc.h"


FILE *ctlfile_open (char *file)
{
    FILE *fp;
    
    if (! file)
	E_FATAL("NULL file argument to ctlfile_open()\n");
    if ((fp = fopen (file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);
    
    return fp;
}


void ctlfile_close (FILE *ctlfp)
{
    fclose (ctlfp);
}


/*
 * Return ptr WITHIN path (i.e., not a copy) of the base filename.
 * FATAL error if path ends in a /.
 */
static char *path2basename (char *path)
{
    int32 i;
    
    for (i = strlen(path)-1; (i >= 0) && (path[i] != '/'); --i);
    i++;
    if (path[i] == '/')
	E_FATAL("Path %s has no basename\n", path);

    return (path + i);
}


int32 ctlfile_next (FILE *fp, char *ctlspec, int32 *sf, int32 *ef, char *uttid)
{
    char line[1024];
    int32 k;
    char *base;
    
    *sf = 0;
    *ef = (int32)0x7ffffff0;

    /* Read next non-comment or non-empty line */
    for (;;) {
	if (fgets (line, sizeof(line), fp) == NULL)
	    return -1;
	
	if ((line[0] != '#') &&
	    ((k = sscanf (line, "%s %d %d %s", ctlspec, sf, ef, uttid)) > 0))
	    break;
    }
    
    switch (k) {
    case 1:
	base = path2basename (ctlspec);
	strcpy (uttid, base);
	break;
	
    case 2:
	E_FATAL("Bad control file line: %s\n", line);
	break;
	
    case 3:
	if ((*sf >= *ef) || (*sf < 0))
	    E_FATAL("Bad control file line: %s\n", line);
	base = path2basename (ctlspec);
	sprintf (uttid, "%s_%d_%d", base, *sf, *ef);
	break;
	
    case 4:
	if ((*sf >= *ef) || (*sf < 0))
	    E_FATAL("Bad control file line: %s\n", line);
	break;
	
    default:
	E_FATAL("Panic: How did I get here?\n");
	break;
    }
    
    return 0;
}


int32 argfile_load (char *file, char *pgm, char ***argvout)
{
    FILE *fp;
    char line[1024], word[1024], *lp, **argv;
    int32 len, n;

    E_INFO("Reading arguments from %s\n", file);

    if ((fp = fopen (file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return -1;
    }

    /* Count #arguments */
    n = 1;	/* Including pgm */
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    n++;
	}
    }
    
    /* Allocate space for arguments */
    argv = (char **) ckd_calloc (n+1, sizeof(char *));
    
    /* Create argv list */
    rewind (fp);
    argv[0] = pgm;
    n = 1;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    argv[n] = ckd_salloc (word);
	    n++;
	}
    }
    argv[n] = NULL;
    *argvout = argv;

    fclose (fp);

    return n;
}


static void hyp_free (hyp_t *list)
{
    hyp_t *h;
    
    while (list) {
	h = list->next;
	listelem_free ((char *)list, sizeof(hyp_t));
	list = h;
    }
}


static hyp_t *nbestfile_parseline (char *sent)
{
    char *lp;
    hyp_t *head, *tail, *hyp;
    char word[1024];
    int32 k, sf, len, lscr;
    
    head = tail = NULL;
    
    lp = sent;
    /* Parse T <score> */
    if ((sscanf (lp, "%s%d%n", word, &k, &len) != 2) || (strcmp (word, "T") != 0)) {
	E_ERROR("Bad sentence: %s\n", sent);
	return NULL;
    }
    lp += len;
    
    /* Parse A <score> */
    if ((sscanf (lp, "%s%d%n", word, &k, &len) != 2) || (strcmp (word, "A") != 0)) {
	E_ERROR("Bad sentence: %s\n", sent);
	return NULL;
    }
    lp += len;
    
    /* Parse L <score> */
    if ((sscanf (lp, "%s%d%n", word, &lscr, &len) != 2) || (strcmp (word, "L") != 0)) {
	E_ERROR("Bad sentence: %s\n", sent);
	return NULL;
    }
    lp += len;
    
    /* Parse each hyp word */
    while ((k = sscanf (lp, "%d%s%n", &sf, word, &len)) == 2) {
	lp += len;

	hyp = (hyp_t *) listelem_alloc (sizeof(hyp_t));
	hyp->word = (char *) ckd_salloc (word);
	hyp->sf = sf;
	hyp->lscr = lscr;	/* HACK!! Every entry has the TOTAL LM score */
	hyp->next = NULL;
	if (! head)
	    head = hyp;
	else
	    tail->next = hyp;
	tail = hyp;
    }
    
    if ((k > 0) || (sscanf (lp, "%s", word) > 0)) {
	E_ERROR("Bad sentence: %s\n", sent);
	hyp_free (head);
	return NULL;
    }
    
    return head;
}


void nbestlist_free (hyp_t **hyplist, int32 nhyp)
{
    int32 i;
    
    for (i = 0; i < nhyp; i++)
	hyp_free (hyplist[i]);
    ckd_free (hyplist);
}


#define NBEST_HYP_MAX		4092

/*
 * Read an Nbest file and create an array of hypotheses (array of hyp_t lists).
 * Return value: #hypotheses read, -1 if unsuccessful.
 */
int32 nbestfile_load (char *dir, char *uttid, hyp_t ***hyplist_out)
{
    char filename[1024];
    FILE *fp;
    hyp_t **hyplist, *h, *nexth;
    char line[65535], *lp, word[1024];
    int32 k, nhyp, nfrm;

    *hyplist_out = NULL;	/* Initialize to illegal value */
    
    if ((! dir) || (! uttid) || (! hyplist_out)) {
	E_ERROR("nbestfile_load: NULL argument\n");
	return -1;
    }
    
    sprintf (filename, "%s/%s.nbest", dir, uttid);
    if ((fp = fopen (filename, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", filename);
	return -1;
    }
    E_INFO("Reading nbest file %s\n", filename);
    
    hyplist = (hyp_t **) ckd_calloc (NBEST_HYP_MAX, sizeof(hyp_t *));
    nhyp = 0;
    
    /* Skip header comments in the file */
    nfrm = -1;
    while ((lp = fgets (line, sizeof(line), fp)) != NULL) {
	k = strlen(line) - 1;
	if (line[k] != '\n') {
	    E_FATAL("Line does not end with newline (increase sizeof(line)?):\n%s\n",
		    line);
	}
	if (line[0] != '#')
	    break;
	if ((sscanf (line+1, "%s%d", word, &k) == 2) && (strcmp (word, "frames") == 0))
	    nfrm = k;
    }
    if (nfrm < 0) {
	E_ERROR("frames parameter missing in header in %s\n", filename);
	goto load_error;
    }
    if (! lp) {
	E_ERROR("Premature EOF(%s)\n", filename);
	goto load_error;
    }
    
    while ((line[0] == 'T') && (line[1] == ' ')) {
	if (nhyp >= NBEST_HYP_MAX)
	    E_FATAL("Increase NBEST_HYP_MAX\n");
	
	if ((hyplist[nhyp] = nbestfile_parseline (line)) == NULL)
	    goto load_error;
	
	nhyp++;
	
	/* Fill in end frame information in hyp */
	for (h = hyplist[nhyp]; h; h = nexth) {
	    nexth = h->next;
	    h->ef = nexth ? nexth->sf - 1 : nfrm-1;
	    if (h->ef >= nfrm) {
		E_ERROR("%s: Start frame value (%d) >= #frames in header (%d)\n",
			filename, h->ef+1, nfrm);
		goto load_error;
	    }
	}
	
	if ((lp = fgets (line, sizeof(line), fp)) == NULL)
	    break;
	k = strlen(line) - 1;
	if (line[k] != '\n') {
	    E_FATAL("Line does not end with newline (increase sizeof(line)?):\n%s\n",
		    line);
	}
    }

    if ((! lp) || (strncmp (line, "End", 3) != 0)) {
	E_ERROR("No End marker in %s\n", filename);
	goto load_error;
    }

    fclose (fp);
    *hyplist_out = hyplist;
    return nhyp;

load_error:
    fclose (fp);
    nbestlist_free (hyplist, nhyp);
    return -1;
}
