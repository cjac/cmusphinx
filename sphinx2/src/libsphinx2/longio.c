/*
 * longword I/O routines
 * 
 * Created October 11, 1989 by Joe Keane.
 * 
 * These routines read and write longwords in a machine-independent format:
 * four bytes, most significant first.  On the Sun this is the same as the
 * internal format, so these routines can use fread and fwrite.  The routines
 * return negative result on error, except for read_long, where the caller
 * has to check for errors.
 */

#include <stdio.h>


long read_long (FILE *stream)
{
  int             c;
  long            word;

  c = getc (stream);
  if (c == EOF)
    return -1;
  word = c;
  c = getc (stream);
  if (c == EOF)
    return -1;
  word = word << 8 | c;
  c = getc (stream);
  if (c == EOF)
    return -1;
  word = word << 8 | c;
  c = getc (stream);
  if (c == EOF)
    return -1;
  return word << 8 | c;
}


int write_long (FILE *stream, long word)
{
  if (putc (word >> 24, stream) == EOF)
    return -1;
  if (putc (word >> 16, stream) == EOF)
    return -1;
  if (putc (word >> 8, stream) == EOF)
    return -1;
  if (putc (word, stream) == EOF)
    return -1;
  return 0;
}


int read_long_array (FILE *stream, long *base, int length)
{
#ifdef sun
  return fread ((char *) base, length * 4, 1, stream) != 1 ? -1 : 0;
#else
  int             counter;
  long           *ptr;

  counter = length;
  ptr = base;
  while (--counter >= 0)
  {
    int             c;
    long            word;

    c = getc (stream);
    if (c == EOF)
      return -1;
    word = c;
    c = getc (stream);
    if (c == EOF)
      return -1;
    word = word << 8 | c;
    c = getc (stream);
    if (c == EOF)
      return -1;
    word = word << 8 | c;
    c = getc (stream);
    if (c == EOF)
      return -1;
    *ptr++ = word << 8 | c;
  }
  return 0;
#endif
}


int write_long_array (FILE *stream, long *base, int length)
{
#ifdef sun
  return fwrite((char *) base, length * 4, 1, stream) != 1 ? -1 : 0;
#else
  int             counter;
  long           *ptr;

  counter = length;
  ptr = base;
  while (--counter >= 0)
  {
    long            word;

    word = *ptr++;
    if (putc (word >> 24, stream) == EOF)
      return -1;
    if (putc (word >> 16, stream) == EOF)
      return -1;
    if (putc (word >> 8, stream) == EOF)
      return -1;
    if (putc (word, stream) == EOF)
      return -1;
  }
  return 0;
#endif
}
