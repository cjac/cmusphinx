/*
 * kbcore.c -- Structures for maintain the main models.
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
 * 11-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Removed svqpp stuff.  It doesn't work too well anyway.
 * 
 * 06-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added kb_t.svqpp_t and related handling.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "kbcore.h"
#include "logs3.h"


kbcore_t *kbcore_init (float64 logbase,
		       char *feattype,
		       char *cmn,
		       char *varnorm,
		       char *agc,
		       char *mdeffile,
		       char *dictfile,
		       char *fdictfile,
		       char *compsep,
		       char *lmfile,
		       char *fillpenfile,
		       float64 silprob,
		       float64 fillprob,
		       float64 langwt,
		       float64 inspen,
		       float64 uw,
		       char *meanfile,
		       char *varfile,
		       float64 varfloor,
		       char *mixwfile,
		       float64 mixwfloor,
		       char *subvqfile,
		       char *tmatfile,
		       float64 tmatfloor)
{
    kbcore_t *kb;
    
    E_INFO("Initializing core models:\n");
    
    kb = (kbcore_t *) ckd_calloc (1, sizeof(kbcore_t));
    kb->fcb = NULL;
    kb->mdef = NULL;
    kb->dict = NULL;
    kb->dict2pid = NULL;
    kb->lm = NULL;
    kb->fillpen = NULL;
    kb->dict2lmwid = NULL;
    kb->mgau = NULL;
    kb->svq = NULL;
    kb->tmat = NULL;
    
    logs3_init (logbase);
    
    if (feattype) {
	if ((kb->fcb = feat_init (feattype, cmn, varnorm, agc)) == NULL)
	    E_FATAL("feat_init(%s) failed\n", feattype);
	if (feat_n_stream(kb->fcb) != 1)
	    E_FATAL("#Feature streams(%d) != 1\n", feat_n_stream(kb->fcb));
    }
    
    if (mdeffile) {
	if ((kb->mdef = mdef_init (mdeffile)) == NULL)
	    E_FATAL("mdef_init(%s) failed\n", mdeffile);
    }
    
    if (dictfile) {
	if (! compsep)
	    compsep = "";
	else if ((compsep[0] != '\0') && (compsep[1] != '\0')) {
	    E_FATAL("Compound word separator(%s) must be empty or single character string\n",
		    compsep);
	}
	if ((kb->dict = dict_init (kb->mdef, dictfile, fdictfile, compsep[0])) == NULL)
	    E_FATAL("dict_init(%s,%s,%s) failed\n", dictfile,
		    fdictfile ? fdictfile : "", compsep);
    }
    
    if (lmfile) {
	if ((kb->lm = lm_read (lmfile, langwt, inspen, uw)) == NULL)
	    E_FATAL("lm_read(%s, %e, %e, %e) failed\n", lmfile, langwt, inspen, uw);
    }
    
    if (fillpenfile || (lmfile && kb->dict)) {
	if (! kb->dict)		/* Sic */
	    E_FATAL("No dictionary for associating filler penalty file(%s)\n", fillpenfile);
	
	if ((kb->fillpen = fillpen_init (kb->dict, fillpenfile, silprob, fillprob,
					 langwt, inspen)) == NULL)
	    E_FATAL("fillpen_init(%s) failed\n", fillpenfile);
    }
    
    if (meanfile) {
	if ((! varfile) || (! mixwfile))
	    E_FATAL("Varfile or mixwfile not specified along with meanfile(%s)\n", meanfile);
	kb->mgau = mgau_init (meanfile, varfile, varfloor, mixwfile, mixwfloor, TRUE);
	if (kb->mgau == NULL)
	    E_FATAL("gauden_init(%s, %s, %e) failed\n", meanfile, varfile, varfloor);

	if (subvqfile) {
	    if ((kb->svq = subvq_init (subvqfile, varfloor, -1, kb->mgau)) == NULL)
		E_FATAL("subvq_init (%s, %e, -1) failed\n", subvqfile, varfloor);
	}
    }
    
    if (tmatfile) {
	if ((kb->tmat = tmat_init (tmatfile, tmatfloor)) == NULL)
	    E_FATAL("tmat_init (%s, %e) failed\n", tmatfile, tmatfloor);
    }
    
    if (kb->dict && kb->lm) {	/* Initialize dict2lmwid */
	if ((kb->dict2lmwid = wid_dict_lm_map (kb->dict, kb->lm)) == NULL)
	    E_FATAL("Dict/LM word-id mapping failed\n");
    }
    
    if (kb->mdef && kb->dict) {	/* Initialize dict2pid */
	kb->dict2pid = dict2pid_build (kb->mdef, kb->dict);
    }
    
    /* ***************** Verifications ***************** */
    E_INFO("Verifying models consistency:\n");
    
    if (kb->fcb && kb->mgau) {
	/* Verify feature streams against gauden codebooks */
	if (feat_stream_len(kb->fcb, 0) != mgau_veclen(kb->mgau))
	    E_FATAL("Feature streamlen(%d) != mgau streamlen(%d)\n",
		    feat_stream_len(kb->fcb, 0), mgau_veclen(kb->mgau));
    }
    
    if (kb->mdef && kb->mgau) {
	/* Verify senone parameters against model definition parameters */
	if (kb->mdef->n_sen != mgau_n_mgau(kb->mgau))
	    E_FATAL("Mdef #senones(%d) != mgau #senones(%d)\n",
		    kb->mdef->n_sen, mgau_n_mgau(kb->mgau));
    }
    
    if (kb->mdef && kb->tmat) {
	/* Verify transition matrices parameters against model definition parameters */
	if (kb->mdef->n_tmat != kb->tmat->n_tmat)
	    E_FATAL("Mdef #tmat(%d) != tmatfile(%d)\n", kb->mdef->n_tmat, kb->tmat->n_tmat);
	if (kb->mdef->n_emit_state != kb->tmat->n_state)
	    E_FATAL("Mdef #states(%d) != tmat #states(%d)\n",
		    kb->mdef->n_emit_state, kb->tmat->n_state);
    }
    
    return kb;
}
