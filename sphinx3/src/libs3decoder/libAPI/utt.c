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
/************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 ************************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.29  2006/02/22  23:35:29  arthchan2003
 * Changed the code following Evandro's suggestion.
 * 
 * Revision 1.28  2006/02/22 22:46:51  arthchan2003
 * Merged from branch: SPHINX3_5_2_RCI_IRII: Change prototypes of utt_decode, this allow utterance-base resource (lm, mllr, file id) allocation to be carried out.
 *
 * Revision 1.27.4.3  2005/09/26 02:19:06  arthchan2003
 * 1, Forced exit when the decoder cannot find a file, 2, fixed dox-doc.
 *
 * Revision 1.27.4.2  2005/09/25 18:57:15  arthchan2003
 * Changed srch_utt_decode_blk from a FATAL to an ERROR. This corresponds to the problem of putting history pointer to the vithistory routine.
 *
 * Revision 1.27.4.1  2005/07/27 23:16:26  arthchan2003
 * 1, Fixed dox-doc, 2, Move set_lm and setmllr to utt_decode.
 *
 * Revision 1.27  2005/06/22 02:54:55  arthchan2003
 * Log. hand the implementation of utt_begin, utt_decode and utt_end to
 * srch, utt.c now only maintain a wrapper for search operation. In
 * future, I expect it to become the prototype of batch mode API of search.
 *
 * Revision 1.9  2005/05/04 05:15:25  archan
 * reverted the last change, seems to be not working because of compilation issue. Try not to deal with it now.
 *
 * Revision 1.8  2005/05/04 04:46:04  archan
 * Move srch.c and srch.h to search. More and more this type of refactoring will be done in future
 *
 * Revision 1.7  2005/04/20 03:45:30  archan
 * utt.c pass all the control of the searching to srch.c, from now on the code will only expose APIs for batch mode decoding
 *
 * Revision 1.6  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 *
 * 15-Jun-2004  Yitao Sun (yitao@cs.cmu.edu) at Carnegie Mellon University
 *              Modified utt_end() to save hypothesis in the kb structure.
 *
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Added utt_decode_block() to allow block-based decoding 
 *		and decoding of piped input.
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Moved all utt_*() routines into utt.c to make them independent
 *		of main() during compilation
 * 
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to allow runtime choice between 3-state and 5-state
 *              HMM topologies (instead of compile-time selection).
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxwpf.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

#ifdef WIN32
#include <direct.h>		/* RAH, added */
#endif

#include "kb.h"
#include "corpus.h"
#include "utt.h"
#include "logs3.h"
#include "srch.h"

/*
 * Begin search at bigrams of <s>, backing off to unigrams; and fillers. 
 * Update kb->lextree_next_active with the list of active lextrees.
 */
void utt_begin (kb_t *kb)
{
  srch_utt_begin((srch_t*) kb->srch);
}

void utt_end (kb_t *kb)
{
  srch_utt_end((srch_t*) kb->srch);
}

void utt_decode (void *data, utt_res_t *ur, int32 sf, int32 ef, char *uttid)
{
  kb_t *kb;
  kbcore_t *kbcore;
  int32 num_decode_frame;
  int32 total_frame;
  stat_t *st;
  
  num_decode_frame=0;
  E_INFO("Processing: %s\n", uttid);

  kb = (kb_t *) data;
  kbcore = kb->kbcore;
  kb->uttid = uttid;
  st = kb->stat;

  
  /* Read mfc file and build feature vectors for entire utterance */
  if((total_frame = feat_s2mfc2feat(kbcore_fcb(kbcore), ur->uttfile, 
				    cmd_ln_str("-cepdir"), cmd_ln_str("-cepext"),
				    sf, ef, kb->feat, S3_MAX_FRAMES)) < 0)
    {
      E_FATAL("Cannot read file %s. Forced exit\n",ur->uttfile);
    }

  /* Also need to make sure we don't set resource if it is the same. Well, this mechanism
     could be provided inside the following function. 
   */
  if(ur->lmname!=NULL) srch_set_lm((srch_t*)kb->srch,ur->lmname);
  if(ur->regmatname!=NULL) kb_setmllr(ur->regmatname,ur->cb2mllrname,kb);

  utt_begin (kb);
  utt_decode_block(kb->feat,total_frame,&num_decode_frame,kb);
  utt_end (kb);

  st->tot_fr += st->nfr;
}


/** This function decodes a block of incoming feature vectors.
 * Feature vectors have to be computed by the calling routine.
 * The utterance level index of the last feature vector decoded
 * (before the current block) must be passed. 
 * The current status of the decode is stored in the kb structure that 
 * is passed in.
 */

void utt_decode_block (float ***block_feat,  /* Incoming block of featurevecs */
		       int32 no_frm, /* No. of vecs in cepblock */
		       int32 *curfrm,	     /* Utterance level index of
						frames decoded so far */
		       kb_t *kb  	     /* kb structure with all model
						and decoder info */
		       )
{

  srch_t* s;
  s=(srch_t*) kb->srch;
  s->uttid = kb->uttid;

  if(srch_utt_decode_blk(s,block_feat,no_frm,curfrm)==SRCH_FAILURE){
    E_ERROR("srch_utt_decode_blk failed. \n");
  }
}

