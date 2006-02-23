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
 * vithist.c -- Viterbi history
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
 * $Log$
 * Revision 1.9  2006/02/23  16:56:12  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Split latticehist_t from flat_fwd.c to  here.
 * 2, Introduced vithist_entry_cp.  This is much better than the direct
 * copy we have been using. (Which could cause memory problem easily)
 * 
 * Revision 1.8.4.12  2006/01/16 18:11:39  arthchan2003
 * 1, Important Bug fixes, a local pointer is used when realloc is needed.  This causes invalid writing of the memory, 2, Acoustic scores of the last segment in IBM lattice generation couldn't be found in the past.  Now, this could be handled by the optional acoustic scores in the node of lattice.  Application code is not yet checked-in
 *
 * Revision 1.8.4.11  2005/11/17 06:46:02  arthchan2003
 * 3 changes. 1, Code was added for full triphone implementation, not yet working. 2, Senone scale is removed from vithist table. This was a bug introduced during some fixes in CALO.
 *
 * Revision 1.8.4.10  2005/10/17 04:58:30  arthchan2003
 * vithist.c is the true source of memory leaks in the past for full cwtp expansion.  There are two changes made to avoid this happen, 1, instead of using ve->rc_info as the indicator whether something should be done, used a flag bFullExpand to control it. 2, avoid doing direct C-struct copy (like *ve = *tve), it becomes the reason of why memory are leaked and why the code goes wrong.
 *
 * Revision 1.8.4.9  2005/10/07 20:05:05  arthchan2003
 * When rescoring in full triphone expansion, the code should use the score for the word end with corret right context.
 *
 * Revision 1.8.4.8  2005/09/26 07:23:06  arthchan2003
 * Also fixed a bug such SINGLE_RC_HISTORY=0 compiled.
 *
 * Revision 1.8.4.7  2005/09/26 02:28:00  arthchan2003
 * Remove a E_INFO in vithist.c
 *
 * Revision 1.8.4.6  2005/09/25 19:33:40  arthchan2003
 * (Change for comments) Added support for Viterbi history.
 *
 * Revision 1.8.4.5  2005/09/25 19:23:55  arthchan2003
 * 1, Added arguments for turning on/off LTS rules. 2, Added arguments for turning on/off composite triphones. 3, Moved dict2pid deallocation back to dict2pid. 4, Tidying up the clean up code.
 *
 * Revision 1.8.4.4  2005/09/11 03:00:15  arthchan2003
 * All lattice-related functions are not incorporated into vithist. So-called "lattice" is essentially the predecessor of vithist_t and fsg_history_t.  Later when vithist_t support by right context score and history.  It should replace both of them.
 *
 * Revision 1.8.4.3  2005/07/26 02:20:39  arthchan2003
 * merged hyp_t with srch_hyp_t.
 *
 * Revision 1.8.4.2  2005/07/17 05:55:45  arthchan2003
 * Removed vithist_dag_write_header
 *
 * Revision 1.8.4.1  2005/07/04 07:25:22  arthchan2003
 * Added vithist_entry_display and vh_lmstate_display in vithist.
 *
 * Revision 1.8  2005/06/22 02:47:35  arthchan2003
 * 1, Added reporting flag for vithist_init. 2, Added a flag to allow using words other than silence to be the last word for backtracing. 3, Fixed doxygen documentation. 4, Add  keyword.
 *
 * Revision 1.9  2005/06/16 04:59:10  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.8  2005/05/26 00:46:59  archan
 * Added functionalities that such that <sil> will not be inserted at the end of the utterance.
 *
 * Revision 1.7  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.6  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.5  2005/04/20 03:46:30  archan
 * factor dag header writer into vithist.[ch], do the corresponding change for lm_t
 *
 * Revision 1.4  2005/03/30 01:08:38  archan
 * codebase-wide update. Performed an age-old trick: Adding $Log into all .c and .h files. This will make sure the commit message be preprended into a file.
 *
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added vithist_free() to free allocated memory
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Added vithist_partialutt_end() to allow backtracking in
 *		the middle of an utterance
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added maxwpf handling.
 * 
 * 24-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "vithist.h"
#include "lextree.h"


void vh_lmstate_display(vh_lmstate_t *vhl,dict_t *dict)
{
  /* TODO: Also translate wid to string if dict is not NULL*/
  E_INFO("lwid[0] %d\n",vhl->lm3g.lwid[0]);
  E_INFO("lwid[1] %d\n",vhl->lm3g.lwid[1]);
  E_INFO("lwid[2] %d\n",vhl->lm3g.lwid[2]);
}

void vithist_entry_display(vithist_entry_t *ve, dict_t* dict)
{

  E_INFO("Word ID %d \n",ve->wid);
  E_INFO("Sf %d Ef %d \n",ve->sf,ve->ef);
  E_INFO("Ascr %d Lscr %d \n",ve->ascr,ve->lscr);
  E_INFO("Score %d \n",ve->score);
  E_INFO("Type %d\n", ve->type);
  E_INFO("Valid for LM rescoring? %d\n", ve->valid);
  vh_lmstate_display(&(ve->lmstate),dict);
}


vithist_t *vithist_init (kbcore_t *kbc, int32 wbeam, int32 bghist, int32 isRescore, int32 isbtwsil, int32 isFullExpand, int32 isreport)
{
    vithist_t *vh;
    lmset_t *lmset;
    dict_t *dict;
    wordprob_t *wp;
    int i;
 
    int max=-1;

    if(isreport) 
      E_INFO("Initializing Viterbi-history module\n");
    
    vh = (vithist_t *) ckd_calloc (1, sizeof(vithist_t));
    
    vh->entry = (vithist_entry_t **) ckd_calloc (VITHIST_MAXBLKS, sizeof(vithist_entry_t *));

    vh->n_entry = 0;
    
    vh->frame_start = (int32 *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(int32));
    
    vh->bestscore = (int32 *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(int32));
    vh->bestvh = (int32 *) ckd_calloc (S3_MAX_FRAMES+1, sizeof(int32));
    
    vh->wbeam = wbeam;
    vh->bLMRescore=isRescore;
    vh->bBtwSil=isbtwsil;
    vh->bghist = bghist;
    vh->bFullExpand = isFullExpand;
    
    lmset =kbcore_lmset(kbc);
    dict =kbcore_dict(kbc);

    wp = (wordprob_t *) ckd_calloc (dict_size(dict), sizeof(wordprob_t));    
    
    for(i=0;i<kbc->lmset->n_lm;i++){
      if(lm_n_ug(lmset->lmarray[i])>max){
	max=lm_n_ug(lmset->lmarray[i]);
      }
    }
    /*E_INFO("Allocation for viterbi history, finall size %d\n",max);*/
    vh->lms2vh_root = (vh_lms2vh_t **) ckd_calloc (max,sizeof(vh_lms2vh_t *));
    vh->n_ci = mdef_n_ciphone(kbc->mdef);

    ckd_free ((void *) wp);

    vh->lwidlist = NULL;
    
    return vh;
}

/**
   This function cleans up rc_info
 */
static void clean_up_rc_info(scr_hist_pair *rc_info, int32 n_rc_info)
{
  int32 i;
  for(i=0;i<n_rc_info;i++){
    rc_info[i].score=S3_LOGPROB_ZERO;
    rc_info[i].pred=-1;
  }
}

/**
   Copy vithist entry from the source vb to to destination va without
   copying rc_info. This is a specific function and please don't use
   it for copying if you haven't traced it.
 */
static void vithist_entry_dirty_cp (vithist_entry_t *va,  vithist_entry_t *vb, int32 n_rc_info)
{
  scr_hist_pair *tmpshp;
  assert (vb->rc_info==NULL);
  
  tmpshp=va->rc_info;
  /* Do a direct copy */
  *va = *vb;
  va->rc_info=tmpshp;
  va->n_rc_info=n_rc_info;

#if 0
  va->wid = vb->wid;
  va->sf = vb->sf;
  va->ef = vb->ef;
  va->ascr = vb->ascr;
  va->lscr = vb->lscr;
  va->score = vb->score;
  va->pred = vb->pred;
  va->type = vb->type;
  va->valid = vb->valid;
  va->lmstate.lm3g.lwid[0] = vb->lmstate.lm3g.lwid[0];
  va->lmstate.lm3g.lwid[1] = vb->lmstate.lm3g.lwid[1];
#endif

}

/**
   Whole thing copying. 
 */

static void vithist_entry_cp (vithist_entry_t *va,  vithist_entry_t *vb)
{
  int i;
  /* Do a direct copy */
  va->wid = vb->wid;
  va->sf = vb->sf;
  va->ef = vb->ef;
  va->ascr = vb->ascr;
  va->lscr = vb->lscr;
  va->score = vb->score;
  va->pred = vb->pred;
  va->type = vb->type;
  va->valid = vb->valid;
  va->lmstate.lm3g.lwid[0] = vb->lmstate.lm3g.lwid[0];
  va->lmstate.lm3g.lwid[1] = vb->lmstate.lm3g.lwid[1];
  va->n_rc_info=vb->n_rc_info;

  if(va->rc_info){
    for(i=0;i<vb->n_rc_info;i++)
      va->rc_info[i]=vb->rc_info[i];
  }
}


/*
 * Allocate a new entry at vh->n_entry if one doesn't exist and return ptr to it.
 */
static vithist_entry_t *vithist_entry_alloc (vithist_t *vh)
{
    int32 b, l, i;
    vithist_entry_t *ve;
    vithist_entry_t *tmpve;
    
    b = VITHIST_ID2BLK(vh->n_entry);
    l = VITHIST_ID2BLKOFFSET (vh->n_entry);
    
    if (l == 0) {	/* Crossed block boundary; allocate a new block of vithist space */
	if (b >= VITHIST_MAXBLKS)
	    E_FATAL("Viterbi history array exhausted; increase VITHIST_MAXBLKS\n");
	
	assert (vh->entry[b] == NULL);
	
	ve = (vithist_entry_t *) ckd_calloc (VITHIST_BLKSIZE, sizeof(vithist_entry_t));

	vh->entry[b] = ve;

	if(vh->bFullExpand){
	  for(i=0;i<VITHIST_BLKSIZE;i++){
	    tmpve=vh->entry[b]+i;
	    tmpve->rc_info=NULL;
	  }
	}

    } else
	ve = vh->entry[b] + l;


    if(vh->bFullExpand&&ve->rc_info!=NULL)
      clean_up_rc_info(ve->rc_info,ve->n_rc_info);

    vh->n_entry++;
    return ve;
}


int32 vithist_utt_begin (vithist_t *vh, kbcore_t *kbc)
{
    vithist_entry_t *ve;
    lm_t *lm;
    dict_t *dict;
    int32 i;
    
    lm = kbcore_lm(kbc);
    dict = kbcore_dict(kbc);
    
    assert (vh->n_entry == 0);
    assert (vh->entry[0] == NULL);
    assert (vh->lwidlist == NULL);
    
    /* Create an initial dummy <s> entry.  This is the root for the utterance */
    ve = vithist_entry_alloc (vh);
    
    ve->wid = dict_startwid(dict);
    ve->sf = -1;
    ve->ef = -1;
    ve->ascr = 0;
    ve->lscr = 0;
    ve->score = 0;
    ve->pred = -1;
    ve->type = 0;
    ve->valid = 1;
    ve->lmstate.lm3g.lwid[0] = lm_startwid(lm);
    ve->lmstate.lm3g.lwid[1] = BAD_S3LMWID;


    if(vh->bFullExpand){
      if(ve->rc_info==NULL){
	ve->n_rc_info=get_rc_nssid(kbc->dict2pid,dict_startwid(dict),kbc->dict);
	ve->rc_info=ckd_calloc(vh->n_ci,sizeof(scr_hist_pair)); 
	/* Always used n_ci phone to be the allocation size */
      }
      for(i=0;i<ve->n_rc_info;i++){
	ve->rc_info[i].score=S3_LOGPROB_ZERO;
	ve->rc_info[i].pred=-1;
      }
    }
    
    vh->n_frm = 0;
    vh->frame_start[0] = 1;
    vh->bestscore[0] = MAX_NEG_INT32;
    vh->bestvh[0] = -1;
    
    return 0;
}


static int32 vh_lmstate_find (vithist_t *vh, vh_lmstate_t *lms)
{
    vh_lms2vh_t *lms2vh;
    s3lmwid_t lwid;
    gnode_t *gn;
    
    lwid = lms->lm3g.lwid[0];
    if ((lms2vh = vh->lms2vh_root[lwid]) == NULL)
	return -1;

    assert (lms2vh->state == lwid);
    
    lwid = lms->lm3g.lwid[1];
    for (gn = lms2vh->children; gn; gn = gnode_next(gn)) {
      lms2vh = (vh_lms2vh_t *) gnode_ptr (gn);
	if (lms2vh->state == lwid)
	    return lms2vh->vhid;
    }
    
    return -1;
}


/*
 * Enter a new LMstate into the current frame LMstates trees; called ONLY IF not already
 * present.
 */
static void vithist_lmstate_enter (vithist_t *vh, int32 vhid, vithist_entry_t *ve)
{
    vh_lms2vh_t *lms2vh, *child;
    s3lmwid_t lwid;
    
    lwid = ve->lmstate.lm3g.lwid[0];
    if ((lms2vh = vh->lms2vh_root[lwid]) == NULL) {
	lms2vh = (vh_lms2vh_t *) ckd_calloc (1, sizeof(vh_lms2vh_t));
	vh->lms2vh_root[lwid] = lms2vh;
	
	lms2vh->state = lwid;
	lms2vh->children = NULL;
	
	vh->lwidlist = glist_add_int32 (vh->lwidlist, (int32) lwid);
    } else {
	assert (lms2vh->state == lwid);
    }
    
    child = (vh_lms2vh_t *) ckd_calloc (1, sizeof(vh_lms2vh_t));
    child->state = ve->lmstate.lm3g.lwid[1];
    child->children = NULL;
    child->vhid = vhid;
    child->ve = ve;
    
    lms2vh->children = glist_add_ptr (lms2vh->children, (void *)child);
}


vithist_entry_t *vithist_id2entry (vithist_t *vh, int32 id)
{
    vithist_entry_t *ve;

    ve = vh->entry[VITHIST_ID2BLK(id)] + VITHIST_ID2BLKOFFSET(id);
    return ve;
}

static int32 vithist_entry_maxscr(vithist_entry_t *ve, int32 isFullExpand)
{
  int32 i;
  int32 max=S3_LOGPROB_ZERO;
  if(ve->rc_info==NULL){
    return ve->score;
  }else{
    assert(isFullExpand);
    for(i=0;i<ve->n_rc_info;i++){
      /*     E_INFO("Max %d, ve->rc_info[i].score %d\n",max, ve->rc_info[i].score);*/
      if(max < ve->rc_info[i].score){
	max = ve->rc_info[i].score;
      }
    }
    /*    E_INFO("MAX %d ve->wid %d\n", max, ve->wid);*/
    return max;
  }
}

/* Rclist is separate from tve because C structure copying is used in *ve = *tve 
 */
static void vithist_enter (vithist_t *vh,  /**< The history table */
			   kbcore_t *kbc,  /**< a KB core */
			   vithist_entry_t *tve,  /**< an input vithist element */
			   int32 comp_rc        /**< a compressed rc. If it is the actual rc, it won't work. */
			   )

{
    vithist_entry_t *ve;
    int32 vhid;
    int32 n_ci;
    dict_t* d;
    int32 n_rc_info;
    int32 old_n_rc_info;

    d=kbc->dict;
    n_ci=vh->n_ci;
    /* Check if an entry with this LM state already exists in current frame */
    vhid = vh_lmstate_find (vh, &(tve->lmstate));

    if(vh->bFullExpand)
      n_rc_info=get_rc_nssid(kbc->dict2pid,tve->wid,kbc->dict);
    else
      n_rc_info=0; /* Just fill in something if not using crossword triphon*/


    assert(comp_rc < n_rc_info);

    if (vhid < 0) {	/* Not found; allocate new entry */
	vhid = vh->n_entry;
	ve = vithist_entry_alloc (vh);

	vithist_entry_dirty_cp(ve, tve, n_rc_info);
	vithist_lmstate_enter (vh, vhid, ve);	/* Enter new vithist info into LM state tree */

	/*	E_INFO("Start a new entry wid %d\n",ve->wid);*/
	if(ve->rc_info!=NULL)
	  clean_up_rc_info(ve->rc_info,ve->n_rc_info);

	if(comp_rc != -1) {
	  if(ve->rc_info==NULL){
	    ve->n_rc_info=get_rc_nssid(kbc->dict2pid,ve->wid,kbc->dict);
	    /* Always allocate n_ci for rc_info*/
	    ve->rc_info=ckd_calloc(vh->n_ci,sizeof(scr_hist_pair)); 
	    clean_up_rc_info(ve->rc_info,ve->n_rc_info);
	  }

	  assert(comp_rc < ve->n_rc_info);
	  if(ve->rc_info[comp_rc].score < tve->score){
	    ve->rc_info[comp_rc].score = tve->score;
	    ve->rc_info[comp_rc].pred = tve->pred;
	  }
	}

	
    } else {
	ve = vh->entry[VITHIST_ID2BLK(vhid)] + VITHIST_ID2BLKOFFSET(vhid);

	/*	E_INFO("Replace the old entry\n"); */
	/*		E_INFO("Old entry wid %d, New entry wid %d\n",ve->wid, tve->wid);*/

	if(comp_rc == -1 ){
	  if (ve->score < tve->score){
	    vithist_entry_dirty_cp(ve,tve,n_rc_info);
	    assert(comp_rc < n_rc_info);
	    if(ve->rc_info!=NULL)
	      clean_up_rc_info(ve->rc_info,ve->n_rc_info);
	  }

	}else{

	  /* This is wrong, the score 
	     Alright, how vhid was searched in the first place? 
	   */
	  if(vithist_entry_maxscr(ve,vh->bFullExpand) < tve->score){
	    old_n_rc_info=ve->n_rc_info;
	    vithist_entry_dirty_cp(ve,tve,n_rc_info);
	    assert(comp_rc < n_rc_info);

	    assert(ve->rc_info);
	    clean_up_rc_info(ve->rc_info,ve->n_rc_info);
	    ve->rc_info[comp_rc].score = tve->score;
	    ve->rc_info[comp_rc].pred = tve->pred;
	  }

	}

    }

    /*    E_INFO("vhid %d, vh->n_frm %d, ve->wid %d, ve->rc_info[comp_rc].score  %d, tve->score %d, comp_rc %d, ve->n_rc_info %d, vh->bestscore[vh->nfrm] %d\n",vhid, vh->n_frm, ve->wid, ve->rc_info[comp_rc].score, tve->score, comp_rc, ve->n_rc_info, vh->bestscore[vh->n_frm]);*/
    /* Update best exit score in this frame */
    if (vh->bestscore[vh->n_frm] < tve->score) {
	vh->bestscore[vh->n_frm] = tve->score;
	vh->bestvh[vh->n_frm] = vhid;
    }
}


void vithist_rescore (vithist_t *vh, kbcore_t *kbc,
		      s3wid_t wid, int32 ef, int32 score, 
		      int32 pred, int32 type, int32 rc)
{
    vithist_entry_t *pve, tve;
    s3lmwid_t lwid;
    int32 se, fe;
    int32 i;
    int32 ci;
    s3cipid_t *rcmap;
    
    assert (vh->n_frm == ef);
    if(pred == -1) { 
      /* Always do E_FATAL assuming upper level function take care of error checking. */
      E_FATAL("Hmm->out.history equals to -1 with score %d, some active phone was not computed?\n",score);
    }

    /* pve is the previous entry before word with wid or, se an fe is the
       first to the last entry before pve. So pve is w_{n-1} */

    pve = vh->entry[VITHIST_ID2BLK(pred)] + VITHIST_ID2BLKOFFSET(pred);

    /* Create a temporary entry with all the info currently available */
    tve.wid = wid;
    tve.sf = pve->ef + 1;
    tve.ef = ef;
    tve.type = type;
    tve.valid = 1;

    /* || dict_filler_word(kbcore_dict(kbc),wid))*/

    if(!vh->bFullExpand)
      tve.ascr = score - pve->score; 
    else {
      ci=dict_first_phone(kbc->dict,tve.wid); 
      rcmap=(s3cipid_t*) dict2pid_get_rcmap(kbc->dict2pid,pve->wid,kbc->dict);


      tve.ascr = score - pve->score; 
      /*      tve.ascr = score - pve->rc_info[ci].score;*/

#if 0 /*FIX ME, should use the context-specific score instead. */
      if(pve->rc_info[ci].score> S3_LOGPROB_ZERO)
	tve.ascr = score - pve->rc_info[ci].score;
      else
	tve.ascr = score - pve->score; 
#endif
    }

    tve.lscr = 0;
    tve.rc_info=NULL;
    tve.n_rc_info=0;

    if (pred == 0) {	/* Special case for the initial <s> entry */
	se = 0;
	fe = 1;
    } else {
	se = vh->frame_start[pve->ef];
	fe = vh->frame_start[pve->ef + 1];
    }
    
    if (dict_filler_word (kbcore_dict(kbc), wid)) {

	tve.score = score ; 

	if(vh->bLMRescore){
	  tve.lscr = fillpen (kbcore_fillpen(kbc), wid);
	  tve.score += tve.lscr;
	}

	tve.pred = pred;
	tve.lmstate.lm3g = pve->lmstate.lm3g;
	vithist_enter (vh, kbc, &tve,rc);
    } else {

      /* Now if it is a word, backtrack again to get all possible previous word
	 So  pve becomes the w_{n-2}. 
       */

	lwid = kbc->lmset->cur_lm->dict2lmwid[wid];

	tve.lmstate.lm3g.lwid[0] = lwid;
	
	for (i = se; i < fe; i++) {
	    pve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	    
	    if (pve->valid) {
		
	      if(!vh->bFullExpand)
		tve.score = pve->score; 
	      else {
		/* Remember to use context as well */
		ci=dict_first_phone(kbc->dict,tve.wid); 
		rcmap=(s3cipid_t*)dict2pid_get_rcmap(kbc->dict2pid,pve->wid,kbc->dict);

		tve.score = pve->score; 
		/*		tve.score = pve->rc_info[rcmap[ci]].score;*/

#if 0


		/*		tve.score = pve->score;*/

		if(pve->rc_info[rcmap[ci]].score> S3_LOGPROB_ZERO)
		  tve.score = pve->rc_info[rcmap[ci]].score;
		else
		  tve.score = pve->score; 
#endif

#if 0  /*FIX ME, should use the context-specific score instead. */
		if(pve->rc_info[ci].score > S3_LOGPROB_ZERO)
		  tve.score = pve->rc_info[ci].score;
		else
		  tve.score = pve->score;
#endif


	      }

	      tve.score += tve.ascr;

		if(vh->bLMRescore){
		  tve.lscr = lm_tg_score (kbcore_lm(kbc),
					pve->lmstate.lm3g.lwid[1],
					pve->lmstate.lm3g.lwid[0],
					lwid,
					wid);
		  tve.score += tve.lscr;
		}
		if ((tve.score - vh->wbeam) >= vh->bestscore[vh->n_frm]) {
		    tve.pred = i;
		    tve.lmstate.lm3g.lwid[1] = pve->lmstate.lm3g.lwid[0];
		    
		    vithist_enter (vh, kbc, &tve,rc);
		}
	    }
	}
    }
}


/*
 * Garbage collect invalid entries in current frame, right after marking invalid entries.
 */
static void vithist_frame_gc (vithist_t *vh, int32 frm)
{
    vithist_entry_t *ve, *tve;
    int32 se, fe, te, bs, bv;
    int32 i, j,k;
    int32 l;
    int32 n_rc_info;
    
    n_rc_info=0;
    se = vh->frame_start[frm];
    fe = vh->n_entry - 1;
    te = se;
    
    bs = MAX_NEG_INT32;
    bv = -1;
    for (i = se; i <= fe; i++) {
	ve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	if (ve->valid) {
	    if (i != te) {	/* Move i to te */
		tve = vh->entry[VITHIST_ID2BLK(te)] + VITHIST_ID2BLKOFFSET(te);
		/**tve = *ve;*/
		vithist_entry_cp(tve,ve);
	    }
	    
	    if (ve->score > bs) {
		bs = ve->score;
		bv = te;
	    }

#if 1
	    if (ve->rc_info !=NULL){
	      assert(vh->bFullExpand);
	      
	      for(k=0;k<ve->n_rc_info;k++){
		if(ve->rc_info[k].score > bs){
		  bs = ve->rc_info[k].score;
		  bv = te;
		}
	      }
	    }
#endif
	    te++;
	}
    }


    /*    E_INFO("frm %d, bs %d vh->bestscore[frm] %d\n",frm, bs, vh->bestscore[frm]);*/

    /* Can't assert this any more because history pruning could actually make the 
       best token go away 

    */
    assert (bs == vh->bestscore[frm]);           


    vh->bestvh[frm] = bv;
    
    /* Free up entries [te..n_entry-1] */
    i = VITHIST_ID2BLK(vh->n_entry - 1);
    j = VITHIST_ID2BLK(te - 1);
    for (; i > j; --i) {

      for(l=0 ; l < VITHIST_BLKSIZE; l++){
	ve=vh->entry[i]+l;
	if(ve->rc_info!=NULL){
	  ckd_free(ve->rc_info);
	  ve->rc_info=NULL;
	}
      }

      ckd_free ((void *) vh->entry[i]);
      vh->entry[i] = NULL;
    }
    vh->n_entry = te;
}



void vithist_prune (vithist_t *vh, dict_t *dict, int32 frm,
		    int32 maxwpf, int32 maxhist, int32 beam)
{
    int32 se, fe, filler_done, th;
    vithist_entry_t *ve;
    heap_t h;
    s3wid_t *wid;
    int32 i;
    
    assert (frm >= 0);
    
    se = vh->frame_start[frm];
    fe = vh->n_entry - 1;
    
    th = vh->bestscore[frm] + beam;
    
    h = heap_new ();
    wid = (s3wid_t *) ckd_calloc (maxwpf+1, sizeof(s3wid_t));
    wid[0] = BAD_S3WID;
    
    for (i = se; i <= fe; i++) {
	ve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	heap_insert (h, (void *)ve, -(ve->score));
	ve->valid = 0;
	/*	E_INFO("Vhid %d, For frame %d, I have word ID %d with max score %d\n", i, frm, ve->wid, vithist_entry_maxscr(ve,vh->bFullExpand));*/
    }
    
    /* Mark invalid entries: beyond maxwpf words and below threshold */
    filler_done = 0;
    while ((heap_pop (h, (void **)(&ve), &i) > 0) && 
	   ( vithist_entry_maxscr(ve,vh->bFullExpand) >= th)  &&  /* the score (or the cw scores) is above threshold */
	   (maxhist > 0)         /* Number of history is larger than 0*/ 
	   ) 
      {

#if 1
	if (dict_filler_word (dict, ve->wid)) {
	    /* Major HACK!!  Keep only one best filler word entry per frame */
	    if (filler_done)
		continue;
	    filler_done = 1;
	}

#endif
	/*	E_INFO("ve->wid survived, maxwpf %d, maxhistpf %d\n",maxwpf,maxhist);*/
	/* Check if this word already valid (e.g., under a different history) */
	for (i = 0; IS_S3WID(wid[i]) && (wid[i] != ve->wid); i++);
	if (NOT_S3WID(wid[i])) {
	    /* New word; keep only if <maxwpf words already entered, even if >= thresh */
	    if (maxwpf > 0) {
		wid[i] = ve->wid;
		wid[i+1] = BAD_S3WID;
		
		--maxwpf;
		--maxhist;
		ve->valid = 1;
	    }
	} else if (! vh->bghist) {
	    --maxhist;
	    ve->valid = 1;
	}
    }
    
    ckd_free ((void *) wid);
    heap_destroy (h);
    
    /* Garbage collect invalid entries */
    vithist_frame_gc (vh, frm);
}


static void vithist_lmstate_reset (vithist_t *vh)
{
    gnode_t *lgn, *gn;
    int32 i;
    vh_lms2vh_t *lms2vh, *child;
    
    for (lgn = vh->lwidlist; lgn; lgn = gnode_next(lgn)) {
	i = (int32) gnode_int32 (lgn);
	lms2vh = vh->lms2vh_root[i];
	
	for (gn = lms2vh->children; gn; gn = gnode_next(gn)) {
	    child = (vh_lms2vh_t *) gnode_ptr (gn);
	    ckd_free ((void *) child);
	}
	glist_free (lms2vh->children);
	
	ckd_free ((void *) lms2vh);
	
	vh->lms2vh_root[i] = NULL;
    }
    
    glist_free (vh->lwidlist);
    vh->lwidlist = NULL;
}


void vithist_frame_windup (vithist_t *vh, int32 frm, FILE *fp, kbcore_t *kbc)
{
    assert (vh->n_frm == frm);
    
    vh->n_frm++;
    vh->frame_start[vh->n_frm] = vh->n_entry;
    
    if (fp)
	vithist_dump (vh, frm, kbc, fp);
    
    vithist_lmstate_reset (vh);
    
    vh->bestscore[vh->n_frm] = MAX_NEG_INT32;
    vh->bestvh[vh->n_frm] = -1;
}


int32 vithist_utt_end (vithist_t *vh, kbcore_t *kbc)
{
    int32 f, i, b, l;
    int32 sv, nsv, scr, bestscore, bestvh, vhid;
    vithist_entry_t *ve, *bestve=0;
    s3lmwid_t endwid;
    lm_t *lm;
    dict_t *dict;

    bestscore = MAX_NEG_INT32;
    bestvh = -1;

    /* Find last frame with entries in vithist table */
    /* by ARCHAN 20050525, it is possible that the last frame will not be reached in decoding */

    for (f = vh->n_frm-1; f >= 0; --f) {
	sv = vh->frame_start[f];	/* First vithist entry in frame f */
	nsv = vh->frame_start[f+1];	/* First vithist entry in next frame (f+1) */
	
	if (sv < nsv)
	    break;
    }
    if (f < 0)
	return -1;
    
    if (f != vh->n_frm-1)
	E_WARN("No word exit in frame %d, using exits from frame %d\n", vh->n_frm-1, f);
    
    /* Terminate in a final </s> node (make this optional?) */
    lm = kbcore_lm (kbc);
    dict = kbcore_dict (kbc);

    if(vh->bBtwSil){
      endwid = lm_finishwid (lm);    
    
      for (i = sv; i < nsv; i++) {
	b = VITHIST_ID2BLK (i);
	l = VITHIST_ID2BLKOFFSET (i);
	ve = vh->entry[b] + l;
	
	scr = ve->score;
	scr += lm_tg_score (lm, ve->lmstate.lm3g.lwid[1], ve->lmstate.lm3g.lwid[0], endwid,dict_finishwid(dict));
	
	if (bestscore < scr) {
	  bestscore = scr;
	  bestvh = i;
	  bestve = ve;
	}
      }
      assert (bestvh >= 0);
    }

    
    if (f != vh->n_frm-1) {
	E_ERROR("No word exit in frame %d, using exits from frame %d\n", vh->n_frm-1, f);
	
	/* Add a dummy silwid covering the remainder of the utterance */
	assert (vh->frame_start[vh->n_frm-1] == vh->frame_start[vh->n_frm]);
	vh->n_frm -= 1;
	vithist_rescore (vh, kbc, dict_silwid (dict), vh->n_frm, bestve->score, bestvh, -1,-1);
	vh->n_frm += 1;
	vh->frame_start[vh->n_frm] = vh->n_entry;
	
	return vithist_utt_end (vh, kbc);
    }
    
    /*    vithist_dump(vh,-1,kbc,stdout);*/
    /* Create an </s> entry */

    if(vh->bBtwSil){
      ve = vithist_entry_alloc (vh);
      
      ve->wid = dict_finishwid (dict);
      ve->sf = (bestve->ef == BAD_S3FRMID) ? 0 : bestve->ef + 1;
      ve->ef = vh->n_frm;
      ve->ascr = 0;
      ve->lscr = bestscore - bestve->score;
      ve->score = bestscore;
      ve->pred = bestvh;
      ve->type = 0;
      ve->valid = 1;
      ve->lmstate.lm3g.lwid[0] = endwid;
      ve->lmstate.lm3g.lwid[1] = ve->lmstate.lm3g.lwid[0];
    }

    vhid=vh->n_entry-1;


    /*vithist_dump(vh,-1,kbc,stdout);*/

    return vhid;

}


int32 vithist_partialutt_end (vithist_t *vh, kbcore_t *kbc)
{
    int32 f, i, b, l;
    int32 sv, nsv, scr, bestscore, bestvh;
    vithist_entry_t *ve, *bestve;
    s3lmwid_t endwid;
    lm_t *lm;
    dict_t *dict;

    /* Find last frame with entries in vithist table */
    for (f = vh->n_frm-1; f >= 0; --f) {
	sv = vh->frame_start[f];	/* First vithist entry in frame f */
	nsv = vh->frame_start[f+1];	/* First vithist entry in next frame (f+1) */
	
	if (sv < nsv)
	    break;
    }
    if (f < 0)
	return -1;
    
    if (f != vh->n_frm-1){
	E_ERROR("No word exits from in block with last frame= %d\n",vh->n_frm-1);
	return -1;
    }
    
    /* Terminate in a final </s> node (make this optional?) */
    lm = kbcore_lm (kbc);
    dict = kbcore_dict (kbc);    
    endwid = lm_finishwid (lm);
    
    bestscore = MAX_NEG_INT32;
    bestvh = -1;
    
    for (i = sv; i < nsv; i++) {
	b = VITHIST_ID2BLK (i);
	l = VITHIST_ID2BLKOFFSET (i);
	ve = vh->entry[b] + l;
	
	scr = ve->score;
	scr += lm_tg_score (lm, ve->lmstate.lm3g.lwid[1], ve->lmstate.lm3g.lwid[0], endwid, dict_finishwid(dict));
	
	if (bestscore < scr) {
	    bestscore = scr;
	    bestvh = i;
	    bestve = ve;
	}
    }

    return bestvh;
}


void vithist_utt_reset (vithist_t *vh)
{
    int32 b,l;
    int32 ent;
    vithist_entry_t *ve;
    
    vithist_lmstate_reset (vh);
    
    for (b = VITHIST_ID2BLK(vh->n_entry-1); b >= 0; --b) {

      /* If rc_info is used, then free them */
      if(b!=0)
	ent=VITHIST_BLKSIZE-1;
      else
	ent=vh->n_entry-1;

      if(vh->bFullExpand){
	/* This is bad, could it be just a kind of clean up? */
	for(l =0 ;l < VITHIST_BLKSIZE ; l++){
	  ve =vh->entry[b]+l;
	  /*	  E_INFO("Freeing, entry %d, b %d\n",l,b);*/
	  if(ve->rc_info!=NULL){
	    ckd_free(ve->rc_info);
	    ve->rc_info=NULL;
	  }
	}
      }

      ckd_free ((void *) vh->entry[b]);
      vh->entry[b] = NULL;
    }
    vh->n_entry = 0;
    
    vh->bestscore[0] = MAX_NEG_INT32;
    vh->bestvh[0] = -1;
}


#if 0
static void vithist_lmstate_subtree_dump (vithist_t *vh, kbcore_t *kbc,
					  vh_lmstate2vithist_t *lms2vh, int32 level, FILE *fp)
{
    gnode_t *gn;
    vh_lmstate2vithist_t *child;
    int32 i;
    lm_t *lm;
    
    lm = kbcore_lm (kbc);
    
    for (gn = lms2vh->children; gn; gn = gnode_next(gn)) {
	child = (vh_lmstate2vithist_t *) gnode_ptr (gn);
	
	for (i = 0; i < level; i++)
	    fprintf (fp, "\t");
	fprintf (fp, "\t%s -> %d\n", lm_wordstr(lm, child->state), child->vhid);
	
	vithist_lmstate_subtree_dump (vh, kbc, child, level+1, fp);
    }
}


static void vithist_lmstate_dump (vithist_t *vh, kbcore_t *kbc, FILE *fp)
{
    glist_t gl;
    gnode_t *lgn, *gn;
    int32 i;
    vh_lmstate2vithist_t *lms2vh;
    mdef_t *mdef;
    lm_t *lm;
    
    mdef = kbcore_mdef (kbc);
    lm = kbcore_lm (kbc);
    
    fprintf (fp, "LMSTATE\n");
    for (lgn = vh->lwidlist; lgn; lgn = gnode_next(lgn)) {
	i = (int32) gnode_int32 (lgn);
	
	gl = vh->lmstate_root[i];
	assert (gl);
	
	for (gn = gl; gn; gn = gnode_next(gn)) {
	    lms2vh = (vh_lmstate2vithist_t *) gnode_ptr (gn);
	    
	    fprintf (fp, "\t%s.%s -> %d\n",
		     lm_wordstr(lm, i), mdef_ciphone_str (mdef, lms2vh->state), lms2vh->vhid);
	    vithist_lmstate_subtree_dump (vh, kbc, lms2vh, 1, fp);
	}
    }
    fprintf (fp, "END_LMSTATE\n");
    fflush (fp);
}
#endif


void vithist_dump (vithist_t *vh, int32 frm, kbcore_t *kbc, FILE *fp)
{
    int32 b, l, i, j;
    dict_t *dict;
    lm_t *lm;
    vithist_entry_t *ve;
    s3lmwid_t lwid;
    int32 sf, ef;
    
    dict = kbcore_dict (kbc);
    lm = kbcore_lm (kbc);
    
    if (frm >= 0) {
	sf = frm;
	ef = frm;
	
	fprintf (fp, "VITHIST  frame %d  #entries %d\n",
		 frm, vh->frame_start[sf+1] - vh->frame_start[sf]);
    } else {
	sf = 0;
	ef = vh->n_frm - 1;
	
	fprintf (fp, "VITHIST  #frames %d  #entries %d\n", vh->n_frm, vh->n_entry);
    }
    fprintf (fp, "\t%7s %5s %5s %11s %9s %8s %7s %4s Word (LM-state)\n",
	     "Seq/Val", "SFrm", "EFrm", "PathScr", "SegAScr", "SegLScr", "Pred", "Type");
    
    for (i = sf; i <= ef; i++) {
	fprintf (fp, "%5d BS: %11d BV: %8d\n", i, vh->bestscore[i], vh->bestvh[i]);
	
	for (j = vh->frame_start[i]; j < vh->frame_start[i+1]; j++) {
	    /* This entry */
	    b = VITHIST_ID2BLK(j);
	    l = VITHIST_ID2BLKOFFSET (j);
	    ve = vh->entry[b] + l;
	    
	    fprintf (fp, "\t%c%6d %5d %5d %11d %9d %8d %7d %4d %s",
		     (ve->valid ? ' ' : '*'), j,
		     ve->sf, ve->ef, ve->score, ve->ascr, ve->lscr, ve->pred, ve->type,
		     dict_wordstr (dict, ve->wid));
	    
	    fprintf (fp, " (%s", lm_wordstr (lm, ve->lmstate.lm3g.lwid[0]));
	    lwid = ve->lmstate.lm3g.lwid[1];
	    if (IS_S3LMWID(lwid))
		fprintf (fp, ", %s", lm_wordstr (lm, lwid));
	    fprintf (fp, ")\n");
	}
	
	if (j == vh->frame_start[i])
	    fprintf (fp, "\n");
    }

#if 0    
    if (vh->lwidlist)
	vithist_lmstate_dump (vh, kbc, fp);
#endif

    fprintf (fp, "END_VITHIST\n");
    fflush (fp);
}

glist_t vithist_backtrace (vithist_t *vh, int32 id, dict_t *dict)
{
    vithist_entry_t *ve;
    int32 b, l;
    glist_t hyp;
    srch_hyp_t *h;
    s3cipid_t ci;
    
    hyp = NULL;
    ci = BAD_S3CIPID;
    while (id > 0) {

      /*      E_INFO("id %d\n",id);*/

	b = VITHIST_ID2BLK(id);
	l = VITHIST_ID2BLKOFFSET(id);

	ve = vh->entry[b] + l;

	assert(ve);
#if 1
	if(vh->bFullExpand){
	  ci = dict_pron(dict,h->id,0); /* In this case, one will need to know the right context of 
					 the previous word . I also need to first word ID in this case
					 and that could determine the first phone id. 
				      */
	}
#endif
	
	h = (srch_hyp_t *) ckd_calloc (1, sizeof(srch_hyp_t));
	h->id = ve->wid;
	h->sf = ve->sf;
	h->ef = ve->ef;
	h->ascr = ve->ascr;
	h->lscr = ve->lscr;

	/* FIX ME ! Senone-scale should be computed, Probably shouldn't be computed at here 
	  h->senscale=ve->senscale;
	*/
	h->type = ve->type;
	h->vhid = id;
	
	hyp = glist_add_ptr (hyp, h);

	id = ve->pred;
#if 0
	if(vh->bFullExpand) /* No right context information, Just get it from pred*/
	  id= ve->rc_info[ci].pred;
	else
	  id = ve->pred;
#endif
	

    }
    
    return hyp;
}


typedef struct {
    s3wid_t wid;
    int32 fef, lef;
    int32 seqid;	/* Node sequence no. */

#if 1 /* augmented in 3.6 to store the acoustic score */
  int32 ascr; 
  int32 lscr;
#endif
    glist_t velist;	/* Vithist entries for this dagnode */
} vithist_dagnode_t;


/*
 * Header written BEFORE this function is called.
 */
void vithist_dag_write (vithist_t *vh, glist_t hyp, dict_t *dict, int32 oldfmt, FILE *fp, int32 dump_nodescr)
{

  /* WARNING!!!! DO NOT INSERT a # in the format arbitrarily because the dag_reader is not very robust */
    glist_t *sfwid;		/* To maintain <start-frame, word-id> pair dagnodes */
    vithist_entry_t *ve, *ve2;
    gnode_t *gn, *gn2, *gn3;
    vithist_dagnode_t *dn, *dn2;
    int32 sf, ef, n_node;
    int32 f, i;
    srch_hyp_t *h;
    
    sfwid = (glist_t *) ckd_calloc (vh->n_frm+1, sizeof(glist_t));
    
    n_node = 0;
    for (i = 0; i < vh->n_entry; i++) {	/* This range includes the dummy <s> and </s> entries */
	ve = vh->entry[VITHIST_ID2BLK(i)] + VITHIST_ID2BLKOFFSET(i);
	if (! ve->valid)
	    continue;
	
	/*
	 * The initial <s> entry (at 0) is a dummy, with start/end frame = -1.  But the old S3
	 * code treats it like a real word, so we have to reintroduce it in the dag file with
	 * a start time of 0.  And shift the start time of words starting at frame 0 up by 1.
	 * MAJOR HACK!!
	 */
	if (ve->sf <= 0) {
	    assert (ve->sf >= -1);
	    assert ((ve->ef == -1) || (ve->ef > 1));
	    
	    sf = ve->sf + 1;
	    ef = (ve->ef < 0) ? 0 : ve->ef;
	} else {
	    sf = ve->sf;
	    ef = ve->ef;
	}
	
	for (gn = sfwid[sf]; gn; gn = gnode_next(gn)) {
	    dn = (vithist_dagnode_t *) gnode_ptr(gn);
	    if (dn->wid == ve->wid)
		break;
	}
	if (! gn) {
	    dn = (vithist_dagnode_t *) ckd_calloc (1, sizeof(vithist_dagnode_t));
	    dn->wid = ve->wid;

	    /* New at 20051116, to take care of the issues of last
	       segment for converting S3 lattice to IBM lattice */

	    dn->ascr = ve->ascr;
	    dn->lscr = ve->lscr;

	    dn->fef = ef;
	    dn->lef = ef;
	    dn->seqid = -1;	/* Initially all invalid, selected ones validated below */
	    dn->velist = NULL;
	    n_node++;
	    
	    sfwid[sf] = glist_add_ptr (sfwid[sf], (void *) dn);
	} else {
	    dn->lef = ef;
	}
	
	/*
	 * Check if an entry already exists under dn->velist (generated by a different
	 * LM context; retain only the best scoring one.
	 */
	for (gn = dn->velist; gn; gn = gnode_next(gn)) {
	    ve2 = (vithist_entry_t *) gnode_ptr (gn);
	    if (ve2->ef == ve->ef)
		break;
	}
	if (gn) {
	    if (ve->score > ve2->score)
		gnode_ptr(gn) = (void *)ve;
	} else
	    dn->velist = glist_add_ptr (dn->velist, (void *) ve);
    }
    
    /*
     * Validate segments with >1 end times; if only 1 end time, can be pruned.
     * But keep segments in the original hypothesis, regardless; mark them first.
     */
    for (gn = hyp; gn; gn = gnode_next(gn)) {
	h = (srch_hyp_t *) gnode_ptr (gn);
	for (gn2 = sfwid[h->sf]; gn2; gn2 = gnode_next(gn2)) {
	    dn = (vithist_dagnode_t *) gnode_ptr (gn2);
	    if (h->id == dn->wid)
		dn->seqid = 0;	/* Do not discard (prune) this dagnode */
	}
    }
    
    /* Validate startwid and finishwid nodes */
    dn = (vithist_dagnode_t *) gnode_ptr(sfwid[0]);
    assert (dn->wid == dict_startwid(dict));
    dn->seqid = 0;
    dn = (vithist_dagnode_t *) gnode_ptr(sfwid[vh->n_frm]);
    assert (dn->wid == dict_finishwid(dict));
    dn->seqid = 0;
    
    /* Now prune dagnodes with only 1 end frame if not validated above */
    i = 0;
    for (f = vh->n_frm; f >= 0; --f) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (vithist_dagnode_t *) gnode_ptr(gn);
	    if ((dn->lef > dn->fef) || (dn->seqid >= 0))
		dn->seqid = i++;
	    else
		dn->seqid = -1;		/* Flag: discard */
	}
    }
    n_node = i;
    
    /* Write nodes info; the header should have been written before this function is called */
    fprintf (fp, "Nodes %d (NODEID WORD STARTFRAME FIRST-ENDFRAME LAST-ENDFRAME ASCORE LSCORE)\n", n_node);
    for (f = vh->n_frm; f >= 0; --f) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (vithist_dagnode_t *) gnode_ptr(gn);
	    if (dn->seqid >= 0) {
		fprintf (fp, "%d %s %d %d %d ",
			 dn->seqid, dict_wordstr(dict, dn->wid), f, 
			 dn->fef, dn->lef);
		
		if(dump_nodescr)
		  fprintf(fp, "%d %d\n",dn->ascr,dn->lscr);
		else
		  fprintf(fp, "\n");

	    }
	}
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "Initial %d\nFinal %d\n", n_node-1, 0);
    fprintf (fp, "#\n");
    
    fprintf (fp, "BestSegAscr 0 (NODEID ENDFRAME ASCORE)\n");
    fprintf (fp, "#\n");
    
    /* Edges */
    if (oldfmt)
	fprintf (fp, "Edges (FROM-NODEID TO-NODEID ASCORE)\n");
    else
	fprintf (fp, "Edges (FROM-NODEID ENDFRAME ASCORE)\n");
    for (f = vh->n_frm-1; f >= 0; --f) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (vithist_dagnode_t *) gnode_ptr(gn);
	    /* Look for transitions from this dagnode to later ones, if not discarded */
	    if (dn->seqid < 0)
		continue;
	    
	    for (gn2 = dn->velist; gn2; gn2 = gnode_next(gn2)) {
		ve = (vithist_entry_t *) gnode_ptr (gn2);
		
		sf = (ve->ef < 0) ? 1 : (ve->ef + 1);
		
		if (oldfmt) {
		    for (gn3 = sfwid[sf]; gn3; gn3 = gnode_next(gn3)) {
			dn2 = (vithist_dagnode_t *) gnode_ptr(gn3);
			if (dn2->seqid >= 0){
			    fprintf (fp, "%d %d %d\n", dn->seqid, dn2->seqid, ve->ascr);
			}
		    }
		} else {
		    for (gn3 = sfwid[sf]; gn3; gn3 = gnode_next(gn3)) {
			dn2 = (vithist_dagnode_t *) gnode_ptr(gn3);
			if (dn2->seqid >= 0) {
			    fprintf (fp, "%d %d %d\n", dn->seqid, sf-1, ve->ascr);
			    break;
			}
		    }
		}
	    }
	}
    }
    fprintf (fp, "End\n");
    
    /* Free dagnodes structure */
    for (f = 0; f <= vh->n_frm; f++) {
	for (gn = sfwid[f]; gn; gn = gnode_next(gn)) {
	    dn = (vithist_dagnode_t *) gnode_ptr(gn);
	    
	    glist_free (dn->velist);
	    ckd_free ((void *) dn);
	}
	
	glist_free (sfwid[f]);
    }
    ckd_free ((void *) sfwid);
}

/* 
 * RAH, free memory allocated in vithist_free 
 */
void vithist_free (vithist_t *v)
{

  if (v) {
    if(v->entry){
      ckd_free ((void *) v->entry);
    }

    if(v->frame_start)
      ckd_free ((void *) v->frame_start);

    if(v->bestscore)
      ckd_free ((void *) v->bestscore);

    if(v->bestvh)
      ckd_free ((void *) v->bestvh);

    if(v->lms2vh_root)
      ckd_free ((void *) v->lms2vh_root);    
    
    ckd_free ((void *) v);
  }

}

void vithist_report(vithist_t *vh)
{
  E_INFO_NOFN("Initialization of vithist_t, report:\n");
  if(vh){
    E_INFO_NOFN("Word beam = %d\n",vh->wbeam);
    E_INFO_NOFN("Bigram Mode =%d\n",vh->bghist);
    E_INFO_NOFN("Rescore Mode =%d\n",vh->bLMRescore);
    E_INFO_NOFN("Trace sil Mode =%d\n",vh->bBtwSil);
    E_INFO_NOFN("\n");
  }else{
    E_INFO_NOFN("Viterbi history is (null)\n");
  }
}

latticehist_t* latticehist_init(int32 init_alloc_size,
				int32 num_frames
				)
{
  latticehist_t *lat;
  lat=ckd_calloc(1, sizeof(latticehist_t));
    /* Allocate output word lattice structure */
  lat->lat_alloc = init_alloc_size;
  lat->lattice = (lattice_t *) ckd_calloc (lat->lat_alloc, sizeof(lattice_t));
  lat->n_lat_entry = 0;
  lat->n_cand=0; /*By default, we don't load in word candidate. */

  /* Space for first lattice entry in each frame (+ terminating sentinel) */
  lat->frm_latstart = (s3latid_t *) ckd_calloc (num_frames, sizeof(s3latid_t));

  return lat;
}

void latticehist_reset(latticehist_t *lathist)
{
  int32 l;
    /* Free rcscores for each lattice entry */
  for (l = 0; l < lathist->n_lat_entry; l++) {
    if (lathist->lattice[l].rcscore) {
      ckd_free (lathist->lattice[l].rcscore);
      lathist->lattice[l].rcscore = NULL;
    }
    if(lathist->lattice[l].rchistory) {
      ckd_free (lathist->lattice[l].rchistory);
      lathist->lattice[l].rchistory = NULL;
    }
  }
  lathist->n_lat_entry = 0;

}

void latticehist_free(latticehist_t *lathist)
{
  if(lathist){
    if(lathist->lattice)
      ckd_free(lathist->lattice);
    
    if(lathist->frm_latstart)
      ckd_free(lathist->frm_latstart);

    ckd_free(lathist);
  }
}

void latticehist_dump (latticehist_t *lathist, FILE *fp, dict_t *dict,ctxt_table_t *ct,int32 dumpRC)
{
    int32 i;
    s3cipid_t npid;
    s3cipid_t rc;
    lattice_t *lat;

    lat=lathist->lattice;

    for (i = 0; i < lathist->n_lat_entry; i++) {
      fprintf (fp, "%6d: %5d %6d %11d %s\n", 
	       i,
	       lat[i].frm, 
	       lat[i].history, 
	       lat[i].score,
	       dict_wordstr (dict,lat[i].wid));


      if(lat[i].rcscore && dumpRC){
	npid = get_rc_npid (ct,lat[i].wid,dict );
	for (rc = 0; rc < npid; rc++){
	  fprintf(fp,
		  "rc(Compacted) %d, rcscore %5d rchistory %6d\n",
		  rc, 
		  lat[i].rcscore[rc],
		  lat[i].rchistory[rc]);
	}
      }
    }
    fflush (fp);
}




/**
 * Record a word exit in word lattice.
 * NOTE: All exits from a single word in a given frame (for different right context
 * ciphones) must occur contiguously.
 */

void lattice_entry (latticehist_t *lathist, s3wid_t w, int32 f, int32 score, s3latid_t history, int32 rc, ctxt_table_t *ct, dict_t* dict)
{
    s3cipid_t rc_ind, npid;
    

    assert(lathist->lattice);
    if ((lathist->n_lat_entry <= 0) ||
	(lathist->lattice[lathist->n_lat_entry-1].wid != w) || (lathist->lattice[lathist->n_lat_entry-1].frm != f)) {
	/* New lattice entry */
	if (lathist->n_lat_entry >= lathist->lat_alloc) {
	    printf ("\n");
	    E_INFO("Lattice size(%d) exceeded; increasing to %d\n",
		   lathist->lat_alloc,lathist->lat_alloc+LAT_ALLOC_INCR);
	    lathist->lat_alloc += LAT_ALLOC_INCR;
	    lathist->lattice = ckd_realloc (lathist->lattice, lathist->lat_alloc * sizeof(lattice_t));
	}
	
	lathist->lattice[lathist->n_lat_entry].wid = w;
	lathist->lattice[lathist->n_lat_entry].frm = (s3frmid_t) f;
	lathist->lattice[lathist->n_lat_entry].score = score;
	lathist->lattice[lathist->n_lat_entry].history = history;

	/* Allocate space for different right context scores */
	npid = get_rc_npid (ct, w,dict );
	assert (npid > 0);

	lathist->lattice[lathist->n_lat_entry].rcscore = (int32 *) ckd_calloc (npid, sizeof(int32));

	for (rc_ind = 0; rc_ind < npid; rc_ind++)
	    lathist->lattice[lathist->n_lat_entry].rcscore[rc_ind] = S3_LOGPROB_ZERO;

	lathist->n_lat_entry++;

#if !SINGLE_RC_HISTORY
	/* ARCHAN: set up individual history for each right context */
	lathist->lattice[lathist->n_lat_entry].rchistory = (s3latid_t *) ckd_calloc (npid, sizeof(s3latid_t));
	for (rc_ind = 0; rc_ind < npid; rc_ind++)
	  lathist->lattice[lathist->n_lat_entry].rchistory[rc_ind] = BAD_S3LATID;
#endif

    }

    if (lathist->lattice[lathist->n_lat_entry-1].score < score) {
	lathist->lattice[lathist->n_lat_entry-1].score = score;
	lathist->lattice[lathist->n_lat_entry-1].history = history;
    }

#if SINGLE_RC_HISTORY
    lathist->lattice[lathist->n_lat_entry-1].rcscore[rc] = score;
#else

    /* ARCHAN, fix the bug where right context doesn't maintain its own history*/
    if(lathist->lattice[lathist->n_lat_entry-1].rcscore[rc] < score){
       lathist->lattice[lathist->n_lat_entry-1].rcscore[rc] = score;
       lathist->lattice[lathist->n_lat_entry-1].rchistory[rc] = history;
    }
#endif

}

int32 lat_pscr_rc (latticehist_t *lathist, s3latid_t l, s3wid_t w_rc, ctxt_table_t *ct, dict_t *dict)
{
    s3cipid_t *rcmap, rc;
    
    if ( (NOT_S3WID(w_rc)) || (! lathist->lattice[l].rcscore)  )
	return lathist->lattice[l].score;
    
    rcmap = get_rc_cimap (ct,lathist->lattice[l].wid,dict);
    rc = dict->word[w_rc].ciphone[0];
    return (lathist->lattice[l].rcscore[rcmap[rc]]);
}

/**
 * Find the path history for lattice entry l for the given right
 * context word.  If context word is BAD_S3WID it's a wild card;
 * return the history. 
 */

s3latid_t lat_pscr_rc_history (latticehist_t *lathist, s3latid_t l, s3wid_t w_rc, ctxt_table_t *ct, dict_t *dict)
{
    s3cipid_t *rcmap, rc;
    
    if ( (NOT_S3WID(w_rc)) || (! lathist->lattice[l].rchistory)  )
	return lathist->lattice[l].history;
    
    rcmap = get_rc_cimap (ct,lathist->lattice[l].wid,dict);
    rc = dict->word[w_rc].ciphone[0];
    return (lathist->lattice[l].rchistory[rcmap[rc]]);
}


/**
 * Get the last two non-filler, non-silence lattice words w0 and w1 (base word-ids),
 * starting from l.  w1 is later than w0.  At least w1 must exist; w0 may not.
 */
#if SINGLE_RC_HISTORY
void two_word_history (latticehist_t *lathist, s3latid_t l, s3wid_t *w0, s3wid_t *w1, dict_t *dict)
#else
void two_word_history (latticehist_t *lathist, s3latid_t l, s3wid_t *w0, s3wid_t *w1, s3wid_t w_rc, dict_t *dict, ctxt_table_t *ct)
#endif
{
    s3latid_t l0, l1;
    l0=0;

#if SINGLE_RC_HISTORY    
    for (l1 = l; dict_filler_word(dict, lathist->lattice[l1].wid); l1 = lathist->lattice[l1].history);
    
    /* BHIKSHA HACK - PERMIT MULTIPLE PRONS FOR <s> */
    if (l1 != -1) 
      for (l0 = lathist->lattice[l1].history; 
	   (IS_S3LATID(l0)) && (dict_filler_word(dict,lathist->lattice[l0].wid));
	   l0 = lathist->lattice[l0].history);
#else
    for (l1 = l; dict_filler_word(dict, lathist->lattice[l1].wid); l1 = lat_pscr_rc_history(lathist,l1,w_rc,ct,dict));

    if (l1 != -1) 
      for (l0 = lat_pscr_rc_history(lathist,l1,w_rc,ct,dict); 
	   (IS_S3LATID(l0)) && (dict_filler_word(dict,lathist->lattice[l0].wid));
	   l0 = lat_pscr_rc_history(lathist,l0,lathist->lattice[l1].wid,ct,dict));
#endif
    
    /* BHIKSHA HACK - PERMIT MULTIPLE PRONS FOR <s> */
    if (l1 == -1) *w1 = 0; else
      *w1 = dict_basewid(dict, lathist->lattice[l1].wid);
    if (l1 == -1) *w0 = BAD_S3WID; else
      *w0 = (NOT_S3LATID(l0)) ? BAD_S3WID : dict_basewid(dict,lathist->lattice[l0].wid);
}


/**
 * Find LM score for transition into lattice entry l.
 */
#if SINGLE_RC_HISTORY
int32 lat_seg_lscr (latticehist_t *lathist, s3latid_t l, lm_t *lm, dict_t *dict, ctxt_table_t *ct, fillpen_t *fpen, int32 isCand)
#else
int32 lat_seg_lscr (latticehist_t *lathist, s3latid_t l, s3wid_t w_rc, lm_t *lm, dict_t *dict, ctxt_table_t *ct, fillpen_t *fpen, int32 isCand)
#endif
{
    s3wid_t bw0, bw1, bw2;
    s3lmwid_t lw0;
    int32 lscr, bowt, bo_lscr;
    tg_t *tgptr;
    bg_t *bgptr;
    
    bw2 = dict_basewid (dict,lathist->lattice[l].wid);

    if (dict_filler_word (dict,bw2))
	return (fillpen(fpen,bw2));

#if SINGLE_RC_HISTORY    
    if (NOT_S3LATID(lathist->lattice[l].history)) {
	assert (bw2 == dict->startwid);
	return 0;
    }
    
    two_word_history (lathist, lathist->lattice[l].history, &bw0, &bw1,dict);
#else
    if (NOT_S3LATID(lat_pscr_rc_history(lathist,l,w_rc,ct,dict))) {
	assert (bw2 == dict->startwid);
	return 0;
    }
    
    two_word_history (lathist,
		      lat_pscr_rc_history(lathist,l,w_rc,ct,dict),
		      &bw0, &bw1,w_rc,dict,ct
		      );
#endif

    /*    E_INFO("lathist->lattice[l].history %d , bw0 %d, bw1 %d. bw2 %d\n",lathist->lattice[l].history,bw0,bw1,bw2);*/
    lw0 = IS_S3WID(bw0) ? lm->dict2lmwid[dict_basewid(dict,bw0)] : BAD_S3LMWID;
    lscr = lm_tg_score (lm, 
			lw0, 
			lm->dict2lmwid[dict_basewid(dict,bw1)], 
			lm->dict2lmwid[bw2],
			bw2);
    if (isCand)
	return lscr;

    /* Correction for backoff cpase if that scores better (see word_trans) */
    bo_lscr = 0;
    if ((IS_S3WID(bw0)) && (lm_tglist (lm,
				       lm->dict2lmwid[dict_basewid(dict,bw0)], 
				       lm->dict2lmwid[dict_basewid(dict,bw1)],
				       &tgptr, &bowt) > 0))
	bo_lscr = bowt;
    if (lm_bglist (lm, lm->dict2lmwid[dict_basewid(dict,bw1)], &bgptr, &bowt) > 0)
	bo_lscr += bowt;
    bo_lscr += lm_ug_score (lm,lm->dict2lmwid[dict_basewid(dict,bw2)], dict_basewid(dict,bw2));

    return ((lscr > bo_lscr) ? lscr : bo_lscr);
}



/**
 * Find acoustic and LM score for segmentation corresponding to lattice entry l with
 * the given right context word.
 */
void lat_seg_ascr_lscr (latticehist_t *lathist, 
			s3latid_t l, 
			s3wid_t w_rc, 
			int32 *ascr, 
			int32 *lscr, 
			lm_t *lm, 
			dict_t *dict, 
			ctxt_table_t *ct, 
			fillpen_t *fillpen)
{
    int32 start_score, end_score;

    /* Score with which l ended with right context = w_rc */
    if ((end_score = lat_pscr_rc (lathist,l, w_rc,ct,dict)) <= S3_LOGPROB_ZERO) {
	*ascr = *lscr = S3_LOGPROB_ZERO;
	return;
    }
    
    /* Score with which l was begun */
#if SINGLE_RC_HISTORY
    start_score = IS_S3LATID(lathist->lattice[l].history) ?
	lat_pscr_rc (lathist, lathist->lattice[l].history, lathist->lattice[l].wid, ct, dict) : 0;

    /* LM score for the transition into l */
    *lscr = lat_seg_lscr (lathist,l, lm,dict, ct, fillpen,(lathist->n_cand>0));
    *ascr = end_score - start_score - *lscr;
#else
    start_score = IS_S3LATID(lat_pscr_rc_history(lathist,l,w_rc,ct,dict)) ?
	lat_pscr_rc (lathist,
		     lat_pscr_rc_history(lathist,l,w_rc,ct,dict), 
		     lathist->lattice[l].wid,ct,dict) : 0;

    /* LM score for the transition into l */
    *lscr = lat_seg_lscr (lathist,l,w_rc, lm,dict,ct,fillpen, (lathist->n_cand>0));
    *ascr = end_score - start_score - *lscr;

#endif


}


s3latid_t lat_final_entry (latticehist_t* lathist,dict_t* dict,int32 curfrm, char* uttid)
{
    s3latid_t l, bestl;
    int32 f, bestscore;
    
    bestl=BAD_S3LATID;

    if(cmd_ln_int32("-bt_wsil")){

      /* Find lattice entry in last frame for FINISH_WORD */
      for (l = lathist->frm_latstart[curfrm-1]; l < lathist->n_lat_entry; l++){
	if (dict_basewid(dict,lathist->lattice[l].wid) == dict->finishwid)
	  break;
      }
      
      if (l < lathist->n_lat_entry) {
	/* FINISH_WORD entry found; backtrack to obtain best Viterbi path */
	return (l);
      }
    
      /* Find last available lattice entry with best ending score */
      E_WARN("When %s is used as final word, %s: Search didn't end in %s\n", dict_wordstr(dict,dict->finishwid), uttid, dict_wordstr(dict,dict->finishwid));
    }else{
    }


    bestscore = S3_LOGPROB_ZERO;
    for (f = curfrm-1; (f >= 0) && (bestscore <= S3_LOGPROB_ZERO); --f) {
	for (l = lathist->frm_latstart[f]; l < lathist->frm_latstart[f+1]; l++) {
	    if ((lathist->lattice[l].wid != dict->startwid) && (bestscore < lathist->lattice[l].score)) {
		bestscore = lathist->lattice[l].score;
		bestl = l;
	    }
	}
    }
    assert(! NOT_S3LATID(l));
    return ((f >= 0) ? bestl : BAD_S3LATID);
}



srch_hyp_t *lattice_backtrace (latticehist_t* lathist,
			       s3latid_t l, 
			       s3wid_t w_rc,
			       srch_hyp_t **hyp,
			       lm_t *lm, 
			       dict_t *dict, 
			       ctxt_table_t *ct, 
			       fillpen_t *fillpen)
{
    srch_hyp_t *h, *prevh;

    if (IS_S3LATID(l)) {
#if SINGLE_RC_HISTORY
      prevh = lattice_backtrace (lathist, lathist->lattice[l].history, lathist->lattice[l].wid, hyp,
				 lm,dict,ct,fillpen);
#else
      prevh = lattice_backtrace (lathist, lat_pscr_rc_history(lathist,l,w_rc,ct,dict), lathist->lattice[l].wid, hyp,
				 lm,dict,ct,fillpen);
#endif

	h = (srch_hyp_t *) listelem_alloc (sizeof(srch_hyp_t));
	if (! prevh)
	    *hyp = h;
	else
	    prevh->next = h;
	h->next = NULL;
	
	h->id = lathist->lattice[l].wid;
	h->word = dict_wordstr(dict,h->id);
	h->sf = prevh ? prevh->ef+1 : 0;
	h->ef = lathist->lattice[l].frm;
	h->pscr = lathist->lattice[l].score;
	lat_seg_ascr_lscr (lathist, l, w_rc, &(h->ascr), &(h->lscr), 
			   lm,dict,ct,fillpen);

	return h;
    } else {
	return NULL;
    }
}



int32 latticehist_dag_write (latticehist_t *lathist, 
			     char *dir, 
			     int32 onlynodes, 
			     char *id, 
			     char* latfile_ext, 
			     int32 totfrm, 
			     dag_t *dag,
			     lm_t *lm, 
			     dict_t *dict, 
			     ctxt_table_t *ct, 
			     fillpen_t *fillpen)

{

  /* WARNING!!!! DO NOT INSERT a # in the format arbitrarily because the dag_reader is not very robust */
    int32 i;
    dagnode_t *d, *initial, *final;
    daglink_t *l;
    char filename[2048];
    FILE *fp;
    int32 ascr, lscr;
    int32 ispipe;
    
    initial = dag->root;
    final = lathist->lattice[dag->latfinal].dagnode;

    sprintf (filename, "%s/%s.%s", dir, id, latfile_ext);
    E_INFO("Writing lattice file in Sphinx III format: %s\n", filename);
    if ((fp = fopen_comp (filename, "w", &ispipe)) == NULL) {
	E_ERROR("fopen_comp (%s,w) failed\n", filename);
	return -1;
    }
    
    dag_write_header(fp,totfrm,1);
    
    for (i = 0, d = dag->list; d; d = d->alloc_next, i++);
    fprintf (fp, "Nodes %d (NODEID WORD STARTFRAME FIRST-ENDFRAME LAST-ENDFRAME)\n", i);
    for (i = 0, d = dag->list; d; d = d->alloc_next, i++) {
	d->seqid = i;
	fprintf (fp, "%d %s %d %d %d\n", i, dict_wordstr(dict,d->wid), d->sf, d->fef, d->lef);
    }


    fprintf (fp, "#\n");

    fprintf (fp, "Initial %d\nFinal %d\n", initial->seqid, final->seqid);
    
    /* Best score (i.e., regardless of Right Context) for word segments in word lattice */
    fprintf (fp, "BestSegAscr %d (NODEID ENDFRAME ASCORE)\n", lathist->n_lat_entry);
    
    if (! onlynodes) {
	for (i = 0; i < lathist->n_lat_entry; i++) {
	    lat_seg_ascr_lscr (lathist, i, BAD_S3WID, &ascr, &lscr, lm,dict,ct,fillpen);
	    fprintf (fp, "%d %d %d\n",
		     (lathist->lattice[i].dagnode)->seqid, lathist->lattice[i].frm, ascr);
	}
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "Edges (FROM-NODEID TO-NODEID ASCORE)\n");
    if (! onlynodes) {
	for (d = dag->list; d; d = d->alloc_next) {
	    for (l = d->succlist; l; l = l->next)
		fprintf (fp, "%d %d %d\n", d->seqid, l->node->seqid, l->ascr);
	}
    }
    fprintf (fp, "End\n");

    fclose_comp (fp, ispipe);

    return 0;
}
