/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * cepdist.c -- Cepstrum vector distance (Euclidean) between all given vectors
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
 * 21-Jan-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started
 */


#include <libutil/libutil.h>
#include <s3.h>
#include <libio/libio.h>
#include <libfeat/libfeat.h>


static int32 cepsize;		/* Cepstrum vector length */

static arg_t defn[] = {
    { "-feat",
      CMD_LN_STRING,
      "s3_1x39",
      "Feature stream:\n\t\t\t\ts2_4x: Sphinx-II type 4 streams, 12cep, 24dcep, 3pow, 12ddcep\n\t\t\t\ts3_1x39: Single stream, 12cep+12dcep+3pow+12ddcep" },
    { "-ctlfn",
      CMD_LN_STRING,
      NULL,
      "Input control file listing utterances to be decoded" },
    { "-ctloffset",
      CMD_LN_INT32,
      "0",
      "No. of utterances at the beginning of -ctlfn file to be skipped" },
    { "-ctlcount",
      CMD_LN_INT32,
      "2147483640",
      "No. of utterances in -ctlfn file to be processed (after skipping -ctloffset)" },
    { "-win",
      CMD_LN_INT32,
      "20",
      "No. frames before and after for which cepdist computed" },
    { "-cepdir",
      CMD_LN_STRING,
      ".",
      "Directory for utterances in -ctlfn file (if relative paths specified)." },
    { "-cepext",
      CMD_LN_STRING,
      "mfc",
      "File extension appended to utterances listed in -ctlfn file" },
    
    { NULL, CMD_LN_INT32, NULL, NULL }
};


static float64 cepdist (float32 *f1, float32 *f2, int32 veclen)
{
    int32 i;
    float64 dist, d;
    
    dist = 0.0;
    for (i = 0; i < veclen; i++) {
	d = f1[i] - f2[i];
	dist += d*d;
    }

    return dist;
}


#if 0
static void process_utt (char *uttid, float32 **mfc, int32 nfr, int32 sf)
{
    int32 i, j, k;
    float64 d;
    float32 *diff1, *diff2;
    
    printf ("    ");
    for (i = 0; i < nfr; i++)
	printf (" %3d", (sf+i)%1000);
    printf ("\n");
    printf ("    ----------------------------------------------------------------\n");
    
    for (i = 0; i < nfr; i++) {
	printf ("%3d ", (sf+i)%1000);
	
	for (j = 0; j < nfr; j++) {
	    d = cepdist (mfc[i], mfc[j], cepsize);
	    printf (" %3d", (int32)(d*10.0));
	}
	printf ("\n");
    }
    printf ("\n");
    
    diff1 = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    diff2 = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    
    for (i = 1; i < nfr; i++) {
	for (k = 0; k < cepsize; k++)
	    diff1[k] = mfc[i][k] - mfc[i-1][k];
	
	printf ("%3d     ", (sf+i)%1000);
	
	for (j = 1; j < nfr; j++) {
	    for (k = 0; k < cepsize; k++)
		diff2[k] = mfc[j][k] - mfc[j-1][k];
	    
	    d = cepdist (diff1, diff2, cepsize);
	    printf (" %3d", (int32)(d*10.0));
	}
	printf ("\n");
    }
    printf ("\n");

    ckd_free (diff1);
    ckd_free (diff2);
}

#else

static void process_utt (char *uttid, float32 **mfc, int32 nfr, int32 win, int32 sf)
{
    int32 i, j, k;
    float64 d;
    float32 *diff1, *diff2;
    
    printf ("    ");
    for (i = 0; i <= win; i++)
	printf (" %3d", i);
    printf ("\n");
    printf ("    ----------------------------------------------------------------\n");
    
    for (i = 0; i < nfr; i++) {
	printf ("%3d ", (sf+i)%1000);
	
	for (j = i; (j < nfr) && (j <= i+win); j++) {
	    d = cepdist (mfc[i], mfc[j], cepsize);
	    printf (" %3d", (int32)(d*10.0));
	}
	printf ("\n");
    }
    printf ("\n");
    
    diff1 = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    diff2 = (float32 *) ckd_calloc (cepsize, sizeof(float32));
    
    for (i = 1; i < nfr; i++) {
	for (k = 0; k < cepsize; k++)
	    diff1[k] = mfc[i][k] - mfc[i-1][k];
	
	printf ("%3d ", (sf+i)%1000);
	
	for (j = i; (j < nfr) && (j < i+win); j++) {
	    for (k = 0; k < cepsize; k++)
		diff2[k] = mfc[j][k] - mfc[j-1][k];
	    
	    d = cepdist (diff1, diff2, cepsize);
	    printf (" %3d", (int32)(d*10.0));
	}
	printf ("\n");
    }
    printf ("\n");

    ckd_free (diff1);
    ckd_free (diff2);
}
#endif


/* Process utterances in the control file (-ctlfn argument) */
static void process_ctlfile (char *ctlfile)
{
    FILE *ctlfp;
    char *cepdir, *cepext;
    char line[4096], cepfile[4096], uttid[4096], ctlspec[4096];
    int32 ctloffset, ctlcount, sf, ef, nfr;
    int32 i, k, win;
    float32 **mfc;
    
    if (strcmp (ctlfile, "-") == 0)
	ctlfp = stdin;
    else {
	if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	    E_FATAL("fopen(%s,r) failed\n", ctlfile);
    }
    E_INFO("Processing ctl file %s\n", (ctlfp == stdin) ? "(stdin)" : ctlfile);
    
    cepdir = (char *) cmd_ln_access("-cepdir");
    cepext = (char *) cmd_ln_access("-cepext");
    assert ((cepdir != NULL) && (cepext != NULL));
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));
    win = *((int32 *) cmd_ln_access("-win"));
    
    /* Skipping initial offset */
    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", cepfile) > 0)
	    --ctloffset;
    }

    /* Process the specified number of utterance or until end of control file */
    while ((ctlcount > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	printf ("\n");
	E_INFO("Utterance: %s", line);
	
	sf = 0;
	ef = (int32)0x7ffffff0;
	if ((k = sscanf (line, "%s %d %d %s", cepfile, &sf, &ef, uttid)) <= 0)
	    continue;	    /* Empty line */

	if ((k == 2) || ( (k >= 3) && ((sf >= ef) || (sf < 0))) )
	    E_FATAL("Error in ctlfile: %s\n", line);

	if (k < 4) {
	    /* Create utt-id from mfc-filename (and sf/ef if specified) */
	    for (i = strlen(cepfile)-1; (i >= 0) && (cepfile[i] != '/'); --i);
	    if (k == 3)
		sprintf (uttid, "%s_%d_%d", cepfile+i+1, sf, ef);
	    else
		strcpy (uttid, cepfile+i+1);
	}

	strcpy (ctlspec, cepfile);
	if (ctlspec[0] != '/')
	    sprintf (cepfile, "%s/%s.%s", cepdir, ctlspec, cepext);
	else
	    sprintf (cepfile, "%s.%s", ctlspec, cepext);
	
	/* Read and process mfc file */
	if ((nfr = s2mfc_read (cepfile, sf, ef, &mfc)) <= 0)
	    E_ERROR("Utt %s: MFC file read (%s) failed\n", uttid, cepfile);
	else {
	    E_INFO ("%d mfc frames\n", nfr);
	    process_utt (uttid, mfc, nfr, win, sf);
	}
	
	--ctlcount;
    }
    printf ("\n");

    while (fgets(line, sizeof(line), ctlfp) != NULL) {
	if (sscanf (line, "%s", ctlspec) > 0) {
	    E_INFO("Skipping rest of control file beginning with:\n\t%s", line);
	    break;
	}
    }

    if (ctlfp != stdin)
	fclose (ctlfp);
}


main (int32 argc, char *argv[])
{
    char *ctlfile;

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    if (argc < 2) {
	cmd_ln_print_help (stderr, defn);
	exit(0);
    }

    cmd_ln_parse (defn, argc, argv);

    unlimit ();
    
#if (! WIN32)
    {
	char buf[1024];
	
	gethostname (buf, 1024);
	buf[1023] = '\0';
	E_INFO ("Executing on: %s\n", buf);
    }
#endif

    /* Initialize feature stream type */
    feat_init ((char *) cmd_ln_access ("-feat"));
    cepsize = feat_cepsize ();
    
    if ((ctlfile = (char *) cmd_ln_access("-ctlfn")) == NULL)
	E_FATAL("-ctlfn argument missing\n");
    process_ctlfile (ctlfile);
    
    exit(0);
}
