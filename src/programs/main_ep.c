/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All rights 
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
 * HISTORY
 * $Log$
 * Revision 1.21  2006/03/03  20:04:06  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 * 
 * Revision 1.20  2006/02/24 04:20:06  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH, Used some macros. Merged with Dave and Evandro's changes.
 *
 *
 * Revision 1.19  2006/02/02 22:56:26  dhdfu
 * add a missing parameter
 *
 * Revision 1.18  2005/07/05 13:12:36  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.17  2005/07/04 20:57:52  dhdfu
 * Finally remove the "temporary hack" for the endpointer, and do
 * everything in logs3 domain.  Should make it faster and less likely to
 * crash on Alphas.
 *
 * Actually it kind of duplicates the existing GMM computation functions,
 * but it is slightly different (see the comment in classify.c).  I don't
 * know the rationale for this.
 *
 * Revision 1.16  2005/07/02 04:24:46  egouvea
 * Changed some hardwired constants to user defined parameters in the end pointer. Tested with make test-ep.
 *
 * Revision 1.15.4.5  2005/09/08 02:24:53  arthchan2003
 * Fix the mistake in last check-in.
 *
 * Revision 1.15.4.4  2005/09/07 23:48:58  arthchan2003
 * Remove .
 *
 * Revision 1.15.4.3  2005/07/18 23:21:24  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.15.4.2  2005/07/05 21:35:00  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.18  2005/07/05 13:12:36  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.15.4.1  2005/07/05 06:49:38  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.17  2005/07/04 20:57:52  dhdfu
 * Finally remove the "temporary hack" for the endpointer, and do
 * everything in logs3 domain.  Should make it faster and less likely to
 * crash on Alphas.
 *
 * Actually it kind of duplicates the existing GMM computation functions,
 * but it is slightly different (see the comment in classify.c).  I don't
 * know the rationale for this.
 *
 * Revision 1.16  2005/07/02 04:24:46  egouvea
 * Changed some hardwired constants to user defined parameters in the end pointer. Tested with make test-ep.
 *
 * Revision 1.15  2005/06/22 05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.2  2005/03/30 00:43:41  archan
 *
 * Add $Log$
 * Revision 1.21  2006/03/03  20:04:06  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 * 
 * Add Revision 1.20  2006/02/24 04:20:06  arthchan2003
 * Add Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH, Used some macros. Merged with Dave and Evandro's changes.
 * Add
 *
 */
#include <stdio.h>
#include <stdlib.h>
#if !defined(WIN32)
#include <unistd.h>
#include <sys/file.h>
#if !defined(O_BINARY)
#define O_BINARY 0
#endif
#endif
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>

#if defined(WIN32)
#include <io.h>
#include <errno.h>
#define srand48(x) srand(x)
#define lrand48() rand()
#endif

#include "cont_mgau.h"
#include "classify.h"
#include "endptr.h"
#include "fe.h"
#include "main_ep.h"
#include "logs3.h"
#include "cmdln_macro.h"
#include "byteorder.h"

/* Define a null device that depends on the platform */
#if defined(WIN32)
#define NULL_DEVICE "NUL"
#else
#define NULL_DEVICE "/dev/null"
#endif

/** \file main_ep.c
    \brief Driver for end-pointing
 */

const char helpstr[] = "Description: \n\
Print speech detection timing information from audio file.\n		\
									\
The main parameters that affect the final output, with typical values, are:\n \
									\
samprate, typically 8000, 11025, or 16000\n				\
lowerf, 200, 130, 130, for the respective sampling rates above\n	\
upperf, 3700, 5400, 6800, for the respective sampling rates above\n	\
nfilt, 31, 36, 40, for the respective sampling rates above\n		\
nfft, 256 or 512\n							\
format, raw or nist or mswav\n						\
\"";

const char examplestr[] = "Example: \n\
This example displays the timing information reporting beginning/end of speech from an input audio file named \"input.raw\", which is a raw audio file (no header information), which was originally sampled at 16kHz. \n \
\n									\
ep -i  input.raw \n						\
        -raw 1 \n							\
        -input_endian little \n						\
        -samprate  16000 \n						\
        -lowerf    130 \n						\
        -upperf    6800 \n						\
        -nfilt     40 \n						\
        -nfft      512";
static arg_t arg[] = {

    waveform_to_cepstral_command_line_macro()
        log_table_command_line_macro()
        common_application_properties_command_line_macro()
        gmm_command_line_macro(){"-machine_endian",
                                 ARG_STRING,
#ifdef WORDS_BIGENDIAN
                                 "big",
#else
                                 "little",
#endif
                                 "Endianness of machine, big or little"},

    {"-i",
     ARG_STRING,
     NULL,
     "Single audio input file"},
    {"-o",
     ARG_STRING,
     NULL_DEVICE,
     "Single cepstral output file. Not used by ep."},
    {"-c",
     ARG_STRING,
     NULL,
     "Control file for batch processing. Not used by ep."},
    {"-di",
     ARG_STRING,
     NULL,
     "Input directory, input file names are relative to this, if defined. Not used by ep."},
    {"-ei",
     ARG_STRING,
     NULL,
     "Input extension to be applied to all input files. Not used by ep."},
    {"-do",
     ARG_STRING,
     NULL,
     "Output directory, output files are relative to this. Not used by ep."},
    {"-eo",
     ARG_STRING,
     NULL,
     "Output extension to be applied to all output files. Not used by ep."},
    {"-nist",
     ARG_INT32,
     "0",
     "Defines input format as NIST sphere"},
    {"-raw",
     ARG_INT32,
     "0",
     "Defines input format as raw binary data"},
    {"-mswav",
     ARG_INT32,
     "0",
     "Defines input format as Microsoft Wav (RIFF)"},
    {"-nchans",
     ARG_INT32,
     "1",
     "Number of channels of data (interlaced samples assumed)"},
    {"-whichchan",
     ARG_INT32,
     "1",
     "Channel to process"},
    {"-logspec",
     ARG_INT32,
     "0",
     "Write out logspectral files instead of cepstra"},
    {"-feat",
     ARG_STRING,
     "sphinx",
     "SPHINX format - big endian"},
    {"-verbose",
     ARG_INT32,
     "0",
     "Show input filenames"},
    {"-mdef",
     ARG_STRING,
     NULL,
     "The model definition file"},
    {"-pad_before",
     ARG_FLOAT32,
     PAD_T_BEFORE,
     "Pad these many seconds before speech begin"},
    {"-pad_after",
     ARG_FLOAT32,
     PAD_T_AFTER,
     "Pad these many seconds after speech end"},
    {"-speech_start",
     ARG_FLOAT32,
     UTT_T_START,
     "Declare speech after these many seconds of speech (pad not accounted)"},
    {"-speech_end",
     ARG_FLOAT32,
     UTT_T_END,
     "Declare end of speech after these many seconds of non-speech (pad not accounted)"},
    {"-speech_cancel",
     ARG_FLOAT32,
     UTT_T_CANCEL,
     "Cancel a start of speech  after these many seconds of non-speech"},
    {NULL, ARG_INT32, NULL, NULL}
};

static void fe_parse_options(fewrap_t *FEW);

/*** Function to initialize the front-end wrapper ***/
fewrap_t *
few_initialize()
{
    fewrap_t *FEW = (fewrap_t *) ckd_calloc(1, sizeof(fewrap_t));

    /********************** INITIALIZING COMPONENTS ******************/

    /* initialize parameters */
    fe_parse_options(FEW);

    /* initialize the front-end parameters */
    if ((FEW->FE = fe_init(FEW->P)) == NULL) {
        fprintf(stderr,
                "memory alloc failed in fe_convert_files()\n...exiting\n");
        exit(0);
    }

    /*************** Allocate memory for each frame of speech *******************/

    FEW->fr_data =
        (int16 *) ckd_calloc(FEW->FE->FRAME_SIZE, sizeof(int16));
    FEW->fr_cep =
        (float32 *) ckd_calloc(FEW->FE->NUM_CEPSTRA, sizeof(float32));

    return (FEW);

}

/*** Function to free the front-end wrapper ***/
void
few_free(fewrap_t * FEW)
{
    if (FEW) {
        if (FEW->fr_data)
            free(FEW->fr_data);

        if (FEW->fr_cep)
            free(FEW->fr_cep);

        fe_close(FEW->FE);
        free(FEW->P);
        free(FEW);
    }

}

/** Validate the param_t function. 
 */
static void
fe_validate_parameters(fewrap_t * FEW            /**< A parameter structure */
    )
{
    param_t *P = FEW->P;
    if ((FEW->is_batch) && (FEW->is_single)) {
        E_FATAL
            ("You cannot define an input file and a control file at the same time.\n");
    }

    if (FEW->wavfile == NULL && FEW->wavdir == NULL) {
        E_FATAL("No input file or file directory given\n");
    }

    if (FEW->cepfile == NULL && FEW->cepdir == NULL) {
        E_FATAL("No cepstra file or file directory given\n");
    }

    if (FEW->ctlfile == NULL && FEW->cepfile == NULL && FEW->wavfile == NULL) {
        E_FATAL("No control file given\n");
    }

    if (FEW->nchans > 1) {
        E_INFO("Files have %d channels of data\n", FEW->nchans);
        E_INFO("Will extract features for channel %d\n", FEW->whichchan);
    }

    if (FEW->whichchan > FEW->nchans) {
        E_FATAL("You cannot select channel %d out of %d\n", FEW->whichchan,
                FEW->nchans);
    }

    if ((P->UPPER_FILT_FREQ * 2) > P->SAMPLING_RATE) {

        E_WARN("Upper frequency higher than Nyquist frequency");
    }

    if (P->doublebw) {
        E_INFO("Will use double bandwidth filters\n");
    }
}

static void
fe_parse_options(fewrap_t *FEW)
{
    param_t *P;
    int32 format;
    char *endian;

    if ((P = (param_t *) malloc(sizeof(param_t))) == NULL) {
        E_FATAL("memory alloc failed in fe_parse_options()\n...exiting\n");
    }
    FEW->P = P;

    fe_init_params(P);

    FEW->wavfile = cmd_ln_str("-i");
    if (FEW->wavfile != NULL) {
        FEW->is_single = 1;
    }

    FEW->cepfile = cmd_ln_str("-o");

    FEW->ctlfile = cmd_ln_str("-c");
    if (FEW->ctlfile != NULL) {
        char *nskip;
        char *runlen;

        FEW->is_batch = 1;

        nskip = cmd_ln_str("-nskip");
        runlen = cmd_ln_str("-runlen");
        if (nskip != NULL) {
            FEW->nskip = atoi(nskip);
        }
        if (runlen != NULL) {
            FEW->runlen = atoi(runlen);
        }
    }

    FEW->wavdir = cmd_ln_str("-di");
    FEW->cepdir = cmd_ln_str("-do");
    FEW->wavext = cmd_ln_str("-ei");
    FEW->cepext = cmd_ln_str("-eo");
    format = cmd_ln_int32("-raw");
    if (format) {
        FEW->input_format = RAW;
    }
    format = cmd_ln_int32("-nist");
    if (format) {
        FEW->input_format = NIST;
    }
    format = cmd_ln_int32("-mswav");
    if (format) {
        FEW->input_format = MSWAV;
    }

    FEW->nchans = cmd_ln_int32("-nchans");
    FEW->whichchan = cmd_ln_int32("-whichchan");
    P->PRE_EMPHASIS_ALPHA = cmd_ln_float32("-alpha");
    P->SAMPLING_RATE = cmd_ln_float32("-samprate");
    P->WINDOW_LENGTH = cmd_ln_float32("-wlen");
    P->FRAME_RATE = cmd_ln_int32("-frate");

    if (!strcmp(cmd_ln_str("-feat"), "sphinx")) {
        P->FB_TYPE = MEL_SCALE;
        FEW->output_endian = BIG;
    }
    else {
        E_ERROR("MEL_SCALE IS CURRENTLY THE ONLY IMPLEMENTATION\n\n");
        E_FATAL("Make sure you specify '-feat sphinx'\n");
    }
    P->NUM_FILTERS = cmd_ln_int32("-nfilt");
    P->NUM_CEPSTRA = cmd_ln_int32("-ncep");
    P->LOWER_FILT_FREQ = cmd_ln_float32("-lowerf");
    P->UPPER_FILT_FREQ = cmd_ln_float32("-upperf");

    P->warp_type = cmd_ln_str("-warp_type");
    P->warp_params = cmd_ln_str("-warp_params");

    P->FFT_SIZE = cmd_ln_int32("-nfft");
    if (cmd_ln_int32("-doublebw")) {
        P->doublebw = 1;
    }
    else {
        P->doublebw = 0;
    }
    FEW->blocksize = cmd_ln_int32("-blocksize");
    FEW->verbose = cmd_ln_int32("-verbose");

    endian = cmd_ln_str("-machine_endian");
    if (!strcmp("big", endian)) {
        FEW->machine_endian = BIG;
    }
    else {
        if (!strcmp("little", endian)) {
            FEW->machine_endian = LITTLE;
        }
        else {
            E_FATAL("Machine must be big or little Endian\n");
        }
    }
    endian = cmd_ln_str("-input_endian");
    if (!strcmp("big", endian)) {
        FEW->input_endian = BIG;
    }
    else {
        if (!strcmp("little", endian)) {
            FEW->input_endian = LITTLE;
        }
        else {
            E_FATAL("Input must be big or little Endian\n");
        }
    }
    P->dither = strcmp("no", cmd_ln_str("-dither"));
    P->seed = cmd_ln_int32("-seed");
    P->logspec = cmd_ln_int32("-logspec");

    fe_validate_parameters(FEW);
}

static char *
fe_copystr(char *dest_str, char *src_str)
{
    int i, src_len, len;
    char *s;

    src_len = strlen(src_str);
    len = src_len;
    s = (char *) malloc(len + 1);
    for (i = 0; i < src_len; i++)
        *(s + i) = *(src_str + i);
    *(s + src_len) = NULL_CHAR;

    return (s);
}

static int32
fe_build_filenames(fewrap_t * FEW, char *fileroot, char **infilename,
                   char **outfilename)
{
    char cbuf[MAXCHARS];
    char chanlabel[MAXCHARS];

    if (FEW->nchans > 1)
        sprintf(chanlabel, ".ch%d", FEW->whichchan);

    if (FEW->is_batch) {
        assert(fileroot);
        sprintf(cbuf, "%s", "");
        strcat(cbuf, FEW->wavdir);
        strcat(cbuf, "/");
        strcat(cbuf, fileroot);
        strcat(cbuf, ".");
        strcat(cbuf, FEW->wavext);
        if (infilename != NULL) {
            *infilename = fe_copystr(*infilename, cbuf);
        }

        sprintf(cbuf, "%s", "");
        strcat(cbuf, FEW->cepdir);
        strcat(cbuf, "/");
        strcat(cbuf, fileroot);
        if (FEW->nchans > 1)
            strcat(cbuf, chanlabel);
        strcat(cbuf, ".");
        strcat(cbuf, FEW->cepext);
        if (outfilename != NULL) {
            *outfilename = fe_copystr(*outfilename, cbuf);
        }
    }
    else if (FEW->is_single) {
        /*      assert(fileroot==NULL); */
        sprintf(cbuf, "%s", "");
        strcat(cbuf, FEW->wavfile);
        if (infilename != NULL) {
            *infilename = fe_copystr(*infilename, cbuf);
        }

        sprintf(cbuf, "%s", "");
        strcat(cbuf, FEW->cepfile);
        if (outfilename != NULL) {
            *outfilename = fe_copystr(*outfilename, cbuf);
        }
    }
    else {
        E_FATAL("Unspecified Batch or Single Mode\n");
    }

    return 0;
}

static int32
fe_count_frames(fe_t * FE, int32 nsamps, int32 count_partial_frames)
{
    int32 frame_start, frame_count = 0;

    assert(FE->FRAME_SIZE != 0);
    for (frame_start = 0; frame_start + FE->FRAME_SIZE <= nsamps;
         frame_start += FE->FRAME_SHIFT) {
        frame_count++;
    }

    /* dhuggins@cs, 2006-04-25: Update this to match the updated
     * partial frame condition in fe_process_utt(). */
    if (count_partial_frames) {
        if (frame_count * FE->FRAME_SHIFT < nsamps)
            frame_count++;
    }

    return (frame_count);
}

static int32
fe_openfiles(fewrap_t * FEW, char *infile, int32 * fp_in,
             int32 * nsamps, int32 * nframes, int32 * nblocks,
             char *outfile, int32 * fp_out)
{
    struct stat filestats;
    int fp = 0, len = 0, outlen, numframes, numblocks;
    FILE *fp2;
    char line[MAXCHARS];
    int got_it = 0;
    fe_t * FE = FEW->FE;
    param_t * P = FEW->P;

    /* Note: this is kind of a hack to read the byte format from the
       NIST header */
    if (FEW->input_format == NIST) {
        if ((fp2 = fopen(infile, "rb")) == NULL) {
            E_ERROR("Cannot read %s\n", infile);
            return (FE_INPUT_FILE_READ_ERROR);
        }
        *line = 0;
        got_it = 0;
        while (strcmp(line, "end_head") && !got_it) {
            fscanf(fp2, "%s", line);
            if (!strcmp(line, "sample_byte_format")) {
                fscanf(fp2, "%s", line);
                if (!strcmp(line, "-s2")) {
                    fscanf(fp2, "%s", line);
                    if (!strcmp(line, "01")) {
                        FEW->input_endian = LITTLE;
                        got_it = 1;
                    }
                    else if (!strcmp(line, "10")) {
                        FEW->input_endian = BIG;
                        got_it = 1;
                    }
                    else
                        E_ERROR("Unknown/unsupported byte order\n");
                }
                else
                    E_ERROR("Error determining byte format\n");
            }
        }
        if (!got_it) {
            E_WARN
                ("Can't find byte format in header, setting to machine's endian\n");
            FEW->input_endian = FEW->machine_endian;
        }
        fclose(fp2);
    }
    else if (FEW->input_format == RAW) {
        /*
           FEW->input_endian = FEW->machine_endian;
         */
    }
    else if (FEW->input_format == MSWAV) {
        FEW->input_endian = LITTLE;       /* Default for MS WAV riff files */
    }


    if ((fp = open(infile, O_RDONLY | O_BINARY, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", infile);
        return (FE_INPUT_FILE_OPEN_ERROR);
    }
    else {
        if (fstat(fp, &filestats) != 0)
            printf("fstat failed\n");

        if (FEW->input_format == NIST) {
            short *hdr_buf;

            len = (filestats.st_size - HEADER_BYTES) / sizeof(short);
            /* eat header */
            hdr_buf =
                (short *) calloc(HEADER_BYTES / sizeof(short),
                                 sizeof(short));
            if (read(fp, hdr_buf, HEADER_BYTES) != HEADER_BYTES) {
                E_ERROR("Cannot read %s\n", infile);
                return (FE_INPUT_FILE_READ_ERROR);
            }
            free(hdr_buf);
        }
        else if (FEW->input_format == RAW) {
            len = filestats.st_size / sizeof(int16);
        }
        else if (FEW->input_format == MSWAV) {
            /* Read the header */
            MSWAV_hdr *hdr_buf;
            if ((hdr_buf =
                 (MSWAV_hdr *) calloc(1, sizeof(MSWAV_hdr))) == NULL) {
                E_ERROR("Cannot allocate for input file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            if (read(fp, hdr_buf, sizeof(MSWAV_hdr)) != sizeof(MSWAV_hdr)) {
                E_ERROR("Cannot allocate for input file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            /* Check header */
            if (strncmp(hdr_buf->rifftag, "RIFF", 4) != 0 ||
                strncmp(hdr_buf->wavefmttag, "WAVEfmt", 7) != 0) {
                E_ERROR("Error in mswav file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            if (strncmp(hdr_buf->datatag, "data", 4) != 0) {
                /* In this case, there are other "chunks" before the
                 * data chunk, which we can ignore. We have to find the
                 * start of the data chunk, which begins with the string
                 * "data".
                 */
                int16 found = 0;
                char readChar;
                char *dataString = "data";
                int16 charPointer = 0;
                printf("LENGTH: %d\n", strlen(dataString));
                while (!found) {
                    if (read(fp, &readChar, sizeof(char)) != sizeof(char)) {
                        E_ERROR("Failed reading wav file.\n");
                        return (FE_INPUT_FILE_READ_ERROR);
                    }
                    if (readChar == dataString[charPointer]) {
                        charPointer++;
                    }
                    if (charPointer == (int) strlen(dataString)) {
                        found = 1;
                        strcpy(hdr_buf->datatag, dataString);
                        if (read(fp, &(hdr_buf->datalength), sizeof(int32))
                            != sizeof(int32)) {
                            E_ERROR("Failed reading wav file.\n");
                            return (FE_INPUT_FILE_READ_ERROR);
                        }
                    }
                }
            }
            if (FEW->input_endian != FEW->machine_endian) { /* If machine is Big Endian */
                hdr_buf->datalength = SWAP_INT32(&(hdr_buf->datalength));
                hdr_buf->data_format = SWAP_INT16(&(hdr_buf->data_format));
                hdr_buf->numchannels = SWAP_INT16(&(hdr_buf->numchannels));
                hdr_buf->BitsPerSample = SWAP_INT16(&(hdr_buf->BitsPerSample));
                hdr_buf->SamplingFreq = SWAP_INT32(&(hdr_buf->SamplingFreq));
                hdr_buf->BytesPerSec = SWAP_INT32(&(hdr_buf->BytesPerSec));
            }
            /* Check Format */
            if (hdr_buf->data_format != 1 || hdr_buf->BitsPerSample != 16) {
                E_ERROR("MS WAV file not in 16-bit PCM format\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            len = hdr_buf->datalength / sizeof(short);
            FEW->nchans = hdr_buf->numchannels;
            /* DEBUG: Dump Info */
            if (FEW->verbose) {
                E_INFO("Reading MS Wav file %s:\n", infile);
                E_INFO("\t16 bit PCM data, %d channels %d samples\n",
                       FEW->nchans, len);
                E_INFO("\tSampled at %d\n", hdr_buf->SamplingFreq);
            }
            free(hdr_buf);
        }
        else {
            E_ERROR("Unknown input file format\n");
            return (FE_INPUT_FILE_OPEN_ERROR);
        }
    }


    len = len / FEW->nchans;
    *nsamps = len;
    *fp_in = fp;

    numblocks = (int) ((float) len / (float) FEW->blocksize);
    if (numblocks * FEW->blocksize < len)
        numblocks++;

    *nblocks = numblocks;

    /* ARCHAN 20050708, If outfile and fp_out = NULL, then we don't
       consider the output file in the process. In general, why? why
       in an open file function we need to think of the output as
       well? To avoid deep factoring, I just apply this hack for now
       to eliminate ep_fe_openfiles
     */
    if (outfile == NULL && fp_out == NULL) {

        numframes = fe_count_frames(FE, len, COUNT_PARTIAL);
        *nframes = numframes;

        return 0;
    }

    if ((fp =
         open(outfile, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
              0644)) < 0) {
        E_ERROR("Unable to open %s for writing features\n", outfile);
        return (FE_OUTPUT_FILE_OPEN_ERROR);
    }
    else {
        if (FEW->verbose) {
            E_INFO("Saving data to %s\n", outfile);
        }
        /* compute number of frames and write cepfile header */
        numframes = fe_count_frames(FE, len, COUNT_PARTIAL);
        if (!P->logspec)
            outlen = numframes * FE->NUM_CEPSTRA;
        else
            outlen = numframes * FE->MEL_FB->num_filters;
        if (FEW->output_endian != FEW->machine_endian)
            SWAP_INT32(&outlen);
        if (write(fp, &outlen, 4) != 4) {
            E_ERROR("Data write error on %s\n", outfile);
            close(fp);
            return (FE_OUTPUT_FILE_WRITE_ERROR);
        }
        if (FEW->output_endian != FEW->machine_endian)
            SWAP_INT32(&outlen);
    }

    *nframes = numframes;
    *fp_out = fp;

    return 0;
}

static int32
fe_readblock_spch(fewrap_t * FEW, int32 fp, int32 nsamps, int16 * buf)
{
    int32 bytes_read, cum_bytes_read, nreadbytes, actsamps, offset, i, j,
        k;
    int16 *tmpbuf;
    int32 nchans, whichchan;

    nchans = FEW->nchans;
    whichchan = FEW->whichchan;

    if (nchans == 1) {
        if (FEW->input_format == RAW || FEW->input_format == NIST
            || FEW->input_format == MSWAV) {
            nreadbytes = nsamps * sizeof(int16);
            if ((bytes_read = read(fp, buf, nreadbytes)) != nreadbytes) {
                E_ERROR("error reading block\n");
                return (0);
            }
        }
        else {
            E_ERROR("unknown input file format\n");
            return (0);
        }
        cum_bytes_read = bytes_read;
    }
    else if (nchans > 1) {

        if (nsamps < FEW->blocksize) {
            actsamps = nsamps * nchans;
            tmpbuf = (int16 *) calloc(nsamps * nchans, sizeof(int16));
            cum_bytes_read = 0;
            if (FEW->input_format == RAW || FEW->input_format == NIST) {

                k = 0;
                nreadbytes = actsamps * sizeof(int16);

                if ((bytes_read =
                     read(fp, tmpbuf, nreadbytes)) != nreadbytes) {
                    E_ERROR("error reading block (got %d not %d)\n",
                            bytes_read, nreadbytes);
                    return (0);
                }

                for (j = whichchan - 1; j < actsamps; j = j + nchans) {
                    buf[k] = tmpbuf[j];
                    k++;
                }
                cum_bytes_read += bytes_read / nchans;
            }
            else {
                E_ERROR("unknown input file format\n");
                return (0);
            }
            free(tmpbuf);
        }
        else {
            tmpbuf = (int16 *) calloc(nsamps, sizeof(int16));
            actsamps = nsamps / nchans;
            cum_bytes_read = 0;

            if (actsamps * nchans != nsamps) {
                E_WARN
                    ("Blocksize %d is not an integer multiple of Number of channels %d\n",
                     nsamps, nchans);
            }

            if (FEW->input_format == RAW || FEW->input_format == NIST) {
                for (i = 0; i < nchans; i++) {

                    offset = i * actsamps;
                    k = 0;
                    nreadbytes = nsamps * sizeof(int16);

                    if ((bytes_read =
                         read(fp, tmpbuf, nreadbytes)) != nreadbytes) {
                        E_ERROR("error reading block (got %d not %d)\n",
                                bytes_read, nreadbytes);
                        return (0);
                    }

                    for (j = whichchan - 1; j < nsamps; j = j + nchans) {
                        buf[offset + k] = tmpbuf[j];
                        k++;
                    }
                    cum_bytes_read += bytes_read / nchans;
                }
            }
            else {
                E_ERROR("unknown input file format\n");
                return (0);
            }
            free(tmpbuf);
        }
    }

    else {
        E_ERROR("unknown number of channels!\n");
        return (0);
    }

    if (FEW->input_endian != FEW->machine_endian) {
        for (i = 0; i < nsamps; i++)
            SWAP_INT16(&buf[i]);
    }

    return (cum_bytes_read / sizeof(int16));

}

static int16 *
fe_convert_files_to_spdata(fewrap_t *FEW, int32 * splenp,
                           int32 * nframesp)
{
    char *infile = NULL, fileroot[MAXCHARS];
    int32 splen = 0, total_samps = 0, frames_proc, nframes = 0, nblocks = 0;
    int32 fp_in = 0, last_blocksize = 0, curr_block, total_frames;
    int16 *spdata;
    fe_t *FE = FEW->FE;

    spdata = NULL;

    /* 20040917: ARCHAN The fe_copy_str and free pair is very fishy
       here. If the number of utterance being processed is larger than
       1M, there may be chance, we will hit out of segment problem. */

    if (FEW->is_single) {

        fe_build_filenames(FEW, fileroot, &infile, NULL);
        if (FEW->verbose)
            printf("%s\n", infile);

        if (fe_openfiles
            (FEW, infile, &fp_in, &total_samps, &nframes, &nblocks, NULL,
             NULL) != FE_SUCCESS) {
            E_FATAL("fe_openfiles exited!\n");
        }

        if (FEW->blocksize < total_samps) {
            E_FATAL
                ("Block size (%d) has to be at least the number of samples in the file (%d)\n",
                 FEW->blocksize, total_samps);
        }

        if (nblocks * FEW->blocksize >= total_samps)
            last_blocksize = total_samps - (nblocks - 1) * FEW->blocksize;

        if (!fe_start_utt(FE)) {
            curr_block = 1;
            total_frames = frames_proc = 0;
            /*printf("Total frames %d, last_blocksize: %d\n", total_frames, last_blocksize); */

            /* process last (or only) block */
            if (spdata != NULL)
                free(spdata);
            splen = last_blocksize;
            if ((spdata = (int16 *) calloc(splen, sizeof(int16))) == NULL) {
                E_FATAL
                    ("Unable to allocate memory block of %d shorts for input speech\n",
                     splen);
            }
            if (fe_readblock_spch(FEW, fp_in, splen, spdata) != splen) {
                E_FATAL("Error reading speech data\n");
            }

            close(fp_in);

        }
        else {
            E_FATAL("fe_start_utt() failed\n");
        }

    }
    else {
        E_FATAL("Unknown mode - single or batch?\n");
    }

    ckd_free(infile);


    *splenp = splen;
    *nframesp = nframes;
    return (spdata);
}

void process_fe_class(fewrap_t * FEW, class_t * CLASSW,
                      endpointer_t * ENDPTR, int16 * spbuffer,
                      int32 splen);

/*       
	26-Aug-04 Archan - modified the code to make it incorporate sphinx's library.  
	24-Jun-04 Z. Al Bawab - modified the code to integrate with the Meeting 
	recorder.
	 
	9-Mar-04 Z. Al Bawab - modified the code to perform frame by frame 
   	 features extraction and classifcation.

	 7-Feb-00 M. Seltzer - wrapper created for new front end -
	 does blockstyle processing if necessary. If input stream is
	 greater than DEFAULT_BLOCKSIZE samples (currently 200000)
	 then it will read and write in DEFAULT_BLOCKSIZE chunks. 
	 
	 Had to change fe_process_utt(). Now the 2d feature array
	 is allocated internally to that function rather than
	 externally in the wrapper. 
	 
	 Added usage display with -help switch for help

	 14-Feb-00 M. Seltzer - added NIST header parsing for 
	 big endian/little endian parsing. kind of a hack.

	 changed -wav switch to -nist to avoid future confusion with
	 MS wav files
	 
	 added -mach_endian switch to specify machine's byte format
*/

void
process_fe_class(fewrap_t * FEW, class_t * CLASSW, endpointer_t * ENDPTR,
                 int16 * spbuffer, int32 splen)
{

    int32 spbuf_len, offset, nsamps, frame_count, frame_start;
    int16 *spdata;
    int i, myclass, postclass, wincap, endutt;
    int classcount[NUMCLASSES];
    mgau_model_t *mgau;

  /*****************************************************************/
  /********************** INITIALIZING COMPONENTS ******************/
  /*****************************************************************/

    endutt = 0;

    wincap = 0;                 /* voting window current capacity */

    for (i = 0; i < NUMCLASSES; i++)
        classcount[i] = 0;

    mgau = CLASSW->g;

  /***************** DONE WITH INITIALIZATIONS ********************/
  /****************************************************************/

    nsamps = splen;
    /*spdata = spbuffer; */

    E_INFO("%d samples, %d overflow, %d frame size\n", nsamps,
           FEW->FE->NUM_OVERFLOW_SAMPS, FEW->FE->FRAME_SIZE);

    /* are there enough samples to make at least 1 frame? */
    if (nsamps + FEW->FE->NUM_OVERFLOW_SAMPS >= FEW->FE->FRAME_SIZE) {
      /****************************************************************/
      /****** Prepend any overflow data from last call   **************/
      /****************************************************************/

        /* if there are previous samples, pre-pend them to input speech samps */
        if ((FEW->FE->NUM_OVERFLOW_SAMPS > 0)) {
            if ((spdata =
                 (int16 *) malloc(sizeof(int16) *
                                  (FEW->FE->NUM_OVERFLOW_SAMPS +
                                   nsamps))) == NULL) {
                E_FATAL
                    ("memory alloc failed in process_fe_class()...exiting\n");
            }
            /* RAH */
            memcpy(spdata, FEW->FE->OVERFLOW_SAMPS,
                   FEW->FE->NUM_OVERFLOW_SAMPS * (sizeof(int16)));
            memcpy(spdata + FEW->FE->NUM_OVERFLOW_SAMPS, spbuffer,
                   nsamps * (sizeof(int16)));
            nsamps += FEW->FE->NUM_OVERFLOW_SAMPS;
            FEW->FE->NUM_OVERFLOW_SAMPS = 0;    /*reset overflow samps count */
        }
        else {
            if ((spdata =
                 (int16 *) malloc(sizeof(int16) * nsamps)) == NULL) {
                E_FATAL
                    ("memory alloc failed in fe_process_utt()...exiting\n");
            }
            /* RAH */
            E_INFO("number of samples %d\n", nsamps);
            memcpy(spdata, spbuffer, nsamps * (sizeof(int16)));
        }

      /****************************************************************/
      /**************** Start the processing **************************/
      /****************************************************************/

        /* initialize the frame_count and frame_start each time a new buffer of speech is available */
        frame_count = 0;
        frame_start = 0;

        while (frame_start + FEW->FE->FRAME_SIZE <= nsamps) {
            for (i = 0; i < FEW->FE->FRAME_SIZE; i++)
                FEW->fr_data[i] = spdata[frame_start + i];

            fe_process_frame(FEW->FE, FEW->fr_data, FEW->FE->FRAME_SIZE,
                             FEW->fr_cep);

            FEW->FE->FRAME_COUNTER++;
            /*              printf("\nFrame: %d Frame_start: %d Frame_end: %d\n",FEW->FE->FRAME_COUNTER,frame_start,frame_start+FEW->FE->FRAME_SIZE); */
            /*Do CMN HERE */

            /*for(i = 0; i < FEW->FE->NUM_CEPSTRA; i++)
               printf ("cepblock[%d][%d]: %f\n",frame_count,i,FEW->fr_cep[i]);                      
               fr_cep[i] = cepblock[frame_count][i];                        
               cmn_prior(&fr_cep, varnorm, 1, DIMENSIONS, endutt); */

            myclass =
                classify(FEW->fr_cep, mgau, CLASSW->priors,
                         CLASSW->classmap);

            if (CLASSW->postprocess == 1) {
                postclass =
                    postclassify(CLASSW->window, CLASSW->windowlen,
                                 &wincap, myclass);
            }
            else {
                postclass = myclass;
            }
            endpointer_update_stat(ENDPTR, FEW->FE, CLASSW, postclass);

#if 0
            switch (postclass) {
            case 0:
                printf(" N");
                classcount[0]++;
                break;
            case 1:
                printf(" O");
                classcount[1]++;
                break;
            case 2:
                printf(" S");
                classcount[2]++;
                break;
            case 3:
                printf(" SIL");
                classcount[3]++;
                break;
            }
            printf("\n");
#endif

            frame_count++;      /* update the frame counter */
            frame_start += FEW->FE->FRAME_SHIFT;        /* update the frame shift */

        }

      /****************************************************************/
      /**************** end the processing **************************/
      /****************************************************************/
      /****************************************************************/
      /********** store new overflow data within the FE  **************/
      /****************************************************************/

        /* Calculate the number of samples processed */
        spbuf_len =
            (frame_count - 1) * FEW->FE->FRAME_SHIFT + FEW->FE->FRAME_SIZE;

        /* assign samples which don't fit an entire frame to FE overflow buffer for use on next pass */
        if (spbuf_len < nsamps) {
            /*printf("spbuf_len: %d\n",spbuf_len); */
            offset = ((frame_count) * FEW->FE->FRAME_SHIFT);
            memcpy(FEW->FE->OVERFLOW_SAMPS, spdata + offset,
                   (nsamps - offset) * sizeof(int16));
            FEW->FE->NUM_OVERFLOW_SAMPS = nsamps - offset;
            FEW->FE->PRIOR = spdata[offset - 1];

            /* assert FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE */
            assert(FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE);
        }
        free(spdata);           /* close if there are enough samples for 1 frame */
    }
    else {                      /* if not enough total samps for a single frame,
                                   append new samps to previously stored overlap samples */
        memcpy(FEW->FE->OVERFLOW_SAMPS + FEW->FE->NUM_OVERFLOW_SAMPS,
               spbuffer, nsamps * (sizeof(int16)));
        FEW->FE->NUM_OVERFLOW_SAMPS += nsamps;
        /*printf("saved : %d\n samples in the OVERFLOW buffer",FEW->FE->NUM_OVERFLOW_SAMPS);                       */
        assert(FEW->FE->NUM_OVERFLOW_SAMPS < FEW->FE->FRAME_SIZE);
        frame_count = 0;
    }
}

int32
main(int32 argc, char **argv)
{
    fewrap_t *FEW;
    class_t *CLASSW;
    endpointer_t *ENDPTR;
    ptmr_t tm_class;
    int32 splen, nframes;
    int16 *spbuffer;


    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", arg);

    if (cmd_ln_int32("-logspec"))
        E_FATAL("-logspec is currently not supported\n");

    if (strcmp(cmd_ln_str("-o"), NULL_DEVICE) != 0) {
        E_FATAL
            ("Output file not used by ep: code outputs only timing info.\n");
    }

    if (cmd_ln_str("-c") ||
        cmd_ln_str("-di") ||
        cmd_ln_str("-do") || cmd_ln_str("-ei") || cmd_ln_str("-eo")) {
        E_FATAL
            ("Only one file processed at a time, control file not allowed\n");
    }

    unlimit();
    ptmr_init(&tm_class);

    logs3_init(cmd_ln_float32("-logbase"), 1, cmd_ln_int32("-log3table"));

    FEW = few_initialize();
    CLASSW = classw_initialize(cmd_ln_str("-mdef"), cmd_ln_str("-mean"),
                               cmd_ln_str("-var"),
                               cmd_ln_float32("-varfloor"),
                               cmd_ln_str("-mixw"),
                               cmd_ln_float32("-mixwfloor"), TRUE,
                               ".cont.");
    ENDPTR =
        endpointer_initialize(FEW->FE, cmd_ln_float32("-pad_before"),
                              cmd_ln_float32("-pad_after"),
                              cmd_ln_float32("-speech_start"),
                              cmd_ln_float32("-speech_end"),
                              cmd_ln_float32("-speech_cancel"));

    spbuffer =
        fe_convert_files_to_spdata(FEW, &splen, &nframes);

    ptmr_start(&tm_class);
    process_fe_class(FEW, CLASSW, ENDPTR, spbuffer, splen);
    ptmr_stop(&tm_class);

    E_INFO
        ("Time used: %6.3f sec CPU, %6.3f sec Clk;  TOT: %8.3f sec CPU, %8.3f sec Clk\n\n",
         tm_class.t_cpu, tm_class.t_elapsed, tm_class.t_tot_cpu,
         tm_class.t_tot_elapsed);

  /** free the stuff for the front-end wraper */
    few_free(FEW);

  /** free the stuff for the class wraper */
    classw_free(CLASSW);
    endpointer_free(ENDPTR);

    cmd_ln_appl_exit();

    return (0);
}
