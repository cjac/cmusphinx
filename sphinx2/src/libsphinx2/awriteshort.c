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
awriteshort (char const *file, short *data, int length)
{
  int             fd;
  int             size;
  int             offset;

  if ((fd = open (file, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
  {
    fprintf (stderr, "awriteshort: %s: can't create\n", file);
    return -1;
  }

  SWAPL(&length);
  if (write (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "awriteshort: %s: can't write length\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&length);

  /* FIXME: gack.  shouldn't modify data! */
  for(offset = 0; offset < length; offset++)
    SWAPW(data + offset);
  size = length * sizeof (short);
  if (write (fd, (char *) data, size) != size)
  {
    fprintf (stderr, "awriteshort: %s: can't write data\n", file);
    close (fd);
    return (-1);
  }
  for(offset = 0; offset < length; offset++)
    SWAPW(data + offset);

  printf ("Wrote %d shorts in %s.\n", length, file);
  close (fd);
  return length;
}
