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
 * 3. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
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
 * adplay.c -- Playback the given input file containing raw audio samples.
 * 
 * HISTORY
 * 
 * 27-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#if (! WIN32)
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/param.h>
#else
#include <fcntl.h>
#endif

#include "ad.h"
#include "err.h"


/* Linked list of buffers of data to be played back */
typedef struct buf_s {
    int16 buf[4000];
    int32 len;
    struct buf_s *next;
} buf_t;


static void sw16 (unsigned int16 *data)
{
    *data = ((*data >> 8) & 0x00ff) | ((*data << 8) & 0xff00);
}


static void playfile (char *file,
		      int32 sps,	/* Sampling rate */
		      int32 sf, int32 ef,	/* Start/end frame (10msec/frame) */
		      int32 hdrsize,	/* #Bytes to skip at beginning of file */
		      int32 sw,		/* Byteswap */
		      int32 sc)		/* Scale to max volume */
{
    FILE *fp;
    buf_t *head = NULL, *tail, *buf;
    int32 i, k, ns, max, spf;
    int16 *bufp;
    ad_play_t *ad;
    double scale;
    
    /* Open file and read it into memory (linked list of buffers) */
    if ((fp = fopen (file, "rb")) == NULL) {
	E_ERROR("fopen(%s,rb) failed\n", file);
	return;
    }
    
    /* Samples/frame */
    spf = sps/100;	/* HACK!! Hardwired constant for frames/sec (100) */
    
    /* Skip header and upto startframe */
    k = hdrsize + (sf * spf) * sizeof(int16);
    if (k > 0)
	fseek (fp, k, SEEK_SET);

    /* Read in data to be played back */
    max = (int32)0x80000000;
    if (ef < 0)
	ns = (int32)0x7fff0000;
    else
	ns = (ef - sf + 1) * spf;
    while (ns > 0) {
	if ((buf = (buf_t *) malloc (sizeof(buf_t))) == NULL)
	    E_FATAL("malloc(%d) failed\n", sizeof(buf_t));
	buf->len = 0;
	buf->next = NULL;
	
	k = (ns < 4000) ? ns : 4000;
	if ((k = fread (buf->buf, sizeof(int16), k, fp)) <= 0)
	    break;
	buf->len = k;
	ns -= k;
	if (sw) {
	    for (i = 0; i < k; i++)
		sw16 ((unsigned int16 *) &(buf->buf[i]));
	}
	if (sc) {
	    for (i = 0; i < k; i++) {
		if (buf->buf[i] < 0) {
		    if (max < -buf->buf[i])
			max = -buf->buf[i];
		} else {
		    if (max < buf->buf[i])
			max = buf->buf[i];
		}
	    }
	}
	
	if (! head)
	    head = buf;
	else
	    tail->next = buf;
	tail = buf;
    }
    fclose (fp);

    /* Scale data if necessary */
    if (sc) {
	scale = (double)sc/(double)max;
	for (buf = head; buf; buf = buf->next) {
	    bufp = buf->buf;
	    for (i = 0; i < buf->len; i++)
		bufp[i] *= scale;
	}
    }
    
    /* Open A/D device for playback */
    if ((ad = ad_open_play_sps (sps)) == NULL)
	E_FATAL("ad_open_play_sps(%d) failed\n", sps);

    /* Playback buffers one at a time */
    if (ad_start_play (ad) < 0)
	E_FATAL("ad_start_play failed\n");

    for (buf = head; buf; buf = buf->next) {
	bufp = buf->buf;
	while (buf->len > 0) {
	    /* NOTE: ad_write is non-blocking and may accept less than given data */
	    if ((k = ad_write (ad, bufp, buf->len)) < 0)
		E_FATAL("ad_write failed\n");

	    bufp += k;
	    buf->len -= k;
	}
    }

    ad_stop_play (ad);
    ad_close_play (ad);
}


static void usagemsg (char *pgm)
{
    E_FATAL("Usage: %s <file(8/16khz,16bit)> [-r<samplingrate> -s<start-frame> -e<end-frame> -h<headersize(bytes)> -m<max-scaled-sample-value> -b(to byteswap)]\n", pgm);
}


main (int32 argc, char *argv[])
{
    int32 sw, sf, ef, hs, sc, sps;
    int32 i;
    
    if (argc < 2)
	usagemsg (argv[0]);
    
    sps = 16000;		/* Default sampling rate */
    sw = 0;		/* No byteswapping */
    sf = 0;		/* Start frame = 0 */
    ef = -1;		/* End frame = end of data */
    hs = 0;		/* Header size = 0 bytes */
    sc = 0;
    
    /* parse arguments */
    for (i = 2; i < argc; i++) {
	if (argv[i][0] != '-')
	    usagemsg(argv[0]);
	
	switch (argv[i][1]) {
	case 'r':
	    if (sscanf (&(argv[i][2]), "%d", &sps) != 1)
		usagemsg(argv[0]);
	    break;
	case 's':
	    if (sscanf (&(argv[i][2]), "%d", &sf) != 1)
		usagemsg(argv[0]);
	    break;
	case 'e':
	    if ((sscanf (&(argv[i][2]), "%d", &ef) != 1) || (ef < 0))
		usagemsg(argv[0]);
	    break;
	case 'h':
	    if (sscanf (&(argv[i][2]), "%d", &hs) != 1)
		usagemsg(argv[0]);
	    break;
	case 'm':
	    if (sscanf (&(argv[i][2]), "%d", &sc) != 1)
		usagemsg(argv[0]);
	    break;
	case 'b':
	    sw = 1;
	    break;
	default: usagemsg (argv[0]);
	    break;
	}
    }
    if (((sps != 8000) && (sps != 16000)) ||
	((ef < sf) && (ef != -1)) ||
	(sf < 0) ||
	((ef < 0) && (ef != -1)) ||
	(hs < 0) ||
	(sc < 0))
	usagemsg(argv[0]);
    
    playfile (argv[1], sps, sf, ef, hs, sw, sc);
}
