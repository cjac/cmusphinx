/*
 * svqtest.c -- SubVQ module test
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
 * 12-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "subvq.h"
#include "feat.h"
#include "logs3.h"
#include "corpus.h"
#include "s3types.h"


typedef struct {
    feat_t *fcb;
    mgau_model_t *mgau;
    subvq_t *svq;
    int32 svqbeam;
    int32 nsen;
    int32 *sen_active;
    float32 ***feat;
    int32 *senscr;
    char *cepdir;
    ptmr_t tm;
} kb_t;


static arg_t arg[] = {
    { "-hmmdir",
      REQARG_STRING,
      NULL,
      "HMM directory" },
    { "-ctl",
      ARG_STRING,
      NULL,
      "Control file" },
    { "-ctloffset",
      ARG_INT32,
      "0",
      "Offset into control file" },
    { "-ctlcount",
      ARG_INT32,
      "1000000000",
      "No. of control file entries to process" },
    { "-cepdir",
      ARG_STRING,
      ".",
      "Cepstrum directory prefix (for entries in ctl file)" },
    { "-svqbeam",
      ARG_FLOAT64,
      "3.0e-3",
      "Beam selecting best components within each mixture Gaussian [0(widest)..1(narrowest)]" },
    { "-nsen",
      ARG_INT32,
      "-1",
      "No. of active senones (0, -1(all) or approx. 50%)" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


static void process_utt (void *data, char *uttfile, int32 sf, int32 ef, char *uttid)
{
    kb_t *kb;
    int32 utt_nfr, ns, ng;
    int32 f, i, j, r;
    
    kb = (kb_t *) data;
    utt_nfr = feat_s2mfc2feat (kb->fcb, uttfile, kb->cepdir, sf, ef, kb->feat, S3_MAX_FRAMES);
    
    ptmr_reset (&(kb->tm));
    ptmr_start (&(kb->tm));
    
    ns = 0;
    ng = 0;
    for (f = 0; f < utt_nfr; f++) {
	/* Determine active senones */
	if (kb->nsen == 0)
	    memset (kb->sen_active, 0, mgau_n_mgau(kb->mgau) * sizeof(int32));
	else if (kb->nsen >= mgau_n_mgau(kb->mgau)) {
	    for (i = 0; i < mgau_n_mgau(kb->mgau); i++)
		kb->sen_active[i] = 1;
	} else {	/* Randomly select about 50% of senones */
	    for (i = 0; i < mgau_n_mgau(kb->mgau); i += 32) {
		r = random();
		for (j = 0; j < 32; j++)
		    kb->sen_active[i+j] = ((r & (1 << j)) != 0);
	    }
	}
	
	subvq_frame_eval (kb->svq, kb->mgau, kb->svqbeam, kb->feat[f][0], kb->sen_active,
			  kb->senscr);
	ns += kb->mgau->frm_sen_eval;
	ng += kb->mgau->frm_gau_eval;
	if ((f % 100) == 0) {
	    printf (".");
	    fflush (stdout);
	}
    }
    printf ("\n");
    
    ptmr_stop (&(kb->tm));
    
    E_INFO("%5d frames,  %4d sen, %5d gau/fr,  %4.1f xCPU, %4.1f xClk\n", utt_nfr,
	   ns / utt_nfr, ng / utt_nfr,
	   (kb->tm.t_cpu * 100.0) / utt_nfr,
	   (kb->tm.t_elapsed * 100.0) / utt_nfr);
}


main (int32 argc, char *argv[])
{
    kb_t kb;
    char mean[8192], var[8192], mixw[8192], svq[8192];
    
    cmd_ln_parse (arg, argc, argv);
    
    logs3_init (1.0003);
    kb.fcb = feat_init ("s3_1x39", "current", "no", "none");
    
    sprintf (mean, "%s/mean", cmd_ln_str("-hmmdir"));
    sprintf (var, "%s/var", cmd_ln_str("-hmmdir"));
    sprintf (mixw, "%s/mixw", cmd_ln_str("-hmmdir"));
    sprintf (svq, "%s/subvq", cmd_ln_str("-hmmdir"));
    if ((kb.mgau = mgau_init (mean, var, 0.0001 /* varfloor */, mixw, 0.0000001, TRUE)) == NULL)
	E_FATAL("mgau_init() failed\n");
    if ((kb.svq = subvq_init (svq, 0.0001 /* varfloor */, -1, kb.mgau)) == NULL)
	E_FATAL("subvq_init() failed\n");
    kb.cepdir = cmd_ln_str("-cepdir");
    if ((kb.feat = feat_array_alloc (kb.fcb, S3_MAX_FRAMES)) == NULL)
	E_FATAL("feat_array_alloc() failed\n");
    kb.senscr = (int32 *) ckd_calloc (mgau_n_mgau(kb.mgau), sizeof(int32));
    kb.svqbeam = logs3 (cmd_ln_float64("-svqbeam"));
    kb.nsen = cmd_ln_int32 ("-nsen");
    if ((kb.nsen >= mgau_n_mgau(kb.mgau)) || (kb.nsen < 0))
	kb.nsen = mgau_n_mgau(kb.mgau);
    kb.sen_active = (int32 *) ckd_calloc (mgau_n_mgau(kb.mgau) + 32, sizeof(int32));
    
    E_INFO("logs3(svqbeam)= %d\n", kb.svqbeam);
    
    ctl_process (cmd_ln_str("-ctl"),
		 cmd_ln_int32("-ctloffset"),
		 cmd_ln_int32("-ctlcount"),
		 process_utt,
		 (void *)(&kb));
}
