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
  $Log: text2wfreq.c,v $
  Revision 1.6  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.5  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */


/* Very strongly based on the program wordfreq, by Gary Cook
   (gdc@eng.cam.ac.uk), adapted (with permission) for the sake of
   consistency with the rest of the toolkit by Philip Clarkson,
   27/9/96 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toolkit.h"  
#include "pc_general.h"
#include "general.h" 
#include "ac_hash.h"
#include "ac_lmfunc_impl.h"

#define DEFAULT_HASH 1000000

void help_message()
{
  fprintf(stderr,"text2wfreq : Generate a word frequency list for text.\n");
  fprintf(stderr,"Usage : text2freq [ -hash %d ]\n",DEFAULT_HASH);
  fprintf(stderr,"                  [ -verbosity 2 ]\n");
  fprintf(stderr,"                  < .text > .wfreq\n");
}

int main( int argc, char **argv )
{
  int init_nwords;
  int verbosity;
  FILE* ifp; 
  FILE* ofp;

  if (pc_flagarg( &argc, argv,"-help")) {
    help_message();
    exit(1);
  }

  /* process command line */

  report_version(&argc,argv);
  init_nwords = pc_intarg( &argc, argv, "-hash", DEFAULT_HASH );

  verbosity = pc_intarg(&argc,argv,"-verbosity",DEFAULT_VERBOSITY);

  pc_report_unk_args(&argc,argv,verbosity);

  pc_message(verbosity,2,"text2wfreq : Reading text from standard input...\n");

  ifp=stdin;
  ofp=stdout;

  text2wfreq_impl(ifp,ofp,init_nwords,verbosity);

  pc_message(verbosity,0,"text2wfreq : Done.\n");
  return 0;
}


