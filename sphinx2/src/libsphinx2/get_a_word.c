/*
 * HISTORY
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created by ANONYMOUS.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/* default: more_separator = ' ' */
char *get_a_word (line, word, more_separator)
char *line, *word, more_separator;
{
  register int i;

  while (*line == more_separator || isspace(*line)) line++;
  if (*line == '\0') return NULL;
  i = 0;
  do { word[i++] = *line++;} while (!isspace(*line) && *line != more_separator&& *line != '\0');
  word[i] = '\0';
  return line;
}

#if 0 /* Doesn't appear to be used anywhere */
static void
find_sentid (char *file_head, char *sentid)
{
  register int i, j;
  int len, suffix;

  suffix = 0;
  len = strlen (file_head);
  if (file_head[len-1] == 'b' && file_head[len-2] == '-')
  {
    suffix = 1;
    len -= 2;
    file_head[len] = '\0';
  }
  i = len;
  while (file_head[--i] != '/');
  j = 0;
  while ( (sentid[j++] = file_head[++i]) != '\0');
  if (suffix)
  {
    file_head[len] = '-';	file_head[len+1] = 'b';
  }
}
#endif
