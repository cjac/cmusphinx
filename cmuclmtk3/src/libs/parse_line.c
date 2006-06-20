/* ====================================================================
 * Copyright (c) 1999-2006 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */


#include <ctype.h>

void parse_line(
	char *line, int mwords, int canonize,
	char **pword_begin, char **pword_end, int *p_nwords, int *p_overflow)
{
  char *pl, *psq, *ptmp, *pbegin, *pend;
  int  nwords=0;

  *p_overflow = 0;
  pl = line-1;
  psq = line;
  do {
     do pl++; while (isspace(*pl));           /* find beginning of next word */
     if (*pl==0) break;			      /* no more words */
     if (nwords>=mwords) {*p_overflow=1; break;} /* no room for next word */
     nwords++;
     pbegin = pl;
     do pl++; while (!isspace(*pl) && *pl!=0); /* find end of current word */
     pend = pl;   /* (word ends in whitespace or e.o.line) */

     if (canonize) {
        *pword_begin++ = psq;
        if (psq!=pbegin) for (ptmp=pbegin; ptmp<pend;) *psq++ = *ptmp++;
        else psq = pend;
        *pword_end++ = psq;
        *psq++ = ' ';
     }
     else {
        *pword_begin++ = pbegin;
        *pword_end++ = pend;
     }
  } while (*pl!=0);

  if (canonize) **(pword_end-1) = '\0';
  *p_nwords = nwords;
}
