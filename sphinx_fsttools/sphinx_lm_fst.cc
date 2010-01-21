/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2008 Carnegie Mellon University.  All rights 
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
 * @file sphinx_lm_fst.cc
 *
 * Build finite-state transducers from Sphinx language models.
 */

#include <fst/fstlib.h>

namespace sb {
#include <ngram_model.h>
#include <ckd_alloc.h>
#include <hash_table.h>
#include <assert.h>
#include <stdarg.h>
#include <err.h>
};

#include "fstprinter.h"

using namespace fst;
using namespace sb;
using sb::int32; /* Override conflicting typedefs */
typedef VectorFst<LogArc> LogVectorFst;
typedef StdArc::StateId StateId;
typedef StdArc::Label Label;

static const arg_t args[] = {
    { "-lm",
      REQARG_STRING,
      NULL,
      "Language model input file" },
    { "-binfst",
      ARG_STRING,
      NULL,
      "Binary FST output file" },
    { "-txtfst",
      ARG_STRING,
      NULL,
      "Text FST output file" },
    { "-exact",
      ARG_BOOLEAN,
      "no",
      "Use exact representation (Allauzen, 2003) (FIXME: Not yet implemented)" },
    { "-sym",
      ARG_STRING,
      NULL,
      "Use this symbol table file for word IDs" },
    { "-symout",
      ARG_STRING,
      NULL,
      "Write symbol table file for word IDs to this file" },
    { "-ssymout",
      ARG_STRING,
      NULL,
      "Write symbol table file for states to this file" },
    { "-closure",
      ARG_BOOLEAN,
      "yes",
      "Output Kleene closure of dictionary" },
    { "-maxn",
      ARG_INT32,
      "0",
      "Limit model to N-grams of this order (0 to use all)" },
    { "-verbose",
      ARG_BOOLEAN,
      "no",
      "Print verbose debug info" },
    { NULL, 0, NULL, NULL }
};

static int verbose = 0;
static void
dprintf(char const *format, ...)
{
    va_list args;

    va_start(args, format);
    if (verbose)
        vprintf(format, args);
    va_end(args);
}

static void
add_mgram_states(StdVectorFst *model, int m,
                 ngram_model_t *lm,
                 logmath_t *lmath,
                 hash_table_t *sidtab)
{
    ngram_iter_t *itor = ngram_model_mgrams(lm, m);
    

    /* Add states for each M-gram at this order M. */
    for (; itor; itor = ngram_iter_next(itor)) {
        /* Retrieve M-gram information. */
        int32 prob, bowt;
        int32 const *wids = ngram_iter_get(itor, &prob, &bowt);

        /* If the final word is not in the symbol table then skip it. */
        Label wid = model->InputSymbols()->Find(ngram_word(lm, wids[m]));
        if (wid == -1) {
            dprintf("Skipping unknown word %s\n",
                    ngram_word(lm, wids[m]));
            continue;
        }

        /* Any M-gram starting with </s> is meaningless, so skip it. */
        /* Except if M == 1! */
        if (m != 0 && 0 == strcmp(ngram_word(lm, wids[0]), "</s>"))
            continue;

        /* Find the source state for this arc. */
        StateId src;
        if (m == 0) {
            /* Use the epsilon state as the source. */
            src = 0;
        }
        else if (hash_table_lookup_bkey_int32(sidtab, (char const *)wids,
                                                  m * sizeof(*wids),
                                                  &src) != 0) {
            /* We currently expect w(1,N-1) to exist, and in fact it
             * always should, by the definition of a backoff model,
             * unless one of the words is not in the symbol table (in
             * which case we want to skip this M-gram anyway). */
            continue;
        }

        StateId dest;
        bool final, newstate;
        /* Only one final state is necessary, so don't create any new ones. */
        if (0 == strcmp(ngram_word(lm, wids[m]), "</s>")) {
            final = true;
            newstate = false;
            /* Look for an existing unigram </s> state. */
            if (hash_table_lookup_bkey_int32(sidtab,
                                                 (char const *)(wids + m),
                                                 sizeof(*wids),
                                                 &dest) != 0) {
                dest = model->AddState();
                dprintf("Final state %d\n", dest);
                model->SetFinal(dest, 0);
                newstate = true;
            }
        }
        else {
            final = false;
            newstate = true;
            dest = model->AddState();
        }

        /* <s> is a non-event (FIXME: should be configurable) */
        if (0 == strcmp(ngram_word(lm, wids[m]), "<s>")) {
            /* Also its target should be the initial state. */
            if (m == 0) {
                dprintf("Initial state %d\n", dest);
                model->SetStart(dest);
            }
        }
        else {
            model->AddArc(src, StdArc(wid, wid,
                                      -logmath_log_to_ln(lmath, prob),
                                        dest));
            dprintf("Adding %d-gram arc %d => %d %s/%.4f\n", m+1, src, dest,
                    ngram_word(lm, wids[m]), logmath_log_to_log10(lmath, prob));
        }

        /* Track state IDs. */
        if (newstate) {
            /* Ugh, we have to copy the keys... */
            int32 *newwids;
            int len;
            if (final) {
                newwids = (int32 *)ckd_malloc(sizeof(*wids));
                newwids[0] = wids[m];
                len = 1;
            }
            else {
                len = m+1;
                newwids = (int32 *)ckd_malloc(len * sizeof(*wids));
                memcpy(newwids, wids, len * sizeof(*wids));
            }
            hash_table_enter_bkey_int32(sidtab, (char *)newwids,
                                            len * sizeof(*newwids), dest);
            dprintf("Entered state ID mapping %d <=", dest);
            for (int j = 0; j < len; ++j) {
                dprintf(" %s", ngram_word(lm, wids[j]));
            }
            dprintf("\n");
        }

        /* Create a backoff arc to the suffix M-1-gram, if it exists. */
        if (bowt && !final) {
            int32 bo = 0; /* epsilon state */
            if (m > 0
                && hash_table_lookup_bkey_int32(sidtab, (char const *)(wids + 1),
                                                m * sizeof(*wids),
                                                &bo) != 0) {
                continue;
            }
            /* Add a backoff arc back to the M-1-gram state. */
            model->AddArc(dest, StdArc(0, 0,
                                       -logmath_log_to_ln(lmath, bowt),
                                       bo));
            dprintf("Adding backoff arc %d => %d %.4f\n", dest, bo,
                    logmath_log_to_log10(lmath, bowt));
        }
    }
}

static void
add_ngram_arcs(StdVectorFst *model, int n, hash_table_t *sidtab,
               ngram_model_t *lm, logmath_t *lmath)
{
    ngram_iter_t *itor = ngram_model_mgrams(lm, n-1);

    /* Add arcs for all N-grams to N-1-gram states. */
    for (; itor; itor = ngram_iter_next(itor)) {
        /* Retrieve N-gram information. */
        int32 prob, bowt;
        int32 const *wids = ngram_iter_get(itor, &prob, &bowt);

        /* Get word symbol ID for arc. */
        Label wid = model->InputSymbols()->Find(ngram_word(lm, wids[n-1]));
        if (wid == -1)
            continue;

        /* <s> is a non-event (FIXME: should be configurable) */
        if (0 == strcmp(ngram_word(lm, wids[n-1]), "<s>"))
            continue;

        /* Find source and destination states.  Source is w(1,N-1) and
         * destination is w(2,N). */
        int32 src, dest;
        if (hash_table_lookup_bkey_int32(sidtab, (char const *)wids,
                                             (n-1) * sizeof(*wids),
                                             &src) != 0) {
            /* We currently expect w(1,N-1) to exist, and in fact it
             * always should, by the definition of a backoff model,
             * unless one of the words is not in the symbol table. */
            int j;
            for (j = 0; j < n; ++j) {
                if (model->InputSymbols()->Find(ngram_word(lm, wids[j])) == -1)
                    break;
            }
            if (j == n) {
                dprintf("Failed to find prefix %d-gram for", n-1);
                for (int j = 0; j < n; ++j) {
                    dprintf(" %s", ngram_word(lm, wids[j]));
                }
                dprintf("\n");
            }
            continue;
        }
        /* It's entirely possible that w(2,N), however, might not
         * exist, particularly in a pruned language model.  In this
         * case we will back off to shorter final N-grams without
         * applying any backoff weight (this is consistent with what
         * the ARPA file header says)
         */
        int32 endn = n - 1;
        while (endn > 0
               && hash_table_lookup_bkey_int32(sidtab, (char const *)(wids + n - endn),
                                                   endn * sizeof(*wids),
                                                   &dest) != 0) {
            dprintf("Failed to find state ID for final %d-gram", endn);
            for (int j = n - endn; j < n; ++j) {
                dprintf(" %s", ngram_word(lm, wids[j]));
            }
            dprintf("\n");
            --endn;
        }
        if (endn == 0) {
            /* FIXME: This is probably a serious error. */
            dprintf("Failed to find any suffix N-grams for");
            for (int j = 0; j < n; ++j) {
                dprintf(" %s", ngram_word(lm, wids[j]));
            }
            dprintf("\n");
            continue;
        }

        /* Add the arc. */
        model->AddArc(src, StdArc(wid, wid,
                                  -logmath_log_to_ln(lmath, prob),
                                  dest));
        dprintf("Adding %d-gram arc %d => %d %s/%.4f\n", n, src, dest,
                ngram_word(lm, wids[n-1]), logmath_log_to_log10(lmath, prob));
    }
}

int
main(int argc, char *argv[])
{
    cmd_ln_t *config;

    ngram_model_t *lm;
    logmath_t *lmath;
    int32 const *counts;
    int32 n, nn;

    if ((config = cmd_ln_parse_r(NULL, args, argc, argv, TRUE)) == NULL) {
        return 1;
    }

    lmath = logmath_init(1.0001, 0, 0);
    lm = ngram_model_read(NULL, cmd_ln_str_r(config, "-lm"),
                              NGRAM_AUTO, lmath);
    n = ngram_model_get_size(lm);
    if ((nn = cmd_ln_int32_r(config, "-maxn")) != 0)
        n = nn;
    counts = ngram_model_get_counts(lm);
    verbose = cmd_ln_boolean_r(config, "-verbose");

    /* FIXME: Should this be in the Log semiring instead? */
    StdVectorFst model;

    /* If there is a symbol table file, use it.  Otherwise, construct
     * it from the unigram strings. */
    SymbolTable *sym;
    char const *filename;
    if ((filename = cmd_ln_str_r(config, "-sym")) != NULL) {
        sym = SymbolTable::ReadText(filename);
        dprintf("Read symbols from %s, next index %lld\n",
               filename, sym->AvailableKey());
    }
    else {
        sym = new SymbolTable("words");
        sym->AddSymbol("&epsilon;", 0);
        for (int32 i = 0; i < counts[0]; ++i) {
            sym->AddSymbol(ngram_word(lm, i));
        }
        dprintf("Constructed symbols from LM, next index %lld\n",
               sym->AvailableKey());
    }

    model.SetInputSymbols(sym);
    model.SetOutputSymbols(sym);

    /* The algorithm goes like this:
     *
     * Create an epsilon state
     * For M in 1 to N-1:
     *  For each M-gram w(1,M):
     *   Create a state q(1,M)
     *   Create an arc from state q(1,M-1) to q(1,M) with weight P(w(1,M))
     *   Create an arc from state q(1,M) to q(2,M) with weight bowt(w(1,M-1))
     * For each N-gram w(1,N):
     *   Create an arc from state q(1,N-1) to q(2,N) with weight P(w(1,N))
     */
    StateId eps = model.AddState();
    assert(eps == 0);

    /* Hash all M-grams to State IDs. */
    int nngrams = 0;
    for (int i = 0; i < n; ++i) nngrams += counts[i];
    dprintf("Allocating hash table for %d N-grams\n", nngrams);
    hash_table_t *sidtab = hash_table_new(nngrams, HASH_CASE_YES);

    /* Do M-grams from 1 to N-1, tracking state IDs. */
    for (int m = 0; m < n-1; ++m) {
        add_mgram_states(&model, m, lm, lmath, sidtab);
    }

    /* Now do N-grams. */
    add_ngram_arcs(&model, n, sidtab, lm, lmath);

    /* Find backoff paths which overestimate probabilities and remove
     * them by creating explicit backoff states (Allauzen, 2003) */

    /* And now ... elaborately free the N-gram hash table, writing a
     * state symbol table in the process. */
    char const *ssym = cmd_ln_str_r(config, "-ssymout");
    FILE *ssymfh = NULL;
    if (ssym) ssymfh = fopen(ssym, "w");
    if (ssymfh)
        fprintf(ssymfh, "&epsilon;\t%d\n", eps);
    for (hash_iter_t *itor = hash_table_iter(sidtab);
         itor; itor = hash_table_iter_next(itor)) {
        if (ssymfh) {
            int32 *wids = (int32 *)hash_entry_key(itor->ent);
            fprintf(ssymfh, "%s", ngram_word(lm, wids[0]));
            for (int j = 1; j < (int)(hash_entry_len(itor->ent) / 4); ++j) {
                fprintf(ssymfh, "_%s", ngram_word(lm, wids[j]));
            }
            fprintf(ssymfh, "\t%d\n", (int32)(long)hash_entry_val(itor->ent));
        }
        ckd_free((void *)itor->ent->key);
    }
    hash_table_free(sidtab);

    /* The model should already be deterministic (aside from
     * containing epsilon arcs).  We should connect it though to
     * remove impossible word sequences.  However, this means the
     * state symbol table would be meaningless.  So don't do that. */

    if (cmd_ln_boolean_r(config, "-closure"))
        Closure(&model, CLOSURE_PLUS);

    /* Also sort it so it can be composed with a dictionary. */
    ArcSort(&model, ILabelCompare<StdArc>());

    /* Write the model in the requested format. */
    char const *outfile;
    if ((outfile = cmd_ln_str_r(config, "-binfst")) != NULL) {
        model.Write(outfile);
    }
    if ((outfile = cmd_ln_str_r(config, "-txtfst")) != NULL) {
        FstPrinter<StdArc> printer(model,
                                   model.InputSymbols(),
                                   model.OutputSymbols(),
                                   NULL, true);
        ostream *ostrm = new ofstream(outfile);
        printer.Print(ostrm, outfile);
    }
    if ((outfile = cmd_ln_str_r(config, "-symout")) != NULL) {
        model.InputSymbols()->WriteText(outfile);
    }

    ngram_model_free(lm);
    cmd_ln_free_r(config);
    return 0;
}
