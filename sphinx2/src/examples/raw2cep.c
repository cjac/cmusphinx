/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * raw2cep.c -- convert raw A/D data to cepstra.
 * 
 * HISTORY
 * 
 * 18-Apr-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "s2types.h"
#include "CM_macros.h"
#include "err.h"
#include "ad.h"
#include "cont_ad.h"
#include "fe.h"


/*
 * Convert raw A/D data (16KHz, 16-bit PCM) into cepstra
 * Options:
 *     - Take A/D input from a file or the audio device (if the latter, for a given #sec)
 *     - Filter out silence regions
 *     - Break input into separate UTTERANCES when silence regions exceed a given length
 *     - For each UTTERANCE, write filtered raw data, cepstra only, or both.
 * Utterance files are named dddddddd.{raw,mfc}, where dddddddd is where (sample
 * no.) the utterance begins in the input stream.
 */


static ad_rec_t *ad = NULL;
static cont_ad_t *cont = NULL;
static FILE *adfp = NULL;

static int32 utt_break_ns;	/* #Silence samples needed to declare utterance break */
static int32 cur_ns_read;	/* #Raw (unfiltered) samples read so far */
static int32 max_ns_read;	/* Max #raw (unfiltered) samples to read */

#define ADBUFSIZE	4096


/*
 * Read A/D device or rawfile, depending on which of ad or adfp is non-NULL
 */
static int32 adread_raw (ad_rec_t *dummy, int16 *buf, int32 len)
{
    int32 k;
    
    if (ad) {
	assert (! adfp);
	
	k = ad_read (ad, buf, len);
	return k;
    } else {
	assert (adfp);
	
	k = fread (buf, sizeof(int16), len, adfp);
	return ((k <= 0) ? -1 : k);
    }
}


/*
 * Pass result of adread_raw through silence filter, if specified.  Return only when
 * some non-zero amount of data is available, or utterance break detected, or end of
 * raw A/D data.
 */
static int32 adread_filtered (int16 *buf, int32 len)
{
    int32 k;

    if (cur_ns_read >= max_ns_read)
	return -1;
    
    if (! cont) {
	for (;;) {
	    k = adread_raw (NULL, buf, len);
	    if (k < 0)
		return -1;
	    
	    if (k > 0) {
		cur_ns_read += k;
		return k;
	    }
	}
    } else {
	for (;;) {
	    k = cont_ad_read (cont, buf, len);
	    if (k < 0)
		return -1;
	    
	    if (k == 0) {
		if ((utt_break_ns > 0) && ((cont->read_ts - cur_ns_read) > utt_break_ns)) {
		    cur_ns_read = cont->read_ts;
		    return 0;
		}
	    } else {
		cur_ns_read = cont->read_ts;
		return k;
	    }
	}
    }
}


static void cleanup_and_exit ( void )
{
    if (ad)
	ad_stop_rec (ad);
    
    if (cont)
	cont_ad_close (cont);
    
    if (ad)
	ad_close (ad);
    
    if (adfp)
	fclose (adfp);

    exit(0);
}


static void usage (char *pgm)
{
    printf ("Usage: %s \\\n", pgm);
    printf ("\t-sps <samples/sec> \\\n");
    printf ("\t[-removesil] \\\n");
    printf ("\t[-uttbreak <#sec>] (implies -removesil) \\\n");
    printf ("\t[-outraw] \\\n");
    printf ("\t[-outmfc] (at least one of -outraw and -outmfc must be specified) \\\n");
    printf ("\t{-inraw <file> | -ad <#sec>}\n");

    cleanup_and_exit();
}


int
main (int32 argc, char **argv)
{
    FILE *adout, *mfcout;
    int32 i, k, ns, nc, n_utt, tot_ns, write_raw, write_mfc, removesil, sps;
    int16 adbuf[ADBUFSIZE];
    float **mfcbuf = NULL;
    fe_t *fe = NULL;
    char line[1024];
    char filename[4096], uttid[256], *rawfile;
    
    cur_ns_read = 0;
    max_ns_read = (int32) 0x7fffff00;
    utt_break_ns = -1;	/* Default: Do not break-up utterances */
    removesil = 0;
    write_raw = 0;
    write_mfc = 0;
    sps = -1;
    rawfile = NULL;
    
    for (i = 1; i < argc; i++) {
	if (strcmp (argv[i], "-ad") == 0) {
	    if (sps < 0) {
		E_ERROR("Specify -sps argument first\n");
		usage (argv[0]);
	    }
	    
	    if (adfp || ad)
		usage (argv[0]);

	    if ((i == argc-1) || (sscanf (argv[i+1], "%d", &k) != 1))
		usage (argv[0]);
	    if (k > 130000) {
		E_ERROR("-ad argument (#sec) must be < 130000\n");
		cleanup_and_exit ();
	    }
	    max_ns_read = k * sps;
	    
	    if ((ad = ad_open_sps (sps)) == NULL) {
		E_ERROR("ad_open failed\n");
		cleanup_and_exit ();
	    }
	    
	    i++;
	} else if (strcmp (argv[i], "-sps") == 0) {
	    if (sps >= 0)
		usage (argv[0]);
	    
	    if ((i == argc-1) || (sscanf (argv[i+1], "%d", &sps) != 1))
		usage (argv[0]);
	    if ((sps != 8000) && (sps != 16000))
		E_FATAL("Samples/sec can only be 8000 or 16000\n");
	    
	    i++;
	} else if (strcmp (argv[i], "-inraw") == 0) {
	    if (adfp || ad)
		usage (argv[0]);
	    
	    if (i == argc-1)
		usage (argv[0]);
	    
	    rawfile = argv[i+1];
	    if ((adfp = fopen(rawfile, "rb")) == NULL) {
		E_ERROR("fopen(%s,rb) failed\n", rawfile);
		cleanup_and_exit ();
	    }
	    
	    i++;
	} else if (strcmp (argv[i], "-removesil") == 0) {
	    removesil = 1;
	} else if (strcmp (argv[i], "-uttbreak") == 0) {
	    if (sps < 0) {
		E_ERROR("Specify -sps argument first\n");
		usage (argv[0]);
	    }
	    removesil = 1;

	    if ((i == argc-1) || (sscanf (argv[i+1], "%d", &k) != 1) || (k <= 0))
		usage (argv[0]);
	    if (k > 130000) {
		E_ERROR("-uttbreak argument (#sec) must be < 130000\n");
		cleanup_and_exit ();
	    }
	    utt_break_ns = k * sps;
	    
	    i++;
	} else if (strcmp (argv[i], "-outraw") == 0) {
	    write_raw = 1;
	} else if (strcmp (argv[i], "-outmfc") == 0) {
	    write_mfc = 1;
	} else {
	    usage (argv[0]);
	}
    }

    if ((! ad) && (! adfp))
	usage (argv[0]);
    if ((! write_raw) && (! write_mfc))
	usage (argv[0]);

    if (removesil) {
	if ((cont = cont_ad_init (NULL, adread_raw)) == NULL) {
	    E_ERROR("cont_ad_init failed\n");
	    cleanup_and_exit();
	}
	
	/* Calibrate continuous listening for background noise/silence level */
	if (ad) {
	    printf ("Hit <CR> when ready to calibrate continuous listening module: ");
	    fflush (stdout);
	    fgets (line, sizeof(line), stdin);
	    
	    ad_start_rec (ad);
	    if (cont_ad_calib (cont) < 0) {
		E_ERROR ("cont_ad_calib failed\n");
		cleanup_and_exit();
	    }
	    ad_stop_rec (ad);

	    cont_ad_reset (cont);
	} else {
	    assert (adfp);
	    
	    if (cont_ad_calib (cont) < 0) {
		E_ERROR ("cont_ad_calib failed\n");
		cleanup_and_exit();
	    }

	    rewind (adfp);
	}
    }
    
    if (write_mfc) {
	param_t param;
	float *mfcp;

	mfcp = (float *) CM_calloc (4096 * DEFAULT_NUM_CEPSTRA, sizeof(float));
	mfcbuf = (float **) CM_calloc (4096, sizeof(float *));

	for (i = 0; i < 4096; i++) {
	    mfcbuf[i] = mfcp;
	    mfcp += DEFAULT_NUM_CEPSTRA;
	}

	memset(&param, 0, sizeof(param));
	param.SAMPLING_RATE = sps;
	if ((fe = fe_init (&param)) == NULL)
	    E_FATAL("fe_init(%d) failed\n", sps);
    }
    
    adout = mfcout = NULL;
    ns = tot_ns = nc = n_utt = 0;

    if (ad) {
	printf ("Hit <CR> when ready to start recording: ");
	fflush (stdout);
	fgets (line, sizeof(line), stdin);

	ad_start_rec (ad);
    }
    
    for (;;) {
	k = adread_filtered (adbuf, ADBUFSIZE);
	
	if (k <= 0) {
	    /* Utterance break */
	    if (adout)
		fclose (adout);

	    if (mfcout) {
		int slop;

		slop = fe_end_utt(fe, mfcbuf[0]);
		if (slop)
		    fwrite (mfcbuf[0], sizeof(float), 13, mfcout);

		fflush (mfcout);
		fseek (mfcout, 0, SEEK_SET);
		k = nc * 13;
		fwrite (&k, sizeof(int32), 1, mfcout);
		
		fclose (mfcout);

		E_INFO("%s: %d samples, %d cepstrum frames\n", uttid, ns, nc);
	    }
	    
	    adout = mfcout = NULL;
	    ns = nc = 0;
	    
	    if (k < 0)	/* EOF */
		break;
	} else {
	    if ((write_raw && (! adout)) || (write_mfc && (! mfcout))) {
		assert ((! mfcout) && (! adout));	/* Start of new utterance */
		
		n_utt++;
		if ((! rawfile) || write_raw || (utt_break_ns >= 0))
		    sprintf (uttid, "%08d", (cur_ns_read - k)/(sps/100));
		else {
		    /* Convert raw filename to uttid */
		    for (i = strlen(rawfile)-1;
			 (i >= 0) && (rawfile[i] != '/') && (rawfile[i] != '\\');
			 i--);
		    strcpy (uttid, rawfile+i+1);
		    for (i = strlen(uttid)-1; (i >= 0) && (uttid[i] != '.'); --i);
		    if (i >= 0)
			uttid[i] = '\0';
		}
		
		if (write_raw) {
		    sprintf (filename, "%s.raw", uttid);
		    adout = CM_fopen(filename, "wb");
		    ns = 0;
		}
		if (write_mfc) {
		    sprintf (filename, "%s.mfc", uttid);
		    mfcout = CM_fopen(filename, "wb");
		    nc = 0;
		    fwrite (&nc, sizeof(int32), 1, mfcout);	/* header placeholder */

		    fe_start_utt (fe);
		}
	    }
	    
	    if (adout) {
		fwrite (adbuf, sizeof(int16), k, adout);
		fflush (adout);
	    }
	    ns += k;
	    tot_ns += k;
	    
	    if (mfcout) {
		
		k = fe_process_utt (fe, adbuf, k, mfcbuf);
		nc += k;
		
		for (i = 0; i < k; i++)
		    fwrite (mfcbuf[i], sizeof(float), 13, mfcout);
		fflush (mfcout);
	    }
	}
    }

    E_INFO("%d utterances, %.2f sec read, %.2f sec written\n", n_utt,
	   (double) cur_ns_read / (double) sps, (double) tot_ns / (double) sps);

    cleanup_and_exit();
    return 0;
}
