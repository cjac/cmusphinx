/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.	All rights
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
#include <live_decode.h>
#include <args.h>

#define BUFFER_LENGTH 4096
main(int argc, char **argv)
{
  live_decoder_t decoder;
  short samples[BUFFER_LENGTH];
  int len;
  char *hypstr;
  FILE *rawfd;

  /*
   * Initializing
   */
  if (argc != 3) {
    printf("Usage:  livepretend config_file raw_file\n");
    return -1;
  }

  if (cmd_ln_parse_file(arg_def, argv[1])) {
    printf("Bad arguments file (%s).\n", argv[1]);
    return -1;
  }

  if ((rawfd = fopen(argv[2], "rb")) == 0) {
    printf("Bad raw file (%s).\n", argv[2]);
    return -1;
  }

  if (ld_init(&decoder)) {
    printf("Initialization failed.\n");
    return -1;
  }

  if (ld_utt_begin(&decoder, 0)) {
    printf("Cannot start decoding.\n");
    return -1;
  }

  while ((len = fread(samples, sizeof(short), BUFFER_LENGTH, rawfd)) > 0) {
    if (ld_utt_proc_raw(&decoder, samples, len)) {
      printf("Data processing error.\n");
      break;
    }
    
    if (ld_utt_hyps(&decoder, &hypstr, 0)) {
      printf("Cannot retrieve hypothesis.\n");
    }
    else {
      printf("Hypothesis:\n%s\n", hypstr);
    }
  }

  fclose(rawfd);

  if (ld_utt_end(&decoder)) {
    printf("Cannot end decoding.\n");
    return -1;
  }

  if (ld_utt_hyps(&decoder, &hypstr, 0)) {
    printf("Cannot retrieve hypothesis.\n");
  }
  else {
    printf("Hypothesis:\n%s\n", hypstr);
  }

  ld_finish(&decoder);

  return 0;
}
