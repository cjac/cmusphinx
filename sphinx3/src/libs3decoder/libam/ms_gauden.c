/*
 * gauden.c -- gaussian density module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * HISTORY
 * $Log$
 * Revision 1.6  2005/10/05  00:31:14  dhdfu
 * Make int8 be explicitly signed (signedness of 'char' is
 * architecture-dependent).  Then make a bunch of things use uint8 where
 * signedness is unimportant, because on the architecture where 'char' is
 * unsigned, it is that way for a reason (signed chars are slower).
 * 
 * Revision 1.5  2005/06/21 18:55:09  arthchan2003
 * 1, Add comments to describe this modules, 2, Fixed doxygen documentation. 3, Added $ keyword.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed gauden_param_read to use the new libio/bio_fread functions.
 * 
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added gauden_mean_reload() for application of MLLR; and correspondingly
 * 		made gauden_param_read allocate memory for parameter only if not
 * 		already allocated.
 * 
 * 09-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Interleaved two density computations for speed improvement.
 * 
 * 19-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added compute_dist_all special case for improving speed.
 * 
 * 26-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added check for underflow and floor insertion in gauden_dist.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added active argument to gauden_dist_norm and gauden_dist_norm_global,
 * 		and made the latter a static function.
 * 
 * 07-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Initial version created.
 * 		Very liberally borrowed/adapted from Eric's S3 trainer implementation.
 */


#include "ms_gauden.h"
#include "logs3.h"

/* #include <s3.h> */

#include "bio.h"

#include <assert.h>
#include <string.h>
#include <math.h>
#include <float.h>


#define GAUDEN_PARAM_VERSION	"1.0"

#undef M_PI
#define M_PI	3.1415926535897932385e0


static float64 min_density;	/* Density values, once converted to (int32)logs3 domain,
				   can underflow (or overflow?), causing headaches all
				   around.  To avoid underflow, use this floor value */

#if 0
void gauden_dump (const gauden_t *g)
{
    int32 c, f, d, i;
    
    for (c = 0; c < g->n_mgau; c++) {
	for (f = 0; f < g->n_feat; f++) {
	    E_INFO ("Codebook %d, Feature %d (%dx%d):\n",
		    c, f, g->n_density, g->featlen[f]);
	    
	    for (d = 0; d < g->n_density; d++) {
		printf ("m[%3d]", d);
		for (i = 0; i < g->featlen[f]; i++)
		    printf (" %7.4f", g->mean[c][f][d][i]);
		printf ("\n");
	    }
	    printf ("\n");
	    
	    for (d = 0; d < g->n_density; d++) {
		printf ("v[%3d]", d);
		for (i = 0; i < g->featlen[f]; i++)
		    printf (" %7.4f", g->var[c][f][d][i]);
		printf ("\n");
	    }
	    printf ("\n");

	    for (d = 0; d < g->n_density; d++)
		printf ("d[%3d] %7.4f\n", d, g->det[c][f][d]);
	}
    }
}
#endif


static int32 gauden_param_read(vector_t ****out_param,	/* Alloc space iff *out_param == NULL */
			       int32 *out_n_mgau,
			       int32 *out_n_feat,
			       int32 *out_n_density,
			       int32 **out_veclen,
			       const char *file_name)
{
    char tmp;
    FILE *fp;
    int32 i, j, k, l, n, blk;
    int32 n_mgau;
    int32 n_feat;
    int32 n_density;
    int32 *veclen;
    int32 byteswap, chksum_present;
    vector_t ***out;
    float32 *buf;
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
    *out_n_mgau = n_mgau;

    /* #Features/codebook */
    if (bio_fread (&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#features) failed\n", file_name);
    *out_n_feat = n_feat;

    /* #Gaussian densities/feature in each codebook */
    if (bio_fread (&n_density, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#density/codebook) failed\n", file_name);
    *out_n_density = n_density;
    
    /* #Dimensions in each feature stream */
    veclen = ckd_calloc(n_feat, sizeof(uint32));
    *out_veclen = veclen;
    if (bio_fread (veclen, sizeof(int32), n_feat, fp, byteswap, &chksum) != n_feat)
	E_FATAL("fread(%s) (feature-lengths) failed\n", file_name);

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
    if (! (*out_param)) {
	out = (vector_t ***) ckd_calloc_3d (n_mgau, n_feat, n_density,
					    sizeof(vector_t));
	buf = (float32 *) ckd_calloc (n, sizeof(float));
	for (i = 0, l = 0; i < n_mgau; i++) {
	    for (j = 0; j < n_feat; j++) {
		for (k = 0; k < n_density; k++) {
		    out[i][j][k] = &buf[l];
		    
		    l += veclen[j];
		}
	    }
	}
    } else {
	out = *out_param;
	buf = out[0][0][0];
    }
    
    /* Read mixture gaussian densities data */
    if (bio_fread (buf, sizeof(float32), n, fp, byteswap, &chksum) != n)
	E_FATAL("fread(%s) (densitydata) failed\n", file_name);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);

    if (fread (&tmp, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);
    
    fclose(fp);

    *out_param = out;
    
    E_INFO("%d codebook, %d feature, size",
	   n_mgau, n_feat);
    for (i = 0; i < n_feat; i++)
	printf (" %dx%d", n_density, veclen[i]);
    printf ("\n");

    return 0;
}


/*
 * Some of the gaussian density computation can be carried out in advance:
 * 	log(determinant) calculation,
 * 	1/(2*var) in the exponent,
 * NOTE; The density computation is performed in log domain.
 */
static int32 gauden_dist_precompute (gauden_t *g, float32 varfloor)
{
    int32 i, m, f, d, flen;
    float32 *varp, *detp;

    /* Allocate space for determinants */
    g->det = (float32 ***) ckd_calloc_3d (g->n_mgau, g->n_feat, g->n_density,
					  sizeof(float32));

    for (m = 0; m < g->n_mgau; m++) {
	for (f = 0; f < g->n_feat; f++) {
	    flen = g->featlen[f];
	    
	    /* Determinants for all variance vectors in g->[m][f] */
	    for (d = 0, detp = g->det[m][f]; d < g->n_density; d++, detp++) {
		*detp = (float32) 0.0;

		for (i = 0, varp = g->var[m][f][d]; i < flen; i++, varp++) {
		    if (*varp < varfloor)
			*varp = varfloor;

		    *detp += (float32) (log(*varp));
		    
		    /* Precompute this part of the exponential */
		    *varp = (float32) (1.0 / (*varp * 2.0));
		}

		/* 2pi */
		*detp += (float32) (flen * log(2.0 * M_PI));

		/* Sqrt */
		*detp *= (float32) 0.5;
	    }
	}
    }

    return 0;
}


gauden_t *gauden_init (char *meanfile, char *varfile, float32 varfloor)
{
    int32 i, m, f, d, *flen;
    gauden_t *g;
    
    assert (meanfile != NULL);
    assert (varfile != NULL);
    assert (varfloor > 0.0);
    
    g = (gauden_t *) ckd_calloc (1, sizeof(gauden_t));
    g->mean = g->var = NULL;	/* To force them to be allocated */
    
    /* Read means and (diagonal) variances for all mixture gaussians */
    gauden_param_read (&(g->mean), &g->n_mgau, &g->n_feat, &g->n_density, &g->featlen,
		       meanfile);
    gauden_param_read (&(g->var), &m, &f, &d, &flen, varfile);
    
    /* Verify mean and variance parameter dimensions */
    if ((m != g->n_mgau) || (f != g->n_feat) || (d != g->n_density))
	E_FATAL("Mixture-gaussians dimensions for means and variances differ\n");
    for (i = 0; i < g->n_feat; i++)
	if (g->featlen[i] != flen[i])
	    E_FATAL("Feature lengths for means and variances differ\n");
    ckd_free (flen);

    /* Floor variances and precompute variance determinants */
    gauden_dist_precompute (g, varfloor);

    /* Floor for density values */
    min_density = logs3_to_log (S3_LOGPROB_ZERO);
    
    return g;
}


int32 gauden_mean_reload (gauden_t *g, char *meanfile)
{
    int32 i, m, f, d, *flen;
    
    assert (g->mean != NULL);
    
    gauden_param_read (&(g->mean), &m, &f, &d, &flen, meanfile);
    
    /* Verify original and new mean parameter dimensions */
    if ((m != g->n_mgau) || (f != g->n_feat) || (d != g->n_density))
	E_FATAL("Mixture-gaussians dimensions for original and new means differ\n");
    for (i = 0; i < g->n_feat; i++)
	if (g->featlen[i] != flen[i])
	    E_FATAL("Feature lengths for original and new means differ\n");
    ckd_free (flen);

    return 0;
}

/*
 * Temporary structure for computing density values.  The only difference between
 * this and gauden_dist_t is the use of float64 for dist.
 */


typedef struct {
    int32 id;
    float64 dist;		/* Can probably use float32 */
} dist_t;

static dist_t *dist;
static int32 n_dist = 0;


/* See compute_dist below */
static int32 compute_dist_all (dist_t *out_dist, vector_t obs, int32 featlen,
			       vector_t *mean, vector_t *var, float32 *det, int32 n_density)
{
    int32 i, d;
    vector_t m1, m2, v1, v2;
    float64 dval1, dval2, diff1, diff2;

    for (d = 0; d < n_density-1; d += 2) {
	m1 = mean[d];
	v1 = var[d];
	dval1 = det[d];
	m2 = mean[d+1];
	v2 = var[d+1];
	dval2 = det[d+1];
	
	for (i = 0; i < featlen; i++) {
	    diff1 = obs[i] - m1[i];
	    dval1 += diff1 * diff1 * v1[i];
	    diff2 = obs[i] - m2[i];
	    dval2 += diff2 * diff2 * v2[i];
	}
	
	out_dist[d].dist = dval1;
	out_dist[d].id = d;
	out_dist[d+1].dist = dval2;
	out_dist[d+1].id = d+1;
    }

    if (d < n_density) {
	m1 = mean[d];
	v1 = var[d];
	dval1 = det[d];
	
	for (i = 0; i < featlen; i++) {
	    diff1 = obs[i] - m1[i];
	    dval1 += diff1 * diff1 * v1[i];
	}
	
	out_dist[d].dist = dval1;
	out_dist[d].id = d;
    }

    return 0;
}


/*
 * Compute the top-N closest gaussians from the chosen set (mgau,feat)
 * for the given input observation vector.
 * NOTE: The density values computed are in log-domain, and while they are being
 * computed they're really the DENOMINATOR of the distance expression.
 */
static int32 compute_dist (dist_t *out_dist, int32 n_top,
			   vector_t obs, int32 featlen,
			   vector_t *mean, vector_t *var, float32 *det, int32 n_density)
{
    int32 i, j, d;
    dist_t *worst;
    vector_t m, v;
    float64 dval, diff;

    /* Special case optimization when n_density <= n_top */
    if (n_top >= n_density)
	return (compute_dist_all (out_dist, obs, featlen, mean, var, det, n_density));

    /*
     * We are really computing denominators in the gaussian density expression:
     *   sqrt(2pi * det), and (x-mean)^2*var.
     * To maximize the density value, we want to minimize the denominators.
     */
    
    for (i = 0; i < n_top; i++)
	out_dist[i].dist = DBL_MAX;
    worst = &(out_dist[n_top-1]);

    for (d = 0; d < n_density; d++) {
	m = mean[d];
	v = var[d];
	dval = det[d];

	for (i = 0; (i < featlen) && (dval <= worst->dist); i++) {
	    diff = obs[i] - m[i];
	    dval += diff * diff * v[i];
	}
	
	if ((i < featlen) || (dval >= worst->dist))	/* Codeword d worse than worst */
	    continue;

	/* Codeword d at least as good as worst so far; insert in the ordered list */
	for (i = 0; (i < n_top) && (dval >= out_dist[i].dist); i++);
	assert (i < n_top);
	for (j = n_top-1; j > i; --j)
	    out_dist[j] = out_dist[j-1];
	out_dist[i].dist = dval;
	out_dist[i].id = d;
    }
    
    return 0;
}


#if 1
/*
 * Compute distances of the input observation from the top N codewords in the given
 * codebook (g->{mean,var}[mgau]).  The input observation, obs, includes vectors for
 * all features in the codebook.
 */
int32 gauden_dist (gauden_t *g,
		   s3mgauid_t mgau,
		   int32 n_top,
		   vector_t *obs,
		   gauden_dist_t **out_dist)
{
    int32 f, t;
    
    assert ((n_top > 0) && (n_top <= g->n_density));
    
    /* Allocate temporary space for distance computation, if necessary */
    if (n_dist < n_top) {
	if (n_dist > 0)
	    ckd_free (dist);
	n_dist = n_top;
	dist = (dist_t *) ckd_calloc (n_dist, sizeof(dist_t));
    }
    
    for (f = 0; f < g->n_feat; f++) {
	compute_dist (dist, n_top,
		      obs[f], g->featlen[f],
		      g->mean[mgau][f], g->var[mgau][f], g->det[mgau][f], g->n_density);

	/*
	 * Convert distances to logs3 domain and return result.  Remember that until now,
	 * we've been computing log(DENOMINATOR) of the normal density function.
	 * Check for underflow before converting to (int32)logs3 value.
	 */
	for (t = 0; t < n_top; t++) {
	    out_dist[f][t].id = dist[t].id;

	    dist[t].dist = -dist[t].dist;	/* log(numerator) = -log(denom.) */
	    if (dist[t].dist < min_density) {
#if 0
		E_ERROR("Density[%d][%d][%d] too small (%.3e); flooring it to %.3e\n",
		       mgau, f, dist[t].id, dist[t].dist, min_density);
#endif
		dist[t].dist = min_density;
	    }
	    out_dist[f][t].dist = (int32) log_to_logs3 (dist[t].dist);
	}
    }
    
    return 0;
}
#endif

/*
 * Normalize density values, but globally.
 */
static int32 gauden_dist_norm_global (gauden_t *g,
				      int32 n_top, gauden_dist_t ***dist, uint8 *active)
{
    int32 gid, f, t;
    int32 best;

    best = S3_LOGPROB_ZERO;
    
    for (gid = 0; gid < g->n_mgau; gid++) {
	if ((! active) || active[gid]) {
	    for (f = 0; f < g->n_feat; f++) {
		for (t = 0; t < n_top; t++)
		    if (best < dist[gid][f][t].dist)
			best = dist[gid][f][t].dist;
	    }
	}
    }

    for (gid = 0; gid < g->n_mgau; gid++) {
	if ((! active) || active[gid]) {
	    for (f = 0; f < g->n_feat; f++) {
		for (t = 0; t < n_top; t++)
		    dist[gid][f][t].dist -= best;
	    }
	}
    }
    
    return (best * g->n_feat);	/* Scale factor applied to EVERY senone score */
}


/*
 * Normalize density values.
 */
int32 gauden_dist_norm (gauden_t *g, int32 n_top, gauden_dist_t ***dist, uint8 *active)
{
    int32 gid, f, t;
    int32 sum, scale;

    if (g->n_mgau > 1) {
	/* Normalize by subtracting max(density values) from each density */
	return (gauden_dist_norm_global (g, n_top, dist, active));
    }
    
    /* Normalize by subtracting log(sum of density values) from each density */
    gid = 0;
    scale = 0;
    for (f = 0; f < g->n_feat; f++) {
	sum = dist[gid][f][0].dist;
	for (t = 1; t < n_top; t++)
	    sum = logs3_add (sum, dist[gid][f][t].dist);
	
	for (t = 0; t < n_top; t++)
	    dist[gid][f][t].dist -= sum;

	scale += sum;
    }
    
    return scale;	/* Scale factor applied to EVERY senone score */
}
