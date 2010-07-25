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
 * dag.h -- Library for DAG
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log: dag.h,v $
 * Revision 1.2  2006/02/23 05:22:32  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed bugs from last check in, lw should be * instead of +, 2, Moved most of the functions from flat_fwd.c and s3_dag.c to here.  Things that required specified will be prefixed.
 *
 * Revision 1.1.4.5  2005/11/17 06:25:04  arthchan2003
 * 1, Added structure to record node-based ascr and lscr. 2, Added a version of dag_link that copies the langauge model score as well.
 *
 * Revision 1.1.4.4  2005/09/25 19:20:43  arthchan2003
 * Added hooks in dag_node and dag_link. Probably need some time to use it various routines of ours.
 *
 * Revision 1.1.4.3  2005/09/11 23:07:28  arthchan2003
 * srch.c now support lattice rescoring by rereading the generated lattice in a file. When it is operated, silence cannot be unlinked from the dictionary.  This is a hack and its reflected in the code of dag, kbcore and srch. code
 *
 * Revision 1.1.4.2  2005/09/11 02:56:47  arthchan2003
 * Log. Incorporated all dag related functions from s3_dag.c and
 * flat_fwd.c.  dag_search, dag_add_fudge, dag_remove_filler is now
 * shared by dag and decode_anytopo. (Hurray!). s3_astar.c still has
 * special functions and it probably unavoidable.
 *
 * Revision 1.1.4.1  2005/07/17 05:44:31  arthchan2003
 * Added dag_write_header so that DAG header writer could be shared between 3.x and 3.0. However, because the backtrack pointer structure is different in 3.x and 3.0. The DAG writer still can't be shared yet.
 *
 * Revision 1.1  2005/06/21 22:37:47  arthchan2003
 * Build a stand-alone wrapper for direct acyclic graph, it is now shared across dag/astar and decode_anytopo.  This eliminate about 500 lines of code in decode_anytopo/dag and astar. However, its existence still can't exterminate code duplication between dag/decode_anytopo.  That effectively means we have many refactoring to do.  Things that are still pretty difficult to merge include dag_search(decode_anytopo/dag) and dag_read (dag/astar).
 *
 * Revision 1.2  2005/06/03 06:45:28  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.1  2005/06/03 05:46:19  archan
 * Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 */

#ifndef _LIBFBS_DAG_H_
#define _LIBFBS_DAG_H_

#include <stdio.h>

#include <sphinxbase/listelem_alloc.h>
#include <sphinxbase/cmd_ln.h>

#include "s3types.h"
#include "search.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

#define SPHINX_LATTICE_FORMAT 0
#define IBM_LATTICE_FORMAT 1 


/** \file dag.h
    \brief data structure for dag. Adapted from s3_dag.h in s3.5
*/

/**
 * DAG structure representation of word lattice.  A unique <wordid,startframe> is a node.
 * Edges are formed if permitted by time adjacency.  (See comment before dag_build.)
 */
typedef struct dagnode_s {
    s3wid_t wid;
    int32 seqid;			/**< Running sequence no. for identification */
    s3frmid_t sf;			/**< Start frame for this occurrence of wid */
    s3frmid_t fef, lef;			/**< First and last end frames */
    struct dagnode_s *alloc_next;	/**< Next in linear list of allocated nodes */
    struct daglink_s *succlist;		/**< List of successor nodes (adjacent in time) */
    struct daglink_s *predlist;		/**< List of preceding nodes (adjacent in time) */
    int32 node_ascr;                      /**< Node acoustic score */
    int32 node_lscr;                      /**< Node language score */
    void *hook;                           /**< A hook that could allow arbitrary data structure to use dagnode_t */
    uint8 reachable;                      /**< In astar: Whether final node reachable from here 
                                             In flat_fwd's dag_to_wordgraph: A marker for whether 
                                             a node is already marked. 
                                          */
} dagnode_t;

/** 
    \struct daglink_t
    A DAG node can have several successor or predecessor nodes, each represented by a link 
    Multiple-purpose, so some fields may not be used some time. 
*/
typedef struct daglink_s {
    dagnode_t *node;		/**< Target of link (source determined by dagnode_t.succlist
                                   or dagnode_t.predlist) */
    dagnode_t *src;		/**< Source node of link */
    struct daglink_s *next;	/**< Next in same dagnode_t.succlist or dagnode_t.predlist */
    struct daglink_s *history;	/**< Previous link along best path (for traceback) */
    struct daglink_s *bypass;	/**< If this links A->B, bypassing A->fillnode->B, then
                                   bypass is ptr to fillnode->B */


    int32 ascr;			/**< Acoustic score for segment of source node ending just
                                   before the end point of this link.  (Actually this gets
                                   corrupted because of filler node deletion.) */
    int32 lscr;			/**< LM score to the SUCCESSOR node */
    int32 pscr;			/**< Best path score to root beginning with this link */
    int32 hscr;			/**< Heuristic score from end of link to dag exit node */

    s3frmid_t ef;		/**< End time for this link.  Should be 1 before the start
				   time of destination node (or source node for reverse
				   links), but gets corrupted because of filler deletion */
    int16 pscr_valid;		/**< Flag to avoid evaluating the same path multiple times */

    void *hook;                           /**< A hook that could allow arbitrary data structure to use daglink_t */

} daglink_t;

/** 
    \struct dag_t 
    Summary of DAG structure information 
    Multiple-purpose, so some fields may not be used some time. 

    FIXME, latfinal and exit are very very similar things, they just
    happened to be declared by Ravi different time. 
*/
typedef struct {
    dagnode_t *list;		/**< Linear list of nodes allocated */
    dagnode_t *root;            /**< Corresponding to the node of (\<s\>,0)  */
    dagnode_t *end;             /**< Final node (\</s\>,nfrm) */

    daglink_t entry;		/**< Entering (\<s\>,0) */
    daglink_t final;            /**< Exit link from final DAG node */

    s3wid_t orig_exitwid;	/**< If original exit node is not a filler word */

    int32 nfrm;                 /**< Number of frames */
    int32 nlink;                /**< Number of links */
    int32 nnode;                /**< Number of nodes */
    int32 nbypass;              /**< The number of links which are by-passed */

    int32 maxedge;              /**< (New in S3.6) Used in dag/astar/decode_anytopo, this decides whether
                                   parts of the dag code will exceed the maximum no of edge 
                                */

    int32 lmop;		        /**< (Temporary Variable): \#LM ops actually made */
    int32 maxlmop;		/**< Max LM ops allowed before utterance aborted 
				 */

    int32 filler_removed;       /**< Whether filler nodes removed from DAG to help search */
    int32 fudged;               /**< Whether fudge edges have been added */

    void *hook;                   /**< A hook for general purpose */

    cmd_ln_t *config;
    listelem_alloc_t *node_alloc; /**< Allocator for nodes. */
    listelem_alloc_t *link_alloc; /**< Allocator for edges. */
    logmath_t *logmath;
} dag_t;


/** Clean up the hypothesis list */
S3DECODER_EXPORT
void hyp_free (srch_hyp_t *list);

/** Initialize a dag_t */
void dag_init(dag_t* dagp, cmd_ln_t *config, logmath_t *logmath);


/** Link two DAG nodes with the given arguments
 * @return 0 if successful, -1 if maxedge limit exceeded.
 */
S3DECODER_EXPORT
int32 dag_link (dag_t * dagp,    /**< A pointer to a DAG */
                dagnode_t *pd,  
                dagnode_t *d,   
                int32 ascr,     /**< The acoustic score */
                int32 lscr,     /**< The language score (0 if none) */
                int32 ef,       /**< The ending frame */
                daglink_t *byp  
    );


daglink_t *find_succlink (dagnode_t *src,
                          dagnode_t *dst,
                          int32 bypass    /**< Find bypass links only? */
    );

daglink_t *find_predlink (dagnode_t *src,
                          dagnode_t *dst,
                          int32 bypass    /**< Find bypass links only? */
    );

int32 dag_update_link (dag_t* dagp,  /**< A pointer to a DAG */
		       dagnode_t *pd, 
		       dagnode_t *d, 
		       int32 ascr,  /**< The acoustic scores */
		       int32 ef,    /**< The ending frame */
		       daglink_t *byp /**< A filler link being bypassed by this one */
    );

/** Routine to read the dag header 
 */

int32 dag_param_read (FILE *fp, /** file pointer */
		      char *param, /** The parameter name */
		      int32 *lineno /** IN/Out The pointer of line no */
    );


/**
 * Recursive step in dag_search:  best backward path from src to root beginning with l.
 * @return 0 if successful, -1 otherwise.
 */
int32 dag_bestpath (
    dag_t* dagp,  /**< A pointer to a DAG */
    daglink_t *l,	/**< Backward link! */
    dagnode_t *src,	/**< Source node for backward link l */
    float64 lwf,         /**< Language weight multiplication factor */ 
    dict_t *dict,        /**<  The dictionary */
    lm_t *lm,             /**< The LM */
    s3lmwid32_t *dict2lmwid /**< A map from dictionary id to lm id, should use wid2lm insteead*/
    ); 


/** Check whether the link score is larger than zero
 * @return 0 if not, return -1 otherwise. 
 */
int32 dag_chk_linkscr (
    dag_t *dagp /**< A pointer to a DAG */
    );

/** destroy a dag 
 */
S3DECODER_EXPORT
int32 dag_destroy ( 
    dag_t *dagp /**< A pointer to a DAG */
    );

/**
 * For each link compute the heuristic score (hscr) from the END of the link to the
 * end of the utterance; i.e. the best score from the end of the link to the dag
 * exit node.
 */
S3DECODER_EXPORT
void dag_compute_hscr(dag_t *dag, dict_t *dict, lm_t *lm, float64 lwf);

/**
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
S3DECODER_EXPORT
srch_hyp_t *dag_backtrace (srch_hyp_t **hyp, /**< A pointer of a pointer to the hypothesis*/
			   daglink_t *l,  /**< A pointer to the final link*/
			   float64 lwf,    /**< The language weight factor */
			   dict_t* dict,   /**< The dictionary*/
			   fillpen_t* fpen /**< The filler penalty structure */
    );

/**
 * writing the header of dag in Sphinx 3's format
 */
S3DECODER_EXPORT
void dag_write_header(FILE *fp, cmd_ln_t *config);

/**
 * Write a DAG (without segment scores) in Sphinx3 format
 **/
S3DECODER_EXPORT
int32 dag_write(dag_t * dag,
                const char *filename,
                lm_t * lm,
                dict_t * dict);

/**
 * Write a DAG (without segment scores) in HTK format
 **/
S3DECODER_EXPORT
int32 dag_write_htk(dag_t *dag,
                    const char *filename,
                    const char *uttid,
                    lm_t * lm,
                    dict_t * dict);


/**
 * Search a dag given language model (lm) and filler penalty struct (fpen)
 * Final global best path through DAG constructed from the word lattice.
 * Assumes that the DAG has already been constructed and is consistent with the word
 * lattice.
 * The search uses a recursive algorithm to find the best (reverse) path from the final
 * DAG node to the root:  The best path from any node (beginning with a particular link L)
 * depends on a similar best path for all links leaving the endpoint of L.  (This is
 * sufficient to handle trigram LMs.)
 */
S3DECODER_EXPORT
srch_hyp_t *dag_search (dag_t *dagp, /**< The initalized directed acyclic graph to search */
			char *utt,  /**< The utterance ID */
			float64 lwf,  /**< LM weight */
			dagnode_t *final,  /**< The final node, see source code in flat_fwd.c and dag.c */
			dict_t *dict,  /**< Dict */
			lm_t *lm,      /**< LM */
			fillpen_t *fpen /**< Fillpen */
    );

/**
 * Add fudges into a DAG 
 */

void dag_add_fudge_edges (dag_t* dagp,  /** An initialized DAG */
			  int32 fudge,  /**< Number of fudges */
			  int32 min_ef_range, /**< Minimum ending frame ranges */
			  void *lathist, /**< lattice history, compilation problem cause me to cast it as void. 
					    It should be in latticehist_t */
			  dict_t *dict  /**< Dictionary */
    );


/**
 * Add auxiliary links bypassing filler nodes in DAG.  In principle, a new such
 * auxiliary link can end up at ANOTHER filler node, and the process must be repeated
 * for complete transitive closure.  But removing fillers in the order in which they
 * appear in dag->list ensures that succeeding fillers have already been bypassed.
 * @return: 0 if successful; -1 if DAG maxedge limit exceeded.
 */
S3DECODER_EXPORT
int32 dag_bypass_filler_nodes (dag_t* dagp,  /**< DAG */
			       float64 lwf,  /**< language weight factor */
			       dict_t *dict,  /**< Dictionary */
			       fillpen_t *fpen /**< The filler penalty */
    );

/**
 * Remove links created by dag_bypass_filler_nodes().
 */
S3DECODER_EXPORT
void dag_remove_bypass_links(dag_t *dag);

/**
 * Remove nodes from which the final exit node is not reachable.
 */
S3DECODER_EXPORT
void dag_remove_unreachable(dag_t *dag);

/**
 * Load a DAG from a file: each unique <word-id,start-frame> is a node, i.e. with
 * a single start time but it can represent several end times.  Links are created
 * whenever nodes are adjacent in time.
 * dagnodes_list = linear list of DAG nodes allocated, ordered such that nodes earlier
 * in the list can follow nodes later in the list, but not vice versa:  Let two DAG
 * nodes d1 and d2 have start times sf1 and sf2, and end time ranges [fef1..lef1] and
 * [fef2..lef2] respectively.  If d1 appears later than d2 in dag.list, then
 * fef2 >= fef1, because d2 showed up later in the word lattice.  If there is a DAG
 * edge from d1 to d2, then sf1 > fef2.  But fef2 >= fef1, so sf1 > fef1.  Reductio ad
 * absurdum.
 * @return: 0 if successful, -1 otherwise.
 */
S3DECODER_EXPORT
dag_t * dag_load(char *file,          /**< Input: File to lod from */
           int32 maxedge,        /**< Maximum # of edges */
           float32 logbase,         /**< Logbase in float */
           int32 fudge,           /**< The number of fudges added */
           dict_t * dict,             /**< Dictionary */
           fillpen_t * fpen,          /**< Filler penalty structure */
           cmd_ln_t *config,
           logmath_t *logmath
    );

#ifdef __cplusplus
}
#endif


#endif
