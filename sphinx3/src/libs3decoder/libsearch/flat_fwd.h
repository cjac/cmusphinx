/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * fwd.c -- Forward Viterbi beam search
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 28-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity 
 *              First created it. 
 *
 * $Log$
 * Revision 1.12  2006/02/23  05:25:32  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Most of the functions are removed and now resided in srch_flat_fwd.h
 * 
 * Revision 1.10.4.7  2005/11/17 06:27:48  arthchan2003
 * 1, Clean up. 2, removed fwg in dag_build.
 *
 * Revision 1.10.4.6  2005/10/26 03:53:12  arthchan2003
 * Put add_fudge and remove_filler_nodes into srch_flat_fwd.c . This conformed to s3.0 behavior.
 *
 * Revision 1.10.4.5  2005/09/18 01:18:24  arthchan2003
 * Only retain processing of the array whmm_t in flat_fwd.[ch]
 *
 * Revision 1.10.4.4  2005/09/11 02:58:10  arthchan2003
 * remove most dag-related functions except dag_build. Use latticehist_t insteads of loosed arrays.
 *
 * Revision 1.10.4.3  2005/09/07 23:40:06  arthchan2003
 * Several Bug Fixes and Enhancements to the flat-lexicon
 * 1, Fixed Dox-doc.
 * 2, Add -worddumpef and -hmmdumpef in parrallel to -worddumpsf and
 * -hmmdumpsf. Usage is trivial. a structure called fwd_dbg_t now wrapped
 * up all these loose parameters.  Methods of fwd_dbg are implemented.
 * 3, word_ugprob is now initialized by init_word_ugprob
 * 4, Full-triphone expansion is implemented. User can change this
 * behavior by specifying -multiplex_multi and -multiplex_single. The
 * former turn on multiplex triphone for word-begin for multi-phone word.
 * The latter do that for single-phone word. Turning off both could
 * tremendously increase computation.
 * 5, Word expansions of possible right contexts now records independent
 * history.  The behavior in the past was to use only one history for a
 * word.
 *
 * Revision 1.10.4.2  2005/08/02 21:12:45  arthchan2003
 * Changed senlist from 8-bit to 32-bit. It will be compatible to the setting of ascr's sen_active.
 *
 * Revision 1.10.4.1  2005/07/15 07:50:33  arthchan2003
 * Remove hmm computation and context building code from flat_fwd.c.
 *
 *
 */

#ifndef _LIBFBS_FWD_H_
#define _LIBFBS_FWD_H_

/* Added by BHIKSHA; Fix for 3 state hmms? 
#define ANYHMMTOPO	1
 End modification by BHIKSHA */
#include "dag.h"
#include "vithist.h"

/** \file flat_fwd.h
   \brief (Currently not opened to public) Header for forward search for flat lexicon

 * SOME ASSUMPTIONS
 *   - All phones (ciphones and triphones) have same HMM topology with n_state states.
 *   - Initial state = state 0; final state = state n_state-1.
 *   - Final state is a non-emitting state with no arcs out of it.
 *   - Some form of Bakis topology (ie, no cycles, except for self-transitions).
 */

/**
 * The flat forward search version of DAG search. Very similar to what
 * one could find in s3dag_dag_search. 
 */

srch_hyp_t *s3flat_fwd_dag_search (char *utt /**< utterance id */
				   );

/**
 * Build a DAG from the lattice: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * dagnodes_list = linear list of DAG nodes allocated, ordered such that nodes earlier
 * in the list can follow nodes later in the list, but not vice versa:  Let two DAG
 * nodes d1 and d2 have start times sf1 and sf2, and end time ranges [fef1..lef1] and
 * [fef2..lef2] respectively.  If d1 appears later than d2 in dag.list, then
 * fef2 >= fef1, because d2 showed up later in the word lattice.  If there is a DAG
 * edge from d1 to d2, then sf1 > fef2.  But fef2 >= fef1, so sf1 > fef1.  Reductio ad
 * absurdum.
 */

dag_t* dag_build (s3latid_t endid, latticehist_t * lathist, dict_t *dict, lm_t *lm, ctxt_table_t* ctxt, fillpen_t* fpen, int32 n_frm);



void flat_fwd_dag_remove_filler_nodes (dag_t* dag, 
				       latticehist_t *lathist, 
				       float64 lwf, 
				       lm_t *lm, 
				       dict_t* dict, ctxt_table_t *ct_table, fillpen_t *fpen);
#endif /* _LIBFBS_FWD_H_*/

