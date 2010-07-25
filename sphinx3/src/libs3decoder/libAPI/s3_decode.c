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
* HISTORY
 * $Log$
 * Revision 1.26  2006/03/07  21:04:19  dhdfu
 * Fill in the word string in srch_hyp_t
 * 
 * Revision 1.25  2006/02/24 13:41:26  arthchan2003
 * Used lm_read_advance instead of lm_read
 *
 * Revision 1.24  2006/02/22 22:36:56  arthchan2003
 * changed cmd_ln_access to cmd_ln_int32 in live_decode_API.c
 *
 * Revision 1.23  2006/02/22 21:46:51  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII:
 *
 * 1, Supported -dither (also -seed).
 * 2, Changed implementation from hyp_t to srch_hyp_t.  The definition will have NO CHANGE because hyp_t will be typedef as srch_hyp_t.
 * 3, Supported reading, addition and deletion of LMs through ld_read,
 * ld_set_lm and ld_delete_lm.
 *
 * Revision 1.22.4.7  2005/09/25 18:56:11  arthchan2003
 * Added dict argument into vithist_backtrace.
 *
 * Revision 1.22.4.6  2005/07/26 02:16:42  arthchan2003
 * Merged hyp_t with srch_hyp_t
 *
 * Revision 1.22.4.5  2005/07/20 19:42:30  arthchan2003
 * Completed live decode layer of lm add. Added command-line arguments for fsg and phone insertion.
 *
 * Revision 1.22.4.4  2005/07/18 19:00:36  arthchan2003
 * Changed the type of machine_endian and input_endian to "little" or "big" , changed the type of sampling rate to float32.
 *
 * Revision 1.22.4.3  2005/07/13 02:02:52  arthchan2003
 * Added -dither and -seed in the option.  Dithering is also support in livepretend. The behavior will be conformed s3.x's wave2feat, start to re-incorproate lm_read. Not completed yet.
 *
 * Revision 1.22.4.2  2005/07/07 02:31:54  arthchan2003
 * Remove -lminsearch, it proves to be useless and FSG implementation.
 *
 * Revision 1.22.4.1  2005/06/28 06:57:41  arthchan2003
 * Added protype of initializing and reading FSG, still not working.
 *
 * Revision 1.22  2005/06/22 02:49:34  arthchan2003
 * 1, changed ld_set_lm to call srch_set_lm, 2, hand the accounting tasks to stat_t, 3, added several empty functions for future use.
 *
 * Revision 1.6  2005/06/15 21:13:27  archan
 * Open ld_set_lm, ld_delete_lm in live_decode_API.[ch], Not yet decided whether ld_add_lm and ld_update_lm should be added at this point.
 *
 * Revision 1.5  2005/04/20 03:40:23  archan
 * Add many empty functions into ld, many of them are not yet implemented.
 *
 * Revision 1.4  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
* May 14, 2004
*   Created by Yitao Sun (yitao@cs.cmu.edu) based on the live.c created by
*   Rita Singh.  This version is meant to expose features with a simpler and
*   more explicit API.
*
* Jun 10, 2004
*   Modified by Yitao Sun.  Added argument parsing.
*/

/* OLD LOGS before renaming to live_decode_API.h
----------------------------
revision 1.8
date: 2004/09/03 18:17:11;  author: yitao;  state: Exp;  lines: +15 -11

changed ld_process_frames to ld_process_ceps
----------------------------
revision 1.7
date: 2004/08/25 20:44:31;  author: yitao;  state: Exp;  lines: +65 -41


1.  added code to record uttid in live-decode
2.  added more code to flesh out remote-decode.  not compiling yet.
----------------------------
revision 1.6
date: 2004/08/24 18:05:50;  author: yitao;  state: Exp;  lines: +2 -2

fixed compilation bug in function ld_utt_free_hyps().
----------------------------
revision 1.5
date: 2004/08/23 20:41:36;  author: yitao;  state: Exp;  lines: +7 -14

basic implementation for remote-decode API.  not compiling yet.
----------------------------
revision 1.4
date: 2004/08/10 22:13:48;  author: yitao;  state: Exp;  lines: +18 -10

added some minor comments in the code.  no significant change.
----------------------------
revision 1.3
date: 2004/08/09 21:40:36;  author: yitao;  state: Exp;  lines: +122 -93

1.  fixed some bugs in Live-Decode API.  changed kb.c, kb.h, utt.c, live_decode.c, liv
e_decode.h.
2.  changed some filenames in src/programs/.  now there are 2 sets of livedecode and l
ivepretend: one that uses the old API (livedecode and livepretend), and one that uses 
the new API (livedecode2 and livepretend2).
3.  modified Makefile.am to reflect the filename changes above.
----------------------------
revision 1.2
date: 2004/08/08 23:34:50;  author: arthchan2003;  state: Exp;  lines: +1 -1
temporary fixes of live_decode.c and live_decode.h
----------------------------
revision 1.1
date: 2004/08/06 15:07:39;  author: yitao;  state: Exp;
*** empty log message ***
=============================================================================
*/

#include <string.h>
#include <time.h>
#include <sphinxbase/feat.h>

#include "s3_decode.h"
#include "cmdln_macro.h"
#include "utt.h"
#include "lm.h"
#include "srch.h"

arg_t S3_DECODE_ARG_DEFS[] = {
    waveform_to_cepstral_command_line_macro(),
    cepstral_to_feature_command_line_macro(),

    acoustic_model_command_line_macro(),
    speaker_adaptation_command_line_macro(),
    
    language_model_command_line_macro(),
    log_table_command_line_macro(),
    dictionary_command_line_macro(),
    phoneme_lookahead_command_line_macro(),
    histogram_pruning_command_line_macro(),
    fast_GMM_computation_command_line_macro(),
    common_filler_properties_command_line_macro(),
    common_s3x_beam_properties_command_line_macro(),
    control_file_handling_command_line_macro(),
    hypothesis_file_handling_command_line_macro(),
    score_handling_command_line_macro(),
    output_lattice_handling_command_line_macro(),
    dag_handling_command_line_macro(),
    second_stage_dag_handling_command_line_macro(),
    input_lattice_handling_command_line_macro(),
    flat_fwd_debugging_command_line_macro(),
    history_table_command_line_macro(),
    
    decode_specific_command_line_macro(),
    search_specific_command_line_macro(),
    search_modeTST_specific_command_line_macro(),
    search_modeWST_specific_command_line_macro(),
    control_lm_mllr_file_command_line_macro(),
    finite_state_grammar_command_line_macro(),
    phone_insertion_penalty_command_line_macro(),
    
    partial_hypothesis_command_line_macro(),

    {
	"-bestscoredir",
	ARG_STRING,
	NULL,
	"(Mode 3) Directory for writing best score/frame (used to set "
	"beamwidth; one file/utterance)"
    },

    {
	"-machine_endian",
	ARG_STRING,
#ifdef WORDS_BIGENDIAN
	"big",
#else
	"little",
#endif
	"Endianness of machine, big or little"
    },

    {
	"-rawext",
	ARG_STRING,
	".raw",
	"Input raw files extension"
    },

    {NULL, ARG_INT32, NULL, NULL}
};

/* Utility function declarations */
static int
s3_decode_set_uttid(s3_decode_t * _decode, char *_uttid);

static int
s3_decode_record_hyps(s3_decode_t * _decode, int _end_utt);

static void
s3_decode_free_hyps(s3_decode_t * _decode);

int
s3_decode_init(s3_decode_t * _decode, cmd_ln_t *_config)
{
    if (_decode == NULL)
        return S3_DECODE_ERROR_NULL_POINTER;

    /* capture decoder parameters */
    kb_init(&_decode->kb, _config);

    /* initialize decoder variables */
    _decode->kbcore = _decode->kb.kbcore;
    _decode->hyp_frame_num = -1;
    _decode->uttid = NULL;
    _decode->state = S3_DECODE_STATE_IDLE;
    _decode->hyp_str = NULL;
    _decode->hyp_segs = NULL;

    _decode->swap =
            strcmp(cmd_ln_str_r(_config,"-machine_endian"),
                   cmd_ln_str_r(_config,"-input_endian"));

    if (_decode->swap)
        E_INFO("Input data WILL be byte swapped\n");
    else
        E_INFO("Input data will NOT be byte swapped\n");

    _decode->phypdump = (cmd_ln_int32_r(_config, "-phypdump"));

    if (_decode->phypdump)
        E_INFO("Partial hypothesis WILL be dumped\n");
    else
        E_INFO("Partial hypothesis will NOT be dumped\n");

    _decode->rawext = (cmd_ln_str_r(_config, "-rawext"));

    return S3_DECODE_SUCCESS;
}

void
s3_decode_close(s3_decode_t * _decode)
{
    if (_decode == NULL)
        return;

    kb_free(&_decode->kb);
    s3_decode_free_hyps(_decode);
    if (_decode->uttid != NULL) {
        ckd_free(_decode->uttid);
        _decode->uttid = NULL;
    }
    _decode->state = S3_DECODE_STATE_FINISHED;
}

int
s3_decode_begin_utt(s3_decode_t * _decode, char *_uttid)
{
    if (_decode == NULL)
        return S3_DECODE_ERROR_NULL_POINTER;

    if (_decode->state != S3_DECODE_STATE_IDLE) {
        E_WARN("Cannot begin new utterance in current decoder state.\n");
        return S3_DECODE_ERROR_INVALID_STATE;
    }

    s3_decode_free_hyps(_decode);

    utt_begin(&_decode->kb);

    _decode->num_frames_decoded = 0;
    _decode->num_frames_entered = 0;
    _decode->state = S3_DECODE_STATE_DECODING;

    stat_clear_utt(_decode->kb.stat);

    return s3_decode_set_uttid(_decode, _uttid);
}

void
s3_decode_end_utt(s3_decode_t * _decode)
{
    int32 num_features;

    if (_decode == NULL)
        return;

    if (_decode->state != S3_DECODE_STATE_DECODING) {
        E_WARN("Cannot end utterance in current decoder state.\n");
        return;
    }

    /* Call this with no frames, to update CMN and AGC statistics. */
    num_features = feat_s2mfc2feat_live(kbcore_fcb(_decode->kbcore),
                                        NULL, NULL, FALSE,
                                        TRUE, _decode->kb.feat);
    if (num_features > 0)
        utt_decode_block(_decode->kb.feat,
                         num_features,
                         &_decode->num_frames_decoded,
                         &_decode->kb);

    _decode->kb.stat->tot_fr += _decode->kb.stat->nfr;
    s3_decode_record_hyps(_decode, TRUE);
    utt_end(&_decode->kb);
    _decode->state = S3_DECODE_STATE_IDLE;
}

int
s3_decode_process(s3_decode_t * _decode,
                  float32 ** _cep_frames,
                  int32 _num_frames)
{
    int32 num_features = 0;
    int32 begin_utt = _decode->num_frames_entered == 0;

    if (_decode == NULL)
        return S3_DECODE_ERROR_NULL_POINTER;

    if (_num_frames >= S3_MAX_FRAMES)
        return S3_DECODE_ERROR_OUT_OF_MEMORY;


    if (_num_frames > 0) {
        num_features = feat_s2mfc2feat_live(kbcore_fcb(_decode->kbcore),
                                            _cep_frames,
                                            &_num_frames,
                                            begin_utt,
                                            FALSE,
                                            _decode->kb.feat);
        _decode->num_frames_entered += _num_frames;

        if (num_features > 0) {
            if (_decode->num_frames_entered >= S3_MAX_FRAMES)
                return S3_DECODE_ERROR_OUT_OF_MEMORY;

            utt_decode_block(_decode->kb.feat,
                             num_features,
                             &_decode->num_frames_decoded,
                             &_decode->kb);
        }
    }

    return S3_DECODE_SUCCESS;
}

int
s3_decode_hypothesis(s3_decode_t * _decode, char **_uttid, char **_hyp_str,
                     hyp_t *** _hyp_segs)
{
    int rv = S3_DECODE_SUCCESS;

    if (_decode == NULL)
        return S3_DECODE_ERROR_NULL_POINTER;

    /* re-record the hypothesis if there is a frame number mismatch */
    if (_decode->num_frames_decoded != _decode->hyp_frame_num)
        rv = s3_decode_record_hyps(_decode, FALSE);

    if (_uttid != NULL)
        *_uttid = _decode->uttid;

    if (_hyp_str != NULL)
        *_hyp_str = _decode->hyp_str;

    if (_hyp_segs != NULL)
        *_hyp_segs = _decode->hyp_segs;

    return rv;
}

dag_t *
s3_decode_word_graph(s3_decode_t *_decode)
{
    srch_t *s;

    if (_decode == NULL)
        return NULL;

    if (_decode->state != S3_DECODE_STATE_IDLE) {
        E_WARN("Cannot retrieve word graph in current decoder state.\n");
        return NULL;
    }

    s = (srch_t *) _decode->kb.srch;
    assert(s != NULL);

    return srch_get_dag(s);
}

void
s3_decode_read_lm(s3_decode_t * _decode,
           const char *lmpath, const char *lmname)
{
    srch_t *s;
    lm_t *lm;
    int32 ndict;
    s = (srch_t *) _decode->kb.srch;

    ndict = dict_size(_decode->kb.kbcore->dict);


    lm = lm_read_advance(lmpath, lmname,
                         cmd_ln_float32_r(kbcore_config(_decode->kbcore), "-lw"),
                         cmd_ln_float32_r(kbcore_config(_decode->kbcore), "-wip"),
                         cmd_ln_float32_r(kbcore_config(_decode->kbcore), "-uw"),
                         ndict, NULL, 1,   /* Weight apply */
                         kbcore_logmath(s->kbc)
        );

    s->funcs->add_lm(s, lm, lmname);
}

void
s3_decode_set_lm(s3_decode_t * _decode, const char *lmname)
{
    srch_t *s;
    s = (srch_t *) _decode->kb.srch;
    s->funcs->set_lm(s, lmname);
}

void
s3_decode_delete_lm(s3_decode_t * _decode, const char *lmname)
{
    srch_t *s;
    s = (srch_t *) _decode->kb.srch;
    s->funcs->delete_lm(s, lmname);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

int
s3_decode_set_uttid(s3_decode_t * _decode, char *_uttid)
{
    char *local_uttid = NULL;
    struct tm *times;
    time_t t;

    if (_decode == NULL)
        return S3_DECODE_ERROR_NULL_POINTER;

    if (_decode->uttid != NULL) {
        ckd_free(_decode->uttid);
        _decode->uttid = NULL;
    }

    /* automatically-generated uttid */
    if (_uttid == NULL) {
        t = time(NULL);
        times = localtime(&t);
        if ((local_uttid = ckd_malloc(17)) == NULL) {
            E_WARN("Failed to allocate space for utterance id.\n");
            return S3_DECODE_ERROR_OUT_OF_MEMORY;
        }
        sprintf(local_uttid, "*%4d%2d%2dZ%2d%2d%2d",
                times->tm_year, times->tm_mon, times->tm_mday,
                times->tm_hour, times->tm_min, times->tm_sec);
    }
    /* user-defined uttid */
    else {
        if ((local_uttid = ckd_malloc(strlen(_uttid) + 1)) == NULL) {
            E_WARN("Failed to allocate space for utterance id.\n");
            return S3_DECODE_ERROR_OUT_OF_MEMORY;
        }
        strcpy(local_uttid, _uttid);
    }
    _decode->uttid = local_uttid;
    /* Also set the kb internal uttid. This sets the uttid in the results. */
    kb_set_uttid(_decode->uttid, NULL, &(_decode->kb));

    return S3_DECODE_SUCCESS;
}

int
s3_decode_record_hyps(s3_decode_t * _decode, int _end_utt)
{
    int32 i = 0;
    glist_t hyp_list;
    gnode_t *node;
    srch_hyp_t *hyp;
    char *hyp_strptr = 0;
    char *hyp_str = 0;
    srch_t *srch;
    srch_hyp_t **hyp_segs = 0;
    int hyp_seglen = 0;
    int hyp_strlen = 0;
    int finish_wid = 0;
    kb_t *kb = 0;
    dict_t *dict;
    int rv;

    if (_decode == NULL)
        return S3_DECODE_ERROR_NULL_POINTER;

    s3_decode_free_hyps(_decode);

    kb = &_decode->kb;
    dict = kbcore_dict(_decode->kbcore);
    srch = (srch_t *) _decode->kb.srch;
    hyp_list = srch_get_hyp(srch);
    if (hyp_list == NULL) {
        E_WARN("Failed to retrieve viterbi history.\n");
        return S3_DECODE_ERROR_INTERNAL;
    }

    /** record the segment length and the overall string length */
    finish_wid = dict_finishwid(dict);
    for (node = hyp_list; node != NULL; node = gnode_next(node)) {
        hyp = (srch_hyp_t *) gnode_ptr(node);
        hyp_seglen++;
        if (!dict_filler_word(dict, hyp->id) && hyp->id != finish_wid) {
            hyp_strlen +=
                strlen(dict_wordstr(dict, dict_basewid(dict, hyp->id))) +
                1;
        }
    }

    if (hyp_strlen == 0) {
        hyp_strlen = 1;
    }

  /** allocate array to hold the segments and/or decoded string */
    hyp_str = (char *) ckd_calloc(hyp_strlen, sizeof(char));
    hyp_segs =
        (srch_hyp_t **) ckd_calloc(hyp_seglen + 1, sizeof(srch_hyp_t *));
    if (hyp_segs == NULL || hyp_str == NULL) {
        E_WARN("Failed to allocate storage for hypothesis.\n");
        rv = S3_DECODE_ERROR_OUT_OF_MEMORY;
        goto s3_decode_record_hyps_cleanup;
    }

  /** iterate thru to fill in the array of segments and/or decoded string */
    i = 0;
    hyp_strptr = hyp_str;
    for (node = hyp_list; node != NULL; node = gnode_next(node), i++) {
        hyp = (srch_hyp_t *) gnode_ptr(node);
        hyp_segs[i] = hyp;

        hyp->word = dict_wordstr(dict, dict_basewid(dict, hyp->id));
        if (!dict_filler_word(dict, hyp->id) && hyp->id != finish_wid) {
            strcat(hyp_strptr,
                   dict_wordstr(dict, dict_basewid(dict, hyp->id)));
            hyp_strptr += strlen(hyp_strptr);
            *hyp_strptr = ' ';
            hyp_strptr += 1;
        }
    }
    glist_free(hyp_list);

    hyp_str[hyp_strlen - 1] = '\0';
    hyp_segs[hyp_seglen] = 0;
    _decode->hyp_frame_num = _decode->num_frames_decoded;
    _decode->hyp_segs = hyp_segs;
    _decode->hyp_str = hyp_str;

    return S3_DECODE_SUCCESS;

  s3_decode_record_hyps_cleanup:
    if (hyp_segs != NULL) {
        ckd_free(hyp_segs);
    }
    if (hyp_str != NULL) {
        ckd_free(hyp_str);
    }
    if (hyp_list != NULL) {
        for (node = hyp_list; node != NULL; node = gnode_next(node)) {
            if ((hyp = (srch_hyp_t *) gnode_ptr(node)) != NULL) {
                ckd_free(hyp);
            }
        }
        glist_free(hyp_list);
    }

    return rv;
}

void
s3_decode_free_hyps(s3_decode_t * _decode)
{
    srch_hyp_t **h;

    if (_decode == NULL)
        return;

  /** set the reference frame number to something invalid */
    _decode->hyp_frame_num = -1;

  /** free and reset the hypothesis string */
    if (_decode->hyp_str) {
        ckd_free(_decode->hyp_str);
        _decode->hyp_str = 0;
    }

  /** free and reset the hypothesis word segments */
    if (_decode->hyp_segs) {
        for (h = _decode->hyp_segs; *h; h++) {
            ckd_free(*h);
        }
        ckd_free(_decode->hyp_segs);
        _decode->hyp_segs = 0;
    }
}
