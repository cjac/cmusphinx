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
