/*
 * cepdist-var.c -- feature vector variances
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
 * 16-May-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <s3.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>

static int32 cepsize, featlen;
static float32 **feat;
static char *feattype;


static void cepd1 (float32 **mfc, float32 *f)
{
    int32 i;

    for (i = 0; i < cepsize; i++)
	f[i] = mfc[0][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+i] = mfc[0][i] - mfc[-1][i];
}


static void cepd2 (float32 **mfc, float32 *f)
{
    int32 i;

    for (i = 0; i < cepsize; i++)
	f[i] = mfc[0][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+i] = mfc[1][i] - mfc[-1][i];
}


static void cepd1_c0 (float32 **mfc, float32 *f)
{
    int32 i;

    for (i = 1; i < cepsize; i++)
	f[i-1] = mfc[0][i];
    for (i = 1; i < cepsize; i++)
	f[cepsize+i-2] = mfc[0][i] - mfc[-1][i];
}


static void cepd1d2avg (float32 **mfc, float32 *f)
{
    int32 i;

    for (i = 0; i < cepsize; i++)
	f[i] = mfc[0][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+i] = 0.5 * ((mfc[0][i] - mfc[-1][i]) + (mfc[1][i] - mfc[0][i]));
}


static void cepd1d2 (float32 **mfc, float32 *f)
{
    int32 i;
    
    for (i = 0; i < cepsize; i++)
	f[i] = mfc[0][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+i] = mfc[0][i] - mfc[-1][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+cepsize+i] = mfc[1][i] - mfc[0][i];
}


static void cep_d_dd (float32 **mfc, float32 *f)
{
    int32 i;

    for (i = 0; i < cepsize; i++)
	f[i] = mfc[0][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+i] = mfc[0][i] - mfc[-1][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+cepsize+i] = mfc[1][i] - mfc[0][i];
    for (i = 0; i < cepsize; i++)
	f[cepsize+cepsize+i] -= f[cepsize+i];
}


static int32 add_cepfile (char *cepfile)
{
    int32 i, j, w, nfr, sf, ef, n_feat;
    float32 **mfc;

    if ((nfr = s2mfc_read (cepfile, 0, (int32)0x7fffffff, &mfc)) <= 0) {
	E_ERROR("MFC file read (%s) failed\n", cepfile);
	return;
    }
    
    w = feat_window_size ();
    sf = w;
    ef = nfr-w-2;
    
    n_feat = 0;
    
    if (strcmp (feattype, "cep") == 0) {
	for (i = sf; i <= ef; i++) {
	    for (j = 0; j < featlen; j++)
		feat[n_feat][j] = mfc[i][j];
	    n_feat++;
	}
    } else if (strcmp (feattype, "d1") == 0) {
	for (i = sf; i <= ef; i++) {
	    cepd1 (mfc+i, feat[n_feat]);
	    n_feat++;
	}
    } else if (strcmp (feattype, "d2") == 0) {
	for (i = sf; i <= ef; i++) {
	    cepd2 (mfc+i, feat[n_feat]);
	    n_feat++;
	}
    } else if (strcmp (feattype, "d1_c0") == 0) {
	for (i = sf; i <= ef; i++) {
	    cepd1_c0 (mfc+i, feat[n_feat]);
	    n_feat++;
	}
    } else if (strcmp (feattype, "d1d2avg") == 0) {
	for (i = sf; i <= ef; i++) {
	    cepd1d2avg (mfc+i, feat[n_feat]);
	    n_feat++;
	}
    } else if (strcmp (feattype, "d1d2") == 0) {
	for (i = sf; i <= ef; i++) {
	    cepd1d2 (mfc+i, feat[n_feat]);
	    n_feat++;
	}
    } else if (strcmp (feattype, "d_dd") == 0) {
	for (i = sf; i <= ef; i++) {
	    cep_d_dd (mfc+i, feat[n_feat]);
	    n_feat++;
	}
    } else if (strcmp (feattype, "s3") == 0) {
	for (i = sf; i <= ef; i++) {
	    feat_cep2feat (mfc+i, feat+n_feat);
	    n_feat++;
	}
    } else
	E_FATAL("Unknown feattype: %s\n", feattype);

    return n_feat;
}


static void process_ctlfile (FILE *fp)
{
    char cepfile[4096], uttid[4096];
    float32 *sum, *sumsq, *lsum, *lsumsq, *mean, *var;
    int32 i, j, nfr, n_feat;
    
    sum = (float32 *) ckd_calloc (featlen, sizeof(float32));
    sumsq = (float32 *) ckd_calloc (featlen, sizeof(float32));
    lsum = (float32 *) ckd_calloc (featlen, sizeof(float32));
    lsumsq = (float32 *) ckd_calloc (featlen, sizeof(float32));
    mean = (float32 *) ckd_calloc (featlen, sizeof(float32));
    var = (float32 *) ckd_calloc (featlen, sizeof(float32));
    
    E_INFO("Computing means\n");
    for (i = 0; i < featlen; i++)
	sum[i] = 0.0;
    
    nfr = 0;
    while (fscanf (fp, "%s", uttid) == 1) {
	sprintf (cepfile, "%s.mfc", uttid);

	n_feat = add_cepfile (cepfile);
	E_INFO("%5d feature vectors\n", n_feat);

	for (i = 0; i < featlen; i++)
	    lsum[i] = 0.0;
	
	for (i = 0; i < n_feat; i++) {
	    for (j = 0; j < featlen; j++)
		lsum[j] += feat[i][j];
	}

	for (i = 0; i < featlen; i++)
	    sum[i] += lsum[i];

	nfr += n_feat;
    }
    rewind (fp);
    
    for (i = 0; i < featlen; i++) {
	mean[i] = sum[i]/(float64)nfr;
	printf (" %11.3e", mean[i]);
    }
    printf ("\n");
    fflush (stdout);
    
    E_INFO("Computing variances\n");
    for (i = 0; i < featlen; i++)
	sum[i] = 0.0;
    
    while (fscanf (fp, "%s", uttid) == 1) {
	sprintf (cepfile, "%s.mfc", uttid);

	n_feat = add_cepfile (cepfile);
	E_INFO("%5d feature vectors\n", n_feat);

	for (i = 0; i < featlen; i++)
	    lsum[i] = 0.0;
	
	for (i = 0; i < n_feat; i++) {
	    for (j = 0; j < featlen; j++)
		lsum[j] += (feat[i][j] - mean[j]) * (feat[i][j] - mean[j]);
	}

	for (i = 0; i < featlen; i++)
	    sum[i] += lsum[i];
    }
    
    for (i = 0; i < featlen; i++) {
	var[i] = sum[i]/(float64)nfr;
	printf (" %11.3e", var[i]);
    }
    printf ("\n");
}


main (int32 argc, char *argv[])
{
    int32 *fl;	/* temporary featlen */
    char *ctlfile;
    int32 i, n_feat;
    FILE *ctlfp;
    
    if (argc < 3)
	E_FATAL("Usage: %s feattype ctlfile\n", argv[0]);
    
    feattype = argv[1];
    ctlfile = argv[2];
    
    feat_init ("s3_1x39");
    cepsize = feat_cepsize ();
    n_feat = feat_featsize (&fl);
    assert ((n_feat == 1) && (fl[0] == 39));
    
    if (strcmp (feattype, "cep") == 0) {
	featlen = 13;
    } else if (strcmp (feattype, "d1") == 0) {
	featlen = 26;
    } else if (strcmp (feattype, "d2") == 0) {
	featlen = 26;
    } else if (strcmp (feattype, "d1_c0") == 0) {
	featlen = 24;
    } else if (strcmp (feattype, "d1d2avg") == 0) {
	featlen = 26;
    } else if (strcmp (feattype, "d1d2") == 0) {
	featlen = 39;
    } else if (strcmp (feattype, "d_dd") == 0) {
	featlen = 39;
    } else if (strcmp (feattype, "s3") == 0) {
	featlen = fl[0];
    } else
	E_FATAL("Unknown feattype: %s\n", feattype);

    feat = (float32 **) ckd_calloc_2d (S3_MAX_FRAMES, featlen, sizeof(float32));
    
    if (strcmp (ctlfile, "-") != 0) {
	if ((ctlfp = fopen(ctlfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", ctlfile);
    } else
	ctlfp = stdin;
    
    process_ctlfile (ctlfp);
    
    exit(0);
}
