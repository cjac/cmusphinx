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
f2write (char *file, float *data1, float *data2, int length)
{
  int             fd;
  int             size;
  int             offset;

  if ((fd = open (file, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
  {
    fprintf (stderr, "f2write: %s: can't create\n", file);
    return -1;
  }

  SWAPL(&length);
  if (write (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "f2write: %s: can't write length\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&length);

  for(offset = 0; offset < length; offset++) {
    SWAPF(data1 + offset);
    SWAPF(data2 + offset);
  }

  size = length * sizeof (float);

  if (write (fd, (char *) data1, size) != size)
  {
    fprintf (stderr, "f2write: %s: can't write data1\n", file);
    close (fd);
    return -1;
  }

  if (write (fd, (char *) data2, size) != size)
  {
    fprintf (stderr, "f2write: %s: can't write data2\n", file);
    close (fd);
    return -1;
  }

  for(offset = 0; offset < length; offset++) {
    SWAPF(data1 + offset);
    SWAPF(data2 + offset);
  }

  printf ("Wrote %d * 2 floats in %s.\n", length, file);
  close (fd);
  return length;
}
