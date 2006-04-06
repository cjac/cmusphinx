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
 * Revision 1.14  2006/04/06  14:03:02  dhdfu
 * Prevent confusion among future generations by calling this s2_semi_mgau instead of sc_vq
 * 
 * Revision 1.13  2006/04/05 20:27:34  dhdfu
 * A Great Reorganzation of header files and executables
 *
 * Revision 1.12  2006/02/28 02:06:46  egouvea
 * Updated MS Visual C++ 6.0 support files. Fixed things that didn't
 * compile in Visual C++ (declarations didn't match, etc). There are
 * still some warnings, so this is not final. Also, sorted files in
 * several Makefile.am.
 *
 * Revision 1.11  2006/02/23 05:54:58  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added linksilences.  This allows silences to be linked correctly in mode FLAT, TREE.
 * 2, Added s3_am_init: an all-in-one initalization routine SCHMM and CDHMM.
 *
 * Revision 1.10.4.12  2006/01/16 18:25:16  arthchan2003
 * Sphinx 3.x tree decoders assume silences are unlinked (set them to BAD_S3WID) before used. Whereas the flat lexicon decode doesn't have this assumption.  The changes in versions this branch also significantly changed behavior of the decoder. Thus the function LinkSilences is introduced to change back the behavior if necessary.
 *
 * Revision 1.10.4.11  2005/09/26 02:27:22  arthchan2003
 * Fixed bugs when using of -hmm, also fixed a bug when the dictionary is not specified.  Now, it is safe-guard. Notice, this is a case we cannot use RARG in cmd_ln
 *
 * Revision 1.10.4.10  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.10.4.9  2005/09/18 01:29:37  arthchan2003
 * 1, .s3cont. mode is supported.  When it is specified by -senmgau, it will invoke the MS version of GMM computation even for CDHMM. Not supposed to be documented for users. 2, Remove unlinkSilences and put it inside search-specific initialization.  Apparently, remove it entirely will screw up the current test of mode 4 and 5.  add it back will screw up mode 3.  That's why I used temp solution.
 *
 * Revision 1.10.4.8  2005/09/11 23:07:28  arthchan2003
 * srch.c now support lattice rescoring by rereading the generated lattice in a file. When it is operated, silence cannot be unlinked from the dictionary.  This is a hack and its reflected in the code of dag, kbcore and srch. code
 *
 * Revision 1.10.4.7  2005/08/03 19:59:07  arthchan2003
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

    /* Store the word ID for later uses */
    kbc->startwid=lm_startwid(l);
    kbc->finishwid=lm_finishwid(l);

  lm_lmwid2dictwid(l, lm_startwid(l)) = BAD_S3WID;
  lm_lmwid2dictwid(l, lm_finishwid(l)) = BAD_S3WID;

  for (w = dict_startwid(d); IS_S3WID(w); w = dict_nextalt(d, w))
    l->dict2lmwid[w] = BAD_S3LMWID;
  for (w = dict_finishwid(d); IS_S3WID(w); w = dict_nextalt(d, w))
      l->dict2lmwid[w] = BAD_S3LMWID;

}

void linksilences(lm_t* l,kbcore_t *kbc, dict_t *d)
{
  s3wid_t w;

    /* Store the word ID for later uses */

  lm_lmwid2dictwid(l, lm_startwid(l)) = kbc->startwid;
  lm_lmwid2dictwid(l, lm_finishwid(l)) = kbc->finishwid;

  for (w = dict_startwid(d); IS_S3WID(w); w = dict_nextalt(d, w))
    l->dict2lmwid[w] = lm_startwid(l);
  for (w = dict_finishwid(d); IS_S3WID(w); w = dict_nextalt(d, w))
      l->dict2lmwid[w] = lm_finishwid(l);

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

  kbc->mgau=NULL;
  kbc->ms_mgau=NULL;

  if(s3hmmdir && (mdeffile||meanfile||varfile||mixwfile||tmatfile)){
    E_WARN("-s3hmmdir is specified together with (-mdef||-mean||-var||-mix||-tmat). Assume the later overide what -s3hmmdir specified.\n");
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

    if(mdeffile) {
      E_WARN("-mdef overrides <-s3hmmdir>/mdef");
      sprintf(mdefstr,"%s",mdeffile);
    }
    else sprintf(mdefstr,"%s/mdef",s3hmmdir);
    
    if(meanfile) {
      E_WARN("-mean overrides <-s3hmmdir>/means");
      sprintf(meanstr,"%s",meanfile);
    }
    else sprintf(meanstr,"%s/means",s3hmmdir);
    
    if(varfile)  {
      E_WARN("-var overrides <-s3hmmdir>/variances");
      sprintf(meanstr,"%s",varfile);
    }
    else sprintf(varstr,"%s/variances",s3hmmdir);
    
    if(mixwfile)  {
      E_WARN("-mixw overrides <-s3hmmdir>/mixture_weights");
      sprintf(mixwstr,"%s",mixwfile);
    }
    else sprintf(mixwstr,"%s/mixture_weights",s3hmmdir);
    
    if(tmatfile)  {
      E_WARN("-tmat overrides <-s3hmmdir>/transition_matrics");
      sprintf(tmatstr,"%s",tmatfile);
    }
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

  }else if(strcmp(senmgau,".s2semi.") == 0){
    /* SC_VQ initialization. */
    E_INFO("Using Sphinx2 multi-stream GMM computation\n");
    kbc->s2_mgau=s2_semi_mgau_init(meanstr,
				   varstr,varfloor,
				   mixwstr,mixwfloor, 
				   topn);
    if (kbc->mdef && kbc->s2_mgau) {
      /* Verify senone parameters against model definition parameters */
      if (kbc->mdef->n_sen != kbc->s2_mgau->CdWdPDFMod)
	E_FATAL("Mdef #senones(%d) != s2_semi_mgau #PDFs(%d)\n",
		kbc->mdef->n_sen, kbc->s2_mgau->CdWdPDFMod);
    }
    /* FIXME: This should probably move as soon as we support kd-trees
     * for other model types. */
    if (cmd_ln_access("-kdtree")) {
	    if (s2_semi_mgau_load_kdtree(kbc->s2_mgau,
					 cmd_ln_str("-kdtree"),
					 cmd_ln_int32("-kdmaxdepth"),
					 cmd_ln_int32("-kdmaxbbi")) < 0) {
		    E_FATAL("Failed to load kdtrees from %s\n",
			    cmd_ln_str("-kdtree"));
	    }
    }
  }else if(strcmp(senmgau,".semi.") == 0 || strcmp(senmgau,".s3cont.") == 0){

    senone_t* sen;
    /* Multiple stream Gaussian mixture Initialization*/
    E_INFO("Using multi-stream GMM computation\n");
    kbc->ms_mgau=ms_mgau_init(meanstr,
			      varstr,varfloor,
			      mixwstr,mixwfloor, 
			      TRUE, /*Do precomputation */
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
	
	E_INFO("%s\n",senmgau);
	if(strcmp(senmgau,".cont.") == 0) {
	  if (feat_n_stream(kb->fcb) != 1)
	    E_FATAL("#Feature streams(%d) in the feature for continuous HMM!= 1\n", feat_n_stream(kb->fcb));
	}else if(strcmp(senmgau,".semi.") == 0){
	  if (feat_n_stream(kb->fcb) != 4)
	    E_FATAL("#Feature streams(%d) in the feature for semi-continuous HMM!= 4\n", feat_n_stream(kb->fcb));
	}else if(strcmp(senmgau,".s3cont.")==0){

	  E_WARN("Secret Mode .s3cont. is used!!\n");
	}else if(strcmp(senmgau,".s2semi.")==0){

	  E_WARN("Secret Mode .s2semi. is used!!\n");
	}else{
	  E_FATAL("Feature should be either .semi. or .cont.\n");
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
    
    assert(kb->mdef!=NULL);

    if (dictfile) {
	if (! compsep)
	    compsep = "";
	else if ((compsep[0] != '\0') && (compsep[1] != '\0')) {
	    E_FATAL("Compound word separator(%s) must be empty or single character string\n",
		    compsep);
	}
	if ((kb->dict = dict_init (kb->mdef, dictfile, fdictfile, compsep[0],cmd_ln_int32("-lts_mismatch"),REPORT_KBCORE)) == NULL)
	    E_FATAL("dict_init(%s,%s,%s) failed\n", dictfile,
		    fdictfile ? fdictfile : "", compsep);
    }else{
      E_FATAL("Dictionary file is not specified. \n");
    }

    assert(kb->dict!=NULL);
    if(REPORT_KBCORE) {
      dict_report(kb->dict);
    }
    
    if (kb->mgau){
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
    }else{
      E_INFO("Slow GMM computation is used, SVQ and GS will not be used\n");
    }

    /* Two sore points here 
       1, lmset initialization should be handed to the search itself. This will make it 
       parrallel to the FSG code
     */
    if( (lmfile||lmctlfile) && (fsgfile||fsgctlfile)){
      E_FATAL("Only one of the group (-lm|-lmctfile)  or (-fsg|-fsgctlfile) could be specified\n");
    }
    
    if(!(lmfile||lmctlfile) && !(fsgfile||fsgctlfile)){
      E_FATAL("Please specify one of the group (-lm|-lmctfile)  or (-fsg|-fsgctlfile)\n");
    }

    assert(kb->dict);
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

	/*HACK! Only thing that is op_mode-specific. (Hopefully),
	  op-mode name is also hard-wired. Evandro will kill me.  */

	if(cmd_ln_int32("-op_mode")==4||cmd_ln_int32("-op_mode")==5)
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

    assert(kb->tmat);
    /* CHECK: that HMM topology restrictions are not violated */
    if (tmat_chk_1skip (kb->tmat) < 0)
	E_FATAL("Transition matrices contain arcs skipping more than 1 state, not supported in s3.x's decode\n");
    
    /* This should be removed and put into the search */
    if (kb->mdef && kb->dict) {	/* Initialize dict2pid */
	kb->dict2pid = dict2pid_build (kb->mdef, kb->dict,cmd_ln_int32("-composite"));
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
  if(kbcore->lmset){
    lmset_free(kbcore->lmset);
    kbcore->lmset=NULL;
  }
  
  /* Clean up the dictionary stuff*/
  if(kbcore->dict){
    dict_free (kbcore->dict);
    kbcore->dict=NULL;
  }

  /* dict2pid */
  if(kbcore->dict2pid){
    dict2pid_free(kbcore->dict2pid);
    kbcore->dict2pid=NULL;
  }

  /* Clean up the mdef stuff */  
  if(kbcore->mdef){
    mdef_free (kbcore->mdef);
    kbcore->mdef=NULL;
  }

  if(kbcore->fillpen){
    fillpen_free (kbcore->fillpen);
    kbcore->fillpen=NULL;
  }

  if(kbcore->tmat){
    tmat_free (kbcore->tmat);
    kbcore->tmat=NULL;
  }

  if(kbcore->svq){
    subvq_free (kbcore->svq);
    kbcore->svq=NULL;
  }

  if(kbcore->mgau){
    mgau_free (kbcore->mgau);
    kbcore->mgau=NULL;
  }

  if(kbcore->ms_mgau){
    ms_mgau_free (kbcore->ms_mgau);
    kbcore->ms_mgau=NULL;
  }

  /* memory allocated in kbcore*/
  if (kbcore->fcb){
    feat_free (kbcore->fcb);
    kbcore->fcb=NULL;
  }

  /* Free the memory allocated by this module*/
  logs_free();  

  /* Free the object */
  ckd_free ((void *) kbcore);
}
