/*
 * ad.c -- Wraps a "sphinx-II standard" audio interface around the basic audio
 * 		utilities.
 *
 * 
 * HISTORY
 * 
 * 11-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Modified to conform to new A/D API.
 * 
 * 12-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Dummy template created.
 */

#include <stdio.h>
#include <string.h>

#include "s3types.h"
#include "ad.h"

#define QUIT(x)		{fprintf x; exit(-1);}


ad_rec_t *ad_open_sps (int32 samples_per_sec)
{
    fprintf(stderr, "A/D library not implemented\n");
    return NULL;
}


ad_rec_t *ad_open ( void )
{
    return ad_open_sps(DEFAULT_SAMPLES_PER_SEC);
}


int32 ad_start_rec (ad_rec_t *r)
{
    return -1;
}


int32 ad_stop_rec (ad_rec_t *r)
{
    return -1;
}


int32 ad_read (ad_rec_t *r, int16 *buf, int32 max)
{
    return -1;
}


int32 ad_close (ad_rec_t *r)
{
    return 0;
}
