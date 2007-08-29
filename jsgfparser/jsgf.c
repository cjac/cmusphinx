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

#include <ckd_alloc.h>
#include <string.h>
#include <err.h>

#include "jsgf.h"

/**
 * \file jsgf.c
 *
 * This file implements the data structures for parsing JSGF grammars
 * into Sphinx finite-state grammars.
 **/

jsgf_atom_t *
jsgf_atom_new(char *name, float weight)
{
    jsgf_atom_t *atom;

    atom = ckd_calloc(1, sizeof(*atom));
    atom->name = name;
    atom->weight = weight;
    return atom;
}

jsgf_t *
jsgf_grammar_new(char *version, char *charset, char *locale)
{
    jsgf_t *grammar;

    grammar = ckd_calloc(1, sizeof(*grammar));
    grammar->version = version;
    grammar->charset = charset;
    grammar->locale = locale;

    return grammar;
}

void
jsgf_add_link(jsgf_t *grammar, jsgf_atom_t *atom, int from, int to)
{
    jsgf_link_t *link;

    link = ckd_calloc(1, sizeof(*link));
    link->from = from;
    link->to = to;
    link->atom = atom;
    grammar->links = glist_add_ptr(grammar->links, link);
}

static int expand_rule(jsgf_t *grammar, jsgf_rule_t *rule);
static int
expand_rhs(jsgf_t *grammar, jsgf_rule_t *rule, jsgf_rhs_t *rhs)
{
    gnode_t *gn;
    int lastnode;

    /* Last node expanded in this sequence. */
    lastnode = rule->entry;

    /* Iterate over atoms in rhs and generate links/nodes */
    for (gn = rhs->atoms; gn; gn = gnode_next(gn)) {
        jsgf_atom_t *atom = gnode_ptr(gn);
        if (jsgf_atom_is_rule(atom)) {
            jsgf_rule_t *subrule;
            void *val;

            /* Special case for <NULL> and <VOID> pseudo-rules */
	    if (0 == strcmp(atom->name, "<NULL>")) {
                /* Emit a NULL transition */
                jsgf_add_link(grammar, atom,
                              lastnode, grammar->nstate);
                lastnode = grammar->nstate;
                ++grammar->nstate;
                continue;
	    }
            else if (0 == strcmp(atom->name, "<VOID>")) {
                /* Make this entire RHS unspeakable */
                return -1;
            }

            if (hash_table_lookup(grammar->rules, atom->name, &val) == -1) {
                E_ERROR("Undefined rule in RHS: %s\n", atom->name);
                return -1;
            }
            subrule = val;
            /* Allow right-recursion only. */
            if (subrule == rule) {
                if (gnode_next(gn) != NULL) {
                    E_ERROR("Only right-recursion is permitted (in %s)\n", rule->name);
                    return -1;
                }
                /* Add a link back to the beginning of this rule instance */
                E_INFO("Right recursion %s %d => %d\n", atom->name, lastnode, rule->entry);
                jsgf_add_link(grammar, atom, lastnode, rule->entry);
            }
            else {
                /* FIXME: Mutually recursive rules will break this massively. */
                /* Expand the subrule */
                if (expand_rule(grammar, subrule) == -1)
                    return -1;
                /* Add a link into the subrule. */
                jsgf_add_link(grammar, atom,
                         lastnode, subrule->entry);
                lastnode = subrule->exit;
            }
        }
        else {
            /* Add a link for this token and create a new exit node. */
            jsgf_add_link(grammar, atom,
                     lastnode, grammar->nstate);
            lastnode = grammar->nstate;
            ++grammar->nstate;
        }
    }

    return lastnode;
}

static int
expand_rule(jsgf_t *grammar, jsgf_rule_t *rule)
{
    jsgf_rhs_t *rhs;
    float norm;

    /* Normalize weights for all alternatives exiting rule->entry */
    norm = 0;
    for (rhs = rule->rhs; rhs; rhs = rhs->alt) {
        if (rhs->atoms) {
            jsgf_atom_t *atom = gnode_ptr(rhs->atoms);
            norm += atom->weight;
        }
    }

    rule->entry = grammar->nstate++;
    rule->exit = grammar->nstate++;
    if (norm == 0) norm = 1;
    for (rhs = rule->rhs; rhs; rhs = rhs->alt) {
        int lastnode;

        if (rhs->atoms) {
            jsgf_atom_t *atom = gnode_ptr(rhs->atoms);
	    atom->weight /= norm;
        }
        lastnode = expand_rhs(grammar, rule, rhs);
        if (lastnode == -1) {
            return -1;
        }
        else {
            jsgf_add_link(grammar, NULL, lastnode, rule->exit);
        }
    }
    return rule->exit;
}

int
jsgf_write_fsg(jsgf_t *grammar, jsgf_rule_t *rule, FILE *outfh)
{
    gnode_t *gn;

    /* Clear previous links */
    for (gn = grammar->links; gn; gn = gnode_next(gn)) {
        ckd_free(gnode_ptr(gn));
    }
    glist_free(grammar->links);
    grammar->links = NULL;
    rule->entry = rule->exit = 0;
    grammar->nstate = 0;

    expand_rule(grammar, rule);

    printf("FSG_BEGIN %s\n", rule->name);
    printf("NUM_STATES %d\n", grammar->nstate);
    printf("START_STATE %d\n", rule->entry);
    printf("FINAL_STATE %d\n", rule->exit);
    printf("\n# Transitions\n");

    grammar->links = glist_reverse(grammar->links);
    for (gn = grammar->links; gn; gn = gnode_next(gn)) {
        jsgf_link_t *link = gnode_ptr(gn);

        if (link->atom) {
            if (jsgf_atom_is_rule(link->atom)) {
                printf("TRANSITION %d %d %f\n",
                       link->from, link->to,
                       link->atom->weight);
            }
            else {
                printf("TRANSITION %d %d %f %s\n",
                       link->from, link->to,
                       link->atom->weight, link->atom->name);
            }
        }
        else {
            printf("TRANSITION %d %d %f\n", link->from, link->to, 1.0);
        }               
    }

    printf("FSG_END\n");

    return 0;
}
