/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.	All rights
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
 *  Aug 6, 2004
 *  Created by Yitao Sun (yitao@cs.cmu.edu).
 *
 * HISTORY
 * $Log$
 *
 */


/** \file main_continuous.c
 * \brief Driver for live-mode simulation with continuous audio
 * (energy-based endpointer).
 */
#include <stdio.h>

#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>
#include <sphinxbase/info.h>

#include "s3_decode.h"
#include "kb.h"
#include "s3types.h"
#include "corpus.h"
#include "srch.h"

#define SAMPLE_BUFFER_LENGTH	4096
#define FILENAME_LENGTH		512

static fe_t *fe;
static s3_decode_t decoder;
static char *rawdirfn;
static stat_t *st;
static FILE *rawfd;

static int32
ad_file_read(ad_rec_t * ad, int16 * buf, int32 max)
{
    size_t nread;

    nread = fread(buf, sizeof(int16), max, rawfd);
    if (nread == 0 && feof(rawfd))
        return -1;
    else
        return nread;
}

static void
utt_livepretend(void *data, utt_res_t * ur, int32 sf, int32 ef,
                char *uttid)
{
    char fullrawfn[FILENAME_LENGTH];
    char *hypstr;

    cont_ad_t *cont_ad;
    ad_rec_t bogus_ad;
    short samples[SAMPLE_BUFFER_LENGTH];
    float32 **frames;
    kb_t *kb;
    int nread, n_frames, seg_n_frames;
    int ts, listening;

    kb = (kb_t *) data;

    /*  report_utt_res(ur); */
    sprintf(fullrawfn, "%s/%s%s", rawdirfn, ur->uttfile, decoder.rawext);
    if ((rawfd = fopen(fullrawfn, "rb")) == NULL) {
        E_FATAL("Cannnot open raw file %s.\n", fullrawfn);
    }

    if (ur->lmname != NULL)
        srch_set_lm((srch_t *) kb->srch, ur->lmname);
    if (ur->regmatname != NULL)
        kb_setmllr(ur->regmatname, ur->cb2mllrname, kb);

    bogus_ad.sps = (int32) cmd_ln_float32_r(kb->kbcore->config, "-samprate");
    if ((cont_ad = cont_ad_init(&bogus_ad, ad_file_read)) == NULL) {
        E_FATAL("Failed to initialize energy-based endpointer");
    }
    listening = 0;
    ts = 0;
    seg_n_frames = 0;
    while ((nread = cont_ad_read(cont_ad, samples, SAMPLE_BUFFER_LENGTH))
	   >= 0) {
        if (nread) {
            ts = cont_ad->read_ts;
            if (!listening) {
                char uttid[FILENAME_LENGTH];
                sprintf(uttid, "%s_%.3f", ur->uttfile,
                        (double) ts / bogus_ad.sps);
                if (s3_decode_begin_utt(&decoder, uttid) != S3_DECODE_SUCCESS)
                    E_FATAL("Cannot begin utterance decoding.\n");
                listening = 1;
            }
            ptmr_start(&(st->tm));
	    fe_process_utt(fe, samples, nread, &frames, &n_frames);
	    seg_n_frames += n_frames;
	    if (frames != NULL) {
		s3_decode_process(&decoder, frames, n_frames);
		ckd_free_2d((void **)frames);
	    }
            ptmr_stop(&(st->tm));

            if (s3_decode_hypothesis(&decoder, NULL, &hypstr, NULL) ==
                S3_DECODE_SUCCESS) {
                if (decoder.phypdump) {
                    E_INFO("PARTIAL_HYP: %s\n", hypstr);
                }
            }

	    /* If the segment is too long, break it. */
	    if (seg_n_frames > 15000) {
                s3_decode_end_utt(&decoder);
                listening = 0;
	    }
        }
        else {
            if (listening && cont_ad->read_ts - ts > 8000) {    /* HACK */
                s3_decode_end_utt(&decoder);
                listening = 0;
            }
        }
    }
    fclose(rawfd);
    cont_ad_close(cont_ad);
    if (listening)
        s3_decode_end_utt(&decoder);
}

int
main(int _argc, char **_argv)
{
    char *ctrlfn;
    char *cfgfn;
    cmd_ln_t *config;

    print_appl_info(_argv[0]);

    if (_argc != 4) {
        printf("\nUSAGE: %s <ctrlfile> <rawdir> <cfgfile>\n", _argv[0]);
        return -1;
    }

    ctrlfn = _argv[1];
    rawdirfn = _argv[2];
    cfgfn = _argv[3];

    if ((config = cmd_ln_parse_file_r(NULL, S3_DECODE_ARG_DEFS, cfgfn, TRUE)) == NULL)
        E_FATAL("Bad configuration file %s.\n", cfgfn);

    if (s3_decode_init(&decoder, config) != S3_DECODE_SUCCESS)
        E_FATAL("Failed to initialize live-decoder.\n");

    fe = fe_init_auto_r(config); 

    st = decoder.kb.stat;
    ptmr_init(&(st->tm));


    if (ctrlfn) {
        /* When -ctlfile is speicified, corpus.c will look at -ctl_lm and
	   -ctl_mllr to get the corresponding LM and MLLR for the utterance */
        st->tm = ctl_process(ctrlfn,
                             cmd_ln_str_r(config, "-ctl_lm"),
                             cmd_ln_str_r(config, "-ctl_mllr"),
                             cmd_ln_int32_r(config, "-ctloffset"),
                             cmd_ln_int32_r(config, "-ctlcount"),
                             utt_livepretend, &(decoder.kb));
    }
    else {
        E_FATAL("control file is not specified.\n");
    }

    s3_decode_close(&decoder);

    stat_report_corpus(decoder.kb.stat);

    return 0;
}
