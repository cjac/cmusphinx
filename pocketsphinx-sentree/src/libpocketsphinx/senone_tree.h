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
 * @file senone_tree.h
 * @brief Senone trees for GMM selection.
 */

#ifndef __SENONE_TREE_H__
#define __SENONE_TREE_H__

/* SphinxBase headesr. */
#include <sphinx_types.h>
#include <logmath.h>
#include <bitvec.h>
#include <listelem_alloc.h>

/* Local headers. */

/**
 * Node in a senone tree.
 */
typedef struct senone_node_s {
    int16 left;         /**< Offset of left-hand subnode. */
    int16 right;        /**< Offset of right-hand subnode. */
    int16 leaf_start;   /**< Offset of leaves inside senone vector, in bits. */
    int16 n_leaf_bits;  /**< Length of leaves in bits. */
    bitvec_t *leaves;   /**< Bitvector of senones under this node. */
    uint8 *mixw;        /**< Mixture weights (for all features). */
} senone_node_t;

/**
 * Senone tree
 */
typedef struct senone_tree_s {
    senone_node_t *nodes;
    listelem_alloc_t *mixw_alloc;
    int32 first_leaf_node;
    int32 n_nodes;
    int32 n_feat;
    int32 n_mixw;
    int32 n_density;
    float32 logbase;
} senone_tree_t;

senone_tree_t *senone_tree_read(char const *file, int max_nodes);

void senone_tree_free(senone_tree_t *stree);

#endif /* __SENONE_TREE_H__ */
