/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
#ifndef _FE_DUMP_H_
#define _FE_DUMP_H_

#define DECIMAL 1
#define HEXADECIMAL 2
#define SCIENTIFIC 3

#include <libutil/libutil.h>
#include "metrics.h"
#include "s3types.h"
#include "libs3decoder/new_fe.h"

FILE *fe_dumpfile;
int fe_dump;
int fe_dump_type;

struct integer64 {
    long half1;
    long half2;
};

union data64 {
    double f;
    struct integer64 i;
};

struct integer32 {
    int half1;
    int half2;
};

union data32 {
    float f;
    struct integer32 i;
};


/* Functions */

int32 fe_dump_process_utt(fe_t *FE, int16 *spch, int32 nsamps, 
                          float32 ***cep_block);

int32 fe_dump_end_utt(fe_t *FE, float32 *cepvector);


/* Dump functions */
void fe_dump_short_frame(FILE *stream, short *shortframe, int bufferSize);
void fe_dump_double_frame(FILE *stream, double *preemphasizedAudio,
			  int bufferSize, char *processorName);
void fe_dump_float_frame(FILE *stream, float32 *data, int eachArraySize,
			 char *processorName, char *individualName);
void fe_dump2d_float_frame(FILE *stream, float32 **data, int array2DSize,
			   int eachArraySize, char *processorName,
			   char *individualName);


#endif
