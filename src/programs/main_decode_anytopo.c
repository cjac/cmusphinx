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
 * main.c -- Main S3 decoder driver.
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
 * 26-Jul-04    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporated from sphinx 3.0 code base to 3.X codebase. 
 *
 * $Log$
 * Revision 1.15  2006/02/24  04:16:16  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH, tied commands to macros, use utt_decode as the decoding interface.
 * 
 *
 * Revision 1.14  2005/10/05 00:31:14  dhdfu
 * Make int8 be explicitly signed (signedness of 'char' is
 * architecture-dependent).  Then make a bunch of things use uint8 where
 * signedness is unimportant, because on the architecture where 'char' is
 * unsigned, it is that way for a reason (signed chars are slower).
 *
 * Revision 1.13  2005/07/12 01:39:27  arthchan2003
 * Make default -btwsil to be 1 again. In general, the value of -btwsil (or whether to use silence as the final word) largely depends on different databases.  In Communicator, 5% relative improvement is seen. In WSJ, there is 5% drop. So, it is largely a matter of tuning.
 *
 * Revision 1.12.4.17  2006/01/16 20:29:52  arthchan2003
 * Changed -ltsoov to -lts_mismatch. Changed lm_rawscore interface. Change from cmd_ln_access to cmd_ln_str.
 *
 * Revision 1.12.4.16  2005/09/25 20:14:07  arthchan2003
 * Added bogus argument for decode_anytopo. Fixed compilation rules.
 *
 * Revision 1.12.4.15  2005/09/18 01:51:09  arthchan2003
 * put decode_anytopo as a permanent interface for flat lexicon decoding. However, it starts to used the standard utt.c interface.
 *
 * Revision 1.12.4.14  2005/09/11 23:08:47  arthchan2003
 * Synchronize command-line for 2-nd stage rescoring in decode/decode_anytopo/dag, move s3dag_dag_load to dag.c so that srch.c could use it.
 *
 * Revision 1.12.4.13  2005/09/11 02:54:19  arthchan2003
 * Remove s3_dag.c and s3_dag.h, all functions are now merged into dag.c and shared by decode_anytopo and dag.
 *
 * Revision 1.12.4.12  2005/09/07 23:48:09  arthchan2003
 * 1, Removed old recognition loop. 2, Start to only compute active senones indicated by the search routine. 3, Fixed counters.
 *
 * Revision 1.12.4.11  2005/08/03 20:01:33  arthchan2003
 * Added the -topn argument into acoustic_model_command_line_macro
 *
 * Revision 1.12.4.10  2005/08/03 18:55:03  dhdfu
 * Remove bogus initialization of ms_mgau's internals from here
 *
 * Revision 1.12.4.9  2005/08/02 21:42:33  arthchan2003
 * 1, Moved static variables from function level to the application level. 2, united all initialization of HMM using s3_am_init, 3 united all GMM computation using ms_cont_mgau_frame_eval.
 *
 * Revision 1.12.4.8  2005/07/28 03:12:03  arthchan2003
 * Initialized mllr correctly in decode_anytopo.
 *
 * Revision 1.12.4.7  2005/07/27 23:23:39  arthchan2003
 * Removed process_ctl in allphone, dag, decode_anytopo and astar. They were duplicated with ctl_process and make Dave and my lives very miserable.  Now all application will provided their own utt_decode style function and will pass ctl_process.  In that way, the mechanism of reading would not be repeated. livepretend also follow the same mechanism now.  align is still not yet finished because it read yet another thing which has not been considered : transcription.
 *
 * Revision 1.12.4.6  2005/07/26 02:22:27  arthchan2003
 * Merged srch_hyp_t and hyp_t
 *
 * Revision 1.12.4.5  2005/07/24 19:37:19  arthchan2003
 * Removed GAUDEN_EVAL_WINDOW, put it in srch.h now.
 *
 * Revision 1.12.4.4  2005/07/22 03:46:55  arthchan2003
 * 1, cleaned up the code, 2, fixed dox-doc. 3, use srch.c version of log_hypstr and log_hyp_detailed.
 *
 * Revision 1.12.4.3  2005/07/20 21:25:42  arthchan2003
 * Shared to code of Multi-stream GMM initialization in align/allphone and decode_anytopo.
 *
 * Revision 1.12.4.2  2005/07/18 23:21:23  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.12.4.1  2005/07/13 01:19:44  arthchan2003
 * Changed back the default of -btwsil to 1, this conform to the tip of the trunk.
 *
 * Revision 1.12  2005/06/22 05:39:55  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.13  2005/06/19 04:51:48  archan
 * Add multi-class MLLR support for align, decode_anytopo as well as allphone.
 *
 * Revision 1.12  2005/06/19 03:58:17  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.11  2005/06/18 18:17:52  archan
 * Update decode_anytopo such that it also used the lmset interface. Notice it still doesn't support multiple LMs and class-based LM at this point
 *
 * Revision 1.10  2005/06/17 23:46:06  archan
 * Sphinx3 to s3.generic 1, Remove bogus log messages in align and allphone, 2, Unified the logbase value from 1.0001 to 1.0003
 *
 * Revision 1.9  2005/06/03 06:53:00  archan
 * Push required argument into decode_anytopo.c
 *
 * Revision 1.8  2005/06/03 06:45:30  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.7  2005/06/03 06:12:57  archan
 * 1, Simplify and unify all call of logs3_init, move warning when logbase > 1.1 into logs3.h.  2, Change arguments to require arguments in align and astar.
 *
 * Revision 1.6  2005/06/03 05:46:42  archan
 * Log. Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 * Revision 1.5  2005/05/27 01:15:46  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.4  2005/05/26 22:03:18  archan
 * Add support for backtracking without assuming silence </s> has to be the last word.
 *
 * Revision 1.3  2005/04/21 23:50:27  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.2  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.1.1.1  2005/03/24 15:24:01  archan
 * I found Evandro's suggestion is quite right after yelling at him 2 days later. So I decide to check this in again without any binaries. (I have done make distcheck. ) . Again, this is a candidate for s3.6 and I believe I need to work out 4-5 intermediate steps before I can complete the first prototype.  That's why I keep local copies. 
 *
 * Revision 1.9  2005/02/09 05:59:30  arthchan2003
 * Sychronize the -option names in slow and faster decoders.  This makes many peopple's lives easier. Also update command-line. make test-full is done.
 *
 * Revision 1.8  2005/02/05 15:30:07  egouvea
 * Changed default cepdir to null, so a user can choose not to specify it, and making it consistent with decode.c. Removed assert checking if cepdir was null
 *
 * Revision 1.7  2004/12/23 21:05:22  arthchan2003
 * Enable compilation of decode_anytopo, change option names from -match to -hyp, it makes the code more consistent.
 *
 * Revision 1.6  2004/12/14 00:50:33  arthchan2003
 * 1, Change the code to accept extension, 2, add timer to livepretend, 3, fixing the s3_astar to separate the bypass variable to bypass and is_filler_bypass.  4, Add some doxygen comments. 5, Don't care about changes in main_decode_anytopo.c. It is still under work, 6, remove option -help and -example from 3.5 releases.
 *
 * Revision 1.5  2004/12/06 11:31:47  arthchan2003
 * Fix brief comments for programs.
 *
 * Revision 1.4  2004/12/06 11:15:11  arthchan2003
 * Enable doxygen in the program directory.
 *
 * Revision 1.3  2004/12/05 12:01:32  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.2  2004/11/16 05:13:19  arthchan2003
 * 1, s3cipid_t is upgraded to int16 because we need that, I already check that there are no magic code using 8-bit s3cipid_t
 * 2, Refactor the ep code and put a lot of stuffs into fe.c (should be renamed to something else.
 * 3, Check-in codes of wave2feat and cepview. (cepview will not dump core but Evandro will kill me)
 * 4, Make the same command line frontends for decode, align, dag, astar, allphone, decode_anytopo and ep . Allow the use a file to configure the application.
 * 5, Make changes in test such that test-allphone becomes a repeatability test.
 * 6, cepview, wave2feat and decode_anytopo will not be installed in 3.5 RCIII
 * (Known bugs after this commit)
 * 1, decode_anytopo has strange bugs in some situations that it cannot find the end of the lattice. This is urgent.
 * 2, default argument file's mechanism is not yet supported, we need to fix it.
 * 3, the bug discovered by SonicFoundry is still not fixed.
 *
 * Revision 1.1  2004/11/14 07:00:08  arthchan2003
 * 1, Finally, a version of working flat decoder is completed. It is not compiled in the standard compilation yet because there are two many warnings. 2, eliminate the statics variables in  fe_sigproc.c
 *
 * Revision 1.2  2002/12/03 23:02:40  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 *
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 * 
 * 02-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Restricted MLLR transformation to CD mixture densities only.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the meaning of -matchsegfn and, correspondingly, log_hypseg().
 * 
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -min_endfr and -dagfudge arguments.
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added BSTXCT: reporting since that became available from dag_search.
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ,NODES suffix to -outlatdir argument for dumping only words to
 * 		lattice output files.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added fillpen_init() and removed explicit addition of SILENCE_WORD,
 * 		START_WORD and FINISH_WORD to the dictionary.
 * 
 * 04-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added unlimit() call to remove malloc restrictions.
 *  
 * 26-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added separate language weight (-bestpathlw) for bestpath DAG search.
 * 		Added -mllrctlfn flag and handling.
 * 
 * 21-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -bptblsize argument.
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added optional start/end frame specification in control file, for
 * 		processing selected segments (utterances) from a large cepfile.
 * 		Control spec: cepfile [startframe endframe [uttid]].
 * 
 * 13-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: added senscale to bestscr before writing best score file.
 * 		(Otherwise, the scaled scores are meaningless.)
 * 		Added ,EXACT suffix option to -matchfn argument, and correspondingly
 * 		added "exact" argument to log_hypstr().  (But running bestpath search
 * 		will still cause <sil> and filler words to be removed from matchfile.)
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed structure of gauden/senone computation:
 * 		    from: foreach (gauden) {eval gauden}; foreach (senone) {senone}
 * 		    to:   foreach (gauden) {eval gauden;  foreach (senone in gauden) {...}}
 * 		reducing memory space for results of gauden, specially in block mode.
 * 		Normalized senone scores (subtracting the best) rather than density scores.
 * 		Changed active senone list to flags.
 * 
 * 09-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Block-mode gauden computation for improving cache performance: changed
 *		    from: foreach(frame) {foreach(gauden)...}
 * 		    to:   foreach(gauden) {foreach(frame)...}
 * 		within a block of frames.  Must evaluate all gauden, not just active ones.
 * 		But even so the resulting caching performance is better.
 * 
 * 29-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed argument -inlatext to -latext.
 * 		Added check to ensure input and output lattice directories are different.
 * 		Added reporting of hostname.
 * 
 * 23-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed profiling to use timing_ functions, available on all platforms.
 * 		Added write_bestscore() for writing best statescore in each frame,
 * 		for helping determine desirable beamwidth.
 * 
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added DAG search option and consolidated logging and reporting.
 * 		Added backtrace option.
 * 
 * 22-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added absolute (unnormalized) acoustic scores in log file.
 * 		Added uttid in log file with each word segmentation.
 * 		Compute only active codebooks and senones if multiple codebooks present.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added specification of input word lattices to limit search (-inlatdir
 * 		argument...).  Added computation of active senone and gauden codebook
 * 		lists when such a lattice is provided, to minimize computation.
 * 		Added -cmn and -agc flags.
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added call to feat_init(); added cepsize variable and initialization;
 * 		Changed argument to norm_mean from featlen[0]+1 to cepsize.
 * 
 * 13-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed call to senone_eval to senone_eval_all optimized for the
 * 		semi-continuous case.
 * 		Completed handling multiple mixture-gaussian codebooks.
 * 
 * 01-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */

/** \file main_decode_anytopo.c
 * \brief Main driver for sphinx 3.0 decoding (or the slow decoder)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <s3types.h>
#include "logs3.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"
#include "feat.h"
#include "bio.h"
#include <wid.h>
#include "search.h"

#include "cmn.h"
#include "agc.h"

#include "flat_fwd.h"
#include "ms_mllr.h"
#include "ms_mgau.h"
#include "ms_gauden.h"
#include "ms_senone.h"
#include "dag.h"
#include "cb2mllr_io.h"
#include "cmdln_macro.h"
#include "corpus.h"
#include "kbcore.h"
#include "kb.h"

#include "srch.h"
#include "utt.h"

static int32 matchexact;
static int32 outlat_onlynodes;
static char *inlatdir;
static char *outlatdir;

/*
 * Command line arguments.
 */
static arg_t defn[] = {
  log_table_command_line_macro()
  cepstral_to_feature_command_line_macro()
  acoustic_model_command_line_macro()
  speaker_adaptation_command_line_macro()
  language_model_command_line_macro()
  dictionary_command_line_macro()

  fast_GMM_computation_command_line_macro()
  common_filler_properties_command_line_macro()
  common_application_properties_command_line_macro()
  control_file_handling_command_line_macro()
  hypothesis_file_handling_command_line_macro()
  score_handling_command_line_macro()

  cepstral_input_handling_command_line_macro()

  input_lattice_handling_command_line_macro() 
  flat_fwd_multiplex_treatment_command_line_macro() 
  flat_fwd_debugging_command_line_macro() 
  output_lattice_handling_command_line_macro()
  dag_handling_command_line_macro()
  search_specific_command_line_macro()
  control_lm_mllr_file_command_line_macro()
  phone_insertion_penalty_command_line_macro()
  second_stage_dag_handling_command_line_macro()
  history_table_command_line_macro()
  common_s3x_beam_properties_command_line_macro()
  phoneme_lookahead_command_line_macro() 

  /* Things which are not yet synchronized with decode/dag/astar */
  { "-op_mode",
    ARG_INT32,
    "3",
    "decode_anytopo's operation mode.  It can only be set at 3 in this interface. Please use decode for a generic interface."},
    { "-lambda",
      ARG_STRING,
      NULL,
      "Interpolation weights (CD/CI senone) parameters input file" },
/* ADDED BY BHIKSHA: 6 JAN 98 */
    { "-ceplen",
      ARG_INT32,
      "13",
      "Length of input feature vector" },
    { "-bestscoredir",
      ARG_STRING,
      NULL,
      "Directory for writing best score/frame (used to set beamwidth; one file/utterance)" },
    { "-hmmdump", 
      ARG_INT32, 
      "0",
      "Not used in this interface. " }, 
    {"-composite", 
      ARG_INT32, 
     "1",
     "Not used in this interface, exit if set to 0"},
  {"-fsg",
   ARG_STRING,
   NULL,
   "Not used in this interface"},

    { NULL, ARG_INT32,  NULL, NULL }


};

int main (int32 argc, char *argv[])
{
    kb_t kb;
    stat_t* st;

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc,argv,"default.arg",defn);
    unlimit ();

    if(cmd_ln_int32("-op_mode")!=OPERATION_FLATFWD){
      E_FATAL("decode_anytopo only provides interface for flat lexicon decoding. Please use decode instead\n");
      cmd_ln_appl_exit();
    }

    kb_init (&kb);
    st = kb.stat;
    fprintf (stdout, "\n"); 
   
    inlatdir = (char *) cmd_ln_access ("-inlatdir");
    outlatdir = (char *) cmd_ln_access ("-outlatdir");
    if (outlatdir) {
	int32 k;
	
	k = strlen(outlatdir);
	if ((k > 6) && (strcmp (outlatdir+(k-6), ",NODES") == 0)) {
	    outlat_onlynodes = 1;
	    outlatdir[k-6] = '\0';
	} else
	    outlat_onlynodes = 0;
    }

    if (inlatdir && outlatdir && (strcmp (inlatdir, outlatdir) == 0))
	E_FATAL("Input and output lattice directories are the same\n");

    /*    tot_nfr = 0;*/

#if 0
    /* Decode_anytopo-specific, a search for suffix ",EXACT " and create exact log hypothesis */
    if ((matchfile = (char *) cmd_ln_access("-hyp")) == NULL) {
	matchfp = NULL;
    } else {
	/* Look for ,EXACT suffix, for retaining fillers/pronunciation specs in output */
	k = strlen (matchfile);
	if ((k > 6) && (strcmp (matchfile+(k-6), ",EXACT") == 0)) {
	    matchexact = 1;
	    matchfile[k-6] = '\0';
	} else
	    matchexact = 0;

	if ((matchfp = fopen (matchfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchfile);
    }
    
#endif

    
    if (cmd_ln_str ("-ctl")) {
      /* When -ctlfile is speicified, corpus.c will look at -ctl_mllr to get
	 the corresponding  MLLR for the utterance */
      ctl_process (cmd_ln_str("-ctl"),
		   NULL, /* -ctl_lm is set to zero, that is to say, we can't support it in decode_anytopo*/
		   cmd_ln_str("-ctl_mllr"),
		   cmd_ln_int32("-ctloffset"),
		   cmd_ln_int32("-ctlcount"),
		   utt_decode, 
		   &kb);
    } else {
      /* Is error checking good enough?" */
      E_FATAL(" -ctl are not specified.\n");
    }

    if (kb.matchsegfp)
	fclose (kb.matchsegfp);
    if (kb.matchfp) 
        fclose (kb.matchfp);

    stat_report_corpus(kb.stat);

    kb_free(&kb);


#if (! WIN32)
#if defined(_SUN4)
    system("ps -el | grep decode_anytopo");
#else
    system ("ps aguxwww | grep decode_anytopo");
#endif
#endif


    cmd_ln_appl_exit();

    return 0 ;
}

