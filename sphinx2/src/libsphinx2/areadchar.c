/*
 *
 * **********************************************
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

#include "s2types.h"
#include "byteorder.h"


int32 areadchar (char *file, char **data_ref, int32 *length_ref)
{
  int             fd;
  int             length;
  char           *data;

  if ((fd = open (file, O_RDONLY, 0644)) < 0)
  {
    fprintf (stderr, "areadchar: %s: can't open\n", file);
    return -1;
  }
  if (read (fd, (char *) &length, 4) != 4)
  {
    fprintf (stderr, "areadchar: %s: can't read length (empty file?)\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&length);
  if (!(data = malloc ((unsigned) length)))
  {
    fprintf (stderr, "areadchar: %s: can't alloc data\n", file);
    close (fd);
    return -1;
  }
  if (read (fd, data, length) != length)
  {
    fprintf (stderr, "areadchar: %s: can't read data\n", file);
    close (fd);
    free (data);
    return -1;
  }
  close (fd);
  *data_ref = data;
  *length_ref = length;
  return length;
}
