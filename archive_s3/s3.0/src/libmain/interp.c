/*
 * interp.c -- CD-senone and CI-senone score interpolation
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
 * 18-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started based on original S3 implementation.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libmisc/libmisc.h>
#include "interp.h"


#define INTERP_VERSION "1.0"


#if _INTERP_TEST_
void interp_dump (interp_t *ip)
{
    int32 i;
    
    for (i = 0; i < ip->n_sen; i++)
	printf ("%6d %12d %12d\n", i, ip->wt[i].cd, ip->wt[i].ci);
}
#endif


interp_t *interp_init (char *file_name)
{
    FILE *fp;
    interp_t *ip;
    int32 byteswap, chksum_present;
    int32 i;
    char eofchk;
    float f;
    char **argname, **argval;
    uint32 chksum;
    
    assert (file_name != NULL);
    
    ip = (interp_t *) ckd_calloc (1, sizeof(interp_t));
    
    E_INFO("Reading interpolation weights: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], INTERP_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
		       file_name, argval[i], INTERP_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;
    
    chksum = 0;
    
    /* Read #senones */
    if (bio_fread (&(ip->n_sen), sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (arraysize) failed\n", file_name);
    if (ip->n_sen <= 0)
	E_FATAL("%s: arraysize= %d in header\n", file_name, ip->n_sen);
    if (ip->n_sen >= MAX_SENID)
	E_FATAL("%s: #senones (%d) exceeds limit (%d)\n", file_name, ip->n_sen, MAX_SENID);

    ip->wt = (struct interp_wt_s *) ckd_calloc (ip->n_sen, sizeof(struct interp_wt_s));
    
    for (i = 0; i < ip->n_sen; i++) {
	if (bio_fread (&f, sizeof(float32), 1, fp, byteswap, &chksum) != 1)
	    E_FATAL("fread(%s) (arraydata) failed\n", file_name);
	if ((f < 0.0) || (f > 1.0))
	    E_FATAL("%s: interpolation weight(%d)= %e\n", file_name, i, f);
	
	ip->wt[i].cd = (f == 0.0) ? LOGPROB_ZERO : logs3(f);
	ip->wt[i].ci = (f == 1.0) ? LOGPROB_ZERO : logs3(1.0-f);
    }

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);
    
    E_INFO("Read %d interpolation weights\n", ip->n_sen);

    return ip;
}


int32 interp_cd_ci (interp_t *ip, int32 *senscr, int32 cd, int32 ci)
{
    assert ((ci >= 0) && (ci < ip->n_sen));
    assert ((cd >= 0) && (cd < ip->n_sen));

    senscr[cd] = logs3_add (senscr[cd] + ip->wt[cd].cd,
			    senscr[ci] + ip->wt[cd].ci);
    
    return 0;
}


int32 interp_all (interp_t *ip, int32 *senscr, s3senid_t *cd2cimap, int32 n_ci_sen)
{
    int32 ci, cd;
    
    assert (n_ci_sen <= ip->n_sen);
    
    for (cd = n_ci_sen; cd < ip->n_sen; cd++) {
	ci = cd2cimap[cd];
	senscr[cd] = logs3_add (senscr[cd] + ip->wt[cd].cd,
				senscr[ci] + ip->wt[cd].ci);
    }
    
    return 0;
}


#if _INTERP_TEST_
main (int32 argc, char *argv[])
{
    interp_t *ip;
    
    if (argc < 2)
	E_FATAL("Usage: %s interpfile\n", argv[0]);

    logs3_init ((float64) 1.0001);
    
    ip = interp_init (argv[1]);

    interp_dump (ip);
}
#endif
