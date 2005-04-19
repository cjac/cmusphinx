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


/** \file main_livepretend.c
 * \brief Driver for live-mode simulation.
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
  ptmr_t tm;
  int32 nskip, count;

  print_appl_info(_argv[0]);

  if (_argc != 4) {
    printf("\nUSAGE: %s <ctrlfile> <rawdir> <cfgfile>\n", _argv[0]);
    return -1;
  }

  ctrlfn = _argv[1];
  rawdirfn = _argv[2];
  cfgfn = _argv[3];

  ptmr_init(&tm);

  if ((ctrlfd = fopen(ctrlfn, "r")) == NULL) {
    E_FATAL("Cannot open control file %s.\n", ctrlfn);
  }

  if (cmd_ln_parse_file(arg_def, cfgfn)) {
    E_FATAL("Bad configuration file %s.\n", cfgfn);
  }

  if (ld_init(&decoder) != LD_SUCCESS) {
    E_FATAL("Failed to initialize live-decoder.\n");
  }

  nskip = cmd_ln_int32("-ctloffset");
  count = cmd_ln_int32("-ctlcount");

  if (nskip > 0) {
    E_INFO("Skipping %d entries at the beginning of %s\n", nskip, ctrlfn);
    
    for (; nskip > 0; --nskip)
      if (fscanf(ctrlfd, "%s", rawfn) == EOF)
	E_FATAL("EOF while skipping initial lines\n");
  }

  while (fscanf(ctrlfd, "%s", rawfn) != EOF) {
    if (count-- == 0)
      break;

    sprintf(fullrawfn, "%s/%s%s", rawdirfn, rawfn,decoder.rawext);
    if ((rawfd = fopen(fullrawfn, "rb")) == NULL) {
      E_FATAL("Cannnot open raw file %s.\n", fullrawfn);
    }

    if (ld_begin_utt(&decoder, rawfn) != LD_SUCCESS) {
      E_FATAL("Cannot begin utterance decoding.\n");
    }
    len = fread(samples, sizeof(short), SAMPLE_BUFFER_LENGTH, rawfd);

    while (len > 0) {
      ptmr_start (&tm);
      ld_process_raw(&decoder, samples, len);
      ptmr_stop (&tm);

      if (ld_retrieve_hyps(&decoder, NULL, &hypstr, NULL) == LD_SUCCESS) {
	if(decoder.phypdump){
	  E_INFO("PARTIAL_HYP: %s\n", hypstr);
	}
      }
      len = fread(samples, sizeof(short), SAMPLE_BUFFER_LENGTH, rawfd);
    }
    fclose(rawfd);
    ld_end_utt(&decoder);
    ptmr_reset(&tm);

  }

  ld_finish(&decoder);

  E_INFO("SUMMARY:  %d fr;  %d cdsen, %d cisen, %d cdgau %d cigau/fr, %.2f xCPU [%.2f xOvrhd];  %d hmm/fr, %d wd/fr, %.2f xCPU;  tot: %.2f xCPU, %.2f xClk\n",
	 decoder.kb.tot_fr,
	 (int32)(decoder.kb.tot_sen_eval / decoder.kb.tot_fr),
	 (int32)(decoder.kb.tot_ci_sen_eval / decoder.kb.tot_fr),
	 (int32)(decoder.kb.tot_gau_eval / decoder.kb.tot_fr),
	 (int32)(decoder.kb.tot_ci_gau_eval / decoder.kb.tot_fr),
	 decoder.kb.tm_sen.t_tot_cpu * 100.0 / decoder.kb.tot_fr,
	 decoder.kb.tm_ovrhd.t_tot_cpu * 100.0 / decoder.kb.tot_fr,
	 (int32)(decoder.kb.tot_hmm_eval / decoder.kb.tot_fr),
	 (int32)(decoder.kb.tot_wd_exit / decoder.kb.tot_fr),
	 decoder.kb.tm_srch.t_tot_cpu * 100.0 / decoder.kb.tot_fr,
	 tm.t_tot_cpu * 100.0 / decoder.kb.tot_fr,
	 tm.t_tot_elapsed * 100.0 / decoder.kb.tot_fr);
  
  return 0;
}

