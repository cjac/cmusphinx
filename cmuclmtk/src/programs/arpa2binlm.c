/* ====================================================================
 * Copyright (c) 1999-2006 Carnegie Mellon University.  All rights
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
/*
 *
 * arpa2binlm.c - a program that converts an arpa LM to binary LM 
 * called headers of CMU-Cambridge LM Toolkit 
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2006 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: arpa2binlm.c,v $
 * Revision 1.1  2006/04/02 23:57:12  archan
 * NOT COMPILED, add a simple version of arpa2binlm.c , currently it doesn't make sense to convert and ARPA to V2 binlm format. However, it will make sense when we implement null smoothing.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../liblmest/ngram.h"
#include "../liblmest/toolkit.h"
#include "../libs/pc_general.h"
#include "../libs/general.h"
#include "../liblmest/idngram2lm.h"
#include "../liblmest/evallm.h"

void help_message(){
  fprintf(stderr,"arpa2lm : Convert an ARPA format to CMU-Cambrige LM Toolkit binary format.\n");
  fprintf(stderr,"Usage : arpa2binlm  -arpa .arpa\n");
  fprintf(stderr,"                    -binary .binlm\n");
  fprintf(stderr,"                 [ -verbosity n ]\n");
}

int main (int argc,char **argv) {
  char *bin_path;
  int verbosity;
  arpa_lm_t ng;

  if (pc_flagarg(&argc,argv,"-help") || argc == 1) {
    help_message();
    exit(1);
  }

  report_version(&argc,argv);

  verbosity = pc_intarg(&argc,argv,"-verbosity",DEFAULT_VERBOSITY);

  ng.arpa_filename = salloc(pc_stringarg(&argc,argv,"-arpa",""));
  ng.arpa_fp = rr_oopen(ng.arpa_filename);

  if (!strcmp(ng.arpa_filename,""))
    quit(-1,"Error : must specify an ARPA language model file.\n");

  bin_path = salloc(pc_stringarg(&argc,argv,"-binary",""));

  if (!strcmp(bin_path,"")) 
    quit(-1,"Error : must specify a binary language model file.\n");

  pc_report_unk_args(&argc,argv,verbosity);
  pc_message(verbosity,1,"Reading binary language model from %s...",bin_path);

  load_arpa_lm(&ng,ng.arpa_filename);

  if (verbosity>=2) 
    display_stats(&ng);

  pc_message(verbosity,1,"Done\n");
  write_bin_lm(&ng,verbosity);

  pc_message(verbosity,0,"arpa2lm : Done.\n");

  exit(0);
}
