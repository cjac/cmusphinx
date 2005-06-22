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
 *
 * HISTORY
 * $Log$
 * Revision 1.14  2005/06/22  05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 * 
 * Revision 1.5  2005/06/15 21:41:56  archan
 * Sphinx3 to s3.generic: 1) added -ctloffset and -ctlcount support for livepretend. 2) Also allow lmname to be set correctly.  Now it is using ld_set_lm instead of kb_setlm.  That makes the code cleaner.
 *
 * Revision 1.4  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.3  2005/03/30 00:43:41  archan
 * Add $Log$
 * Revision 1.14  2005/06/22  05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 * 
 * Add Revision 1.5  2005/06/15 21:41:56  archan
 * Add Sphinx3 to s3.generic: 1) added -ctloffset and -ctlcount support for livepretend. 2) Also allow lmname to be set correctly.  Now it is using ld_set_lm instead of kb_setlm.  That makes the code cleaner.
 * Add
 * Add Revision 1.4  2005/04/20 03:50:36  archan
 * Add Add comments on all mains for preparation of factoring the command-line.
 * Add into most of the .[ch] files. It is easy to keep track changes.
 *
 */


/** \file main_livepretend.c
 * \brief Driver for live-mode simulation.
 */
#include <stdio.h>
#include <live_decode_API.h>
#include <live_decode_args.h>

#include "kb.h"
#include "s3types.h"

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
  char lmname[FILENAME_LENGTH];
  char fullrawfn[FILENAME_LENGTH];
  FILE *ctrlfd;
  FILE *ctllmfd;
  FILE *rawfd;
  stat_t* st;
  int32 nskip, count;

  print_appl_info(_argv[0]);

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

  st = decoder.kb.stat;
  ptmr_init(&(st->tm));

  nskip = cmd_ln_int32("-ctloffset");
  count = cmd_ln_int32("-ctlcount");
  if (cmd_ln_str("-ctl_lm")) {
    if ((ctllmfd = fopen(cmd_ln_str("-ctl_lm"), "r")) == NULL)
      E_FATAL("Cannot open LM control file %s.\n", cmd_ln_str("-ctl_lm"));
  }
  else
    ctllmfd = NULL;

  if (nskip > 0) {
    E_INFO("Skipping %d entries at the beginning of %s\n", nskip, ctrlfn);
    
    for (; nskip > 0; --nskip) {
      if (fscanf(ctrlfd, "%s", rawfn) == EOF)
	E_FATAL("EOF while skipping initial lines\n");
      if (ctllmfd && fscanf(ctllmfd, "%s", lmname) == EOF)
	E_FATAL("EOF while skipping initial lines (in ctl_lm)\n");
    }
  }

  while (fscanf(ctrlfd, "%s", rawfn) != EOF) {
    if (count-- == 0)
      break;

    if (ctllmfd) {
      if (fscanf(ctllmfd, "%s", lmname) == EOF)
	E_FATAL("EOF while reading LM control file\n");

      /* This is somewhat inelegant; the live-decode API should make
         an allowance for dynamic LM switching - dhuggins@cs,
         2005-04-19 */
      /*kb_setlm(lmname, &decoder.kb);*/

      /* 20050615 Dave's last comment is valid. Now a lm switching
	 interface is available implemented and it is a two-layered
	 wrapper of lm switching function in the search implementation
	 modules.
      */
      ld_set_lm(&decoder,lmname);
    }

    sprintf(fullrawfn, "%s/%s%s", rawdirfn, rawfn,decoder.rawext);
    if ((rawfd = fopen(fullrawfn, "rb")) == NULL) {
      E_FATAL("Cannnot open raw file %s.\n", fullrawfn);
    }

    if (ld_begin_utt(&decoder, rawfn) != LD_SUCCESS) {
      E_FATAL("Cannot begin utterance decoding.\n");
    }
    len = fread(samples, sizeof(short), SAMPLE_BUFFER_LENGTH, rawfd);

    while (len > 0) {
      ptmr_start (&(st->tm));
      ld_process_raw(&decoder, samples, len);
      ptmr_stop (&(st->tm));

      if (ld_retrieve_hyps(&decoder, NULL, &hypstr, NULL) == LD_SUCCESS) {
	if(decoder.phypdump){
	  E_INFO("PARTIAL_HYP: %s\n", hypstr);
	}
      }
      len = fread(samples, sizeof(short), SAMPLE_BUFFER_LENGTH, rawfd);
    }
    fclose(rawfd);
    ld_end_utt(&decoder);
    ptmr_reset(&(st->tm));

  }

  ld_finish(&decoder);

  stat_report_corpus(decoder.kb.stat);
  
  return 0;
}

