/*
 * misc.h -- Misc. routines (especially I/O) needed by many S3 applications.
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
 * 12-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFBS_MISC_H_
#define _LIBFBS_MISC_H_


#include <libutil/libutil.h>
#include "s3types.h"
#include "search.h"


/* Return value: control file; E_FATAL if cannot open */
FILE *ctlfile_open (char *file);

/*
 * Read next control file entry.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 ctlfile_next (FILE *fp, char *ctlspec, int32 *sf_out, int32 *ef_out, char *uttid);

void  ctlfile_close (FILE *fp);

int32 argfile_load (char *file, char *pgm, char ***argvout);

void  nbestlist_free (hyp_t **hyplist, int32 nhyp);

int32 nbestfile_load (char *dir, char *uttid, hyp_t ***hyplist_out);


#endif
