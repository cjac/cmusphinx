/* ====================================================================
 * Copyright (c) 1993-2000 Carnegie Mellon University.  All rights 
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

#include <stdio.h>

#include "s2types.h"
#include "c.h"

static float agc_thresh = 0.2;

static int32
find_peak (int32 const *buf, int32 cnt)
{
   int32	i, max, maxi, maxcnt;
   int32 hyst;	/* histeriesus band */

   maxcnt = 0;
   for (i = 0; i < cnt; i++)
	if (maxcnt < buf[i])
	    maxcnt = buf[i];

   /*
    * Peak must exceed 20% of the max count.
    */
   hyst = 0.20 * maxcnt;
   max = 0;
   maxi = 0;
   for (i = 0; i < cnt; i++) {
  	if (buf[i] > max) {
	   max = buf[i];
	   maxi = i;
	}
	if (buf[i] < (max - hyst))
	    break;
   }
   return (maxi);
}

void
real_agc_noise(float *cep,		/* The cepstrum data */
	       register int32 fcnt,	/* Number of cepstrum frame availiable */
	       register int32 cf_cnt)	/* Number of coeff's per frame */
{
    register float     *p;		   /* Data pointer */
    register float 	min_energy;        /* Minimum log-energy */
    register float 	noise_level;       /* Average noise_level */
    register int32      i;                 /* frame index */
    register int32      noise_frames;      /* Number of noise frames */

    /* Determine minimum log-energy in utterance */
    min_energy = *cep;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
      if (*p < min_energy)
	min_energy = *p;
    }

    /* Average all frames between min_energy and min_energy + agc_thresh */
    noise_frames = 0;
    noise_level = 0.0;
    min_energy += agc_thresh;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
      if (*p < min_energy) {
	noise_level += *p;
	noise_frames++;
      }
    }
    noise_level /= noise_frames;

    printf ("%6.3f = AGC NOISE\n", noise_level);

    /* Subtract noise_level from all log_energy values */
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt)
      *p -= noise_level;
}

void
agc_set_threshold (float threshold)
{
    agc_thresh = threshold;
}

float
histo_noise_level(float const *cep,	/* The cepstrum data */
		  register int32 fcnt,	/* Number of cepstrum frame availiable */
		  register int32 cf_cnt)/* Number of coeff's per frame */
{
    register float const *p;		   /* Data pointer */
    register float 	min_energy;        /* Minimum log-energy */
    register float 	max_energy;        /* Maximum log-energy */
    float		dr;		   /* Dynamic range */
    float	 	noise_level;       /* Average noise_level */
    register int32      i;                 /* frame index */
    int32		histo[101];
    int32		idx;

    /* Determine minimum/maximum log-energy in utterance 
     * Plus a histogram of the energy
     */
    for (idx = 0; idx < 101; idx++)
	histo[idx] = 0;

    min_energy = *cep;
    max_energy = *cep;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
      if (*p < min_energy)
	min_energy = *p;
      else
      if (*p > max_energy)
	max_energy = *p;
    }

    dr = max_energy - min_energy;

    if (dr == 0.0)
	return (min_energy);

    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
      idx = ((*p - min_energy) / dr) * 100.0;
      histo[idx]++;
    }

    idx = 5 + find_peak (histo, 101);
    if (idx > 100)
	idx = 100;
    noise_level = idx;
    noise_level = (noise_level * dr / 100.0) + min_energy;

    printf ("%.3f = Histo noise (%d)\n", noise_level, idx);

    return (noise_level);
}

int32
delete_background (float *cep, int32 fcnt, int32 cf_cnt, double thresh)
{
    int32	i, j;
    float	*p;
    static char delete[8000];

    if (cf_cnt > 8000) {
	printf ("%s(%d): number frames %d, exceeds max (8000)\n",
		__FILE__, __LINE__, fcnt);
	return fcnt;
    }

    /*
     * Mark the frames that are below the threshold
     */
    for (p = cep, i = 0; i < fcnt; i++, p += cf_cnt) {
	delete[i] = (*p < thresh) ? 1 : 0;
    }

    /*
     * Select the frames to delete
     */
    for (i = 2; i < fcnt-2; i++) {
	if (delete[i-2] &&
	    delete[i-1] &&
	    delete[i]   &&
	    delete[i+1] &&
	    delete[i+2])
	{
	    	delete[i] = 2;
	}
    }

    /*
     * Delete the frames
     */
    for (j = i = 0; i < fcnt; i++) {
	if (delete[i] != 2) {
	    memcpy (&cep[j*cf_cnt], &cep[i*cf_cnt], cf_cnt * sizeof(float));
	    j++;
	}
    }
    printf ("%s(%d): Deleted %d background frames out of %d frames\n",
	    __FILE__, __LINE__, fcnt-j, fcnt);
    return (j);
}

void
agc_max(float *cep,		/* The cepstrum data */
	register int32 fcnt,	/* Number of cepstrum frame availiable */
	register int32 cf_cnt)	/* Number of coeff's per frame */
{
    register float     *p;		   /* Data pointer */
    register float 	max_energy;        /* Minimum log-energy */
    register int32      i;                 /* frame index */

    /* Determine max log-energy in utterance */
    max_energy = *cep;
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt) {
      if (*p > max_energy)
	max_energy = *p;
    }

    printf ("%6.3f = AGC MAX\n", max_energy);

    /* Subtract max_energy from all log_energy values
     */
    for (i = 0, p = cep; i < fcnt; i++, p += cf_cnt)
      *p -= max_energy;
}

static float observed_min = 100.0;
static float observed_max = -100.0;
static float observed_dr;
static float min_energy = -20.0;
static float max_energy =  20.0;
static float dynamic_range = 40.0;
static int32 histo[1001];
static float noise_level = -100.0;       /* Average noise_level */
static int32 noise_frm_cnt = 0;
static int32 noise_frames_discarded = 0;

int32 histo_add_c0 (float c0)
{
    int32		idx;

    if (c0 < noise_level)
	noise_frm_cnt++;
    else
	noise_frm_cnt = 0;

    observed_min = MIN(c0,observed_min);
    observed_max = MAX(c0,observed_max);

    c0 = MAX (min_energy, c0);
    c0 = MIN (max_energy, c0);
    c0 -= min_energy;

    idx = (c0 / dynamic_range) * 1000.0;
    histo[idx]++;

    if (noise_frm_cnt < 5)
	return TRUE;
    else {
	noise_frames_discarded++;
	return FALSE;
    }
}


void compute_noise_level (void)
{
    int32		idx;
    int32		i;

    idx = find_peak (histo, 1001);
    /*
     * Discard half of the counts
     */
    for (i = 0; i < 1001; i++)
	histo[i] = histo[i] >> 1;
    /*
     * compute the observed dynamic range and contract the observed min and
     * max by 10% each. Reset noise_frm_cnt to 0.
     */
    observed_dr = observed_max - observed_min;
    observed_min += observed_dr / 10.0;
    observed_max -= observed_dr / 10.0;
    noise_frm_cnt = 0;

    /*
     * Compute the noise level as 5% of the dr above the first peak.
     */
    noise_level = idx;
    noise_level = (noise_level * dynamic_range / 1000.0) + min_energy;
    noise_level += (observed_dr * 0.05);

    printf ("%.3f = Histo noise (%d)\n", noise_level, idx);
    printf ("%d Frames discarded\n", noise_frames_discarded);
    noise_frames_discarded = 0;
}
