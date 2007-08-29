/* -*- c-basic-offset:4; indent-tabs-mode: nil -*- */
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
%{
#include <stdio.h>

#include <hash_table.h>
#include <ckd_alloc.h>
#include <err.h>

#include "jsgf.h"

/** The global output grammar. */
static jsgf_t *global_grammar;
/** The global symbol (rule) table. */
static hash_table_t *global_symtab;

static jsgf_rule_t *
define_rule(char *name, jsgf_rhs_t *rhs, int public)
{
    jsgf_rule_t *rule;
    void *val;

    if (global_symtab == NULL) {
	global_symtab = hash_table_new(42, 0);
    }

    rhs->atoms = glist_reverse(rhs->atoms);

    rule = ckd_calloc(1, sizeof(*rule));
    rule->name = name;
    rule->rhs = rhs;
    rule->public = public;

    E_INFO("Defined rule: %s%s\n",
           rule->public ? "PUBLIC " : "",
           rule->name);
    val = hash_table_enter(global_symtab, name, rule);
    if (val != (void *)rule) {
	E_WARN("Multiply defined symbol: %s\n", name);
    }
    return rule;
}

static jsgf_atom_t *
jsgf_atom_new(char *name, float weight)
{
    jsgf_atom_t *atom;

    atom = ckd_calloc(1, sizeof(*atom));
    atom->name = name;
    atom->weight = weight;
    return atom;
}

static jsgf_t *
jsgf_grammar_new(char *version, char *charset, char *locale)
{
    jsgf_t *grammar;

    grammar = ckd_calloc(1, sizeof(*grammar));
    grammar->version = version;
    grammar->charset = charset;
    grammar->locale = locale;

    return grammar;
}

%}

%union {
       char *name;
       float weight;
       jsgf_t *grammar;
       jsgf_rule_t *rule;
       jsgf_rhs_t *rhs;
       jsgf_atom_t *atom;
}

%token           HEADER GRAMMAR IMPORT PUBLIC
%token <name>    TOKEN RULENAME TAG
%token <weight>  WEIGHT
%type  <atom>    rule_atom rule_item tagged_rule_item
%type  <rhs>     rule_expansion alternate_list
%type  <grammar> jsgf_header header grammar
%type  <name>    grammar_header
%%

grammar: header { $$ = $1; global_grammar = $$; }
	| header rule_list { $$ = $1;
			     $$->rules = global_symtab;
			     global_grammar = $$; }
	;

header: jsgf_header grammar_header { $$ = $1; $$->name = $2; }
	| jsgf_header grammar_header import_header { $$ = $1;
						     $$->name = $2; }
	;

jsgf_header: HEADER ';' { $$ = jsgf_grammar_new(NULL, NULL, NULL); }
	| HEADER TOKEN ';' { $$ = jsgf_grammar_new($2, NULL, NULL); }
	| HEADER TOKEN TOKEN ';' { $$ = jsgf_grammar_new($2, $3, NULL); }
	| HEADER TOKEN TOKEN TOKEN ';' { $$ = jsgf_grammar_new($2, $3, $4); }
	;

grammar_header: GRAMMAR TOKEN ';' { $$ = $2; }
	;

import_header: import_statement
	| import_header import_statement
	;

import_statement: IMPORT RULENAME ';'
	;

rule_list: rule
	| rule_list rule
	;

rule: RULENAME '=' alternate_list ';' { define_rule($1, $3, 0); }
	| PUBLIC RULENAME '=' alternate_list ';'  { define_rule($2, $4, 1); }
	;

alternate_list: rule_expansion
	| alternate_list '|' rule_expansion { $$ = $3; $$->alt = $1; }
	;

rule_expansion: tagged_rule_item { $$ = ckd_calloc(1, sizeof(*$$));
				   $$->atoms = glist_add_ptr($$->atoms, $1); }
	| rule_expansion tagged_rule_item { $$ = $1;
					    $$->atoms = glist_add_ptr($$->atoms, $2); }
	;

tagged_rule_item: rule_item
	| tagged_rule_item TAG { $$ = $1;
				 $$->tags = glist_add_ptr($$->tags, $2); }
	;

rule_item: rule_atom
	| WEIGHT rule_atom { $$ = $2; $$->weight = $1; }
	;

/*
rule_group: '(' alternate_list ')'
	;

rule_optional: '[' alternate_list ']'
	;
*/

rule_atom: TOKEN  { $$ = jsgf_atom_new($1, 1.0); }
	| RULENAME  { $$ = jsgf_atom_new($1, 1.0); }
/*	| rule_group
	| rule_optional
	| rule_atom '*'
	| rule_atom '+' */
	;

%%

extern FILE *yyin;

static int expand_rule(jsgf_t *grammar, jsgf_rule_t *rule);

static void
add_link(jsgf_t *grammar, jsgf_atom_t *atom, int from, int to)
{
    jsgf_link_t *link;

    link = ckd_calloc(1, sizeof(*link));
    link->from = from;
    link->to = to;
    link->atom = atom;
    grammar->links = glist_add_ptr(grammar->links, link);
}

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
                add_link(grammar, atom, lastnode, rule->entry);
            }
            else {
                /* FIXME: Mutually recursive rules will break this massively. */
                /* Expand the subrule */
                if (expand_rule(grammar, subrule) == -1)
                    return -1;
                /* Add a link into the subrule. */
                add_link(grammar, atom,
                         lastnode, subrule->entry);
                lastnode = subrule->exit;
            }
        }
        else {
            /* Add a link for this token and create a new exit node. */
            add_link(grammar, atom,
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

    rule->entry = grammar->nstate++;
    rule->exit = grammar->nstate++;

    /* Normalize incoming weights */
    norm = 0;
    for (rhs = rule->rhs; rhs; rhs = rhs->alt) {
        if (rhs->atoms) {
            jsgf_atom_t *atom = gnode_ptr(rhs->atoms);
            norm += atom->weight;
        }
    }
    if (norm == 0) norm = 1;
    for (rhs = rule->rhs; rhs; rhs = rhs->alt) {
        if (rhs->atoms) {
            jsgf_atom_t *atom = gnode_ptr(rhs->atoms);
	    atom->weight /= norm;
        }
    }

    for (rhs = rule->rhs; rhs; rhs = rhs->alt) {
        int lastnode;

        lastnode = expand_rhs(grammar, rule, rhs);
        if (lastnode == -1) {
            return -1;
        }
        add_link(grammar, NULL, lastnode, rule->exit);
    }
    return rule->exit;
}

static int
write_fsg(jsgf_t *grammar, jsgf_rule_t *rule)
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

static int
write_fsgs(jsgf_t *grammar)
{
    glist_t rules;
    int32 nrules;
    gnode_t *gn;

    /* Generate an FSG for each "public" rule in the grammar. */
    rules = hash_table_tolist(grammar->rules, &nrules);
    for (gn = rules; gn; gn = gnode_next(gn)) {
        hash_entry_t *he = gnode_ptr(gn);
        jsgf_rule_t *rule = hash_entry_val(he);

        if (rule->public) {
            E_INFO("Top-level rule: %s\n", rule->name);
            write_fsg(grammar, rule);
        }
    }
    glist_free(rules);

    return 0;
}

int
main(int argc, char *argv[])
{
    int yyrv;

    /* yydebug = 1; */
    yyrv = yyparse();
    if (yyrv != 0) {
        fprintf(stderr, "JSGF parse failed\n");
    }
    else {
        write_fsgs(global_grammar);
    }

    return yyrv;
}

int
yyerror(char *s)
{
    fprintf(stderr, "ERROR: %s\n", s);
    return 0;
}
