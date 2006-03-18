/* ====================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All rights
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
 * ad_esd.c -- Wraps a "sphinx-II standard" audio interface around the basic audio
 * 		utilities.
 *
 * HISTORY
 * 
 * 18-Mar-2006 David Huggins-Daines <dhuggins@cs.cmu.edu>
 *	       Created this.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <esd.h>

#include "s3types.h"
#include "ad.h"

#define QUIT(x)		{fprintf x; exit(-1);}


ad_rec_t *ad_open_sps (int32 sps)
{
	ad_rec_t *ad;
	int fd;

	if ((fd = esd_record_stream_fallback(ESD_MONO | ESD_BITS16,
					     sps, NULL, NULL)) < 0) {
		/* FIXME: We'd like a better error message, probably. */
		fprintf(stderr, "Failed to open ESD record stream.\n");
		return NULL;
	}

	if ((ad = calloc(1, sizeof(ad_rec_t))) == NULL) {
		fprintf(stderr, "calloc(%ld) failed\n", sizeof(ad_rec_t));
		abort();
	}
	ad->fd = fd;
	ad->recording = 0;
	ad->sps = sps;
	ad->bps = sizeof(int16);
	return ad;
}


ad_rec_t *ad_open ( void )
{
	return ad_open_sps(DEFAULT_SAMPLES_PER_SEC);
}


int32 ad_start_rec (ad_rec_t *r)
{
	return 0;
}


int32 ad_stop_rec (ad_rec_t *r)
{
	return 0;
}


int32 ad_read (ad_rec_t *r, int16 *buf, int32 max)
{
	int32 length;

	length = max * r->bps;
	if ((length = read(r->fd, buf, length)) > 0) {
		length /= r->bps;
	}
	if (length < 0) {
		if (errno!=EAGAIN){ 
			fprintf(stderr, "Audio read error: %s\n",
				strerror(errno));
			return AD_ERR_GEN; 
		} else {
			length=0; 
		}
	}
    
	if ((length == 0) && (! r->recording))
		return AD_EOF;

	return length;
}


int32 ad_close (ad_rec_t *r)
{
	esd_close(r->fd);
	free(r);
	return 0;
}
