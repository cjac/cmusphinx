/* Various random string functions (dating back to the dawn of time,
   non-reentrant, and probably redundant with modern C libraries) that
   were never prototyped. */

#ifndef _STRINGFUNCS_H_
#define _STRINGFUNCS_H_

char *nxtarg (char **q, char const *brk);
char *skipto(unsigned char *string,
	     unsigned char const *charset);
char *skipover (unsigned char *string,
		unsigned char const *charset);
char *salloc(char const *str);
int mystrcasecmp(char const *a, char const *b);

#endif /* _STRINGFUNCS_H_ */
