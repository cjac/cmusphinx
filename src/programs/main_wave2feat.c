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

#include "cont_mgau.h"
#include "classify.h"
#include "endptr.h"
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
wave2feat -i  input.raw \n						\
        -o   output.mfc \n						\
        -raw 1 \n							\
        -input_endian little \n						\
        -samprate  16000 \n						\
        -lowerf    130 \n						\
        -upperf    6800 \n						\
        -nfilt     40 \n						\
        -nfft      512";

static arg_t defn[] = {
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
  { "-o",
    ARG_STRING,
    NULL,
    "Single cepstral output file" },
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
    ONE_CHAN,
    "Number of channels of data (interlaced samples assumed)" },
  { "-whichchan",
    ARG_INT32,
    ONE_CHAN,
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
    DEFAULT_PRE_EMPHASIS_ALPHA,
    "Preemphasis parameter" },
  { "-srate",
    ARG_FLOAT32,
    DEFAULT_SAMPLING_RATE,
    "Sampling rate" },
  { "-frate",
    ARG_INT32,
    DEFAULT_FRAME_RATE,
    "Frame rate" },
  { "-wlen",
    ARG_FLOAT32,
    DEFAULT_WINDOW_LENGTH,
    "Hamming window length" },
  { "-nfft",
    ARG_INT32,
    DEFAULT_FFT_SIZE,
    "Size of FFT" },
  { "-nfilt",
    ARG_INT32,
    DEFAULT_NUM_FILTERS,
    "Number of filter banks" },
  { "-lowerf",
    ARG_FLOAT32,
    DEFAULT_LOWER_FILT_FREQ,
    "Lower edge of filters" },
  { "-upperf",
    ARG_FLOAT32,
    DEFAULT_UPPER_FILT_FREQ,
    "Upper edge of filters" },
  { "-ncep",
    ARG_INT32,
    DEFAULT_NUM_CEPSTRA,
    "Number of cep coefficients" },
  { "-doublebw",
    ARG_INT32,
    "0",
    "Use double bandwidth filters (same center freq)" },
  { "-blocksize",
    ARG_INT32,
    DEFAULT_BLOCKSIZE,
    "Block size, used to limit the number of samples used at a time when reading very large audio files" },
  { "-dither",
    ARG_INT32,
    "0",
    "Add 1/2-bit noise" },
  { "-verbose",
    ARG_INT32,
    "0",
    "Show input filenames" },
  { "-logfn",
    ARG_STRING,
    NULL,
    "Log file (default stdout/stderr)" },
  { "-seed",
    ARG_INT32,
    "-1",
    "The seed for the random generator"},
  { NULL, ARG_INT32,  NULL, NULL }
};

/*       
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

int32 main(int32 argc, char **argv)
{
  param_t *P;

  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);
  unlimit();

  P = fe_parse_options(argc,argv);
  if (fe_convert_files(P) != FE_SUCCESS){
    E_FATAL("error converting files...exiting\n");
  }
  
  fe_free_param(P);

  cmd_ln_appl_exit();

  return(0);
}


