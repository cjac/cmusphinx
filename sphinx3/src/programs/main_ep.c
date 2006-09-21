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

#include "s3_endpointer.h"
#include "main_ep.h"
#include "logs3.h"
#include "fe.h"
#include "cmdln_macro.h"
#include "byteorder.h"

#define NFR		100

static arg_t defn[] = {
    { "-mean",
      ARG_STRING,
      NULL,
      "Mixture gaussian means input file" },
    { "-var",
      ARG_STRING,
      NULL,
      "Mixture gaussian variances input file" },
    { "-varfloor",
      ARG_FLOAT32,
      "0.0001",
      "Mixture gaussian variance floor (applied to data from -var file)" },
    { "-mixw",
      ARG_STRING,
      NULL,
      "Senone mixture weights input file" },
    { "-mixwfloor",
      ARG_FLOAT32,
      "0.0000001",
      "Senone mixture weights floor (applied to data from -mixw file)" },
    { "-senmgau",
      ARG_STRING,
      ".cont.",
      "Senone to mixture-gaussian mapping file (or .semi. or .cont.)" }, 
    { "-frate",
      ARG_INT32,
      "100",
      "Frame rate for frame-to-time conversion"},
    { "-ncep", 
      ARG_INT32, 
      ARG_STRINGIFY(DEFAULT_NUM_CEPSTRA),
      "Number of cepstrums" }, 
    { "-postclassify",
      ARG_INT32,
      "1",
      "Use voting windows to update the frame classes"},
    { "-begin_window",
      ARG_INT32,
      "6",
      "Size of the window to examine for start of utterance"},
    { "-begin_threshold",
      ARG_INT32,
      "2",
      "Threshold to mark for start of utterance"},
    { "-begin_pad",
      ARG_INT32,
      "50",
      "Padded frames before the start of utterance"},
    { "-end_window",
      ARG_INT32,
      "80",
      "Size of the window to examine for end of utterance"},
    { "-end_threshold",
      ARG_INT32,
      "2",
      "Threshold to mark for end of utterance"},
    { "-end_pad",
      ARG_INT32,
      "80",
      "Padded frames after the end of utterance"},
    { "-logbase",
      ARG_FLOAT32,
      "1.0003",
      "Base in which all log-likelihoods calculated" },
    { NULL, ARG_INT32, NULL, NULL}
};

void
get_filename_base(char *_fn, char *_base);

int
main(int _argc, char **_argv)
{
    s3_endpointer_t ep;
    char *cfg_fn;
    char *mfcc_fn;
    char base_fn[1024];
    FILE *in;
    float32 **frames;
    int n_frames, n_floats, n_ceps, i, swap, frate, begin_frame, end_frame;
    struct stat statbuf;

    cfg_fn = _argv[1];
    mfcc_fn = _argv[2];

    cmd_ln_parse_file(defn, cfg_fn);

    if (stat(mfcc_fn, &statbuf) < 0)
	E_FATAL("Cannot stat input file %s\n", mfcc_fn);

    if ((in = fopen(mfcc_fn, "rb")) == NULL)
	E_FATAL("Cannot open input file %s\n", mfcc_fn);

    get_filename_base(mfcc_fn, base_fn);

    if (fread(&n_floats, sizeof(int32), 1, in) != 1) {
	fclose(in);
	E_FATAL("MFCC file must start with total vector size\n");
    }

    if ((int)(n_floats * sizeof(float32) + 4) != statbuf.st_size) {
	SWAP_INT32(&n_floats);
	if ((int)(n_floats * sizeof(float32) + 4) != statbuf.st_size)
	    E_FATAL("MFCC indicated size (%d) and actual (%d) size are"
		    "different\n",
		    n_floats * sizeof(float32) + 4,
		    statbuf.st_size);
	swap = 1;
    }

    n_ceps = cmd_ln_int32("-ncep");
    frate = cmd_ln_int32("-frate");
    logs3_init(cmd_ln_float32("-logbase"), 0, 0);

    s3_endpointer_init(&ep,
		       cmd_ln_str("-mean"),
		       cmd_ln_str("-var"),
		       cmd_ln_float32("-varfloor"),
		       cmd_ln_str("-mixw"),
		       cmd_ln_float32("-mixwfloor"),
		       cmd_ln_str("-senmgau"),
		       1, /* post classify.  fixed at TRUE for now */
		       cmd_ln_int32("-begin_window"),
		       cmd_ln_int32("-begin_threshold"),
		       cmd_ln_int32("-begin_pad"),
		       cmd_ln_int32("-end_window"),
		       cmd_ln_int32("-end_threshold"),
		       cmd_ln_int32("-end_pad")
		       );

    frames = (float32 **)ckd_calloc_2d(NFR, n_ceps, sizeof(float32));

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
	       (float)begin_frame / frate,
	       (float)end_frame / frate);
	
    }

    s3_endpointer_close(&ep);

    return 0;
}

void
get_filename_base(char *_fn, char *_base)
{
    int len = strlen(_fn);
    int i, j;

    for (j = len - 1; j >= 0; j--)
	if (_fn[j] == '.')
	    break;
    
    if (j < 0)
	j = len;

    for (i = j; i >= 0; i--)
	if (_fn[i] == '/')
	    break;

    if (i < 0)
	i = 0;

    strncpy(_base, _fn + i + 1, j - i - 1);
    _base[j - i] = '\0';
}
