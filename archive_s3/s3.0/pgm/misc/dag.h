/*
 * dag.h -- DAG structure containing word instances and edges defining adjacency.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 07-Feb-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _DAG_H_
#define _DAG_H_


#include <libutil/libutil.h>
#include <main/s3types.h>
#include <main/dict.h>


/*
 * DAG structure representation of word lattice.  A unique <wordid,startframe> is a node.
 * Edges are formed if permitted by time adjacency.
 */
typedef struct dagnode_s {
    s3wid_t wid;
    int32 seqid;		/* Running sequence no. for identification */
    int32 reachable;		/* Whether DAG end points reachable from here */
    s3frmid_t sf;		/* Start frame for this occurrence of wid */
    s3frmid_t fef, lef;		/* First and last end frames */
    struct dagnode_s *next;	/* Next in linear list of allocated nodes */
    struct daglink_s *succlist;	/* List of successor nodes (adjacent in time) */
    struct daglink_s *predlist;	/* List of preceding nodes (adjacent in time) */
} dagnode_t;

/*
 * A DAG node can have several successor or predecessor nodes, each represented by a link
 */
typedef struct daglink_s {
    dagnode_t *src;		/* Source node of link */
    dagnode_t *dst;		/* Target node of link */
    struct daglink_s *next;	/* Next in same dagnode_t.succlist or dagnode_t.predlist */
} daglink_t;

/* Summary of DAG structure information */
typedef struct {
    dagnode_t **node_sf;	/* node_sf[f] = list of dagnodes with start time = f */
    daglink_t entry;		/* Entering (<s>,0) */
    daglink_t exit;		/* Exiting (</s>,finalframe) */
    int32 nfrm;			/* #Frames in utterance */
    int32 nnode;		/* #Nodes in DAG */
    int32 nlink;		/* #Links in DAG */
} dag_t;


/*
 * Mark d and all successors along path to end as reachable.
 */
void dag_reachable_fwd (dagnode_t *d);


/*
 * Mark d and all predecessors along path to root as reachable.
 */
void dag_reachable_bwd (dagnode_t *d);


/*
 * Add a dummy node with the given wid to the end of the DAG; link all ending nodes to it.
 */
void dag_append_sentinel (dag_t *dag, s3wid_t wid);


/*
 * Link two DAG nodes: dst is in succlist of src and src is in predlist of dst.
 */
void dag_link (dagnode_t *src, dagnode_t *dst);


/*
 * Destroy the given DAG and deallocate its space.
 */
void dag_destroy (dag_t *dag);

void dag_dump (dag_t *dag, dict_t *dict);


#endif
