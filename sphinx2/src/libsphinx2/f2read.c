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
