
/*
 * align.h -- Exported time-aligner functions and data structures.
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
 * Revision 1.1  2004/08/09  00:17:12  arthchan2003
 * Incorporating s3.0 align, at this point, there are still some small problems in align but they don't hurt. For example, the score doesn't match with s3.0 and the output will have problem if files are piped to /dev/null/. I think we can go for it.
 * 
 * Revision 1.1  2003/02/14 14:40:34  cbq
 * Compiles.  Analysis is probably hosed.
 *
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * 
 * 13-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed align_sen_active to flag active senones instead of building a list
 * 		of them.
 * 
 * 15-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

#ifndef _LIBFBS_DAG_H_
#define _LIBFBS_DAG_H_

#include <libutil/libutil.h>
#include "s3types.h"
#include "search.h"
#include "dict.h"

/*
 * DAG structure representation of word lattice.  A unique <wordid,startframe> is a node.
 * Edges are formed if permitted by time adjacency.  (See comment before dag_build.)
 */
typedef struct dagnode_s {
    s3wid_t wid;
    int32 seqid;			/* Running sequence no. for identification */
    s3frmid_t sf;			/* Start frame for this occurrence of wid */
    s3frmid_t fef, lef;			/* First and last end frames */
  uint8 reachable;                      /* Whether final node reachable from here */
    struct dagnode_s *alloc_next;	/* Next in linear list of allocated nodes */
    struct daglink_s *succlist;		/* List of successor nodes (adjacent in time) */
    struct daglink_s *predlist;		/* List of preceding nodes (adjacent in time) */
} dagnode_t;

/* A DAG node can have several successor or predecessor nodes, each represented by a link */
typedef struct daglink_s {
    dagnode_t *node;		/* Target of link (source determined by dagnode_t.succlist
				   or dagnode_t.predlist) */
    dagnode_t *src;		/* Source node of link */
    struct daglink_s *next;	/* Next in same dagnode_t.succlist or dagnode_t.predlist */
    struct daglink_s *history;	/* Previous link along best path (for traceback) */
    struct daglink_s *bypass;	/* If this links A->B, bypassing A->fillnode->B, then
				   bypass is ptr to fillnode->B */
    int32 ascr;			/* Acoustic score for segment of source node ending just

				   before the end point of this link.  (Actually this gets

				   corrupted because of filler node deletion.) */
    int32 hscr;			/* Heuristic score from end of link to dag exit node */
    int32 lscr;			/* LM score to the SUCCESSOR node */
    int32 pscr;			/* Best path score to root beginning with this link */
    s3frmid_t ef;		/* End time for this link.  Should be 1 before the start
				   time of destination node (or source node for reverse
				   links), but gets corrupted because of filler deletion */
    uint8 pscr_valid;		/* Flag to avoid evaluating the same path multiple times */
} daglink_t;

/* Summary of DAG structure information */
typedef struct {
    dagnode_t *list;		/* Linear list of nodes allocated */
    dagnode_t *root;            /* Corresponding to (<s>,0) */
    daglink_t final;            /* Exit link from final DAG node */
    daglink_t entry;		/* Entering (<s>,0) */
    daglink_t exit;		/* Exiting (</s>,finalframe) */
    s3wid_t orig_exitwid;	/* If original exit node is not a filler word */

    int32 nfrm;
    int32 nlink;
    int32 nbypass;

    int32 filler_removed;       /* Whether filler nodes removed from DAG to help search */
    int32 fudged;               /* Whether fudge edges have been added */

    s3latid_t latfinal;         /* Lattice entry determined to be final end point */

} dag_t;

srch_hyp_t *s3dag_dag_search (char *utt);
int32 s3dag_dag_load (char *file);
int32 dag_destroy ( void );
void dag_init (dict_t* _dict);

#endif
