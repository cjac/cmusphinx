/*
 * allphone.h -- Exported allphone decoding functions and data structures.
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
 * $Log$
 * Revision 1.1  2004/08/09  03:19:26  arthchan2003
 * fix syntax problems in Makefile.am and add s3_allphone.h
 * 
 * Revision 1.1  2003/02/14 14:40:34  cbq
 * Compiles.  Analysis is probably hosed.
 *
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * 
 * 14-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBFBS_ALLPHONE_H_
#define _LIBFBS_ALLPHONE_H_


#include <libutil/libutil.h>
#include "s3types.h"

/* Phone level segmentation information */
typedef struct phseg_s {
    s3cipid_t ci;		/* CI-phone id */
    s3frmid_t sf, ef;		/* Start and end frame for this phone occurrence */
    int32 score;		/* Acoustic score for this segment of alignment */
    struct phseg_s *next;	/* Next entry in alignment */
} phseg_t;

int32 allphone_init ( mdef_t *mdef, tmat_t *tmat );

int32 allphone_start_utt (char *uttid);

int32 allphone_frame (int32 *senscr);

phseg_t *allphone_end_utt (char *uttid);


#endif
