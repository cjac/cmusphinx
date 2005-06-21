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
 * $Log$
 * Revision 1.1  2005/06/21  22:37:47  arthchan2003
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

#include <s3types.h>
#include "search.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"

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

  uint8 reachable;                      /**< astar specific: Whether final node reachable from here */

} dagnode_t;

/** A DAG node can have several successor or predecessor nodes, each represented by a link */
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


    s3frmid_t ef;		/**< End time for this link.  Should be 1 before the start
				   time of destination node (or source node for reverse
				   links), but gets corrupted because of filler deletion */
    uint8 pscr_valid;		/**< Flag to avoid evaluating the same path multiple times */

  int32 is_filler_bypass;       /**< Astar specific:Whether this is a filler bypass link */
  int32 hscr;			/**< Astar specific:Heuristic score from end of link to dag exit node */

} daglink_t;

/** Summary of DAG structure information */
typedef struct {
    dagnode_t *list;		/**< Linear list of nodes allocated */
    dagnode_t *root;            /**< Corresponding to (<s>,0) */
    daglink_t final;            /**< Exit link from final DAG node */
    daglink_t entry;		/**< Entering (<s>,0) */
    daglink_t exit;		/**< Exiting (</s>,finalframe) */
    s3wid_t orig_exitwid;	/**< If original exit node is not a filler word */

    int32 nfrm;
    int32 nlink;
    int32 nbypass;
    int32 maxedge;              /**< (New in S3.6) Used in both dag and flat foward, this decides whether
				     parts of the dag code will exceed the maximum no of edge 
				 */

    int32 lmop;		        /**< (Temporary Variable): #LM ops actually made */
    int32 maxlmop;		/**< Max LM ops allowed before utterance aborted 
				 */

    int32 filler_removed;       /**< Whether filler nodes removed from DAG to help search */
    int32 fudged;               /**< Whether fudge edges have been added */

    s3latid_t latfinal;         /**< Lattice entry determined to be final end point */

} dag_t;

/** Clean up the hypothesis list */

void hyp_free (srch_hyp_t *list);

/** Initialize a dag_t */
void dag_init(dag_t* dagp);


/** Link two DAG nodes with the given arguments
 * @return 0 if successful, -1 if maxedge limit exceeded.
 */
int32 dag_link (dag_t * dagp,    /**< A pointer to a DAG */
		dagnode_t *pd,  
		dagnode_t *d,   
		int32 ascr,     /**< The acoustic scores */
		int32 ef,       /**< The ending frame */
		daglink_t *byp  
		);


daglink_t *find_succlink (dagnode_t *src, dagnode_t *dst);

daglink_t *find_predlink (dagnode_t *src, dagnode_t *dst);

int32 dag_update_link (dag_t* dagp,  /**< A pointer to a DAG */
		       dagnode_t *pd, 
		       dagnode_t *d, 
		       int32 ascr,  /**< The acoustic scores */
		       int32 ef,    /**< The ending frame */
		       daglink_t *byp
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
		    s3lmwid_t *dict2lmwid /**< A map from dictionary id to lm id, should use wid2lm insteead*/
		    ); 


/** Check whether the link score is larger than zero
 * @return 0 if not, return -1 otherwise. 
 */
int32 dag_chk_linkscr (
		       dag_t *dagp /**< A pointer to a DAG */
		       );

/** destroy a dag 
 */
int32 dag_destroy ( 
		   dag_t *dagp /**< A pointer to a DAG */
		   );

/**
 * Recursive backtrace through DAG (from final node to root) using daglink_t.history.
 * Restore bypassed links during backtrace.
 */
srch_hyp_t *dag_backtrace (srch_hyp_t *hyp, /**< A pointer to the hypothesis*/
			   daglink_t *l,  /**< A pointer to the final link*/
			   float64 lwf,    /**< The language weight factor */
			   dict_t* dict,   /**< The dictionary*/
			   fillpen_t* fpen /**< The filler penalty structure */
			   );


#endif
