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
 * whmm.c -- hmm structure that is used by sphinx 3.0 decode_anytopo (and perhaps
 * the fsg search as well)
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 14-Jul-05    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity 
 *              First created it. 
 *
 * $Log$
 * Revision 1.2  2006/02/23  05:07:53  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: split whmm's routine.
 * 
 * Revision 1.1.2.4  2005/09/07 23:40:06  arthchan2003
 * Several Bug Fixes and Enhancements to the flat-lexicon
 * 1, Fixed Dox-doc.
 * 2, Add -worddumpef and -hmmdumpef in parrallel to -worddumpsf and
 * -hmmdumpsf. Usage is trivial. a structure called fwd_dbg_t now wrapped
 * up all these loose parameters.  Methods of fwd_dbg are implemented.
 * 3, word_ugprob is now initialized by init_word_ugprob
 * 4, Full-triphone expansion is implemented. User can change this
 * behavior by specifying -multiplex_multi and -multiplex_single. The
 * former turn on multiplex triphone for word-begin for multi-phone word.
 * The latter do that for single-phone word. Turning off both could
 * tremendously increase computation.
 * 5, Word expansions of possible right contexts now records independent
 * history.  The behavior in the past was to use only one history for a
 * word.
 *
 * Revision 1.1.2.3  2005/07/24 01:42:58  arthchan2003
 * Added whmm_alloc_light, that will by-pass and not use any internal list inside whmm.c
 *
 * Revision 1.1.2.2  2005/07/17 05:57:25  arthchan2003
 * 1, Removed wid from the argument list of eval_*_whmm, 2, Allow  allocation of whmm_alloc to be more flexible.
 *
 * Revision 1.1.2.1  2005/07/15 07:48:32  arthchan2003
 * split the hmm (whmm_t) and context building process (ctxt_table_t) from the the flat_fwd.c
 *
 *
 */

#include <whmm.h>

void
whmm_free(whmm_t * h)
{
    hmm_deinit((hmm_t *)h);
    ckd_free(h);
}


whmm_t *
whmm_alloc(hmm_context_t *ctx, int32 pos, int mpx, s3ssid_t ssid, s3tmatid_t tmatid)
{
    whmm_t *h;

    h = ckd_calloc(1, sizeof(*h));
    hmm_init(ctx, (hmm_t *)h, mpx, ssid, tmatid);
    h->pos = pos;
    return h;
}

void
dump_whmm(s3wid_t w, whmm_t * h, int32 * senscr, tmat_t * tmat,
          int32 n_frame, dict_t * dict, mdef_t * mdef)
{
    printf("[%4d]", n_frame);
    printf(" [%s]", dict->word[w].word);

    printf(" ci= %s, pos= %d, lc=%d, rc= %d, bestscore= %d multiplex %s\n",
           mdef_ciphone_str(mdef, hmm_tmatid(h)),
	   h->pos, h->lc, h->rc, hmm_bestscore(h),
	   hmm_is_mpx(h) ? "yes" : "no");
    hmm_dump((hmm_t *)h, stdout);
}
