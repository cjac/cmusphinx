/*
 * vector.c
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
 * 12-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Copied from Eric Thayer.
 */


#include "vector.h"

#include <s3.h>
#include <libutil/prim_type.h>


int32 vector_normalize (float32 *vec, int32 len)
{
    int32 i;
    float32 sum, invsum;

    sum = (float32) 0.0;
    for (i = 0; i < len; i++)
	sum += vec[i];

    if (sum <= 0.0)
	return S3_ERROR;

    invsum = (float32) (1.0/sum);

    for (i = 0; i < len; i++)
	vec[i] *= invsum;

    return S3_SUCCESS;
}


void vector_floor (float32 *vec, int32 len, float32 flr)
{
    int32 i;

    for (i = 0; i < len; i++)
	if (vec[i] < flr)
	    vec[i] = flr;
}


void vector_nz_floor (float32 *vec, int32 len, float32 flr)
{
    int32 i;

    for (i = 0; i < len; i++)
	if ((vec[i] != 0.0) && (vec[i] < flr))
	    vec[i] = flr;
}
