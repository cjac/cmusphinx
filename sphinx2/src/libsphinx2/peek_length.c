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
peek_length (char const *file)
{
  int             fd, len;
/*  char		  buff[4]; */
  int buff;

  if ((fd = open (file, O_RDONLY, 0644)) < 0)
  {
    char temp[200];

    sprintf (temp, "peek_length: '%s'",file);
    perror (temp);
    return -1;
  }
  if (read (fd, &buff, 4) != 4)
  {
    fprintf (stderr, "peek_length: %s: can't read length\n", file);
    close (fd);
    return -1;
  }
  SWAPL(&buff);
  close(fd);
/*  len = buff[0] << 24 | buff[1] << 16 | buff[2] << 8 | buff[3]; */
  len = buff;
  if (len < 0)
    fprintf (stderr, "Warning: peek_length: %s: length(%d) < 0\n", file, len);
  return len;
}
