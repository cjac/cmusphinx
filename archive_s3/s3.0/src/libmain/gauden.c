/*
 * gauden.c -- gaussian density module.
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
 * 11-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added det_varinv argument to gauden_init().
 * 
 * 09-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added gauden_cbmean0().
 * 
 * 19-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started, based on original S3 version.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libmisc/libmisc.h>
#include "gauden.h"

#if _GAUDEN_TEST_
#include <libfeat/libfeat.h>
#include "cmn.h"
#include "agc.h"
#endif


#define GAUDEN_PARAM_VERSION	"1.0"

#undef M_PI
#define M_PI	3.1415926535897932385e0


#define GAUDEN_MEAN	1
#define GAUDEN_VAR	2


#if _GAUDEN_TEST_
static void gauden_dump (const gauden_t *g)
{
    int32 c, f, d, i;
    mgau_t *mg;
    
    for (c = 0; c < g->n_mgau; c++) {
	for (f = 0; f < g->n_feat; f++) {
	    mg = &(g->mgau[c][f]);
	    
	    E_INFO ("Codebook %d, Feature %d (%dx%d):\n", c, f, mg->n_mean, g->featlen[f]);
	    
	    if (mg->n_var == mg->n_mean) {
		for (d = 0; d < mg->n_mean; d++) {
		    printf ("m[%3d]", d);
		    for (i = 0; i < g->featlen[f]; i++)
			printf (" %7.4f", mg->mean[d][i]);
		    printf ("\n");

		    printf ("v[%3d]", d);
		    for (i = 0; i < g->featlen[f]; i++)
			printf (" %7.4f", mg->var[d][i]);
		    printf (" det %7.4f\n", mg->det[d]);
		}
	    } else {
		for (d = 0; d < mg->n_mean; d++) {
		    printf ("m[%3d]", d);
		    for (i = 0; i < g->featlen[f]; i++)
			printf (" %7.4f", mg->mean[d][i]);
		    printf ("\n");
		}
		
		if (mg->n_var == 1) {
		    printf ("v[%3d]", 0);
		    for (i = 0; i < g->featlen[f]; i++)
			printf (" %7.4f", mg->var[0][i]);
		    printf (" det %7.4f\n", mg->det[0]);
		}
	    }
	    printf ("\n");
	    fflush (stdout);
	}
    }
}
#endif


static int32 chk_cbmean0 (gauden_t *g, int32 m, int32 f)
{
    int32 c;
    
    for (c = 0;
	 (c < gauden_n_mean(g, m, f)) && vector_is_zero(g->mgau[m][f].mean[c], g->featlen[f]);
	 c++);
    
    return (c == gauden_n_mean(g, m, f));	/* TRUE iff all vectors are 0.0 */
}


static void gauden_cbmean0 (gauden_t *g)
{
    int32 m, f, n;
    
    n = 0;
    for (m = 0; m < gauden_n_mgau(g); m++) {
	for (f = 0; f < g->n_feat; f++) {
	    if (chk_cbmean0 (g, m, f)) {
		g->mgau[m][f].cb0 = 1;
		n++;
	    } else
		g->mgau[m][f].cb0 = 0;
	}
    }
    E_INFO("%d codebook means are all 0\n", n);
}


static void gauden_var_floor (gauden_t *g, float64 floor)
{
    int32 i, m, f, d, flen;
    float32 *var;
    
    E_INFO("Applying variance floor\n");
    
    for (m = 0; m < g->n_mgau; m++) {
	for (f = 0; f < g->n_feat; f++) {
	    flen = g->featlen[f];
	    
	    for (d = 0; d < g->mgau[m][f].n_var; d++) {
		var = g->mgau[m][f].var[d];

		for (i = 0; i < flen; i++) {
		    if (var[i] < floor)
			var[i] = (float32) floor;
		}
	    }
	}
    }
}


void gauden_var_nzvec_floor (gauden_t *g, float64 floor)
{
    int32 i, m, f, d, flen;
    float32 *var;
    
    E_INFO("Applying variance floor\n");
    
    for (m = 0; m < g->n_mgau; m++) {
	for (f = 0; f < g->n_feat; f++) {
	    flen = g->featlen[f];
	    
	    for (d = 0; d < g->mgau[m][f].n_var; d++) {
		var = g->mgau[m][f].var[d];
		
		if (! vector_is_zero(var, flen)) {
		    for (i = 0; i < flen; i++) {
			if (var[i] < floor)
			    var[i] = (float32) floor;
		    }
		}
	    }
	}
    }
}


/*
 * At the moment, S3 models have the same #means in each codebook and 1 var/mean.
 * If g->mgau already allocated (non-NULL) simply verify the various dimensions.
 */
static int32 gauden_param_read(const char *file_name, gauden_t *g, int32 type)
{
    char tmp;
    FILE *fp;
    int32 i, j, k, n, blk;
    int32 n_mgau;
    int32 n_feat;
    int32 n_density;
    int32 *veclen;
    int32 byteswap, chksum_present;
    float32 *buf;
    vector_t *pbuf;
    char **argname, **argval;
    uint32 chksum;
    
    E_INFO("Reading mixture gaussian parameter: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], GAUDEN_PARAM_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
		       file_name, argval[i], GAUDEN_PARAM_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;
    
    /* #Codebooks */
    if (bio_fread (&n_mgau, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#codebooks) failed\n", file_name);
    if (n_mgau >= MAX_MGAUID)
	E_FATAL("%s: #gauden (%d) exceeds limit (%d)\n", file_name, n_mgau, MAX_MGAUID);
    if (g->mgau && (n_mgau != g->n_mgau))
	E_FATAL("#Codebooks conflict: %d allocated, %d in file\n", g->n_mgau, n_mgau);
    g->n_mgau = n_mgau;
    
    /* #Features/codebook */
    if (bio_fread (&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#features) failed\n", file_name);
    if (g->mgau && (n_feat != g->n_feat))
	E_FATAL("#Features conflict: %d allocated, %d in file\n", g->n_feat, n_feat);
    g->n_feat = n_feat;

    /* #Gaussian densities/feature in each codebook */
    if (bio_fread (&n_density, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#density/codebook) failed\n", file_name);
    if (g->mgau && (n_density != g->mgau[0][0].n_mean))
	E_FATAL("#Densities conflict: %d allocated, %d in file\n",
		g->mgau[0][0].n_mean, n_density);

    /* Vector length of each feature stream */
    veclen = ckd_calloc(n_feat, sizeof(uint32));
    if (bio_fread (veclen, sizeof(int32), n_feat, fp, byteswap, &chksum) != n_feat)
	E_FATAL("fread(%s) (feature-lengths) failed\n", file_name);
    if (g->featlen) {
	/* Verify feature lengths */
	for (i = 0; i < n_feat; i++)
	    if (veclen[i] != g->featlen[i])
		E_FATAL("Feature[%d] length conflict: %d allocated, %d in file\n",
			i, g->featlen[i], veclen[i]);
	ckd_free (veclen);
	veclen = g->featlen;
    } else
	g->featlen = veclen;

    /* blk = total vector length of all feature streams */
    for (i = 0, blk = 0; i < n_feat; i++)
	blk += veclen[i];

    /* #Floats to follow; for the ENTIRE SET of CODEBOOKS */
    if (bio_fread (&n, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (total #floats) failed\n", file_name);
    if (n != n_mgau * n_density * blk) {
	E_FATAL("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
		file_name, n, n_mgau, n_density, blk);
    }
    
    /* Allocate memory for mixture gaussian densities if not already allocated */
    if (! g->mgau) {
	g->mgau = (mgau_t **) ckd_calloc_2d (n_mgau, n_feat, sizeof(mgau_t));
	g->max_n_mean = n_density;
    } else
	assert (g->max_n_mean == n_density);
    
    if (type == GAUDEN_MEAN) {
	if (! g->mgau[0][0].mean) {	/* Means can be reloaded multiple times */
	    buf = (float32 *) ckd_calloc (n, sizeof(float));
	    pbuf = (vector_t *) ckd_calloc (n_mgau * n_feat * n_density, sizeof(vector_t));
	    
	    for (i = 0; i < n_mgau; i++) {
		for (j = 0; j < n_feat; j++) {
		    g->mgau[i][j].mean = pbuf;
		    g->mgau[i][j].n_mean = n_density;
		    
		    for (k = 0; k < n_density; k++) {
			g->mgau[i][j].mean[k] = buf;
			buf += veclen[j];
		    }
		    pbuf += n_density;
		}
	    }
	} else
	    assert (g->mgau[0][0].n_mean == n_density);

	buf = g->mgau[0][0].mean[0];
    } else {
	assert (type == GAUDEN_VAR);
	assert (! g->mgau[0][0].var);	/* Can only load variance once */

	buf = (float32 *) ckd_calloc (n, sizeof(float));
	pbuf = (vector_t *) ckd_calloc (n_mgau * n_feat * n_density, sizeof(vector_t));
	
	for (i = 0; i < n_mgau; i++) {
	    for (j = 0; j < n_feat; j++) {
		g->mgau[i][j].var = pbuf;
		g->mgau[i][j].n_var = n_density;
		
		for (k = 0; k < n_density; k++) {
		    g->mgau[i][j].var[k] = buf;
		    buf += veclen[j];
		}
		pbuf += n_density;
	    }
	}

	buf = g->mgau[0][0].var[0];
    }
    
    /* Read mixture gaussian densities data */
    if (bio_fread (buf, sizeof(float32), n, fp, byteswap, &chksum) != n)
	E_FATAL("fread(%s) (densitydata) failed\n", file_name);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);

    if (fread (&tmp, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);
    
    fclose(fp);
    
    E_INFO("%d codebook, %d feature, size", n_mgau, n_feat);
    for (i = 0; i < n_feat; i++)
	fprintf (stderr, " %dx%d", n_density, veclen[i]);
    fprintf (stderr, "\n");

    if (type == GAUDEN_MEAN)
	gauden_cbmean0 (g);
    
    fflush (stderr);
    
    return 0;
}


/*
 * Some of the gaussian density computation can be carried out in advance:
 * 	log(determinant) calculation,
 * 	1/(2*var) in the exponent,
 * NOTE; The density computation is performed in log domain.
 */
static int32 gauden_dist_precompute (gauden_t *g)
{
    int32 i, m, n, f, d, flen;
    float32 *var, *det;

    E_INFO("Precomputing co-variance determinants, variance reciprocals\n");
    
    /* Allocate space for determinants */
    assert (! g->mgau[0][0].det);
    n = 0;
    for (m = 0; m < g->n_mgau; m++)
	for (f = 0; f < g->n_feat; f++)
	    n += g->mgau[m][f].n_var;
    det = (float32 *) ckd_calloc (n, sizeof(float32));
    for (m = 0; m < g->n_mgau; m++) {
	for (f = 0; f < g->n_feat; f++) {
	    g->mgau[m][f].det = det;
	    det += g->mgau[m][f].n_var;
	}
    }
    
    /* Precompute invariant portion of density function */
    for (m = 0; m < g->n_mgau; m++) {
	for (f = 0; f < g->n_feat; f++) {
	    flen = g->featlen[f];
	    det = g->mgau[m][f].det;

	    /* Determinants for all variance vectors in g->mgau[m][f] */
	    for (d = 0; d < g->mgau[m][f].n_var; d++) {
		var = g->mgau[m][f].var[d];

		det[d] = (float32) 0.0;
		for (i = 0; i < flen; i++) {
		    det[d] += (float32) (log(var[i]));
		    
		    /* Precompute this part of the exponential */
		    var[i] = (float32) (1.0 / (var[i] * 2.0));
		}

		/* 2 pi */
		det[d] += (float32) (flen * log(2.0 * M_PI));

		/* Sqrt */
		det[d] *= (float32) 0.5;
	    }
	}
    }

    return 0;
}


/* At the moment, S3 models have the same #means in each codebook and 1 var/mean */
gauden_t *gauden_init (char *meanfile, char *varfile, float64 varfloor, int32 det_varinv)
{
    gauden_t *g;
    
    assert (meanfile != NULL);
    assert (varfile != NULL);
    assert (varfloor >= 0.0);
    
    g = (gauden_t *) ckd_calloc (1, sizeof(gauden_t));
    g->min_density = logs3_to_log (LOGPROB_ZERO);	/* Floor for density values */
    
    /* Read means and (diagonal) variances for all mixture gaussians */
    gauden_param_read (meanfile, g, GAUDEN_MEAN);
    gauden_param_read (varfile, g, GAUDEN_VAR);
    gauden_var_floor (g, varfloor);
    
    if (det_varinv)
	gauden_dist_precompute (g);
    
    return g;
}


int32 gauden_mean_reload (gauden_t *g, char *meanfile)
{
    assert ((g->mgau != NULL) && (g->mgau[0][0].mean != NULL));
    
    gauden_param_read (meanfile, g, GAUDEN_MEAN);

    return 0;
}


int32 gauden_dist (gauden_t *g, int32 m, int32 f, vector_t obs, int32 *dist)
{
    int32 i, d, featlen, n_mean;
    vector_t m1, m2, v1, v2;
    float64 dval1, dval2, diff1, diff2;
    vector_t *mean, *var;
    float32 *det;
    
    assert ((m >= 0) && (m < g->n_mgau));
    assert ((f >= 0) && (f < g->n_feat));
    
    featlen = g->featlen[f];
    mean    = g->mgau[m][f].mean;
    var     = g->mgau[m][f].var;
    det     = g->mgau[m][f].det;
    n_mean  = g->mgau[m][f].n_mean;
    assert (g->mgau[m][f].n_var == n_mean);	/* For now n_var==1/0 not handled */

    /* Interleave two iterations for speed */
    for (d = 0; d < n_mean-1; d += 2) {
	m1 = mean[d];
	m2 = mean[d+1];
	v1 = var[d];
	v2 = var[d+1];
	dval1 = det[d];
	dval2 = det[d+1];
	
	for (i = 0; i < featlen; i++) {
	    diff1 = obs[i] - m1[i];
	    dval1 += diff1 * diff1 * v1[i];
	    diff2 = obs[i] - m2[i];
	    dval2 += diff2 * diff2 * v2[i];
	}
	
	dval1 = -dval1;			/* log(numerator) = -log(denominator) */
	dval2 = -dval2;
	if (dval1 < g->min_density)	/* Floor */
	    dval1 = g->min_density;
	if (dval2 < g->min_density)
	    dval2 = g->min_density;
	dist[d]   = log_to_logs3(dval1);
	dist[d+1] = log_to_logs3(dval2);
    }
    
    /* Remaining iteration if n_mean odd */
    if (d < n_mean) {
	m1 = mean[d];
	v1 = var[d];
	dval1 = det[d];
	
	for (i = 0; i < featlen; i++) {
	    diff1 = obs[i] - m1[i];
	    dval1 += diff1 * diff1 * v1[i];
	}
	
	dval1 = -dval1;
	if (dval1 < g->min_density)
	    dval1 = g->min_density;
	dist[d] = log_to_logs3(dval1);
    }

    return n_mean;
}


#if (_GAUDEN_TEST_)
main (int32 argc, char *argv[])
{
    gauden_t *g;
    float64 flr;
    float32 **feat, **mfc;
    int32 i, w, m, f, d, k, nfr, sf, ef, th, max, maxid;
    int32 *dist;
    char *cepfile;
    feat_t *fcb;
    FILE *fp, *fp2;
    char str[4096];

    if ((argc != 5) && (argc != 7))
	E_FATAL("Usage: %s meanfile varfile varfloor cepfile [sf ef]\n", argv[0]);

    if (sscanf (argv[3], "%lf", &flr) != 1)
	E_FATAL("Usage: %s meanfile varfile varfloor cepfile [sf ef]\n", argv[0]);

    cepfile = argv[4];

    if (argc > 5) {
	if ((sscanf (argv[5], "%d", &sf) != 1) || (sscanf (argv[6], "%d", &ef) != 1))
	    E_FATAL("Usage: %s meanfile varfile varfloor cepfile [sf ef]\n", argv[0]);
    } else {
	sf = 0;
	ef = 10000000;
    }
    
    logs3_init ((float64) 1.0003);
    
    fcb = feat_init ("s3_1x39");
    feat = feat_vector_alloc(fcb);
    mfc = (float32 **) ckd_calloc_2d (S3_MAX_FRAMES, feat_cepsize(fcb), sizeof(float32));
    
    th = logs3(1.0e-160);
    E_INFO("Threshold: %d\n", th);
    
    g = gauden_init (argv[1], argv[2], flr, TRUE);
#if 0
    gauden_dump (g);
#endif
    
    dist = (int32 *) ckd_calloc (g->max_n_mean, sizeof(int32));
    w = feat_window_size (fcb);
    
    nfr = s2mfc_read (cepfile, sf, ef, w, mfc, S3_MAX_FRAMES);
    E_INFO("%d frames\n", nfr);

    cmn (mfc, nfr, feat_cepsize(fcb));
    agc_max (mfc, nfr);
    
    for (i = w; i < nfr-w-1; i++) {
	if ((fp = fopen("gauden.out", "w")) == NULL)
	    E_FATAL("fopen(gauden.out,w) failed\n");
	if ((fp2 = fopen("gauden-top1.out", "w")) == NULL)
	    E_FATAL("fopen(gauden-top1.out,w) failed\n");
	
	fcb->compute_feat (fcb, mfc+i, feat);
	
	for (m = 0; m < g->n_mgau; m++) {
	    for (f = 0; f < g->n_feat; f++) {
		k = gauden_dist (g, m, f, feat[f], dist);
#if 0
		printf ("[%5d %3d]", m, f);
		for (d = 0; d < k; d++)
		    printf (" %11d", dist[d]);
		printf ("\n");
		fflush (stdout);
#else
		maxid = -1;
		for (d = 0; d < k; d++) {
		    if (dist[d] >= th)
			fprintf (fp, "%6d %2d %6d %12d\n",
				 i, f, m*g->max_n_mean + d, dist[d]);
		    if ((maxid < 0) || (dist[d] > dist[maxid])) {
			maxid = d;
			max = dist[d];
		    }
		}
		if (max >= th)
		    fprintf (fp2, "%6d %2d %6d %12d\n",
			     i, f, m*g->max_n_mean + maxid, max);
#endif
	    }
	}
#if 0
#else	
	fclose (fp);
	fclose (fp2);
	
	printf ("[%d %d %d] continue?", i, m, f);
	scanf ("%s", str);
#endif
    }
}
#endif
