/*
 * ascr.c -- Acoustic (senone) scores
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
 * 09-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "ascr.h"


ascr_t *ascr_init (int32 n_sen, int32 n_comsen)
{
    ascr_t *ascr;
    
    ascr = (ascr_t *) ckd_calloc (1, sizeof(ascr_t));
    ascr->sen = (int32 *) ckd_calloc (n_sen + n_comsen, sizeof(int32));
    ascr->comsen = ascr->sen + n_sen;
    
    return ascr;
}
