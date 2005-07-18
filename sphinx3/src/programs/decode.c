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

/** \file decode.c
 * \brief main function for application decode
 */
#include "utt.h"
#include "kb.h"
#include "logs3.h"		/* RAH, added to resolve log3_free */
#include "corpus.h"
#include "cmdln_macro.h"


#if 0

    { "-lambda",
      ARG_STRING,
      NULL,
      "Interpolation weights (CD/CI senone) parameters input file" },

arg_t s33_decode_arg[] ={
#if 0
    /* Commented out; not supported */
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
#endif
    { "-Nlextree",
      ARG_INT32,
      "3",
      "No. of lextrees to be instantiated; entries into them staggered in time" },
    { "-epl",
      ARG_INT32,
      "3",
      "Entries Per Lextree; #successive entries into one lextree before lextree-entries shifted to the next" },
    { "-bptbldir",
      ARG_STRING,
      NULL,
      "Directory in which to dump word Viterbi back pointer table (for debugging)" },
    { "-hmmdump",
      ARG_INT32,
      "0",
      "Whether to dump active HMM details to stderr (for debugging)" },
    { "-bghist",
      ARG_INT32,
      "0",
      "Bigram-mode: If TRUE only one BP entry/frame; else one per LM state" },
    { "-ptranskip",
      ARG_INT32,
      "0",
      "Use wbeam for phone transitions every so many frames (if >= 1)" },
    { "-treeugprob",
      ARG_INT32,
      "1",
      "If TRUE (non-0), Use unigram probs in lextree" },
};

arg_t s36_decode_arg[] ={
    { "-Nstalextree",
      ARG_INT32,
      "25",
      "No. of lextrees to be instantiated statically; " },
};

arg_t s3lp_arg[] = {
    { "-maxhyplen",
      ARG_INT32,
      "1000",
      "Maximum number of words in a partial hypothesis (for block decoding)" },
    { "-phypdump",
      ARG_INT32,
      "1",
      "dump parital hypothesis on the screen"},
};

arg_t output_arg[] = {
    { "-hyp",
      ARG_STRING,
      NULL,
      "Recognition result file, with only words" },
    { "-hypseg",
      ARG_STRING,
      NULL,
      "Recognition result file, with word segmentations and scores" },

}

arg_t search_arg[] = {
    { "-op_mode",
      ARG_INT32,
      "4",
      "Operation Mode. Mode 4: TST search, Mode 5: WST search, Mode 1369: Debug "},
};

arg_t ctl_arg[]=
{
    { "-rawdir",
      ARG_STRING,
      NULL,
      "Input raw files directory (prefixed to filespecs in control file)" },
    { "-rawext",
      ARG_STRING,
      ".raw",
      "Input raw files extension (prefixed to filespecs in control file)"},
    { "-ctl_lm",
      ARG_STRING,
      NULL,
      "Control file that list the corresponding LM for an utterance" },
    { "-mllrctl",
      ARG_STRING,
      NULL,
      "Control file that list the corresponding MLLR matrix for an utterance"},
    { "-utt",
      ARG_STRING,
      NULL,
      "Utterance file to be processed (-ctlcount argument times)" },
}


#endif
 
static arg_t arg[] = {
  log_table_command_line_macro()
  cepstral_to_feature_command_line_macro()
  acoustic_model_command_line_macro()
  speaker_adaptation_command_line_macro()
  language_model_command_line_macro()
  dictionary_command_line_macro()
  phoneme_lookahead_command_line_macro()
  histogram_pruning_command_line_macro()
  fast_GMM_computation_command_line_macro()
  common_filler_properties_command_line_macro()
  common_s3x_beam_properties_command_line_macro()
  common_application_properties_command_line_macro()
  control_file_handling_command_line_macro()
  hypothesis_file_handling_command_line_macro() 
  output_lattice_handling_command_line_macro()

  cepstral_input_handling_command_line_macro()
  decode_specific_command_line_macro()
  search_specific_command_line_macro()
  search_modeTST_specific_command_line_macro()
  search_modeWST_specific_command_line_macro()
  partial_hypothesis_command_line_macro()

  control_lm_mllr_file_command_line_macro()

#if 0
    /* Commented out; not supported */
     { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
#endif

  /** ARCHAN 20050717: The only argument which I didn't refactor,
      reason is it makes sense to make every s3.0 family of tool to
      accept -utt */

    { "-utt",
      ARG_STRING,
      NULL,
      "Utterance file to be processed (-ctlcount argument times)" },

    { NULL, ARG_INT32, NULL, NULL }
    
};

int32 main (int32 argc, char *argv[])
{
    kb_t kb;
    stat_t* st;


    
    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc,argv,"default.arg",arg);

    unlimit ();
    
    kb_init (&kb);
    st = kb.stat;
    fprintf (stdout, "\n");

    if (cmd_ln_str ("-ctl")) {
      if(cmd_ln_str("-ctl_lm")&&cmd_ln_str("-lmctlfn")) {
	st->tm = ctl_process_dyn_lm (cmd_ln_str("-ctl"),
				 cmd_ln_str("-ctl_lm"),
				 cmd_ln_str("-ctl_mllr"),
			  cmd_ln_int32("-ctloffset"),
			  cmd_ln_int32("-ctlcount"),
			  utt_decode, &kb);
      }else{
	if(cmd_ln_str("-ctl_lm")){
	  E_WARN("Ignoring argument from option -ctl_lm\n");
	}
	st->tm = ctl_process (cmd_ln_str("-ctl"),
			  cmd_ln_str("-ctl_mllr"),
			  cmd_ln_int32("-ctloffset"),
			  cmd_ln_int32("-ctlcount"),
			  utt_decode, &kb);
      }
    } else if (cmd_ln_str ("-utt")) {
      /* Defunct at this moment, LM and MLLR is not correctly loaded in in this mode. */
      E_FATAL("-utt is disabled  at this moment, LM and MLLR is not correctly loaded in in this mode.");
	st->tm = ctl_process_utt (cmd_ln_str("-utt"), 
			      cmd_ln_int32("-ctlcount"), 
			      utt_decode, &kb);

    } else {
      /* Is error checking good enough?" */
      E_FATAL("Both -utt and -ctl are not specified.\n");
      
    }
    
    if (kb.matchsegfp)
	fclose (kb.matchsegfp);
    if (kb.matchfp) 
        fclose (kb.matchfp);
    
    stat_report_corpus(kb.stat);

    kb_free(&kb);
  cmd_ln_appl_exit();
  exit(0);
}
