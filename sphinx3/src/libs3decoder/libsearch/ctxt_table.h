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
 * ctxt_table.h -- Phone Context Table Structure
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 14-Jul-05    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity 
 *              First created it. 
 *
 * $Log$
 * Revision 1.1.2.1  2005/07/15  07:48:32  arthchan2003
 * split the hmm (whmm_t) and context building process (ctxt_table_t) from the the flat_fwd.c
 * 
 *
 */

/* 
 * \file ctxt_table.h
 * \brief data structure for building cross word triphones for Sphinx 3. 
 */

#ifndef _CTX_TAB_
#define _CTX_TAB_

#include <s3types.h>
#include <prim_type.h>
#include <mdef.h>
#include <dict.h>

/**
 * Triphone information in the flat lexicon search in Sphinx 3.0
 * for all word hmm modelling broken up into 4 cases:
 * 	within-word triphones
 * 	left-context cross-word triphones (multi-phone words)
 * 	right-context cross-word triphones (multi-phone words)
 * 	left- and right-cross-word triphones (single-phone words)
 * These 4 cases captured by the following data structures.
 */

/**
 * \struct xwdpid_t
 * \brief cross word triphone model structure 
 */

typedef struct {
    s3pid_t   *pid;	/**< Pid list for all context ciphones; compressed, unique */
    s3cipid_t *cimap;	/**< Index into pid[] above for each ci phone */
    int32    n_pid;	/**< #Unique pid in above, compressed pid list */
} xwdpid_t;



/**
 * \struct ctxt_table_t 
 *
 * First, the within word triphone models.  wwpid[w] = list of
 * triphone pronunciations for word w.  Since left and right extremes
 * require cross-word modelling (see below), wwpid[w][0] and
 * wwpid[w][pronlen-1] contain no information and shouldn't be
 * touched.
 *
 * Left context mapping (for multiphone words): given the 1st base phone, b, of a word
 * and its right context, r, the triphone for any left context l =
 *     lcpid[b][r].pid[lcpid[b][r].cimap[l]].
 * 
 * Similarly, right context mapping (for multiphone words): given b and left context l,
 * the triphone for any right context r =
 *     rcpid[b][l].pid[lcpid[b][l].cimap[r]].
 * 
 * A single phone word is a combination of the above, where both l and r are unknown.
 * Triphone, given any l and r context ciphones:
 *     lrcpid[b][l].pid[lcpid[b][l].cimap[r]].
 * For simplicity, all cimap[] vectors (for all l) must be identical.  For now, this is
 * attained by avoiding any compression and letting cimap be the identity map.
 * 
 * Reason for compressing pid[] and indirect access: senone sequences for triphones not
 * distinct.  Hence, cross-word modelling fanout at word exits can be limited by fanning
 * out to only distinct ones and sharing the results among all ciphones.
 *
 */

typedef struct {
  xwdpid_t **lcpid; /**< Left context phone id table */
  xwdpid_t **rcpid; /**< right context phone id table */
  xwdpid_t **lrcpid; /**< left-right context phone id table */


  s3pid_t **wwpid;  /**< Within word triphone models */
  int32 n_backoff_ci; /**< # of backoff CI phone */
} ctxt_table_t ;

/**
 * Initialize a context table 
 */

ctxt_table_t *ctxt_table_init(dict_t *dict,  /**< A dictionary*/
			      mdef_t *mdef   /**< A model definition*/
			      );
/**
 * Get the array of right context phone ID for the last phone. 
 */
void get_rcpid (ctxt_table_t *ct,  /**< A context table */
		s3wid_t w,         /**< A word for query */
		s3pid_t **pid,     /**< Out: An array of right context phone ID */
		int32 *npid,        /**< Out: Number of phone ID */
		dict_t *dict        /**< In: a dictionary */
		);

/**
 * Get the context-independent phone map for the last phone of a
 * parcitular word 
 * @return an array of ciphone ID. 
 */
s3cipid_t *get_rc_cimap (ctxt_table_t *ct, /**< A context table */
			 s3wid_t w, /**< A word for query*/
			 dict_t *dict /**< A dictionary */
			 );

/**
 * Get number of right context for the last phone of a word. 
 * @return number of right context 
 *
 */
int32 get_rc_npid (ctxt_table_t *ct,  /**< A context table */
		   s3wid_t w,          /**< Word for query. */
		   dict_t *dict        /**< A dictionary */
		   );

#endif /*_CTX_TAB_*/
