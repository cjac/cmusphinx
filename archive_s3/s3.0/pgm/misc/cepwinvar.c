/*
 * cepwinvar.c -- Variance of a sliding window on a cepstrum stream.
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
 * 15-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>
#include <s3.h>


static char *cepdir;
static int32 cepsize;
static char uttid[4096];
static float32 *var;		/* Feature dimension variances */

typedef struct {
    char p[16];
    int32 sf, nf;
} pseg_t;
static pseg_t *pseg;
static int32 n_pseg;


static void load_pseg (char *line)
{
    char *lp, wd[1024];
    int32 k, len, sf, nf;
    
    n_pseg = 0;
    lp = line;
    while ((k = sscanf (lp, "%s %d %d%n", wd, &sf, &nf, &len)) == 3) {
	lp += len;
	
	strcpy (pseg[n_pseg].p, wd);
	pseg[n_pseg].sf = sf;
	pseg[n_pseg].nf = nf;

	n_pseg++;
    }

    assert (k == 1);
    assert (wd[0] == '(');
    k = strlen(wd);
    assert (wd[k-1] == ')');
    wd[k-1] = '\0';

    strcpy (uttid, wd+1);
}


static int32 frm2pseg (int32 fr)
{
    int32 i;
    
    for (i = 0; i < n_pseg; i++)
	if ((pseg[i].sf <= fr) && (pseg[i].sf + pseg[i].nf > fr))
	    return i;
    return -1;
}


static float64 eucl_dist (float32 *f1, float32 *f2, int32 veclen)
{
    int32 i;
    float64 dist, d;
    
    dist = 0.0;
    for (i = 0; i < veclen; i++) {
	d = f1[i] - f2[i];
	dist += d*d;
    }

    return sqrt(dist);
}


static float64 maha_dist (float32 *f1, float32 *f2, int32 veclen)
{
    int32 i;
    float64 dist, d;
    
    dist = 0.0;
    for (i = 0; i < veclen; i++) {
	d = (f1[i] - f2[i]);
	dist += d*d / var[i];
    }

    return sqrt(dist);
}


static void compute_var (float32 **mfc, int32 nfr, int32 cepsize)
{
    float32 *sum, *mean;
    int32 i, j;

    sum = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    mean = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    
    for (i = 0; i < cepsize; i++)
	sum[i] = 0.0;
    for (i = 0; i < nfr; i++) {
	for (j = 0; j < cepsize; j++)
	    sum[j] += mfc[i][j];
    }
    for (i = 0; i < cepsize; i++)
	mean[i] = sum[i]/(float64)nfr;
    
    for (i = 0; i < cepsize; i++)
	sum[i] = 0.0;
    for (i = 0; i < nfr; i++) {
	for (j = 0; j < cepsize; j++)
	    sum[j] += (mfc[i][j] - mean[j]) * (mfc[i][j] - mean[j]);
    }
    for (i = 0; i < cepsize; i++)
	var[i] = sum[i]/(float64)nfr;
    
#if 0
    /* Hack!! Scale variances */
    for (i = 0; i < cepsize; i++)
	printf (" %.3f", var[i]);
    printf ("\n");
    for (i = 0; i < cepsize; i++)
	var[i] *= (1.0 + (float64)(i*cepsize*0.5)/(float64)cepsize);
    for (i = 0; i < cepsize; i++)
	printf (" %.3f", var[i]);
    printf ("\n");
#endif

    ckd_free (sum);
    ckd_free (mean);
}


static void cepwinvar (float32 **mfc, int32 nfr, int32 cepsize)
{
    int32 f, i, w, p;
    float64 logd, prevd[3];
    
    prevd[0] = 0.0;
    prevd[1] = 0.0;
    prevd[2] = 0.0;
    
    for (f = 3; f < nfr-3; f++) {
	if ((p = frm2pseg (f)) < 0)
	    E_FATAL("frm2pseg(%d) returned -1\n", f);

	printf ("%5d %10s ", f, pseg[p].p);
	
	for (w = 1; w <= 3; w++) {
	    compute_var (mfc+f-w, w+w+1, cepsize);
	    
	    logd = 0.0;
	    for (i = 0; i < cepsize; i++)
		logd += log(var[i]);
	    
	    printf ("    %8.2f %8.2f", logd, logd - prevd[w-1]);
	    prevd[w-1] = logd;
	}
	printf ("\n");
	fflush (stdout);
    }
}


static void process_psegfile (char *cepdir)
{
    char cepfile[4096], line[16384];
    int32 nfr;
    float32 **mfc;
    
    while (fgets (line, sizeof(line), stdin) != NULL) {
	load_pseg (line);

	sprintf (cepfile, "%s/%s.mfc", cepdir, uttid);
	if ((nfr = s2mfc_read (cepfile, 0, (int32)0x70000000, 0, &mfc)) <= 0)
	    E_FATAL("MFC file read (%s) failed\n", cepfile);

	E_INFO("%d frames, %d phone segments\n", nfr, n_pseg);

	cepwinvar (mfc, nfr, cepsize);
    }
}


main (int32 argc, char *argv[])
{
    char *psegfile;
    int32 i;
    
    if (argc != 2)
	E_FATAL("Usage: %s cepdir < psegfile\n", argv[0]);
    
    cepdir = argv[1];
    
    feat_init ("s3_1x39");
    cepsize = feat_cepsize ();

    var = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    pseg = (pseg_t *) ckd_calloc (S3_MAX_FRAMES, sizeof(pseg_t));
    
    process_psegfile (cepdir);

    exit(0);
}
