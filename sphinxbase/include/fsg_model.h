/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * fsg_model.h -- Word-level finite state graph
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2003 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 */


#ifndef __FSG_MODEL_H__
#define __FSG_MODEL_H__

/* System headers. */
#include <stdio.h>
#include <string.h>

/* SphinxBase headers. */
#include <prim_type.h>
#include <glist.h>
#include <logmath.h>
#include <hash_table.h>
#include <bitvec.h>
#include <listelem_alloc.h>
#include <sphinxbase_export.h>

/*
 * A single transition in the FSG.
 */
typedef struct fsg_link_s {
    int32 from_state;
    int32 to_state;
    int32 logs2prob;	/**< log(transition probability)*lw */
    int32 wid;		/**< Word-ID; <0 if epsilon or null transition */
} fsg_link_t;

/* Access macros */
#define fsg_link_from_state(l)	((l)->from_state)
#define fsg_link_to_state(l)	((l)->to_state)
#define fsg_link_wid(l)		((l)->wid)
#define fsg_link_logs2prob(l)	((l)->logs2prob)


/**
 * Word level FSG definition.
 * States are simply integers 0..n_state-1.
 * A transition emits a word and has a given probability of being taken.
 * There can also be null or epsilon transitions, with no associated emitted
 * word.
 */
typedef struct fsg_model_s {
    int refcount;       /**< Reference count. */
    char *name;		/**< A unique string identifier for this FSG */
    int32 n_word;       /**< Number of unique words in this FSG */
    int32 n_word_alloc; /**< Number of words allocated in vocab */
    char **vocab;       /**< Vocabulary for this FSG. */
    bitvec_t *silwords; /**< Indicates which words are silence/fillers. */
    bitvec_t *altwords; /**< Indicates which words are pronunciation alternates. */
    logmath_t *lmath;	/**< Pointer to log math computation object. */
    int32 n_state;	/**< number of states in FSG */
    int32 start_state;	/**< Must be in the range [0..n_state-1] */
    int32 final_state;	/**< Must be in the range [0..n_state-1] */
    float32 lw;		/**< Language weight that's been applied to transition
			   logprobs */
    glist_t **trans;	/**< trans[i][j] = glist of non-epsilon transitions or
			   links (fsg_link_t *) from state i to state j,
			   if any; NULL if none. */
    fsg_link_t ***null_trans; /**< null_trans[i][j] = epsilon or null link
                                 from state i to j, if any; NULL if none.
                                 (At most one null transition between two
                                 given states.) */
    listelem_alloc_t *link_alloc; /**< Allocator for FSG links. */
} fsg_model_t;

/* Access macros */
#define fsg_model_name(f)		((f)->name)
#define fsg_model_n_state(f)		((f)->n_state)
#define fsg_model_start_state(f)	((f)->start_state)
#define fsg_model_final_state(f)	((f)->final_state)
#define fsg_model_log(f,p)		logmath_log((f)->lmath, p)
#define fsg_model_lw(f)			((f)->lw)
#define fsg_model_trans(f,i,j)		((f)->trans[i][j])
#define fsg_model_null_trans(f,i,j)	((f)->null_trans[i][j])
#define fsg_model_n_word(f)		((f)->n_word)
#define fsg_model_word_str(f,wid)       (wid == -1 ? "(NULL)" : (f)->vocab[wid])

/**
 * Have silence transitions been added?
 */
#define fsg_model_has_sil(f)            ((f)->silwords != NULL)

/**
 * Have alternate word transitions been added?
 */
#define fsg_model_has_alt(f)            ((f)->altwords != NULL)

#define fsg_model_is_filler(f,wid) \
    (fsg_model_has_sil(f) ? bitvec_is_set((f)->silwords, wid) : FALSE)
#define fsg_model_is_alt(f,wid) \
    (fsg_model_has_alt(f) ? bitvec_is_set((f)->altwords, wid) : FALSE)

/**
 * Create a new FSG.
 */
SPHINXBASE_EXPORT
fsg_model_t *fsg_model_init(char const *name, logmath_t *lmath,
                            float32 lw, int32 n_state);

/**
 * Read a word FSG from the given file and return a pointer to the structure
 * created.  Return NULL if any error occurred.
 * 
 * File format:
 * 
 * <pre>
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
 * </pre>
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
 * Return value: a new fsg_model_t structure if the file is successfully
 * read, NULL otherwise.
 */
SPHINXBASE_EXPORT
fsg_model_t *fsg_model_readfile(const char *file, logmath_t *lmath, float32 lw);

/**
 * Like fsg_model_readfile(), but from an already open stream.
 */
SPHINXBASE_EXPORT
fsg_model_t *fsg_model_read(FILE *fp, logmath_t *lmath, float32 lw);

/**
 * Retain ownership of an FSG.
 *
 * @return Pointer to retained FSG.
 */
SPHINXBASE_EXPORT
fsg_model_t *fsg_model_retain(fsg_model_t *fsg);

/**
 * Free the given word FSG.
 *
 * @return new reference count (0 if freed completely)
 */
SPHINXBASE_EXPORT
int fsg_model_free(fsg_model_t *fsg);

/**
 * Add a word to the FSG vocabulary.
 *
 * @return Word ID for this new word.
 */
SPHINXBASE_EXPORT
int fsg_model_word_add(fsg_model_t *fsg, char const *word);

/**
 * Look up a word in the FSG vocabulary.
 *
 * @return Word ID for this word
 */
SPHINXBASE_EXPORT
int fsg_model_word_id(fsg_model_t *fsg, char const *word);

/**
 * Add the given transition to the FSG transition matrix.
 *
 * Duplicates (i.e., two transitions between the same states, with the
 * same word label) are flagged and only the highest prob retained.
 */
SPHINXBASE_EXPORT
void fsg_model_trans_add(fsg_model_t * fsg,
                         int32 from, int32 to, int32 logp, int32 wid);

/**
 * Add a null transition between the given states.
 *
 * There can be at most one null transition between the given states;
 * duplicates are flagged and only the best prob retained.  Transition
 * probs must be <= 1 (i.e., logprob <= 0).
 *
 * @return 1 if a new transition was added, 0 if the prob of an existing
 * transition was upgraded; -1 if nothing was changed.
 */
SPHINXBASE_EXPORT
int32 fsg_model_null_trans_add(fsg_model_t * fsg, int32 from, int32 to, int32 logp);

/**
 * Obtain transitive closure of null transitions in the given FSG.
 *
 * @param nulls List of null transitions, or NULL to find them automatically.
 * @return Updated list of null transitions.
 */
SPHINXBASE_EXPORT
glist_t fsg_model_null_trans_closure(fsg_model_t * fsg, glist_t nulls);

/**
 * Add silence word transitions to each state in given FSG.
 *
 * @param state state to add a self-loop to, or -1 for all states.
 * @param silprob probability of silence transition.
 */
SPHINXBASE_EXPORT
int fsg_model_add_silence(fsg_model_t * fsg, char const *silword,
                          int state, float32 silprob);

/**
 * Add alternate pronunciation transitions for a word in given FSG.
 */
SPHINXBASE_EXPORT
int fsg_model_add_alt(fsg_model_t * fsg, char const *baseword,
                      char const *altword);

/**
 * Write FSG to a file.
 */
SPHINXBASE_EXPORT
void fsg_model_write(fsg_model_t *fsg, FILE *fp);

/**
 * Write FSG to a file.
 */
SPHINXBASE_EXPORT
void fsg_model_writefile(fsg_model_t *fsg, char const *file);

#endif /* __FSG_MODEL_H__ */