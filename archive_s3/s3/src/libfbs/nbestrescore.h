/*
 * nbestrescore.h -- Alpha (forward algorithm) rescoring of N-best lists.
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
 * 		Started.
 */


#ifndef _LIBFBS_NBESTRESCORE_H_
#define _LIBFBS_NBESTRESCORE_H_


#include <libutil/libutil.h>
#include <search.h>


int32 alpha_start_utt (hyp_t **hyplist, int32 nhyp, char *uttid);

int32 alpha_sen_active (s3senid_t *sen_active, int32 nsen);

int32 alpha_frame (char *uttid, int32 *senscr);

int32 alpha_end_utt (char *outdir, char *uttid);

int32 alpha_init ( void );


#endif
