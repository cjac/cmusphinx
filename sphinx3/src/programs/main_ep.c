/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2006 Carnegie Mellon University.  All rights 
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

#include <string.h>
#include <sys/stat.h>

#include <sphinxbase/fe.h>
#include <sphinxbase/byteorder.h>

#include "s3_endpointer.h"
#include "classify.h"
#include "endptr.h"
#include "logs3.h"
#include "cmdln_macro.h"

#define NFR		100

static arg_t defn[] = {
    {"-mean",
     ARG_STRING,
     NULL,
     "Mixture gaussian means input file"},
    {"-var",
     ARG_STRING,
     NULL,
     "Mixture gaussian variances input file"},
    {"-varfloor",
     ARG_FLOAT32,
     "0.0001",
     "Mixture gaussian variance floor (applied to data from -var file)"},
    {"-mixw",
     ARG_STRING,
     NULL,
     "Senone mixture weights input file"},
    {"-mixwfloor",
     ARG_FLOAT32,
     "0.0000001",
     "Senone mixture weights floor (applied to data from -mixw file)"},
    {"-senmgau",
     ARG_STRING,
     ".cont.",
     "Senone to mixture-gaussian mapping file (or .semi. or .cont.)"},
    {"-frate",
     ARG_INT32,
     "100",
     "Frame rate for frame-to-time conversion"},
    {"-input",
     ARG_STRING,
     NULL,
     "Input cepstra file"},
    {"-ncep",
     ARG_INT32,
     ARG_STRINGIFY(DEFAULT_NUM_CEPSTRA),
     "Dimension of cepstral vector"},
    {"-postclassify",
     ARG_INT32,
     "1",
     "Use voting windows to update the frame classes"},
    {"-begin_window",
     ARG_INT32,
     "6",
     "Size of the window to examine for start of utterance"},
    {"-begin_threshold",
     ARG_INT32,
     "2",
     "Threshold to mark for start of utterance"},
    {"-begin_pad",
     ARG_INT32,
     "50",
     "Padded frames before the start of utterance"},
    {"-end_window",
     ARG_INT32,
     "80",
     "Size of the window to examine for end of utterance"},
    {"-end_threshold",
     ARG_INT32,
     "2",
     "Threshold to mark for end of utterance"},
    {"-end_pad",
     ARG_INT32,
     "80",
     "Padded frames after the end of utterance"},
    {"-logbase",
     ARG_FLOAT32,
     "1.0003",
     "Base in which all log-likelihoods calculated"},
    {NULL, ARG_INT32, NULL, NULL}
};

void
 get_filename_base(const char *_fn, char *_base);

int
main(int _argc, char **_argv)
{
    s3_endpointer_t ep;
    char *cfg_fn;
    const char *mfcc_fn;
    char base_fn[1024];
    FILE *in;
    float32 **frames;
    int n_frames, n_floats, n_ceps, i, swap = 0, frate, begin_frame, end_frame;
    struct stat statbuf;
    cmd_ln_t *config;
    logmath_t *logmath;

    if ((_argc == 3) && (_argv[1][0] != '-')) {
        cfg_fn = _argv[1];
        mfcc_fn = _argv[2];

        if ((config = cmd_ln_parse_file_r(NULL, defn, cfg_fn, TRUE)) == NULL)
            E_FATAL("Cannot parse config file\n");
    }
    else {
        if ((config = cmd_ln_parse_r(NULL, defn, _argc, _argv, TRUE)) == NULL)
            E_FATAL("Cannot parse command line\n");
        mfcc_fn = cmd_ln_str_r(config, "-input");
    }

    if (stat(mfcc_fn, &statbuf) < 0)
        E_FATAL("Cannot stat input file %s\n", mfcc_fn);

    if ((in = fopen(mfcc_fn, "rb")) == NULL)
        E_FATAL("Cannot open input file %s\n", mfcc_fn);

    get_filename_base(mfcc_fn, base_fn);

    if (fread(&n_floats, sizeof(int32), 1, in) != 1) {
        fclose(in);
        E_FATAL("MFCC file must start with total vector size\n");
    }

    if ((int) (n_floats * sizeof(float32) + 4) != statbuf.st_size) {
        SWAP_INT32(&n_floats);
        if ((int) (n_floats * sizeof(float32) + 4) != statbuf.st_size)
            E_FATAL("MFCC indicated size (%d) and actual (%d) size are"
                    "different\n",
                    n_floats * sizeof(float32) + 4, statbuf.st_size);
        swap = 1;
    }

    n_ceps = cmd_ln_int32_r(config, "-ncep");
    frate = cmd_ln_int32_r(config, "-frate");
    logmath = logs3_init(cmd_ln_float64_r(config, "-logbase"), 0, 0);

    s3_endpointer_init(&ep, cmd_ln_str_r(config, "-mean"), cmd_ln_str_r(config, "-var"), cmd_ln_float32_r(config, "-varfloor"), cmd_ln_str_r(config, "-mixw"), cmd_ln_float32_r(config, "-mixwfloor"), cmd_ln_str_r(config, "-senmgau"), 1, /* post classify.  fixed at TRUE for now */
                       cmd_ln_int32_r(config, "-begin_window"),
                       cmd_ln_int32_r(config, "-begin_threshold"),
                       cmd_ln_int32_r(config, "-begin_pad"),
                       cmd_ln_int32_r(config, "-end_window"),
                       cmd_ln_int32_r(config, "-end_threshold"),
                       cmd_ln_int32_r(config, "-end_pad"),
                       logmath
        );

    frames = (float32 **) ckd_calloc_2d(NFR, n_ceps, sizeof(float32));

    while (!feof(in)) {
        n_floats = fread(frames[0], sizeof(float32), NFR * n_ceps, in);
        if (swap)
            for (i = 0; i < n_floats; i++)
                SWAP_FLOAT32(&frames[0][i]);
        n_frames = n_floats / n_ceps;

        if (n_frames > 0)
            s3_endpointer_feed_frames(&ep, frames, n_frames, feof(in));


        if (!s3_endpointer_next_utt(&ep))
            continue;

        begin_frame = s3_endpointer_frame_count(&ep);

        while (0 <= (n_frames = s3_endpointer_read_utt(&ep, frames, NFR))) {
            if (n_frames > 0)
                continue;

            n_floats = fread(frames[0], sizeof(float32), NFR * n_ceps, in);
            if (swap)
                for (i = 0; i < n_floats; i++)
                    SWAP_FLOAT32(&frames[0][i]);
            n_frames = n_floats / n_ceps;

            if (n_frames > 0)
                s3_endpointer_feed_frames(&ep, frames, n_frames, feof(in));
        }

        end_frame = s3_endpointer_frame_count(&ep);

        printf("%s 1 local %0.3f %0.3f BLAH\n",
               base_fn,
               (float) begin_frame / frate, (float) end_frame / frate);

    }

    s3_endpointer_close(&ep);

    fclose(in);
    ckd_free_2d(frames);
    logmath_free(logmath);
    cmd_ln_free_r(config);

    return 0;
}

#ifndef PATHSEP
#ifdef _WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif /* _WIN32 */
#endif /* PATHSEP */

void
get_filename_base(const char *_fn, char *_base)
{
    char *dot, *slash;

    if ((slash = strrchr(_fn, PATHSEP)) != NULL)
        strcpy(_base, slash+1);
    else
        strcpy(_base, _fn);
    if ((dot = strrchr(_base, '.')) != NULL)
        *dot = '\0';
}
