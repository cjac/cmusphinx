/*
 * lmtest.c -- Interactive tests on an LM file
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
 * 17-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "lm.h"
#include "logs3.h"


static arg_t arg[] = {
    { "-lm",
      REQARG_STRING,
      NULL,
      "LM dump file" },
    { "-lw",
      ARG_FLOAT64,
      "9.0",
      "Language weight" },
    { "-wip",
      ARG_FLOAT64,
      "0.2",
      "Word insertion penalty" },
    { "-uw",
      ARG_FLOAT64,
      "0.7",
      "Unigram weight" },
    
    { NULL, ARG_INT32, NULL, NULL }
};


int32 main (int32 argc, char *argv[])
{
    lm_t *lm;
    char line[8192], wd[1024], *lp;
    s3lmwid_t wid[3], unkwid;
    int32 lscr, sum;
    int32 n;
    
    cmd_ln_parse (arg, argc, argv);
    
    logs3_init (1.0003);
    
    lm = lm_read (cmd_ln_str("-lm"),
		  cmd_ln_float64("-lw"),
		  cmd_ln_float64("-wip"),
		  cmd_ln_float64("-uw"));
    
    unkwid = lm_wid (lm, S3_UNKNOWN_WORD);
    
    for (;;) {
	fprintf (stderr, "\nUtt: ");
	if (fgets (line, sizeof(line), stdin) == NULL)
	    break;
	
	wid[0] = BAD_S3LMWID;
	wid[1] = BAD_S3LMWID;
	
	lp = line;
	sum = 0;
	while (sscanf (lp, "%s%n", wd, &n) == 1) {
	    lp += n;
	    
	    wid[2] = lm_wid (lm, wd);
	    
	    if (NOT_S3LMWID(wid[2])) {
		if (NOT_S3LMWID(unkwid)) {
		    E_ERROR("Unknown word: '%s'\n", wd);
		    break;
		}
		
		E_ERROR("Unknown word: '%s', using %s\n", wd, S3_UNKNOWN_WORD);
		wid[2] = unkwid;
	    }
	    
	    lscr = lm_tg_score (lm, wid[0], wid[1], wid[2]);
	    sum += lscr;
	    
	    E_INFO("\t%8d (lscr);  %10d (sum);  %d (access-type)\t%s\n",
		   lscr, sum, lm_access_type (lm), wd);
	    
	    wid[0] = wid[1];
	    wid[1] = wid[2];
	}
    }
    
    exit(0);
}
