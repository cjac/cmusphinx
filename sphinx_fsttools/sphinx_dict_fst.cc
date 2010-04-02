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
 * @file sphinx_dict_fst.cc
 *
 * Build finite-state transducers from Sphinx dictionaries.
 */

#include <fst/fstlib.h>

namespace sb {
#include <cmd_ln.h>
#include <hash_table.h>
#include <err.h>
#include "dict.h"
#include "mdef.h"
};

#include "fstprinter.h"

using namespace fst;
using namespace sb;
using sb::int32; /* Override conflicting typedefs */
typedef StdArc::StateId StateId;
typedef StdArc::Label Label;

static const arg_t args[] = {
    { "-mdef",
      ARG_STRING,
      NULL,
      "Model definition input file" },
    { "-dict",
      REQARG_STRING,
      NULL,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-dictcase",
      ARG_BOOLEAN,
      "no",
      "Dictionary is case sensitive (NOTE: case insensitivity applies to ASCII characters only)" },
    { "-fdict",
      ARG_STRING,
      NULL,
      "Noise word pronunciation dictionary input file" },
    { "-usealt",
      ARG_BOOLEAN,
      "no",
      "Keep alternate pronunciation markers on word strings" },
    { "-binfst",
      ARG_STRING,
      NULL,
      "Binary FST output file" },
    { "-txtfst",
      ARG_STRING,
      NULL,
      "Text FST output file" },
    { "-isym",
      ARG_STRING,
      NULL,
      "Input symbol table output file" },
    { "-osym",
      ARG_STRING,
      NULL,
      "Output symbol table output file" },
    { "-closure",
      ARG_BOOLEAN,
      "yes",
      "Output Kleene closure of dictionary" },
    { "-determinize",
      ARG_BOOLEAN,
      "no",
      "Determinize output FST" },
    { "-minimize",
      ARG_BOOLEAN,
      "yes",
      "Minimize output FST (only if -determinize yes)" },
    { NULL, 0, NULL, NULL }
};

static StdVectorFst *
dict_to_fst(dict_t *dict, bool usealt)
{
    /* Create a mutable FST, and symbol tables. */
    StdVectorFst *model = new StdVectorFst;
    SymbolTable *isym = new SymbolTable("phones");
    SymbolTable *osym = new SymbolTable("words");
    StateId start = model->AddState();
    StateId end = model->AddState();
    model->SetStart(start);
    model->SetFinal(end, 0);

    /* Populate symbol tables with phones and words from the dictionary. */
    isym->AddSymbol("&epsilon;", 0);
    for (int32 i = 0; i < dict_n_ciphone(dict); ++i) {
        isym->AddSymbol(dict_ciphone_str(dict, i), i+1);
    }
    Label first_wbound = isym->AddSymbol("#0");
    Label last_wbound = first_wbound;
    osym->AddSymbol("&epsilon;", 0);
    for (int32 i = 0; i < dict_n_words(dict); ++i) {
        if (usealt) {
            osym->AddSymbol(dict_word_str(dict, i), i+1);
        }
        else {
            if (osym->Find(dict_base_str(dict, i)) == -1) {
                osym->AddSymbol(dict_base_str(dict, i),
                                dict_base_wid(dict, i) + 1);
            }
        }
    }

    /* Track boundary symbols for each unique pronunciation. */
    hash_table_t *pbsym = hash_table_new(dict_n_words(dict), HASH_CASE_YES);

    /* For each word in the dictionary: */
    for (int32 i = 0; i < dict_n_words(dict); ++i) {
        /* Look for homophones. */
        int32 wbound;
        if (hash_table_lookup_bkey_int32(pbsym,
                                         (char const *)dict->dict_list[i]->ci_phone_ids,
                                         dict_pronlen(dict, i)
                                         * sizeof(*dict->dict_list[i]->ci_phone_ids),
                                         &wbound) != 0) {
            wbound = first_wbound;
        }
        else {
            ++wbound;
            if (wbound > last_wbound) {
                char wbsym[16];
                sprintf(wbsym, "#%d", wbound - first_wbound);
                isym->AddSymbol(wbsym, wbound);
                last_wbound = wbound;
            }
            printf("Word %s has duplicate pronunciation, using #%d\n",
                   dict_word_str(dict, i), wbound - first_wbound);
        }
        hash_table_replace_bkey_int32(pbsym,
                                      (char const *)dict->dict_list[i]->ci_phone_ids,
                                      dict_pronlen(dict, i)
                                      * sizeof(*dict->dict_list[i]->ci_phone_ids),
                                      wbound);

        /* Build arcs for this word. */
        Label wid;
        if (usealt) {
            wid = osym->Find(dict_word_str(dict, i));
        }
        else {
            wid = osym->Find(dict_base_str(dict, i));
        }
        StateId n = start;
        for (int32 j = 0; j < dict_pronlen(dict, i); ++j) {
            StateId m = model->AddState();
            Label ci = dict_ciphone(dict, i, j) + 1;
            if (j == 0)
                model->AddArc(n, StdArc(ci, wid, 0, m));
            else
                model->AddArc(n, StdArc(ci, 0, 0, m));
            n = m;
        }
        model->AddArc(n, StdArc(wbound, 0, 0, end));
    }
    hash_table_free(pbsym);
    model->SetInputSymbols(isym);
    model->SetOutputSymbols(osym);
    return model;
}

int
main(int argc, char *argv[])
{
    /* Load Sphinx model files. */
    dict_t *dict;
    mdef_t *mdef = NULL;
    cmd_ln_t *config;

    if ((config = cmd_ln_parse_r(NULL, args, argc, argv, TRUE)) == NULL) {
        return 1;
    }
    if (cmd_ln_str_r(config, "-mdef")) {
        if ((mdef = mdef_init(cmd_ln_str_r(config, "-mdef"), TRUE)) == NULL) {
            E_ERROR("Failed to load model definition file from %s\n",
                    cmd_ln_str_r(config, "-mdef"));
            return 1;
        }
    }
    if ((dict = dict_init(config, mdef)) == NULL) {
        E_ERROR("Failed to load dictionary files\n");
        return 1;
    }

    /* Create FST from dict. */
    StdVectorFst *model = dict_to_fst(dict, cmd_ln_boolean_r(config, "-usealt"));

    /* Determinize, minimize, and epsilon-remove the model. */
    if (cmd_ln_boolean_r(config, "-determinize")) {
        StdVectorFst *detmodel = new StdVectorFst();
        Determinize(*model, detmodel);
        if (cmd_ln_boolean_r(config, "-minimize")) {
            Minimize(detmodel);
        }
        delete model;
        model = detmodel;
    }

    if (cmd_ln_boolean_r(config, "-closure"))
        Closure(model, CLOSURE_PLUS);

    /* Also sort it so it can be composed with a grammar. */
    ArcSort(model, OLabelCompare<StdArc>());

    /* Write the model in the requested format. */
    char const *outfile;
    if ((outfile = cmd_ln_str_r(config, "-binfst")) != NULL) {
        model->Write(outfile);
    }
    if ((outfile = cmd_ln_str_r(config, "-txtfst")) != NULL) {
        FstPrinter<StdArc> printer(*model,
                                   model->InputSymbols(),
                                   model->OutputSymbols(),
                                   NULL, false);
        ostream *ostrm = new ofstream(outfile);
        printer.Print(ostrm, outfile);
    }
    if ((outfile = cmd_ln_str_r(config, "-isym")) != NULL) {
        model->InputSymbols()->WriteText(outfile);
    }
    if ((outfile = cmd_ln_str_r(config, "-osym")) != NULL) {
        model->OutputSymbols()->WriteText(outfile);
    }

    dict_free(dict);
    mdef_free(mdef);
    cmd_ln_free_r(config);

    return 0;
}
