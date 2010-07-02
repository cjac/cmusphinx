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
/*
 * decode.c --  
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to allow runtime choice between 3-state and 5-state HMM
 * 		topologies (instead of compile-time selection).
 * 
 * 13-Aug-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxwpf.
 * 
 * 10-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */

/** \file main_decode.c
 * \brief main function for application decode
 */
#include "info.h"
#include "unlimit.h"
#include "utt.h"
#include "kb.h"
#include "corpus.h"
#include "cmdln_macro.h"

static arg_t arg[] = {
    log_table_command_line_macro(),
    waveform_to_cepstral_command_line_macro(),
    cepstral_to_feature_command_line_macro(),

    acoustic_model_command_line_macro(),
    speaker_adaptation_command_line_macro(),
    language_model_command_line_macro(),
    dictionary_command_line_macro(),
    phoneme_lookahead_command_line_macro(),
    histogram_pruning_command_line_macro(),
    fast_GMM_computation_command_line_macro(),
    common_filler_properties_command_line_macro(),
    common_s3x_beam_properties_command_line_macro(),
    common_application_properties_command_line_macro(),
    control_file_handling_command_line_macro(),
    hypothesis_file_handling_command_line_macro(),
    score_handling_command_line_macro(),
    output_lattice_handling_command_line_macro(),
    dag_handling_command_line_macro(),
    second_stage_dag_handling_command_line_macro(),
    input_lattice_handling_command_line_macro(),
    flat_fwd_debugging_command_line_macro(),
    history_table_command_line_macro(),

    cepstral_input_handling_command_line_macro(),
    decode_specific_command_line_macro(),
    search_specific_command_line_macro(),
    search_modeTST_specific_command_line_macro(),
    search_modeWST_specific_command_line_macro(),
    control_lm_mllr_file_command_line_macro(),
    finite_state_grammar_command_line_macro(),
    phone_insertion_penalty_command_line_macro(),

    /* the following functions are used for MMIE training
       lqin 2010-03 */
    unigram_only_lm_command_line_macro(),
    bigram_only_lm_command_line_macro(),
    /* end */

    /* Things are yet to refactored */
#if 0
    /* Commented out; not supported */
    {"-compsep",
     ARG_STRING,
     /* Default: No compound word (NULL separator char) */
     "",
     "Separator character between components of a compound word (NULL if "
     "none)"},
#endif

    {"-phsegdir",
     ARG_STRING,
     NULL,
     "(Allphone mode only) Output directory for phone segmentation files"},

    {"-bestscoredir",
     ARG_STRING,
     NULL,
     "(Mode 3) Directory for writing best score/frame (used to set beamwidth; "
     "one file/utterance)"},

    /** ARCHAN 20050717: The only argument which I didn't refactor,
        reason is it makes sense to make every s3.0 family of tool to
        accept -utt.  DHD 20070525: I have no idea what that means. */

    {"-utt",
     ARG_STRING,
     NULL,
     "Utterance file to be processed (-ctlcount argument times)"},

    {NULL, ARG_INT32, NULL, NULL}

};

int32
main(int32 argc, char *argv[])
{
    kb_t kb;
    stat_t *st;
    cmd_ln_t *config;

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", arg);

    unlimit();

    config = cmd_ln_get();
    kb_init(&kb, config);
    st = kb.stat;
    fprintf(stdout, "\n");

    if (cmd_ln_str_r(config, "-ctl")) {
        /* When -ctlfile is speicified, corpus.c will look at -ctl_lm and
           -ctl_mllr to get the corresponding LM and MLLR for the utterance */
        st->tm = ctl_process(cmd_ln_str_r(config, "-ctl"),
                             cmd_ln_str_r(config, "-ctl_lm"),
                             cmd_ln_str_r(config, "-ctl_mllr"),
                             cmd_ln_int32_r(config, "-ctloffset"),
                             cmd_ln_int32_r(config, "-ctlcount"), utt_decode, &kb);
    }
    else if (cmd_ln_str_r(config, "-utt")) {
        /* When -utt is specified, corpus.c will wait for the utterance to
           change */
        st->tm = ctl_process_utt(cmd_ln_str_r(config, "-utt"),
                                 cmd_ln_int32_r(config, "-ctlcount"),
                                 utt_decode, &kb);

    }
    else {
        /* Is error checking good enough?" */
        E_FATAL("Both -utt and -ctl are not specified.\n");

    }

    if (kb.matchsegfp)
        fclose(kb.matchsegfp);
    if (kb.matchfp)
        fclose(kb.matchfp);

    stat_report_corpus(kb.stat);

    kb_free(&kb);

#if (! WIN32)
#if defined(_SUN4)
    system("ps -el | grep sphinx3_decode");
#else
    system("ps aguxwww | grep sphinx3_decode");
#endif
#endif

    cmd_ln_free_r(config);
    exit(0);
}
