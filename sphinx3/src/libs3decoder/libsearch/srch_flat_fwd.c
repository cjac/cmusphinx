/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/* srch_flat_fwd.c
 * HISTORYg
 * 
 * $Log$
 * Revision 1.4  2006/02/24  12:42:43  arthchan2003
 * Removed warnings in srch_flat_fwd.c and lextree.c
 * 
 * Revision 1.3  2006/02/24 12:33:56  arthchan2003
 * Removed ls from line 634 of srch_flat_fwd.c. Code compile.
 *
 * Revision 1.2  2006/02/23 05:16:14  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Add wrapper of flat_fwd.c
 *
 * Revision 1.1.2.8  2006/02/17 19:32:08  arthchan2003
 * Use specific version flat_fwd_dag_add_fudge_edge instead dag_add_fudge_edge.
 *
 * Revision 1.1.2.7  2006/01/16 20:11:23  arthchan2003
 * Interfaces for 2nd stage search, now commented.
 *
 * Revision 1.1.2.6  2005/11/17 06:42:15  arthchan2003
 * Added back crossword triphone traversing timing for search. Also. for consistency with srch.c.  Some dummy code of IBM lattice conversion was added. They are now bypassed because it is not fully function.
 *
 * Revision 1.1.2.5  2005/10/26 03:53:12  arthchan2003
 * Put add_fudge and remove_filler_nodes into srch_flat_fwd.c . This conformed to s3.0 behavior.
 *
 * Revision 1.1.2.4  2005/10/09 20:00:45  arthchan2003
 * Added back match file logging in mode 3. Safe-guard the code from using LM switching in mode 3 and mode 5.
 *
 * Revision 1.1.2.3  2005/09/27 07:41:40  arthchan2003
 * Not trying to free hyp. But correctly free the context table.
 *
 * Revision 1.1.2.2  2005/09/18 01:45:19  arthchan2003
 * Filled in all implementation in srch_flat_fwd.[ch], like the FSG mode, it takes care of reporting itselft.
 *
 * Revision 1.1.2.1  2005/07/24 01:40:37  arthchan2003
 * (Incomplete) The implementation of flat-lexicon decoding.
 *
 *
 *
 */

#include <sphinxbase/pio.h>

#include "srch_flat_fwd.h"
#include "srch_flat_fwd_internal.h"
#include "gmm_wrap.h"
#include "srch.h"
#include "astar.h"
#include "whmm.h"
#include "corpus.h"
#include "logs3.h"

static void
fwd_timing_dump(srch_FLAT_FWD_graph_t * fwg)
{
    E_INFO("[H %6.2fx ]", fwg->tm_hmmeval.t_cpu * 100.0 / fwg->n_frm);
    E_INFOCONT("[XH %6.2fx]", fwg->tm_hmmtrans.t_cpu * 100.0 / fwg->n_frm);
    E_INFOCONT("[XW %6.2fx]\n",
               fwg->tm_wdtrans.t_cpu * 100.0 / fwg->n_frm);
    E_INFOCONT("[mpx %d][~mpx %d]", fwg->ctr_mpx_whmm->count, fwg->ctr_nonmpx_whmm->count);
}


static fwd_dbg_t *
init_fwd_dbg(srch_FLAT_FWD_graph_t * fwg)
{
    const char *tmpstr;
    fwd_dbg_t *fd;

    fd = (fwd_dbg_t *) ckd_calloc(1, sizeof(fwd_dbg_t));

    assert(fd);
    /* Word to be traced in detail */
    if ((tmpstr = cmd_ln_str_r(kbcore_config(fwg->kbcore), "-tracewhmm")) != NULL) {
        fd->trace_wid = dict_wordid(fwg->kbcore->dict, tmpstr);
        if (NOT_S3WID(fd->trace_wid))
            E_ERROR("%s not in dictionary; cannot be traced\n", tmpstr);
    }
    else
        fd->trace_wid = BAD_S3WID;

    /* Active words to be dumped for debugging after and before the given frame nos, if any */
    fd->word_dump_sf = (int32) 0x7ffffff0;
    if (cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-worddumpsf"))
        fd->word_dump_sf = cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-worddumpsf");

    fd->word_dump_ef = (int32) 0x7ffffff0;
    if (cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-worddumpef"))
        fd->word_dump_ef = cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-worddumpef");

    /* Active HMMs to be dumped for debugging after and before the given frame nos, if any */
    fd->hmm_dump_sf = (int32) 0x7ffffff0;
    if (cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-hmmdumpsf"))
        fd->hmm_dump_sf = cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-hmmdumpsf");

    fd->hmm_dump_ef = (int32) 0x7ffffff0;
    if (cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-hmmdumpef"))
        fd->hmm_dump_ef = cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-hmmdumpef");

    return fd;
}

/** ARCHAN: Dangerous! Mixing global and local */
static void
dump_fwd_dbg_info(srch_FLAT_FWD_graph_t * fwg, fwd_dbg_t * fd,
                  ascr_t * ascr, int32 bestscr, int32 whmm_thresh,
                  int32 word_thresh, int32 * senscr)
{
    whmm_t *h;
    int32 n_frm;
    kbcore_t *kbc;
    tmat_t *tmat;
    dict_t *dict;
    mdef_t *mdef;

    n_frm = fwg->n_frm;
    kbc = fwg->kbcore;
    dict = kbcore_dict(kbc);
    tmat = kbcore_tmat(kbc);
    mdef = kbcore_mdef(kbc);

    /* Dump bestscore and pruning thresholds if any detailed tracing specified */
    if (((fd->hmm_dump_sf < n_frm) && (n_frm < fd->hmm_dump_ef)) ||
        ((fd->word_dump_sf < n_frm) && (n_frm < fd->word_dump_ef)) ||
        (IS_S3WID(fd->trace_wid) && fwg->whmm[fd->trace_wid])) {
        printf
            ("[%4d]: >>>> bestscore= %11d, whmm-thresh= %11d, word-thresh= %11d\n",
             n_frm, bestscr, whmm_thresh, word_thresh);
    }

    /* Dump all active HMMs or words, if indicated */
    if (fd->hmm_dump_sf < n_frm && n_frm < fd->hmm_dump_ef)
        dump_all_whmm(fwg, fwg->whmm, n_frm, ascr->senscr);
    else if (fd->word_dump_sf < n_frm && n_frm < fd->word_dump_ef)
        dump_all_word(fwg, fwg->whmm);

    /* Trace active HMMs for specified word, if any */
    if (IS_S3WID(fd->trace_wid)) {
        for (h = fwg->whmm[fd->trace_wid]; h; h = h->next)
            dump_whmm(fd->trace_wid, h, senscr, tmat, n_frm, dict,
                      mdef);
    }

}


int
srch_FLAT_FWD_init(kb_t * kb,    /**< The KB */
                   void *srch     /**< The pointer to a search structure */
    )
{
    srch_FLAT_FWD_graph_t *fwg;
    kbcore_t *kbc;
    srch_t *s;
    mdef_t *mdef;
    dict_t *dict;
    lm_t *lm;

    kbc = kb->kbcore;
    s = (srch_t *) srch;
    mdef = kbcore_mdef(kbc);
    dict = kbcore_dict(kbc);
    lm = kbcore_lm(kbc);


    fwg = ckd_calloc(1, sizeof(srch_FLAT_FWD_graph_t));

    E_INFO("Initialization\n");

    /** For convenience */
    fwg->kbcore = s->kbc;

    /* Allocate whmm structure */
    fwg->hmmctx = hmm_context_init(mdef_n_emit_state(mdef),
				   kbcore_tmat(kbc)->tp, NULL,
				   mdef->sseq);
    fwg->whmm = (whmm_t **) ckd_calloc(dict->n_word, sizeof(whmm_t *));

    /* Data structures needed during word transition */
    /* These five things need to be tied into the same structure.  Such that when multiple LM they could be switched.  */
    fwg->rcscore = NULL;
    fwg->rcscore = (int32 *) ckd_calloc(mdef->n_ciphone, sizeof(int32));
    fwg->ug_backoff =
        (backoff_t *) ckd_calloc(mdef->n_ciphone, sizeof(backoff_t));
    fwg->filler_backoff =
        (backoff_t *) ckd_calloc(mdef->n_ciphone, sizeof(backoff_t));
    fwg->tg_trans_done = (uint8 *) ckd_calloc(dict->n_word, sizeof(uint8));
    fwg->word_ugprob = init_word_ugprob(mdef, lm, dict);

    /* Input candidate-word lattices information to restrict search; if any */
    fwg->word_cand_dir = cmd_ln_str_r(kbcore_config(fwg->kbcore), "-inlatdir");
    fwg->latfile_ext = cmd_ln_str_r(kbcore_config(fwg->kbcore), "-latext");
    fwg->word_cand_win = cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-inlatwin");
    if (fwg->word_cand_win < 0) {
        E_ERROR("Invalid -inlatwin argument: %d; set to 50\n",
                fwg->word_cand_win);
        fwg->word_cand_win = 50;
    }
    /* Allocate pointers to lists of word candidates in each frame */
    if (fwg->word_cand_dir) {
        fwg->word_cand =
            (word_cand_t **) ckd_calloc(S3_MAX_FRAMES,
                                        sizeof(word_cand_t *));
        fwg->word_cand_cf =
            (s3wid_t *) ckd_calloc(dict->n_word + 1, sizeof(s3wid_t));
    }


    /* Initializing debugging information such as trace_wid,
       word_dump_sf, word_dump_ef, hmm_dump_sf and hmm_dump_ef */

    fwg->fwdDBG = init_fwd_dbg(fwg);

    fwg->ctr_mpx_whmm = pctr_new("mpx");
    fwg->ctr_nonmpx_whmm = pctr_new("~mpx");
    fwg->ctr_latentry = pctr_new("lat");

    /** Initialize the context table */
    fwg->ctxt = ctxt_table_init(kbcore_dict(kbc), kbcore_mdef(kbc));

    /* Viterbi history structure */
    fwg->lathist = latticehist_init(cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-bptblsize"),
				   S3_MAX_FRAMES + 1);

    /* Glue the graph structure */
    s->grh->graph_struct = fwg;
    s->grh->graph_type = GRAPH_STRUCT_FLAT;



    return SRCH_SUCCESS;

}

int
srch_FLAT_FWD_uninit(void *srch)
{

    srch_FLAT_FWD_graph_t *fwg;
    srch_t *s;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;

    if (fwg->rcscore)
        ckd_free(fwg->rcscore);

    if (fwg->ug_backoff)
        ckd_free(fwg->ug_backoff);

    if (fwg->filler_backoff)
        ckd_free(fwg->filler_backoff);

    if (fwg->tg_trans_done)
        ckd_free(fwg->tg_trans_done);

    if (fwg->word_cand_cf)
        ckd_free(fwg->word_cand_cf);

    if (fwg->word_cand)
        ckd_free(fwg->word_cand);

    if (fwg->ctxt)
        ctxt_table_free(fwg->ctxt);

    if (fwg->lathist)
	latticehist_free(fwg->lathist);

    if (fwg->fwdDBG)
	ckd_free(fwg->fwdDBG);

    if (fwg->whmm)
	ckd_free(fwg->whmm);

    if (fwg->hmmctx)
	hmm_context_free(fwg->hmmctx);

    if (fwg->word_ugprob)
	word_ugprob_free(fwg->word_ugprob, kbcore_mdef(s->kbc)->n_ciphone);

    pctr_free(fwg->ctr_mpx_whmm);
    pctr_free(fwg->ctr_nonmpx_whmm);
    pctr_free(fwg->ctr_latentry);

    ckd_free(fwg);

    return SRCH_SUCCESS;
}

int
srch_FLAT_FWD_begin(void *srch)
{
    srch_FLAT_FWD_graph_t *fwg;
    srch_t *s;
    kbcore_t *kbc;
    int32 w, ispipe;
    char str[1024];
    FILE *fp;
    dict_t *dict;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;
    kbc = s->kbc;
    dict = kbcore_dict(kbc);

    assert(fwg);

    ptmr_reset(&(fwg->tm_hmmeval));
    ptmr_reset(&(fwg->tm_hmmtrans));
    ptmr_reset(&(fwg->tm_wdtrans));

    latticehist_reset(fwg->lathist);

    /* If input lattice file containing word candidates to be searched specified; use it */
    if (fwg->word_cand_dir) {
        ctl_outfile(str, fwg->word_cand_dir, fwg->latfile_ext,
                    (s->uttfile ? s->uttfile : s->uttid), s->uttid,
                    cmd_ln_boolean_r(kbcore_config(s->kbc), "-build_outdirs"));
        E_INFO("Reading input lattice: %s\n", str);

        if ((fp = fopen_compchk(str, &ispipe)) == NULL)
            E_ERROR("fopen_compchk(%s) failed; running full search\n",
                    str);
        else {
            if ((fwg->n_word_cand =
                 word_cand_load(fp, fwg->word_cand, dict,
                                s->uttid)) <= 0) {
                E_ERROR("Bad or empty lattice file: %s; ignored\n", str);
                word_cand_free(fwg->word_cand);
                fwg->n_word_cand = 0;
            }
            else
                E_INFO("%d lattice entries read\n", fwg->n_word_cand);

            fclose_comp(fp, ispipe);
        }
    }

    if (fwg->n_word_cand > 0)
        latticehist_n_cand(fwg->lathist) = fwg->n_word_cand;

    /* Enter all pronunciations of startwid (begin silence) */


    fwg->n_frm = -1;
    for (w = dict->startwid; IS_S3WID(w); w = dict->word[w].alt)
        word_enter(fwg, w, 0, BAD_S3LATID,
                   dict->word[dict->silwid].ciphone[dict->
                                                    word[dict->silwid].
                                                    pronlen - 1]);

    fwg->renormalized = 0;
    fwg->n_frm = 0;

#if 0
    E_INFO("After\n");

    dump_all_whmm(fwg, fwg->whmm, fwg->n_frm, fwg->n_state, NULL);
#endif

    return SRCH_SUCCESS;

}

int
srch_FLAT_FWD_end(void *srch)
{
    srch_FLAT_FWD_graph_t *fwg;
    srch_t *s;
    kbcore_t *kbc;
    dict_t *dict;
    stat_t *st;

    whmm_t *h, *nexth;
    s3wid_t w;
    lm_t *lm;


    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;
    kbc = s->kbc;
    dict = kbcore_dict(kbc);
    st = s->stat;

    lm = s->kbc->lmset->cur_lm;

    fwg->lathist->frm_latstart[fwg->n_frm] = fwg->lathist->n_lat_entry;     /* Add sentinel */
    pctr_increment(fwg->ctr_latentry, fwg->lathist->n_lat_entry);

    /* Free whmm search structures */
    for (w = 0; w < dict->n_word; w++) {
        for (h = fwg->whmm[w]; h; h = nexth) {
            nexth = h->next;
            whmm_free(h);
        }
        fwg->whmm[w] = NULL;
    }

    if (fwg->n_word_cand > 0) {
        word_cand_free(fwg->word_cand);
        fwg->n_word_cand = 0;
    }

    lm_cache_stats_dump(lm);
    lm_cache_reset(lm);

    fwd_timing_dump(fwg);

    return SRCH_SUCCESS;
}

int
srch_FLAT_FWD_set_lm(void *srch_struct, const char *lmname)
{
    E_INFO("In Mode 3, currently the function set LM is not supported\n");
    return SRCH_FAILURE;
}

int
srch_FLAT_FWD_add_lm(void *srch, lm_t * lm, const char *lmname)
{
    E_INFO("In Mode 3, currently the function add LM is not supported\n");
    return SRCH_FAILURE;

}

int
srch_FLAT_FWD_delete_lm(void *srch, const char *lmname)
{
    E_INFO
        ("In Mode 3, currently the function delete LM is not supported\n");
    return SRCH_FAILURE;
}


int
srch_FLAT_FWD_srch_one_frame_lv2(void *srch)
{
    int32 bestscr;              /* Best state score for any whmm evaluated in this frame */
    int32 whmm_thresh;          /* Threshold for any whmm to stay alive in search */
    int32 word_thresh;          /* Threshold for a word-final whmm to succeed */
    int32 phone_penalty;
    srch_FLAT_FWD_graph_t *fwg;
    srch_t *s;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;

    ptmr_start(&(fwg->tm_hmmeval));
    bestscr = whmm_eval(fwg, s->ascr->senscr);
    /*  E_INFO("bestscr %d RENORM_THRESH %d\n",bestscr, RENORM_THRESH); */
    ptmr_stop(&(fwg->tm_hmmeval));

    whmm_thresh = bestscr + s->beam->hmm;
    word_thresh = bestscr + s->beam->word;
    phone_penalty = logs3(kbcore_logmath(s->kbc), cmd_ln_float32_r(kbcore_config(fwg->kbcore), "-phonepen"));

    assert(s->ascr->senscr);
    /*  E_INFO("fwg->n_frm %d\n",fwg->n_frm); */
    dump_fwd_dbg_info(fwg, fwg->fwdDBG, s->ascr, bestscr, whmm_thresh,
                      word_thresh, s->ascr->senscr);

    {
        ptmr_start(&(fwg->tm_hmmtrans));
        fwg->lathist->frm_latstart[fwg->n_frm] = fwg->lathist->n_lat_entry;
        whmm_exit(fwg, fwg->whmm, fwg->lathist,
		  whmm_thresh, word_thresh,
                  phone_penalty);
        ptmr_stop(&(fwg->tm_hmmtrans));

        /* Please read, the In whmm_exit, if word ends are reach,
           n_lat_entry will increase, see whmm_exit(). Then word_trans
           will be triggered.
         */

        ptmr_start(&(fwg->tm_wdtrans));
        if (fwg->lathist->frm_latstart[fwg->n_frm] < fwg->lathist->n_lat_entry)
            word_trans(fwg, fwg->whmm, fwg->lathist, whmm_thresh, phone_penalty);
        ptmr_stop(&(fwg->tm_wdtrans));
    }

    if (bestscr < RENORM_THRESH) {
        E_INFO("Frame %d: bestscore= %d; renormalizing\n", fwg->n_frm,
               bestscr);
        whmm_renorm(fwg, fwg->whmm, bestscr);
    }

    fwg->lathist->n_frm++;
    fwg->n_frm++;
    return SRCH_SUCCESS;
}


int
srch_FLAT_FWD_shift_one_cache_frame(void *srch, int32 win_efv)
{
    ascr_t *ascr;
    srch_t *s;

    s = (srch_t *) srch;

    ascr = s->ascr;

    ascr_shift_one_cache_frame(ascr, win_efv);

    return SRCH_SUCCESS;
}

int
srch_FLAT_FWD_select_active_gmm(void *srch)
{
    kbcore_t *kbc;
    s3wid_t w;
    whmm_t *h;
    int32 st;
    s3pid_t p;
    s3senid_t *senp;
    ascr_t *ascr;
    srch_FLAT_FWD_graph_t *fwg;
    srch_t *s;
    mdef_t *mdef;
    dict_t *dict;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;
    ascr = s->ascr;
    kbc = s->kbc;
    mdef = kbcore_mdef(kbc);
    dict = kbcore_dict(kbc);

    ascr_clear_sen_active(ascr);

    /* Flag active senones */
    for (w = 0; w < dict->n_word; w++) {
        for (h = fwg->whmm[w]; h; h = h->next) {
            if (hmm_is_mpx(h)) {
		for (st = 0; st < hmm_n_emit_state(h); ++st) {
		    p = hmm_mpx_ssid(h, st);
		    if (p == -1)
			break; /* All subsequent ones are also inactive */
                    senp = mdef->sseq[p];
                    ascr->sen_active[senp[st]] = 1;
                }
            }
            else {
                p = hmm_nonmpx_ssid(h);
                senp = mdef->sseq[p];
                for (st = 0; st < hmm_n_emit_state(h); ++st)
                    ascr->sen_active[senp[st]] = 1;

            }
        }
    }

    return SRCH_SUCCESS;
}

int
srch_FLAT_FWD_frame_windup(void *srch_struct, int32 frmno)
{
    return SRCH_SUCCESS;
}

glist_t
srch_FLAT_FWD_gen_hyp(void *srch           /**< a pointer of srch_t */
    )
{
    srch_t *s;
    srch_FLAT_FWD_graph_t *fwg;
    srch_hyp_t *tmph, *hyp;
    glist_t ghyp, rhyp;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;

    if (s->exit_id == -1)
	    s->exit_id = lat_final_entry(fwg->lathist, kbcore_dict(s->kbc), fwg->n_frm,
					 s->uttid);
    if (NOT_S3LATID(s->exit_id)) {
        E_INFO("lattice ID: %d\n", s->exit_id);
        E_ERROR("%s: NO RECOGNITION\n", s->uttid);
        return NULL;
    }
    else {
        /* BAD_S3WID => Any right context */
        lattice_backtrace(fwg->lathist, s->exit_id, BAD_S3WID, &hyp,
                          s->kbc->lmset->cur_lm, kbcore_dict(s->kbc),
                          fwg->ctxt, s->kbc->fillpen);
        ghyp = NULL;
        for (tmph = hyp; tmph; tmph = tmph->next) {
            ghyp = glist_add_ptr(ghyp, (void *) tmph);
        }

        rhyp = glist_reverse(ghyp);
        return rhyp;
    }

}

int
srch_FLAT_FWD_dump_vithist(void *srch)
{
    srch_t *s;
    srch_FLAT_FWD_graph_t *fwg;
    char file[8192];
    FILE *bptfp;

    /* Check if bptable should be dumped (for debugging) */

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;

    assert(fwg->lathist);

    sprintf(file, "%s/%s.bpt", cmd_ln_str_r(kbcore_config(fwg->kbcore), "-bptbldir"), s->uttid);
    if ((bptfp = fopen(file, "w")) == NULL) {
        E_ERROR("fopen(%s,w) failed; using stdout\n", file);
        bptfp = stdout;
    }

    latticehist_dump(fwg->lathist, bptfp, kbcore_dict(s->kbc), fwg->ctxt, 0);

    if (bptfp != stdout)
        fclose(bptfp);
    return SRCH_SUCCESS;
}



dag_t *
srch_FLAT_FWD_gen_dag(void *srch,         /**< a pointer of srch_t */
                      glist_t hyp)
{
    srch_t *s;
    srch_FLAT_FWD_graph_t *fwg;
    dag_t *dag;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;

    dag =
        latticehist_dag_build(fwg->lathist, hyp, kbcore_dict(s->kbc),
                              kbcore_lm(s->kbc), fwg->ctxt, s->kbc->fillpen,
                              s->exit_id, kbcore_config(s->kbc), kbcore_logmath(s->kbc));

    return dag;
}


glist_t
srch_FLAT_FWD_bestpath_impl(void *srch,           /**< A void pointer to a search structure */
                            dag_t * dag)
{
    srch_t *s;
    srch_FLAT_FWD_graph_t *fwg;

    float32 bestpathlw;
    float64 lwf;
    srch_hyp_t *tmph, *bph;
    glist_t ghyp, rhyp;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;

    assert(fwg->lathist);

    bestpathlw = cmd_ln_float32_r(kbcore_config(fwg->kbcore), "-bestpathlw");
    lwf = bestpathlw ? (bestpathlw / cmd_ln_float32_r(kbcore_config(fwg->kbcore), "-lw")) : 1.0;

    flat_fwd_dag_add_fudge_edges(fwg,
				 dag,
				 cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-dagfudge"),
				 cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-min_endfr"),
				 (void *) fwg->lathist, s->kbc->dict);


    /* Bypass filler nodes */
    if (!dag->filler_removed) {
        /* If Viterbi search terminated in filler word coerce final DAG node to FINISH_WORD */
        if (dict_filler_word(s->kbc->dict, dag->end->wid))
            dag->end->wid = s->kbc->dict->finishwid;

        if (dag_bypass_filler_nodes(dag, lwf, s->kbc->dict, s->kbc->fillpen) < 0)
            E_ERROR("maxedge limit (%d) exceeded\n", dag->maxedge);
        else
            dag->filler_removed = 1;
    }

    bph =
        dag_search(dag, s->uttid, lwf, dag->end,
                   s->kbc->dict, s->kbc->lmset->cur_lm, s->kbc->fillpen);

    if (bph != NULL) {
        ghyp = NULL;
        for (tmph = bph; tmph; tmph = tmph->next)
            ghyp = glist_add_ptr(ghyp, (void *) tmph);

        rhyp = glist_reverse(ghyp);
        return rhyp;
    }
    else {
        return NULL;
    }

}

int32
srch_FLAT_FWD_dag_dump(void *srch, dag_t *dag)
{
    char str[2048];
    srch_t *s;
    srch_FLAT_FWD_graph_t *fwg;

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;
    assert(fwg->lathist);

    ctl_outfile(str,
                cmd_ln_str_r(kbcore_config(fwg->kbcore), "-outlatdir"),
                cmd_ln_str_r(kbcore_config(fwg->kbcore), "-latext"),
                (s->uttfile ? s->uttfile : s->uttid), s->uttid,
                cmd_ln_boolean_r(kbcore_config(fwg->kbcore), "-build_outdirs"));
    E_INFO("Writing lattice file: %s\n", str);
    latticehist_dag_write(fwg->lathist,
                          str,
                          dag, kbcore_lm(s->kbc),
                          kbcore_dict(s->kbc), fwg->ctxt,
                          s->kbc->fillpen);

    return SRCH_SUCCESS;
}

glist_t
srch_FLAT_FWD_nbest_impl(void *srch,           /**< A void pointer to a search structure */
                         dag_t * dag)
{
    srch_t *s;
    srch_FLAT_FWD_graph_t *fwg;
    float32 bestpathlw;
    float64 lwf;
    char str[2000];

    s = (srch_t *) srch;
    fwg = (srch_FLAT_FWD_graph_t *) s->grh->graph_struct;
    assert(fwg->lathist);

    if (!(cmd_ln_exists_r(kbcore_config(fwg->kbcore), "-nbestdir")
          && cmd_ln_str_r(kbcore_config(fwg->kbcore), "-nbestdir")))
        return NULL;
    ctl_outfile(str, cmd_ln_str_r(kbcore_config(fwg->kbcore), "-nbestdir"),
                cmd_ln_str_r(kbcore_config(fwg->kbcore), "-nbestext"),
                (s->uttfile ? s->uttfile : s->uttid), s->uttid,
                cmd_ln_boolean_r(kbcore_config(fwg->kbcore), "-build_outdirs"));

    bestpathlw = cmd_ln_float32_r(kbcore_config(fwg->kbcore), "-bestpathlw");
    lwf = bestpathlw ? (bestpathlw / cmd_ln_float32_r(kbcore_config(fwg->kbcore), "-lw")) : 1.0;

    flat_fwd_dag_add_fudge_edges(fwg,
				 dag,
				 cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-dagfudge"),
				 cmd_ln_int32_r(kbcore_config(fwg->kbcore), "-min_endfr"),
				 (void *) fwg->lathist, s->kbc->dict);

    /* Bypass filler nodes */
    if (!dag->filler_removed) {
        /* If Viterbi search terminated in filler word coerce final DAG node to FINISH_WORD */
        if (dict_filler_word(s->kbc->dict, dag->end->wid))
            dag->end->wid = s->kbc->dict->finishwid;
        dag_remove_unreachable(dag);
        if (dag_bypass_filler_nodes(dag, lwf, s->kbc->dict, s->kbc->fillpen) < 0)
            E_ERROR("maxedge limit (%d) exceeded\n", dag->maxedge);
    }

    dag_compute_hscr(dag, kbcore_dict(s->kbc), kbcore_lm(s->kbc), lwf);
    dag_remove_bypass_links(dag);
    dag->filler_removed = 0;

    nbest_search(dag, str, s->uttid, lwf,
                 kbcore_dict(s->kbc),
                 kbcore_lm(s->kbc), kbcore_fillpen(s->kbc)
        );
    return NULL;
}

/* Pointers to all functions */
srch_funcs_t srch_FLAT_FWD_funcs = {
	/* init */			srch_FLAT_FWD_init,
	/* uninit */			srch_FLAT_FWD_uninit,
	/* utt_begin */ 		srch_FLAT_FWD_begin,
	/* utt_end */   		srch_FLAT_FWD_end,
	/* decode */			NULL,
	/* set_lm */			srch_FLAT_FWD_set_lm,
	/* add_lm */			srch_FLAT_FWD_add_lm,
	/* delete_lm */ 		srch_FLAT_FWD_delete_lm,

	/* gmm_compute_lv1 */		approx_ci_gmm_compute,
	/* one_srch_frame_lv1 */	NULL,
	/* hmm_compute_lv1 */		srch_debug_hmm_compute_lv1,
	/* eval_beams_lv1 */		srch_debug_eval_beams_lv1,
	/* propagate_graph_ph_lv1 */	srch_debug_propagate_graph_ph_lv1,
	/* propagate_graph_wd_lv1 */	srch_debug_propagate_graph_wd_lv1,

	/* gmm_compute_lv2 */		s3_cd_gmm_compute_sen,
	/* one_srch_frame_lv2 */	srch_FLAT_FWD_srch_one_frame_lv2,
	/* hmm_compute_lv2 */		NULL,
	/* eval_beams_lv2 */		NULL,
	/* propagate_graph_ph_lv2 */	NULL,
	/* propagate_graph_wd_lv2 */	NULL,

	/* rescoring */			NULL,
	/* frame_windup */		srch_FLAT_FWD_frame_windup,
	/* compute_heuristic */		NULL,
	/* shift_one_cache_frame */	srch_FLAT_FWD_shift_one_cache_frame,
	/* select_active_gmm */		srch_FLAT_FWD_select_active_gmm,

	/* gen_hyp */			srch_FLAT_FWD_gen_hyp,
	/* gen_dag */			srch_FLAT_FWD_gen_dag,
	/* dump_vithist */		srch_FLAT_FWD_dump_vithist,
	/* bestpath_impl */		srch_FLAT_FWD_bestpath_impl,
	/* dag_dump */			srch_FLAT_FWD_dag_dump,
        /* nbest_impl */                srch_FLAT_FWD_nbest_impl,
	NULL
};
