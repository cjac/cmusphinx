/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2007 Carnegie Mellon University.  All rights
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

/**
 * \file astar.h - Routines for A* search over DAGs.
 **/

#ifndef __ASTAR_H__
#define __ASTAR_H__

#include <s3types.h>
#include "dag.h"
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

/**
 * State object for A* search (for obtaining N-best lists)
 **/
typedef struct astar_s astar_t;

/**
 * Initialize A* search
 **/
S3DECODER_EXPORT
astar_t *astar_init(dag_t *dag, dict_t *dict, lm_t *lm,
                    fillpen_t *fpen,
                    float64 beam, /**< Pruning beam width */
                    float64 lwf   /**< Language weight factor (usually 1.0) */
    );

/**
 * Clean up after A* search
 **/
S3DECODER_EXPORT
void astar_free(astar_t *astar);

/**
 * Get next best hypothesis from A* search
 * @return a glist_t of srch_hyp_t (you are responsible for freeing this)
 * or NULL if no more hypotheses are available.
 **/
S3DECODER_EXPORT
glist_t astar_next_hyp(astar_t *astar);

/**
 * Batch-mode function for N-best search.
 * Does A* search and writes the results to the file specified.
 */
S3DECODER_EXPORT
void nbest_search(dag_t *dag, char *filename, char *uttid, float64 lwf,
                  dict_t *dict, lm_t *lm, fillpen_t *fpen);

#ifdef __cplusplus
}
#endif


#endif /* __ASTAR_H__ */
