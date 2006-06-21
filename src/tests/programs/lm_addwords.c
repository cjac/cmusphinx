/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * lm_addwords.c -- Add words to language model. 
 * This is a test program so it only gives minimal 
 * functionalities of LM conversion. 
 *
 * I am also planning to re-write it in future so that it could carry
 * out some general LM editing work.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * Started by Arthur Chan at March 8, 2006
 *
 */

#include "lm.h"
#include "s3types.h"
#include "cmd_ln.h"
#include "cmdln_macro.h"
#include "encoding.h"

static arg_t arg[] = {
  common_application_properties_command_line_macro()
  { "-input",
    REQARG_STRING,
    NULL,
    "Input file"},
  { "-output",
    REQARG_STRING,
    NULL,
    "Output file, if not specified. the output format name will be used a suffix. "},
  { "-inputfmt",
    REQARG_STRING,
    "TXT",
    "Input LM format. TXT (ARPA LM Format), DMP (Sphinx 3 efficient LM format 16 bits)"},
  { "-outputfmt",
    ARG_STRING,
    "DMP",
    "Output LM format: TXT (ARPA LM Format), DMP (Sphinx 3 efficient LM format 16 bits), DMP32 (Sphinx 3 efficient LM format 32 bits) "}, 
  /* Also FST for AT&T file format 
     but the option is hidden because it is not well tested*/
  { "-outputdir",
    ARG_STRING,
    ".",
    "Output Directory."},
  { "-wordlist",
    ARG_STRING,
    ".",
    "A word list we will use in testing.\n"},
  { "-lminmemory",
    ARG_INT32,
    "1",
    "Whether lm is memory-based(1) or disk-based (0)"},
  { NULL, ARG_INT32,  NULL, NULL }
};

int main(int argc, char *argv[])
{
  char *inputfn, *outputfn;
  char *inputfmt, *outputfmt;
  char *outputdir;
  char *outputpath;

  lm_t* lm;
  char separator[1];

  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc, argv, "default.arg", arg);

  inputfn=NULL;
  outputfn=NULL;
  inputfmt=NULL;
  outputfmt=NULL;
  outputdir=NULL;

  inputfn=cmd_ln_str("-input");
  outputfn=cmd_ln_str("-output");

  inputfmt=cmd_ln_str("-inputfmt");
  outputfmt=cmd_ln_str("-outputfmt");

  outputdir=cmd_ln_str("-outputdir");

  if(!strcmp(inputfmt,outputfmt))
    E_FATAL("Input and Output file formats and encodings are the same (%s). Do nothing\n",inputfmt);

  if(!strcmp(inputfmt,"TXT")&&!cmd_ln_int32("-lminmemory"))
    E_FATAL("When plain-txt LM is used as an input, only in memory mode of LM reading can be used, please set -lminmemory to be 1");
  
  /* Read LM */
  if((lm=lm_read_advance(inputfn,"default",1.0,0.1,1.0,0,inputfmt,0))==NULL)
    E_FATAL("Fail to read inputfn %s in inputfmt %s\n",inputfn,inputfmt);

  outputpath=(char*) ckd_calloc(1,strlen(outputdir)+strlen(outputfn)+6);

  if(cmd_ln_str("-wordlist"))
    lm_add_wordlist(lm,NULL, cmd_ln_str("-wordlist"));


#if WIN32
  strcpy(separator,"\\");
#else
  strcpy(separator,"/");
#endif

  sprintf(outputpath,"%s%s%s",outputdir,separator,outputfn);
  lm_write(lm,outputpath,inputfn,outputfmt);

  
  ckd_free(outputpath);
  lm_free(lm);
  cmd_ln_appl_exit();
  return 0;
}
