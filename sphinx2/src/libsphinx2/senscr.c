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

/*
 * senscr.c -- 	Senone score computation module.
 * 		Hides details of s2 (semi-continuous) and s3 (continuous)
 * 		models, and computes generic "senone scores".
 *
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2004/12/10  16:48:57  rkm
 * Added continuous density acoustic model handling
 * 
 * 
 * 02-Dec-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added acoustic score weight (applied only to S3 continuous
 * 		acoustic models).
 * 
 * 01-Dec-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added senone active flag related functions.
 * 
 * 20-Nov-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "s2types.h"
#include "s3types.h"
#include "err.h"
#include "ckd_alloc.h"
#include "logs3.h"
#include "cont_mgau.h"
#include "kb.h"
#include "scvq.h"
#include "phone.h"
#include "hmm_tied_r.h"
#include "search.h"
#include "senscr.h"


/*
 * Compute the best senone score from the given array of senone scores.
 * (All senones, not just active ones, should have been already computed
 * elsewhere.)
 * Also, compute the best senone score (CI or CD) per CIphone, and update
 * the corresponding bestpscr array (maintained in the search module).
 * 
 * Return value: the best senone score this frame.
 */
static int32 best_senscr_all (int32 *senscr)
{
  int32 b, i, j, k;
  int32 n_ci;		/* #CI phones */
  int32 *n_psen;	/* #Senones (CI+CD) for each CIphone */
  int32 *bestpscr;	/* Best senone score (CI or CD) for each CIphone */
  
  n_ci = phoneCiCount();
  n_psen = hmm_get_psen();
  bestpscr = search_get_bestpscr();
  
  b = (int32) 0x80000000;
  
  for (i = 0; i < n_ci; i++) {
    k = (int32) 0x80000000;
    
    /* Senones (CI+CD) for CIphone i are in one contiguous block */
    for (j = n_psen[i]; j > 0; --j, senscr++)
      if (k < *senscr)
	k = *senscr;
    
    bestpscr[i] = k;
    
    if (b < k)
      b = k;
  }
  
  return b;
}


/*
 * Like best_senscr_all, but computed only from the active senones in
 * the current frame.  The bestpscr array is not updated since all senone
 * scores are not available.
 * 
 * Return value: the best senone score this frame.
 */
static int32 best_senscr_active (int32 *senscr)
{
  int32 b, i, s;
  
  b = (int32) 0x80000000;

  for (i = 0; i < n_senone_active; i++) {
    s = senone_active[i];
    
    if (b < senscr[s])
      b = senscr[s];
  }
  
  return b;
}


/*
 * Compute s3 feature vector from the given input vectors; note that
 * cep, dcep and ddcep include c0, dc0, and ddc0, which should be
 * omitted.
 */
static void s3feat_build (float32 *s3feat,
			  float32 *cep,
			  float32 *dcep,
			  float32 *pcep,
			  float32 *ddcep)
{
  int32 i, j;
  
  for (i = 1, j = 0; i < CEP_VECLEN; i++, j++)	/* Omit cep[0] */
    s3feat[j] = cep[i];
  for (i = 1; i < CEP_VECLEN; i++, j++)		/* Omit dcep[0] */
    s3feat[j] = dcep[i];
  for (i = 0; i < POW_VECLEN; i++, j++)
    s3feat[j] = pcep[i];
  for (i = 1; i < CEP_VECLEN; i++, j++)		/* Omit ddcep[0] */
    s3feat[j] = ddcep[i];
}


static int32 senscr_compute (int32 *senscr,
			     float32 *cep,
			     float32 *dcep,
			     float32 *dcep_80ms,
			     float32 *pcep,
			     float32 *ddcep,
			     int32 all)
{
  mgau_model_t *g;
  int32 i, s2id, s3id, best;
  int32 *s3senscr;
  float32 *s3feat;
  int32 *s2_s3_senmap;
  int32 ascr_sf;
  
  g = kb_s3model();
  ascr_sf = kb_get_ascr_scale();
  
  if (g != NULL) {	/* Use S3 acoustic model */
    s3senscr = kb_s3senscr();
    s3feat = kb_s3feat();
    s2_s3_senmap = kb_s2_s3_senmap();
    assert (s3senscr != NULL);
    assert (s3feat != NULL);
    assert (s2_s3_senmap != NULL);
    
    /* Build S3 format (single-stream) feature vector */
    s3feat_build (s3feat, cep, dcep, pcep, ddcep);
    
    best = (int32)0x80000000;
    
    if (all) {
      /* Evaluate all senones */
      for (s3id = 0; s3id < mgau_n_mgau(g); s3id++) {
	s3senscr[s3id] = mgau_eval (g, s3id, NULL, s3feat);
	if (ascr_sf != 0)
	  s3senscr[s3id] >>= ascr_sf;
	
	if (best < s3senscr[s3id])
	  best = s3senscr[s3id];
      }
      
      /* Reorder senones to S2 order and normalize scores */
      for (s2id = 0; s2id < mgau_n_mgau(g); s2id++) {
	s3id = s2_s3_senmap[s2id];
	senscr[s2id] = s3senscr[s3id] - best;
      }
    } else {
      /* Evaluate only active senones */
      for (i = 0; i < n_senone_active; i++) {
	s2id = senone_active[i];
	s3id = s2_s3_senmap[s2id];
	
	s3senscr[s3id] = mgau_eval (g, s3id, NULL, s3feat);
	if (ascr_sf != 0)
	  s3senscr[s3id] >>= ascr_sf;
	
	if (best < s3senscr[s3id])
	  best = s3senscr[s3id];
      }
      
      /* Reorder senones to S2 order and normalize scores */
      for (i = 0; i < n_senone_active; i++) {
	s2id = senone_active[i];
	s3id = s2_s3_senmap[s2id];
	
	senscr[s2id] = s3senscr[s3id] - best;
      }
    }
  } else {
    /* S2 (semi-continuous) senone scores */
    if (all) {
      SCVQScores_all(senscr, cep, dcep, dcep_80ms, pcep, ddcep);
    } else {
      SCVQScores(senscr, cep, dcep, dcep_80ms, pcep, ddcep);
    }
  }
  
  if (all) {
    /* Compute best senscore within each CI phone, and overall best */
    return best_senscr_all (senscr);
  } else {
    /* Compute best senscore overall */
    return best_senscr_active (senscr);
  }
}


int32 senscr_all (int32 *senscr,
		  float32 *cep,
		  float32 *dcep,
		  float32 *dcep_80ms,
		  float32 *pcep,
		  float32 *ddcep)
{
  return senscr_compute (senscr, cep, dcep, dcep_80ms, pcep, ddcep, TRUE);
}


int32 senscr_active (int32 *senscr,
		     float32 *cep,
		     float32 *dcep,
		     float32 *dcep_80ms,
		     float32 *pcep,
		     float32 *ddcep)
{
  return senscr_compute (senscr, cep, dcep, dcep_80ms, pcep, ddcep, FALSE);
}


void sen_active_clear ( void )
{
  memset (senone_active_flag, 0, kb_get_total_dists() * sizeof(char));
  n_senone_active = 0;
}


void rhmm_sen_active(ROOT_CHAN_T *rhmm)
{
  int32 s, d, *dist;
  SMD *Models;
  
  Models = kb_get_models();
  
  if (rhmm->mpx) {
    for (s = 0; s < HMM_LAST_STATE; s++) {
      dist = Models[rhmm->sseqid[s]].dist;
      d = dist[s*3];
      senone_active_flag[d] = 1;
    }
  } else {
    dist = Models[rhmm->sseqid[0]].dist;
    for (s = 0; s < TRANS_CNT; s += 3) {
      d = dist[s];
      senone_active_flag[d] = 1;
    }
  }
}


void hmm_sen_active(CHAN_T *hmm)
{
  int32 s, d, *dist;
  SMD *Models;
  
  Models = kb_get_models();
  
  dist = Models[hmm->sseqid].dist;
  for (s = 0; s < TRANS_CNT; s += 3) {
    d = dist[s];
    senone_active_flag[d] = 1;
  }
}


int32 sen_active_flags2list ( void )
{
  int32 i, j, total_dists;
  char *flagptr;
  
  total_dists = kb_get_total_dists();
  
  j = 0;
  for (i = 0, flagptr = senone_active_flag; i < total_dists; i++, flagptr++) {
    if (*flagptr)
      senone_active[j++] = i;
  }
  
  n_senone_active = j;
  
  return j;
}
