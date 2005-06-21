/* ====================================================================
 * Copyrightgot (c) 1999-2004 Carnegie Mellon University.  All rights
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
/************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 ************************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.26  2005/06/21  23:21:58  arthchan2003
 * Log. This is a big refactoring for kb.c and it is worthwhile to give
 * words on why and how things were done.  There were generally a problem
 * that the kb structure itself is too flat.  That makes it has to
 * maintained many structure that could be maintained by smaller
 * structures.  For example, the count of A and the array of A should
 * well be put into the same structure to increase readability and
 * modularity. One can explain why histprune_t, pl_t, stat_t and
 * adapt_am_t were introduced with that line of reasoning.
 * 
 * In srch_t, polymorphism of implementation is also one important
 * element in separting all graph related members from kb_t to srch_t.
 * One could probably implement the polymorphism as an interface of kb
 * but it is not trivial from the semantic meaning of kb.  That is
 * probably why srch_t is introduced as the gateway of search interfaces.
 * 
 * Another phenonemon one could see in the code was bad interaction
 * between modules. This is quite serious in two areas: logging and
 * checking. The current policy is unless something required cross
 * checking two structures, they would be done internally inside a module
 * initialization.
 * 
 * Finally, kb_setlm is now removed and is replaced by ld_set_lm (by
 * users) or srch_set_lm (by developers). I think this is quite
 * reasonable.
 * 
 * Revision 1.14  2005/06/19 19:41:23  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.13  2005/06/10 03:01:50  archan
 * Fixed file_open.
 *
 * Revision 1.12  2005/05/26 00:46:59  archan
 * Added functionalities that such that <sil> will not be inserted at the end of the utterance.
 *
 * Revision 1.11  2005/05/04 05:15:25  archan
 * reverted the last change, seems to be not working because of compilation issue. Try not to deal with it now.
 *
 * Revision 1.10  2005/05/04 04:46:04  archan
 * Move srch.c and srch.h to search. More and more this type of refactoring will be done in future
 *
 * Revision 1.9  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.8  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.7  2005/04/20 03:36:18  archan
 * Remove setlm from kb entirely, refactor it to search implementations, do the corresponding change for the changes in ascr and pl
 *
 * Revision 1.6  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 30-Dec-2000	Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Moved kb_*() routines into kb.c to make them independent of
 *		main() during compilation
 *
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to allow runtime choice between 3-state and 5-state HMM
 * 		topologies (instead of compile-time selection).
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxwpf.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "kb.h"
#include "logs3.h"		/* RAH, added to resolve log3_free */
#include "srch.h"


#define REPORT_KB 1


/* 20050321 Duplicated function. can also be io.c. Clean it up later. */
FILE* file_open(char* filepath)
{
  FILE *fp;
  fp=NULL;
  if (filepath) {
#ifdef WIN32
    if ((fp= fopen(filepath, "wt")) == NULL)
#else
    if ((fp= fopen(filepath, "w")) == NULL)
#endif
      E_ERROR("fopen(%s,w) failed; use FWDXCT: from std logfile\n", filepath);
  }
  return fp;
}


/*ARCHAN, to allow backward compatibility -lm, -lmctlfn coexists. This makes the current implmentation more complicated than necessary. */
void kb_init (kb_t *kb)
{
    kbcore_t *kbcore;
    mdef_t *mdef;
    dict_t *dict;
    dict2pid_t *d2p;
    lmset_t *lmset;
    int32 cisencnt;
    
    /* STRUCTURE: Initialize the kb structure to zero, just in case */
    memset(kb, 0, sizeof(*kb));
    kb->kbcore = NULL;
    kb->kbcore = kbcore_init (cmd_ln_float32 ("-logbase"),
			      cmd_ln_str("-feat"),
			      cmd_ln_str("-cmn"),
			      cmd_ln_str("-varnorm"),
			      cmd_ln_str("-agc"),
			      cmd_ln_str("-mdef"),
			      cmd_ln_str("-dict"),
			      cmd_ln_str("-fdict"),
			      "",	/* Hack!! Hardwired constant 
						for -compsep argument */
			      cmd_ln_str("-lm"),
			      cmd_ln_str("-lmctlfn"),
			      cmd_ln_str("-lmdumpdir"),
			      cmd_ln_str("-fillpen"),
			      cmd_ln_str("-senmgau"),
			      cmd_ln_float32("-silprob"),
			      cmd_ln_float32("-fillprob"),
			      cmd_ln_float32("-lw"),
			      cmd_ln_float32("-wip"),
			      cmd_ln_float32("-uw"),
			      cmd_ln_str("-mean"),
			      cmd_ln_str("-var"),
			      cmd_ln_float32("-varfloor"),
			      cmd_ln_str("-mixw"),
			      cmd_ln_float32("-mixwfloor"),
			      cmd_ln_str("-subvq"),
			      cmd_ln_str("-gs"),
			      cmd_ln_str("-tmat"),
			      cmd_ln_float32("-tmatfloor"));
    if(kb->kbcore==NULL)
      E_FATAL("Initialization of kb failed\n");
    
    kbcore = kb->kbcore;
    mdef = kbcore_mdef(kbcore);
    dict = kbcore_dict(kbcore);
    lmset=kbcore_lmset(kbcore);
    d2p = kbcore_dict2pid(kbcore);

    /* STRUCTURE INITIALIZATION: Initialize the beam data structure */
    kb->beam = beam_init (
			  cmd_ln_float64("-beam"),
			  cmd_ln_float64("-pbeam"),
			  cmd_ln_float64("-wbeam"),
			  cmd_ln_float64("-wend_beam"),
			  cmd_ln_int32("-ptranskip"),
			  mdef_n_ciphone(mdef)
			  );

    /* REPORT : Report the parameters in the beam data structure */
    if(REPORT_KB)
      beam_report(kb->beam);


    /* STRUCTURE INITIALIZATION: Initialize the fast GMM computation data structure */
    kb->fastgmm = fast_gmm_init(cmd_ln_int32("-ds"),
			        cmd_ln_int32("-cond_ds"),
				cmd_ln_int32("-dist_ds"),
				cmd_ln_int32("-gs4gs"),
				cmd_ln_int32("-svq4svq"),
				cmd_ln_float64("-subvqbeam"),
				cmd_ln_float64("-ci_pbeam"),
				cmd_ln_float64("-tighten_factor"),
				cmd_ln_int32("-maxcdsenpf"),
				mdef->n_ci_sen
				);

    /* REPORT : Report the parameters in the fast_gmm_t data struture */
    if(REPORT_KB)
      fast_gmm_report(kb->fastgmm);
    
    /* STRUCTURE INITIALIZATION: Initialize the phoneme lookahead data structure */
    kb->pl = pl_init(cmd_ln_int32("-pheurtype"),
		     cmd_ln_int32("-pl_beam"),
		     mdef_n_ciphone(mdef)
		     );

    /* REPORT : Report the parameters in the pl_t data struture */
    if(REPORT_KB)
      pl_report(kb->pl);


    /* STRUCTURE INITIALIZATION: Initialize the acoustic score data structure */
    for(cisencnt=0;cisencnt==mdef->cd2cisen[cisencnt];cisencnt++) ;
    kb->ascr = ascr_init (mgau_n_mgau(kbcore_mgau(kbcore)), 
			  kb->kbcore->dict2pid->n_comstate,
			  mdef_n_sseq(mdef),
			  dict2pid_n_comsseq(d2p),
			  cmd_ln_int32("-pl_window"),
			  cisencnt
			  );

    if(REPORT_KB)
      ascr_report(kb->ascr);
    
    /* STRUCTURE INITIALIZATION: Initialize the Viterbi history data structure */
    kb->vithist = vithist_init(kbcore, kb->beam->word,
			       cmd_ln_int32("-bghist"),
			       cmd_ln_int32("-lmrescore"),
			       cmd_ln_int32("-bt_wsil"),
			       REPORT_KB);

    if(REPORT_KB)
      vithist_report(kb->vithist);

    /* STRUCTURE INITIALIZATION : The feature vector */
    if ((kb->feat = feat_array_alloc(kbcore_fcb(kbcore),S3_MAX_FRAMES)) == NULL)
	E_FATAL("feat_array_alloc() failed\n");

    /* STRUCTURE INITIALIZATION : The statistics for the search */
    kb->stat = stat_init();

    /* STRUCTURE INITIALIZATION : The adaptation routines of the search */
    kb->adapt_am = adapt_am_init();

    if (cmd_ln_str("-mllr")) {
      kb_setmllr(cmd_ln_str("-mllr"), cmd_ln_str("-cb2mllr"), kb);
    }

    /* CHECK: make sure when (-cond_ds) is specified, a Gaussian map is also specified */
    if(cmd_ln_int32("-cond_ds")>0&&kb->kbcore->gs==NULL) 
      E_FATAL("Conditional Down Sampling require the use of Gaussian Selection map\n");

    /* MEMORY ALLOCATION : Word best score and exit */
    /* Open hypseg file if specified */
    kb->matchsegfp = kb->matchfp = NULL; 
    kb->matchsegfp=file_open(cmd_ln_str("-hypseg"));
    kb->matchfp=file_open(cmd_ln_str("-hyp"));
    kb->hmmdumpfp = cmd_ln_int32("-hmmdump") ? stderr : NULL;
    
    /* STRUCTURE INITIALIZATION : The search data structure, done only
       after kb is initialized kb is acted as a clipboard. */

    kb->op_mode=cmd_ln_int32("-op_mode");
    if((kb->srch = (srch_t *) srch_init (kb, kb->op_mode))==NULL){
      E_FATAL("Search initialization failed. Forced exit\n");
    }

    if(REPORT_KB){
      srch_report(kb->srch);
    }
}

void kb_set_uttid(char *_uttid,kb_t* _kb)
{
  assert(_kb != NULL);
  assert(_uttid!=NULL);

  if (_kb->uttid != NULL) {
    ckd_free(_kb->uttid);
    _kb->uttid = NULL;
  }
  if ((_kb->uttid = ckd_malloc(strlen(_uttid) + 1)) == NULL) {
    E_FATAL("Failed to allocate space for utterance id.\n");
  }
  strcpy(_kb->uttid,_uttid);
}

void kb_setmllr(char* mllrname,
		char* cb2mllrname, /** < In: The filename of the MLLR class map */
		kb_t* kb)
{
/*  int32 veclen;*/
  int32 *cb2mllr;
  E_INFO("Using MLLR matrix %s\n", mllrname);
  
  if(strcmp(kb->adapt_am->prevmllrfn,mllrname)!=0){ /* If there is a change of mllr file name */
    /* Reread the gaussian mean from the file again */
    E_INFO("Reloading mean\n");
    mgau_mean_reload(kbcore_mgau(kb->kbcore),cmd_ln_str("-mean"));

    /* Read in the mllr matrix */

#if MLLR_DEBUG
    /*This generates huge amount of information */
    /*    mgau_dump(kbcore_mgau(kb->kbcore),1);*/
#endif

    mllr_read_regmat(mllrname,
		     &(kb->adapt_am->regA),
		     &(kb->adapt_am->regB),
		     &(kb->adapt_am->mllr_nclass),
		     mgau_veclen(kbcore_mgau(kb->kbcore)));

    if (cb2mllrname && strcmp(cb2mllrname, ".1cls.") != 0) {
      int32 ncb, nmllr;

      cb2mllr_read(cb2mllrname,
		   &cb2mllr,
		   &ncb, &nmllr);
      if (nmllr != kb->adapt_am->mllr_nclass)
	E_FATAL("Number of classes in cb2mllr does not match mllr (%d != %d)\n",
		ncb, kb->adapt_am->mllr_nclass);
      if (ncb != kbcore_mdef(kb->kbcore)->n_sen)
	E_FATAL("Number of senones in cb2mllr does not match mdef (%d != %d)\n",
		ncb, kbcore_mdef(kb->kbcore)->n_sen);
    }
    else
      cb2mllr = NULL;

    /* Transform all the mean vectors */

    mllr_norm_mgau(kbcore_mgau(kb->kbcore),kb->adapt_am->regA,kb->adapt_am->regB,kb->adapt_am->mllr_nclass,cb2mllr);
    ckd_free(cb2mllr);

#if MLLR_DEBUG
    /*#if 1*/
    mllr_dump(kb->adapt_am->regA,kb->adapt_am->regB,mgau_veclen(kbcore_mgau(kb->kbcore)),kb-adapt_am->mllr_class,cb2mllr);
    /*This generates huge amount of information */
    /*mgau_dump(kbcore_mgau(kb->kbcore),1);*/
#endif 


    /* allocate memory for the prevmllrfn if it is too short*/
    if(strlen(mllrname)*sizeof(char) > 1024){
      kb->adapt_am->prevmllrfn=(char*)ckd_calloc(strlen(mllrname), sizeof(char));
    }

    strcpy(kb->adapt_am->prevmllrfn,mllrname);
  }else{
    /* No need to change anything for now */
  }
}

/* RAH 4.15.01 Lots of memory is allocated, but never freed, this function will clean up.
 * First pass will get the low hanging fruit.*/
void kb_free (kb_t *kb)
{

  if(kb->srch){
    /** Add search free code */
  }

  /* vithist */
  if (kb->vithist) 
    vithist_free((void*) kb->vithist);

  if(kb->ascr)
    ascr_free((void*) kb->ascr);

  if(kb->fastgmm)
    ckd_free((void *) kb->fastgmm);

  if(kb->beam)
    beam_free((void*) kb->beam);

  if(kb->histprune)
    histprune_free((void*) kb->histprune);
  
  if(kb->pl)
    pl_free((void*)kb->pl);

  if(kb->kbcore)
    kbcore_free (kb->kbcore);

  /* This is awkward, currently, there are two routines to control MLLRs and I don't have time 
     to unify them yet. TBD*/
  if(kb->adapt_am->regA && kb->adapt_am->regB) mllr_free_regmat(kb->adapt_am->regA, kb->adapt_am->regB);
  if(kb->adapt_am) adapt_am_free(kb->adapt_am);

  if (kb->feat) {
    ckd_free ((void *)kb->feat[0][0]);
    ckd_free_2d ((void **)kb->feat);
  }

  if (kb->matchsegfp) fclose(kb->matchsegfp);
  if (kb->matchfp) fclose(kb->matchfp);

}
