/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * am.h -- Acoustic model evaluation
 *
 * 
 * HISTORY
 * 
 * 22-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_AM_H_
#define _LIBMAIN_AM_H_


#include <libfeat/libfeat.h>

#include "gauden.h"
#include "senone.h"


/*
 * Data structures needed to evaluate acoustic models in each frame (for a given
 * acoustic model.)
 */
typedef struct acoustic_s {
    feat_t *fcb;	/* Feature-type descriptor */
    gauden_t *gau;	/* READ-ONLY; FOR INTERNAL USE ONLY */
    senone_t *sen;	/* READ-ONLY; FOR INTERNAL USE ONLY */
    float32 ***feat;	/* Space for input speech feature vector */
    float32 **mfc;	/* Space for input speech MFC data */
    int32 *dist;	/* Density values in a single mixture Gaussian */
    int32 mgaubeam;	/* (base-logs3).  Beamwidth for pruning low-scoring components
			   within a mixture Gaussian. */
    int32 *dist_valid;	/* Indices of density values (in dist[]) that are above the
			   pruning threshold (mgaubeam) */
    int32 n_dist_valid;	/* #Valid entries in dist_valid[] */
    int32 *senscr;	/* Senone score for each senone */
    int32 *senscale;	/* senscale[f] = scaling applied to senone scores in frame f */
    bitvec_t sen_active;	/* Boolean flags for senones active in current frame;
				   set (TRUE) iff active */
    bitvec_t gauden_active;	/* Booleans for which mixtures active in current frame */
    float64 tot_dist_valid;	/* For active mgau statistics */
    int32 tot_mgau_eval;	/* For active mgau statistics */
} acoustic_t;


/*
 * Initialize a structure containing intermediate data used in acoustic model evaluation.
 * Return value: created object if successful, NULL otherwise.
 */
acoustic_t *acoustic_init (feat_t *f,	/* In: Feature-type descriptor */
			   gauden_t *g,	/* In: Mixture Gaussian density codebooks */
			   senone_t *s,	/* In: Senones */
			   float64 beam,	/* In: (See acoustic_t.mgaubeam) Use
						   1.0 for top-1 and 0.0 for selecting
						   all components (infinite beamwidth) */
			   int32 maxfr);	/* In: Max #frames / utterance */

/*
 * Compute the normalized acoustic model (senone) scores for the given mfc data.
 * The normalization is done by subtracting the best raw score from all.
 * Return value: The best raw senone score, i.e. the normalization factor.
 * NOTE: am->senscale is NOT updated with this return value; the caller should do this.
 */
int32 acoustic_eval (acoustic_t *am,	/* In/Out: On return am->senscr contains the
					   senone scores (output likelihoods). */
		     int32 frm);	/* In: Frame for which evaluation is to be done.
					   NOTE: If this object needs to compute features
					   from MFC cepstra, neighboring frames might be
					   needed for that purpose. */

#endif
