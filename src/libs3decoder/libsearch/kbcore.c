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
		       char *lmctlfile,
		       char *lmdumpdir,
		       char *fillpenfile,
		       char *senmgau,
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
		       char *gsfile,
		       char *tmatfile,
		       float64 tmatfloor)
{
    kbcore_t *kb;
    int i;
    
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
    kb->n_lm =0;
    kb->n_alloclm=0;
    
    logs3_init (logbase);
    

    if (!feattype){
      E_FATAL("Please specified the feature type\n");
    }
    if (feattype) {
	if ((kb->fcb = feat_init (feattype, cmn, varnorm, agc)) == NULL)
	    E_FATAL("feat_init(%s) failed\n", feattype);
	
	if(strcmp(senmgau,".cont.") == 0) {
	  if (feat_n_stream(kb->fcb) != 1)
	    E_FATAL("#Feature streams(%d) in the feature for continuous HMM!= 1\n", feat_n_stream(kb->fcb));
	}else if(strcmp(senmgau,".semi.") == 0){
	  if (feat_n_stream(kb->fcb) != 4)
	    E_FATAL("#Feature streams(%d) in the feature for semi-continuous HMM!= 4\n", feat_n_stream(kb->fcb));
	}else{
	  E_FATAL("Feature should be either .semi. or .cont.");
	}
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
    
    /* Make option lmfile and lmcftfile to be mutually exclusive */
    if(lmfile && lmctlfile)
      E_FATAL("Please only specify either -lmfile or -lmctlfile\n");
   
    if(!lmfile && !lmctlfile)
      E_FATAL("Please specify either one of -lmfile or -lmctlfile\n");

    if (lmfile) {
	if ((kb->lm = lm_read (lmfile, langwt, inspen, uw)) == NULL)
	    E_FATAL("lm_read(%s, %e, %e, %e) failed\n", lmfile, langwt, inspen, uw);
    }

    if (lmctlfile) {
      E_INFO("Reading LM ctl file\n");
      kb->lmset=lm_read_ctl(lmctlfile,kb->dict,langwt,uw,inspen,lmdumpdir,&(kb->n_lm),&(kb->n_alloclm),dict_size(kb->dict));

      E_INFO("kb->lmset[0].name %s\n",kb->lmset[0].name);
      if(kb->lmset==NULL)
	E_FATAL("lm_read_ctl(%s,%e,%e,%e) failed\n:",lmctlfile,langwt,inspen,uw);

    }
    
    if (fillpenfile || (lmfile && kb->dict) || (lmctlfile && kb->dict)) {
	if (! kb->dict)		/* Sic */
	    E_FATAL("No dictionary for associating filler penalty file(%s)\n", fillpenfile);
	
	if ((kb->fillpen = fillpen_init (kb->dict, fillpenfile, silprob, fillprob,
					 langwt, inspen)) == NULL)
	    E_FATAL("fillpen_init(%s) failed\n", fillpenfile);


    }

    if (kb->dict && kb->lm) {	/* Initialize dict2lmwid */
	if ((kb->dict2lmwid = wid_dict_lm_map (kb->dict, kb->lm,langwt)) == NULL)
	    E_FATAL("Dict/LM word-id mapping failed\n");
    }
    if(kb->dict && kb->lmset) {
      for(i=0;i<kb->n_lm;i++)
	if ((kb->lmset[i].lm->dict2lmwid = wid_dict_lm_map (kb->dict, kb->lmset[i].lm,langwt)) == NULL)
	    E_FATAL("Dict/LM word-id mapping failed for LM index %d, named %s\n",i,kb->lmset[i].name);
    }
    
    if (meanfile) {
	if ((! varfile) || (! mixwfile))
	    E_FATAL("Varfile or mixwfile not specified along with meanfile(%s)\n", meanfile);
	kb->mgau = mgau_init (meanfile, varfile, varfloor, mixwfile, mixwfloor, TRUE,senmgau);
	if (kb->mgau == NULL)
	    E_FATAL("gauden_init(%s, %s, %e) failed\n", meanfile, varfile, varfloor);

	if (subvqfile) {
	    if ((kb->svq = subvq_init (subvqfile, varfloor, -1, kb->mgau)) == NULL)
		E_FATAL("subvq_init (%s, %e, -1) failed\n", subvqfile, varfloor);
	}
	
	if(gsfile) 
	  {
	    if((kb->gs=gs_read(gsfile))==NULL)
	      E_FATAL("gs_read(%s) failed\n",gsfile);

	    E_INFO("After reading the number of senones: %d\n",kb->gs->n_mgau);

	  }
    }
    
    if (tmatfile) {
	if ((kb->tmat = tmat_init (tmatfile, tmatfloor)) == NULL)
	    E_FATAL("tmat_init (%s, %e) failed\n", tmatfile, tmatfloor);
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

/* RAH 4.19.01 free memory allocated within this module */
void kbcore_free (kbcore_t *kbcore)
{
  feat_t *fcb = kbcore_fcb (kbcore); /*  */
  mdef_t *mdef = kbcore_mdef(kbcore);
  dict_t *dict = kbcore_dict (kbcore);
  dict2pid_t *dict2pid = kbcore_dict2pid (kbcore);		/*  */
  /*dictword_t *word;   */
  lm_t *lm = kbcore_lm (kbcore); /*  */


  lm_free (lm);
  
  /* Clean up the dictionary stuff*/
  dict_free (dict);


  /* dict2pid */
  ckd_free ((void *) dict2pid->comwt );
  ckd_free ((void *) dict2pid->comsseq );
  ckd_free ((void *) dict2pid->comstate );
  ckd_free_2d ((void *) dict2pid->single_lc );
  ckd_free_3d ((void ***) dict2pid->ldiph_lc );
  
  /* RAH, this bombs    
     for (i=0;i<dict_size(dict);i++) 
     ckd_free ((void *) dict2pid->internal[i]); 
  */
  ckd_free ((void *) dict2pid->internal);

  /* Clean up the mdef stuff */  
  mdef_free (mdef);

  fillpen_free (kbcore->fillpen);

  tmat_free (kbcore->tmat);
  subvq_free (kbcore->svq);
  mgau_free (kbcore->mgau);

  /* memory allocated in kbcore*/
  if (fcb) {
    ckd_free ((void *)fcb->name);
    ckd_free ((void *)fcb->stream_len);
    ckd_free ((void *)fcb);
  }

  /* Free the memory allocated by this module*/
  logs_free();  
  feat_free (kbcore->fcb);

  /* Free the object */
  ckd_free ((void *) kbcore);
}
