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
 * fsg_psubtree.h -- Phone-level FSG subtree representing all transitions
 * out of a single FSG state. 
 * (Note: Currently, it is actually a flat lexicon representation
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:10:18  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Adaptation of Sphinx 2's FSG search into Sphinx 3
 *
 * Revision 1.1.2.5  2005/07/24 01:34:54  arthchan2003
 * Mode 2 is basically running. Still need to fix function such as resulting and build the correct utterance ID
 *
 * Revision 1.1.2.4  2005/07/20 21:18:30  arthchan2003
 * FSG can now be read, srch_fsg_init can now be initialized, psubtree can be built. Sounds like it is time to plug in other function pointers.
 *
 * Revision 1.1.2.3  2005/07/17 05:44:32  arthchan2003
 * Added dag_write_header so that DAG header writer could be shared between 3.x and 3.0. However, because the backtrack pointer structure is different in 3.x and 3.0. The DAG writer still can't be shared yet.
 *
 * Revision 1.1.2.2  2005/07/13 18:39:47  arthchan2003
 * (For Fun) Remove the hmm_t hack. Consider each s2 global functions one-by-one and replace them by sphinx 3's macro.  There are 8 minor HACKs where functions need to be removed temporarily.  Also, there are three major hacks. 1,  there are no concept of "phone" in sphinx3 dict_t, there is only ciphone. That is to say we need to build it ourselves. 2, sphinx2 dict_t will be a bunch of left and right context tables.  This is currently bypass. 3, the fsg routine is using fsg_hmm_t which is just a duplication of CHAN_T in sphinx2, I will guess using hmm_evaluate should be a good replacement.  But I haven't figure it out yet.
 *
 * Revision 1.1.2.1  2005/06/27 05:26:29  arthchan2003
 * Sphinx 2 fsg mainpulation routines.  Compiled with faked functions.  Currently fended off from users.
 *
 * Revision 1.1  2004/07/16 00:57:12  egouvea
 * Added Ravi's implementation of FSG support.
 *
 * Revision 1.3  2004/06/25 14:49:08  rkm
 * Optimized size of history table and speed of word transitions by maintaining only best scoring word exits at each state
 *
 * Revision 1.2  2004/05/27 14:22:57  rkm
 * FSG cross-word triphones completed (but for single-phone words)
 *
 * Revision 1.1.1.1  2004/03/01 14:30:31  rkm
 *
 *
 * Revision 1.2  2004/02/27 15:05:21  rkm
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/23 15:53:45  rkm
 * Renamed from fst to fsg
 *
 * Revision 1.4  2004/02/23 15:09:50  rkm
 * *** empty log message ***
 *
 * Revision 1.3  2004/02/19 21:16:54  rkm
 * Added fsg_search.{c,h}
 *
 * Revision 1.2  2004/02/18 15:02:34  rkm
 * Added fsg_lextree.{c,h}
 *
 * Revision 1.1  2004/02/17 21:11:49  rkm
 * *** empty log message ***
 *
 * 
 * 09-Feb-2004	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */


#ifndef __S2_FSG_PSUBTREE_H__
#define __S2_FSG_PSUBTREE_H__


#include <stdio.h>

#include "s3types.h"
#include "word_fsg.h"
#include "fsg.h"
#include "hmm.h"
#include "dict.h"
#include "mdef.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

/*
 * **HACK-ALERT**!!  Compile-time constant determining the size of the
 * bitvector fsg_pnode_t.fsg_pnode_ctxt_t.bv.  (See below.)
 * But it makes memory allocation simpler and more efficient.
 */
#define FSG_PNODE_CTXT_BVSZ	2

typedef struct {
    uint32 bv[FSG_PNODE_CTXT_BVSZ];
} fsg_pnode_ctxt_t;


/**
 * \struct fsg_pnode_s 
 * \brief an fsg node. 
 * All transitions (words) out of any given FSG state represented are by a
 * phonetic prefix lextree (except for epsilon or null transitions; they
 * are not part of the lextree).  Lextree leaf nodes represent individual
 * FSG transitions, so no sharing is allowed at the leaf nodes.  The FSG
 * transition probs are distributed along the lextree: the prob at a node
 * is the max of the probs of all leaf nodes (and, hence, FSG transitions)
 * reachable from that node.
 * 
 * To conserve memory, the underlying HMMs with state-level information are
 * allocated only as needed.  Root and leaf nodes must also account for all
 * the possible phonetic contexts, with an independent HMM for each distinct
 * context.
 */
typedef struct fsg_pnode_s {
    /**
     * If this is not a leaf node, the first successor (child) node.  Otherwise
     * the parent FSG transition for which this is the leaf node (for figuring
     * the FSG destination state, and word emitted by the transition).  A node
     * may have several children.  The succ ptr gives just the first; the rest
     * are linked via the sibling ptr below.
     */
    union {
        struct fsg_pnode_s *succ;
        word_fsglink_t *fsglink;
    } next;
  
    /*
     * For simplicity of memory management (i.e., freeing the pnodes), all
     * pnodes allocated for all transitions out of a state are maintained in a
     * linear linked list through the alloc_next pointer.
     */
    struct fsg_pnode_s *alloc_next;
  
    /*
     * The next node that is also a child of the parent of this node; NULL if
     * none.
     */
    struct fsg_pnode_s *sibling;

    /*
     * The transition (log) probability to be incurred upon transitioning to
     * this node.  (Transition probabilities are really associated with the
     * transitions.  But a lextree node has exactly one incoming transition.
     * Hence, the prob can be associated with the node.)
     * This is a logs2(prob) value, and includes the language weight.
     */
    int32 logs2prob;
  
    /*
     * The root and leaf positions associated with any transition have to deal
     * with multiple phonetic contexts.  However, different contexts may result
     * in the same SSID (senone-seq ID), and can share a single pnode with that
     * SSID.  But the pnode should track the set of context CI phones that share
     * it.  Hence the fsg_pnode_ctxt_t bit-vector set-representation.  (For
     * simplicity of implementation, its size is a compile-time constant for
     * now.)  Single phone words would need a 2-D array of context, but that's
     * too expensive.  For now, they simply use SIL as right context, so only
     * the left context is properly modelled.
     * (For word-internal phones, this field is unused, of course.)
     */
    fsg_pnode_ctxt_t ctxt;
  
    uint8 ci_ext;		/* This node's CIphone as viewed externally (context) */
    uint8 ppos;		/* Phoneme position in pronunciation */
    uint8 leaf;		/* Whether this is a leaf node */
  
    /* HMM-state-level stuff here */
    hmm_t hmm;
} fsg_pnode_t;

/* Access macros */
#define fsg_pnode_leaf(p)	((p)->leaf)
#define fsg_pnode_logs2prob(p)	((p)->logs2prob)
#define fsg_pnode_succ(p)	((p)->next.succ)
#define fsg_pnode_fsglink(p)	((p)->next.fsglink)
#define fsg_pnode_sibling(p)	((p)->sibling)
#define fsg_pnode_hmmptr(p)	(&((p)->hmm))
#define fsg_pnode_ci_ext(p)	((p)->ci_ext)
#define fsg_pnode_ppos(p)	((p)->ppos)
#define fsg_pnode_leaf(p)	((p)->leaf)
#define fsg_pnode_ctxt(p)	((p)->ctxt)

#define fsg_pnode_add_ctxt(p,c)	((p)->ctxt.bv[(c)>>5] |= (1 << ((c)&0x001f)))


/**
 * Build the phone lextree for all transitions out of state from_state.
 * Return the root node of this tree.
 * Also, return a linear linked list of all allocated fsg_pnode_t nodes in
 * *alloc_head (for memory management purposes).
 */
fsg_pnode_t *fsg_psubtree_init (hmm_context_t *ctx,
                                word_fsg_t *fsg, /**< A word fsg */
				int32 from_state, /**< from which state to initalize*/ 
				fsg_pnode_t **alloc_head,
                                cmd_ln_t *config,
				logmath_t *logmath
    );


/**
 * Free the given lextree.  alloc_head: head of linear list of allocated
 * nodes updated by fsg_psubtree_init().
 */
void fsg_psubtree_free (fsg_pnode_t *alloc_head);


/*
 * Dump the list of nodes in the given lextree to the given file.  alloc_head:
 * head of linear list of allocated nodes updated by fsg_psubtree_init().
 */
void fsg_psubtree_dump (fsg_pnode_t *alloc_head, FILE *fp,
			dict_t *dict, mdef_t *mdef
    );


/*
 * Attempt to transition into the given node with the given attributes.
 * If the node is already active in the given frame with a score better
 * than the incoming score, nothing is done.  Otherwise the transition is
 * successful.
 * Return value: TRUE if the node was newly activated for the given frame,
 * FALSE if it was already activated for that frame (whether the incoming
 * transition was successful or not).
 */
int fsg_psubtree_pnode_enter (fsg_pnode_t *pnode,
                              int32 score,
                              int32 frame,
                              int32 bpidx);


/*
 * Mark the given pnode as inactive (for search).
 */
void fsg_psubtree_pnode_deactivate (fsg_pnode_t *pnode);


/* Set all flags on in the given context bitvector */
void fsg_pnode_add_all_ctxt(fsg_pnode_ctxt_t *ctxt);

/*
 * Subtract bitvector sub from bitvector src (src updated with the result).
 * Return 0 if result is all 0, non-zero otherwise.
 */
uint32 fsg_pnode_ctxt_sub (fsg_pnode_ctxt_t *src, fsg_pnode_ctxt_t *sub);

#ifdef __cplusplus
}
#endif


#endif
