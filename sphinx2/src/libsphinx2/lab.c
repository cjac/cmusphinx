/*
 * lab.c -- File I/O for label files
 *
 * HISTORY
 * 
 * 23-Oct-01    Kevin A. Lenzo (lenzo@cs.cmu.edu) Fixed the
 *              magic numbers in save_labs, but it's still hard-coded.
 *              This needs to be calculated from the sample rate and shift.
 *
 * 28-Mar-00	Alan W Black (awb@cs.cmu.edu) Created.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "s2types.h"
#include "CM_macros.h"
#include "basic_types.h"
#include "search_const.h"
#include "err.h"
#include "log.h"
#include "scvq.h"
#include "msd.h"
#include "fbs.h"

#include "time_align.h"
#include "s2params.h"

/* FIXME: prototype this in an internal header file somewhere. */
int save_labs(SEGMENT_T *segs,
	      int num_entries,
	      const char *dirname,
	      const char *filename,
	      const char *extname,
	      const char *labtype)
{
    int i;
    FILE *labfd;
    char *path;

    path=(char *)malloc(strlen(dirname)+strlen(filename)+strlen(extname)+4);
    sprintf(path,"%s/%s.%s",dirname,filename,extname);

    if ((labfd = fopen(path,"w")) == NULL)
    {
	fprintf(stderr, "%s(%d): failed to open label file: %s\n",
		__FILE__, __LINE__, path);
	free(path);
	exit(1);
    }

    if (strcmp(labtype,"xlabel") == 0)
    {
	fprintf(labfd,"#\n");
	for (i=0; i<num_entries; i++)
	{
	  fprintf(labfd,"%0.6f 125 %s ; %d\n", 
		  /*		    segs[i].end * 0.00625,  */
		  segs[i].end * 0.01,
		  segs[i].name,
		  segs[i].score);
	}
    }
/*    else if (strcmp(labtype,"something else") == 0) */
/*    {                                               */
/*    }                                               */
    else  
    {   /* some CMU internal format -- does any one use this */
	printf("%20s %4s %4s %s\n",
	       "Phone", "Beg", "End", "Acoustic Score");
	for (i=0; i<num_entries; i++)
	{
	    fprintf(labfd,"%20s %4d %4d %12d\n",
		    segs[i].name, segs[i].start, segs[i].end, segs[i].score);
	}
    }
    free(path);
    fclose(labfd);
    return 0;
}
