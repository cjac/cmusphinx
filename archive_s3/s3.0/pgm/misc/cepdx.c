/*
 * cepdx.c -- Differentiate a cepstrum vector stream.
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
 * 29-Jul-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>
#include <s3.h>


static int32 cepsize;
static char uttid[4096];
static float32 *var;		/* Feature dimension variances */
static float64 varscale;

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


static void compute_var (float32 **mfc, int32 nfr)
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
    
    /* Hack!! Scale variances */
    for (i = 0; i < cepsize; i++)
	fprintf (stderr, " %.3f", var[i]);
    fprintf (stderr, "\n");
    for (i = 0; i < cepsize; i++)
	var[i] *= (1.0 + (float64)(i*varscale)/(float64)cepsize);
    for (i = 0; i < cepsize; i++)
	fprintf (stderr, " %.3f", var[i]);
    fprintf (stderr, "\n");
    fflush (stderr);
    
    ckd_free (sum);
    ckd_free (mean);
}


static void dx (float32 **mfc, int32 nfr)
{
    int32 i, k, p;
    float64 *d, maxd;
    
    d = (float64 *) ckd_calloc (S3_MAX_FRAMES, sizeof(float64));
    
    maxd = 0.0;
    for (i = 0; i < nfr-1; i++) {
	d[i] = maha_dist (mfc[i+1], mfc[i], cepsize);
	if (d[i] > maxd)
	    maxd = d[i];
    }

    p = 0;
    for (i = 0; i < nfr-1; i++) {
	d[i] = (d[i]*10.0)/maxd;
	
	assert (pseg[p].sf <= i);
	if (pseg[p].sf + pseg[p].nf <= i)
	    p++;
	assert (p < n_pseg);
	assert (pseg[p].sf <= i);
	assert (pseg[p].sf + pseg[p].nf > i);
	
	printf ("%s %4d %5.2f %10s ", uttid, i, d[i], pseg[p].p);
	k = d[i]*5.0 + 0.5;
	for (; k > 0; --k)
	    printf (".");
	printf ("\n");
    }
    fflush (stdout);

    ckd_free (d);
}


static void process_psegfile (char *cepdir, char *psegfile)
{
    char cepfile[4096], line[16384];
    int32 nfr;
    float32 **mfc;
    FILE *fp;
    
    if ((fp = fopen(psegfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", psegfile);
    
    for (;;) {
	if (fgets (line, sizeof(line), fp) == NULL)
	    break;
	fprintf (stderr, "%s", line);
	fflush (stderr);
	
	load_pseg (line);
	
	sprintf (cepfile, "%s/%s.mfc", cepdir, uttid);
	if ((nfr = s2mfc_read (cepfile, 0, (int32)0x70000000, 0, &mfc)) <= 0)
	    E_FATAL("MFC file read (%s) failed\n", cepfile);
	E_INFO("%s: %d frames\n", cepfile, nfr);
	compute_var (mfc, nfr);
#if 0
	fprintf (stderr, "ready? ");
	fgets (line, sizeof(line), stdin);
#endif
	dx (mfc, nfr);
    }

    fclose (fp);
}


main (int32 argc, char *argv[])
{
    char *cepdir, *psegfile;
    
    if (argc != 4)
	E_FATAL("Usage: %s varscale cepdir psegfile\n", argv[0]);
    
    if (sscanf (argv[1], "%lf", &varscale) != 1)
	E_FATAL("Usage: %s varscale cepdir psegfile\n", argv[0]);
    cepdir = argv[2];
    psegfile = argv[3];
    
    feat_init ("s3_1x39");
    cepsize = feat_cepsize ();

    var = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    pseg = (pseg_t *) ckd_calloc (S3_MAX_FRAMES, sizeof(pseg_t));
    
    process_psegfile (cepdir, psegfile);
    
    exit(0);
}
