/*
 * bitvec.c -- Bit vector type.
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
 * 05-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */


#include "bitvec.h"


int32 bitvec_count_set (bitvec_t vec, int32 len)
{
    int32 n, i;
    
    for (i = 0, n = 0; i < len; i++)
	if (bitvec_is_set (vec, i))
	    n++;
    
    return n;
}
