/*
	30 May 1989 David R. Fulmer (drf) updated to do byte order
		conversions when necessary.
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <fcntl.h>
#else
#include <sys/file.h>
#include <sys/fcntl.h>
#include <unistd.h>
#endif
#include "byteorder.h"


int
areadshort (char *file, short **data_ref, int *length_ref)
{
  int             fd;
  int             length;
  int             size;
  int             offset;
  char           *data;

  if ((fd = open (file, O_RDONLY, 0644)) < 0)
  {
    fprintf (stderr, "areadshort: %s: can't open\n", file);
    return -1;
  }
  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "areadshort: %s: can't read length (empty file?)\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&length);
  size = length * sizeof (short);
  if (!(data = malloc ((unsigned) size)))
  {
    fprintf (stderr, "areadshort: %s: can't alloc data\n", file);
    close (fd);
    return -1;
  }
  if (read (fd, data, size) != size)
  {
    fprintf (stderr, "areadshort: %s: can't read data\n", file);
    close (fd);
    free (data);
    return -1;
  }
  close (fd);
  *data_ref = (short *) data;
  for(offset = 0; offset < length; offset++)
    SWAPW(*data_ref + offset);
  *length_ref = length;
  return length;
}
