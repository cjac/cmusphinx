/************************************************************************
 *  skipover and skipto -- skip over characters in string
 *
 *  Usage:	p = skipto (string,charset);
 *		p = skipover (string,charset);
 *
 *  char *p,*charset,*string;
 *
 *  Skipto returns a pointer to the first character in string which
 *  is in the string charset; it "skips until" a character in charset.
 *  Skipover returns a pointer to the first character in string which
 *  is not in the string charset; it "skips over" characters in charset.
 ************************************************************************
 *
 * HISTORY
 * 26-Jun-81  David Smith (drs) at Carnegie-Mellon University
 *	Skipover, skipto rewritten to avoid inner loop at expense of space.
 *
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Skipover, skipto adapted for VAX from skip() and skipx() on the PDP-11
 *	(from Ken Greer).  The names are more mnemonic.
 *
 *	Sindex adapted for VAX from indexs() on the PDP-11 (thanx to Ralph
 *	Guggenheim).  The name has changed to be more like the index()
 *	and rindex() functions from Bell Labs; the return value (pointer
 *	rather than integer) has changed partly for the same reason,
 *	and partly due to popular usage of this function.
 */

#include "strfuncs.h"

static unsigned char tab[256] = {
	0};

char *
skipto (unsigned char *string, unsigned char const *charset)
{
	register unsigned char const *setp;
	register unsigned char *strp;

	tab[0] = 1;		/* Stop on a null, too. */
	for (setp=charset;  *setp;  setp++) tab[*setp]=1;
	for (strp=string;  tab[*strp]==0;  strp++)  ;
	for (setp=charset;  *setp;  setp++) tab[*setp]=0;
	return strp;
}

char *
skipover (unsigned char *string, unsigned char const *charset)
{
    register unsigned char const *setp;
    register unsigned char *strp;

	tab[0] = 0;		/* Do not skip over nulls. */
	for (setp=charset;  *setp;  setp++) tab[*setp]=1;
	for (strp=string;  tab[*strp];  strp++)  ;
	for (setp=charset;  *setp;  setp++) tab[*setp]=0;
	return strp;
}
