// -*- C++ -*-
//
// Copyright (c) 1995-1998 SRI International and Andreas Stolcke.
// All right reserved.
//
// Permission to use, copy, and modify this software and its documentation
// for any non-commercial purpose and without fee is hereby granted,
// provided that this entire copyright notice is included on all copies of this
// software and applications and derivations thereof.
// This software is provided on an "as is" basis, without warranty of any
// kind, either expressed or implied, as to any matter including, but not
// limited to warranty of fitness of purpose, or merchantability, or
// results obtained from use of this software.


/*      $Id: zio.h,v 1.2 2000/01/31 21:55:49 chelba Exp $        */
/*      $Log: zio.h,v $
 *      Revision 1.2  2000/01/31 21:55:49  chelba
 *      .
 *       */

#ifndef _ZIO_H
#define _ZIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include declarations files. */

#include <stdio.h>
#include <stdiostream.h>
#include <unistd.h>
/* Constants */

#define COMPRESS_SUFFIX   ".Z"
#define GZIP_SUFFIX	  ".gz"
#define OLD_GZIP_SUFFIX	  ".z"

/* Define function prototypes. */


FILE *	       zopen (const char *name, const char *mode);
int	       zclose (FILE *stream);

/* Users of this header implicitly always use zopen/zclose in stdio */

#ifdef ZIO_HACK
#define        fopen(name,mode)		zopen(name,mode)
#define        fclose(stream)		zclose(stream)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ZIO_H */

