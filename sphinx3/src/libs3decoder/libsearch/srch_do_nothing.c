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

/* srch_do_nothing.c
 *
 * An empty search operator.  Its purpose is to mainly to test srch.c function
 * pointer code. 
 *
 * HISTORY
 * 
 */

#include "srch.h"

int
srch_do_nothing_init(kb_t * kb, void *srch)
{
    return SRCH_SUCCESS;
}

int
srch_do_nothing_uninit(void *srch)
{
    return SRCH_SUCCESS;
}

int
srch_do_nothing_begin(void *srch)
{
    return SRCH_SUCCESS;
}

int
srch_do_nothing_end(void *srch)
{
    return SRCH_SUCCESS;
}

int
srch_do_nothing_decode(void *srch_struct)
{
    return SRCH_SUCCESS;
}

/* Pointers to all functions */
srch_funcs_t srch_do_nothing_funcs = {
	/* init */			srch_do_nothing_init,
	/* uninit */			srch_do_nothing_uninit,
	/* utt_begin */ 		srch_do_nothing_begin,
	/* utt_end */   		srch_do_nothing_end,
	/* decode */			srch_do_nothing_decode,
	/* set_lm */			NULL,
	/* add_lm */			NULL,
	/* delete_lm */ 		NULL,

	/* gmm_compute_lv1 */		NULL,
	/* one_srch_frame_lv1 */	NULL,
	/* hmm_compute_lv1 */		NULL,
	/* eval_beams_lv1 */		NULL,
	/* propagate_graph_ph_lv1 */	NULL,
	/* propagate_graph_wd_lv1 */	NULL,

	/* gmm_compute_lv2 */		NULL,
	/* one_srch_frame_lv2 */	NULL,
	/* hmm_compute_lv2 */		NULL,
	/* eval_beams_lv2 */		NULL,
	/* propagate_graph_ph_lv2 */	NULL,
	/* propagate_graph_wd_lv2 */	NULL,

	/* rescoring */			NULL,
	/* frame_windup */		NULL,
	/* compute_heuristic */		NULL,
	/* shift_one_cache_frame */	NULL,
	/* select_active_gmm */		NULL,

	/* gen_hyp */			NULL,
	/* gen_dag */			NULL,
	/* dump_vithist */		NULL,
	/* bestpath_impl */		NULL,
	/* dag_dump */			NULL,
        /* nbest_impl */                NULL,
	NULL
};
