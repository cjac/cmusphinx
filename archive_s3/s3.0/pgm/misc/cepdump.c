/*
 * test.c -- Cepstrum vector distance (Euclidean) between all given vectors
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
 * 		Started adding variances for each dimension.
 * 
 * 24-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <s3.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>

static char *cepdir;
static int32 n_feat, cepsize;
static int32 logsc;		/* Whether to logscale distance measures */
static int32 featlen;
static float64 scale;		/* For scaling Euclidean distances */
static float32 **feat;
static char *feattype;
static FILE *allpfp;
static float32 *var;		/* Feature dimension variances */
static float64 mfc_smooth;

#define EUCL_DIST	1
#define MAHA_DIST	2
static int32 dist_type;


typedef struct {
    char p[16];
    int32 sf, nf;
} pseg_t;
static pseg_t *pseg;
static int32 n_pseg;

#define XOFF		36.0
#define YOFF		108.0


#if 0
static char *frm2phone (int32 f)
{
    int32 i;

    for (i = 0; (i < n_pseg) && (f >= pseg[i].sf + pseg[i].nf); i++);
    return ((i < n_pseg) ? pseg[i].p : NULL);
}


static void load_allpseg (char *line, char *uttid)
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
#endif


static void add_allpseg (char *uttid, int32 sfspec, int32 efspec)
{
    char line[16384], *lp, wd[1024];
    int32 k, len, sf, nf, ef;

    if (! allpfp)
	return;
    
    rewind (allpfp);
    
    lp = NULL;
    while (fgets (line, sizeof(line), allpfp) != NULL) {
	/* Obtain uttid */
	k = strlen (line)-2;
	if ((k < 2) || (line[k] != ')')) {
	    E_ERROR("No uttid in: %s\n", line);
	    return;
	}
	line[k] = '\0';
	
	for (--k; (k > 0) && (line[k] != '('); --k);
	if (k <= 0) {
	    E_ERROR("No uttid in: %s\n", line);
	    return;
	}
	
	if (strcmp (line+k+1, uttid) == 0) {
	    lp = line;
	    break;
	}
    }

    if (! lp)
	return;

    while ((k = sscanf (lp, "%s %d %d%n", wd, &sf, &nf, &len)) == 3) {
	lp += len;
	
	ef = sf+nf-1;
	if (sf > efspec)
	    break;
	if (ef < sfspec)
	    continue;
	
	if (sf < sfspec) {
	    nf -= (sfspec - sf);
	    sf = sfspec;
	}
	if (ef > efspec) {
	    nf -= (ef - efspec);
	    ef = efspec;
	}

	assert (nf >= 0);
	
	strcpy (pseg[n_pseg].p, wd);
	pseg[n_pseg].sf = sf;
	pseg[n_pseg].nf = nf;

	n_pseg++;
    }
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


static void add_cepfile (char *cepfile, int32 sf, int32 ef, float64 sm)
{
    int32 i, j, w, nfr, tsf, tef;
    float32 **mfc;

    tsf = sf-10;
    tef = ef+10;
    if (tsf < 0)
	tsf = 0;

    if ((nfr = s2mfc_read (cepfile, tsf, tef, &mfc)) <= 0) {
	E_ERROR("MFC file read (%s) failed\n", cepfile);
	return;
    }
    if (sm != 1.0) {
	for (i = 0; i < nfr-1; i++) {
	    for (j = 0; j < cepsize; j++)
		mfc[i][j] = mfc[i][j] * sm + mfc[i+1][j] * (1.0-sm);
	}
    }

    sf -= tsf;
    ef -= tsf;
    
    w = feat_window_size ();
    if (sf < w) {
	sf = w;
	/* E_WARN("%s: sf bumped up to %d\n", cepfile, sf); */
    }
    if (ef >= nfr-w-1) {
	ef = nfr-w-2;
	/* E_WARN("%s: ef bumped down to %d\n", cepfile, ef); */
    }
    
    if (strcmp (feattype, "cep") == 0) {
	for (i = sf; i <= ef; i++) {
	    for (j = 0; j < featlen; j++)
		feat[n_feat][j] = mfc[i][j];
	    n_feat++;
	}
    } else if (strcmp (feattype, "cep_c0") == 0) {
	for (i = sf; i <= ef; i++) {
	    for (j = 0; j < featlen; j++)
		feat[n_feat][j] = mfc[i][j+1];
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
}


static void calib (char *ctlfile, int32 ncalib)
{
    FILE *fp;
    char cepfile[4096], uttid[4096];
    float32 *sum, *sumsq, *lsum, *lsumsq, *mean;
    int32 i, j, nfr, nc;
    
    if ((fp = fopen(ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);

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
    for (nc = 0; nc < ncalib; nc++) {
	if (fscanf (fp, "%s", uttid) != 1)
	    break;
	
	sprintf (cepfile, "%s.mfc", uttid);

	n_feat = 0;
	add_cepfile (cepfile, 0, (int32)0x7ffffff0, 1.0);

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
	fprintf (stderr, " %11.3e", mean[i]);
    }
    fprintf (stderr, "\n");
    fflush (stderr);
    
    E_INFO("Computing variances\n");
    for (i = 0; i < featlen; i++)
	sum[i] = 0.0;
    
    for (nc = 0; nc < ncalib; nc++) {
	if (fscanf (fp, "%s", uttid) != 1)
	    break;
	
	sprintf (cepfile, "%s.mfc", uttid);

	n_feat = 0;
	add_cepfile (cepfile, 0, (int32)0x7ffffff0, 1.0);

	for (i = 0; i < featlen; i++)
	    lsum[i] = 0.0;
	
	for (i = 0; i < n_feat; i++) {
	    for (j = 0; j < featlen; j++)
		lsum[j] += (feat[i][j] - mean[j]) * (feat[i][j] - mean[j]);
	}

	for (i = 0; i < featlen; i++)
	    sum[i] += lsum[i];
    }
    fclose (fp);
    
    for (i = 0; i < featlen; i++) {
	var[i] = sum[i]/(float64)nfr;
	fprintf (stderr, " %11.3e", var[i]);
    }
    fprintf (stderr, "\n");
    fflush (stderr);
    
    ckd_free (sum);
    ckd_free (sumsq);
    ckd_free (lsum);
    ckd_free (lsumsq);
    ckd_free (mean);
}


static void process_ctlfile (FILE *fp)
{
    char cepfile[4096], uttid[4096], pslabel[16384], line[16384], *lp;
    int32 i, j, k, sf, nf, len;
    float64 d, maxd, sq, sc;
    
    for (;;) {
	fflush (stdout);
	fprintf (stderr, "uttid sf nf...> ");
	fflush (stderr);

	if (fgets (line, sizeof(line), fp) == NULL)
	    break;
	k = strlen(line)-1;
	if (line[k] == '\n')
	    line[k] = '\0';
	
	n_feat = 0;
	n_pseg = 0;
	
	lp = line;
	while (sscanf (lp, "%s %d %d%n", uttid, &sf, &nf, &len) == 3) {
	    lp += len;

	    sprintf (cepfile, "%s/%s.mfc", cepdir, uttid);
	    add_cepfile (cepfile, sf, sf+nf-1, mfc_smooth);
	    add_allpseg (uttid, sf, sf+nf-1);

	    sprintf (pslabel, "%s %s %s",
		     (dist_type == EUCL_DIST) ? "EUCL" : "MAHA", feattype, line);
	}
	/* E_INFO("%d feature vectors\n", n_feat); */

	if (n_feat <= 0)
	    continue;

	for (i = 0; i < n_feat; i++) {
	    printf ("%4d", i);
	    for (j = 0; j < featlen; j++)
		printf (" %.3e", feat[i][j]);
	    printf (" 0.0\n");
	}
	fflush (stdout);
    }
}


main (int32 argc, char *argv[])
{
    int32 *fl;	/* temporary featlen */
    char *allpfile, *calibfile;
    int32 i, ncalib;
    
    if (argc < 7) {
	E_FATAL("Usage: %s feattype disttype(e/m) cepdir calibctlfile ncalib mfcsmooth [allpfile]\n",
		argv[0]);
    }
    
    scale = 10.0;
    feattype = argv[1];
    dist_type = (argv[2][0] == 'e') ? EUCL_DIST : MAHA_DIST;
    cepdir = argv[3];
    calibfile = argv[4];
    if (sscanf (argv[5], "%d", &ncalib) != 1) {
	E_FATAL("Usage: %s feattype disttype(e/m) cepdir calibctlfile ncalib [allpfile]\n",
		argv[0]);
    }
    if (sscanf (argv[6], "%lf", &mfc_smooth) != 1) {
	E_FATAL("Usage: %s feattype disttype(e/m) cepdir calibctlfile ncalib mfcsmooth [allpfile]\n",
		argv[0]);
    }
    allpfile = (argc > 7) ? argv[7] : NULL;
    
    logsc = 0;
    
    feat_init ("s3_1x39");
    cepsize = feat_cepsize ();
    n_feat = feat_featsize (&fl);
    assert ((n_feat == 1) && (fl[0] == 39));
    
    if (strcmp (feattype, "cep") == 0) {
	featlen = 13;
    } else if (strcmp (feattype, "cep_c0") == 0) {
	featlen = 12;
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
    if (allpfile) {
	if ((allpfp = fopen(allpfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", allpfile);
	pseg = (pseg_t *) ckd_calloc (S3_MAX_FRAMES, sizeof(pseg_t));
    } else {
	allpfp = NULL;
	pseg = NULL;
    }
    
    calib (calibfile, ncalib);
    
    process_ctlfile (stdin);

    exit(0);
}
