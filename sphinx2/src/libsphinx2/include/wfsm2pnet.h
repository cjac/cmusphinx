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
 * wnet2pnet.h -- Generic phone-net data structure, build from a wordnet data structure.
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
 * 10-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBMAIN_PNET_H_
#define _LIBMAIN_PNET_H_


#include <libutil/libutil.h>
#include "s3types.h"
#include "mdef.h"
#include "dict.h"
#include "hmm.h"
#include "wnet.h"


/*
 * Phonetic equivalent representation of wordnet.  Another DAG with phone nodes and links.
 */
typedef struct pnode_s {
    s3wid_t wid;	/* Parent word-id for this HMM node */
    int32 pos;		/* Phone position within word */
    hmm_t hmm;		/* Search HMM structure */
    glist_t succ;	/* List of links (plink_t) to successor phone nodes */
} pnode_t;

typedef struct plink_s {
    pnode_t *dst;	/* Target node for a link */
} plink_t;


/*
 * Build a phone HMM net for the given word net.  The given wnet has two dummy anchors
 * (wstart and wend) designating the start and end of the wordnet.  They are ignored in
 * the phone net creation.  Similar dummy anchors are added to the phone net instead.
 * Return value: glist of pnode_t in the constructed phone net.
 */
glist_t wnet2pnet (mdef_t *mdef,	/* In: Model definition for building phone net */
		   dict_t *dict,	/* In: Pronunciation dictionary */
		   glist_t wnet,	/* In: Word net */
		   wnode_t *wstart,	/* In: Dummy anchors at the start and end of the
					   word net; ignored in the phone net */
		   wnode_t *wend,
		   pnode_t **pstart,	/* Out: Dummy anchors at the start and end of the
					   phone net; they should be ignored.  Successors
					   to pstart are the real start of the net, and
					   predecessors to pend are the real end. */
		   pnode_t **pend);

/*
 * Free and destroy the given phone net.
 */
void pnet_free (glist_t pnet);


/*
 * Flag the set of senones in the given pnode list plist in the given bitvector.
 */
void pnet_set_senactive (mdef_t *m,		/* In: HMM model definition */
			 glist_t plist,		/* In: List of pnodes active */
			 bitvec_t active,	/* Out: Bit-vector for the active flags;
						   active[i] set iff senone i is active */
			 int32 n_sen);		/* In: Size of bit-vector */

/*
 * Dump for debugging.
 */
void pnet_dump (mdef_t *mdef, dict_t *dict, glist_t pnet);


#endif
