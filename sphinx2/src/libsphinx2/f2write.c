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
