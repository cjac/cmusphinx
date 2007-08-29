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
#include <string.h>

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
    if (name == NULL) {
        name = ckd_calloc(16, 1);
        sprintf(name, "<g%05d>", hash_table_inuse(global_symtab));
    }

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
%type  <rule>    rule_group rule_optional
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

rule_group: '(' alternate_list ')' { $$ = define_rule(NULL, $2, 0); }
	;

rule_optional: '[' alternate_list ']' {
	       jsgf_rhs_t *rhs = ckd_calloc(1, sizeof(*rhs));
	       jsgf_atom_t *atom = jsgf_atom_new("<NULL>", 1.0);
	       rhs->alt = $2;
	       rhs->atoms = glist_add_ptr(NULL, atom);
	       $$ = define_rule(NULL, rhs, 0); }
	;

rule_atom: TOKEN { $$ = jsgf_atom_new($1, 1.0); }
	| RULENAME { $$ = jsgf_atom_new($1, 1.0); }
	| rule_group { $$ = jsgf_atom_new($1->name, 1.0); }
	| rule_optional { $$ = jsgf_atom_new($1->name, 1.0); }
	| rule_atom '*' {
	  jsgf_rule_t *rule;
	  jsgf_atom_t *atom;
	  jsgf_rhs_t *rhs;

	  rhs = ckd_calloc(1, sizeof(*rhs));
	  rhs->atoms = glist_add_ptr(NULL, jsgf_atom_new("<NULL>", 1.0));
	  rule = define_rule(NULL, rhs, 0);
	  atom = jsgf_atom_new(rule->name, 1.0);
	  rhs = ckd_calloc(1, sizeof(*rhs));
	  rhs->atoms = glist_add_ptr(NULL, atom);
	  rhs->atoms = glist_add_ptr(rhs->atoms, $1);
	  rule->rhs->alt = rhs;
	  $$ = atom;
}
	| rule_atom '+' {
	  jsgf_rule_t *rule;
	  jsgf_atom_t *atom;
	  jsgf_rhs_t *rhs;

	  rhs = ckd_calloc(1, sizeof(*rhs));
	  rhs->atoms = glist_add_ptr(NULL, $1);
	  rule = define_rule(NULL, rhs, 0);
	  atom = jsgf_atom_new(rule->name, 1.0);
	  rhs = ckd_calloc(1, sizeof(*rhs));
	  rhs->atoms = glist_add_ptr(NULL, atom);
	  rhs->atoms = glist_add_ptr(rhs->atoms, $1);
	  rule->rhs->alt = rhs;
	  $$ = atom;
}
	;

%%

extern FILE *yyin;

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
            jsgf_write_fsg(grammar, rule, stdout);
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
