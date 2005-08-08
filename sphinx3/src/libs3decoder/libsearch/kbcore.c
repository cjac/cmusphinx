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
 * $Log$
 * Revision 1.10.4.7  2005/08/03  19:59:07  arthchan2003
 * Added a message to tell user which GMM computation are being used.
 * 
 * Revision 1.10.4.6  2005/08/03 18:54:32  dhdfu
 * Fix the support for multi-stream / semi-continuous models.  It is
 * still kind of a hack, but it now works.
 *
 * Revision 1.10.4.5  2005/08/02 21:33:47  arthchan2003
 * Factored the code of initializing one hmm into s3_am_init. That is to say initialization of mdef, mgau, var, mixw and tmat could all be found one function.
 *
 * Revision 1.10.4.4  2005/07/20 21:19:52  arthchan2003
 * Added options such that finite state grammar option is now accepted.
 *
 * Revision 1.10.4.3  2005/07/13 01:59:35  arthchan2003
 * Remove fcb variable in kbcore.c
 *
 * Revision 1.10.4.2  2005/07/03 23:02:39  arthchan2003
 * Wrapped up feat_t freeing into feat_free.
 *
 * Revision 1.10.4.1  2005/06/28 19:10:06  arthchan2003
 * Add fillpen_report in kbcore.c
 *
 * Revision 1.10  2005/06/21 23:28:48  arthchan2003
 * Log. Please also see comments of kb.[ch].  Major changes you could see
 * is that the lmset interface is now used rather than several interfaces
 * for reading lm. Other than that, you could say most changes are
 * harmless internal interfaces changes.
 *
 * Revision 1.11  2005/06/20 22:20:18  archan
 * Fix non-conforming problems for Windows plot.
 *
 * Revision 1.10  2005/06/18 03:22:29  archan
 * Add lmset_init. A wrapper function of various LM initialization and initialize an lmset It is now used in decode, livepretend, dag and astar.
 *
 * Revision 1.9  2005/06/17 23:44:40  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.8  2005/06/13 22:30:30  archan
 * Bug fix: In lmset_read_lm and lmset_read_ctl, the arguments of insertion penalty and unigram weight was switched.  This problem didn't shows up in standard regression test where the language model is very weak.  It only caused a 1 answer difference in ti46. However, it gives a very obvious effect in tidigits and Communicator and caused a lot of deletions.  This is now fixed.
 *
 * Revision 1.7  2005/05/27 01:15:44  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.6  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.5  2005/04/20 03:38:43  archan
 * Do the corresponding code changes for the lm code.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
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
#include "s3types.h"
#define REPORT_KBCORE 1 


void checkLMstartword(lm_t* l,char* name)
{
  if (NOT_S3LMWID(lm_startwid(l)) || NOT_S3LMWID(lm_finishwid(l))){
    E_FATAL("%s or %s not in LM %s\n", S3_START_WORD, S3_FINISH_WORD,name);
  }
}

/*
 * Unlink <s> and </s> between dictionary and LM, to prevent their 
 * recognition.  They are merely dummy words (anchors) at the beginning 
 * and end of each utterance.
 */
/* This has to be done before tree is built */
void unlinksilences(lm_t* l,kbcore_t *kbc, dict_t *d)
{
  s3wid_t w;

  lm_lmwid2dictwid(l, lm_startwid(l)) = BAD_S3WID;
  lm_lmwid2dictwid(l, lm_finishwid(l)) = BAD_S3WID;


  for (w = dict_startwid(d); IS_S3WID(w); w = dict_nextalt(d, w))
    l->dict2lmwid[w] = BAD_S3LMWID;
  for (w = dict_finishwid(d); IS_S3WID(w); w = dict_nextalt(d, w))
      l->dict2lmwid[w] = BAD_S3LMWID;

}

/**
 * Initialize sphinx 3 single stream hmm (use cont_mgau). This
 * function is duplicated with ms_s3_am_init. 
 */
void s3_am_init(kbcore_t *kbc,
		char *s3hmmdir,
		char *mdeffile,
		char *meanfile,
		char *varfile,
		float64 varfloor,
		char *mixwfile,
		float64 mixwfloor,
		char *tmatfile,
		float64 tmatfloor,
		char *senmgau,
		char *lambdafile,
		int32 topn
		)
{

  char mdefstr[2048];
  char meanstr[2048];
  char varstr[2048];
  char mixwstr[2048];
  char tmatstr[2048];

  assert(kbc);
  if(s3hmmdir && (mdeffile||meanfile||varfile||mixwfile||tmatfile)){
    E_WARN("-s3hmmdir is specified together with (-mdef||-mean||-var||-mix||-tmat). Assume the later overide what -s3hmmdir specified.");
  }

  /** Determine the location of the resources. 
      If -s3hmmdir if not specified, -mdef, -mean, -var, -mixw and
      -tmat will be used.

      If only -s3hmmdir is specified, <-s3hmmdir>/means,
      <s3hmmdir>/variances, <s3hmmdir>/mixture_weights,
      <s3hmmdir>/transition_matrices, <s3hmmdir>/mdef

      . Please do not generalize it because seldom people touch the
      name of the models generated from the trainer. 

      If -s3hmmdir and (-mdef, -mean, -var, -mixw, -tmat) are
      specified, the later will override the -s3hmmdir decision. 

   */

  if(!s3hmmdir){
    if(!mdeffile) 
      E_FATAL("Please specified either -s3hmmdir or -mdef\n");
    else 
      sprintf(mdefstr,"%s",mdeffile);

    if(!meanfile) 
      E_FATAL("Please specified either -s3hmmdir or -mean\n");
    else 
      sprintf(meanstr,"%s",meanfile);

    if(!varfile)  
      E_FATAL("Please specified either -s3hmmdir or -var\n");
    else 
      sprintf(varstr ,"%s",varfile);

    if(!mixwfile) 
      E_FATAL("Please specified either -s3hmmdir or -mixw\n");
    else 
      sprintf(mixwstr,"%s",mixwfile);

    if(!tmatfile) 
      E_FATAL("Please specified either -s3hmmdir or -tmat\n");
    else 
      sprintf(tmatstr,"%s",tmatfile);
  }else{

    if(mdeffile) E_WARN("-mdef overrides <-s3hmmdir>/mdef");
    else sprintf(mdefstr,"%s/mdef",s3hmmdir);
    
    if(meanfile) E_WARN("-mean overrides <-s3hmmdir>/means");
    else sprintf(meanstr,"%s/means",s3hmmdir);
    
    if(varfile)  E_WARN("-var overrides <-s3hmmdir>/variances");
    else sprintf(varstr,"%s/variances",s3hmmdir);
    
    if(mixwfile)  E_WARN("-mixw overrides <-s3hmmdir>/mixture_weights");
    else sprintf(mixwstr,"%s/mixture_weights",s3hmmdir);
    
    if(tmatfile)  E_WARN("-tmat overrides <-s3hmmdir>/transition_matrics");
    else sprintf(tmatstr,"%s/transition_matrices",s3hmmdir);

  }


  E_INFO_NOFN("Reading HMM in Sphinx 3 Model format\n");
  E_INFO_NOFN("Model Definition File: %s\n",mdefstr);
  E_INFO_NOFN("Mean File: %s\n",meanstr);
  E_INFO_NOFN("Variance File: %s\n",varstr);
  E_INFO_NOFN("Mixture Weight File: %s\n",mixwstr);
  E_INFO_NOFN("Transition Matrices File: %s\n",tmatstr);


  if ((kbc->mdef = mdef_init (mdefstr, REPORT_KBCORE)) == NULL)
    E_FATAL("mdef_init(%s) failed\n", mdefstr);

  if(REPORT_KBCORE){
    mdef_report(kbc->mdef);
  }

  if(strcmp(senmgau,".cont.") == 0) {
    /* Single stream optmized GMM computation Initialization */
    E_INFO("Using optimized GMM computation for Continuous HMM, -topn will be ignored\n");
    kbc->mgau = mgau_init (meanstr, 
			 varstr, varfloor, 
			 mixwstr, mixwfloor, 
			 TRUE,  /* Do precomputation*/
			 senmgau,
			 MIX_INT_FLOAT_COMP); /*Use hybrid integer and float routine */

    if (kbc->mdef && kbc->mgau) {
      /* Verify senone parameters against model definition parameters */
      if (kbc->mdef->n_sen != mgau_n_mgau(kbc->mgau))
	E_FATAL("Mdef #senones(%d) != mgau #senones(%d)\n",
		kbc->mdef->n_sen, mgau_n_mgau(kbc->mgau));
    }

  }else if(strcmp(senmgau,".semi.") == 0 || strcmp(senmgau,".s3cont.") == 0){

    /* Multiple stream Gaussian mixture Initialization*/
    E_INFO("Using multi-stream GMM computation\n");
    senone_t* sen;
    kbc->ms_mgau=ms_mgau_init(meanstr,
			      varstr,varfloor,
			      mixwstr,mixwfloor, 
			      senmgau,
			      lambdafile, /* lambda is not a standard option, so user need to specify it*/
			      topn
			      );

    sen=ms_mgau_senone(kbc->ms_mgau);

    /* Verify senone parameters against model definition parameters */
    if (kbc->mdef->n_sen != sen->n_sen)
	E_FATAL("Model definition has %d senones; but #senone= %d\n",
		kbc->mdef->n_sen, sen->n_sen);
  }else {
    E_FATAL("Feature should be either .semi. or .cont.");
  }


  /* STRUCTURE: Initialize the transition matrices */
  if ((kbc->tmat = tmat_init (tmatstr, tmatfloor, REPORT_KBCORE)) == NULL)
    E_FATAL("tmat_init (%s, %e) failed\n", tmatstr, tmatfloor);
  
  if(REPORT_KBCORE){
    tmat_report(kbc->tmat);
  }
  
  if (kbc->mdef && kbc->tmat) {
    /* Verify transition matrices parameters against model definition parameters */
    if (kbc->mdef->n_tmat != kbc->tmat->n_tmat)
      E_FATAL("Mdef #tmat(%d) != tmatfile(%d)\n", kbc->mdef->n_tmat, kbc->tmat->n_tmat);
    if (kbc->mdef->n_emit_state != kbc->tmat->n_state)
      E_FATAL("Mdef #states(%d) != tmat #states(%d)\n",
	      kbc->mdef->n_emit_state, kbc->tmat->n_state);
  }

}

kbcore_t *New_kbcore()
{
  kbcore_t *kbc;
  kbc = (kbcore_t *) ckd_calloc (1, sizeof(kbcore_t));
  return kbc;
}
	
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
		       char *fsgfile,
		       char *fsgctlfile,
		       char *fillpenfile,
		       char *senmgau,
		       float64 silprob,
		       float64 fillprob,
		       float64 langwt,
		       float64 inspen,
		       float64 uw,
		       char *s3hmmdir,
		       char *meanfile,
		       char *varfile,
		       float64 varfloor,
		       char *mixwfile,
		       float64 mixwfloor,
		       char *subvqfile,
		       char *gsfile,
		       char *tmatfile,
		       float64 tmatfloor
		       )
{
    kbcore_t *kb;
    int i;
    s3cipid_t sil;
    
    E_INFO("Begin Initialization of Core Models:\n");
    
    kb = (kbcore_t *) ckd_calloc (1, sizeof(kbcore_t));
    kb->fcb = NULL;
    kb->mdef = NULL;
    kb->dict = NULL;
    kb->dict2pid = NULL;
    kb->fillpen = NULL;

    kb->mgau = NULL;
    kb->svq = NULL;
    kb->tmat = NULL;

    
    if(!logs3_init (logbase, REPORT_KBCORE,cmd_ln_int32 ("-log3table")))
      E_FATAL("Error in logs3_init, exit\n");

    if(REPORT_KBCORE){
      logs3_report();
    }

    if (!feattype)
      E_FATAL("Please specified the feature type\n");
    
    if (feattype) {
	if ((kb->fcb = feat_init (feattype, cmn, varnorm, agc, REPORT_KBCORE)) == NULL)
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
    
    if(REPORT_KBCORE){
      feat_report(kb->fcb);
    }

    /* Initialize sphinx 3 hmm */
    s3_am_init(kb,
	       s3hmmdir,
	       mdeffile,
	       meanfile,
	       varfile,
	       varfloor,
	       mixwfile,
	       mixwfloor,
	       tmatfile,
	       tmatfloor,
	       senmgau,
	       NULL,
	       cmd_ln_int32("-topn") /* ARRRGH!! */
	       );
    
    if (dictfile) {
	if (! compsep)
	    compsep = "";
	else if ((compsep[0] != '\0') && (compsep[1] != '\0')) {
	    E_FATAL("Compound word separator(%s) must be empty or single character string\n",
		    compsep);
	}
	if ((kb->dict = dict_init (kb->mdef, dictfile, fdictfile, compsep[0],REPORT_KBCORE)) == NULL)
	    E_FATAL("dict_init(%s,%s,%s) failed\n", dictfile,
		    fdictfile ? fdictfile : "", compsep);
    }
   
    if(REPORT_KBCORE) {
      dict_report(kb->dict);
    }
    
    if (meanfile){
      if(subvqfile && gsfile){
	E_FATAL("Currently there is no combination scheme of gs and svq in Gaussian Selection\n");
      }
      if (subvqfile) {
	if ((kb->svq = subvq_init (subvqfile, varfloor, -1, kb->mgau)) == NULL)
	  E_FATAL("subvq_init (%s, %e, -1) failed\n", subvqfile, varfloor);
      }
      
      if(gsfile) {
	if((kb->gs=gs_read(gsfile))==NULL)
	  E_FATAL("gs_read(%s) failed\n",gsfile);
	
	E_INFO("After reading the number of senones: %d\n",kb->gs->n_mgau);
      }  
      /* SVQ and GS-specific checking should be done */
    }

    /* Two sore points here 
       1, lmset initialization should be handed to the search itself. This will make it 
       parrallel to the FSG code
       2, 
     */
    if( (lmfile||lmctlfile) && (fsgfile||fsgctlfile)){
      E_FATAL("Only one of the group (-lm|-lmctfile)  or (-fsg|-fsgctlfile) could be specified\n");
    }
    
    if(!(lmfile||lmctlfile) && !(fsgfile||fsgctlfile)){
      E_FATAL("Please specify one of the group (-lm|-lmctfile)  or (-fsg|-fsgctlfile)\n");
    }

    if(lmfile||lmctlfile){
      kb->lmset=lmset_init(lmfile,
			   lmctlfile,
			   cmd_ln_str("-ctl_lm"), /* This two are ugly.*/
			   cmd_ln_str("-lmname"),
			   lmdumpdir,
			   langwt,
			   inspen,
			   uw,
			   kb->dict);
      
      /* CHECK: check whether LM has a start word and end word 
	 Also make sure silences are unlinked. */
      for(i=0;i<kb->lmset->n_lm;i++){
	checkLMstartword(kb->lmset->lmarray[i],lmset_idx_to_name(kb->lmset,i));
	unlinksilences(kb->lmset->lmarray[i],kb,kb->dict);
      }

    }else if (fsgfile||fsgctlfile){
       E_INFO("kbcore will not let the srch_fsg take care of initialization of the search. Hand it to srch_fsg.");
    }
    
    if (fillpenfile ||kb->dict) {
	if (! kb->dict)		/* Sic */
	    E_FATAL("No dictionary for associating filler penalty file(%s)\n", fillpenfile);
	
	if ((kb->fillpen = fillpen_init (kb->dict, fillpenfile, silprob, fillprob,
					 langwt, inspen)) == NULL)
	    E_FATAL("fillpen_init(%s) failed\n", fillpenfile);
    }

    if(REPORT_KBCORE){
      fillpen_report(kb->fillpen);
    }
    
    /* CHECK: that HMM topology restrictions are not violated */
    if (tmat_chk_1skip (kb->tmat) < 0)
	E_FATAL("Transition matrices contain arcs skipping more than 1 state, not supported in s3.x's decode\n");
    
    /* This should be removed and put into the search */
    if (kb->mdef && kb->dict) {	/* Initialize dict2pid */
	kb->dict2pid = dict2pid_build (kb->mdef, kb->dict);
    }
    
    if(REPORT_KBCORE){
      dict2pid_report(kb->dict2pid);
    }
    /* ***************** Verifications ***************** */
    if(REPORT_KBCORE)
      E_INFO("Inside kbcore: Verifying models consistency ...... \n");
    
    if (kb->fcb && kb->mgau) {
	/* Verify feature streams against gauden codebooks */
	if (feat_stream_len(kb->fcb, 0) != mgau_veclen(kb->mgau))
	    E_FATAL("Feature streamlen(%d) != mgau streamlen(%d)\n",
		    feat_stream_len(kb->fcb, 0), mgau_veclen(kb->mgau));
    }
    

    /* CHECK: Verify whether the <s> and </s> is in the dictionary. */
    if (NOT_S3WID(dict_startwid(kb->dict)) || NOT_S3WID(dict_finishwid(kb->dict)))
	E_FATAL("%s or %s not in dictionary\n", S3_START_WORD, S3_FINISH_WORD);

    /* CHECK: Verify whether <sil> is in the dictionary */
    sil = mdef_silphone (kbcore_mdef (kb));
    if (NOT_S3CIPID(sil))
      E_FATAL("Silence phone '%s' not in mdef\n", S3_SILENCE_CIPHONE);


    E_INFO("End of Initialization of Core Models:\n");
    return kb;
}

/* RAH 4.19.01 free memory allocated within this module */
void kbcore_free (kbcore_t *kbcore)
{
  mdef_t *mdef = kbcore_mdef(kbcore);
  dict_t *dict = kbcore_dict (kbcore);
  dict2pid_t *dict2pid = kbcore_dict2pid (kbcore);		/*  */
  lmset_t *lms = kbcore_lmset(kbcore);

  if(lms)
    lmset_free(lms);
  
  /* Clean up the dictionary stuff*/
  if(dict)
    dict_free (dict);

  /* dict2pid */
  if(dict2pid){
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

  }
  

  /* Clean up the mdef stuff */  
  if(mdef)
    mdef_free (mdef);

  if(kbcore->fillpen)
    fillpen_free (kbcore->fillpen);

  if(kbcore->tmat)
    tmat_free (kbcore->tmat);

  if(kbcore->svq)
    subvq_free (kbcore->svq);

  if(kbcore->mgau)
    mgau_free (kbcore->mgau);

  if(kbcore->ms_mgau)
    ms_mgau_free (kbcore->ms_mgau);

  /* memory allocated in kbcore*/
  if (kbcore->fcb) {
    feat_free (kbcore->fcb);
  }

  /* Free the memory allocated by this module*/
  logs_free();  

  /* Free the object */
  ckd_free ((void *) kbcore);
}
