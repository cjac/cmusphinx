/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * CMU CALO Speech Project
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * 17-Jun-2004  Ziad Al Bawab (ziada@cs.cmu.edu) at Carnegie Mellon University
 * Created
 */


#include "fe.h"
#include "fe_internal.h"
#include "classify.h"
#include "endptr.h"


#ifdef __cplusplus
 extern "C" {
 #endif 

#define ON 1
#define OFF 0
#define NULL_CHAR '\0'
#define MAXCHARS 2048

#define WAV 1
#define RAW 2
#define NIST 3
#define MSWAV 4

#define ONE_CHAN 1

#define LITTLE 1
#define BIG 2

#define FE_SUCCESS 0
#define FE_OUTPUT_FILE_SUCCESS 0
#define FE_CONTROL_FILE_ERROR 1
#define FE_START_ERROR 2
#define FE_UNKNOWN_SINGLE_OR_BATCH 3
#define FE_INPUT_FILE_OPEN_ERROR 4
#define FE_INPUT_FILE_READ_ERROR 5
#define FE_INPUT_FILE_ALLOC_MEM_ERROR 6
#define FE_OUTPUT_FILE_WRITE_ERROR 7
#define FE_OUTPUT_FILE_OPEN_ERROR 8

#define COUNT_PARTIAL 1
#define COUNT_WHOLE 0
#define HEADER_BYTES 1024
/*
  #if defined(ALPHA) || defined(ALPHA_OSF1) || defined(alpha_osf1) || defined(__alpha) || defined(mips) 
*/
/*#define SWAPBYTES*/
#define SWAPW(x)        *(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
#define SWAPL(x)        *(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
                        (0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))
#define SWAPF(x)        SWAPL((int *) x)


/* Some defines for MS Wav Files */
/* The MS Wav file is a RIFF file, and has the following 44 byte header */
typedef struct RIFFHeader{
    char rifftag[4];      /* "RIFF" string */
    int32 TotalLength;      /* Total length */
    char wavefmttag[8];   /* "WAVEfmt " string (note space after 't') */
    int32 RemainingLength;  /* Remaining length */
    int16 data_format;    /* data format tag, 1 = PCM */
    int16 numchannels;    /* Number of channels in file */
    int32 SamplingFreq;     /* Sampling frequency */
    int32 BytesPerSec;      /* Average bytes/sec */
    int16 BlockAlign;     /* Block align */
    int16 BitsPerSample;  /* 8 or 16 bit */
    char datatag[4];      /* "data" string */
    int32 datalength;       /* Raw data length */
} MSWAV_hdr;



/* Struct to hold the front-end parameters */
typedef struct{
        param_t *P;
        fe_t *FE;
        int16 *fr_data;
        float32 *fr_cep;
} fewrap_t;



void process_fe_class (fewrap_t *FEW,class_t *CLASSW,endpointer_t *ENDPTR,int16 *spbuffer,int32 splen);
param_t *fe_parse_options(int argc, char **argv);
void fe_init_params(param_t *P);
fewrap_t * few_initialize(int argc, char **argv);
void few_free(fewrap_t *FEW);
int16 * fe_convert_files(param_t *P, fe_t *FE, int32 *splenp, int32 *nframesp);
int32 fe_build_filenames(param_t *P, char *fileroot, char **infilename);
char *fe_copystr(char *dest_str, char *src_str);
int32 fe_count_frames(fe_t *FE, int32 nsamps, int32 count_partial_frames);
int32 fe_readspch(param_t *P, char *infile, int16 **spdata, int32 *splen);
int32 fe_writefeat(fe_t *FE, char *outfile, int32 nframes, float32 **feat);
int32 fe_free_param(param_t *P);
int32 fe_openfiles(param_t *P, fe_t *FE, char *infile, int32 *fp_in, int32 *nsamps, 
		   int32 *nframes, int32 *nblocks);
int32 fe_readblock_spch(param_t *P, int32 fp, int32 nsamps, int16 *buf);
int32 fe_writeblock_feat(param_t *P, fe_t *FE, int32 fp, int32 nframes, float32 **feat);
int32 fe_closefiles(int32 fp_in);
int32 fe_dither(int16 *buffer,int32 nsamps);
static int32 fe_usage(char **argv);

#ifdef __cplusplus
 }
 #endif 
