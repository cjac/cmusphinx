/* -*- c-basic-offset: 4 -*- */
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
 * main_astar.c -- Driver for N-best list creation from DAGs using A* algorithm.
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
 * Revision 1.12  2006/02/24  04:02:15  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Changed commands to macro.
 * 
 * Revision 1.11.4.6  2006/01/16 20:27:41  arthchan2003
 * Changed -ltsoov to -lts_mismatch.
 *
 * Revision 1.11.4.5  2005/09/25 20:09:47  arthchan2003
 * Added support for LTS.
 *
 * Revision 1.11.4.4  2005/09/18 01:52:27  arthchan2003
 * Tied dag handling command line argument.
 *
 * Revision 1.11.4.3  2005/09/11 02:54:19  arthchan2003
 * Remove s3_dag.c and s3_dag.h, all functions are now merged into dag.c and shared by decode_anytopo and dag.
 *
 * Revision 1.11.4.2  2005/07/27 23:23:39  arthchan2003
 * Removed process_ctl in allphone, dag, decode_anytopo and astar. They were duplicated with ctl_process and make Dave and my lives very miserable.  Now all application will provided their own utt_decode style function and will pass ctl_process.  In that way, the mechanism of reading would not be repeated. livepretend also follow the same mechanism now.  align is still not yet finished because it read yet another thing which has not been considered : transcription.
 *
 * Revision 1.11.4.1  2005/07/18 23:21:23  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.11  2005/06/22 05:38:26  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset
 *
 * Revision 1.12  2005/06/19 03:58:17  archan
 * 1, Move checking of Silence wid, start wid, finish wid to dict_init. This unify the checking and remove several segments of redundant code. 2, Remove all startwid, silwid and finishwid.  They are artefacts of 3.0/3.x merging. This is already implemented in dict.  (In align, startwid, endwid, finishwid occured in several places.  Checking is also done multiple times.) 3, Making corresponding changes to all files which has variable startwid, silwid and finishwid.  Should make use of the marco more.
 *
 * Revision 1.11  2005/06/18 21:16:36  archan
 * Fixed a bug in astar, introduced another two tests to test on functionality of class-based LM in dag and astar.
 *
 * Revision 1.10  2005/06/18 20:05:24  archan
 * Sphinx3 to s3.generic: Set lm correctly in dag.c and astar.c.  Same changes should also be applied to decode_anytopo.
 *
 * Revision 1.9  2005/06/18 03:23:13  archan
 * Change to lmset interface.
 *
 * Revision 1.8  2005/06/17 23:46:06  archan
 * Sphinx3 to s3.generic 1, Remove bogus log messages in align and allphone, 2, Unified the logbase value from 1.0001 to 1.0003
 *
 * Revision 1.7  2005/06/03 06:45:30  archan
 * 1, Fixed compilation of dag_destroy, dag_dump and dag_build. 2, Changed RARG to REQARG.
 *
 * Revision 1.6  2005/06/03 06:12:57  archan
 * 1, Simplify and unify all call of logs3_init, move warning when logbase > 1.1 into logs3.h.  2, Change arguments to require arguments in align and astar.
 *
 * Revision 1.5  2005/06/03 05:46:42  archan
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
 * Revision 1.4  2005/05/27 01:15:45  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
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
 * Revision 1.9  2004/12/06 11:31:47  arthchan2003
 * Fix brief comments for programs.
 *
 * Revision 1.8  2004/12/06 11:15:11  arthchan2003
 * Enable doxygen in the program directory.
 *
 * Revision 1.7  2004/12/05 12:01:32  arthchan2003
 * 1, move libutil/libutil.h to s3types.h, seems to me not very nice to have it in every files. 2, Remove warning messages of main_align.c 3, Remove warning messages in chgCase.c
 *
 * Revision 1.6  2004/12/02 18:24:42  dhdfu
 * Remove some compiler warnings
 *
 * Revision 1.5  2004/11/16 05:53:10  arthchan2003
 * small warning problems in main_astar.c
 *
 * Revision 1.4  2004/11/16 05:13:19  arthchan2003
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
 * Revision 1.3  2004/10/07 22:46:26  dhdfu
 * Fix compiler warnings that are also real bugs (but why does this
 * function take an int32 when -lw is a float parameter?)
 *
 * Revision 1.2  2004/09/13 08:13:28  arthchan2003
 * update copyright notice from 200x to 2004
 *
 * Revision 1.1  2004/08/09 05:15:23  arthchan2003
 * incorporate s3.0 astar
 *
 *
 * 26-Jun-2004  ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First incorporate it from CarlQ's patch.
 * 
 * Revision 1.1  2003/02/12 16:30:42  cbq
 * A simple n-best program for sphinx.
 *
 * Revision 1.1  2000/04/24 09:39:41  lenzo
 * s3 import.
 *
 * 
 * 31-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added compound words handling option.  Compound words are broken up
 * 		into component words for computing LM probabilities.
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice and nbest files.
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxedge argument to control memory usage.
 * 		Added -maxlmop and -maxlpf options to control execution time.
 * 		Added -maxppath option to control CPU/memory usage.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 * 
 * 29-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from nbest-main.c
 */

/** \file main_astar.c 
 * \brief Driver for N-best list creation from DAGs using A* algorithm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <sphinxbase/info.h>
#include <sphinxbase/unlimit.h>

#include <s3types.h>
#include <logs3.h>
#include <tmat.h>
#include <mdef.h>
#include <dict.h>
#include <lm.h>
#include <fillpen.h>
#include <search.h>
#include <wid.h>
#include <dag.h>
#include <cmdln_macro.h>
#include <corpus.h>
#include <astar.h>

static mdef_t *mdef;            /* Model definition */

static ptmr_t tm_utt;           /* Entire utterance */

static dict_t *dict;            /* The dictionary       */
static fillpen_t *fpen;         /* The filler penalty structure. */
static lmset_t *lmset;          /* The lmset.           */
static logmath_t *logmath;

static const char *nbestdir;
static cmd_ln_t *config;

/*
 * Command line arguments.
 */
static arg_t defn[] = {
    log_table_command_line_macro(),
    dictionary_command_line_macro(),
    language_model_command_line_macro(),
    common_filler_properties_command_line_macro(),
    common_application_properties_command_line_macro(),
    control_file_handling_command_line_macro(),
    control_lm_file_command_line_macro(),
    dag_handling_command_line_macro(),
    {"-mdef",
     ARG_STRING,
     NULL,
     "Model definition input file: triphone -> senones/tmat tying"},
    {"-beam",
     ARG_FLOAT64,
     "1e-64",
     "Partial path pruned if below beam * score of best partial ppath so far"},
    {"-maxppath",
     ARG_INT32,
     "1000000",
     "Max partial paths created after which utterance aborted; controls CPU/memory use"},
    {"-inlatdir",
     ARG_STRING,
     NULL,
     "Input word-lattice directory with per-utt files for restricting words searched"},
    {"-latext",
     ARG_STRING,
     "lat.gz",
     "Word-lattice filename extension (.gz or .Z extension for compression)"},
    {"-nbestdir",
     ARG_STRING,
     ".",
     "Input word-lattice directory with per-utt files for restricting words searched"},
    {"-nbestext",
     ARG_STRING,
     "nbest.gz",
     "N-best filename extension (.gz or .Z extension for compression)"},
    {"-nbest",
     ARG_INT32,
     "200",
     "Max. n-best hypotheses to generate per utterance"},
    {"-ppathdebug",
     ARG_BOOLEAN,
     "no",
     "Generate debugging information for the search. "},

    {NULL, ARG_INT32, NULL, NULL}
};

/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void
models_init(void)
{
    /* HMM model definition */
    mdef = mdef_init(cmd_ln_str_r(config, "-mdef"), 1);

    /* Dictionary */
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

/*
 * Build a filename int buf as follows (without file extension):
 *     if dir ends with ,CTLand ctlspec does not begin with /, filename is dir/ctlspec
 *     if dir ends with ,CTL and ctlspec DOES begin with /, filename is ctlspec
 *     if dir does not end with ,CTL, filename is dir/uttid,
 * where ctlspec is the complete utterance spec in the input control file, and
 * uttid is the last component of ctlspec.
 */
static void
build_output_uttfile(char *buf, const char *dir, const char *uttid, const char *ctlspec)
{
    int32 k;

    k = strlen(dir);
    if ((k > 4) && (strcmp(dir + k - 4, ",CTL") == 0)) {        /* HACK!! Hardwired ,CTL */
        if (ctlspec[0] != '/') {
            strcpy(buf, dir);
            buf[k - 4] = '/';
            strcpy(buf + k - 3, ctlspec);
        }
        else
            strcpy(buf, ctlspec);
    }
    else {
        strcpy(buf, dir);
        buf[k] = '/';
        strcpy(buf + k + 1, uttid);
    }
}

/* Decode the given mfc file and write result to given directory */
static void
utt_astar(void *data, utt_res_t * ur, int32 sf, int32 ef, char *uttid)
{
    char dagfile[1024], nbestfile[1024];
    const char *latdir;
    const char *latext;
    const char *nbestext;
    dag_t *dag;
    int32 nfrm;

    if (ur->lmname)
        lmset_set_curlm_wname(lmset, ur->lmname);

    latdir = cmd_ln_str_r(config, "-inlatdir");
    latext = cmd_ln_str_r(config, "-latext");
    nbestext = cmd_ln_str_r(config, "-nbestext");
    if (latdir) {
        build_output_uttfile(dagfile, latdir, uttid, ur->uttfile);
        strcat(dagfile, ".");
        strcat(dagfile, latext);
    }
    else
        sprintf(dagfile, "%s.%s", uttid, latext);

    ptmr_reset(&tm_utt);
    ptmr_start(&tm_utt);

    nfrm = 0;
    if ((dag = dag_load(dagfile,
                        cmd_ln_int32_r(config, "-maxedge"),
                        cmd_ln_float32_r(config, "-logbase"),
                        cmd_ln_int32_r(config, "-dagfudge"), dict, fpen, config, logmath)) != NULL) {
        if (dict_filler_word(dict, dag->end->wid))
            dag->end->wid = dict->finishwid;

        dag_remove_unreachable(dag);
        if (dag_bypass_filler_nodes(dag, 1.0, dict, fpen) < 0) {
            E_ERROR("maxedge limit (%d) exceeded\n", dag->maxedge);
            goto search_done;
        }
        dag_compute_hscr(dag, dict, lmset->cur_lm, 1.0);
        dag_remove_bypass_links(dag);

        E_INFO("%5d frames, %6d nodes, %8d edges, %8d bypass\n",
               dag->nfrm, dag->nnode, dag->nlink, dag->nbypass);

        nfrm = dag->nfrm;
        build_output_uttfile(nbestfile, nbestdir, uttid, ur->uttfile);
        strcat(nbestfile, ".");
        strcat(nbestfile, nbestext);
        nbest_search(dag, nbestfile, uttid, 1.0, dict, lmset->cur_lm, fpen);

        lm_cache_stats_dump(lmset->cur_lm);
        lm_cache_reset(lmset->cur_lm);
    }
    else
        E_ERROR("Dag load (%s) failed\n", uttid);
search_done:
    dag_destroy(dag);

    ptmr_stop(&tm_utt);

    printf("%s: TMR: %5d Frm", uttid, nfrm);
    if (nfrm > 0) {
        printf(" %6.2f xEl", tm_utt.t_elapsed * 100.0 / nfrm);
        printf(" %6.2f xCPU", tm_utt.t_cpu * 100.0 / nfrm);
    }
    printf("\n");
    fflush(stdout);
}

int
main(int32 argc, char *argv[])
{
    /*  kb_t kb;
       ptmr_t tm; */

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", defn);
    unlimit();

    config = cmd_ln_get();

    logmath = logs3_init(cmd_ln_float64_r(config, "-logbase"), 1,
                         cmd_ln_int32_r(config, "-log3table"));

    /* Read in input databases */
    models_init();

    ptmr_init(&tm_utt);

    nbestdir = cmd_ln_str_r(config, "-nbestdir");

    if (cmd_ln_str_r(config, "-ctl")) {
        ctl_process(cmd_ln_str_r(config, "-ctl"),
                    cmd_ln_str_r(config, "-ctl_lm"),
                    NULL,
                    cmd_ln_int32_r(config, "-ctloffset"),
                    cmd_ln_int32_r(config, "-ctlcount"), utt_astar, NULL);

    }
    else {
        E_FATAL("-ctl is not specified\n");
    }

    models_free();

    logmath_free(logmath);

#if (! WIN32)
    system("ps aguxwww | grep s3astar");
#endif

    cmd_ln_free_r(config);
    return 0;
}
