/*
 * ascr.h -- Acoustic (senone) scores
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 19-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_ASCR_H_
#define _S3_ASCR_H_


#include <libutil/libutil.h>


/*
 * Structure to hold senone scores (ordinary and composite), in one frame.
 */

typedef struct {
    int32 *sen;		/* Senone scores in current frame */
    int32 *comsen;	/* Composite senone scores in current frame */
} ascr_t;


/*
 * Create an ascr_t structure for the given number of senones (ordinary and composite).
 * Return value: Ptr to created structure if successful, NULL otherwise.
 */
ascr_t *ascr_init (int32 n_sen,		/* In: #Ordinary senones */
		   int32 n_comsen);	/* In: #Composite senones */

#endif
