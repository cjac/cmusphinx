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
 * Revision 1.1  2000/04/24  09:39:41  lenzo
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

int32 allphone_init ( void );

int32 allphone_start_utt (char *uttid);

int32 allphone_frame (int32 *senscr);

phseg_t *allphone_end_utt (char *uttid);


#endif
