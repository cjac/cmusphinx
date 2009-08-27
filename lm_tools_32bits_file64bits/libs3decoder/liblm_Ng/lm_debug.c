/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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

/** \file lm_debug.c
	\brief Debugging library
 
	A library for debugging miscellaneous problem of the LM data structure.
*/

/*
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * 2008/06/27  N. Coetmeur, supervised by Y. Esteve
 * Adjust comments for compatibility with Doxygen 1.5.6
 *
 * 2008/06/17  N. Coetmeur, supervised by Y. Esteve
 * Replace bg_write and tg_write functions by ng_write for working with N-grams
 * for each N value (i.e for a trigram file N=3, for a quadrigram file N=4 and
 * so on...).
 * Add ng32_write function for working in 32 bits.
 *
 *
 */

#include "lm.h"


/*
   Write an unigram structure
*/
void
ug_write(FILE * fp, ug_t * ug)
{
    fprintf(fp, "UG dictwid %d, prob %f, bowt %f, firstbg %d\n",
            ug->dictwid, ug->prob.f, ug->bowt.f, ug->firstbg);
    fflush(fp);
}

/*
   Write a N-gram structure
*/
void
ng_write(FILE * fp, ng_t * ng, uint32 Nc, uint32 max_N)
{
	if ( (Nc <= 2) || (Nc < max_N) )
		fprintf(fp, "%dG lmwid %d, prob ID %d, bowt ID %d, firsttg %d\n",
				Nc, ng->wid, ng->probid, ng->bowtid, ng->firstnng);
	else
		fprintf(fp, "%dG lmwid %d, prob ID %d\n",
				Nc, ng->wid, ng->probid);

	fflush(fp);
}

/*
   Write a 32-bits N-gram structure
*/
void
ng32_write(FILE * fp, ng32_t * ng, uint32 Nc, uint32 max_N)
{
	if ( (Nc <= 2) || (Nc < max_N) )
		fprintf(fp, "%dG lmwid %d, prob ID %d, bowt ID %d, firsttg %d\n",
				Nc, ng->wid, ng->probid, ng->bowtid, ng->firstnng);
	else
		fprintf(fp, "%dG lmwid %d, prob ID %d\n",
				Nc, ng->wid, ng->probid);

	fflush(fp);
}
