/* ====================================================================
 * Copyright (c) 1998-2000 Carnegie Mellon University.  All rights 
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
/*********************************************************************
 *
 * File: cmd_ln_defn.h
 * 
 * Description: 
 * 	Command line argument definition
 *
 * Author: 
 *      
 *********************************************************************/

#ifndef CMD_LN_DEFN_H
#define CMD_LN_DEFN_H

/*#include <s3/cmd_ln.h>
#include "wave2feat.h"*/
#include "fe.h"

const char helpstr[] =
  "Description: \n\
Create cepstra from audio file.\n		\
									\
The main parameters that affect the final output, with typical values, are:\n \
									\
samprate, typically 8000, 11025, or 16000\n				\
lowerf, 130, 200, 130, for the respective sampling rates above\n	\
upperf, 3700, 5200, 6800, for the respective sampling rates above\n	\
nfilt, 31, 37, 40, for the respective sampling rates above\n		\
nfft, 256 or 512\n							\
format, raw or nist or mswav\n						\
\"";

const char examplestr[] =
  "Example: \n\
This example creates a cepstral file named \"output.mfc\" from an input audio file named \"input.raw\", which is a raw audio file (no header information), which was originally sampled at 16kHz. \n \
\n									\
ep -i  input.raw \n						\
        -o   output.mfc \n						\
        -raw no \n							\
        -input_endian little \n						\
        -samprate  16000 \n						\
        -lowerf    130 \n						\
        -upperf    6800 \n						\
        -nfilt     40 \n						\
        -nfft      512";

static arg_t arg[] = {
  { "-help",
    ARG_INT32,
    
    "0",
    "Shows the usage of the tool"},
  
  { "-example",
    ARG_INT32,
    
    "0",
    "Shows example of how to use the tool"},
  
  { "-i",
    ARG_STRING,
    
    NULL,
    "Single audio input file" },
  
  
  { "-c",
    ARG_STRING,
    
    NULL,
    "Control file for batch processing" },
  
  { "-di",
    ARG_STRING,
    
    NULL,
    "Input directory, input file names are relative to this, if defined" },
  
  { "-ei",
    ARG_STRING,
    
    NULL,
    "Input extension to be applied to all input files" },
  
  { "-do",
    ARG_STRING,
    
    NULL,
    "Output directory, output files are relative to this" },
  
  { "-eo",
    ARG_STRING,
    
    NULL,
    "Output extension to be applied to all output files" },
  
  { "-nist",
    ARG_INT32,
    
    "0",
    "Defines input format as NIST sphere" },
  
  { "-raw",
    ARG_INT32,
    
    "0",
    "Defines input format as raw binary data" },
  
  { "-mswav",
    ARG_INT32,
    
    "0",
    "Defines input format as Microsoft Wav (RIFF)" },
  
  { "-input_endian",
    ARG_STRING,
    
    "little",
    "Endianness of input data, big or little, ignored if NIST or MS Wav" },
  
  { "-nchans",
    ARG_INT32,
    
    "1",
    "Number of channels of data (interlaced samples assumed)" },
  
  { "-whichchan",
    ARG_INT32,
    
    "1",
    "Channel to process" },
  
  { "-logspec",
    ARG_INT32,
    
    "0",
    "Write out logspectral files instead of cepstra" },
  
  { "-feat",
    ARG_STRING,
    
    "sphinx",
    "SPHINX format - big endian" },
  
  { "-mach_endian",
    ARG_STRING,
    
#ifdef WORDS_BIGENDIAN
    "big",
#else
    "little",
#endif
    "Endianness of machine, big or little" },
  
  { "-alpha",
    ARG_FLOAT32,
    
    "0.97",
    "Preemphasis parameter" },
  
  { "-srate",
    ARG_FLOAT32,
    
    "16000.0",
    "Sampling rate" },
  
  { "-frate",
    ARG_INT32,
    
    "100",
    "Frame rate" },
  
  { "-wlen",
    ARG_FLOAT32,
    
    "0.0256",
    "Hamming window length" },
  
  { "-nfft",
    ARG_INT32,
    
    "512",
    "Size of FFT" },
  
  { "-nfilt",
    ARG_INT32,
    
    "40",
    "Number of filter banks" },
  
  { "-lowerf",
    ARG_FLOAT32,
    
    "200",
    "Lower edge of filters" },
  
  { "-upperf",
    ARG_FLOAT32,
    
    "3500",
    "Upper edge of filters" },
  
  { "-ncep",
    ARG_INT32,
    
    "13",
    "Number of cep coefficients" },
  
  { "-doublebw",
    ARG_INT32,
    
    "0",
    "Use double bandwidth filters (same center freq)" },
  
  { "-blocksize",
    ARG_INT32,
    
    "200000",
    "Block size, used to limit the number of samples used at a time when reading very large audio files" },
  
  { "-dither",
    ARG_INT32,
    
    "0",
    "Add 1/2-bit noise" },
  
  { "-verbose",
    ARG_INT32,
    
    "0",
    "Show input filenames" },

  { "-mean",
    ARG_STRING,
    NULL,
    "The mean file" },

  { "-var",
    ARG_STRING,
    NULL,
    "The var file" },

  { "-varfloor",
    ARG_FLOAT32,
    "0.0001",
    "Mixture gaussian variance floor (applied to data from -var file)" },
  
  { "-mixw",
    ARG_STRING,
    NULL,
    "The mixture weight file" },
  
  { "-mixwfloor",
    ARG_FLOAT32,
    "0.0000001",
    "Senone mixture weights floor (applied to data from -mixw file)" },

  { NULL, ARG_INT32,  NULL, NULL }
};

    
#define CMD_LN_DEFN_H

#endif /* CMD_LN_DEFN_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2004/10/22  22:14:41  arthchan2003
 * Endpointer code check-in, test can be done by test-ep
 * 
 * Revision 1.1  2004/09/21 05:42:52  archan
 * Incorporating a command-line front-end in the off-line end-pointer.  Some arguments are still not under strick checking.
 *
 * Revision 1.1  2004/09/09 17:59:30  egouvea
 * Adding missing files to wave2feat
 *
 *
 *
 */
