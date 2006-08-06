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
 * $Log$
 * Revision 1.4  2005/06/22  05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 * 
 * Revision 1.2  2005/03/30 00:43:41  archan
 * Add $Log$
 * Revision 1.4  2005/06/22  05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *  into most of the .[ch] files. It is easy to keep track changes.
 *
 */

/** \file main_ep.h
    \brief Just a master header for ep.
 */

#include "fe.h"
#include "classify.h"
#include "endptr.h"


#ifdef __cplusplus
extern "C" {
#endif

/* The following only use in the application level */

#define NULL_CHAR '\0'
#define MAXCHARS 2048

#define WAV 1
#define RAW 2
#define NIST 3
#define MSWAV 4

#define LITTLE 1
#define BIG 2

#define HEADER_BYTES 1024

#define COUNT_PARTIAL 1
#define COUNT_WHOLE 0

/** 
    \struct MSWAV_hdr
    \brief A MS Wavefile header. 
    Some defines for MS Wav Files 
    The MS Wav file is a RIFF file, and has the following 44 byte header 
*/
typedef struct RIFFHeader{
    char rifftag[4];      /**< "RIFF" string */
    int32 TotalLength;      /**< Total length */
    char wavefmttag[8];   /**< "WAVEfmt " string (note space after 't') */
    int32 RemainingLength;  /**< Remaining length */
    int16 data_format;    /**< data format tag, 1 = PCM */
    int16 numchannels;    /**< Number of channels in file */
    int32 SamplingFreq;     /**< Sampling frequency */
    int32 BytesPerSec;      /**< Average bytes/sec */
    int16 BlockAlign;     /**< Block align */
    int16 BitsPerSample;  /**< 8 or 16 bit */
    char datatag[4];      /**< "data" string */
    int32 datalength;       /**< Raw data length */
} MSWAV_hdr;

/**
 * \struct fewrap_t
 * \brief Wrapper structure to hold the front-end parameters  
 */
typedef struct{
    param_t *P;
    fe_t *FE;
    int16 *fr_data;
    float32 *fr_cep;

    char *wavfile;
    char *cepfile;
    char *ctlfile;
    int32 nskip;
    int32 runlen;
    char *wavdir;
    char *cepdir;
    char *wavext;
    char *cepext;
    int32 input_format;
    int32 is_batch;
    int32 is_single;
    int32 blocksize;
    int32 verbose;
    int32 machine_endian;
    int32 input_endian;
    int32 output_endian;
    int32 nchans;
    int32 whichchan;
    int32 splen;
    int32 nframes;
    int16* spdata;
} fewrap_t;

/** 
    Functions that wrap up the front-end operations on the front-end
    wrapper operations.  
*/
fewrap_t * few_initialize();

/**
 *  Free the FEW structure
 */
void few_free(fewrap_t *FEW /**< the FEW structure one wants to free*/
    );

#ifdef __cplusplus
}
#endif
