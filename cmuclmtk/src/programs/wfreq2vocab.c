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
  $Log: wfreq2vocab.c,v
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../liblmest/toolkit.h"
#include "../libs/ac_lmfunc_impl.h"
#include "../libs/general.h"
#include "../libs/pc_general.h"

#define DEFAULT_MAX_RECORDS 3000000

/***************************
      MAIN FUNCTION
 ***************************/

void help_message()
{
  fprintf(stderr,"wfreq2vocab : Generate a vocabulary file from a word frequency file.\n");
  fprintf(stderr,"Usage : wfreq2vocab [ -top 20000 | -gt 10]\n");
  fprintf(stderr,"                    [ -records %d ]\n",DEFAULT_MAX_RECORDS);
  fprintf(stderr,"                    [ -verbosity %d]\n",DEFAULT_VERBOSITY);
  fprintf(stderr,"                    < .wfreq > .vocab\n");
}

int main(int argc, char *argv[]) {

  int verbosity;
  int vocab_size;
  int cutoff;
  int num_recs;
  FILE *ifp, *ofp;

  /* Process command line */

  report_version(&argc,argv);

  if (pc_flagarg( &argc, argv,"-help")) {
    help_message();
    exit(1);
  }

  cutoff = pc_intarg( &argc, argv, "-gt",-1);
  vocab_size = pc_intarg(&argc, argv, "-top",-1);
  num_recs = pc_intarg(&argc, argv, "-records",DEFAULT_MAX_RECORDS);
  verbosity = pc_intarg(&argc, argv, "-verbosity",DEFAULT_VERBOSITY);
  
  pc_report_unk_args(&argc,argv,verbosity);

  ifp=stdin;
  ofp=stdout;
  wfreq2vocab_impl(ifp,ofp,cutoff, vocab_size,num_recs,verbosity);

  exit(0);

}  
    



