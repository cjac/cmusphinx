/*
 * wnet.h -- Generic word-net data structure
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
 * 28-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBMAIN_WNET_H_
#define _LIBMAIN_WNET_H_


#include <libutil/libutil.h>
#include "s3types.h"


/* A node in the wordnet */
typedef struct {
    s3wid_t wid;
    glist_t succ;	/* List of links to successor wnode_t */
    glist_t pred;	/* List of predecessor wnode_t */
    int32 data;		/* Placeholder for any user-defined data */
} wnode_t;

/* A link or edge in the wordnet */
typedef struct {
    wnode_t *dst;	/* Destination node for this link */
} wlink_t;


#endif
