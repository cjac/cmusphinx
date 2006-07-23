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

/* srch_time_switch_tree.c
 * HISTORY
 * 
 * 17-Mar-2005 A. Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 *             Started. This replaced utt.c starting from Sphinx 3.6. 
 */

#include "srch.h"

int
srch_debug_init(kb_t * kb, void *srch)
{
    E_INFOCONT("SEARCH DEBUG: MODE INITIALIZATION \n");
    return SRCH_SUCCESS;
}

int
srch_debug_uninit(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: MODE UNINITIALIZATION \n");
    return SRCH_SUCCESS;
}

int
srch_debug_begin(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: MODE UTT BEGIN\n");
    return SRCH_SUCCESS;
}

int
srch_debug_end(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: MODE UTT END\n");
    return SRCH_SUCCESS;
}

int
srch_debug_decode()
{
    E_INFOCONT("SEARCH DEBUG: MODE UTT DECODE\n");
    return SRCH_SUCCESS;
}

int
srch_debug_set_lm(void *srch, const char *lmname)
{
    E_INFOCONT("SEARCH DEBUG: MODE SET LM\n");
    return SRCH_SUCCESS;
}

int
srch_debug_add_lm(void *srch, lm_t * lm, const char *lmname)
{
    E_INFOCONT("SEARCH DEBUG: MODE ADD LM\n");
    return SRCH_SUCCESS;
}

int
srch_debug_delete_lm(void *srch, const char *lmname)
{
    E_INFOCONT("SEARCH DEBUG: MODE DELETE LM\n");
    return SRCH_SUCCESS;
}

int
srch_debug_gmm_compute_lv1(void *srch, float32 * feat, int32 cache_idx,
                           int32 wav_idx)
{
    E_INFOCONT("SEARCH DEBUG: APPROXIMATE COMPUTATION AT TIME %d\n",
               wav_idx);

    return SRCH_SUCCESS;
}

int
srch_debug_gmm_compute_lv2(void *srch, float32 ** feat, int32 wav_idx)
{
    E_INFOCONT("SEARCH DEBUG: DETAIL COMPUTATION AT TIME %d\n", wav_idx);
    return SRCH_SUCCESS;
}

int
srch_debug_hmm_compute_lv1(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: HMM COMPUTE LV 1\n");
    return SRCH_SUCCESS;
}

int
srch_debug_hmm_compute_lv2(void *srch, int32 wav_idx)
{
    E_INFOCONT("SEARCH DEBUG: HMM COMPUTE LV 2\n");
    return SRCH_SUCCESS;
}

int
srch_debug_propagate_graph_ph_lv1(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: HMM PROPAGATE GRAPH (PHONEME) LV 1\n");
    return SRCH_SUCCESS;
}

int
srch_debug_propagate_graph_wd_lv1(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: HMM PROPAGATE GRAPH (WORD) LV 1\n");
    return SRCH_SUCCESS;
}

int
srch_debug_propagate_graph_ph_lv2(void *srch, int32 wav_idx)
{
    E_INFOCONT("SEARCH DEBUG: HMM PROPAGATE GRAPH (PHONEME) LV 2\n");
    return SRCH_SUCCESS;
}

int
srch_debug_propagate_graph_wd_lv2(void *srch, int32 wav_idx)
{
    E_INFOCONT("SEARCH DEBUG: HMM PROPAGATE GRAPH (WORD) LV 2\n");
    return SRCH_SUCCESS;
}

int
srch_debug_eval_beams_lv1(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: EVAL BEAMS LV1\n");
    return SRCH_SUCCESS;
}

int
srch_debug_eval_beams_lv2(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: EVAL BEAMS LV2\n");
    return SRCH_SUCCESS;
}

int
srch_debug_compute_heuristic(void *srch, int32 win_efv)
{
    E_INFOCONT("SEARCH DEBUG: COMPUTE HEURISTIC\n");
    return SRCH_SUCCESS;
}

int
srch_debug_frame_windup(void *srch_struct, int32 frmno)
{
    E_INFOCONT("SEARCH DEBUG: FRAME WINDUP\n");
    return SRCH_SUCCESS;
}

int
srch_debug_shift_one_cache_frame(void *srch, int32 win_efv)
{
    E_INFOCONT("SEARCH DEBUG: SHIFT ONE CACHE FRAME\n");
    return SRCH_SUCCESS;
}

int
srch_debug_select_active_gmm(void *srch)
{
    E_INFOCONT("SEARCH DEBUG: SELECT ACTIVE GMM\n");
    return SRCH_SUCCESS;
}

int
srch_debug_rescoring(void *srch, int32 frmno)
{
    E_INFOCONT("SEARCH DEBUG: RESCORING AT LV2\n");
    return SRCH_SUCCESS;
}

int
srch_debug_dump_vithist(void *srch     /**< A void pointer to a search structure */
    )
{
    E_INFOCONT("SEARCH DEBUG: DUMP VITHIST\n");
    return SRCH_SUCCESS;
}

glist_t
srch_debug_gen_hyp(void *srch           /**< a pointer of srch_t */
    )
{
    return NULL;
}

dag_t *
srch_debug_gen_dag(void *srch,         /**< a pointer of srch_t */
                   glist_t hyp)
{
    return NULL;
}

glist_t
srch_debug_bestpath_impl(void *srch,          /**< A void pointer to a search structure */
                         dag_t * dag)
{
    return SRCH_SUCCESS;
}

int32
srch_debug_dag_dump(void *srch, glist_t hyp)
{
    return SRCH_SUCCESS;
}
