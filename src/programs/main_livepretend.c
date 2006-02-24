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
 * Revision 1.15  2006/02/24  04:40:53  arthchan2003
 * Merged from the branch SPHINX3_5_2_RCI_IRII_BRANCH: Used ctl_process from now on.
 * 
 * Revision 1.14.4.3  2006/01/16 20:29:52  arthchan2003
 * Changed -ltsoov to -lts_mismatch. Changed lm_rawscore interface. Change from cmd_ln_access to cmd_ln_str.
 *
 * Revision 1.14.4.2  2005/07/27 23:23:39  arthchan2003
 * Removed process_ctl in allphone, dag, decode_anytopo and astar. They were duplicated with ctl_process and make Dave and my lives very miserable.  Now all application will provided their own utt_decode style function and will pass ctl_process.  In that way, the mechanism of reading would not be repeated. livepretend also follow the same mechanism now.  align is still not yet finished because it read yet another thing which has not been considered : transcription.
 *
 * Revision 1.14.4.1  2005/07/18 23:21:24  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.14  2005/06/22 05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.5  2005/06/15 21:41:56  archan
 * Sphinx3 to s3.generic: 1) added -ctloffset and -ctlcount support for livepretend. 2) Also allow lmname to be set correctly.  Now it is using ld_set_lm instead of kb_setlm.  That makes the code cleaner.
 *
 * Revision 1.4  2005/04/20 03:50:36  archan
 * Add comments on all mains for preparation of factoring the command-line.
 *
 * Revision 1.3  2005/03/30 00:43:41  archan
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
#include "corpus.h"

#define SAMPLE_BUFFER_LENGTH	4096
#define FILENAME_LENGTH		512

static live_decoder_t decoder;
static char *rawdirfn;
static stat_t* st;

/**
   
 */
static void utt_livepretend(void *data, utt_res_t *ur, int32 sf, int32 ef, char *uttid)
{
  char fullrawfn[FILENAME_LENGTH];
  char *hypstr;

  short samples[SAMPLE_BUFFER_LENGTH];
  kb_t *kb;
  FILE *rawfd;
  int len;

  kb = (kb_t *) data;

  /*  report_utt_res(ur);*/
  sprintf(fullrawfn, "%s/%s%s", rawdirfn, ur->uttfile,decoder.rawext);
  if ((rawfd = fopen(fullrawfn, "rb")) == NULL) {
    E_FATAL("Cannnot open raw file %s.\n", fullrawfn);
  }
  
  if(ur->lmname!=NULL) srch_set_lm((srch_t*)kb->srch,ur->lmname);
  if(ur->regmatname!=NULL) kb_setmllr(ur->regmatname,ur->cb2mllrname,kb);

  if (ld_begin_utt(&decoder, ur->uttfile) != LD_SUCCESS) {
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

}

int
main(int _argc, char **_argv)
{
  char *ctrlfn;
  char *cfgfn;

  print_appl_info(_argv[0]);

  if (_argc != 4) {
    printf("\nUSAGE: %s <ctrlfile> <rawdir> <cfgfile>\n", _argv[0]);
    return -1;
  }

  ctrlfn = _argv[1];
  rawdirfn = _argv[2];
  cfgfn = _argv[3];

  if (cmd_ln_parse_file(arg_def, cfgfn)) {
    E_FATAL("Bad configuration file %s.\n", cfgfn);
  }

  if (ld_init(&decoder) != LD_SUCCESS) {
    E_FATAL("Failed to initialize live-decoder.\n");
  }

  st = decoder.kb.stat;
  ptmr_init(&(st->tm));


  if (ctrlfn) {
    /* When -ctlfile is speicified, corpus.c will look at -ctl_lm and -ctl_mllr to get
       the corresponding LM and MLLR for the utterance */
    st->tm = ctl_process (ctrlfn,
			  cmd_ln_str("-ctl_lm"),
			  cmd_ln_str("-ctl_mllr"),
			  cmd_ln_int32("-ctloffset"),
			  cmd_ln_int32("-ctlcount"),
			  utt_livepretend, &(decoder.kb));
  } else {
    E_FATAL("control file is not specified.\n");
  }

  ld_finish(&decoder);

  stat_report_corpus(decoder.kb.stat);
  
  return 0;
}

