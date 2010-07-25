/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * word_graph.h -- Library for word graph a linked-based DAG. 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:15:12  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Word graphs with word attached to links. Now mainly used in conversion from Sphinx to IBM format
 *
 * Revision 1.1.2.1  2005/11/17 06:39:30  arthchan2003
 * Added a structure for linked-based lattice.
 *
 */


#ifndef _WORD_GRAPH_H_
#define _WORD_GRAPH_H_

#include <stdio.h>

#include <sphinxbase/profile.h>

#include <s3types.h>
#include <dag.h>
#include <dict.h>
#include <lm.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

/*
 *  word_graph_t is a linked-based word graph.  That is the word-ID
 *  lies on the arc of the graph.  (As opposed to dag_t which
 *  essentially link segments together or node-based)
 * 
 */

#define INVALID_START_FRAME -1
#define INVALID_START_INDEX -1

#define OUTLATFMT_SPHINX3 0
#define OUTLATFMT_IBM 1
#define dag_node_mark(d) d->reachable

/** 
 * \struct word_graph_link_t
 *
 * A link in the word_graph
 */
typedef struct{
    int32 srcidx; /**< Start Node Idx */
    int32 tgtidx; /**< End Node Idx */
    int32 wid;   /**< Word ID */
    float64 ascr; /**< Acoustic Score */
    float64 lscr; /**< Language Score */
    float64 cscr; /**< Confidence Score */
    int32 linkidx; /**< The index for this node */
} word_graph_link_t;

/** 
 * \struct word_graph_node_t
 *
 * A node in the word_graph
 */

typedef struct{
    int32 time;  /**< The time moment of this node */
    int32 nodeidx; /**< The index for this node */
    glist_t child_node_list; /**< The list of all children. */
} word_graph_node_t;

/**
 * \struct word_graph_t
 *
 * The word graph structure. (linked-based word graph)
 */
typedef struct{
    glist_t link; /**< List of link */
    glist_t node; /**< List of node */
    int32 n_link; /**< Number of node */
    int32 n_node; /**< Number of node */

} word_graph_t;

/**
 * Print a word_graph structure 
 */
void print_wg(FILE *fp,  /**< File pointer */
	      word_graph_t *wg,  /**< Word graph */
	      dict_t *dict,
	      int32 fmt  /**< 
			    Format of the word graph 
			    fmt=0: simple format
			    fmt=1: IBM format. 
			 */
    );

/**
   Convert a dag to wordgraph. 
*/
word_graph_t* dag_to_wordgraph (dag_t* dag,  /**< a DAG structure */
				int32 *senscale, /**< Scaling factor of the acoustic score */
				lm_t* lm, /**< LM */
				dict_t* dict /**< Dict */
    );


/**
   Dump the word graph. This is similar to s3flat_fwd_dag_dump interface. 
*/

void word_graph_dump(char *dir, /**< Directory name*/
		     char *uttfile,  /**< Utterance Filename */
		     char *id,  /**< Utterance ID */
		     char *latfile_ext,  /**< Lattice file Extension */
		     dag_t *dag,   /**< DAG */
		     dict_t *dict,  /**< Dictionary */
		     lm_t *lm,     /**< LM */
		     int32 *senscale /**< Senone scale */
    );


/**
   Free wordgraph. 
*/
void wordgraph_free(word_graph_t *wg /**< Word graph */
    );

#ifdef __cplusplus
}
#endif


#endif  /*_WORD_GRAPH_H_*/

