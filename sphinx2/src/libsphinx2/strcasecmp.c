/*
 * HISTORY
 *
 * 14-Oct-92 Eric Thayer (eht+@cmu.edu) Carnegie Mellon University
 *	added formal declarations for a and
 * 14-Oct-92 Eric Thayer (eht+@cmu.edu) Carnegie Mellon University
 *	installed ulstrcmp() for strcasecmp() because DEC alpha for some reason
 *	seg faults on call to strcasecmp(). (OSF/1 BL8)
 */

#include "strfuncs.h"

/*
 * case INSENSITIVE.
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

static int
ulstrcmp(char const *str1, char const *str2)
{
    register char c1,c2;

    for(;;) {
	c1 = *str1++;
	if (c1 <= 'Z')
	    if (c1 >= 'A')
		c1 += 040;
	c2 = *str2++;
	if (c2 <= 'Z')
	    if (c2 >= 'A')
		c2 += 040;
	if (c1 != c2)
	    break;
	if (c1 == '\0')
	    return(0);
    }
    return(c1 - c2);
}

int
mystrcasecmp (char const *a, char const *b)
{
    if (a && b)
	return ulstrcmp(a,b);
    else
    	return 1;
}
