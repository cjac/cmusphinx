/*
 * pscr.c -- Phone scores processing utility.
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
 * 11-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <libutil/libutil.h>


static int32 *pscr;


static int32 cmp_pscr (int32 *a, int32 *b)
{
    return (pscr[*b] - pscr[*a]);
}


main (int32 argc, char *argv[])
{
    char line[1024], *lp, wd[1024], *filename;
    FILE *fp;
    int32 len, np, sf, nf, best, th, beam;
    int32 p, k, f;
    char **phone;
    uint8 *pscr_in;
    int32 *pscr_id;
    float64 logf, sump, invsump, tmp, tot_sump;
    
    if (argc < 2)
	E_FATAL("Usage: %s [<beam>] <.pscr-file>\n", argv[0]);

    if (argc > 2) {
	if (sscanf (argv[1], "%d", &beam) != 1)
	    E_FATAL("Usage: %s <.pscr-file> [<beam>]\n", argv[0]);
	if (beam > 0)
	    beam = -beam;
	filename = argv[2];
    } else
	filename = argv[1];
    
    logf = log(1.0001);
    
    if ((fp = fopen (filename, "rb")) == NULL)
	E_FATAL("fopen(%s,rb) failed\n", filename);

    /* Read phone names */
    if (fgets (line, sizeof(line), fp) == NULL)
	E_FATAL("fgets(%s) failed\n", filename);
    assert (line[0] == '#');

    if (sscanf (line+1, "%d%n", &np, &len) != 1)
	E_FATAL("Failed to read header in %s\n", filename);
    lp = line+1+len;
    assert (np > 0);

    phone = (char **) ckd_calloc (np, sizeof(char *));
    for (p = 0; p < np; p++) {
	if (sscanf (lp, "%s%n", wd, &len) != 1)
	    E_FATAL("Failed to read header in %s\n", filename);
	lp += len;
	phone[p] = (char *) ckd_salloc (wd);
    }

    /* Read scalefactor */
    if (fgets (line, sizeof(line), fp) == NULL)
	E_FATAL("fgets(%s) failed\n", filename);
    assert (line[0] == '#');
    if (sscanf (line+1, "%s%d", wd, &sf) != 2)
	E_FATAL("Failed to read header in %s\n", filename);

    if (argc == 2)
	beam = -sf * 256;
    E_INFO("Beam = %d\n", beam);

    /* Skip past end comment marker */
    while ((fgets(line, sizeof(line), fp) != NULL) && (strcmp(line, "*end_comment*\n") != 0));
    
    /* Read byte order magic */
    if (fread (&k, sizeof(int32), 1, fp) != 1)
	E_FATAL("Failed to read byte-order magic in %s\n", filename);
    assert (k == (int32)0x11223344);	/* For now assume native byte order */
    
    /* Read #frames */
    if (fread (&nf, sizeof(int32), 1, fp) != 1)
	E_FATAL("Failed to read #frames in %s\n", filename);
    E_INFO("#frames= %d\n", nf);
    
    /* Read #phones */
    if (fread (&k, sizeof(int32), 1, fp) != 1)
	E_FATAL("Failed to read #frames in %s\n", filename);
    assert (k == np);
    
    /* Allocate phone scores / frame */
    pscr = (int32 *) ckd_calloc (np, sizeof(int32));
    pscr_in = (uint8 *) ckd_calloc (np, sizeof(uint8));
    pscr_id = (int32 *) ckd_calloc (np, sizeof(int32));

    /* Read phone scores */
    tot_sump = 0.0;
    for (f = 0; f < nf; f++) {
	if (fread (&best, sizeof(int32), 1, fp) != 1)
	    E_FATAL("Failed to read bestscore in frame %d in %s\n", f, filename);
	if (fread (pscr_in, sizeof(uint8), np, fp) != np)
	    E_FATAL("Failed to read phone scores in frame %d in %s\n", f, filename);

	for (p = 0; p < np; p++) {
	    pscr[p] = best - (pscr_in[p] * sf);
	    pscr_id[p] = p;
	}
	
	qsort (pscr_id, np, sizeof(int32), cmp_pscr);
	assert (pscr[pscr_id[0]] == best);
	th = best + beam;
	
	sump = 0.0;
	for (p = 0; p < np; p++) {
	    if (pscr_in[p] < 255) {
		tmp = -logf;
		tmp *= (float64)(pscr_in[p]);
		tmp *= (float64)sf;
		tmp = exp(tmp);
		sump += tmp;
	    }
	}
	invsump = 1.0/sump;
	sump = 0.0;
	for (p = 0; p < np; p++) {
	    if (pscr_in[p] < 255) {
		tmp = -logf;
		tmp *= (float64)(pscr_in[p]);
		tmp *= (float64)sf;
		tmp *= invsump;
		sump -= (exp(tmp) * tmp);
	    }
	}
	
	printf ("%5d: %12.6f", f, sump);
	for (p = 0; (p < np) && (pscr[pscr_id[p]] >= th); p++)
	    printf (" %2s", phone[pscr_id[p]]);
	printf ("\n");

	tot_sump += sump;
    }
    printf ("Ave. entropy: %12.6f\n", tot_sump / (float64)nf);
}
