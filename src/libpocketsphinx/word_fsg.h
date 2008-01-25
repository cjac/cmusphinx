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
 * word_fsg.h -- Word-level finite state graph
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2003 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 */


#ifndef __S2_WORD_FSG_H__
#define __S2_WORD_FSG_H__

/* System headers. */
#include <stdio.h>
#include <string.h>

/* SphinxBase headers. */
#include <prim_type.h>
#include <glist.h>

/* Local headers. */
#include "fbs.h"


/*
 * A single transition in the FSG.
 */
typedef struct word_fsglink_s {
  int32 from_state;
  int32 to_state;
  int32 wid;		/* Word-ID; <0 if epsilon or null transition */
  int32 logs2prob;	/* logs2(transition probability)*lw */
} word_fsglink_t;

/* Access macros */
#define word_fsglink_from_state(l)	((l)->from_state)
#define word_fsglink_to_state(l)	((l)->to_state)
#define word_fsglink_wid(l)		((l)->wid)
#define word_fsglink_logs2prob(l)	((l)->logs2prob)


/*
 * Word level FSG definition.
 * States are simply integers 0..n_state-1.
 * A transition emits a word and has a given probability of being taken.
 * There can also be null or epsilon transitions, with no associated emitted
 * word.
 */
typedef struct word_fsg_s {
  char *name;		/* A unique string identifier for this FSG */
  int32 n_state;	/* #states in FSG */
  int32 start_state;	/* Must be in the range [0..n_state-1] */
  int32 final_state;	/* Must be in the range [0..n_state-1] */
  boolean use_altpron;	/* Whether transitions for alternative pronunciations
			   have been added to the FSG */
  boolean use_filler;	/* Whether silence and noise filler-word transitions
			   have been added at each state */
  float32 lw;		/* Language weight that's been applied to transition
			   logprobs */
  glist_t **trans;	/* trans[i][j] = glist of non-epsilon transitions or
			   links (word_fsglink_t *) from state i to state j,
			   if any; NULL if none. */
  word_fsglink_t ***null_trans;	/* null_trans[i][j] = epsilon or null link
				   from state i to j, if any; NULL if none.
				   (At most one null transition between two
				   given states.) */

  /*
   * Left and right CIphone sets for each state.
   * Left context CIphones for a state S: If word W transitions into S, W's
   * final CIphone is in S's {lc}.  Words transitioning out of S must consider
   * these left context CIphones.
   * Similarly, right contexts for state S: If word W transitions out of S,
   * W's first CIphone is in S's {rc}.  Words transitioning into S must consider
   * these right contexts.
   * 
   * NOTE: Words may transition into and out of S INDIRECTLY, with intermediate
   *   null transitions.
   * NOTE: Single-phone words are difficult; only SILENCE right context is
   *   modelled for them.
   * NOTE: Non-silence filler phones aren't included in these sets.  Filler
   *   words don't use context, and present the SILENCE phone as context to
   *   adjacent words.
   */
  int8 **rc, **lc;
} word_fsg_t;

/* Access macros */
#define word_fsg_name(f)		((f)->name)
#define word_fsg_n_state(f)		((f)->n_state)
#define word_fsg_start_state(f)		((f)->start_state)
#define word_fsg_final_state(f)		((f)->final_state)
#define word_fsg_lw(f)			((f)->lw)
#define word_fsg_use_altpron(f)		((f)->use_altpron)
#define word_fsg_use_filler(f)		((f)->use_filler)
#define word_fsg_trans(f,i,j)		((f)->trans[i][j])
#define word_fsg_null_trans(f,i,j)	((f)->null_trans[i][j])
#define word_fsg_lc(f,s)		((f)->lc[s])
#define word_fsg_rc(f,s)		((f)->rc[s])


/*
 * Read a word FSG from the given file and return a pointer to the structure
 * created.  Return NULL if any error occurred.
 * 
 * File format:
 * 
 *   Any number of comment lines; ignored
 *   FSG_BEGIN [<fsgname>]
 *   N <#states>
 *   S <start-state ID>
 *   F <final-state ID>
 *   T <from-state> <to-state> <prob> [<word-string>]
 *   T ...
 *   ... (any number of state transitions)
 *   FSG_END
 *   Any number of comment lines; ignored
 * 
 * The FSG spec begins with the line containing the keyword FSG_BEGIN.
 * It has an optional fsg name string.  If not present, the FSG has the empty
 * string as its name.
 * 
 * Following the FSG_BEGIN declaration is the number of states, the start
 * state, and the final state, each on a separate line.  States are numbered
 * in the range [0 .. <numberofstate>-1].
 * 
 * These are followed by all the state transitions, each on a separate line,
 * and terminated by the FSG_END line.  A state transition has the given
 * probability of being taken, and emits the given word.  The word emission
 * is optional; if word-string omitted, it is an epsilon or null transition.
 * 
 * Comments can also be embedded within the FSG body proper (i.e. between
 * FSG_BEGIN and FSG_END): any line with a # character in col 1 is treated
 * as a comment line.
 * 
 * Other arguments:
 * - use_altpron: if TRUE, if a transition labelled W exists between two
 *   states, all the alternative pronunciations of W (in the dictionary) are
 *   also added as transitions with the same given probability.
 * - use_filler: if TRUE, a separate transition for each silence and noise
 *   filler word in the lexicon is added at each state (loop transition).
 *   However, if noiseword penalty is 0.0, noise words are ignored.
 * - silprob: transition prob for silence word transitions (if use_filler)
 * - fillprob: transition prob for noise word transitions (if use_filler)
 * - lw: language weight, applied as a multiplicative factor to the LOG of
 *   the transition probs.
 * 
 * Return value: a new word_fsg_t structure if the file is successfully
 * read, NULL otherwise.
 */
word_fsg_t *word_fsg_readfile (char *file,
			       boolean use_altpron, boolean use_filler,
			       float32 silprob, float32 fillprob,
			       float32 lw);


/*
 * Like word_fsg_readfile(), but from an already open stream.
 */
word_fsg_t *word_fsg_read (FILE *fp,
			   boolean use_altpron, boolean use_filler,
			   float32 silprob, float32 fillprob,
			   float32 lw);


/*
 * Like word_fsg_read(), but from an in-memory structure.
 */
word_fsg_t *word_fsg_load (s2_fsg_t *s2_fsg,
			   boolean use_altpron, boolean use_filler,
			   float32 silprob, float32 fillprob,
			   float32 lw);


/*
 * Write the given fsg structure to the given file.
 * (This needs options for writing in format readable by word_fsg_read().)
 */
void word_fsg_writefile (word_fsg_t *fsg, char *file);


/*
 * Like word_fsg_writefile(), but to an already open stream.
 */
void word_fsg_write (word_fsg_t *fsg, FILE *fp);


/*
 * Free the given word FSG
 */
void word_fsg_free(word_fsg_t *);


/*
 * Set the FSG start state (or the final state) to the given state.
 * Return value: -1 if there was any error, otherwise the previous start
 * state (or final state).
 */
int32 word_fsg_set_start_state (word_fsg_t *fsg, int32 new_start_state);
int32 word_fsg_set_final_state (word_fsg_t *fsg, int32 new_final_state);


#endif
