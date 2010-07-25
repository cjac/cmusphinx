/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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

/*
 * main_conf.c -- Driver for Confidence Annotation based on
 * lattice-base word posteriror word probabilities. Adapted from Rong
 * Zhang's confidence programs.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 

 * $Log$
 * Revision 1.4  2006/03/03  19:45:03  egouvea
 * Clean up the log handling. In logs3.c, removed unnecessary variables
 * (e.g. "f", exactly the same as "F") and functions (e.g. "logs3_10base()").
 * 
 * In confidence.c, replace (logs3_to_log10(r_lscr) * logs3_10base())
 * with r_lscr, since the only difference is that one is a double, the
 * other an int (and as such, they differ on the order of 1e-12).
 * 
 * In future cleanups.... replace the "int" declaration with "int32",
 * used in the rest of the code.
 * 
 * Revision 1.3  2006/03/01 00:11:34  egouvea
 * Added MS Visual C projects for each of the missing executables, fixed
 * compilation problems. Code now compiles in MS .NET, with several
 * warnings such as "signed/unsigned mismatch" and "conversion ...,
 * possible loss of data", which we normally ignore.
 *
 * Revision 1.2  2006/02/24 03:50:11  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Added application conf.
 *
 * Revision 1.1.2.1  2006/01/16 20:23:20  arthchan2003
 * Added a routine to compute confidence scores and do string matching.
 *
 *
 * main_conf.c: Calculate confidence scores from word lattices using
 * posterior word probabilities and backoff.
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * 
 * Author: Rong Zhang <rongz@cs.cmu.edu>
 * Arthur Chan has largely adapt and migrate it to s3.6. 
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <sphinxbase/info.h>
#include <sphinxbase/unlimit.h>

#include "lm.h"
#include "fillpen.h"
#include "dict.h"
#include "logs3.h"
#include "corpus.h"
#include "srch_output.h"
#include "confidence.h"
#include "cmdln_macro.h"

static arg_t defn[] = {
    log_table_command_line_macro(),
    language_model_command_line_macro(),
    control_file_handling_command_line_macro(),
    common_filler_properties_command_line_macro(),
    common_application_properties_command_line_macro(),
    control_lm_file_command_line_macro(),
    dictionary_command_line_macro(),
    {"-mdef",
     REQARG_STRING,
     NULL,
     "Model definition input file: triphone -> senones/tmat tying"},
    {"-inlatdir",
     REQARG_STRING,
     NULL,
     "Input word-lattice directory with per-utt files for restricting words searched"},
    {"-latext",
     ARG_STRING,
     "lat.gz",
     "Word-lattice filename extension (.gz or .Z extension for compression)"},
    {"-inhypseg",
     REQARG_STRING,
     NULL,
     "Recognition result file, with word segmentations and scores"},
    {"-conf_thr",
     ARG_FLOAT64,
     "-2000.0",
     "Confidence threshold"},
    {"-confoutputfmt",
     ARG_STRING,
     "scores",
     "hypseg format, temporarily supersedes option -hypsegfmt, either `s3' or `scores'. "},
    {"-output",
     REQARG_STRING,
     NULL,
     "Output file"},
    {NULL, ARG_INT32, NULL, NULL}
};

static mdef_t *mdef;            /**< Model definition */
static dict_t *dict;            /**< The dictionary */
static fillpen_t *fpen;         /**< The filler penalty structure */
static lmset_t *lmset;          /**< The lmset. Replace lm */
static ptmr_t tm_utt; /**< The model definition */
static cmd_ln_t *config;
static logmath_t *logmath;

static FILE *inmatchsegfp, *outconfmatchsegfp;  /* The segment file */

static void confidence_utt(char *uttid, FILE * _confmatchsegfp);

static void
models_init(void)
{

    mdef = mdef_init(cmd_ln_str_r(config, "-mdef"), 1);

    dict = dict_init(mdef,
                     cmd_ln_str_r(config, "-dict"),
                     cmd_ln_str_r(config, "-fdict"),
		     cmd_ln_boolean_r(config, "-lts_mismatch"),
		     cmd_ln_boolean_r(config, "-mdef_fillers"),
		     FALSE, TRUE);

    lmset = lmset_init(cmd_ln_str_r(config, "-lm"),
                       cmd_ln_str_r(config, "-lmctlfn"),
                       cmd_ln_str_r(config, "-ctl_lm"),
                       cmd_ln_str_r(config, "-lmname"),
                       cmd_ln_str_r(config, "-lmdumpdir"),
                       cmd_ln_float32_r(config, "-lw"),
                       cmd_ln_float32_r(config, "-wip"),
                       cmd_ln_float32_r(config, "-uw"), dict,
                       logmath);

    /* Filler penalties */
    fpen = fillpen_init(dict, cmd_ln_str_r(config, "-fillpen"),
                        cmd_ln_float32_r(config, "-silprob"),
                        cmd_ln_float32_r(config, "-fillprob"),
                        cmd_ln_float32_r(config, "-lw"),
                        cmd_ln_float32_r(config, "-wip"),
                        logmath);

}

static void
models_free(void)
{
    fillpen_free(fpen);
    lmset_free(lmset);
    dict_free(dict);
    mdef_free(mdef);
}

int
dump_line(FILE * fp_output, seg_hyp_line_t * seg_hyp_line, dict_t * dict)
{
    double CONFIDENCE_THRESHOLD;
    conf_srch_hyp_t *w;

    CONFIDENCE_THRESHOLD = cmd_ln_float64_r(config, "-conf_thr");
    fprintf(fp_output, "%s ", seg_hyp_line->seq);
    fprintf(fp_output, "%d ", seg_hyp_line->cscore);
    fprintf(fp_output, "%d   ", (int) seg_hyp_line->lmtype);
    for (w = seg_hyp_line->wordlist; w; w = w->next) {
        fprintf(fp_output, "%s ", w->sh.word);
        if (dict_filler_word(dict, w->sh.id) || w->sh.id == BAD_S3WID)
            fprintf(fp_output, "-UNK- ");
        else {
            /*      fprintf(fp_output, "w->sh.cscr %d, CONFIDENCE_THRESHOLD %f\n", w->sh.cscr, CONFIDENCE_THRESHOLD); */
            if (w->sh.cscr > CONFIDENCE_THRESHOLD)
                fprintf(fp_output, "-OK- ");
            else
                fprintf(fp_output, "-BAD- ");
        }

        fprintf(fp_output, "%d ", w->sh.cscr);
        fprintf(fp_output, "%3.1f   ", w->lmtype);
    }
    fprintf(fp_output, "\n");
    fflush(fp_output);
    return CONFIDENCE_SUCCESS;
}

static void
utt_confidence(void *data, utt_res_t * ur, int32 sf, int32 ef, char *uttid)
{
    if (ur->lmname)
        lmset_set_curlm_wname(lmset, ur->lmname);
    confidence_utt(uttid, inmatchsegfp);
}

/* Find the confidence score and dump the confidence output into the file */
static void
confidence_utt(char *uttid, FILE * _confmatchsegfp)
{
    seg_hyp_line_t s_hypline;
    char line[16384];
    char dagfile[16384];
    const char *fmt;
    const char *latdir;
    const char *latext;
    E_INFO("Processing %s\n", uttid);
    if (fgets(line, sizeof(line), _confmatchsegfp) == NULL)
        E_FATAL("Fail to read a line in the matchsegfp for uttid %s\n",
                uttid);

    /* Read the hypseg */
    if (read_s3hypseg_line(line, &s_hypline, lmset->cur_lm, dict) ==
        HYPSEG_FAILURE)
        E_FATAL("Fail to parse matchseg in utt ID %s\n", uttid);

    E_INFO("Matchseg file name %s\n", s_hypline.seq);
    if (strcmp(uttid, s_hypline.seq))
        E_FATAL("Uttids in control file and matchseg file mismatches\n");

    /* Read the lattice */
    latdir = cmd_ln_str_r(config, "-inlatdir");
    latext = cmd_ln_str_r(config, "-latext");

    if (latdir)
        sprintf(dagfile, "%s/%s.%s", latdir, uttid, latext);
    else
        sprintf(dagfile, "%s.%s", uttid, latext);

    E_INFO("Reading DAG file: %s\n", dagfile);

    if (confidence_word_posterior(dagfile,
                                  &s_hypline,
                                  uttid,
                                  lmset->cur_lm,
                                  dict, fpen) == CONFIDENCE_FAILURE) {
        E_INFO("Fail to compute word posterior probability \n");
    }


#if 0
    if (ca_dag_load_lattice
        (dagfile, &word_lattice, lmset->cur_lm, dict,
         fpen) == CONFIDENCE_FAILURE)
        E_FATAL("Unable to load dag %s for uttid %s\n", dagfile, uttid);

    /* Compute Alpha-beta */
    if (alpha_beta(&word_lattice, lmset->cur_lm, dict) ==
        CONFIDENCE_FAILURE)
        E_FATAL("Unable to compute alpha beta score for uttid %s\n",
                uttid);

    /* Compute Posterior WORD probability */
    if (pwp(&s_hypline, &word_lattice) == CONFIDENCE_FAILURE)
        E_FATAL("Unable to compute pwp for uttid %s\n", uttid);
#endif

    /* Compute LM type */
    if (compute_lmtype(&s_hypline, lmset->cur_lm, dict) ==
        CONFIDENCE_FAILURE)
        E_FATAL("Fail to compute lm type\n");

    /* combined LM type */
    if (compute_combined_lmtype(&s_hypline) == CONFIDENCE_FAILURE)
        E_FATAL("Fail to compute lm type\n");

    /* Dump pwp line */
    fmt = cmd_ln_str_r(config, "-confoutputfmt");
    if (!strcmp(fmt, "scores")) {
        dump_line(stdout, &s_hypline, dict);
        dump_line(outconfmatchsegfp, &s_hypline, dict);
    }
    else {
        glist_t hyp;
        srch_hyp_t *s;
        conf_srch_hyp_t *h;

        hyp = NULL;
        for (h = (conf_srch_hyp_t *) s_hypline.wordlist; h; h = h->next) {
            s = &(h->sh);
            hyp = glist_add_ptr(hyp, (void *) s);
        }
        matchseg_write(stdout, hyp, uttid, NULL,
                       lmset->cur_lm, dict, 0, NULL, 0);
        matchseg_write(outconfmatchsegfp, hyp, uttid, NULL,
                       lmset->cur_lm, dict, 0, NULL, 0);
    }

#if 0
    /* Delete lattice, delete hypsegline */
    if (ca_dag_free_lattice(&word_lattice) == CONFIDENCE_FAILURE) {
        E_WARN("Fail to free lattice.\n");
        return CONFIDENCE_FAILURE;
    }
#endif

    if (free_seg_hyp_line(&s_hypline) != HYPSEG_SUCCESS)
        E_FATAL("Fail to free the segment hypothesis line structure. \n");
}


int
main(int argc, char *argv[])
{
    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", defn);

    unlimit();

    config = cmd_ln_get();

    logmath = logs3_init(cmd_ln_float64_r(config, "-logbase"), 1,
                         cmd_ln_int32_r(config, "-log3table"));

    E_INFO("Value of base %f \n", cmd_ln_float32_r(config, "-logbase"));
    models_init();
    ptmr_init(&tm_utt);

    if ((inmatchsegfp = fopen(cmd_ln_str_r(config, "-inhypseg"), "r")) == NULL)
        E_ERROR("fopen(%s,r) failed\n", cmd_ln_str_r(config, "-inhypseg"));


    if ((outconfmatchsegfp = fopen(cmd_ln_str_r(config, "-output"), "w")) == NULL)
        E_ERROR("fopen(%s,w) failed\n", cmd_ln_str_r(config, "-output"));

    if (cmd_ln_str_r(config, "-ctl")) {
        ctl_process(cmd_ln_str_r(config, "-ctl"),
                    cmd_ln_str_r(config, "-ctl_lm"),
                    NULL,
                    cmd_ln_int32_r(config, "-ctloffset"),
                    cmd_ln_int32_r(config, "-ctlcount"), utt_confidence, NULL);
    }
    else {
        E_FATAL("-ctl is not specified\n");
    }

#if (! WIN32)
    system("ps auxwww | grep s3dag");
#endif

    fclose(outconfmatchsegfp);
    fclose(inmatchsegfp);

    models_free();

    logmath_free(logmath);

    cmd_ln_free_r(config);

    return 0;

}
