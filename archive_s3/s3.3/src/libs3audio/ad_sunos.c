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
#include "s3types.h"
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
