/*
 * fillpen.c -- Filler penalties (penalties for words that do not show up in
 * the main LM.
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
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFBS_FILLPEN_H_
#define _LIBFBS_FILLPEN_H_


#include "dict.h"

void fillpen_init (char *file, s3wid_t start, s3wid_t end);

int32 fillpen (s3wid_t w);


#endif
