/* ====================================================================
 * Copyright (c) 1989-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

/*
	30 May 1989 David R. Fulmer (drf) updated to do byte order
		conversions when necessary.
 */

#include <stdio.h>
#include <stdlib.h>
#if (! WIN32)
#include <sys/file.h>
#include <sys/fcntl.h>
#include <unistd.h>
#else
#include <fcntl.h>
#endif
#include "byteorder.h"


int
f2read (char *file, float **data1_ref, float **data2_ref, int *length_ref)
{
  int             fd;
  int             length;
  int             size;
  int             offset;
  char           *data1, *data2;

  if ((fd = open (file, O_RDONLY, 0644)) < 0)
  {
    fprintf (stderr, "f2read: %s: can't open\n", file);
    return -1;
  }

  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "f2read: %s: can't read length (empty file?)\n", file);
    return (-1);
  }
  SWAPL(&length);
  size = length * sizeof (float);

  if (!(data1 = malloc ((unsigned) size)))
  {
    fprintf (stderr, "f2read: %s: can't alloc data1\n", file);
    close (fd);
    return -1;
  }

  if (read (fd, data1, size) != size)
  {
    fprintf (stderr, "f2read: %s: can't read data1\n", file);
    close (fd);
    free (data1);
    return -1;
  }

  if (!(data2 = malloc ((unsigned) size)))
  {
    fprintf (stderr, "f2read: %s: can't alloc data2\n", file);
    close (fd);
    free (data1);
    return -1;
  }

  if (read (fd, data2, size) != size)
  {
    fprintf (stderr, "f2read: %s: can't read data2\n", file);
    close (fd);
    free (data1);
    free (data2);
    return -1;
  }

  close (fd);
  *data1_ref = (float *) data1;
  *data2_ref = (float *) data2;
  for(offset = 0; offset < length; offset++) {
    SWAPF(*data1_ref + offset);
    SWAPF(*data2_ref + offset);
  }
  *length_ref = length;
  return length;
}
