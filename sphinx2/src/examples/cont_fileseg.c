/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * cont_fileseg.c -- Read input file, filter silence regions, and segment into utterances.
 * 
 * HISTORY
 * 
 * 27-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "s2types.h"
#include "ad.h"
#include "cont_ad.h"
#include "err.h"


static FILE *infp;	/* File being segmented */
static int32 swap;


/*
 * Need to provide cont_ad_init with a read function to read the input file.
 * This is it.  The ad_rec_t *r argument is ignored since there is no A/D
 * device involved.
 */
static int32 file_ad_read (ad_rec_t *r, int16 *buf, int32 max)
{
    int32 i, k;
    
    k = fread (buf, sizeof(int16), max, infp);
    if (swap) {
	for (i = 0; i < k; i++) {
	    buf[i] = ((buf[i] >> 8) & 0x00ff) | ((buf[i] << 8) & 0xff00);
	}
    }
    
    return ((k > 0) ? k : -1);
}


static void usagemsg (char *pgm)
{
    E_FATAL("Usage: %s <sampling-rate> <utt-end-sil(sec)> <in-file> [-byteswap] [-debug]\n", pgm);
}


/*
 * Read specified input file, segment it into utterances wherever a silence segment of
 * a given minimum duration is encountered.  Filter out long silences.
 * Utterances are written to files named 00000000.raw, 00000001.raw, 00000002.raw, etc.
 */
int
main (int32 argc, char **argv)
{
    cont_ad_t *cont;
    int32 uttid, ts, uttlen, starttime, siltime, sps, debug;
    int16 buf[4096];
    char file[1024];
    FILE *fp;
    float endsil;
    ad_rec_t ad;
    int32 i, k;
    
    if ((argc < 4) ||
	(sscanf (argv[1], "%d", &sps) != 1) ||
	(sscanf (argv[2], "%f", &endsil) != 1) || (endsil <= 0.0)) {
	usagemsg(argv[0]);
    }
    
    swap = 0;
    debug = 0;
    for (i = 4; i < argc; i++) {
	if (strcmp (argv[i], "-byteswap") == 0)
	    swap = 1;
	else if (strcmp (argv[i], "-debug") == 0)
	    debug = 1;
	else
	    usagemsg(argv[0]);
    }
    
    /* Convert desired min. inter-utterance silence duration to #samples */
    siltime = (int32) (endsil * sps);

    if ((infp = fopen (argv[3], "rb")) == NULL)
	E_FATAL("fopen(%s,rb) failed\n", argv[3]);
    
    /*
     * Associate continuous listening module with opened input file and read function.
     * No A/D device is involved, but need to fill in ad->sps.
     * Calibrate input data using first few seconds of file, but then rewind it!!
     */
    ad.sps = sps;
    ad.bps = sizeof(int16);
    cont = cont_ad_init (&ad, file_ad_read);
    printf ("Calibrating ..."); fflush (stdout);
    if (cont_ad_calib (cont) < 0)
	printf (" failed\n");
    else
	printf (" done\n");
    rewind (infp);
    
    if (debug)
	cont_ad_set_logfp(stdout);
    
    /* Read first non-silence speech */
    while ((k = cont_ad_read (cont, buf, 4096)) == 0);
    if (k < 0)
	E_FATAL("cont_ad_read failed\n");
    
    /* Start recording new utterance in next file */
    uttid = 0;
    sprintf (file, "%08d.raw", uttid);
    if ((fp = fopen(file, "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", file);
    fwrite (buf, sizeof(int16), k, fp);
    
    /* Note start timestamp at beginning of utterance */
    ts = cont->read_ts;
    starttime = ts - k;
    uttlen = k;
    
    /* Copy data for this utterance (until silence segment of desired duration) */
    for (;;) {
	/* Read more data, if any */
	if ((k = cont_ad_read (cont, buf, 4096)) < 0) {
	    /* End of file */
	    break;
	}
	
	/*
	 * Check timestamp to see if gap between new data and previously read
	 * data > desired inter-utterance silence time
	 */
	if (k > 0) {
	    if (cont->read_ts - k - ts > siltime) {
		/* Data belongs to new utterance; close previous file, start new one */
		fclose (fp);
		printf ("Utt %08d, st= %8.2fs, et= %8.2fs, seg= %7.2fs (#samp= %10d)\n",
			uttid,
			(double)starttime/(double)sps,
			(double)(starttime+uttlen)/(double)sps,
			(double)uttlen/(double)sps,
			uttlen);
		fflush (stdout);
		
		uttid++;
		sprintf (file, "%08d.raw", uttid);
		if ((fp = fopen(file, "wb")) == NULL)
		    E_FATAL("fopen(%s,wb) failed\n", file);
		
		/* Update bookkeeping for new utterance */
		starttime = cont->read_ts - k;
		uttlen = 0;
	    }
	    
	    /* Write utterance data and note timestamp for most recent data */
	    fwrite (buf, sizeof(int16), k, fp);
	    ts = cont->read_ts;
	    uttlen += k;
	}
    }
    
    fclose (fp);
    printf ("Utt %08d, st= %8.2fs, et= %8.2fs, seg= %7.2fs (#samp= %10d)\n",
	    uttid,
	    (double)starttime/(double)sps,
	    (double)(starttime+uttlen)/(double)sps,
	    (double)uttlen/(double)sps,
	    uttlen);
    fflush (stdout);

    cont_ad_close (cont);
    return 0;
}
