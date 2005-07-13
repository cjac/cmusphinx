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
 * word_fsg.c -- Finite state LM handling
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2003 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1.2.3  2005/07/13  18:39:48  arthchan2003
 * (For Fun) Remove the hmm_t hack. Consider each s2 global functions one-by-one and replace them by sphinx 3's macro.  There are 8 minor HACKs where functions need to be removed temporarily.  Also, there are three major hacks. 1,  there are no concept of "phone" in sphinx3 dict_t, there is only ciphone. That is to say we need to build it ourselves. 2, sphinx2 dict_t will be a bunch of left and right context tables.  This is currently bypass. 3, the fsg routine is using fsg_hmm_t which is just a duplication of CHAN_T in sphinx2, I will guess using hmm_evaluate should be a good replacement.  But I haven't figure it out yet.
 * 
 * Revision 1.1.2.2  2005/06/28 07:01:20  arthchan2003
 * General fix of fsg routines to make a prototype of fsg_init and fsg_read. Not completed.  The number of empty functions in fsg_search is now decreased from 35 to 30.
 *
 * Revision 1.1.2.1  2005/06/27 05:26:29  arthchan2003
 * Sphinx 2 fsg mainpulation routines.  Compiled with faked functions.  Currently fended off from users.
 *
 * Revision 1.2  2004/07/23 23:36:34  egouvea
 * Ravi's merge, with the latest fixes in the FSG code, and making the log files generated by FSG, LM, and allphone have the same 'look and feel', with the backtrace information presented consistently
 *
 * Revision 1.8  2004/07/20 20:48:40  rkm
 * Added uttproc_load_fsg().
 *
 * Revision 1.7  2004/07/20 13:40:55  rkm
 * Added FSG get/set start/final state functions.
 *
 * Revision 1.1  2004/07/16 00:57:12  egouvea
 * Added Ravi's implementation of FSG support.
 *
 * Revision 1.6  2004/07/12 18:47:43  rkm
 * *** empty log message ***
 *
 * Revision 1.5  2004/06/21 18:16:12  rkm
 * Omitted noise words from FSG if noise penalty = 0
 *
 * Revision 1.4  2004/06/21 18:12:19  rkm
 * *** empty log message ***
 *
 * Revision 1.3  2004/06/21 18:09:17  rkm
 * *** empty log message ***
 *
 * Revision 1.2  2004/05/27 14:22:57  rkm
 * FSG cross-word triphones completed (but for single-phone words)
 *
 * Revision 1.1.1.1  2004/03/01 14:30:30  rkm
 *
 *
 * Revision 1.8  2004/02/27 19:33:01  rkm
 * *** empty log message ***
 *
 * Revision 1.7  2004/02/27 16:15:13  rkm
 * Added FSG switching
 *
 * Revision 1.6  2004/02/27 15:05:21  rkm
 * *** empty log message ***
 *
 * Revision 1.5  2004/02/26 15:35:50  rkm
 * *** empty log message ***
 *
 * Revision 1.4  2004/02/26 01:14:48  rkm
 * *** empty log message ***
 *
 * Revision 1.3  2004/02/25 15:08:19  rkm
 * *** empty log message ***
 *
 * Revision 1.2  2004/02/24 18:13:05  rkm
 * Added NULL transition handling
 *
 * Revision 1.1  2004/02/23 15:53:45  rkm
 * Renamed from fst to fsg
 *
 * Revision 1.6  2004/02/19 21:16:54  rkm
 * Added fsg_search.{c,h}
 *
 * Revision 1.5  2004/02/17 21:11:49  rkm
 * *** empty log message ***
 *
 * Revision 1.4  2004/02/16 21:10:10  rkm
 * *** empty log message ***
 *
 * Revision 1.3  2004/02/09 21:19:18  rkm
 * *** empty log message ***
 *
 * Revision 1.2  2004/02/09 17:30:20  rkm
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/03 21:08:05  rkm
 * *** empty log message ***
 *
 * 
 * 13-Jan-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <s3types.h>
#include <err.h>
#include <str2words.h>
#include <ckd_alloc.h>
#include <dict.h>
#include <logs3.h>
#include <word_fsg.h>


#define __FSG_DBG__		0


#define WORD_FSG_MAX_WORDPTR	4096

#define WORD_FSG_BEGIN_DECL		"FSG_BEGIN"
#define WORD_FST_BEGIN_DECL		"FST_BEGIN"
#define WORD_FSG_END_DECL		"FSG_END"
#define WORD_FST_END_DECL		"FST_END"
#define WORD_FSG_N_DECL			"N"
#define WORD_FSG_NUM_STATES_DECL	"NUM_STATES"
#define WORD_FSG_S_DECL			"S"
#define WORD_FSG_START_STATE_DECL	"START_STATE"
#define WORD_FSG_F_DECL			"F"
#define WORD_FSG_FINAL_STATE_DECL	"FINAL_STATE"
#define WORD_FSG_T_DECL			"T"
#define WORD_FSG_TRANSITION_DECL	"TRANSITION"
#define WORD_FSG_COMMENT_CHAR		'#'


/*#if ARTHUR_CHANGE */
#if 1
/*#define EXP(x)      (exp ((double) (x) * LOG_BASE))*/

#define EXP(x)      (exp ((double) (x) ))
#endif



static int32 nextline_str2words (FILE *fp, int32 *lineno,
				 char **wordptr, int32 max_ptr)
{
  char line[16384];
  int32 n;
  
  for (;;) {
    if (fgets (line, sizeof(line), fp) == NULL)
      return -1;
    
    (*lineno)++;
    
    if (line[0] != WORD_FSG_COMMENT_CHAR) {	/* Skip comment lines */
      if ((n = str2words(line, wordptr, max_ptr)) < 0)
	E_FATAL("Line[%d] too long\n", *lineno);
      
      if (n > 0)				/* Skip blank lines */
	break;
    }
  }
  
  return n;
}


/*
 * Add the given transition to the FSG transition matrix.  Duplicates (i.e.,
 * two transitions between the same states, with the same word label) are
 * flagged and only the highest prob retained.
 */
static void word_fsg_trans_add (word_fsg_t *fsg,
				int32 from, int32 to, int32 logp,
				int32 wid)
{
  word_fsglink_t *link;
  gnode_t *gn;
  
  /* Check for duplicate link (i.e., link already exists with label=wid) */
  for (gn = fsg->trans[from][to]; gn; gn = gnode_next(gn)) {
    link = (word_fsglink_t *) gnode_ptr(gn);
    
    if (link->wid == wid) {
#if 0
      E_WARN("Duplicate transition %d -> %d ('%s'); highest prob kept\n",
	     from, to, dict_wordstr(fsg->dict,wid));
#endif
      if (link->logs2prob < logp)
	link->logs2prob = logp;
      return;
    }
  }
  
  /* Create transition object */
  link = (word_fsglink_t *) ckd_calloc (1, sizeof(word_fsglink_t));
  link->from_state = from;
  link->to_state = to;
  link->logs2prob = logp;
  link->wid = wid;
  
  fsg->trans[from][to] = glist_add_ptr(fsg->trans[from][to], (void *)link);
}


/*
 * Link word_fsg_trans_add, but for a null transition between the given
 * states.  Also, there can be at most one null transition between the
 * given states; duplicates are flagged and only the best prob retained.
 * Transition probs must be <= 1 (i.e., logprob <= 0).
 * Return value: 1 if a new transition was added, 0 if the prob of an existing
 * transition was upgraded; -1 if nothing was changed.
 */
static int32 word_fsg_null_trans_add (word_fsg_t *fsg,
				     int32 from, int32 to, int32 logp)
{
  word_fsglink_t *link;
  
  /* Check for transition probability */
  if (logp > 0) {
    E_FATAL("Null transition prob must be <= 1.0 (state %d -> %d)\n",
	    from, to);
  }
  
  /* Self-loop null transitions (with prob <= 1.0) are redundant */
  if (from == to)
    return -1;
  
  /* Check for a duplicate link; if found, keep the higher prob */
  link = fsg->null_trans[from][to];
  if (link) {
    assert (link->wid < 0);
#if 0
    E_WARN("Duplicate null transition %d -> %d; highest prob kept\n",
	   from, to);
#endif
    if (link->logs2prob < logp) {
      link->logs2prob = logp;
      return 0;
    } else
      return -1;
  }
  
  /* Create null transition object */
  link = (word_fsglink_t *) ckd_calloc (1, sizeof(word_fsglink_t));
  link->from_state = from;
  link->to_state = to;
  link->logs2prob = logp;
  link->wid = -1;
  
  fsg->null_trans[from][to] = link;
  
  return 1;
}


/*
 * Obtain transitive closure of NULL transitions in the given FSG.  (Initial
 * list of such transitions is given.)
 * Return value: Updated list of null transitions.
 */
static glist_t word_fsg_null_trans_closure (word_fsg_t *fsg,
					    glist_t nulls)
{
  gnode_t *gn1, *gn2;
  boolean updated;
  word_fsglink_t *tl1, *tl2;
  int32 k, n;
  
  E_INFO("Computing transitive closure for null transitions\n");

  /*
   * Probably not the most efficient closure implementation, in general, but
   * probably reasonably efficient for a sparse null transition matrix.
   */
  n = 0;
  do {
    updated = FALSE;
    
    for (gn1 = nulls; gn1; gn1 = gnode_next(gn1)) {
      tl1 = (word_fsglink_t *) gnode_ptr(gn1);
      assert (tl1->wid < 0);
      
      for (gn2 = nulls; gn2; gn2 = gnode_next(gn2)) {
	tl2 = (word_fsglink_t *) gnode_ptr(gn2);
	
	if (tl1->to_state == tl2->from_state) {
	  k = word_fsg_null_trans_add (fsg,
				       tl1->from_state,
				       tl2->to_state,
				       tl1->logs2prob + tl2->logs2prob);
	  if (k >= 0) {
	    updated = TRUE;
	    if (k > 0) {
	      nulls = glist_add_ptr (nulls, (void *) fsg->null_trans[tl1->from_state][tl2->to_state]);
	      n++;
	    }
	  }
	}
      }
    }
  } while (updated);
  
  E_INFO("%d null transitions added\n", n);
  
  return nulls;
}


/*
 * Add silence and noise filler words to the FSG.
 * Return the number of transitions added..
 */
static int32 word_fsg_add_filler (word_fsg_t *fsg,
				  float32 silprob, float32 fillprob)
{
  dict_t *dict;
  int32 src;
  int32 wid, silwid, n_word;
  int32 n_trans;
  int32 logsilp, logfillp;
  
  E_INFO("Adding filler words to FSG\n");
  
  assert (fsg);
  
  dict = fsg->dict;
  silwid = dict_silwid(dict);
  n_word = dict_size(dict);

  logsilp = (int32) (logs3(silprob) * fsg->lw);
  logfillp = (int32) (logs3(fillprob) * fsg->lw);
  
  /*
   * Add silence and filler word self-loop transitions to each state.
   * NOTE: No check to see if these words already exist in FSG!
   */
  n_trans = 0;
  if (silwid >= 0) {
    for (src = 0; src < fsg->n_state; src++) {
      word_fsg_trans_add (fsg, src, src, logsilp, silwid);
      n_trans++;
      
      if (fillprob > 0.0) {
	/* Add other filler (noise) words */
	for (wid = silwid+1; wid < n_word; wid++) {
	  word_fsg_trans_add (fsg, src, src, logfillp, wid);
	  n_trans++;
	}
      }
    }
  }

  return n_trans;
}


/*
 * Compute the left and right context CIphone sets for each state.
 * (Needed for building the phone HMM net using cross-word triphones.  Invoke
 * after computing null transitions closure.)
 */
static void word_fsg_lc_rc (word_fsg_t *fsg)
{
  int32 s, d, i, j;
  int32 n_ci;
  gnode_t *gn;
  word_fsglink_t *l;
  int32 silcipid;
  int32 endwid;
  int32 len;
  dict_t* dict;
  mdef_t* mdef;
  
  dict = fsg->dict;
  mdef = fsg->mdef;

  assert(fsg);
  assert(dict);
  assert(mdef);
  endwid = dict_basewid(dict, dict_finishwid(dict));
  
  silcipid = mdef_silphone(mdef);
  assert (silcipid >= 0);
  n_ci = fsg->n_ciphone;
  if (n_ci > 127) {
    E_FATAL("#phones(%d) > 127; cannot use int8** for word_fsg_t.{lc,rc}\n",
	    n_ci);
  }

  /*
   * fsg->lc[s] = set of left context CIphones for state s.  Similarly, rc[s]
   * for right context CIphones.
   */
  fsg->lc = (int8 **) ckd_calloc_2d (fsg->n_state, n_ci+1, sizeof(int8));
  fsg->rc = (int8 **) ckd_calloc_2d (fsg->n_state, n_ci+1, sizeof(int8));
  
  for (s = 0; s < fsg->n_state; s++) {
    for (d = 0; d < fsg->n_state; d++) {
      for (gn = fsg->trans[s][d]; gn; gn = gnode_next(gn)) {
	l = (word_fsglink_t *) gnode_ptr(gn);
	assert (l->wid >= 0);
	
	/*
	 * Add the first CIphone of l->wid to the rclist of state s, and
	 * the last CIphone to lclist of state d.
	 * (Filler phones are a pain to deal with.  There is no direct
	 * marking of a filler phone; but only filler words are supposed to
	 * use such phones, so we use that fact.  HACK!!  FRAGILE!!)
	 */
	if (dict_filler_word(dict, l->wid) || (l->wid == endwid)) {
	  /* Filler phone; use silence phone as context */
	  fsg->rc[s][silcipid] = 1;
	  fsg->lc[d][silcipid] = 1;
	} else {
	  len = dict_pronlen(dict, l->wid);
	  fsg->rc[s][dict_pron(dict, l->wid, 0)] = 1;
	  fsg->lc[d][dict_pron(dict, l->wid, len-1)] = 1;
	}
      }
    }
    
    /*
     * Add SIL phone to the lclist and rclist of each state.  Strictly
     * speaking, only needed at start and final states, respectively, but
     * all states considered since the user may change the start and final
     * states.  In any case, most applications would have a silence self
     * loop at each state, hence these would be needed anyway.
     */
    fsg->lc[s][silcipid] = 1;
    fsg->rc[s][silcipid] = 1;
  }
  
  /*
   * Propagate lc and rc lists past null transitions.  (Since FSG contains
   * null transitions closure, no need to worry about a chain of successive
   * null transitions.  Right??)
   */
  for (s = 0; s < fsg->n_state; s++) {
    for (d = 0; d < fsg->n_state; d++) {
      l = fsg->null_trans[s][d];
      if (l) {
	/*
	 * lclist(d) |= lclist(s), because all the words ending up at s, can
	 * now also end at d, becoming the left context for words leaving d.
	 */
	for (i = 0; i < n_ci; i++)
	  fsg->lc[d][i] |= fsg->lc[s][i];
	/*
	 * Similarly, rclist(s) |= rclist(d), because all the words leaving d
	 * can equivalently leave s, becoming the right context for words
	 * ending up at s.
	 */
	for (i = 0; i < n_ci; i++)
	  fsg->rc[s][i] |= fsg->rc[d][i];
      }
    }
  }
  
  /* Convert the bit-vector representation into a list */
  for (s = 0; s < fsg->n_state; s++) {
    j = 0;
    for (i = 0; i < n_ci; i++) {
      if (fsg->lc[s][i]) {
	fsg->lc[s][j] = i;
	j++;
      }
    }
    fsg->lc[s][j] = -1;	/* Terminate the list */

    j = 0;
    for (i = 0; i < n_ci; i++) {
      if (fsg->rc[s][i]) {
	fsg->rc[s][j] = i;
	j++;
      }
    }
    fsg->rc[s][j] = -1;	/* Terminate the list */
  }
}


word_fsg_t *word_fsg_load (s2_fsg_t *fsg,
			   boolean use_altpron, boolean use_filler,
			   float32 silprob, float32 fillprob,
			   float32 lw,dict_t *dict, mdef_t * mdef)
{
  word_fsg_t *word_fsg;
  s2_fsg_trans_t *trans;
  int32 n_trans, n_null_trans, n_alt_trans, n_filler_trans, n_unk;
  int32 wid;
  int32 logp;
  glist_t nulls;
  int32 i, j;
  
  assert (fsg);
  assert (dict);
  
  /* Some error checking */
  if (lw <= 0.0)
    E_WARN("Unusual language-weight value: %.3e\n", lw);
  if (use_filler && ((silprob < 0.0) || (fillprob < 0.0))) {
    E_ERROR("silprob/fillprob must be >= 0\n");
    return NULL;
  }
  if ((fsg->n_state <= 0)
      || ((fsg->start_state < 0) || (fsg->start_state >= fsg->n_state))
      || ((fsg->final_state < 0) || (fsg->final_state >= fsg->n_state))) {
    E_ERROR("Bad #states/start_state/final_state values: %d/%d/%d\n",
	    fsg->n_state, fsg->start_state, fsg->final_state);
    return NULL;
  }
  for (trans = fsg->trans_list; trans; trans = trans->next) {
    if ((trans->from_state < 0) || (trans->from_state >= fsg->n_state)
	|| (trans->to_state < 0) || (trans->to_state >= fsg->n_state)
	|| (trans->prob <= 0) || (trans->prob > 1.0)) {
      E_ERROR("Bad transition: P(%d -> %d) = %e\n",
	      trans->from_state, trans->to_state, trans->prob);
      return NULL;
    }
  }
  
  
  word_fsg = (word_fsg_t *) ckd_calloc (1, sizeof(word_fsg_t));
  word_fsg->name = ckd_salloc(fsg->name ? fsg->name : "");
  word_fsg->n_state = fsg->n_state;
  word_fsg->start_state = fsg->start_state;
  word_fsg->final_state = fsg->final_state;
  word_fsg->use_altpron = use_altpron;
  word_fsg->use_filler = use_filler;
  word_fsg->lw = lw;
  word_fsg->lc = NULL;
  word_fsg->rc = NULL;
  word_fsg->dict = dict;
  word_fsg->mdef = mdef;
  
  /* Allocate non-epsilon transition matrix array */
  word_fsg->trans = (glist_t **) ckd_calloc_2d (word_fsg->n_state,
						word_fsg->n_state,
						sizeof(glist_t));
  /* Allocate epsilon transition matrix array */
  word_fsg->null_trans = (word_fsglink_t ***)
    ckd_calloc_2d(word_fsg->n_state, word_fsg->n_state,
		  sizeof(word_fsglink_t *));
  
  /* Process transitions */
  n_null_trans = 0;
  n_alt_trans = 0;
  n_filler_trans = 0;
  n_unk = 0;
  nulls = NULL;
  
  for (trans = fsg->trans_list, n_trans = 0;
       trans;
       trans = trans->next, n_trans++) {
    /* Convert prob to logs2prob and apply language weight */
    logp = (int32) (logs3(trans->prob) * lw);
    
    /* Check if word is in dictionary */
    if (trans->word) {
      wid = dict_wordid(dict,trans->word);
      if (wid < 0) {
	E_ERROR("Unknown word '%s'; ignored\n", trans->word);
	n_unk++;
      } else if (use_altpron) {
	wid = dict_basewid(dict, wid);
	assert (wid >= 0);
      }
    } else
      wid = -1;		/* Null transition */
    
    /* Add transition to word_fsg structure */
    i = trans->from_state;
    j = trans->to_state;
    if (wid < 0) {
      if (word_fsg_null_trans_add (word_fsg, i, j, logp) == 1) {
	n_null_trans++;
	nulls = glist_add_ptr (nulls, (void *) word_fsg->null_trans[i][j]);
      }
    } else {
      word_fsg_trans_add (word_fsg, i, j, logp, wid);
      
      /* Add transitions for alternative pronunciations, if any */
      if (use_altpron) {
	for (wid = dict_nextalt(dict, wid);
	     wid >= 0;
	     wid = dict_nextalt(dict, wid)) {
	  word_fsg_trans_add (word_fsg, i, j, logp, wid);
	  n_alt_trans++;
	  n_trans++;
	}
      }
    }
  }
  
  /* Add silence and noise filler word transitions if specified */
  if (use_filler) {
    n_filler_trans = word_fsg_add_filler (word_fsg, silprob, fillprob);
    n_trans += n_filler_trans;
  }
  
  E_INFO("FSG: %d states, %d transitions (%d null, %d alt, %d filler,  %d unknown)\n",
	 word_fsg->n_state, n_trans,
	 n_null_trans, n_alt_trans, n_filler_trans, n_unk);
  
#if __FSG_DBG__
  E_INFO("FSG before NULL closure:\n");
  word_fsg_write (word_fsg, stdout);
#endif

  /* Null transitions closure */
  nulls = word_fsg_null_trans_closure (word_fsg, nulls);
  glist_free (nulls);
  
#if __FSG_DBG__
  E_INFO("FSG after NULL closure:\n");
  word_fsg_write (word_fsg, stdout);
#endif
  
  /* Compute left and right context CIphone lists for each state */
  word_fsg_lc_rc (word_fsg);
  
#if __FSG_DBG__
  E_INFO("FSG after lc/rc:\n");
  word_fsg_write (word_fsg, stdout);
#endif
  
  return word_fsg;
}


static void s2_fsg_free (s2_fsg_t *fsg)
{
  s2_fsg_trans_t *trans;
  
  trans = fsg->trans_list;
  while (trans) {
    fsg->trans_list = trans->next;
    
    ckd_free ((void *) trans->word);
    ckd_free ((void *) trans);
    
    trans = fsg->trans_list;
  }
  
  ckd_free ((void *) fsg->name);
  ckd_free ((void *) fsg);
}


word_fsg_t *word_fsg_read (FILE *fp,
			   boolean use_altpron, boolean use_filler,
			   float32 silprob, float32 fillprob,
			   float32 lw, int32 n_ciphone,dict_t *dict, mdef_t * mdef
			   )
{
  s2_fsg_t *fsg;			/* "External" FSG structure */
  s2_fsg_trans_t *trans;
  word_fsg_t *cfsg;			/* "Compiled" FSG structure */
  char *wordptr[WORD_FSG_MAX_WORDPTR];	/* ptrs to words in an input line */
  int32 lineno;
  int32 n, i, j;
  float32 p;
  
  lineno = 0;
  
  /* Scan upto FSG_BEGIN header */
  for (;;) {
    n = nextline_str2words(fp, &lineno, wordptr, WORD_FSG_MAX_WORDPTR);
    if (n < 0) {
      E_ERROR("%s declaration missing\n", WORD_FSG_BEGIN_DECL);
      return NULL;
    }
    
    if ((strcmp (wordptr[0], WORD_FSG_BEGIN_DECL) == 0)
	|| (strcmp (wordptr[0], WORD_FST_BEGIN_DECL) == 0)) {
      if (n > 2) {
	E_ERROR("Line[%d]: malformed FSG_BEGIN delcaration\n", lineno);
	return NULL;
      }
      break;
    }
  }
  
  /* FSG_BEGIN found; note FSG name */
  fsg = (s2_fsg_t *) ckd_calloc (1, sizeof(s2_fsg_t));
  fsg->name = (n == 2) ? ckd_salloc(wordptr[1]) : NULL;
  fsg->trans_list = NULL;

  
  /* Read #states */
  n = nextline_str2words(fp, &lineno, wordptr, WORD_FSG_MAX_WORDPTR);
  if ((n != 2)
      || ((strcmp (wordptr[0], WORD_FSG_N_DECL) != 0)
	  && (strcmp (wordptr[0], WORD_FSG_NUM_STATES_DECL) != 0))
      || (sscanf (wordptr[1], "%d", &(fsg->n_state)) != 1)
      || (fsg->n_state <= 0)) {
    E_ERROR("Line[%d]: #states declaration line missing or malformed\n",
	    lineno);
    goto parse_error;
  }

  /* Read start state */
  n = nextline_str2words(fp, &lineno, wordptr, WORD_FSG_MAX_WORDPTR);
  if ((n != 2)
      || ((strcmp (wordptr[0], WORD_FSG_S_DECL) != 0)
	  && (strcmp (wordptr[0], WORD_FSG_START_STATE_DECL) != 0))
      || (sscanf (wordptr[1], "%d", &(fsg->start_state)) != 1)
      || (fsg->start_state < 0)
      || (fsg->start_state >= fsg->n_state)) {
    E_ERROR("Line[%d]: start state declaration line missing or malformed\n",
	    lineno);
    goto parse_error;
  }
  
  /* Read final state */
  n = nextline_str2words(fp, &lineno, wordptr, WORD_FSG_MAX_WORDPTR);
  if ((n != 2)
      || ((strcmp (wordptr[0], WORD_FSG_F_DECL) != 0)
	  && (strcmp (wordptr[0], WORD_FSG_FINAL_STATE_DECL) != 0))
      || (sscanf (wordptr[1], "%d", &(fsg->final_state)) != 1)
      || (fsg->final_state < 0)
      || (fsg->final_state >= fsg->n_state)) {
    E_ERROR("Line[%d]: final state declaration line missing or malformed\n",
	    lineno);
    goto parse_error;
  }
  
  /* Read transitions */
  for (;;) {
    n = nextline_str2words(fp, &lineno, wordptr, WORD_FSG_MAX_WORDPTR);
    if (n <= 0) {
      E_ERROR("Line[%d]: transition or FSG_END statement expected\n", lineno);
      goto parse_error;
    }
    
    if ((strcmp (wordptr[0], WORD_FSG_END_DECL) == 0)
	|| (strcmp (wordptr[0], WORD_FST_END_DECL) == 0)) {
      break;
    }
    
    if ((strcmp (wordptr[0], WORD_FSG_T_DECL) == 0)
	|| (strcmp (wordptr[0], WORD_FSG_TRANSITION_DECL) == 0)) {
      if (((n != 4) && (n != 5))
	  || (sscanf (wordptr[1], "%d", &i) != 1)
	  || (sscanf (wordptr[2], "%d", &j) != 1)
	  || (sscanf (wordptr[3], "%f", &p) != 1)
	  || (i < 0) || (i >= fsg->n_state)
	  || (j < 0) || (j >= fsg->n_state)
	  || (p <= 0.0) || (p > 1.0)) {
	E_ERROR("Line[%d]: transition spec malformed; Expecting: from-state to-state trans-prob [word]\n",
		lineno);
	goto parse_error;
      }
    } else {
      E_ERROR("Line[%d]: transition or FSG_END statement expected\n", lineno);
      goto parse_error;
    }
    
    /* Add transition to fsg */
    trans = (s2_fsg_trans_t *) ckd_calloc (1, sizeof(s2_fsg_trans_t));
    trans->from_state = i;
    trans->to_state = j;
    trans->prob = p;
    trans->word = (n > 4) ? ckd_salloc (wordptr[4]) : NULL;
    trans->next = fsg->trans_list;
    fsg->trans_list = trans;
  }
  
  cfsg = word_fsg_load (fsg, use_altpron, use_filler, silprob, fillprob, lw,dict,mdef);
  cfsg->n_ciphone=n_ciphone;
  
  s2_fsg_free (fsg);
  
  return cfsg;
  
 parse_error:
  s2_fsg_free (fsg);
  return NULL;
}


word_fsg_t *word_fsg_readfile (const char *file,
			       boolean use_altpron, boolean use_filler,
			       float32 silprob, float32 fillprob,
			       float32 lw,int32 n_ciphone,dict_t* dict,mdef_t * mdef)
{
  FILE *fp;
  word_fsg_t *fsg;
  
  E_INFO("Reading FSG file '%s' (altpron=%d, filler=%d, lw=%.2f, silprob=%.2e, fillprob=%.2e)\n",
	 file, use_altpron, use_filler, lw, silprob, fillprob);
  
  if ((fp = fopen(file, "r")) == NULL) {
    E_ERROR("fopen(%s,r) failed\n", file);
    return NULL;
  }
  
  fsg = word_fsg_read (fp,
		       use_altpron, use_filler,
		       silprob, fillprob, lw, n_ciphone,dict,mdef);
  
  fclose (fp);
  
  return fsg;
}


void word_fsg_free (word_fsg_t *fsg)
{
  int32 i, j;
  gnode_t *gn;
  word_fsglink_t *tl;
  
  for (i = 0; i < fsg->n_state; i++) {
    for (j = 0; j < fsg->n_state; j++) {
      /* Free all non-null transitions between states i and j */
      for (gn = fsg->trans[i][j]; gn; gn = gnode_next(gn)) {
	tl = (word_fsglink_t *) gnode_ptr(gn);
	ckd_free((void *) tl);
      }
      
      glist_free(fsg->trans[i][j]);
      
      /* Free any null transition i->j */
      ckd_free ((void *) fsg->null_trans[i][j]);
    }
  }
  
  ckd_free_2d ((void **) fsg->trans);
  ckd_free_2d ((void **) fsg->null_trans);
  ckd_free ((void *) fsg->name);
  
  if (fsg->lc)
    ckd_free_2d ((void **) fsg->lc);
  if (fsg->rc)
    ckd_free_2d ((void **) fsg->rc);
  
  ckd_free ((void *) fsg);
}


void word_fsg_write (word_fsg_t *fsg, FILE *fp)
{
  time_t tp;
  int32 i, j;
  gnode_t *gn;
  word_fsglink_t *tl;
  
  assert (fsg);
  assert (fsg->dict);
  
  time(&tp);
  if (tp > 0)
    fprintf (fp, "%c WORD-FSG; %s\n", WORD_FSG_COMMENT_CHAR,
	     ctime(&tp));
  else
    fprintf (fp, "%c WORD-FSG\n", WORD_FSG_COMMENT_CHAR);
  fprintf (fp, "%s\n", WORD_FSG_BEGIN_DECL);
  
  fprintf (fp, "%c #states\n", WORD_FSG_COMMENT_CHAR);
  fprintf (fp, "%s %d\n", WORD_FSG_NUM_STATES_DECL, fsg->n_state);
  
  fprintf (fp, "%c start-state\n", WORD_FSG_COMMENT_CHAR);
  fprintf (fp, "%s %d\n", WORD_FSG_START_STATE_DECL, fsg->start_state);

  fprintf (fp, "%c final-state\n", WORD_FSG_COMMENT_CHAR);
  fprintf (fp, "%s %d\n", WORD_FSG_FINAL_STATE_DECL, fsg->final_state);
  
  fprintf (fp, "%c transitions\n", WORD_FSG_COMMENT_CHAR);
  fprintf (fp, "%c from-state to-state logs2prob*lw word-ID\n",
	   WORD_FSG_COMMENT_CHAR);
  for (i = 0; i < fsg->n_state; i++) {
    for (j = 0; j < fsg->n_state; j++) {
      /* Print non-null transitions */
      for (gn = fsg->trans[i][j]; gn; gn = gnode_next(gn)) {
	tl = (word_fsglink_t *) gnode_ptr(gn);
	
	fprintf (fp, "%c %d %d %d %d\n",
		 WORD_FSG_COMMENT_CHAR,
		 tl->from_state, tl->to_state, tl->logs2prob, tl->wid);
	fprintf (fp, "%s %d %d %.3e %s\n",
		 WORD_FSG_TRANSITION_DECL,
		 tl->from_state, tl->to_state,
		 EXP(tl->logs2prob / fsg->lw),
		 (tl->wid < 0) ? "" : dict_wordstr(fsg->dict,tl->wid));
      }
      
      /* Print null transitions */
      tl = fsg->null_trans[i][j];
      if (tl) {
	fprintf (fp, "%c %d %d %d\n",
		 WORD_FSG_COMMENT_CHAR,
		 tl->from_state, tl->to_state, tl->logs2prob);
	fprintf (fp, "%s %d %d %.3e\n",
		 WORD_FSG_TRANSITION_DECL,
		 tl->from_state, tl->to_state,
		 EXP(tl->logs2prob / fsg->lw));
      }
    }
  }
  
  /* Print lc/rc vectors */
  if (fsg->lc && fsg->rc) {
    for (i = 0; i < fsg->n_state; i++) {
      fprintf (fp, "%c LC[%d]:", WORD_FSG_COMMENT_CHAR, i);
      for (j = 0; fsg->lc[i][j] >= 0; j++)
	fprintf (fp, " %s", mdef_ciphone_str(fsg->mdef,fsg->lc[i][j]));
      fprintf (fp, "\n");
      
      fprintf (fp, "%c RC[%d]:", WORD_FSG_COMMENT_CHAR, i);
      for (j = 0; fsg->rc[i][j] >= 0; j++)
	fprintf (fp, " %s", mdef_ciphone_str(fsg->mdef,fsg->rc[i][j]));
      fprintf (fp, "\n");
    }
  }
  
  fprintf (fp, "%c\n", WORD_FSG_COMMENT_CHAR);
  fprintf (fp, "%s\n", WORD_FSG_END_DECL);
  
  fflush (fp);
}


void word_fsg_writefile (word_fsg_t *fsg, char *file)
{
  FILE *fp;
  
  assert (fsg);
  
  E_INFO("Writing FSG file '%s'\n", file);
  
  if ((fp = fopen(file, "w")) == NULL) {
    E_ERROR("fopen(%s,r) failed\n", file);
    return;
  }
  
  word_fsg_write (fsg, fp);
  
  fclose (fp);
}


int32 word_fsg_set_start_state (word_fsg_t *fsg, int32 state)
{
  int32 prev;
  
  if ((! fsg)
      || (state < 0)
      || (state >= fsg->n_state))
    return -1;
  
  prev = fsg->start_state;
  
  fsg->start_state = state;
  
  return prev;
}


int32 word_fsg_set_final_state (word_fsg_t *fsg, int32 state)
{
  int32 prev;
  
  if ((! fsg)
      || (state < 0)
      || (state >= fsg->n_state))
    return -1;
  
  prev = fsg->final_state;
  
  fsg->final_state = state;
  
  return prev;
}
