/*
 * gautest.c -- Gaussian density tests
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 05-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libfeat/libfeat.h>
#include <libmain/gauden.h>

#include "subvq.h"


static void usagemsg (char *pgm)
{
    E_INFO("Usage: %s meanfile varfile vqfile featfile beam\n", pgm);
    exit(0);
}


static int32 chk_mean0 (float32 *mean, int32 len)
{
    int32 i;
    
    for (i = 0; (i < len) && (mean[i] == 0.0); i++);
    return (i == len);	/* TRUE iff all mean values are 0.0 */
}


/* Create and return a bitvector marking codebooks (mixture Gaussians) whose means are all 0.0 */
static bitvec_t chk_cb0 (gauden_t *g)
{
    int32 m, c, n;
    bitvec_t bv;
    
    E_INFO("Checking codebooks with all 0.0 means\n");
    
    bv = bitvec_alloc (gauden_n_mgau(g));
    bitvec_clear_all (bv, gauden_n_mgau(g));
    
    n = 0;
    for (m = 0; m < gauden_n_mgau(g); m++) {
	for (c = 0; c < gauden_n_mean(g, m, 0); c++) {
	    if (! chk_mean0(g->mgau[m][0].mean[c], g->featlen[0]))
		break;
	}
	
	if (c == gauden_n_mean(g, m, 0)) {
	    bitvec_set (bv, m);
	    n++;
	    printf (" %d", m);
	}
    }
    printf ("\n");
    
    E_INFO("%d codebooks are all 0.0\n", n);
    
    return bv;
}


main (int32 argc, char *argv[])
{
    gauden_t *g;
    float32 **feat;
    float64 fbeam;
    int32 nfr, beam, best;
    int32 *dist;
    char *meanfile, *varfile, *vqfile, *featfile, *beamarg;
    feat_t *fcb;
    int32 i, j, k, f, m;
    int32 n_match, n_match_tot, n_cand, n_cand_tot;
    int32 bv, bc, bv2, bc2, *v2;
    char str[4096];
    bitvec_t cb0;
    subvq_t *vq;
    int32 **vqdist;
    ptmr_t *tm;
    FILE *fp;
    
    if (argc != 6)
	usagemsg (argv[0]);
    
    meanfile = argv[1];
    varfile = argv[2];
    vqfile = argv[3];
    featfile = argv[4];
    beamarg = argv[5];
    
    if (sscanf (beamarg, "%lf", &fbeam) != 1)
	usagemsg (argv[0]);
    
    E_INFO("Assuming single feature-stream\n");
    
    logs3_init ((float64) 1.0003);
    
    fcb = feat_init ("s3_1x39");
    feat = feat_vector_alloc(fcb);
    assert (feat_n_stream(fcb) == 1);
    
    fp = myfopen(featfile, "r");
    
    beam = logs3(fbeam);
    E_INFO("logs3(beam)= %d\n", beam);
    
    g = gauden_init (meanfile, varfile, 0.0001 /* varfloor */, TRUE);
    assert (g->n_feat == 1);
    cb0 = chk_cb0 (g);

    vq = subvq_init (vqfile);
    for (i = 0; i < vq->n_sv; i++) {
	vector_print (stdout, vq->mean[i][0], vq->svsize[i]);
	vector_print (stdout, vq->var[i][0], vq->svsize[i]);
	printf (" %e\n", vq->idet[i][0]);
    }
    
    dist = (int32 *) ckd_calloc (g->max_n_mean, sizeof(int32));
    v2 = (int32 *) ckd_calloc (g->max_n_mean, sizeof(int32));
    vqdist = (int32 **) ckd_calloc_2d (vq->n_sv, vq->vqsize, sizeof(int32));
    
    tm = (ptmr_t *) ckd_calloc (1, sizeof(ptmr_t));
    ptmr_init (tm);
    
    n_match_tot = 0;	/* #Times best VQ codeword and original codeword same */
    n_cand_tot = 0;	/* #Best VQ codeword candidates (i.e., within beam of best) */
    
    nfr = 0;
    for (f = 0;; f++) {
	for (i = 0; i < feat_stream_len(fcb,0); i++)
	    if (fscanf (fp, "%f", &(feat[0][i])) != 1)
		break;
	if (i < feat_stream_len(fcb,0))
	    break;
	nfr++;
	
	subvq_dist_eval (vq, feat[0], vqdist);
	for (i = 0; i < vq->n_sv; i++)
	    printf (" %11d\n", vqdist[i][0]);
	
	n_match = 0;
	n_cand = 0;
	
	for (m = 0; m < g->n_mgau; m++) {
	    if (bitvec_is_set(cb0, m))
		continue;
	    
	    k = gauden_dist (g, m, 0, feat[0], dist);
	    
	    /* Find the best density in this codebook */
	    bv = dist[0];
	    bc = 0;
	    for (i = 1; i < k; i++) {
		if (dist[i] > bv) {
		    bv = dist[i];
		    bc = i;
		}
	    }
	    
	    /* Find the best density according to subvq */
	    for (i = 0; i < k; i++) {
		v2[i] = 0;
		
		for (j = 0; j < vq->n_sv; j++)
		    v2[i] += vqdist[j][vq->map[m][i][j]];
		
		if ((i == 0) || (v2[i] > bv2)) {
		    bv2 = v2[i];
		    bc2 = i;
		}
	    }
	    /* Find all densities within beam */
	    for (i = 0; i < k; i++) {
		if (v2[i] >= bv2 + beam) {
		    n_cand++;
		    if (bc == i)
			n_match++;
		}
	    }
#if 1
	    printf ("Fr[%3d]St[%4d]:  Best-Orig: %2d %11d;  SVQ: %11d;   Best-SVQ: %2d %11d;  Orig: %11d\n",
		    f, m, bc, bv, v2[bc], bc2, bv2, dist[bc2]);
#endif
	}
	
#if 1
	printf ("\n");
	vector_print (stderr, feat[0], 39);
	
	fprintf (stderr, "Fr %d:  #Matches %d, #Candidates %d; Continue? ", f, n_match, n_cand);
	fgets (str, sizeof(str), stdin);
	if ((str[0] == 'n') || (str[0] == 'N') || (str[0] == 'q') || (str[0] == 'Q'))
	    break;
#else
	printf ("Fr %d:  #Matches %5d, #Candidates %7d\n", f, n_match, n_cand);
#endif
	n_match_tot += n_match;
	n_cand_tot += n_cand;
	
	/* Repeat, just for timing */
	ptmr_start (tm);
	for (m = 0; m < g->n_mgau; m++) {
	    if (bitvec_is_set(cb0, m))
		continue;
	    
	    /* Find the best density according to subvq */
	    for (i = 0; i < g->max_n_mean; i++) {
		v2[i] = 0;
		
		for (j = 0; j < vq->n_sv; j++)
		    v2[i] += vqdist[j][vq->map[m][i][j]];
		
		if ((i == 0) || (v2[i] > bv2)) {
		    bv2 = v2[i];
		    bc2 = i;
		}
	    }
	}
	ptmr_stop (tm);
    }
    
    if (nfr > 0) {
	printf ("%5d frames;   %d match/fr, %d cand/fr;   %.1fs CPU, %.2f xRT;   %.1fs Elapsed, %.2f xRT\n",
		nfr, n_match_tot/nfr, n_cand_tot/nfr, tm->t_cpu, tm->t_cpu*100.0/nfr,
		tm->t_elapsed, tm->t_elapsed*100.0/nfr);
    }
}
