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
 * adpow.c -- Compute the input A/D signal power.
 * 
 * HISTORY
 *
 * 02-Aug-99    Kevin Lenzo (lenzo@cs.cmu.edu) at Carnegie Mellon
 *              changed ad_open to ad_open_sps and set sps to the DEFAULT.
 * 
 * 27-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "s2types.h"
#include "ad.h"
#include "err.h"

/*
 * Compute power in input signal for about 2 sec
 */
static void adpow (ad_rec_t *ad)
{
    double en, p;
    int32 i, k, prev, len, s;
    int16 buf[1000];
    
    en = 0.0;
    prev = 0;

    for (len = 0; len < 32000; ) {	/* len = #samples gathered */
	if ((k = ad_read (ad, buf, 1000)) < 0)
	    E_FATAL("ad_read returned %d\n", k);
	
	for (i = 0; i < k; i++) {
	    s = buf[i] - prev;		/* Pre-emphasis filter */
	    en += ((double)s)*((double)s);
	    prev = buf[i];
	}
	
	len += k;
    }
    en /= len;
    p = sqrt (en);
    
    if (p < 1.0)
	p = 1.0;
    printf("log(Power) = %.2f dB\n", 10.0 * log10(p)); 
    fflush(stdout);
    /*     E_INFO("log(Power) = %.2f dB\n", 10.0 * log10(p)); */
}

/*
 * Wait for the user to type a <CR> then capture input audio for approx. 2 sec
 * and compute a measure of its power.
 */
int
main (int32 argc, char *argv[])
{
    char line[1024];
    ad_rec_t *ad;
    int16 buf[1000];
    int32 sps;
    
    sps = DEFAULT_SAMPLES_PER_SEC;

    if ((ad = ad_open_sps (sps)) == NULL)
	E_FATAL("ad_open_sps failed\n");
    
    for (;;) {
	printf ("Hit <CR> to measure A/D power; Q<CR> to quit: ");
	
	fgets (line, sizeof(line), stdin);
	if ((line[0] == 'q') || (line[0] == 'Q'))
	    break;
	
	ad_start_rec (ad);

	adpow(ad);
	
	ad_stop_rec (ad);
	while (ad_read (ad, buf, 1000) >= 0);	/* Flush any buffered, unread data */
    }

    ad_close (ad);
    return 0;
}

