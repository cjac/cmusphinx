/*
 * hypext.c -- Extract hypotheses from multiple recognition hypotheses in reverse
 * 		chronological order.
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
 * 22-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>


static corpus_t **inhyp;
static int32 n_inhyp;
static FILE *outfp;
static char **infilename;


/* Validation function for loading a hypseg corpus */
static int32 validate (char *str)
{
    char tmp[65535], *wdp[4096];
    int32 nwd;
    
    strcpy (tmp, str);
    if ((nwd = str2words (tmp, wdp, 4095)) < 0)
	E_FATAL("str2words failed\n");
    if ((nwd > 0) && (strcmp (wdp[nwd-1], "(null)") == 0))
	return 0;	/* Exclude (null) hypotheses */
    return 1;
}


/* Duplicate resolution */
static int32 dup_resolve (char *s1, char *s2)
{
    if (strcmp (s1, s2) != 0)
	return -1;
    else
	return 0;
}


static void process_utt (char *uttfile, int32 sf, int32 ef, char *uttid)
{
    int32 i, f, nwd;
    char *str;
    char tmp[65535], *wdp[4096];
    
    for (i = 0; i < n_inhyp; i++) {
	if ((str = corpus_lookup (inhyp[i], uttid)) != NULL)
	    break;
    }
    if (i >= n_inhyp)
	E_ERROR("%s: Missing\n", uttid);
    else {
	strcpy (tmp, str);
	if ((nwd = str2words (tmp, wdp, 4095)) < 0)
	    E_FATAL("str2words failed\n");
	if ((nwd == 0) || (sscanf (wdp[nwd-1], "%d", &f) != 1) || (f != (ef-sf+1)))
	    E_ERROR("%s: Bad hyp in %s: %s\n", uttid, infilename[i], str);
	else {
	    fprintf (outfp, "%s %s\n", uttid, str);
	    fflush (outfp);
	    E_INFO("%s: Extracted from %s\n", uttid, infilename[i]);
	}
    }
}


main (int32 argc, char *argv[])
{
    int32 i;
    
    if (argc < 3) {
	E_INFO("Usage: %s outhypfile inhypfile1 inhypfile2 ... < ctlfile\n", argv[0]);
	exit(0);
    }
    
    n_inhyp = argc - 2;
    inhyp = (corpus_t **) ckd_calloc (n_inhyp, sizeof(corpus_t *));
    
    for (i = 2; i < argc; i++)
	inhyp[i-2] = corpus_load_headid (argv[i], validate, dup_resolve);
    
    E_INFO("Extracting\n");
    
    if ((outfp = fopen(argv[1], "w")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,w) failed\n", argv[1]);
    
    infilename = &(argv[2]);
    ctl_process (NULL, 0, 1000000000, process_utt);
    
    fclose (outfp);
    
    E_INFO("Done\n");
    
    exit(0);
}
