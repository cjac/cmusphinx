/*
 * sensprd.c -- Spread of senone scores.
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
 * $Log$
 * Revision 1.1  2000/04/24  09:07:29  lenzo
 * Import s3.0.
 * 
 * 
 * 06-Oct-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>
#include <s3.h>

#include <libmain/gauden.h>
#include <libmain/senone.h>
#include <libmain/tmat.h>
#include <libmain/lm.h>
#include <libmain/fillpen.h>
#include <libmain/hmm.h>
#include <libmain/misc.h>
#include <libmain/cmn.h>
#include <libmain/agc.h>
#include <libmain/logs3.h>
#include <libmain/am.h>


static arg_t arglist[] = {
    { "-logbase",
      ARG_FLOAT32,
      "1.0001",
      "Base in which all log values calculated" },
    { "-feat",
      ARG_STRING,
      "s3_1x39",
      "Feature vector type (s3_1x39 or s2_4x)" },
    { "-mean",
      REQARG_STRING,
      NULL,
      "Mixture gaussian means input file" },
    { "-var",
      REQARG_STRING,
      NULL,
      "Mixture gaussian variances input file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to -var file)" },
    { "-sen2mgaumap",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" },
    { "-mixw",
      REQARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to -mixw file)" },
    { "-ctl",
      REQARG_STRING,
      NULL,
      "Control file listing utterances to be processed" },
    { "-cepdir",
      ARG_STRING,
      ".",
      "Directory of mfc files (for utterances in -ctl file)" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


static void process_ctlfile (acoustic_t *am, char *ctl)
{
    FILE *fp;
    char uttfile[4096];
    char uttid[4096];
    int32 sf, ef, nfr, featwin;
    char *cepdir;
    char cepfile[4096];
    float32 **mfc;
    int32 i, f, sum;
    float64 logp, p, sump, ent;
    
    if ((fp = fopen(ctl, "r")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctl);
    
    cepdir = (char *) cmd_ln_access ("-cepdir");
    featwin = feat_window_size ();
    
    while (_ctl_read (fp, uttfile, &sf, &ef, uttid) >= 0) {
	_ctl2cepfile (uttfile, cepdir, cepfile);
	
	/* nfr below includes the padding */
	if ((nfr = s2mfc_read (cepfile, sf, ef, featwin, &mfc)) < 0)
	    E_FATAL("MFC read (%s) failed\n", uttid);
	E_INFO("%s: %d frames\n", uttid, nfr - (featwin << 1));
	
	cmn (mfc, nfr, feat_cepsize());
	agc_max (mfc, nfr);

	for (f = featwin; f < nfr-featwin; f++) {
	    for (i = 0; i < am->sen->n_sen; i++)
		bitvec_set (am->sen_active, i);
	    acoustic_eval (am, mfc+f);

	    /* Find log(sum(senone-scores)) */
	    sum = LOGPROB_ZERO;
	    for (i = 0; i < am->sen->n_sen; i++)
		sum = logs3_add (sum, am->senscr[i]);
	    
	    sump = 0.0;
	    ent = 0.0;
	    for (i = 0; i < am->sen->n_sen; i++) {
		logp = logs3_to_log (am->senscr[i] - sum);
		p = exp(logp);
		sump += p;
		
		ent -= p * logp;
	    }
	    printf ("%5d %6.2f %7.2f\n", f-featwin, sump, ent);
	}
    }
    
    fclose (fp);
}


main (int32 argc, char *argv[])
{
    acoustic_t *am;
    gauden_t *g;
    senone_t *s;
    int32 cepsize;
    
    unlimit ();
    
    cmd_ln_parse (arglist, argc, argv);
    
    logs3_init ( *((float32 *) cmd_ln_access("-logbase")) );
    feat_init ((char *) cmd_ln_access ("-feat"));
    cepsize = feat_cepsize ();

    g = gauden_init ((char *) cmd_ln_access ("-mean"),
		     (char *) cmd_ln_access ("-var"),
		     *((float32 *) cmd_ln_access ("-varfloor")));
    s = senone_init ((char *) cmd_ln_access ("-mixw"),
		     (char *) cmd_ln_access ("-sen2mgaumap"),
		     *((float32 *) cmd_ln_access ("-mixwfloor")));
    am = acoustic_init (g, s);
    
    process_ctlfile (am, (char *)(cmd_ln_access("-ctl")));
}
