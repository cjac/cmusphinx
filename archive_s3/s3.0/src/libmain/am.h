/*
 * am.h -- Acoustic model evaluation
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
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
