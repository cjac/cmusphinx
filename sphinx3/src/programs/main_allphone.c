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
 * main_allphone.c -- Main driver routine for allphone Viterbi decoding.
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
 * $Log: main_allphone.c,v $
 * Revision 1.19  2006/02/27 16:18:29  arthchan2003
 * Fixed allphone, 1, matchfile and matchsegfile were not generated correctly.  Now is fixed.  2, also added back apply mllr at the beginning of an utterance.
 *
 * Revision 1.18  2006/02/24 16:42:21  arthchan2003
 * Fixed allphone compilation.  At this point, the code doesn't pass make check yet.
 *
 * Revision 1.17  2006/02/24 13:43:43  arthchan2003
 * Temporarily removed allphone's compilation. used lm_read_advance in several cases.
 *
 * Revision 1.16  2006/02/24 04:38:04  arthchan2003
 * Merged Dave's change and my changes: started to use macros.  use Dave's change on -hyp and -hypseg. Used ctl_process.  Still need test.
 *
 *
 * Revision 1.15  2006/02/07 20:51:33  dhdfu
 * Add -hyp and -hypseg arguments to allphone so we can calculate phoneme
 * error rate in a straightforward way.
 *
 * Revision 1.14  2006/02/02 22:56:07  dhdfu
 * Add ARPA language model support to allphone
 *
 * Revision 1.13.4.10  2005/09/26 02:28:26  arthchan2003
 * Changed -s3hmmdir to -hmm
 *
 * Revision 1.13.4.9  2005/09/11 02:54:19  arthchan2003
 * Remove s3_dag.c and s3_dag.h, all functions are now merged into dag.c and shared by decode_anytopo and dag.
 *
 * Revision 1.13.4.8  2005/08/03 20:01:32  arthchan2003
 * Added the -topn argument into acoustic_model_command_line_macro
 *
 * Revision 1.13.4.7  2005/08/03 18:55:03  dhdfu
 * Remove bogus initialization of ms_mgau's internals from here
 *
 * Revision 1.13.4.6  2005/08/02 21:42:33  arthchan2003
 * 1, Moved static variables from function level to the application level. 2, united all initialization of HMM using s3_am_init, 3 united all GMM computation using ms_cont_mgau_frame_eval.
 *
 * Revision 1.13.4.5  2005/07/27 23:23:39  arthchan2003
 * Removed process_ctl in allphone, dag, decode_anytopo and astar. They were duplicated with ctl_process and make Dave and my lives very miserable.  Now all application will provided their own utt_decode style function and will pass ctl_process.  In that way, the mechanism of reading would not be repeated. livepretend also follow the same mechanism now.  align is still not yet finished because it read yet another thing which has not been considered : transcription.
 *
 * Revision 1.13.4.4  2005/07/24 19:37:19  arthchan2003
 * Removed GAUDEN_EVAL_WINDOW, put it in srch.h now.
 *
 * Revision 1.13.4.3  2005/07/22 03:46:55  arthchan2003
 * 1, cleaned up the code, 2, fixed dox-doc. 3, use srch.c version of log_hypstr and log_hyp_detailed.
 *
 * Revision 1.13.4.2  2005/07/20 21:25:42  arthchan2003
 * Shared to code of Multi-stream GMM initialization in align/allphone and decode_anytopo.
 *
 * Revision 1.13.4.1  2005/07/18 23:21:23  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.13  2005/06/22 05:37:45  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init
 *
 * Revision 1.8  2005/06/19 04:51:48  archan
 * Add multi-class MLLR support for align, decode_anytopo as well as allphone.
 *
 * Revision 1.7  2005/06/17 23:46:06  archan
 * Sphinx3 to s3.generic 1, Remove bogus log messages in align and allphone, 2, Unified the logbase value from 1.0001 to 1.0003
 *
 * Revision 1.6  2005/06/03 06:12:57  archan
 * 1, Simplify and unify all call of logs3_init, move warning when logbase > 1.1 into logs3.h.  2, Change arguments to require arguments in align and astar.
 *
 * Revision 1.5  2005/05/27 01:15:45  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.4  2005/04/21 23:50:27  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.2  2005/03/30 00:43:41  archan
 *
 * 19-Jun-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Modified to handle the new libfeat interface.
 * 
 * 02-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added allphone lattice output.
 * 
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added .semi. and .cont. options to -senmgaufn flag.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 * 
 * 15-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3types.h>

#include "feat.h"
#include "logs3.h"
#include "ms_mllr.h"
#include "ms_gauden.h"
#include "ms_senone.h"
#include "ms_mgau.h"
#include "cb2mllr_io.h"
#include "srch.h"
#include "corpus.h"

#ifdef INTERP
#include "interp.h"
#endif

#include "tmat.h"
#include "mdef.h"
#include "s3_allphone.h"
#include "agc.h"
#include "cmn.h"
#include "cmdln_macro.h"

/** \file main_allphone.c
 * \brief  Main driver routine for allphone Viterbi decoding
 */
static arg_t defn[] = {
    cepstral_to_feature_command_line_macro()
    log_table_command_line_macro()
    acoustic_model_command_line_macro()
    fast_GMM_computation_command_line_macro()
    speaker_adaptation_command_line_macro()
    common_application_properties_command_line_macro()
    control_file_handling_command_line_macro()
    hypothesis_file_handling_command_line_macro()
    control_mllr_file_command_line_macro()
    cepstral_input_handling_command_line_macro()
    language_model_command_line_macro()
    control_lm_file_command_line_macro()
    dictionary_command_line_macro()
    finite_state_grammar_command_line_macro()
    common_filler_properties_command_line_macro()

    {"-beam",
     ARG_FLOAT64,
     "1e-64",
     "Main pruning beam applied during search"},
    {"-pbeam",
     ARG_FLOAT64,
     "1e-30",
     "Phone transition beam applied during search"},
    {"-phsegdir",
     ARG_STRING,
     NULL,
     "Output directory for phone segmentation files; optionally end with ,CTL"},
    {"-utt",
     ARG_STRING,
     NULL,
     "Utterance file to be processed (-ctlcount argument times)"},

    {NULL, ARG_INT32, NULL, NULL}
};


static void
allphone_log_hypseg(kb_t *kb,
                    phseg_t * hypptr,   /* In: Hypothesis */
		    char *uttid,
                    int32 nfrm, /* In: #frames in utterance */
                    int32 scl)
{                               /* In: Acoustic scaling for entire utt */
    phseg_t *h;
    int32 ascr, lscr, tscr;
    kbcore_t *kbcore = kb->kbcore;
    FILE *fp = kb->matchsegfp;

    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
        ascr += h->score;
        lscr += h->tscore;      /* FIXME: unscaled score? */
        tscr += h->score + h->tscore;
    }

    fprintf(fp, "%s S %d T %d A %d L %d", uttid, scl, tscr, ascr, lscr);

    if (!hypptr)                /* HACK!! */
        fprintf(fp, " (null)\n");
    else {
        for (h = hypptr; h; h = h->next) {
            fprintf(fp, " %d %d %d %s", h->sf, h->score, h->tscore,
                    mdef_ciphone_str(kbcore_mdef(kbcore), h->ci));
        }
        fprintf(fp, " %d\n", nfrm);
    }

    fflush(fp);
}

/* Write hypothesis in old (pre-Nov95) NIST format */
static void
allphone_log_hypstr(kb_t *kb, phseg_t * hypptr, char *uttid)
{
    kbcore_t *kbcore = kb->kbcore;
    FILE *fp = kb->matchfp;
    phseg_t *h;

    if (!hypptr)                /* HACK!! */
        fprintf(fp, "(null)");

    for (h = hypptr; h; h = h->next) {
        fprintf(fp, "%s ", mdef_ciphone_str(kbcore_mdef(kbcore), h->ci));
    }
    fprintf(fp, " (%s)\n", uttid);
    fflush(fp);
}


/* Write phone segmentation output file */
static void
write_phseg(kb_t *kb, char *dir, char *uttid, phseg_t * phseg)
{
    char str[1024];
    FILE *fp = (FILE *) 0;
    int32 uttscr;
    kbcore_t *kbcore = kb->kbcore;

    /* Attempt to write segmentation for this utt to a separate file */
    if (dir) {
        sprintf(str, "%s/%s.allp", dir, uttid);
        E_INFO("Writing phone segmentation to: %s\n", str);
        if ((fp = fopen(str, "w")) == NULL) {
            E_ERROR("fopen(%s,w) failed\n", str);
            dir = NULL;         /* Flag to indicate fp shouldn't be closed at the end */
        }
    }

    if (!dir) {
        fp = stdout;            /* Segmentations can be directed to stdout this way */
        E_INFO("Phone segmentation (%s):\n", uttid);
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }

    fprintf(fp, "\t%5s %5s %9s %s\n", "SFrm", "EFrm", "SegAScr", "Phone");
    fflush(fp);

    uttscr = 0;
    for (; phseg; phseg = phseg->next) {
        if (!dir) {
            fprintf(fp, "ph:%s>", uttid);
            fflush(fp);
        }
        fprintf(fp, "\t%5d %5d %9d %s\n",
                phseg->sf, phseg->ef, phseg->score,
                mdef_ciphone_str(kbcore_mdef(kbcore), phseg->ci));
        fflush(fp);
        uttscr += (phseg->score);
    }

    if (!dir) {
        fprintf(fp, "PH:%s>", uttid);
        fflush(fp);
    }
    fprintf(fp, " Total score: %11d\n", uttscr);
    fflush(fp);
    if (dir)
        fclose(fp);
    else {
        fprintf(fp, "\n");
        fflush(fp);
    }
}

/*
 * Find Viterbi allphone decoding.
 */
static int32
allphone_utt(kb_t *kb, int32 nfr, char *uttid)
{
    int32 i;
    int32 w;
    phseg_t *phseg;
    int32 scl;
    kbcore_t *kbcore;
    stat_t *st;

    kbcore = kb->kbcore;
    st = kb->stat;
    stat_clear_utt(st);
    st->nfr = nfr;

    w = feat_window_size(kbcore_fcb(kbcore));
    if (nfr < w * 2 + 1) {
        E_ERROR("Utterance %s < %d frames (%d); ignored\n", uttid,
                w * 2 + 1, nfr);
        return -1;
    }

    allphone_start_utt(uttid);

    scl = 0;
    for (i = 0; i < nfr; i++) {
	/* Compute GMM scores "manually" for the time being. */
	/* First the CI senones, which are always computed. */
	ptmr_start(&(st->tm_sen));
	ptmr_start(&(st->tm_ovrhd));
	approx_cont_mgau_ci_eval(kbcore,
				 kb->fastgmm,
				 kbcore_mdef(kbcore),
				 kb->feat[i][0],
				 kb->ascr->cache_ci_senscr[0],
				 &(kb->ascr->cache_best_list[0]), i);
	st->utt_cisen_eval += mgau_frm_cisen_eval(kbcore_mgau(kbcore));
	st->utt_cigau_eval += mgau_frm_cigau_eval(kbcore_mgau(kbcore));
	ptmr_stop(&(st->tm_ovrhd));
	ptmr_stop(&(st->tm_sen));

	/* Now all the active CD senones. */
        ptmr_start(&(st->tm_sen));
	allphone_sen_active(kb->ascr);
	scl +=
	    approx_cont_mgau_frame_eval(kbcore, kb->fastgmm,
					kb->ascr, kb->feat[i][0], i,
					kb->ascr->cache_ci_senscr[0],
					&(st->tm_ovrhd));
	st->utt_sen_eval += mgau_frm_sen_eval(kbcore_mgau(kbcore));
	st->utt_gau_eval += mgau_frm_gau_eval(kbcore_mgau(kbcore));
        ptmr_stop(&(st->tm_sen));

        ptmr_start(&(st->tm_srch));
        allphone_frame(kb->ascr, st);
        if ((i % 10) == 9) {
            printf(".");
            fflush(stdout);
        }
        ptmr_stop(&(st->tm_srch));
    }
    st->tot_fr += nfr;
    printf("\n");

    phseg = allphone_end_utt(uttid);
    write_phseg(kb, (char *) cmd_ln_access("-phsegdir"), uttid, phseg);
    /* Log recognition output to the standard match and matchseg files */
    if (kb->matchfp)
        allphone_log_hypstr(kb, phseg, uttid);

    if (kb->matchsegfp)
        allphone_log_hypseg(kb, phseg, uttid, nfr, scl);

    stat_report_utt(st, uttid);
    stat_update_corpus(st);

    ptmr_reset(&(st->tm_sen));
    ptmr_reset(&(st->tm_srch));
    ptmr_reset(&(st->tm_ovrhd));

    printf("\n");
    fflush(stdout);

    return 0;
}

static void
utt_allphone(void *data, utt_res_t * ur, int32 sf, int32 ef, char *uttid)
{
    kb_t *kb;
    kbcore_t *kbcore;
    int32 nfr;
    char *cepdir, *cepext;
    stat_t *st;

    E_INFO("Processing: %s\n", uttid);

    kb = (kb_t *) data;
    kbcore = kb->kbcore;
    kb_set_uttid(uttid, ur->uttfile, kb);
    st = kb->stat;

    cepdir = cmd_ln_str("-cepdir");
    cepext = cmd_ln_str("-cepext");

    if ((nfr =
	 feat_s2mfc2feat(kbcore_fcb(kbcore), ur->uttfile,
			 cepdir, cepext, sf, ef, kb->feat,
			 S3_MAX_FRAMES)) < 0) {
        E_FATAL("Cannot read file %s. Forced exit\n", ur->uttfile);
    }

    if (ur->lmname != NULL)
        srch_set_lm((srch_t *) kb->srch, ur->lmname);
    if (ur->regmatname != NULL)
        kb_setmllr(ur->regmatname, ur->cb2mllrname, kb);

    allphone_utt(kb, nfr, uttid);
}

int
main(int32 argc, char *argv[])
{
    kb_t kb;
    stat_t *st;

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", defn);
    unlimit();

    kb_init(&kb);
    st = kb.stat;
    fprintf(stdout, "\n");
    allphone_init(&kb);

    if (cmd_ln_str("-ctl")) {
        /* When -ctlfile is speicified, corpus.c will look at -ctl_lm and
	   -ctl_mllr to get the corresponding LM and MLLR for the utterance */
        st->tm = ctl_process(cmd_ln_str("-ctl"),
                             cmd_ln_str("-ctl_lm"),
                             cmd_ln_str("-ctl_mllr"),
                             cmd_ln_int32("-ctloffset"),
                             cmd_ln_int32("-ctlcount"), utt_allphone, &kb);
    }
    else if (cmd_ln_str("-utt")) {
        /* When -utt is specified, corpus.c will wait for the utterance to
	   change */
        st->tm = ctl_process_utt(cmd_ln_str("-utt"),
                                 cmd_ln_int32("-ctlcount"),
                                 utt_allphone, &kb);

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
    system("ps aguxwww | grep s3allphone");
#endif

    cmd_ln_appl_exit();
    return 0;
}
