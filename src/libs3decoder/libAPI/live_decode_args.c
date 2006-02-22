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
/***********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.14  2006/02/22  21:54:50  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH: changed the command line to macros defined cmdln_macro.h
 * 
 * Revision 1.13.4.7  2006/01/16 18:36:03  arthchan2003
 * Add unscaling feature in both decode/livepretend.
 *
 * Revision 1.13.4.6  2005/09/18 01:11:17  arthchan2003
 * Add command line for flat-lexicon decoding in livepretend.
 *
 * Revision 1.13.4.5  2005/07/20 19:42:30  arthchan2003
 * Completed live decode layer of lm add. Added command-line arguments for fsg and phone insertion.
 *
 * Revision 1.13.4.4  2005/07/18 19:02:13  arthchan2003
 * Tied almost every command line arguments with decode, wave2feat and ep. (except -rawext and -machine_endian) using macro in cmdln_macro.h
 *
 * Revision 1.13.4.3  2005/07/13 02:02:53  arthchan2003
 * Added -dither and -seed in the option.  Dithering is also support in livepretend. The behavior will be conformed s3.x's wave2feat, start to re-incorproate lm_read. Not completed yet.
 *
 * Revision 1.13.4.2  2005/07/07 02:31:54  arthchan2003
 * Remove -lminsearch, it proves to be useless and FSG implementation.
 *
 * Revision 1.13.4.1  2005/07/04 07:14:15  arthchan2003
 * Added -lminsearch option.
 *
 * Revision 1.13  2005/06/22 02:53:12  arthchan2003
 * Add flags \-lmrescore (for rescoring paths in the search), \-op_mode (operation mode), \-Nstalextree (number of lexical tree used in mode 5), \-bt_wsil (back track with silence)
 *
 * Revision 1.10  2005/06/19 19:41:20  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.9  2005/06/17 23:44:39  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.8  2005/06/16 04:59:08  archan
 * Sphinx3 to s3.generic, a gentle-refactored version of Dave's change in senone scale.
 *
 * Revision 1.7  2005/05/27 01:15:44  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.6  2005/05/11 06:10:38  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.5  2005/04/25 23:53:35  archan
 * 1, Some minor modification of vithist_t, vithist_rescore can now support optional LM rescoring, vithist also has its own reporting routine. A new argument -lmrescore is also added in decode and livepretend.  This can switch on and off the rescoring procedure. 2, I am reaching the final difficulty of mode 5 implementation.  That is, to implement an algorithm which dynamically decide which tree copies should be entered.  However, stuffs like score propagation in the leave nodes and non-leaves nodes are already done. 3, As briefly mentioned in 2, implementation of rescoring , which used to happened at leave nodes are now separated. The current implementation is not the most clever one. Wish I have time to change it before check-in to the canonical.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 15-Jun-2004  Yitao Sun (yitao@cs.cmu.edu) at Carnegie Mellon University.
 * Created.
 */

/*
----------------------------
revision 1.3
date: 2004/07/27 21:53:05;  author: yitao;  state: Exp;  lines: +1 -1

weird.  i only changed kb.c to fix a small bug.  why so many changes?
----------------------------
revision 1.2
date: 2004/07/21 04:17:01;  author: yitao;  state: Exp;  lines: +8 -0

fixed conflicts when Arthur merged Ziad's fe_process_frame() code
----------------------------
revision 1.1
date: 2004/07/12 20:56:00;  author: yitao;  state: Exp;

moved these files from src/programs to src/libs3decoder so they could be included in t
he library.
=============================================================================
*/

#include "live_decode_args.h"
#include "cmdln_macro.h"

arg_t arg_def[] = {
  waveform_to_cepstral_command_line_macro()
  cepstral_to_feature_command_line_macro()
  acoustic_model_command_line_macro()
  speaker_adaptation_command_line_macro()

  language_model_command_line_macro()
  log_table_command_line_macro()
  dictionary_command_line_macro()
  phoneme_lookahead_command_line_macro()
  histogram_pruning_command_line_macro()
  fast_GMM_computation_command_line_macro()
  common_filler_properties_command_line_macro()
  common_s3x_beam_properties_command_line_macro()
  control_file_handling_command_line_macro()
  hypothesis_file_handling_command_line_macro()
  score_handling_command_line_macro()
  output_lattice_handling_command_line_macro()
  dag_handling_command_line_macro()
  second_stage_dag_handling_command_line_macro()
  input_lattice_handling_command_line_macro() 
  flat_fwd_multiplex_treatment_command_line_macro() 
  flat_fwd_debugging_command_line_macro() 
  history_table_command_line_macro()

  decode_specific_command_line_macro()
  search_specific_command_line_macro()
  search_modeTST_specific_command_line_macro()
  search_modeWST_specific_command_line_macro()
  control_lm_mllr_file_command_line_macro()
  finite_state_grammar_command_line_macro()
  phone_insertion_penalty_command_line_macro()

  partial_hypothesis_command_line_macro()


  { "-bestscoredir",
    ARG_STRING,
    NULL,
    "(Mode 3) Directory for writing best score/frame (used to set beamwidth; one file/utterance)" },

  { "-machine_endian",
    ARG_STRING,
#ifdef WORDS_BIGENDIAN
    "big",
#else
    "little",
#endif
    "Endianness of machine, big or little" },

    { "-rawext",
      ARG_STRING,
      ".raw",
      "Input raw files extension"},

    { NULL, ARG_INT32, NULL, NULL }
};

#if 0
    /* Commented out; not supported */
    { "-compsep",
      ARG_STRING,
      "",	/* Default: No compound word (NULL separator char) */
      "Separator character between components of a compound word (NULL if none)" },
#endif
#if 0 /*ARCHAN: As mentioned by Evandro, the fact that there is both -cepdir and rawdir is very confusing. Removed*/
    { "-cepdir",
      ARG_STRING,
      NULL,
      "Input cepstrum files directory (prefixed to filespecs in control file)" },
#endif

