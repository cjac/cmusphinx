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
 * @file senone_tree.c
 * @brief Senone trees for GMM selection.
 */

/* SphinxBase headesr. */
#include <ckd_alloc.h>
#include <bio.h>
#include <err.h>
#include <string.h>

/* Local headers. */
#include "senone_tree.h"

#define VERSION "0.1"

senone_tree_t *
senone_tree_read(char const *file, int max_nodes)
{
    senone_tree_t *stree;
    char **name, **val;
    FILE *fh;
    int32 swap;
    uint32 chksum;
    int i, n_feat, n_mixw, n_density;
    double logbase = 1.0001;
    uint32 tmp_chksum;
    long pos, endpos;
    int n_nodes;

    if ((fh = fopen(file, "rb")) == NULL)
        return NULL;
    if (bio_readhdr(fh, &name, &val, &swap) < 0)
        return NULL;

    E_INFO("Reading senone tree file: %s\n", file);
    n_feat = 1;
    for (i = 0; name[i]; ++i) {
        if (0 == strcmp(name[i], "n_mixw"))
            n_mixw = atoi(val[i]);
        else if (0 == strcmp(name[i], "n_feat"))
            n_feat = atoi(val[i]);
        else if (0 == strcmp(name[i], "n_density"))
            n_density = atoi(val[i]);
        else if (0 == strcmp(name[i], "logbase"))
            logbase = atof(val[i]);
        ckd_free(name[i]);
        ckd_free(val[i]);
    }
    ckd_free(name);
    ckd_free(val);

    /* FIXME: Do various parameter checking here. */
    stree = ckd_calloc(1, sizeof(*stree));
    stree->mixw_alloc = listelem_alloc_init(n_density * n_feat);
    stree->n_feat = n_feat;
    stree->n_mixw = n_mixw;
    stree->n_density = n_density;
    stree->logbase = (float32)logbase;

    /* Figure out where the end of this tree is. */
    tmp_chksum = 0;
    n_nodes = 0;
    pos = ftell(fh);
    while (!feof(fh)) {
        int16 shortval;

        /* Subtree pointers. */
        if (bio_fread(&shortval, 2, 1, fh, swap, &tmp_chksum) != 1) {
            senone_tree_free(stree);
            return NULL;
        }
        bio_fread(&shortval, 2, 1, fh, swap, &tmp_chksum);
        /* Bitvector offset. */
        bio_fread(&shortval, 2, 1, fh, swap, &tmp_chksum);
        /* Bitvector length. */
        bio_fread(&shortval, 2, 1, fh, swap, &tmp_chksum);
        /* This signifies the end of the tree. */
        if (shortval == 0)
            break;
        /* Skip past bitvector. */
        fseek(fh, shortval * 4, SEEK_CUR);
        /* Skip past mixture weights. */
        fseek(fh, n_density * n_feat, SEEK_CUR);
        ++n_nodes;
    }
    endpos = ftell(fh);
    fseek(fh, pos, SEEK_SET);

    /* In actual fact, this should be n_mixw * 2 - 1 */
    E_INFO("Number of nodes in tree: %d\n", n_nodes);
    if (max_nodes == -1)
        max_nodes = n_nodes;
    stree->n_nodes = max_nodes;
    stree->nodes = ckd_calloc(stree->n_nodes, sizeof(*stree->nodes));

    /* Now read in the nodes and find the start of the leaf nodes. */
    stree->first_leaf_node = 0;
    for (i = 0; i < max_nodes; ++i) {
        senone_node_t *node = stree->nodes + i;

        /* Subtree pointers. */
        bio_fread(&node->left, 2, 1, fh, swap, &chksum);
        bio_fread(&node->right, 2, 1, fh, swap, &chksum);
        /* If the subtree pointers point outside the allocated nodes
         * then stub them out. */
        if (i + node->left >= max_nodes)
            node->left = -1;
        if (i + node->right >= max_nodes)
            node->right = -1;
        /* Technically this should be &&, not sure why that doesn't work. */
        if ((node->left == -1 || node->right == -1)
            && stree->first_leaf_node == 0)
            stree->first_leaf_node = i;
        /* Compressed bitvector. */
        bio_fread(&node->leaf_start, 2, 1, fh, swap, &chksum);
        bio_fread(&node->n_leaf_bits, 2, 1, fh, swap, &chksum);
        node->leaf_start *= 32;
        node->n_leaf_bits *= 32;
        node->leaves = bitvec_alloc(node->n_leaf_bits);
        bio_fread(node->leaves, 4, node->n_leaf_bits / 32, fh, swap, &chksum);
        /* Actual mixture weight centroid. */
        node->mixw = listelem_malloc(stree->mixw_alloc);
        bio_fread(node->mixw, 1, n_density * n_feat, fh, swap, &chksum);
    }
    E_INFO("Start of leaf nodes: %d\n", stree->first_leaf_node);

    fclose(fh);
    return stree;
}

void
senone_tree_free(senone_tree_t *stree)
{
    int i;

    if (stree == NULL)
        return;
    for (i = 0; i < stree->n_nodes; ++i)
        bitvec_free(stree->nodes[i].leaves);
    ckd_free(stree->nodes);
    listelem_alloc_free(stree->mixw_alloc);
    ckd_free(stree);
}
