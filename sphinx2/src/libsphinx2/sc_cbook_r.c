/* ====================================================================
 * Copyright (c) 1991-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * Semi-continuous codebook read routines
 *
 * HISTORY
 * 
 * 19-Nov-97  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added ability to read power variance file if it exists.
 * 
 * 08-Oct-91  Eric Thayer (eht) at Carnegie-Mellon University
 *	Created from system by Xuedong Huang
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "s2types.h"
#include "log.h"
#include "scvq.h"
#include "cepio.h"
#include "sc_vq_internal.h"

static int32 fLenMap[NUM_FEATURES] = {
    CEP_VECLEN, DCEP_VECLEN, POW_VECLEN, CEP_VECLEN
    };

/*
 * reads either a raw .vec (mean) file or a raw .var (variance) file.
 * more computation is done after a raw .var is read.
 */
static int32 readCBFile(feat_t feat, float **CB, char *CBFile)
{
  int32 len;

  /* do some sanity checks on the args. Define NDEBUG to disable */
  assert(CB != NULL);
  assert(CBFile != NULL);
  if (cep_read_bin(CB, &len, CBFile)) {
    perror ("readCBFile:");
    fprintf(stderr, "readCBFile: couldn't read codebook %s\n", CBFile);
    return -1;
  }
#ifndef NDEBUG
  /* sanity check codebook size */
  switch (feat) {
  case CEP_FEAT:
    len /= (sizeof(float)*CEP_VECLEN);
    break;
  case DCEP_FEAT:
    len /= (sizeof(float)*DCEP_VECLEN);
    break;
  case DDCEP_FEAT:
    len /= (sizeof(float)*CEP_VECLEN);
    break;
  case POW_FEAT:
    len /= (sizeof(float)*POW_VECLEN);
    break;
  }
  assert(len == NUM_ALPHABET);
#endif
  return 0;
}

static float vFloor = DEF_VAR_FLOOR;	/* variance floor */

void setVarFloor(double aVal)
{
  vFloor = aVal;
}

/* no processing on mean file, just read it */
int32 readMeanCBFile(feat_t feat, float **CB, char *MeanCBFile)
{
  /* do some sanity checks on the args. Define NDEBUG to disable */
  assert(((int32)feat < NUM_FEATURES) && ((int32)feat >= 0));
  assert(MeanCBFile != NULL);
  assert(CB != NULL);
  
  if (readCBFile(feat, CB, MeanCBFile) < 0) return -1;
  return 0;
}

static double two_pi = 6.2831852;	/* almost M_PI * 2.0; */

/*
 * Arguments:
 * 	feat	- used to identify parameters for a given feature.
 *	det	- det array for the feature (NUM_ALPHABET in length)
 *	CB	- float array for the feature codebook.
 *	VarCBFile - path to raw var CB for the given feature, feat.
 */
int32 readVarCBFile(feat_t feat, int32 *det, float **CB, char *VarCBFile)
{
    int32 vecLen, i, j;
    int32 d;
    float *CBP;

    /* do some sanity checks on the args. Define NDEBUG to disable */
    assert(((int32)feat < NUM_FEATURES) && ((int32)feat >= 0));
    assert(det != NULL);
    assert(CB != NULL);
    assert(VarCBFile != NULL);
    /* no automatically generated power variance file.  see setPowVar(). */

    if (readCBFile(feat, CB, VarCBFile) < 0) return -1;
    CBP = *CB;
    vecLen = fLenMap[(int32)feat];
    if (feat != DCEP_FEAT) {
	for (i = 0; i < NUM_ALPHABET; i++) {
	    d = 0;
	    for (j = 1, *CBP++ = 0; j < vecLen; j++, CBP++) {
		if (*CBP < vFloor) *CBP = vFloor;
		d += LOG (1 / sqrt(*CBP * two_pi));
		*CBP = (1.0 / (2.0 * *CBP * LOG_BASE));
	    }
	    *det++ = d;
	}
    }
    else {
	for (i = 0; i < NUM_ALPHABET; i++) {
	    d = 0;
	    for (j = 1, *CBP++ = 0; j < vecLen; j++, CBP++) {
#if 0
		/* This is a hack from xdh. no int32er used.
		 * might be used in the future. Should parameterize.
		 */
		if (j > CEP_VECLEN) *CBP /= DCEP_LONGWEIGHT;
#endif
		if (*CBP < vFloor) *CBP = vFloor;
		d += LOG (1.0 / sqrt(*CBP * two_pi));
		*CBP = (1.0 / (2.0 * *CBP * LOG_BASE));
	    }
	    *det++ = d;
	}
    }
    return 0;
}

#define POWER_VARIANCE 0.05

int32 setPowVar(int32 *det, float **CB, double pow_var)
{
  float *CBP;
  int32 i = NUM_ALPHABET;

  /* do some sanity checks on the args. Define NDEBUG to disable */
  assert(CB != NULL);
  assert(det != NULL);

  CBP = *CB = (float *) malloc(POW_VECLEN*sizeof(float)*NUM_ALPHABET);
  if (CBP == NULL) {
    perror("readCBFiles: allocating power variances vectors\n");
    return -1;
  }
  do {
#ifdef NO_DIV_POWVAR_BY_2
    *CBP++ = pow_var/LOG_BASE;
    *CBP++ = 1.0/LOG_BASE;
    *CBP++ = 0.125/LOG_BASE;
    *det++ = LOG(pow_var * 1.0 * 0.125);
#else
    *CBP++ = pow_var/(2.0 * LOG_BASE);
    *CBP++ = 1.0/(2.0 * LOG_BASE);
    *CBP++ = 0.125/(2.0 * LOG_BASE);
    *det++ = LOG(pow_var * 1.0 * 0.125)/2.0;
#endif
  } while (--i);

  return 0;
}

