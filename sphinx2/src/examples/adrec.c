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
 * adrec.c -- Record A/D data into a file.
 * 
 * HISTORY
 * 
 * 27-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>

#include "s2types.h"
#include "ad.h"
#include "err.h"


/*
 * Record A/D data for approximately specified number of seconds into specified file.
 */
int
main (int32 argc, char *argv[])
{
    char line[1024];
    ad_rec_t *ad;
    int16 buf[1000];
    int32 len, k, sps;
    FILE *fp;
    
    if ((argc != 4) ||
	(sscanf (argv[1], "%d", &sps) != 1) ||
	(sscanf (argv[2], "%d", &len) != 1)) {
	E_FATAL("Usage: %s <sampling-rate> <#sec-to-record> <output-file>\n", argv[0]);
    }
    if ((fp = fopen (argv[3], "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", argv[3]);
    
    len *= sps;		/* Convert to min. #samples to record */
    
    if ((ad = ad_open_sps (sps)) == NULL)
	E_FATAL("ad_open_sps(%d) failed\n", sps);
    
    printf ("Hit <CR> to start recording\n");
    fgets (line, sizeof(line), stdin);
    
    ad_start_rec (ad);
    
    /* Record desired no. of samples */
    while (len > 0) {
	/* Read A/D */
	if ((k = ad_read (ad, buf, 1000)) < 0)
	    E_FATAL("ad_read returned %d\n", k);

	/* Write data read, if any, to file (ad_read may return 0 samples) */
	if (k > 0) {
	    fwrite (buf, sizeof(int16), k, fp);
	    fflush (fp);
	    len -= k;
	}
    }
    
    ad_stop_rec (ad);
    ad_close (ad);

    fclose (fp);
    return 0;
}
