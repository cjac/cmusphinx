/*
 * beam.c -- Various forms of pruning beam
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 09-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "beam.h"
#include "logs3.h"


beam_t *beam_init (float64 svq, float64 hmm, float64 ptr, float64 wd)
{
    beam_t *beam;
    
    beam = (beam_t *) ckd_calloc (1, sizeof(beam_t));
    
    beam->subvq = logs3 (svq);
    beam->hmm = logs3 (hmm);
    beam->ptrans = logs3 (ptr);
    beam->word = logs3 (wd);
    
    return beam;
}
