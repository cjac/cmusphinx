/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 /*************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 *************************************************
 *
 * 31-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 * Created
 */

#include <libutil/libutil.h>
#include "kb.h"
#include "utt.h"
#include "cmd_ln_args.h"
#include "new_fe.h"		   /* 01.15.01 - RAH, use new_fe.h instead */
#include "live.h"

static fe_t  *fe;

static kb_t  *kb;
static kbcore_t *kbcore;
static FILE  *hmmdumpfp;
static int32 maxwpf;
static int32 maxhistpf;
static int32 maxhmmpf;
static int32 ptranskip;

static partialhyp_t *parthyp = NULL;

/* This routine initializes decoder variables for live mode decoding */
void live_initialize_decoder(char *live_args)
{
    static kb_t live_kb;
    int32   maxcepvecs, maxhyplen, samprate, ceplen;
    param_t *fe_param;

    parse_args_file(live_args);
    unlimit();
    kb_init(&live_kb);
    kb = &live_kb;
    kbcore = kb->kbcore;

    hmmdumpfp = cmd_ln_int32("-hmmdump") ? stderr : NULL;
    maxwpf    = cmd_ln_int32 ("-maxwpf");
    maxhistpf = cmd_ln_int32 ("-maxhistpf");
    maxhmmpf  = cmd_ln_int32 ("-maxhmmpf");
    ptranskip = cmd_ln_int32 ("-ptranskip");

    maxhyplen = cmd_ln_int32 ("-maxhyplen");
    if (!parthyp) 
        parthyp  = (partialhyp_t *) ckd_calloc(maxhyplen, sizeof(partialhyp_t));

    fe_param = (param_t *) ckd_calloc(1, sizeof(param_t));
    samprate = cmd_ln_int32 ("-samprate");
    if (samprate != 8000 && samprate != 16000)
	E_FATAL("Sampling rate %s not supported. Must be 8000 or 16000\n",samprate);
    fe_param->SAMPLING_RATE = (float32) samprate;
    fe_param->FRAME_RATE = 100; /* HARD CODED TO 100 FRAMES PER SECOND */
    fe_param->PRE_EMPHASIS_ALPHA = (float32) 0.97;
    fe = fe_init(fe_param);
    if (!fe)
	E_FATAL("Front end initialization fe_init() failed\n");

    maxcepvecs = cmd_ln_int32 ("-maxcepvecs");
    ceplen = kbcore->fcb->cepsize;
}


/* Routine to decode a block of incoming samples. A partial hypothesis
 * for the utterance upto the current block of samples is returned.
 * The calling routine has to inform the routine if the block of samples
 * being passed is the final block of samples for an utterance by
 * setting live_endutt to 1. On receipt of a live_endutt flag the routine
 * automatically assumes that the next block of samples is the beginning
 * of a new utterance 
 */

int32 live_utt_decode_block (int16 *samples, int32 nsamples, 
		      int32 live_endutt, partialhyp_t **ohyp)
{
    static int32 live_begin_new_utt = 1;
    static int32 frmno;
    float32 **live_feat;
    int32   live_nfr, live_nfeatvec;
    int32   id, nwds;
    glist_t hyp;
    gnode_t *gn;
    hyp_t   *h;
    dict_t  *dict;
    float32 **mfcbuf;

 
    if (live_begin_new_utt){
        fe_start_utt(fe);
	utt_begin (kb);
	frmno = 0;
	kb->nfr = 0;
        kb->utt_hmm_eval = 0;
        kb->utt_sen_eval = 0;
        kb->utt_gau_eval = 0;
        live_begin_new_utt = 0;
    }
    /* 10.jan.01 RAH, fe_process_utt now requires ***mfcbuf and it allocates the memory internally) */
    mfcbuf = NULL;
    live_nfr = fe_process_utt(fe, samples, nsamples, &mfcbuf);
    if (live_endutt)	/*  */
      /* RAH 10.jan.01 live_nfr = number of frames, not index. [live_nfr-1] is needed instead of [live_nfr]*/
      fe_end_utt(fe,mfcbuf[live_nfr-1]); /* Flush out the fe, but dont use the returned final frame */

    /* Compute feature vectors */
    live_nfeatvec = feat_s2mfc2feat_block(kbcore_fcb(kbcore), mfcbuf,
                                         live_nfr, live_begin_new_utt,
					 live_endutt, &live_feat);

    /* decode the block */
    utt_decode_block (live_feat, live_nfeatvec, &frmno, kb, 
		      maxwpf, maxhistpf, maxhmmpf, ptranskip, hmmdumpfp);

    /* Pull out partial hypothesis */
    dict = kbcore_dict (kb->kbcore);

    if (live_endutt)
      id = vithist_utt_end(kb->vithist, kb->kbcore);
    else
      id = vithist_partialutt_end(kb->vithist, kb->kbcore);
    
    if (id > 0) {
        hyp = vithist_backtrace(kb->vithist,id);

        for (gn = hyp,nwds=0; gn; gn = gnode_next(gn),nwds++) {
            h = (hyp_t *) gnode_ptr (gn);
            if (parthyp[nwds].word != NULL) {
                ckd_free(parthyp[nwds].word);
                parthyp[nwds].word = NULL;
            }
            parthyp[nwds].word = strdup(dict_wordstr(dict, h->id));
            parthyp[nwds].sf = h->sf;
            parthyp[nwds].ef = h->ef;
	    parthyp[nwds].ascr = h->ascr;
	    parthyp[nwds].lscr = h->lscr;
        }
        if (parthyp[nwds].word != NULL){
            ckd_free(parthyp[nwds].word);
            parthyp[nwds].word = NULL;
        }
        /* Free hyplist */
        for (gn = hyp; gn && (gnode_next(gn)); gn = gnode_next(gn)) {
            h = (hyp_t *) gnode_ptr (gn);
            ckd_free ((void *) h);
        }
        glist_free (hyp);
    } else {
        nwds = 0;
        if (parthyp[nwds].word != NULL) {
            ckd_free(parthyp[nwds].word);
            parthyp[nwds].word = NULL;
        }
    }
    *ohyp = parthyp;

    /* Clean up */
    if (live_endutt) {
	live_begin_new_utt = 1;
	kb->tot_fr += kb->nfr;
	utt_end(kb);
    }
    else {
	live_begin_new_utt = 0;
    }

    return(nwds);
}
