/* ====================================================================
 * Copyright (c) 1993-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * HISTORY
 * 
 * 02-Aug-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed latnode_t.fef and latnode_t.lef to int32.
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_	1

#include <s2types.h>
#include <basic_types.h>

/*
 * Back pointer table (forward pass lattice; actually a tree)
 */
typedef struct bptbl_s {
    FRAME_ID frame;		/* start or end frame */
    WORD_ID  wid;		/* Word index */
    LAT_ID   bp;		/* Back Pointer */
    int32    score;		/* Score (best among all right contexts) */
    int32    s_idx;		/* Start of BScoreStack for various right contexts*/
    WORD_ID  real_fwid;		/* fwid of this or latest predecessor real word */
    WORD_ID  prev_real_fwid;	/* real word predecessor of real_fwid */
    int32    r_diph;		/* rightmost diphone of this word */
    int32    ascr;
    int32    lscr;
} BPTBL_T;

#define NO_BP		-1

/* #hyp entries */
#define HYP_SZ		1024

/* Interface */
int32 search_result(int32 *fr, char **res);	/* Decoded result as a single string */

/* Interface (OUT OF DATE as of 05-25-94!!) */
int32	search_get_score();
void	search_set_channels_per_frame_target (int32 cpf);
void	search_set_acc_threshold (int32 arg);

/* -------------- Lattice (DAG) search related -------------- */

/*
 * Links between DAG nodes (see latnode_t below).
 * Also used to keep scores in a bestpath search.
 */
typedef struct latlink_s {
    struct latnode_s *from;	/* From node */
    struct latnode_s *to;	/* To node */
    struct latlink_s *next;	/* Next link from the same "from" node */
    struct latlink_s *best_prev;
    struct latlink_s *q_next;
    int32 link_scr;		/* Score for from->wid (from->sf to this->ef) */
    int32 path_scr;		/* Best path score from root of DAG */
    int32 ef;			/* end-frame for the "from" node */
} latlink_t;

typedef struct rev_latlink_s {
    latlink_t *link;
    struct rev_latlink_s *next;
} rev_latlink_t;

/*
 * DAG nodes.
 */
typedef struct latnode_s {
    int32 wid;			/* Dictionary word id */
    int32 fef;			/* First end frame */
    int32 lef;			/* Last end frame */
    int16 sf;			/* Start frame */
    int16 reachable;		/* From </s> or <s> */
    union {
	int32 fanin;		/* #nodes with links to this node */
	int32 rem_score;	/* Estimated best score from node.sf to end */
    } info;
    latlink_t *links;		/* Links out of this node */
    rev_latlink_t *revlinks;	/* Reverse links (for housekeeping purposes only) */
    struct latnode_s *next;	/* Next node (for housekeeping purposes only) */
} latnode_t;

extern void sort_lattice ();
extern latnode_t *search_get_lattice ();

#endif
