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


#include "cont_mgau.h"
#include "feat.h"
#include "corpus.h"
#include "logs3.h"
#include "s3types.h"


typedef struct {
    mgau_model_t *g;
    feat_t *fcb;
    float32 ***feat;
    char *cepdir;
    FILE *logfp;
} kb_t;


static int32 comp[] = {
    24, 25, 0, 12, 1, 13,
    -1,
};


static void process_utt (void *data, char *uttfile, int32 sf, int32 ef, char *uttid)
{
    kb_t *kb;
    int32 nf, bs, BS, BM;
    static int32 *score = NULL;
    float64 d;
    int32 f, s, c, i;
    
    kb = (kb_t *) data;
    
    if (! score)
	score = (int32 *) ckd_calloc (mgau_max_comp (kb->g), sizeof(int32));
    
    nf = feat_s2mfc2feat (kb->fcb, uttfile, kb->cepdir, sf, ef, kb->feat, S3_MAX_FRAMES);
    
    for (f = 0; f < nf; f++) {
	BS = MAX_NEG_INT32;
	BM = 0;
	for (s = 0; s < mgau_n_mgau(kb->g); s++) {
	    if (mgau_n_comp (kb->g, s) > 0) {
		bs = mgau_comp_eval (kb->g, s, kb->feat[f][0], score);
		if (BS < bs) {
		    BS = bs;
		    BM = s;
		}
	    }
	}
	
	for (s = 0; s < mgau_n_mgau(kb->g); s++) {
	    if (mgau_n_comp (kb->g, s) <= 0)
		continue;
	    
	    bs = mgau_comp_eval (kb->g, s, kb->feat[f][0], score);
	    
	    fprintf (kb->logfp,
		     "%6d (f), %5d (m), %5d (BM), %11d (BS), %11d (bs), %11d (bs-BS)\n",
		     f, s, BM, BS, bs, bs-BS);
	    
	    for (c = 0; c < mgau_n_comp (kb->g, s); c++) {
		fprintf (kb->logfp, "\t%3d %11d", c, score[c]-bs);
		
		for (i = 0; comp[i] >= 0; i++) {
		    d = kb->feat[f][0][comp[i]] - kb->g->mgau[s].mean[c][comp[i]];
		    if (d < 0)
			d = -d;
		    
		    fprintf (kb->logfp, "  %7.3f", d);
		}
		fprintf (kb->logfp, "\n");
	    }
	    fprintf (kb->logfp, "\n");
	}
    }
}


static void usagemsg (char *pgm)
{
    E_INFO("Usage: %s hmmdir cepdir ctlfile\n", pgm);
    exit(0);
}


main (int32 argc, char *argv[])
{
    char mean[8192], var[8192], mixw[8192];
    char *hmmdir, *ctlfile;
    float32 logbase;
    kb_t kb;
    
    if (argc != 4)
	usagemsg (argv[0]);
    
    logbase = (float32)1.0003;
    logs3_init (logbase);
    
    hmmdir = argv[1];
    sprintf (mean, "%s/mean", hmmdir);
    sprintf (var, "%s/var", hmmdir);
    sprintf (mixw, "%s/mixw", hmmdir);
    kb.g = mgau_init (mean, var, 0.0001 /* varfloor */, mixw, 0.0000001, TRUE);
    
    kb.fcb = feat_init ("s3_1x39", "current", "no", "max");
    kb.feat = feat_array_alloc(kb.fcb, S3_MAX_FRAMES);
    kb.cepdir = argv[2];
    kb.logfp = stdout;
    
    ctlfile = argv[3];
    ctl_process (ctlfile, 0, 10000000, process_utt, (void *)(&kb));
}
