/* ====================================================================
 * Copyright (c) 1993-2000 Carnegie Mellon University.  All rights 
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
 * ad.c -- Wraps a "sphinx-II standard" audio interface around the basic audio
 * 		utilities.
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
 * 11-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Modified to new A/D API.
 * 
 * 22-Apr-94	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Adapted to latest ad.h interface.
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "audio_utils.h"
#include "s2types.h"
#include "ad.h"


#define QUIT(x)		{fprintf x; exit(-1);}


ad_rec_t *ad_open ( void )
{
    ad_rec_t *r;
    
    if ((r = (ad_rec_t *) calloc (1, sizeof(ad_rec_t))) == NULL)
	return NULL;
    
    r->audio_fd = audioOpen (16000);
    r->recording = 0;
    
    return r;
}


int32 ad_start_rec (ad_rec_t *r)
{
    if (r->recording)
	return -1;
    
    audioStartRecord ();
    r->recording = 1;

    return 0;
}


int32 ad_stop_rec (ad_rec_t *r)
{
    if (! r->recording)
	return -1;
    
    audioStopRecord ();
    r->recording = 0;

    return 0;
}


int32 ad_read (ad_rec_t *r, int16 *buf, int32 max)
{
    int32 len;
    
    /* Get whatever samples are available, upto max requested size */
    len = max*AD_SAMPLE_SIZE;
    len = read (r->audio_fd, buf, len);

    if (len > 0) {
	/* HACK!!  Assume read returns complete samples, but check the assumption */
	if (len & 0x1)
	    QUIT((stderr, "%s(%d): **ERROR**, ad_read() returned odd #bytes\n",
		  __FILE__, __LINE__));
	
	return (len >> 1);
    }
    
    if ((len == 0) && (! r->recording))
	return -1;	/* EOF */

    return 0;
}


int32 ad_close (ad_rec_t *r)
{
    if (r->recording)
	ad_stop_rec (r);
    
    audioClose ();
    free (r);
    
    return 0;
}
