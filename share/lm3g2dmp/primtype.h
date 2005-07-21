/*
 * primtype.h -- Primitive types; more machine-independent.
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
 * 12-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Borrowed from Sphinx-3.
 */


#ifndef _LIBUTIL_PRIMTYPE_H_
#define _LIBUTIL_PRIMTYPE_H_


typedef int		int32;
typedef short		int16;
typedef unsigned short	u_int16;
typedef unsigned int	u_int32;
typedef float		float32;
typedef double		float64;

#if defined(WIN32) && !defined(__CYGWIN__)
typedef char *  caddr_t;
#endif

#endif
