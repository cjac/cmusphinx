/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * feat_s2_4x.c -- Cepstral features as in original Sphinx-II.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <assert.h>

#include "feat_s2_4x.h"
#include <libutil/libutil.h>
#include <libio/libio.h>


typedef enum {FEAT_CEP=0, FEAT_DCEP=1, FEAT_POW=2, FEAT_D2CEP=3} feat_t;

#define N_FEAT			4

#define FEAT_CEPLEN		12
#define FEAT_DCEPLEN		(FEAT_CEPLEN<<1)
#define FEAT_POWLEN		3
#define FEAT_D2CEPLEN		FEAT_CEPLEN

#define FEAT_DCEP_WIN		2
#define FEAT_DCEP_LONGWIN	4


static int32 feat_size[N_FEAT] = {
    FEAT_CEPLEN,
    FEAT_DCEPLEN,
    FEAT_POWLEN,
    FEAT_D2CEPLEN
};


int32 feat_s2_4x_cepsize ( int32 veclen )
{
    return (FEAT_CEPLEN+1);
}


int32 feat_s2_4x_featsize (int32 **size)
{
    *size = feat_size;
    return N_FEAT;
}


/*
 * Feature vectors computed from input mfc vectors using this window (+/- window).
 */
int32 feat_s2_4x_window_size ( void )
{
    return FEAT_DCEP_LONGWIN;
}


void feat_s2_4x_cep2feat (float32 **mfc, float32 **feat)
{
    float32 *f;
    float32 *w, *_w;
    float32 *w1, *w_1, *_w1, *_w_1;
    float32 d1, d2;
    int32 i, j;
    
    /* CEP; skip C0 */
    memcpy (feat[FEAT_CEP], mfc[0]+1, FEAT_CEPLEN * sizeof(float32));
    
    /*
     * DCEP(SHORT): mfc[w] - mfc[-w], where w = FEAT_DCEP_WIN;
     * DCEP(LONG):  mfc[w] - mfc[-w], where w = FEAT_DCEP_LONGWIN
     */
    assert (FEAT_DCEPLEN == (FEAT_CEPLEN<<1));

    w  = mfc[ FEAT_DCEP_WIN] + 1;	/* +1 to skip C0 */
    _w = mfc[-FEAT_DCEP_WIN] + 1;

    f = feat[FEAT_DCEP];
    for (i = 0; i < FEAT_CEPLEN; i++)	/* Short-term */
	f[i] = w[i] - _w[i];

    w  = mfc[ FEAT_DCEP_LONGWIN] + 1;	/* +1 to skip C0 */
    _w = mfc[-FEAT_DCEP_LONGWIN] + 1;

    for (j = 0; i < FEAT_DCEPLEN; i++, j++)	/* Long-term */
	f[i] = w[j] - _w[j];

    /* D2CEP: (mfc[w+1] - mfc[-w+1]) - (mfc[w-1] - mfc[-w-1]), where w = FEAT_DCEP_WIN */
    w1   = mfc[ FEAT_DCEP_WIN+1] + 1;	/* Final +1 to skip C0 */
    _w1  = mfc[-FEAT_DCEP_WIN+1] + 1;
    w_1  = mfc[ FEAT_DCEP_WIN-1] + 1;
    _w_1 = mfc[-FEAT_DCEP_WIN-1] + 1;

    f = feat[FEAT_D2CEP];
    for (i = 0; i < FEAT_CEPLEN; i++) {
	d1 =  w1[i] -  _w1[i];
	d2 = w_1[i] - _w_1[i];

	f[i] = d1 - d2;
    }
    
    /* POW: C0, DC0, D2C0; differences computed as above for rest of cep */
    f = feat[FEAT_POW];

    f[0] = mfc[0][0];
    f[1] = mfc[FEAT_DCEP_WIN][0] - mfc[-FEAT_DCEP_WIN][0];

    d1 = mfc[FEAT_DCEP_WIN+1][0] - mfc[-FEAT_DCEP_WIN+1][0];
    d2 = mfc[FEAT_DCEP_WIN-1][0] - mfc[-FEAT_DCEP_WIN-1][0];
    f[2] = d1 - d2;
}
