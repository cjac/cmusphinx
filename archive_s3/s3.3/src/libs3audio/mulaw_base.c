/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
/*
 * mulaw.c -- u-Law audio routines.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.3  2001/12/11  04:40:55  lenzo
 * License cleanup.
 * 
 * Revision 1.2  2001/12/11 04:10:16  lenzo
 * License.
 *
 * Revision 1.1.1.1  2001/12/03 16:01:45  egouvea
 * Initial import of sphinx3
 *
 * Revision 1.1  2000/12/12 22:55:55  lenzo
 * Separate out a/d lib.
 *
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:52  lenzo
 * Initial import of sphinx2
 *
 *
 * 
 * 22-Jul-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Sunil's version.
 */


#include "s3types.h"
#include "ad.h"
#include "mulaw.h"


void ad_mu2li (int16 *out, unsigned char *in, int32 n_samp)
{
    int32 i;
    
    for (i = 0; i < n_samp; i++)
	out[i] = muLaw[in[i]];
}
