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
 *
 * HISTORY
 * 
 * 19-Nov-97  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added ability to read power variance file if it exists.
 * 
 * 19-Jun-95  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added scvq_set_psen() and scvq_set_bestpscr().  Modified SCVQScores_all to
 * 	also compute best senone score/phone.
 * 
 * 19-May-95  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added check for bad VQ scores in SCVQScores and SCVQScores_all.
 * 
 * 01-Jul-94  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	In SCVQScores, returned result from SCVQComputeScores_opt().
 * 
 * 01-Nov-93  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added compressed, 16-bit senone probs option.
 * 
 *  6-Apr-92  Fil Alleva (faa) at Carnegie-Mellon University
 *	- added SCVQAgcSet() and agcType.
 *
 * 08-Oct-91  Eric Thayer (eht) at Carnegie-Mellon University
 *	Created from system by Xuedong Huang
 * 22-Oct-91  Eric Thayer (eht) at Carnegie-Mellon University
 *	Installed some efficiency improvements to acoustic scoring
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#include <time.h>
#endif

#include "s2types.h"
#include "log.h"
#include "scvq.h"
#include "sc_vq_internal.h"
#include "logmsg.h"

#if FAST8B
#include "log_add.h"
#endif

#define QUIT(x)		{fflush(stdout); fprintf x; exit(-1);}

#define RPCC(x) asm ("rpcc %t0;" "stq %t0,(%0)",&x)


static int32   detArr[NUM_FEATURES*NUM_ALPHABET];	/* storage for det vectors */
static int32   *dets[NUM_FEATURES];	/* det values foreach feature */
static float *means[NUM_FEATURES];	/* mean vectors foreach feature */
static float *vars[NUM_FEATURES];	/* var vectors foreach feature */

static int32 topN = MAX_TOPN;
static int32 CdWdPDFMod;

static int32 *scrPass = NULL;

static double dcep80msWeight = 1.0;

static int32 use20ms_diff_pow = FALSE;
/* static int32 useMeanNormalization = FALSE; */

static int32 num_frames = 0;

static vqFeature_t lcfrm[MAX_TOPN];
static vqFeature_t ldfrm[MAX_TOPN];
static vqFeature_t lxfrm[MAX_TOPN];
static vqFeature_t vtmp;

#ifdef WIN32
static HANDLE pid;
static FILETIME t_create, t_exit, kst, ket, ust, uet;
static double vq_time;
static double scr_time;
extern double win32_cputime();
#endif


static void cepDist0(vqFeature_t *topn, float *z)
{
    register int32	i, j, cw;
    vqFeature_t		*worst, *best = topn, *cur; /*, *src; */
    double		diff, d;
    float		*obs;
    float		*mean;
    float		*var  =  vars[(int32)CEP_FEAT];
    int32			*det  =  dets[(int32)CEP_FEAT], *detP;
    int32			*detE = det + NUM_ALPHABET;

    assert(z    != NULL);
    assert(topn != NULL);
    memcpy (topn, lcfrm, sizeof(vqFeature_t)*topN);
    worst = topn + (topN-1);
    /* initialize topn codewords to topn codewords from previous frame */
    for (i = 0; i < topN; i++) {
	cw = topn[i].codeword;
	mean = means[(int32)CEP_FEAT] + cw*CEP_VECLEN + 1;
	var  =  vars[(int32)CEP_FEAT] + cw*CEP_VECLEN + 1;
	d = det[cw];
	obs = z+1;
	for (j = 1; j < CEP_VECLEN; j++) {
	    diff = *obs++ - *mean++;
	    d -= diff * diff * (*var++);
	}
	topn[i].val.dist = (int32)d;
	if (i == 0) continue;
	vtmp = topn[i];
	for (j = i-1; j >= 0 && (int32)d > topn[j].val.dist; j--) {
	    topn[j+1] = topn[j];
	}
	topn[j+1] = vtmp;
    }
    mean = means[(int32)CEP_FEAT];
    var = vars[(int32)CEP_FEAT];
    for (detP = det, ++mean, ++var; detP < detE; detP++, ++mean, ++var) {
	d = *detP; obs = z+1;
	for (j = 1; (j < CEP_VECLEN) && (d >= worst->val.dist); j++) {
	    diff = *obs++ - *mean++;
	    d -= diff * diff * (*var++);
	}
	if (j < CEP_VECLEN) {
	    /* terminated early, so not in topn */
	    mean += (CEP_VECLEN - j);
	    var  += (CEP_VECLEN - j);
	    continue;
	}
	if (d < worst->val.dist) continue;
	cw = detP - det;
	for (i = 0; i < topN; i++) {
	    /* already there, so don't need to insert */
	    if (topn[i].codeword == cw) break;
	}
	if (i < topN) continue;	/* already there.  Don't insert */
	/* remaining code inserts codeword and dist in correct spot */
	for (cur = worst-1; cur >= best && d >= cur->val.dist; --cur)
	    memcpy (cur+1, cur, sizeof(vqFeature_t));
	++cur;
	cur->codeword = cw;
	cur->val.dist = (int32)d;
    }

    memcpy(lcfrm, topn, sizeof(vqFeature_t)*topN);
}

#if 0
static void cepDist3(vqFeature_t *topn, float *z)
/*------------------------------------------------------------*
 * Identical functionality with respect to cepDist0, but faster
 * since the input vector is loaded into registers.
 */
{
    register int32	cw;
    long		i, j;
    vqFeature_t		*bottom, *top, *cur;
    double		diff, d, worst_dist;
    float		*mean;
    float		*var  =  vars[(int32)CEP_FEAT];
    int32			*det  =  dets[(int32)CEP_FEAT], *detP;
    int32			*detE = det + NUM_ALPHABET;
    float		c1 = z[1],
			c2 = z[2],
			c3 = z[3],
			c4 = z[4],
			c5 = z[5],
			c6 = z[6],
			c7 = z[7],
			c8 = z[8],
			c9 = z[9],
			c10 = z[10],
			c11 = z[11],
			c12 = z[12];

    assert(z    != NULL);
    assert(topn != NULL);

    worst_dist = 1e+300;
    top = &topn[0];
    bottom = &topn[topN-1];
    /*
     * initialize worst score
     */
    for (i = 0; i < topN; i++) {
	cw = lcfrm[i].codeword;
	mean = means[(int32)CEP_FEAT] + cw*CEP_VECLEN;
	var  =  vars[(int32)CEP_FEAT] + cw*CEP_VECLEN;
	d = det[cw];

	diff = c1 - mean[1];
	d -= diff * diff * var[1];
	diff = c2 - mean[2];
	d -= diff * diff * var[2];
	diff = c3 - mean[3];
	d -= diff * diff * var[3];
	diff = c4 - mean[4];
	d -= diff * diff * var[4];
	diff = c5 - mean[5];
	d -= diff * diff * var[5];
	diff = c6 - mean[6];
	d -= diff * diff * var[6];
	diff = c7 - mean[7];
	d -= diff * diff * var[7];
	diff = c8 - mean[8];
	d -= diff * diff * var[8];
	diff = c9 - mean[9];
	d -= diff * diff * var[9];
	diff = c10 - mean[10];
	d -= diff * diff * var[10];
	diff = c11 - mean[11];
	d -= diff * diff * var[11];
	diff = c12 - mean[12];
	d -= diff * diff * var[12];

	if (d < worst_dist)
	    worst_dist = d;
    }

    /*
     * Worst score must be small enough to guarentee that
     * the top 4 scores for this frame are better than worst_score.
     */
    for (i = 0; i < topN; i++) {
	topn[i].val.dist = (worst_dist - 1.5);
	topn[i].codeword = lcfrm[i].codeword;
    }

    mean = means[(int32)CEP_FEAT];
    var = vars[(int32)CEP_FEAT];
    for (detP = det; detP < detE; detP++, mean += CEP_VECLEN, var += CEP_VECLEN) {
	int32 int_d;

	d = *detP;

	diff = c1 - mean[1];
	d -= diff * diff * var[1];
	diff = c2 - mean[2];
	d -= diff * diff * var[2];
	diff = c3 - mean[3];
	d -= diff * diff * var[3];
	diff = c4 - mean[4];
	d -= diff * diff * var[4];
	if (d < worst_dist)
	    continue;
	diff = c5 - mean[5];
	d -= diff * diff * var[5];
	diff = c6 - mean[6];
	d -= diff * diff * var[6];
	diff = c7 - mean[7];
	d -= diff * diff * var[7];
	diff = c8 - mean[8];
	d -= diff * diff * var[8];
	if (d < worst_dist)
	    continue;
	diff = c9 - mean[9];
	d -= diff * diff * var[9];
	diff = c10 - mean[10];
	d -= diff * diff * var[10];
	diff = c11 - mean[11];
	d -= diff * diff * var[11];
	diff = c12 - mean[12];
	d -= diff * diff * var[12];
	if (d < worst_dist)
	    continue;

	cw = detP - det;

	/* 
	 * remaining code inserts codeword and dist in correct spot
	 */
	int_d = (int32)(d - 0.5);
	for (cur = bottom-1; cur >= top && int_d >= cur->val.dist; --cur) {
	    cur[1].codeword = cur[0].codeword;
	    cur[1].val.dist = cur[0].val.dist;
	}
	++cur;
	cur->codeword = cw;
	cur->val.dist = int_d;
	worst_dist = (double) bottom->val.dist;
    }

    memcpy (lcfrm, topn, sizeof(vqFeature_t)*topN);
}
#endif

static void dcepDist0(vqFeature_t *topn, float *dzs, float *dzl)
{
    register int32	i, j, cw;
    vqFeature_t		*worst, *best = topn, *cur; /* , *src; */
    double		diff, d;
    float		*obs1, *obs2;
    float		*mean;
    float		*var;
    int32			*det  =  dets[(int32)DCEP_FEAT], *detP;
    int32			*detE = det + NUM_ALPHABET; /*, tmp; */

    assert(dzs != NULL);
    assert(dzl != NULL);
    assert(topn != NULL);

    memcpy (topn, ldfrm, sizeof(vqFeature_t)*topN);
    worst = topn + (topN-1);
    /* initialize topn codewords to topn codewords from previous frame */
    for (i = 0; i < topN; i++) {
	cw = topn[i].codeword;
	mean = means[(int32)DCEP_FEAT] + cw*DCEP_VECLEN + 1;
	var  =  vars[(int32)DCEP_FEAT] + cw*DCEP_VECLEN + 1;
	d = det[cw];
	obs1 = dzs+1;
	obs2 = dzl+1;
	for (j = 1; j < CEP_VECLEN; j++, obs1++, obs2++, mean++, var++) {
	    diff = *obs1 - *mean;
	    d -= diff * diff * *var;
	    diff = (*obs2 - mean[CEP_VECLEN-1]) * dcep80msWeight;
	    d -= diff * diff * var[CEP_VECLEN-1];
	}
	topn[i].val.dist = (int32)d;
	if (i == 0) continue;
	vtmp = topn[i];
	for (j = i-1; j >= 0 && (int32)d > topn[j].val.dist; j--) {
	    topn[j+1] = topn[j];
	}
	topn[j+1] = vtmp;
    }
    mean = means[(int32)DCEP_FEAT];
    var = vars[(int32)DCEP_FEAT];
    /* one distance value for each codeword */
    for (detP = det; detP < detE; detP++) {
	d = *detP;
	obs1 = dzs+1;		/* reset observed */
	obs2 = dzl+1;		/* reset observed */
	for (j = 1, ++mean, ++var;
	     (j < CEP_VECLEN) && (d NEARER worst->val.dist);
	     j++, obs1++, obs2++, mean++, var++) {
	    diff = *obs1 - *mean;
	    d -= diff * diff * *var;
 	    diff = (*obs2 - mean[CEP_VECLEN-1]) * dcep80msWeight;
	    d -= diff * diff * var[CEP_VECLEN-1];
	}
	mean += CEP_VECLEN-1;
	var += CEP_VECLEN-1;
	if (j < CEP_VECLEN) {
	    mean += (CEP_VECLEN-j);	
	    var  += (CEP_VECLEN-j);
	    continue;
	}
	if (d < worst->val.dist) continue;
	cw = detP - det;
	for (i = 0; i < topN; i++) {
	    /* already there, so don't need to insert */
	    if (topn[i].codeword == cw) break;
	}
	if (i < topN) continue;	/* already there.  Don't insert */
	/* remaining code inserts codeword and dist in correct spot */
	for (cur = worst-1; cur >= best && (int32)d >= cur->val.dist; --cur)
	    memcpy(cur+1, cur, sizeof(vqFeature_t));
	++cur;
	cur->codeword = cw;
	cur->val.dist = (int32)d;
    }

    memcpy (ldfrm, topn, sizeof(vqFeature_t)*topN);
}


static void ddcepDist0(vqFeature_t *topn, float *z)
{
    register int32 i, j, cw;
    vqFeature_t		*worst, *best = topn, *cur; /*, *src; */
    double	diff, d;
    float	*obs;
    float	*mean;
    float	*var;
    int32		*det  =  dets[(int32)DDCEP_FEAT];
    int32		*detE = det + NUM_ALPHABET;
    int32		*detP; /*, tmp; */

    assert(z    != NULL);
    assert(topn != NULL);
    memcpy (topn, lxfrm, sizeof(vqFeature_t)*topN);
    worst = topn + (topN-1);
    /* initialize topn codewords to topn codewords from previous frame */
    for (i = 0; i < topN; i++) {
	cw = topn[i].codeword;
	mean = means[(int32)DDCEP_FEAT] + cw*CEP_VECLEN + 1;
	var  =  vars[(int32)DDCEP_FEAT] + cw*CEP_VECLEN + 1;
	d = det[cw];
	obs = z+1;
	for (j = 1; j < CEP_VECLEN; j++) {
	    diff = *obs++ - *mean++;
	    d -= diff * diff * (*var++);
	}
	topn[i].val.dist = (int32)d;
	if (i == 0) continue;
	vtmp = topn[i];
	for (j = i-1; j >= 0 && (int32)d > topn[j].val.dist; j--) {
	    topn[j+1] = topn[j];
	}
	topn[j+1] = vtmp;
    }
    mean = means[(int32)DDCEP_FEAT];
    var = vars[(int32)DDCEP_FEAT];
    for (detP = det, ++mean, ++var; detP < detE; detP++, ++mean, ++var) {
	d = *detP; obs = z+1;
	for (j = 1; (j < CEP_VECLEN) && (d >= worst->val.dist); j++) {
	    diff = *obs++ - *mean++;
	    d -= diff * diff * (*var++);
	}
	if (j < CEP_VECLEN) {
	    /* terminated early, so not in topn */
	    mean += (CEP_VECLEN - j);
	    var  += (CEP_VECLEN - j);
	    continue;
	}
	if (d < worst->val.dist) continue;
	cw = detP - det;
	for (i = 0; i < topN; i++) {
	    /* already there, so don't need to insert */
	    if (topn[i].codeword == cw) break;
	}
	if (i < topN) continue;	/* already there.  Don't insert */
	/* remaining code inserts codeword and dist in correct spot */
	for (cur = worst-1; cur >= best && (int32)d >= cur->val.dist; --cur)
	    memcpy(cur+1, cur, sizeof(vqFeature_t));
	++cur;
	cur->codeword = cw;
	cur->val.dist = (int32)d;
    }

    memcpy(lxfrm, topn, sizeof(vqFeature_t)*topN);
}


static void powDist(vqFeature_t *topn, float *pz)
{
  register int32	i, j, cw;
  float		nextBest;
  float		dist[NUM_ALPHABET];
  double	diff, d;
  float		*dP, *dE = dist + NUM_ALPHABET;
  float		*obs;
  float		*mean = means[(int32)POW_FEAT];
  float		*var  =  vars[(int32)POW_FEAT];
  int32		*det  =  dets[(int32)POW_FEAT];

  assert(pz  != NULL);
  assert(topn != NULL);
  /* one distance value for each codeword */
  for (dP = dist; dP < dE; dP++) {
    d = 0; obs = pz;
    for (j = 0; j < POW_VECLEN; j++, obs++, mean++, var++) {
      diff = *obs - *mean;
      d += diff * diff * *var;
    }
    *dP = *det++ - d;
  }
  /* compute top N codewords */
  for (i = 0; i < topN; i++) {
    dP = dist;
    nextBest = *dP++; cw = 0;
    for (; dP < dE; dP++) {
      if (*dP NEARER nextBest) {
	nextBest = *dP;
	cw = dP - dist;	/* use address arith to compute cw */
      }
    }
    topn[i].codeword = cw;
    topn[i].val.dist = (int32)nextBest;

    dist[cw] = WORST_DIST;
  }
}

/* the cepstrum circular buffer */
static float  inBufArr[CEP_VECLEN*(INPUT_MASK+1)];
static int32    inIdx = 0;

/* the diff cepstrum circular buffer */
static float   dBufArr[CEP_VECLEN*(DIFF_MASK+1)];
static int32    dIdx = 0;


void SCVQInit(int32 top, int32 numModels, int32 numDist, double vFloor, 
	      int32 use20msdp)
{
    int32 i;

    assert((top <= MAX_TOPN) && (top > 0));
    assert(numModels > 0);
    assert(numDist > 0);
    assert(vFloor >= 0.0);

    use20ms_diff_pow = use20msdp;

    for (i = 0; i < MAX_TOPN; i++) {
	lxfrm[i].val.dist = ldfrm[i].val.dist = lcfrm[i].val.dist = WORST_DIST;
	lxfrm[i].codeword = ldfrm[i].codeword = lcfrm[i].codeword = i;
    }
    log_info("SCVQInit: top %d, %d models, %d dist, %f var floor.\n",
	     top, numModels, numDist, vFloor);
    /*
     * Configure TopN
     */
    topN = top;

    CdWdPDFMod = numModels * numDist; /* # prob values per codeword */
    
    if ((scrPass    = (int32 *)calloc(CdWdPDFMod, sizeof(int32))) == NULL)
	QUIT((stdout, "%s(%d): calloc(%d,%d) failed\n", __FILE__, __LINE__,
	      CdWdPDFMod, sizeof(int32)));
    
    setVarFloor(vFloor);	/* see sc_cbook_r.c */

#ifdef WIN32
    pid = GetCurrentProcess();
#endif
}


void SCVQNewUtt(void)
{
    num_frames = 0;

#ifdef WIN32
    vq_time = scr_time = 0.0;
#endif
}


void SCVQEndUtt ( void )
{
#ifdef WIN32
    log_info ("VQ-TIME= %.1fsec, SCR-TIME= %.1fsec (CPU)\n", vq_time, scr_time);
#endif
}


int SCVQComputeFeatures(float **cep,
			float **dcep,
			float **dcep_80ms,
			float **pow,
			float **ddcep,
			float *in)
{
    register int32 i; /*, j; */
    register float *df, *db, *dout;
    static float	ldBufArr[CEP_VECLEN];
    static float	ddBufArr[CEP_VECLEN];
    static float	pBufArr[POW_VECLEN];
    /* int32		tmp[NUM_FEATURES]; */

    memcpy((char *)(inBufArr + inIdx*CEP_VECLEN), (char *)in,
	  sizeof(float)*CEP_VECLEN);
  
    /* compute short duration difference */
    dout = dBufArr + dIdx*CEP_VECLEN;
    df  = inBufArr + inIdx*CEP_VECLEN;
    db  = inBufArr + INPUT_INDEX(inIdx-4)*CEP_VECLEN;
    i  = CEP_VECLEN;
    do {
	*dout++ = (*df++ - *db++);
    } while (--i);

    /* compute int32 duration difference */
    dout = ldBufArr;
    df = inBufArr + inIdx*CEP_VECLEN;
    db = inBufArr + INPUT_INDEX(inIdx-8)*CEP_VECLEN;
    i  = CEP_VECLEN;
    do {
	*dout++ = (*df++ - *db++);
    } while (--i);

    /* compute 2nd order difference */
    dout = ddBufArr;
    df = dBufArr + DIFF_INDEX(dIdx-1)*CEP_VECLEN;
    db = dBufArr + DIFF_INDEX(dIdx-3)*CEP_VECLEN;
    i  = CEP_VECLEN;
    do {
	*dout++ = (*df++ - *db++);
    } while (--i);

    /* pow , diff pow, and 2nd diff pow */
    if (use20ms_diff_pow)
	pBufArr[0] = *(inBufArr + INPUT_INDEX(inIdx-3)*CEP_VECLEN) -
		     *(inBufArr + INPUT_INDEX(inIdx-5)*CEP_VECLEN);
    else
	pBufArr[0] = *(inBufArr + INPUT_INDEX(inIdx-4)*CEP_VECLEN);
    pBufArr[1] = *(dBufArr  +  DIFF_INDEX( dIdx-2)*CEP_VECLEN);
    pBufArr[2] = *ddBufArr;

    *cep  = inBufArr + INPUT_INDEX(inIdx-4)*CEP_VECLEN;
	
    *dcep = dBufArr + DIFF_INDEX(dIdx-2)*CEP_VECLEN;
    
    *dcep_80ms = ldBufArr;

    *pow = pBufArr;

    *ddcep = ddBufArr;

    ++num_frames;

    inIdx = INPUT_INDEX(inIdx+1);
    dIdx  =  DIFF_INDEX(dIdx+1);

    if (num_frames >= MAX_DIFF_WINDOW) {
	return 1;
    }
    else {
	return 0;
    }
}


/* Output PDF/feature (32-bit logprobs) */
int32 *OPDF[NUM_FEATURES] = {NULL, NULL, NULL, NULL};

/*
 * Output PDF/feature (8-bit logprob ids).  Organized as follows (per feature):
 * 256 (int32)logprobs/codeword, pointed to by prob.  Senone probs for a given
 * codeword can only take on one of the associated 256 values, which approximate
 * the originally computed logprobs.  Therefore:
 *     LogProb(feature f, codeword c, senone s) =
 *         OPDF_8B[f]->prob[c][OPDF_8B[f]->id[c][s]]
 */
typedef struct {
    int32 **prob;	/* 2-D array, #codewords x 256 */
    unsigned char **id;	/* 2-D array, #codewords x #senones */
} OPDF_8BIT_T;

OPDF_8BIT_T *OPDF_8B[NUM_FEATURES] = {NULL, NULL, NULL, NULL};

/* Senone prob size (bits).  Can be 8/32 */
static int32 prob_size = 32;

static int32 SCVQComputeScores ();
static int32 SCVQComputeScores_all ();

/* Various topN cases, PDF-size(#bits) cases, and optimizations */
static int32 get_scores4 ();
static int32 get_scores1 ();
static int32 get_scores ();
static int32 get_scores4_all ();
static int32 get_scores1_all ();
static int32 get_scores_all ();
static int32 get_scores4_8b ();
static int32 get_scores1_8b ();
static int32 get_scores_8b ();
static int32 get_scores4_8b_all ();
static int32 get_scores2_8b_all (int32 *scores, vqFeature_t frm[][MAX_TOPN]);
static int32 get_scores1_8b_all ();
static int32 get_scores_8b_all ();

extern int32 *senone_active;
extern int32 n_senone_active;

static int32 n_phone;
static int32 *psen;		/* psen[p] = #senones in phone p */
static int32 *bestpscr;		/* bestpscr[p] = best senone score for phone p in frame */

/*
 * Compute senone scores for the active senones.  Return best senone score.
 */
int32 SCVQScores (int32 *scores,
		  float *cep, float *dcep, float *dcep_80ms, float *pcep, float *ddcep)
{
    static vqFeature_t f[NUM_FEATURES][MAX_TOPN];
    int	      i, j, best;
    int32     tmp[NUM_FEATURES];

#ifdef WIN32
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
#endif

    cepDist0(f[(int32) CEP_FEAT], cep);
	     
    dcepDist0(f[(int32) DCEP_FEAT], dcep, dcep_80ms);
    
    powDist(f[(int32) POW_FEAT], pcep);

    ddcepDist0(f[(int32) DDCEP_FEAT], ddcep);

    /* normalize the topN feature scores */
    for (j = 0; j < NUM_FEATURES; j++) {
	tmp[j] = f[j][0].val.score;
    }
    for (i = 1; i < topN; i++)
	for (j = 0; j < NUM_FEATURES; j++) {
	    tmp[j] = ADD(tmp[j], f[j][i].val.score);
	}
    for (i = 0; i < topN; i++)
	for (j = 0; j < NUM_FEATURES; j++) {
	    f[j][i].val.score -= tmp[j];
	    if (f[j][i].val.score > 0)
		QUIT((stderr, "%s(%d):  **ERROR** VQ score= %d\n",
		      __FILE__, __LINE__, f[j][i].val.score));
	}
    
#ifdef WIN32
    GetProcessTimes (pid, &t_create, &t_exit, &ket, &uet);
    vq_time += win32_cputime (&ust, &uet);
#endif

    best = SCVQComputeScores(scores, f);
    
#ifdef WIN32
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
    scr_time += win32_cputime (&uet, &ust);
#endif

    return (best);
}


/*
 * Compute scores for all senones.  Compute best senone score per phone into bestpscr.
 * Return bestscore.
 */
int32 SCVQScores_all (int32 *scores,
		      float *cep, float *dcep, float *dcep_80ms, float *pcep, float *ddcep)
{
    static vqFeature_t f[NUM_FEATURES][MAX_TOPN];
    int	      i, j, best;
    int32     tmp[NUM_FEATURES];

#ifdef WIN32
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
#endif

    cepDist0(f[(int32) CEP_FEAT], cep);
	     
    dcepDist0(f[(int32) DCEP_FEAT], dcep, dcep_80ms);
    
    powDist(f[(int32) POW_FEAT], pcep);

    ddcepDist0(f[(int32) DDCEP_FEAT], ddcep);

    /* normalize the topN feature scores */
    for (j = 0; j < NUM_FEATURES; j++) {
	tmp[j] = f[j][0].val.score;
    }
    for (i = 1; i < topN; i++)
	for (j = 0; j < NUM_FEATURES; j++) {
	    tmp[j] = ADD(tmp[j], f[j][i].val.score);
	}
    for (i = 0; i < topN; i++)
	for (j = 0; j < NUM_FEATURES; j++) {
	    f[j][i].val.score -= tmp[j];
	    if (f[j][i].val.score > 0)
		QUIT((stderr, "%s(%d):  **ERROR** VQ score= %d\n",
		      __FILE__, __LINE__, f[j][i].val.score));
	}
    
#ifdef WIN32
    GetProcessTimes (pid, &t_create, &t_exit, &ket, &uet);
    vq_time += win32_cputime (&ust, &uet);
#endif

    best = SCVQComputeScores_all (scores, f);
    
#ifdef WIN32
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
    scr_time += win32_cputime (&uet, &ust);
#endif

    return (best);
}


static int32 SCVQComputeScores(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 	ret;

    if (prob_size == 8) {
	switch (topN) {
	case 4:	 ret = get_scores4_8b(scores, frm); break;
	case 1:	 ret = get_scores1_8b(scores, frm);  break;
	default: ret = get_scores_8b(scores, frm);   break;
	}
    } else {	/* 32 bit PDFs */
	switch (topN) {
	case 4:	 ret = get_scores4(scores, frm); break;
	case 1:	 ret = get_scores1(scores, frm);  break;
	default: ret = get_scores(scores, frm);   break;
	}
    }
    
    return ret;
}


static int32 SCVQComputeScores_all(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 	ret;

    if (prob_size == 8) {
	switch (topN) {
	case 4:	 ret = get_scores4_8b_all(scores, frm); break;
	case 2:	 ret = get_scores2_8b_all(scores, frm); break;
	case 1:	 ret = get_scores1_8b_all(scores, frm);  break;
	default: ret = get_scores_8b_all(scores, frm); break;
	}
    } else {
	switch (topN) {
	case 4:	 ret = get_scores4_all(scores, frm); break;
	case 1:	 ret = get_scores1_all(scores, frm);  break;
	default: ret = get_scores_all(scores, frm);   break;
	}
    }
    
    return ret;
}


static int32 compute_bestpscr (int32 *scrp)
{
    int32 b, i, j, k;
    
    b = (int32) 0x80000000;
    for (i = 0; i < n_phone; i++) {
	k = (int32) 0x80000000;
	for (j = psen[i]; j > 0; --j, scrp++)
	    if (k < *scrp)
		k = *scrp;
	bestpscr[i] = k;
	if (b < k)
	    b = k;
    }
    return (b);
}


static int32 get_scores (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    return (get_scores_all (scores, frm));
}


static int32 get_scores_all (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    long  i, j, k;
    int32 tmp1, tmp2;		/* for distribution score comp. */
    int32 *pdf;			/* current view into output pdfs */
    int32 *scr;			/* observation score */
    int32 *ascr;
    int32 **opdf = OPDF;
    register int32   ts = Table_Size;
    register int16 *at = Addition_Table;
    
    n_senone_active = CdWdPDFMod;
    
    for (j = 0; j < NUM_FEATURES; j++) {
	for (i = 0; i < topN; i++) {
	    frm[j][i].codeword *= CdWdPDFMod;
	}
    }

    for (j = 0; j < NUM_FEATURES; j++) {
       /*
        *  Assumes that codeword is premultiplyed by CdWdPDFMod
        */
	pdf = *opdf + frm[j]->codeword;

	scr = scrPass;
	tmp1 = frm[j]->val.score;
	/*
	 * Compute Score for top 1
	 */
	for (k = CdWdPDFMod; k > 0; k--) {
	    *scr = *pdf + tmp1;
	    scr++; pdf++;
	}
	for (i = 1; i < topN; i++) { /* for each next ranking codeword */
	    /*
             *  Assumes that codeword is premultiplyed by CdWdPDFMod
             */
	    scr = scrPass;
	    tmp1 = (frm[j]+i)->val.score;
	    pdf = *opdf + (frm[j]+i)->codeword;
	    for (k = CdWdPDFMod; k > 0; k--) {
		tmp2 =  *pdf + tmp1;
		FAST_ADD (*scr, *scr, tmp2, at, ts);
		scr++; pdf++;
	    }
	}
	ascr = scores;
	scr  = scrPass;
	if (j == 0) {
	    for (k = CdWdPDFMod; k > 0; k--)
		*ascr++ = *scr++;
	}
	else {
	    for (k = CdWdPDFMod; k > 0; k--, ascr++, scr++) {
		*ascr += *scr;
	    }
	}
	++opdf;
    }

    return (compute_bestpscr (scores));
}


static int32 get_scores4(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    register int32 i, j, k, n;
    int32 tmp1, tmp2;				/* for score comp. */
    int32 *pdf0, *pdf1, *pdf2, *pdf3;		/* pdf pointers */
    int32 *scr;
    int32 w0, w1, w2, w3;			/* weights */
    register int32   ts = Table_Size;
    register int16 *at = Addition_Table;

    for (j = 0; j < NUM_FEATURES; j++) {
	for (k = 0; k < topN; k++) {
	    frm[j][k].codeword *= CdWdPDFMod;
	}
    }

    /*
     *  Assumes that codeword is premultiplyed by CdWdPDFMod
     */
    pdf0 = OPDF[0] + frm[0][0].codeword;
    pdf1 = OPDF[0] + frm[0][1].codeword;
    pdf2 = OPDF[0] + frm[0][2].codeword;
    pdf3 = OPDF[0] + frm[0][3].codeword;
    
    w0 = frm[0][0].val.score;
    w1 = frm[0][1].val.score;
    w2 = frm[0][2].val.score;
    w3 = frm[0][3].val.score;

    scr = scores;
    
    for (i = 0; i < n_senone_active; i++) {
	k = senone_active[i];

	tmp1 = pdf0[k] + w0;
	tmp2 = pdf1[k] + w1;
	FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	tmp2 = pdf2[k] + w2;
	FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	tmp2 = pdf3[k] + w3;
	FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	scr[k] = tmp1;
    }
    
    for (j = 1; j < NUM_FEATURES; j++) {
        /*
         *  Assumes that codeword is premultiplyed by CdWdPDFMod
         */
	pdf0 = OPDF[j] + frm[j][0].codeword;
	pdf1 = OPDF[j] + frm[j][1].codeword;
	pdf2 = OPDF[j] + frm[j][2].codeword;
	pdf3 = OPDF[j] + frm[j][3].codeword;

	w0 = frm[j][0].val.score;
	w1 = frm[j][1].val.score;
	w2 = frm[j][2].val.score;
	w3 = frm[j][3].val.score;

        scr = scores;
	for (k = 0; k < n_senone_active; k++) {
	    n = senone_active[k];

	    tmp1 = pdf0[n] + w0;
	    tmp2 = pdf1[n] + w1;
	    FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	    tmp2 = pdf2[n] + w2;
	    FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	    tmp2 = pdf3[n] + w3;
	    FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	    scr[n] += tmp1;
	}
    }

    /* Find best score */
    k = (int32) 0x80000000;
    for (j = 0; j < n_senone_active; j++) {
	n = senone_active[j];

	if (k < scores[n])
	    k = scores[n];
    }
    
    return (k);
}


static int32 get_scores4_all (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 i; /*, k; */
    
    for (i = 0; i < CdWdPDFMod; i++)
	senone_active[i] = i;
    n_senone_active = CdWdPDFMod;
    
    get_scores4 (scores, frm);

    /* Find best score */
    return (compute_bestpscr (scores));
}


static int32 get_scores1(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    return (get_scores1_all (scores, frm));
}


static int32 get_scores1_all(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 j, k;
    int32 *pdf0, *pdf1, *pdf2, *pdf3;		/* pdf pointers */
    int32 best, b, s, p;
    
    n_senone_active = CdWdPDFMod;

    for (j = 0; j < NUM_FEATURES; j++)
	frm[j][0].codeword *= CdWdPDFMod;
    
    /*
     *  Assumes that codeword is premultiplyed by CdWdPDFMod
     */
    pdf0 = OPDF[0] + frm[0][0].codeword;
    pdf1 = OPDF[1] + frm[1][0].codeword;
    pdf2 = OPDF[2] + frm[2][0].codeword;
    pdf3 = OPDF[3] + frm[3][0].codeword;
    
    best = (int32) 0x80000000;
    k = 0;
    for (p = 0; p < n_phone; p++) {
	b = (int32) 0x80000000;
	for (s = psen[p]; s > 0; --s, k++) {
	    scores[k] = pdf0[k] + pdf1[k] + pdf2[k] + pdf3[k];
	    if (b < scores[k])
		b = scores[k];
	}
	bestpscr[p] = b;
	if (best < b)
	    best = b;
    }
    
    return (best);
}


static int32 get_scores_8b(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    QUIT((stderr, "%s(%d): get_scores_8b() not implemented\n", __FILE__, __LINE__));
}


static int32 get_scores_8b_all(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    QUIT((stderr, "%s(%d): get_scores_8b_all () not implemented\n", __FILE__, __LINE__));
}


#if (! FAST8B)

/*
 * Like get_scores4, but uses OPDF_8B:
 *     LogProb(feature f, codeword c, senone s) =
 *         OPDF_8B[f]->prob[c][OPDF_8B[f]->id[c][s]]
 */
static int32 get_scores4_8b(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    register int32 i, j, k, n;
    int32 tmp1, tmp2;
    int32 *p_cw0, *p_cw1, *p_cw2, *p_cw3;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
    int32 *scr;
    int32 w0, w1, w2, w3;			/* weights */
    register int32   ts = Table_Size;
    register int16 *at = Addition_Table;

    /* Obtain ptrs to senone prob values for the 4 codewords of codebook 0 */
    p_cw0 = OPDF_8B[0]->prob[frm[0][0].codeword];
    p_cw1 = OPDF_8B[0]->prob[frm[0][1].codeword];
    p_cw2 = OPDF_8B[0]->prob[frm[0][2].codeword];
    p_cw3 = OPDF_8B[0]->prob[frm[0][3].codeword];

    /* Similarly, ptrs to senone prob ids */
    pid_cw0 = OPDF_8B[0]->id[frm[0][0].codeword];
    pid_cw1 = OPDF_8B[0]->id[frm[0][1].codeword];
    pid_cw2 = OPDF_8B[0]->id[frm[0][2].codeword];
    pid_cw3 = OPDF_8B[0]->id[frm[0][3].codeword];

    w0 = frm[0][0].val.score;
    w1 = frm[0][1].val.score;
    w2 = frm[0][2].val.score;
    w3 = frm[0][3].val.score;

    scr = scores;
    
    for (i = 0; i < n_senone_active; i++) {
	k = senone_active[i];

	tmp1 = p_cw0[pid_cw0[k]] + w0;
	tmp2 = p_cw1[pid_cw1[k]] + w1;
	FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	tmp2 = p_cw2[pid_cw2[k]] + w2;
	FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	tmp2 = p_cw3[pid_cw3[k]] + w3;
	FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	scr[k] = tmp1;
    }
    
    for (j = 1; j < NUM_FEATURES; j++) {
	/* Ptrs to senone prob values for the 4 codewords of codebook j */
	p_cw0 = OPDF_8B[j]->prob[frm[j][0].codeword];
	p_cw1 = OPDF_8B[j]->prob[frm[j][1].codeword];
	p_cw2 = OPDF_8B[j]->prob[frm[j][2].codeword];
	p_cw3 = OPDF_8B[j]->prob[frm[j][3].codeword];

	/* Similarly, ptrs to senone prob ids */
	pid_cw0 = OPDF_8B[j]->id[frm[j][0].codeword];
	pid_cw1 = OPDF_8B[j]->id[frm[j][1].codeword];
	pid_cw2 = OPDF_8B[j]->id[frm[j][2].codeword];
	pid_cw3 = OPDF_8B[j]->id[frm[j][3].codeword];

	w0 = frm[j][0].val.score;
	w1 = frm[j][1].val.score;
	w2 = frm[j][2].val.score;
	w3 = frm[j][3].val.score;

        scr = scores;
	for (k = 0; k < n_senone_active; k++) {
	    n = senone_active[k];

	    tmp1 = p_cw0[pid_cw0[n]] + w0;
	    tmp2 = p_cw1[pid_cw1[n]] + w1;
	    FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	    tmp2 = p_cw2[pid_cw2[n]] + w2;
	    FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	    tmp2 = p_cw3[pid_cw3[n]] + w3;
	    FAST_ADD (tmp1, tmp1, tmp2, at, ts);
	    scr[n] += tmp1;
	}
    }
    
    /* Find best score */
    k = (int32) 0x80000000;
    for (j = 0; j < n_senone_active; j++) {
	n = senone_active[j];

	if (k < scores[n])
	    k = scores[n];
    }

    return (k);
}


static int32 get_scores4_8b_all (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 i, k;
    
    for (i = 0; i < CdWdPDFMod; i++)
	senone_active[i] = i;
    n_senone_active = CdWdPDFMod;

    get_scores4_8b (scores, frm);

    return (compute_bestpscr (scores));
}

#else

/*
 * Like get_scores4, but uses OPDF_8B:
 *     LogProb(feature f, codeword c, senone s) =
 *         OPDF_8B[f]->prob[c][OPDF_8B[f]->id[c][s]]
 * Also, uses true 8-bit probs, so addition in logspace is an easy lookup.
 */
static int32 get_scores4_8b(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    register int32 i, j, k, n;
    int32 tmp1, tmp2;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
    int32 *scr;
    int32 w0, w1, w2, w3;			/* weights */
    /* register int32   ts = Table_Size;
       register int16 *at = Addition_Table;
       int32 ff, tt, ii; */

    /* ptrs to senone prob ids */
    pid_cw0 = OPDF_8B[0]->id[frm[0][0].codeword];
    pid_cw1 = OPDF_8B[0]->id[frm[0][1].codeword];
    pid_cw2 = OPDF_8B[0]->id[frm[0][2].codeword];
    pid_cw3 = OPDF_8B[0]->id[frm[0][3].codeword];

    w0 = frm[0][0].val.score;
    w1 = frm[0][1].val.score;
    w2 = frm[0][2].val.score;
    w3 = frm[0][3].val.score;

    /* Floor w0..w3 so that when added to senone prob, will not exceed 255<<10 */
    if (w3 < -99000) w3 = -99000;
    if (w2 < -99000) w2 = -99000;
    if (w1 < -99000) w1 = -99000;
    if (w0 < -99000) w0 = -99000;	/* Condition should never be TRUE */

    /* Quantize */
    w3 = (511-w3) >> 10;
    w2 = (511-w2) >> 10;
    w1 = (511-w1) >> 10;
    w0 = (511-w0) >> 10;
    
    scr = scores;
    
    for (i = 0; i < n_senone_active; i++) {
	k = senone_active[i];

	tmp1 = pid_cw0[k] + w0;
	tmp2 = pid_cw1[k] + w1;
	tmp1 = LOG_ADD(tmp1, tmp2);
	tmp2 = pid_cw2[k] + w2;
	tmp1 = LOG_ADD(tmp1, tmp2);
	tmp2 = pid_cw3[k] + w3;
	tmp1 = LOG_ADD(tmp1, tmp2);
	
	scr[k] = -(tmp1 << 10);
    }
    
    for (j = 1; j < NUM_FEATURES; j++) {
	/* ptrs to senone prob ids */
	pid_cw0 = OPDF_8B[j]->id[frm[j][0].codeword];
	pid_cw1 = OPDF_8B[j]->id[frm[j][1].codeword];
	pid_cw2 = OPDF_8B[j]->id[frm[j][2].codeword];
	pid_cw3 = OPDF_8B[j]->id[frm[j][3].codeword];

	w0 = frm[j][0].val.score;
	w1 = frm[j][1].val.score;
	w2 = frm[j][2].val.score;
	w3 = frm[j][3].val.score;

	/* Floor w0..w3 to 256<<10 - 162k */
	if (w3 < -99000) w3 = -99000;
	if (w2 < -99000) w2 = -99000;
	if (w1 < -99000) w1 = -99000;
	if (w0 < -99000) w0 = -99000;	/* Condition should never be TRUE */

	/* Quantize */
	w3 = (511-w3) >> 10;
	w2 = (511-w2) >> 10;
	w1 = (511-w1) >> 10;
	w0 = (511-w0) >> 10;

        scr = scores;
	for (k = 0; k < n_senone_active; k++) {
	    n = senone_active[k];

	    tmp1 = pid_cw0[n] + w0;
	    tmp2 = pid_cw1[n] + w1;
	    tmp1 = LOG_ADD(tmp1, tmp2);
	    tmp2 = pid_cw2[n] + w2;
	    tmp1 = LOG_ADD(tmp1, tmp2);
	    tmp2 = pid_cw3[n] + w3;
	    tmp1 = LOG_ADD(tmp1, tmp2);

	    scr[n] -= tmp1 << 10;
	}
    }

    k = (int32) 0x80000000;
    for (j = 0; j < n_senone_active; j++) {
	n = senone_active[j];
	if (k < scores[n])
	    k = scores[n];
    }
    
    return (k);
}


/*
 * Like get_scores4, but uses OPDF_8B:
 *     LogProb(feature f, codeword c, senone s) =
 *         OPDF_8B[f]->prob[c][OPDF_8B[f]->id[c][s]]
 * Compute all senone scores.
 * Return the best score.
 */
static int32 get_scores4_8b_all (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    register int32 j, k; /*, bestscore; */
    int32 tmp1, tmp2;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;
    int32 *scr;
    int32 w0, w1, w2, w3;			/* weights */
    /* register int32   ts = Table_Size;
       register int16 *at = Addition_Table;
       int32 ff, tt, ii; */

    n_senone_active = CdWdPDFMod;
    
    /* ptrs to senone prob ids */
    pid_cw0 = OPDF_8B[0]->id[frm[0][0].codeword];
    pid_cw1 = OPDF_8B[0]->id[frm[0][1].codeword];
    pid_cw2 = OPDF_8B[0]->id[frm[0][2].codeword];
    pid_cw3 = OPDF_8B[0]->id[frm[0][3].codeword];

    w0 = frm[0][0].val.score;
    w1 = frm[0][1].val.score;
    w2 = frm[0][2].val.score;
    w3 = frm[0][3].val.score;

    /* Floor w0..w3 so that when added to senone prob, will not exceed 255<<10 */
    if (w3 < -99000) w3 = -99000;
    if (w2 < -99000) w2 = -99000;
    if (w1 < -99000) w1 = -99000;
    if (w0 < -99000) w0 = -99000;	/* Condition should never be TRUE */

    /* Quantize */
    w3 = (511-w3) >> 10;
    w2 = (511-w2) >> 10;
    w1 = (511-w1) >> 10;
    w0 = (511-w0) >> 10;
    
    scr = scores;
    for (k = 0; k < CdWdPDFMod; k++) {
	tmp1 = pid_cw0[k] + w0;
	tmp2 = pid_cw1[k] + w1;
	tmp1 = LOG_ADD(tmp1, tmp2);
	tmp2 = pid_cw2[k] + w2;
	tmp1 = LOG_ADD(tmp1, tmp2);
	tmp2 = pid_cw3[k] + w3;
	tmp1 = LOG_ADD(tmp1, tmp2);
	
	scr[k] = -(tmp1 << 10);
    }
    
    for (j = 1; j < NUM_FEATURES; j++) {
	/* ptrs to senone prob ids */
	pid_cw0 = OPDF_8B[j]->id[frm[j][0].codeword];
	pid_cw1 = OPDF_8B[j]->id[frm[j][1].codeword];
	pid_cw2 = OPDF_8B[j]->id[frm[j][2].codeword];
	pid_cw3 = OPDF_8B[j]->id[frm[j][3].codeword];

	w0 = frm[j][0].val.score;
	w1 = frm[j][1].val.score;
	w2 = frm[j][2].val.score;
	w3 = frm[j][3].val.score;

	/* Floor w0..w3 to 256<<10 - 162k */
	if (w3 < -99000) w3 = -99000;
	if (w2 < -99000) w2 = -99000;
	if (w1 < -99000) w1 = -99000;
	if (w0 < -99000) w0 = -99000;	/* Condition should never be TRUE */

	/* Quantize */
	w3 = (511-w3) >> 10;
	w2 = (511-w2) >> 10;
	w1 = (511-w1) >> 10;
	w0 = (511-w0) >> 10;

        scr = scores;
	if (j < NUM_FEATURES-1) {
	    for (k = 0; k < CdWdPDFMod; k++) {
		tmp1 = pid_cw0[k] + w0;
		tmp2 = pid_cw1[k] + w1;
		tmp1 = LOG_ADD(tmp1, tmp2);
		tmp2 = pid_cw2[k] + w2;
		tmp1 = LOG_ADD(tmp1, tmp2);
		tmp2 = pid_cw3[k] + w3;
		tmp1 = LOG_ADD(tmp1, tmp2);
		
		scr[k] -= tmp1 << 10;
	    }
	} else {
	    for (k = 0; k < CdWdPDFMod; k++) {
		tmp1 = pid_cw0[k] + w0;
		tmp2 = pid_cw1[k] + w1;
		tmp1 = LOG_ADD(tmp1, tmp2);
		tmp2 = pid_cw2[k] + w2;
		tmp1 = LOG_ADD(tmp1, tmp2);
		tmp2 = pid_cw3[k] + w3;
		tmp1 = LOG_ADD(tmp1, tmp2);
		
		scr[k] -= tmp1 << 10;
	    }
	}
    }
    
    return (compute_bestpscr(scores));
}


static int32 get_scores2_8b_all (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    register int32 j, k; /* , bestscore; */
    int32 tmp1, tmp2;
    unsigned char *pid_cw0, *pid_cw1;
    int32 *scr;
    int32 w0, w1;			/* weights */
    /* register int32   ts = Table_Size;
       register int16 *at = Addition_Table;
       int32 ff, tt, ii; */

    n_senone_active = CdWdPDFMod;
    
    /* ptrs to senone prob ids */
    pid_cw0 = OPDF_8B[0]->id[frm[0][0].codeword];
    pid_cw1 = OPDF_8B[0]->id[frm[0][1].codeword];

    w0 = frm[0][0].val.score;
    w1 = frm[0][1].val.score;

    /* Floor w0..w3 so that when added to senone prob, will not exceed 255<<10 */
    if (w1 < -99000) w1 = -99000;
    if (w0 < -99000) w0 = -99000;	/* Condition should never be TRUE */

    /* Quantize */
    w1 = (511-w1) >> 10;
    w0 = (511-w0) >> 10;
    
    scr = scores;
    for (k = 0; k < CdWdPDFMod; k++) {
	tmp1 = pid_cw0[k] + w0;
	tmp2 = pid_cw1[k] + w1;
	tmp1 = LOG_ADD(tmp1, tmp2);
	
	scr[k] = -(tmp1 << 10);
    }
    
    for (j = 1; j < NUM_FEATURES; j++) {
	/* ptrs to senone prob ids */
	pid_cw0 = OPDF_8B[j]->id[frm[j][0].codeword];
	pid_cw1 = OPDF_8B[j]->id[frm[j][1].codeword];

	w0 = frm[j][0].val.score;
	w1 = frm[j][1].val.score;

	/* Floor w0..w3 to 256<<10 - 162k */
	if (w1 < -99000) w1 = -99000;
	if (w0 < -99000) w0 = -99000;	/* Condition should never be TRUE */

	/* Quantize */
	w1 = (511-w1) >> 10;
	w0 = (511-w0) >> 10;

        scr = scores;
	if (j < NUM_FEATURES-1) {
	    for (k = 0; k < CdWdPDFMod; k++) {
		tmp1 = pid_cw0[k] + w0;
		tmp2 = pid_cw1[k] + w1;
		tmp1 = LOG_ADD(tmp1, tmp2);
		
		scr[k] -= tmp1 << 10;
	    }
	} else {
	    for (k = 0; k < CdWdPDFMod; k++) {
		tmp1 = pid_cw0[k] + w0;
		tmp2 = pid_cw1[k] + w1;
		tmp1 = LOG_ADD(tmp1, tmp2);
		
		scr[k] -= tmp1 << 10;
	    }
	}
    }
    
    return (compute_bestpscr(scores));
}

#endif


static int32 get_scores1_8b(int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 j, k, bestscore;
    int32 *p_cw0, *p_cw1, *p_cw2, *p_cw3;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;

    /* Ptrs to senone prob values for the top codeword of all codebooks */
    p_cw0 = OPDF_8B[0]->prob[frm[0][0].codeword];
    p_cw1 = OPDF_8B[1]->prob[frm[1][0].codeword];
    p_cw2 = OPDF_8B[2]->prob[frm[2][0].codeword];
    p_cw3 = OPDF_8B[3]->prob[frm[3][0].codeword];

    /* Similarly, ptrs to senone prob ids */
    pid_cw0 = OPDF_8B[0]->id[frm[0][0].codeword];
    pid_cw1 = OPDF_8B[1]->id[frm[1][0].codeword];
    pid_cw2 = OPDF_8B[2]->id[frm[2][0].codeword];
    pid_cw3 = OPDF_8B[3]->id[frm[3][0].codeword];
    
    bestscore = (int32) 0x80000000;
    for (k = 0; k < n_senone_active; k++) {
	j = senone_active[k];

#if (! FAST8B)
	scores[j] = p_cw0[pid_cw0[j]] + p_cw1[pid_cw1[j]] +
	    p_cw2[pid_cw2[j]] + p_cw3[pid_cw3[j]];
#else
	/* ** HACK!! ** <<10 hardwired!! */
	scores[j] = -((pid_cw0[j] + pid_cw1[j] + pid_cw2[j] + pid_cw3[j]) << 10);
#endif
	if (bestscore < scores[j])
	    bestscore = scores[j];
    }
    
    return (bestscore);
}


static int32 get_scores1_8b_all (int32 *scores, vqFeature_t frm[][MAX_TOPN])
{
    int32 p, b, k, bestscore;
    int32 *p_cw0, *p_cw1, *p_cw2, *p_cw3;
    unsigned char *pid_cw0, *pid_cw1, *pid_cw2, *pid_cw3;

    n_senone_active = CdWdPDFMod;
    
    /* Ptrs to senone prob values for the top codeword of all codebooks */
    p_cw0 = OPDF_8B[0]->prob[frm[0][0].codeword];
    p_cw1 = OPDF_8B[1]->prob[frm[1][0].codeword];
    p_cw2 = OPDF_8B[2]->prob[frm[2][0].codeword];
    p_cw3 = OPDF_8B[3]->prob[frm[3][0].codeword];

    /* Similarly, ptrs to senone prob ids */
    pid_cw0 = OPDF_8B[0]->id[frm[0][0].codeword];
    pid_cw1 = OPDF_8B[1]->id[frm[1][0].codeword];
    pid_cw2 = OPDF_8B[2]->id[frm[2][0].codeword];
    pid_cw3 = OPDF_8B[3]->id[frm[3][0].codeword];
    
    bestscore = (int32) 0x80000000;
    for (p = 0; p < n_phone; p++) {
	b = (int32) 0x80000000;
	for (k = psen[p]; k > 0; --k) {
#if (! FAST8B)
	    *scores = p_cw0[*pid_cw0++] + p_cw1[*pid_cw1++] +
		p_cw2[*pid_cw2++] + p_cw3[*pid_cw3++];
#else
	    /* ** HACK!! ** <<10 hardwired!! */
	    *scores = -((*pid_cw0++ + *pid_cw1++ + *pid_cw2++ + *pid_cw3++) << 10);
#endif
	    if (b < *scores)
		b = *scores;
	    scores++;
	}
	bestpscr[p] = b;
	if (bestscore < b)
	    bestscore = b;
    }
    
    return (bestscore);
}


/*
 * Parameter and setup code
 */

#if (FAST8B)

static void quantize_pdfs (int32 f)
{
    int32 s, c, pid, scr, qscr;
    
    log_info ("%s(%d): Quantizing senone PDFs to 8 bits\n", __FILE__, __LINE__);

    for (c = 0; c < NUM_ALPHABET; c++) {
	for (s = 0; s < CdWdPDFMod; s++) {
	    pid = OPDF_8B[f]->id[c][s];
	    scr = OPDF_8B[f]->prob[c][pid];
	    /* ** HACK!! ** hardwired threshold!!! */
	    if (scr < -161900)
		QUIT((stdout, "%s(%d): **ERROR** Too low senone PDF value: %d\n",
		      __FILE__, __LINE__, scr));
	    qscr = (511-scr) >> 10;
	    if ((qscr > 255) || (qscr < 0))
		QUIT((stdout, "%s(%d): scr(%d,%d,%d) = %d\n",
		      __FILE__, __LINE__, f, c, s, scr));
	    OPDF_8B[f]->id[c][s] = (unsigned char) qscr;
	}
    }
}

#endif


/*
 * Read & initialize SC codebooks & output pdfs
 */
int32 SCVQInitFeat(feat_t feat, char *meanPath, char *varPath, int32 *opdf)
{
    int32   *detP;

    assert(((int32)feat < NUM_FEATURES) && ((int32)feat >= 0));
    assert(meanPath != NULL);
    assert(varPath != NULL);
    assert(opdf != NULL);

    if (readMeanCBFile(feat, means + (int32)feat, meanPath) < 0)
	return -1;

    detP = dets[(int32)feat] = detArr + (int32)feat * NUM_ALPHABET;
    if (readVarCBFile(feat, detP, vars + (int32)feat, varPath) < 0) {
	if (feat != POW_FEAT)
	    return -1;
	else {
	    log_debug("Synthesizing power codebook variances\n");
	    if (setPowVar(detP, vars + (int32)feat,
			  (use20ms_diff_pow ? 0.125 : 0.05)) < 0)
		return -1;
	}
    }
    
    if (prob_size == 32)
	OPDF[(int32)feat] = opdf;
    else if (prob_size == 8) {
	OPDF_8B[(int32)feat] = (OPDF_8BIT_T *) opdf;

#if (FAST8B)
	quantize_pdfs ((int32)feat);
#endif
    } else
	QUIT((stderr, "%s(%d): Illegal prob size: %d\n", __FILE__, __LINE__, prob_size));
    
    return 0;
}


/*
 * SCVQSetSenoneCompression:  Must be called before SCVQInitFeat (if senone-probs
 * are compressed).
 */
void
SCVQSetSenoneCompression (int32 size)
{
    if ((size != 8) && (size != 32))
	QUIT((stderr, "%s(%d): Bad #bits/sen-prob: %d\n", __FILE__, __LINE__, size));
    prob_size = size;
}


void SCVQSetdcep80msWeight (double arg)
{
     dcep80msWeight = arg;
}


/*
 * Set senones/phone array
 */
void scvq_set_psen (int32 n, int32 *p)
{
    n_phone = n;
    psen = p;
}


/*
 * Set best phone score array.
 */
void scvq_set_bestpscr (int32 *p)
{
    bestpscr = p;
}
