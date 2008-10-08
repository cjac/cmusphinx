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

#include <string.h>
#include <assert.h>

#include "ckd_alloc.h"
#include "strfuncs.h"
#include "hash_table.h"
#include "err.h"

#include "jsgf_internal.h"
#include "jsgf_parser.h"
#include "jsgf_scanner.h"

int yyparse (yyscan_t yyscanner, jsgf_t *jsgf);

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
    atom->name = ckd_salloc(name);
    atom->weight = weight;
    return atom;
}

void
jsgf_atom_free(jsgf_atom_t *atom)
{
    if (atom == NULL)
        return;
    ckd_free(atom->name);
    ckd_free(atom);
}

jsgf_t *
jsgf_grammar_new(jsgf_t *parent)
{
    jsgf_t *grammar;

    grammar = ckd_calloc(1, sizeof(*grammar));
    /* If this is an imported/subgrammar, then we will share a global
     * namespace with the parent grammar. */
    if (parent) {
        grammar->rules = parent->rules;
        grammar->imports = parent->imports;
        grammar->searchpath = parent->searchpath;
        grammar->parent = parent;
    }
    else {
        char *jsgf_path;

        grammar->rules = hash_table_new(64, 0);
        grammar->imports = hash_table_new(16, 0);
        if ((jsgf_path = getenv("JSGF_PATH")) != NULL) {
            char *word, *c;

            /* FIXME: This should be a function in libsphinxbase. */
            /* FIXME: Also nextword() is totally useless... */
            word = jsgf_path = ckd_salloc(jsgf_path);
            while ((c = strchr(word, ':'))) {
                *c = '\0';
                grammar->searchpath = glist_add_ptr(grammar->searchpath, word);
                word = c + 1;
            }
            grammar->searchpath = glist_add_ptr(grammar->searchpath, word);
            grammar->searchpath = glist_reverse(grammar->searchpath);
        }
        else {
            /* Default to current directory. */
            grammar->searchpath = glist_add_ptr(grammar->searchpath, ckd_salloc("."));
        }
    }

    return grammar;
}

void
jsgf_grammar_free(jsgf_t *jsgf)
{
    /* FIXME: Probably should just use refcounting instead. */
    if (jsgf->parent == NULL) {
        hash_iter_t *itor;
        gnode_t *gn;

        for (itor = hash_table_iter(jsgf->rules); itor;
             itor = hash_table_iter_next(itor)) {
            ckd_free((char *)itor->ent->key);
            jsgf_rule_free((jsgf_rule_t *)itor->ent->val);
        }
        hash_table_free(jsgf->rules);
        for (itor = hash_table_iter(jsgf->imports); itor;
             itor = hash_table_iter_next(itor))
            ckd_free((char *)itor->ent->key);
        hash_table_free(jsgf->imports);
        for (gn = jsgf->searchpath; gn; gn = gnode_next(gn))
            ckd_free(gnode_ptr(gn));
        glist_free(jsgf->searchpath);
        for (gn = jsgf->links; gn; gn = gnode_next(gn))
            ckd_free(gnode_ptr(gn));
        glist_free(jsgf->links);
    }
    ckd_free(jsgf->name);
    ckd_free(jsgf->version);
    ckd_free(jsgf->charset);
    ckd_free(jsgf->locale);
    ckd_free(jsgf);
}

static void
jsgf_rhs_free(jsgf_rhs_t *rhs)
{
    gnode_t *gn;

    if (rhs == NULL)
        return;

    jsgf_rhs_free(rhs->alt);
    for (gn = rhs->atoms; gn; gn = gnode_next(gn))
        jsgf_atom_free(gnode_ptr(gn));
    glist_free(rhs->atoms);
    ckd_free(rhs);
}

jsgf_atom_t *
jsgf_kleene_new(jsgf_t *jsgf, jsgf_atom_t *atom, int plus)
{
    jsgf_rule_t *rule;
    jsgf_atom_t *rule_atom;
    jsgf_rhs_t *rhs;

    /* Generate an "internal" rule of the form (<NULL> | <name> <g0006>) */
    /* Or if plus is true, (<name> | <name> <g0006>) */
    rhs = ckd_calloc(1, sizeof(*rhs));
    if (plus)
        rhs->atoms = glist_add_ptr(NULL, jsgf_atom_new(atom->name, 1.0));
    else
        rhs->atoms = glist_add_ptr(NULL, jsgf_atom_new("<NULL>", 1.0));
    rule = jsgf_define_rule(jsgf, NULL, rhs, 0);
    rule_atom = jsgf_atom_new(rule->name, 1.0);
    rhs = ckd_calloc(1, sizeof(*rhs));
    rhs->atoms = glist_add_ptr(NULL, rule_atom);
    rhs->atoms = glist_add_ptr(rhs->atoms, atom);
    rule->rhs->alt = rhs;

    return jsgf_atom_new(rule->name, 1.0);
}

jsgf_rule_t *
jsgf_optional_new(jsgf_t *jsgf, jsgf_rhs_t *exp)
{
    jsgf_rhs_t *rhs = ckd_calloc(1, sizeof(*rhs));
    jsgf_atom_t *atom = jsgf_atom_new("<NULL>", 1.0);
    rhs->alt = exp;
    rhs->atoms = glist_add_ptr(NULL, atom);
    return jsgf_define_rule(jsgf, NULL, rhs, 0);
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

static char *
extract_grammar_name(char *rule_name)
{
    char* dot_pos;
    char* grammar_name = ckd_salloc(rule_name+1);
    if ((dot_pos = strrchr(grammar_name + 1, '.')) == NULL) {
        ckd_free(grammar_name);
        return NULL;
    }
    *dot_pos='\0';
    return grammar_name;
}

static char *
jsgf_fullname(jsgf_t *jsgf, const char *name)
{
    char *fullname;

    /* Check if it is already qualified */
    if (strchr(name + 1, '.'))
        return ckd_salloc(name);

    /* Skip leading < in name */
    fullname = ckd_malloc(strlen(jsgf->name) + strlen(name) + 4);
    sprintf(fullname, "<%s.%s", jsgf->name, name + 1);
    return fullname;
}

static char *
jsgf_fullname_from_rule(jsgf_rule_t *rule, const char *name)
{
    char *fullname, *grammar_name;

    /* Check if it is already qualified */
    if (strchr(name + 1, '.'))
        return ckd_salloc(name);

    /* Skip leading < in name */
    if ((grammar_name = extract_grammar_name(rule->name)) == NULL)
        return ckd_salloc(name);
    fullname = ckd_malloc(strlen(grammar_name) + strlen(name) + 4);
    sprintf(fullname, "<%s.%s", grammar_name, name + 1);
    ckd_free(grammar_name);

    return fullname;
}

/* Extract as rulename everything after the secondlast dot, if existent. 
 * Because everything before the secondlast dot is the path-specification. */
static char *
importname2rulename(char *importname)
{
    char *rulename = ckd_salloc(importname);
    char *last_dotpos;
    char *secondlast_dotpos;

    if ((last_dotpos = strrchr(rulename+1, '.')) != NULL) {
        *last_dotpos='\0';
        if ((secondlast_dotpos = strrchr(rulename+1, '.')) != NULL) {
            *last_dotpos='.';
            *secondlast_dotpos='<';
            secondlast_dotpos = ckd_salloc(secondlast_dotpos);
            ckd_free(rulename);
            return secondlast_dotpos;
        }
        else {
            *last_dotpos='.';
            return rulename;
        }
    }
    else {
        return rulename;
    }
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
            char *fullname;
            gnode_t *subnode;
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

            fullname = jsgf_fullname_from_rule(rule, atom->name);
            if (hash_table_lookup(grammar->rules, fullname, &val) == -1) {
                E_ERROR("Undefined rule in RHS: %s\n", fullname);
                ckd_free(fullname);
                return -1;
            }
            ckd_free(fullname);
            subrule = val;
            /* Look for this in the stack of expanded rules */
            for (subnode = grammar->rulestack; subnode; subnode = gnode_next(subnode))
                if (gnode_ptr(subnode) == (void *)subrule)
                    break;
            if (subnode != NULL) {
                /* Allow right-recursion only. */
                if (gnode_next(gn) != NULL) {
                    E_ERROR("Only right-recursion is permitted (in %s.%s)\n",
                            grammar->name, rule->name);
                    return -1;
                }
                /* Add a link back to the beginning of this rule instance */
                E_INFO("Right recursion %s %d => %d\n", atom->name, lastnode, subrule->entry);
                jsgf_add_link(grammar, atom, lastnode, subrule->entry);
            }
            else {
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

    /* Push this rule onto the stack */
    grammar->rulestack = glist_add_ptr(grammar->rulestack, rule);

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

    /* Pop this rule from the rule stack */
    grammar->rulestack = gnode_free(grammar->rulestack, NULL);
    return rule->exit;
}

jsgf_rule_iter_t *
jsgf_rule_iter(jsgf_t *grammar)
{
    return hash_table_iter(grammar->rules);
}

jsgf_rule_t *
jsgf_get_rule(jsgf_t *grammar, char const *name)
{
    void *val;

    if (hash_table_lookup(grammar->rules, name, &val) < 0)
        return NULL;
    return (jsgf_rule_t *)val;
}

char const *
jsgf_rule_name(jsgf_rule_t *rule)
{
    return rule->name;
}

int
jsgf_rule_public(jsgf_rule_t *rule)
{
    return rule->public;
}

fsg_model_t *
jsgf_build_fsg(jsgf_t *grammar, jsgf_rule_t *rule, logmath_t *lmath, float32 lw)
{
    fsg_model_t *fsg;
    glist_t nulls;
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

    fsg = fsg_model_init(rule->name, lmath, lw, grammar->nstate);
    fsg->start_state = rule->entry;
    fsg->final_state = rule->exit;
    grammar->links = glist_reverse(grammar->links);
    for (gn = grammar->links; gn; gn = gnode_next(gn)) {
        jsgf_link_t *link = gnode_ptr(gn);

        if (link->atom) {
            if (jsgf_atom_is_rule(link->atom)) {
                fsg_model_null_trans_add(fsg, link->from, link->to,
                                        logmath_log(lmath, link->atom->weight));
            }
            else {
                int wid = fsg_model_word_add(fsg, link->atom->name);
                fsg_model_trans_add(fsg, link->from, link->to,
                                   logmath_log(lmath, link->atom->weight), wid);
            }
        }
        else {
            fsg_model_null_trans_add(fsg, link->from, link->to, 0);
        }            
    }
    nulls = fsg_model_null_trans_closure(fsg, NULL);
    glist_free(nulls);

    return fsg;
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

jsgf_rule_t *
jsgf_define_rule(jsgf_t *jsgf, char *name, jsgf_rhs_t *rhs, int public)
{
    jsgf_rule_t *rule;
    void *val;

    if (name == NULL) {
        name = ckd_malloc(strlen(jsgf->name) + 16);
        sprintf(name, "<%s.g%05d>", jsgf->name, hash_table_inuse(jsgf->rules));
    }
    else {
        char *newname;

        newname = jsgf_fullname(jsgf, name);
        name = newname;
    }

    rule = ckd_calloc(1, sizeof(*rule));
    rule->name = ckd_salloc(name);
    rule->rhs = rhs;
    rule->public = public;

    E_INFO("Defined rule: %s%s\n",
           rule->public ? "PUBLIC " : "",
           rule->name);
    val = hash_table_enter(jsgf->rules, name, rule);
    if (val != (void *)rule) {
        E_WARN("Multiply defined symbol: %s\n", name);
    }
    return rule;
}

void
jsgf_rule_free(jsgf_rule_t *rule)
{
    jsgf_rhs_free(rule->rhs);
    ckd_free(rule->name);
    ckd_free(rule);
}


/* FIXME: This should go in libsphinxutil */
static char *
path_list_search(glist_t paths, char *path)
{
    gnode_t *gn;

    for (gn = paths; gn; gn = gnode_next(gn)) {
        char *fullpath;
        FILE *tmp;

        fullpath = string_join(gnode_ptr(gn), "/", path, NULL);
        tmp = fopen(fullpath, "r");
        if (tmp != NULL) {
            fclose(tmp);
            return fullpath;
        }
        else
            ckd_free(fullpath);
    }
    return NULL;
}

jsgf_rule_t *
jsgf_import_rule(jsgf_t *jsgf, char *name)
{
    char *c, *path, *newpath;
    size_t namelen, packlen;
    void *val;
    jsgf_t *imp;
    int import_all;

    /* Trim the leading and trailing <> */
    namelen = strlen(name);
    path = ckd_malloc(namelen - 2 + 6); /* room for a trailing .gram */
    strcpy(path, name + 1);
    /* Split off the first part of the name */
    c = strrchr(path, '.');
    if (c == NULL) {
        E_ERROR("Imported rule is not qualified: %s\n", name);
        ckd_free(path);
        return NULL;
    }
    packlen = c - path;
    *c = '\0';

    /* Look for import foo.* */
    import_all = (strlen(name) > 2 && 0 == strcmp(name + namelen - 3, ".*>"));

    /* Construct a filename. */
    for (c = path; *c; ++c)
        if (*c == '.') *c = '/';
    strcat(path, ".gram");
    newpath = path_list_search(jsgf->searchpath, path);
    ckd_free(path);
    if (newpath == NULL)
        return NULL;

    path = newpath;
    E_INFO("Importing %s from %s to %s\n", name, path, jsgf->name);

    /* FIXME: Also, we need to make sure that path is fully qualified
     * here, by adding any prefixes from jsgf->name to it. */
    /* See if we have parsed it already */
    if (hash_table_lookup(jsgf->imports, path, &val) == 0) {
        E_INFO("Already imported %s\n", path);
        imp = val;
        ckd_free(path);
    }
    else {
        /* If not, parse it. */
        imp = jsgf_parse_file(path, jsgf);
        val = hash_table_enter(jsgf->imports, path, imp);
        if (val != (void *)imp) {
            E_WARN("Multiply imported file: %s\n", path);
        }
    }
    if (imp != NULL) {
        glist_t rules;
        gnode_t *gn;
        int32 nrules;

        /* Look for public rules matching rulename. */
        rules = hash_table_tolist(imp->rules, &nrules);
        for (gn = rules; gn; gn = gnode_next(gn)) {
            hash_entry_t *he = gnode_ptr(gn);
            jsgf_rule_t *rule = hash_entry_val(he);
            int rule_matches;
            char *rule_name = importname2rulename(name);

            if (import_all) {
                /* Match package name (symbol table is shared) */
                rule_matches = !strncmp(rule_name, rule->name, packlen + 1);
            }
            else {
                /* Exact match */
                rule_matches = !strcmp(rule_name, rule->name);
            }
            ckd_free(rule_name);
            if (rule->public && rule_matches) {
                void *val;
                char *newname;

                /* Link this rule into the current namespace. */
                c = strrchr(rule->name, '.');
                assert(c != NULL);
                newname = jsgf_fullname(jsgf, c);

                E_INFO("Imported %s\n", newname);
                val = hash_table_enter(jsgf->rules, newname, rule);
                if (val != (void *)rule) {
                    E_WARN("Multiply defined symbol: %s\n", newname);
                }
                if (!import_all)
                    return rule;
            }
        }
    }

    return NULL;
}

jsgf_t *
jsgf_parse_file(const char *filename, jsgf_t *parent)
{
    yyscan_t yyscanner;
    jsgf_t *jsgf;
    int yyrv;
    FILE *in = NULL;

    yylex_init(&yyscanner);
    if (filename == NULL) {
        yyset_in(stdin, yyscanner);
    }
    else {
        in = fopen(filename, "r");
        if (in == NULL) {
            fprintf(stderr, "Failed to open %s for parsing: %s\n",
                    filename, strerror(errno));
            return NULL;
        }
        yyset_in(in, yyscanner);
    }

    jsgf = jsgf_grammar_new(parent);
    yyrv = yyparse(yyscanner, jsgf);
    if (yyrv != 0) {
        fprintf(stderr, "JSGF parse of %s failed\n",
                filename ? filename : "(stdin)");
        jsgf_grammar_free(jsgf);
        yylex_destroy(yyscanner);
        return NULL;
    }
    /* Record this parser in the import list to avoid loops */
    hash_table_enter(jsgf->imports, ckd_salloc(jsgf->name), jsgf);
    if (in)
        fclose(in);
    yylex_destroy(yyscanner);

    return jsgf;
}
