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
 *  Aug 6, 2004
 *  Created by Yitao Sun (yitao@cs.cmu.edu).
 */

#include <stdio.h>
#include <live_decode_API.h>
#include <live_decode_args.h>

#define SAMPLE_BUFFER_LENGTH	4096
#define FILENAME_LENGTH		512

int
main(int _argc, char **_argv)
{
  live_decoder_t decoder;
  short samples[SAMPLE_BUFFER_LENGTH];
  int len;
  char *hypstr;
  char *ctrlfn;
  char *cfgfn;
  char *rawdirfn;
  char rawfn[FILENAME_LENGTH];
  char fullrawfn[FILENAME_LENGTH];
  FILE *ctrlfd;
  FILE *rawfd;

  if (_argc != 4) {
    printf("\nUSAGE: %s <ctrlfile> <rawdir> <cfgfile>\n", _argv[0]);
    return -1;
  }

  ctrlfn = _argv[1];
  rawdirfn = _argv[2];
  cfgfn = _argv[3];

  if ((ctrlfd = fopen(ctrlfn, "r")) == NULL) {
    E_FATAL("Cannot open control file %s.\n", ctrlfn);
  }

  if (cmd_ln_parse_file(arg_def, cfgfn)) {
    E_FATAL("Bad configuration file %s.\n", cfgfn);
  }

  if (ld_init(&decoder) != LD_SUCCESS) {
    E_FATAL("Failed to initialize live-decoder.\n");
  }

  while (fscanf(ctrlfd, "%##FILENAME_LENGTH##s", rawfn) != EOF) {
    snprintf(fullrawfn, FILENAME_LENGTH, "%s/%s", rawdirfn, rawfn);
    if ((rawfd = fopen(fullrawfn, "r")) == NULL) {
      E_FATAL("Cannnot open raw file %s.\n", fullrawfn);
    }

    if (ld_begin_utt(&decoder, 0) != LD_SUCCESS) {
      E_FATAL("Cannot begin utterance decoding.\n");
    }
    len = fread(samples, sizeof(short), SAMPLE_BUFFER_LENGTH, rawfd);
    while (len > 0) {
      ld_process_raw(&decoder, samples, len);
      if (ld_retrieve_hyps(&decoder, NULL, &hypstr, NULL) == LD_SUCCESS) {
	E_INFO("PARTIAL_HYP: %s\n", hypstr);
      }
      len = fread(samples, sizeof(short), SAMPLE_BUFFER_LENGTH, rawfd);
    }
    fclose(rawfd);
    ld_end_utt(&decoder);

  }

  ld_finish(&decoder);

  return 0;
}
