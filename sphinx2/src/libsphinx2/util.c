/*
 * **********************************************
 * DESCRIPTION
 *	swapLong(int32 *p)	- do a int32 byte swap at p.
 */

#include "s2types.h"

void swapLong(intp)
/*------------------------------------------------------------*
 * Swap the int32 integer at intp
 */
int32 *intp;
{
  *intp = ((*intp << 24) & 0xFF000000) |
	  ((*intp <<  8) & 0x00FF0000) |
	  ((*intp >>  8) & 0x0000FF00) |
	  ((*intp >> 24) & 0x000000FF);
}

void swapShortBuf (p, cnt)
int16 *p;
int32 cnt;
{
    while (cnt-- > 0) {
	*p = ((*p << 8) & 0x0FF00) |
	     ((*p >>  8) & 0x00FF);
	++p;	/* apollo compiler seems to break with *p++ = f(*p) */
    }
}


void swapLongBuf (p, cnt)
int32 *p;
int32 cnt;
{
    while (cnt-- > 0) {
	*p = ((*p << 24) & 0xFF000000) |
	     ((*p <<  8) & 0x00FF0000) |
	     ((*p >>  8) & 0x0000FF00) |
	     ((*p >> 24) & 0x000000FF);
	++p;	/* apollo compiler seems to break with *p++ = f(*p) */
    }
}
