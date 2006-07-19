/* ====================================================================
 * Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
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
 * fe.c -- Feature vector description and cepstra->feature computation.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: fe.c,v $
 * Revision 1.11  2006/03/03 20:02:37  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 *
 * Revision 1.10  2006/02/23 03:53:02  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added fe_convert_one_file
 * 2, Use fe_convert_one_file twice in fe_convert_files instead of repeating 200 lines of code twice.
 * 3, Replaced -srate by -samprate
 * 4, Replaced -mach_endian by -machine_endian
 * 5, eliminate ep_fe_openfiles.
 * 6, Fixed dox-doc.
 *
 * Revision 1.9.4.3  2005/07/18 19:07:42  arthchan2003
 * 1, Added keyword , 2, Remove unnecessry E_INFO, 3, resolved conflicts in command-line names between wave2feat/ep and decode,  because both ep and wave2feat are relatively new, both follow decode's convention, now call -mach_endian to be -machine_endian, -srate to be -samprate. 4, assert, FRAME_SIZE not equal to 0, in fe_count_frame, if not that could cause infinite loop.
 *
 *
 */

#include "fe.h"

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#if defined(WIN32)

#include <io.h>
#include <errno.h>

/*Now using customized random generator. */
/*#define srand48(x) srand(x)
  #define lrand48() rand()*/
#endif



/*** Function to initialize the front-end wrapper ***/
fewrap_t *
few_initialize()
{
    fewrap_t *FEW = (fewrap_t *) ckd_calloc(1, sizeof(fewrap_t));

    /********************** INITIALIZING COMPONENTS ******************/

    /* initialize parameters */
    FEW->P = fe_parse_options();

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
        fe_free_param(FEW->P);

        free(FEW);
    }

}

/** 
 * 
 */
static int32
fe_convert_one_file(param_t * P,             /**< A parameter structure */
                    fe_t * FE,                /**< A FE parameter structure */
                    char *fileroot              /**< the file root*/
    )
{
    char *infile, *outfile;
    int16 *spdata = NULL;

    int32 splen, total_samps, frames_proc, nframes, nblocks, last_frame;
    int32 fp_in, fp_out, last_blocksize = 0, curr_block, total_frames;
    float32 **cep = NULL, **last_frame_cep;
    int32 return_value;
    int32 warn_zero_energy = OFF;
    int32 process_utt_return_value;

    fe_build_filenames(P, fileroot, &infile, &outfile);

    if (P->verbose)
        E_INFO("%s\n", infile);

    return_value =
        fe_openfiles(P, FE, infile, &fp_in, &total_samps, &nframes,
                     &nblocks, outfile, &fp_out);
    if (return_value != FE_SUCCESS) {
        return (return_value);
    }

    warn_zero_energy = OFF;

    if (nblocks * P->blocksize >= total_samps)
        last_blocksize = total_samps - (nblocks - 1) * P->blocksize;

    if (!fe_start_utt(FE)) {
        curr_block = 1;
        total_frames = frames_proc = 0;
        /*execute this loop only if there is more than 1 block to
           be processed */
        while (curr_block < nblocks) {
            splen = P->blocksize;
            if ((spdata = (int16 *) calloc(splen, sizeof(int16))) == NULL) {
                E_ERROR
                    ("Unable to allocate memory block of %d shorts for input speech\n",
                     splen);
                return (FE_MEM_ALLOC_ERROR);
            }
            if (fe_readblock_spch(P, fp_in, splen, spdata) != splen) {
                E_ERROR("Error reading speech data\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            process_utt_return_value =
                fe_process_utt(FE, spdata, splen, &cep, &frames_proc);
            if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
                warn_zero_energy = ON;
            }
            else {
                assert(process_utt_return_value == FE_SUCCESS);
            }
            if (frames_proc > 0)
                fe_writeblock_feat(P, FE, fp_out, frames_proc, cep);
            ckd_free_2d((void **) cep);
            curr_block++;
            total_frames += frames_proc;
            if (spdata != NULL) {
                free(spdata);
                spdata = NULL;
            }
        }
        /* process last (or only) block */
        if (spdata != NULL) {
            free(spdata);
        }
        splen = last_blocksize;

        if ((spdata = (int16 *) calloc(splen, sizeof(int16))) == NULL) {
            E_ERROR
                ("Unable to allocate memory block of %d shorts for input speech\n",
                 splen);
            return (FE_MEM_ALLOC_ERROR);
        }

        if (fe_readblock_spch(P, fp_in, splen, spdata) != splen) {
            E_ERROR("error reading speech data\n");
            return (FE_INPUT_FILE_READ_ERROR);
        }

        process_utt_return_value =
            fe_process_utt(FE, spdata, splen, &cep, &frames_proc);
        if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
            warn_zero_energy = ON;
        }
        else {
            assert(process_utt_return_value == FE_SUCCESS);
        }
        if (frames_proc > 0)
            fe_writeblock_feat(P, FE, fp_out, frames_proc, cep);
        ckd_free_2d((void **) cep);
        curr_block++;
        if (P->logspec != ON)
            last_frame_cep =
                (float32 **) ckd_calloc_2d(1, FE->NUM_CEPSTRA,
                                           sizeof(float32));
        else
            last_frame_cep =
                (float32 **) ckd_calloc_2d(1, FE->MEL_FB->num_filters,
                                           sizeof(float32));
        process_utt_return_value =
            fe_end_utt(FE, last_frame_cep[0], &last_frame);
        if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
            warn_zero_energy = ON;
        }
        else {
            assert(process_utt_return_value == FE_SUCCESS);
        }
        if (last_frame > 0) {
            fe_writeblock_feat(P, FE, fp_out, last_frame, last_frame_cep);
            frames_proc++;
        }
        total_frames += frames_proc;

        fe_closefiles(fp_in, fp_out);
        free(spdata);
        spdata = 0;
        ckd_free_2d((void **) last_frame_cep);

    }
    else {
        E_ERROR("fe_start_utt() failed\n");
        return (FE_START_ERROR);
    }
    if (ON == warn_zero_energy && !P->dither) {
        E_WARN
            ("File %s has some frames with zero energy. Consider using dither\n",
             infile);
    }

    return (FE_SUCCESS);
}

int32
fe_convert_files(param_t * P)
{
    fe_t *FE;
    char fileroot[MAXCHARS];
    FILE *ctlfile;

    if ((FE = fe_init(P)) == NULL) {
        E_ERROR("Initialization failed...exiting\n");
        return (FE_MEM_ALLOC_ERROR);
    }

    if (P->is_batch) {
        if ((ctlfile = fopen(P->ctlfile, "r")) == NULL) {
            E_ERROR("Unable to open control file %s\n", P->ctlfile);
            return (FE_CONTROL_FILE_ERROR);
        }
        while (fscanf(ctlfile, "%s", fileroot) != EOF) {
            fe_convert_one_file(P, FE, fileroot);
        }
    }
    else if (P->is_single)
        fe_convert_one_file(P, FE, NULL);
    else {
        E_ERROR("Unknown mode - single or batch?\n");
        return (FE_UNKNOWN_SINGLE_OR_BATCH);
    }

    fe_close(FE);
    return (FE_SUCCESS);

}

int16 *
fe_convert_files_to_spdata(param_t * P, fe_t * FE, int32 * splenp,
                           int32 * nframesp)
{
    char *infile, fileroot[MAXCHARS];
    int32 splen = 0, total_samps, frames_proc, nframes, nblocks;
    int32 fp_in, last_blocksize = 0, curr_block, total_frames;
    int16 *spdata;

    spdata = NULL;

    /* 20040917: ARCHAN The fe_copy_str and free pair is very fishy
       here. If the number of utterance being processed is larger than
       1M, there may be chance, we will hit out of segment problem. */

    if (P->is_single) {

        fe_build_filenames(P, fileroot, &infile, NULL);
        if (P->verbose)
            printf("%s\n", infile);

        if (fe_openfiles
            (P, FE, infile, &fp_in, &total_samps, &nframes, &nblocks, NULL,
             NULL) != FE_SUCCESS) {
            E_FATAL("fe_openfiles exited!\n");
        }

        if (P->blocksize < total_samps) {
            E_FATAL
                ("Block size (%d) has to be at least the number of samples in the file (%d)\n",
                 P->blocksize, total_samps);
        }

        if (nblocks * P->blocksize >= total_samps)
            last_blocksize = total_samps - (nblocks - 1) * P->blocksize;

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
            if (fe_readblock_spch(P, fp_in, splen, spdata) != splen) {
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


/** Validate the param_t function. 
 */
static void
fe_validate_parameters(param_t * P            /**< A parameter structure */
    )
{

    if ((P->is_batch) && (P->is_single)) {
        E_FATAL
            ("You cannot define an input file and a control file at the same time.\n");
    }

    if (P->wavfile == NULL && P->wavdir == NULL) {
        E_FATAL("No input file or file directory given\n");
    }

    if (P->cepfile == NULL && P->cepdir == NULL) {
        E_FATAL("No cepstra file or file directory given\n");
    }

    if (P->ctlfile == NULL && P->cepfile == NULL && P->wavfile == NULL) {
        E_FATAL("No control file given\n");
    }

    if (P->nchans > 1) {
        E_INFO("Files have %d channels of data\n", P->nchans);
        E_INFO("Will extract features for channel %d\n", P->whichchan);
    }

    if (P->whichchan > P->nchans) {
        E_FATAL("You cannot select channel %d out of %d\n", P->whichchan,
                P->nchans);
    }

    if ((P->UPPER_FILT_FREQ * 2) > P->SAMPLING_RATE) {

        E_WARN("Upper frequency higher than Nyquist frequency");
    }

    if (P->doublebw == ON) {
        E_INFO("Will use double bandwidth filters\n");
    }
}


param_t *
fe_parse_options()
{
    param_t *P;
    int32 format;
    char *endian;

    if ((P = (param_t *) malloc(sizeof(param_t))) == NULL) {
        E_FATAL("memory alloc failed in fe_parse_options()\n...exiting\n");
    }

    fe_init_params(P);

    P->wavfile = cmd_ln_str("-i");
    if (P->wavfile != NULL) {
        P->is_single = ON;
    }

    P->cepfile = cmd_ln_str("-o");

    P->ctlfile = cmd_ln_str("-c");
    if (P->ctlfile != NULL) {
        char *nskip;
        char *runlen;

        P->is_batch = ON;

        nskip = cmd_ln_str("-nskip");
        runlen = cmd_ln_str("-runlen");
        if (nskip != NULL) {
            P->nskip = atoi(nskip);
        }
        if (runlen != NULL) {
            P->runlen = atoi(runlen);
        }
    }

    P->wavdir = cmd_ln_str("-di");
    P->cepdir = cmd_ln_str("-do");
    P->wavext = cmd_ln_str("-ei");
    P->cepext = cmd_ln_str("-eo");
    format = cmd_ln_int32("-raw");
    if (format) {
        P->input_format = RAW;
    }
    format = cmd_ln_int32("-nist");
    if (format) {
        P->input_format = NIST;
    }
    format = cmd_ln_int32("-mswav");
    if (format) {
        P->input_format = MSWAV;
    }

    P->nchans = cmd_ln_int32("-nchans");
    P->whichchan = cmd_ln_int32("-whichchan");
    P->PRE_EMPHASIS_ALPHA = cmd_ln_float32("-alpha");
    P->SAMPLING_RATE = cmd_ln_float32("-samprate");
    P->WINDOW_LENGTH = cmd_ln_float32("-wlen");
    P->FRAME_RATE = cmd_ln_int32("-frate");

    if (!strcmp(cmd_ln_str("-feat"), "sphinx")) {
        P->FB_TYPE = MEL_SCALE;
        P->output_endian = BIG;
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
        P->doublebw = ON;
    }
    else {
        P->doublebw = OFF;
    }
    P->blocksize = cmd_ln_int32("-blocksize");
    P->verbose = cmd_ln_int32("-verbose");

    endian = cmd_ln_str("-machine_endian");
    if (!strcmp("big", endian)) {
        P->machine_endian = BIG;
    }
    else {
        if (!strcmp("little", endian)) {
            P->machine_endian = LITTLE;
        }
        else {
            E_FATAL("Machine must be big or little Endian\n");
        }
    }
    endian = cmd_ln_str("-input_endian");
    if (!strcmp("big", endian)) {
        P->input_endian = BIG;
    }
    else {
        if (!strcmp("little", endian)) {
            P->input_endian = LITTLE;
        }
        else {
            E_FATAL("Input must be big or little Endian\n");
        }
    }
    P->dither = strcmp("no", cmd_ln_str("-dither"));
    P->seed = cmd_ln_int32("-seed");
    P->logspec = cmd_ln_int32("-logspec");

    fe_validate_parameters(P);

    return (P);

}


int32
fe_build_filenames(param_t * P, char *fileroot, char **infilename,
                   char **outfilename)
{
    char cbuf[MAXCHARS];
    char chanlabel[MAXCHARS];

    if (P->nchans > 1)
        sprintf(chanlabel, ".ch%d", P->whichchan);

    if (P->is_batch) {
        assert(fileroot);
        sprintf(cbuf, "%s", "");
        strcat(cbuf, P->wavdir);
        strcat(cbuf, "/");
        strcat(cbuf, fileroot);
        strcat(cbuf, ".");
        strcat(cbuf, P->wavext);
        if (infilename != NULL) {
            *infilename = fe_copystr(*infilename, cbuf);
        }

        sprintf(cbuf, "%s", "");
        strcat(cbuf, P->cepdir);
        strcat(cbuf, "/");
        strcat(cbuf, fileroot);
        if (P->nchans > 1)
            strcat(cbuf, chanlabel);
        strcat(cbuf, ".");
        strcat(cbuf, P->cepext);
        if (outfilename != NULL) {
            *outfilename = fe_copystr(*outfilename, cbuf);
        }
    }
    else if (P->is_single) {
        /*      assert(fileroot==NULL); */
        sprintf(cbuf, "%s", "");
        strcat(cbuf, P->wavfile);
        if (infilename != NULL) {
            *infilename = fe_copystr(*infilename, cbuf);
        }

        sprintf(cbuf, "%s", "");
        strcat(cbuf, P->cepfile);
        if (outfilename != NULL) {
            *outfilename = fe_copystr(*outfilename, cbuf);
        }
    }
    else {
        E_FATAL("Unspecified Batch or Single Mode\n");
    }

    return 0;
}


char *
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

int32
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

int32
fe_openfiles(param_t * P, fe_t * FE, char *infile, int32 * fp_in,
             int32 * nsamps, int32 * nframes, int32 * nblocks,
             char *outfile, int32 * fp_out)
{
    struct stat filestats;
    int fp = 0, len = 0, outlen, numframes, numblocks;
    FILE *fp2;
    char line[MAXCHARS];
    int got_it = 0;


    /* Note: this is kind of a hack to read the byte format from the
       NIST header */
    if (P->input_format == NIST) {
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
                        P->input_endian = LITTLE;
                        got_it = 1;
                    }
                    else if (!strcmp(line, "10")) {
                        P->input_endian = BIG;
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
            P->input_endian = P->machine_endian;
        }
        fclose(fp2);
    }
    else if (P->input_format == RAW) {
        /*
           P->input_endian = P->machine_endian;
         */
    }
    else if (P->input_format == MSWAV) {
        P->input_endian = LITTLE;       /* Default for MS WAV riff files */
    }


    if ((fp = open(infile, O_RDONLY | O_BINARY, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", infile);
        return (FE_INPUT_FILE_OPEN_ERROR);
    }
    else {
        if (fstat(fp, &filestats) != 0)
            printf("fstat failed\n");

        if (P->input_format == NIST) {
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
        else if (P->input_format == RAW) {
            len = filestats.st_size / sizeof(int16);
        }
        else if (P->input_format == MSWAV) {
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
                int16 found = OFF;
                char readChar;
                char *dataString = "data";
                int16 charPointer = 0;
                printf("LENGTH: %d\n", strlen(dataString));
                while (found != ON) {
                    if (read(fp, &readChar, sizeof(char)) != sizeof(char)) {
                        E_ERROR("Failed reading wav file.\n");
                        return (FE_INPUT_FILE_READ_ERROR);
                    }
                    if (readChar == dataString[charPointer]) {
                        charPointer++;
                    }
                    if (charPointer == (int) strlen(dataString)) {
                        found = ON;
                        strcpy(hdr_buf->datatag, dataString);
                        if (read(fp, &(hdr_buf->datalength), sizeof(int32))
                            != sizeof(int32)) {
                            E_ERROR("Failed reading wav file.\n");
                            return (FE_INPUT_FILE_READ_ERROR);
                        }
                    }
                }
            }
            if (P->input_endian != P->machine_endian) { /* If machine is Big Endian */
                hdr_buf->datalength = SWAPL(&(hdr_buf->datalength));
                hdr_buf->data_format = SWAPW(&(hdr_buf->data_format));
                hdr_buf->numchannels = SWAPW(&(hdr_buf->numchannels));
                hdr_buf->BitsPerSample = SWAPW(&(hdr_buf->BitsPerSample));
                hdr_buf->SamplingFreq = SWAPL(&(hdr_buf->SamplingFreq));
                hdr_buf->BytesPerSec = SWAPL(&(hdr_buf->BytesPerSec));
            }
            /* Check Format */
            if (hdr_buf->data_format != 1 || hdr_buf->BitsPerSample != 16) {
                E_ERROR("MS WAV file not in 16-bit PCM format\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            len = hdr_buf->datalength / sizeof(short);
            P->nchans = hdr_buf->numchannels;
            /* DEBUG: Dump Info */
            if (P->verbose) {
                E_INFO("Reading MS Wav file %s:\n", infile);
                E_INFO("\t16 bit PCM data, %d channels %d samples\n",
                       P->nchans, len);
                E_INFO("\tSampled at %d\n", hdr_buf->SamplingFreq);
            }
            free(hdr_buf);
        }
        else {
            E_ERROR("Unknown input file format\n");
            return (FE_INPUT_FILE_OPEN_ERROR);
        }
    }


    len = len / P->nchans;
    *nsamps = len;
    *fp_in = fp;

    numblocks = (int) ((float) len / (float) P->blocksize);
    if (numblocks * P->blocksize < len)
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
        if (P->verbose) {
            E_INFO("Saving data to %s\n", outfile);
        }
        /* compute number of frames and write cepfile header */
        numframes = fe_count_frames(FE, len, COUNT_PARTIAL);
        if (P->logspec != ON)
            outlen = numframes * FE->NUM_CEPSTRA;
        else
            outlen = numframes * FE->MEL_FB->num_filters;
        if (P->output_endian != P->machine_endian)
            SWAPL(&outlen);
        if (write(fp, &outlen, 4) != 4) {
            E_ERROR("Data write error on %s\n", outfile);
            close(fp);
            return (FE_OUTPUT_FILE_WRITE_ERROR);
        }
        if (P->output_endian != P->machine_endian)
            SWAPL(&outlen);
    }

    *nframes = numframes;
    *fp_out = fp;

    return 0;
}

int32
fe_readblock_spch(param_t * P, int32 fp, int32 nsamps, int16 * buf)
{
    int32 bytes_read, cum_bytes_read, nreadbytes, actsamps, offset, i, j,
        k;
    int16 *tmpbuf;
    int32 nchans, whichchan;

    nchans = P->nchans;
    whichchan = P->whichchan;

    if (nchans == 1) {
        if (P->input_format == RAW || P->input_format == NIST
            || P->input_format == MSWAV) {
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

        if (nsamps < P->blocksize) {
            actsamps = nsamps * nchans;
            tmpbuf = (int16 *) calloc(nsamps * nchans, sizeof(int16));
            cum_bytes_read = 0;
            if (P->input_format == RAW || P->input_format == NIST) {

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

            if (P->input_format == RAW || P->input_format == NIST) {
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

    if (P->input_endian != P->machine_endian) {
        for (i = 0; i < nsamps; i++)
            SWAPW(&buf[i]);
    }

    return (cum_bytes_read / sizeof(int16));

}

int32
fe_writeblock_feat(param_t * P, fe_t * FE, int32 fp, int32 nframes,
                   float32 ** feat)
{

    int32 i, length, nwritebytes;

    if (P->logspec == ON)
        length = nframes * FE->MEL_FB->num_filters;
    else
        length = nframes * FE->NUM_CEPSTRA;

    if (P->output_endian != P->machine_endian) {
        for (i = 0; i < length; ++i)
            SWAPF(feat[0] + i);
    }

    nwritebytes = length * sizeof(float32);
    if (write(fp, feat[0], nwritebytes) != nwritebytes) {
        close(fp);
        E_FATAL("Error writing block of features\n");
    }

    if (P->verbose) {
        E_INFO("Saved %d floats\n", length);
    }

    if (P->output_endian != P->machine_endian) {
        for (i = 0; i < length; ++i)
            SWAPF(feat[0] + i);
    }

    return (length);
}


int32
fe_closefiles(int32 fp_in, int32 fp_out)
{
    close(fp_in);
    close(fp_out);
    return 0;
}

int32
fe_free_param(param_t * P)
{

    /* but no one calls it (29/09/00) */
    return 0;
}
