/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * cont_fileseg.c -- Read input file, filter silence regions, and segment into utterances.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.8  2005/02/01  22:21:13  rkm
 * Added raw data logging, and raw data pass-through mode to cont_ad
 * 
 * Revision 1.7  2004/07/16 00:57:11  egouvea
 * Added Ravi's implementation of FSG support.
 *
 * Revision 1.3  2004/06/25 14:58:05  rkm
 * *** empty log message ***
 *
 * Revision 1.2  2004/06/23 20:32:08  rkm
 * Exposed several cont_ad config parameters
 *
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

/* Max size read by file_ad_read function on each invocation, for debugging */
static int32 max_ad_read_size;


/*
 * Need to provide cont_ad_init with a read function to read the input file.
 * This is it.  The ad_rec_t *r argument is ignored since there is no A/D
 * device involved.
 */
static int32 file_ad_read (ad_rec_t *r, int16 *buf, int32 max)
{
    int32 i, k;
    
    if (max > max_ad_read_size)
      max = max_ad_read_size;
    
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
    E_INFO("Usage: %s \\\n", pgm);
    E_INFOCONT("\t[-? | -h] \\\n");
    E_INFOCONT("\t[-d | -debug] \\\n");
    E_INFOCONT("\t[-sps <sampling-rate> (16000)] \\\n");
    E_INFOCONT("\t[-b | -byteswap] \\\n");
    E_INFOCONT("\t[{-s | -silsep} <length-silence-separator(sec) (0.5)]> \\\n");
    E_INFOCONT("\t[-w | -writeseg] \\\n");
    E_INFOCONT("\t[-delta-sil <delta-sil>] \\\n");
    E_INFOCONT("\t[-delta-speech <delta-speech>] \\\n");
    E_INFOCONT("\t[-sil-onset <sil-onset>] \\\n");
    E_INFOCONT("\t[-speech-onset <speech-onset>] \\\n");
    E_INFOCONT("\t[-adapt-rate <adapt-rate>] \\\n");
    E_INFOCONT("\t[-max-adreadsize <ad_read_blksize>] \\\n");
    E_INFOCONT("\t[-c <copy-input-file>] \\\n");
    E_INFOCONT("\t[-r | -rawmode] \\\n");
    E_INFOCONT("\t-i <input-file>\n");

    exit(0);
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
    int32 uttid, ts, uttlen, starttime, siltime, sps, debug, writeseg, rawmode;
    int16 buf[4096];
    char *infile, *copyfile, segfile[1024];
    FILE *fp;
    float endsil;
    ad_rec_t ad;
    int32 i, k;
    int32 min_noise, max_noise, winsize, leader, trailer;
    int32 orig_delta_sil, orig_delta_speech, orig_speech_onset, orig_sil_onset;
    int32 delta_sil, delta_speech, sil_onset, speech_onset;
    float32 orig_adapt_rate;
    float32 adapt_rate;
    int32 total_speech_samples;
    float32 total_speech_sec;
    FILE *rawfp;
    
    /* Set argument defaults */
    cont = NULL;
    sps = 16000;
    swap = 0;
    endsil = 0.5;
    writeseg = 0;
    delta_sil = delta_speech = sil_onset = speech_onset = -1;
    adapt_rate = -1.0;
    max_ad_read_size = (int32) 0x7ffffff0;
    debug = 0;
    infile = NULL;
    copyfile = NULL;
    rawfp = NULL;
    rawmode = 0;
    
    /* Parse arguments */
    for (i = 1; i < argc; i++) {
      if ((strcmp (argv[i], "-help") == 0)
	  || (strcmp (argv[i], "-h") == 0)
	  || (strcmp (argv[i], "-?") == 0)) {
	usagemsg(argv[0]);
      } else if ((strcmp (argv[i], "-debug") == 0)
		 || (strcmp (argv[i], "-d") == 0)) {
	debug = 1;
      } else if (strcmp (argv[i], "-sps") == 0) {
	i++;
	if ((i == argc)
	    || (sscanf (argv[i], "%d", &sps) != 1)
	    || (sps <= 0)) {
	  E_ERROR("Invalid -sps argument\n");
	  usagemsg(argv[0]);
	}
      } else if ((strcmp (argv[i], "-byteswap") == 0)
		 || (strcmp (argv[i], "-b") == 0)) {
	swap = 1;
      } else if ((strcmp (argv[i], "-silsep") == 0)
		 || (strcmp (argv[i], "-s") == 0)) {
	i++;
	if ((i == argc)
	    || (sscanf (argv[i], "%f", &endsil) != 1)
	    || (endsil <= 0.0)) {
	  E_ERROR("Invalid -silsep argument\n");
	  usagemsg(argv[0]);
	}
      } else if ((strcmp (argv[i], "-writeseg") == 0)
		 || (strcmp (argv[i], "-w") == 0)) {
	writeseg = 1;
      } else if (strcmp (argv[i], "-delta-sil") == 0) {
	i++;
	if ((i == argc) ||
	    (sscanf (argv[i], "%d", &delta_sil) != 1) ||
	    (delta_sil < 0)) {
	  E_ERROR("Invalid -delta-sil argument\n");
	  usagemsg(argv[0]);
	}
      } else if (strcmp (argv[i], "-delta-speech") == 0) {
	i++;
	if ((i == argc) ||
	    (sscanf (argv[i], "%d", &delta_speech) != 1) ||
	    (delta_speech < 0)) {
	  E_ERROR("Invalid -delta-speech argument\n");
	  usagemsg(argv[0]);
	}
      } else if (strcmp (argv[i], "-sil-onset") == 0) {
	i++;
	if ((i == argc) ||
	    (sscanf (argv[i], "%d", &sil_onset) != 1) ||
	    (sil_onset < 1)) {
	  E_ERROR("Invalid -sil-onset argument\n");
	  usagemsg(argv[0]);
	}
      } else if (strcmp (argv[i], "-speech-onset") == 0) {
	i++;
	if ((i == argc) ||
	    (sscanf (argv[i], "%d", &speech_onset) != 1) ||
	    (speech_onset < 1)) {
	  E_ERROR("Invalid -speech-onset argument\n");
	  usagemsg(argv[0]);
	}
      } else if (strcmp (argv[i], "-adapt-rate") == 0) {
	i++;
	if ((i == argc) ||
	    (sscanf (argv[i], "%f", &adapt_rate) != 1) ||
	    (adapt_rate < 0.0) || (adapt_rate > 1.0)) {
	  E_ERROR("Invalid -adapt-rate argument\n");
	  usagemsg(argv[0]);
	}
      } else if (strcmp (argv[i], "-max-adreadsize") == 0) {
	i++;
	if ((i == argc) ||
	    (sscanf (argv[i], "%d", &max_ad_read_size) != 1) ||
	    (max_ad_read_size < 1)) {
	  E_ERROR("Invalid -max-adreadsize argument\n");
	  usagemsg(argv[0]);
	}
      } else if (strcmp (argv[i], "-c") == 0) {
	i++;
	if (i == argc) {
	  E_ERROR("Invalid -c argument\n");
	  usagemsg(argv[0]);
	}
	copyfile = argv[i];
      } else if ((strcmp (argv[i], "-rawmode") == 0)
		 || (strcmp (argv[i], "-r") == 0)) {
	rawmode = 1;
      } else if (strcmp (argv[i], "-i") == 0) {
	i++;
	if (i == argc) {
	  E_ERROR("Invalid -i argument\n");
	  usagemsg(argv[0]);
	}
	infile = argv[i];
      } else {
	usagemsg(argv[0]);
      }
    }
    
    if (infile == NULL) {
      E_ERROR("No input file specified\n");
      usagemsg(argv[0]);
    }
    
    if ((infp = fopen (infile, "rb")) == NULL)
	E_FATAL("fopen(%s,rb) failed\n", infile);
    
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
	printf (" failed; file too short?\n");
    else
	printf (" done\n");
    rewind (infp);
    
    if (rawmode)
      cont_ad_set_rawmode (cont, 1);
    
    /* Convert desired min. inter-utterance silence duration to #samples */
    siltime = (int32) (endsil * sps);
    
    /* Enable writing raw input to output by the cont module if specified */
    if (copyfile) {
      if ((rawfp = fopen(copyfile, "wb")) == NULL)
	E_ERROR("fopen(%s,wb) failed; not dumping raw file\n", copyfile);
      else
	cont_ad_set_rawfp (cont, rawfp);
    }
    
    cont_ad_get_params(cont,
		       &orig_delta_sil, &orig_delta_speech,
		       &min_noise, &max_noise,
		       &winsize,
		       &orig_speech_onset, &orig_sil_onset,
		       &leader, &trailer,
		       &orig_adapt_rate);
    
    E_INFO("Standard configuration: delta-sil = %d, delta-speech = %d, sil-onset = %d, speech-onset = %d, adapt_rate = %.3f\n",
	   orig_delta_sil, orig_delta_speech,
	   orig_sil_onset, orig_speech_onset,
	   orig_adapt_rate);
    
    if (delta_sil < 0)
      delta_sil = orig_delta_sil;
    if (delta_speech < 0)
      delta_speech = orig_delta_speech;
    if (sil_onset < 0)
      sil_onset = orig_sil_onset;
    if (speech_onset < 0)
      speech_onset = orig_speech_onset;
    if (adapt_rate < 0.0)
      adapt_rate = orig_adapt_rate;
    
    cont_ad_set_params(cont,
		       delta_sil, delta_speech,
		       min_noise, max_noise,
		       winsize,
		       speech_onset, sil_onset,
		       leader, trailer,
		       adapt_rate);
    E_INFO("Current configuration:  delta-sil = %d, delta-speech = %d, sil-onset = %d, speech-onset = %d, adapt_rate = %.3f\n",
	   delta_sil, delta_speech, sil_onset, speech_onset, adapt_rate);
    E_INFO("Sampling rate: %d\n", sps);
    E_INFO("Byteswap: %s\n", swap ? "Yes" : "No");
    E_INFO("Max ad-read size: %d\n", max_ad_read_size);
    
    if (debug)
	cont_ad_set_logfp(cont, stdout);
    
    /* Read first non-silence speech */
    while ((k = cont_ad_read (cont, buf, 4096)) == 0);
    if (k < 0)
	E_FATAL("cont_ad_read failed\n");
    
    /*
     * Start recording new utterance in next file.
     * Hack!!  Using /dev/null to avoid writing.
     */
    uttid = 0;
    if (writeseg)
      sprintf (segfile, "%08d.raw", uttid);
    else
      strcpy (segfile, "/dev/null");
    if ((fp = fopen(segfile, "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", segfile);
    fwrite (buf, sizeof(int16), k, fp);
    
    /* Note start timestamp at beginning of utterance */
    ts = cont->read_ts;
    starttime = ts - k;
    uttlen = k;
    total_speech_samples = 0;
    total_speech_sec = 0.0;
    
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
		total_speech_samples += uttlen;
		total_speech_sec += (double)uttlen/(double)sps;
		
		uttid++;
		if (writeseg)
		  sprintf (segfile, "%08d.raw", uttid);
		else
		  strcpy (segfile, "/dev/null");
		if ((fp = fopen(segfile, "wb")) == NULL)
		    E_FATAL("fopen(%s,wb) failed\n", segfile);
		
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
    total_speech_samples += uttlen;
    total_speech_sec += (double)uttlen/(double)sps;

    if (rawfp)
      fclose(rawfp);
    
    cont_ad_close (cont);
    
    E_INFO("Total speech detected = %d samples, %.2f sec\n",
	   total_speech_samples, total_speech_sec);
    
    return 0;
}
