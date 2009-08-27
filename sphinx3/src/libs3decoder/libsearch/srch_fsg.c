/* -*- c-basic-offset:4; indent-tabs-mode: nil -*- */
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

/* srch_fsm.c
 * HISTORY
 * 
 * $Log$
 * Revision 1.3  2006/02/28  02:06:46  egouvea
 * Updated MS Visual C++ 6.0 support files. Fixed things that didn't
 * compile in Visual C++ (declarations didn't match, etc). There are
 * still some warnings, so this is not final. Also, sorted files in
 * several Makefile.am.
 * 
 * Revision 1.2  2006/02/23 05:10:18  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Adaptation of Sphinx 2's FSG search into Sphinx 3
 *
 * Revision 1.1.2.8  2006/01/16 20:12:42  arthchan2003
 * Interfaces for the 2nd-stage search. Now commented.
 *
 * Revision 1.1.2.7  2005/09/18 01:45:43  arthchan2003
 * Clean up the interfaces for srch_fsg.[ch]
 *
 * Revision 1.1.2.6  2005/07/26 02:20:39  arthchan2003
 * merged hyp_t with srch_hyp_t.
 *
 * Revision 1.1.2.5  2005/07/24 01:41:18  arthchan2003
 * Filled in all function pointers for mode 2
 *
 * Revision 1.1.2.4  2005/07/20 21:18:30  arthchan2003
 * FSG can now be read, srch_fsg_init can now be initialized, psubtree can be built. Sounds like it is time to plug in other function pointers.
 *
 * Revision 1.1.2.3  2005/07/13 18:41:01  arthchan2003
 * minor changes of srch_fsg.c to make it conform with hacks in fsg_*
 *
 * Revision 1.1.2.2  2005/06/28 07:01:21  arthchan2003
 * General fix of fsg routines to make a prototype of fsg_init and fsg_read. Not completed.  The number of empty functions in fsg_search is now decreased from 35 to 30.
 *
 * Revision 1.1.2.1  2005/06/27 05:27:09  arthchan2003
 * Fixed added srch_fsg.[ch] and replace it by srch_fsm.c
 *
 */

#include "srch.h"
#include "gmm_wrap.h"
#include "srch_fsg.h"
#include "kb.h"
#include "kbcore.h"
#include "word_fsg.h"
#include "fsg_search.h"
#include <string.h>

static word_fsg_t *srch_FSG_read_fsgfile(void *srch, const char *fsgfilename);


int
srch_FSG_init(kb_t * kb,    /**< The KB */
              void *srch     /**< The pointer to a search structure */
    )
{
    srch_t *s = (srch_t *)srch;
    fsg_search_t *fsgsrch;
    word_fsg_t *wordfsg;

    /* This is very strange */
    fsgsrch = fsg_search_init(NULL, s);

    s->grh->graph_struct = fsgsrch;
    s->grh->graph_type = GRAPH_STRUCT_GENGRAPH;

    if ((wordfsg = srch_FSG_read_fsgfile(s,
                                         cmd_ln_str_r(kbcore_config(s->kbc), "-fsg"))) == NULL) {
        E_INFO("Could not read wordfsg with file name %s\n",
               cmd_ln_str_r(kbcore_config(s->kbc), "-fsg"));
        return SRCH_FAILURE;
    }

    if (!fsg_search_set_current_fsg(fsgsrch, wordfsg->name)) {
        E_INFO("Could not set the current fsg with name %s\n",
               wordfsg->name);
        return SRCH_FAILURE;
    }

    return SRCH_SUCCESS;
}

static word_fsg_t *
srch_FSG_read_fsgfile(void *srch, const char *fsgfilename)
{
    word_fsg_t *fsg;
    srch_t *s;
    fsg_search_t *fsgsrch;

    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    fsg = word_fsg_readfile(fsgfilename,
                            cmd_ln_int32_r(kbcore_config(s->kbc), "-fsgusealtpron"),
                            cmd_ln_int32_r(kbcore_config(s->kbc), "-fsgusefiller"),
			    s->kbc);

    if (!fsg) {
        E_INFO("Fail to read fsg from file name %s\n", fsgfilename);
        return NULL;
    }

    if (!fsg_search_add_fsg(fsgsrch, fsg)) {
        E_ERROR("Failed to add FSG '%s' to system\n", word_fsg_name(fsg));
        word_fsg_free(fsg);
        return NULL;
    }
    return fsg;

}

int
srch_FSG_uninit(void *srch)
{
    srch_t *s;
    fsg_search_t *fsgsrch;
    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    fsg_search_free(fsgsrch);

    return SRCH_SUCCESS;
}

int
srch_FSG_begin(void *srch)
{
    srch_t *s;
    fsg_search_t *fsgsrch;
    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    fsg_search_utt_start(fsgsrch);

    return SRCH_SUCCESS;
}

int
srch_FSG_end(void *srch)
{
    srch_t *s;
    fsg_search_t *fsgsrch;
    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    fsgsrch->senscale = s->ascale;
    fsg_search_utt_end(fsgsrch);

    return SRCH_SUCCESS;

}

int
srch_FSG_srch_one_frame_lv2(void *srch)
{
    srch_t *s;
    fsg_search_t *fsgsrch;
    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;
    fsgsrch->uttid = s->uttid;

    fsg_search_frame_fwd(fsgsrch);

    return SRCH_SUCCESS;

}

/* This should be removed. It is currently to make the checker happy.*/
int
srch_FSG_set_lm(void *srch, const char *lmname)
{
    return SRCH_SUCCESS;

}

int
srch_FSG_add_lm(void *srch, lm_t * lm, const char *lmname)
{
    return SRCH_SUCCESS;

}

int
srch_FSG_delete_lm(void *srch, const char *lmname)
{
    return SRCH_SUCCESS;

}


int
srch_FSG_shift_one_cache_frame(void *srch, int32 win_efv)
{
    ascr_t *ascr;
    srch_t *s;

    s = (srch_t *) srch;

    ascr = s->ascr;

    ascr_shift_one_cache_frame(ascr, win_efv);

    return SRCH_SUCCESS;

}

int
srch_FSG_select_active_gmm(void *srch)
{
    srch_t *s;
    fsg_search_t *fsgsrch;
    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    fsg_search_sen_active(fsgsrch);
    return SRCH_SUCCESS;
}

int
srch_FSG_windup(void *srch, int32 frmno)
{
    return SRCH_SUCCESS;
}

int
srch_FSG_dump_vithist(void *srch)
{
    FILE *latfp;
    char file[8192];
    srch_t *s;
    fsg_search_t *fsgsrch;

    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    sprintf(file, "%s/%s.hist",
            cmd_ln_str_r(kbcore_config(s->kbc), "-bptbldir"),
            fsgsrch->uttid);
    if ((latfp = fopen(file, "w")) == NULL)
        E_ERROR("fopen(%s,w) failed\n", file);
    else {
        fsg_history_dump(fsgsrch->history, fsgsrch->uttid, latfp,
                         fsgsrch->dict);
        fclose(latfp);
    }

    return SRCH_SUCCESS;
}

glist_t
srch_FSG_gen_hyp(void *srch           /**< a pointer of srch_t */
    )
{
    srch_t *s;
    fsg_search_t *fsgsrch;
    srch_hyp_t *h;
    glist_t ghyp, rhyp;

    s = (srch_t *) srch;
    fsgsrch = (fsg_search_t *) s->grh->graph_struct;

    fsg_search_history_backtrace(fsgsrch, TRUE);

    ghyp = NULL;
    for (h = fsgsrch->hyp; h; h = h->next) {
	srch_hyp_t *tmph;

	/* Skip NULL states */	
	if (h->id < 0)
	    continue;
	
	/* We have to copy the nodes here since fsgsrch retains
	 * ownership of the hyp... */
	tmph = ckd_calloc(1, sizeof(*tmph));
	memcpy(tmph, h, sizeof(*tmph));
	tmph->next = NULL;
        ghyp = glist_add_ptr(ghyp, (void *) tmph);
    }

    rhyp = glist_reverse(ghyp);

    return rhyp;
}

/* Pointers to all functions */
srch_funcs_t srch_FSG_funcs = {
	/* init */			srch_FSG_init,
	/* uninit */			srch_FSG_uninit,
	/* utt_begin */ 		srch_FSG_begin,
	/* utt_end */   		srch_FSG_end,
	/* decode */			NULL,
	/* set_lm */			srch_FSG_set_lm,
	/* add_lm */			srch_FSG_add_lm,
	/* delete_lm */ 		srch_FSG_delete_lm,

	/* gmm_compute_lv1 */		approx_ci_gmm_compute,
	/* one_srch_frame_lv1 */	NULL,
	/* hmm_compute_lv1 */		srch_debug_hmm_compute_lv1,
	/* eval_beams_lv1 */		srch_debug_eval_beams_lv1,
	/* propagate_graph_ph_lv1 */	srch_debug_propagate_graph_ph_lv1,
	/* propagate_graph_wd_lv1 */	srch_debug_propagate_graph_wd_lv1,

	/* gmm_compute_lv2 */		s3_cd_gmm_compute_sen,
	/* one_srch_frame_lv2 */	srch_FSG_srch_one_frame_lv2,
	/* hmm_compute_lv2 */		NULL,
	/* eval_beams_lv2 */		NULL,
	/* propagate_graph_ph_lv2 */	NULL,
	/* propagate_graph_wd_lv2 */	NULL,

	/* rescoring */			NULL,
	/* frame_windup */		srch_FSG_windup,
	/* compute_heuristic */		NULL,
	/* shift_one_cache_frame */	srch_FSG_shift_one_cache_frame,
	/* select_active_gmm */		srch_FSG_select_active_gmm,

	/* gen_hyp */			srch_FSG_gen_hyp,
	/* gen_dag */			NULL,
	/* dump_vithist */		NULL,
	/* bestpath_impl */		NULL,
	/* dag_dump */			NULL,
        /* nbest_impl */                NULL,
	NULL
};
