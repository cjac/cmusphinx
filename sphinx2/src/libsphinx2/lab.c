/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * lab.c -- File I/O for label files
 *
 * HISTORY
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
		    segs[i].end*0.00625,  /* need to check this - awb */
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
