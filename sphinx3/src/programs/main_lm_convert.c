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
 * main_lm_convert.c -- Language model format converter
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
 * Started by Arthur Chan at July 11, 2005
 * 
 * $Log$
 * Revision 1.1.2.2  2005/07/17  06:00:21  arthchan2003
 * Added default argument in main_lm_convert.c, so the code will not die when -outputfmt is specified as nothing.
 * 
 * Revision 1.1.2.1  2005/07/13 01:20:40  arthchan2003
 * Added lm_convert that could inter-convert DMP and plain-text format of language model file.
 *
 *
 */

#include "lm.h"
#include "s3types.h"
#include "cmd_ln.h"

static arg_t arg[] = {
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
    "Input LM format. TXT or DMP"},
  { "-outputfmt",
    ARG_STRING,
    "DMP",
    "Output LM format: TXT or DMP"},
  { "-outputdir",
    ARG_STRING,
    ".",
    "Output Directory."},
  { "-lminmemory",
    ARG_INT32,
    "1",
    "Whether lm is memory-based(1) or disk-based (0)"},
  { "-logfn",
    ARG_STRING,
    NULL,
    "Log file (default stdout/stderr)" },
  { NULL, ARG_INT32,  NULL, NULL }
};



int main(int argc, char *argv[])
{
  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc, argv, "default.arg", arg);

  char *inputfn, *outputfn;
  char *inputfmt, *outputfmt;
  char *outputdir;
  char *outputpath;
  lm_t* lm;
  char separator[1];

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

  if(!strcmp(inputfmt,outputfmt)){
    E_INFO("Input and Output file formats are the same (%s). Do nothing\n",inputfmt);
    return 0;
  }

  if(!strcmp(inputfmt,"TXT")&&!cmd_ln_int32("-lminmemory")){
    E_FATAL("When plain-txt LM is used as an input, only in memory mode of LM reading can be used, please set -lminmemory to be 1");
  }


  /* Read LM */
  if((lm=lm_read(inputfn,"default",1.0,0.1,1.0,0,inputfmt,0))==NULL)
    E_FATAL("Fail to read inputfn %s in inputfmt %s\n",inputfn,inputfmt);


  /* Outputpath = outputdir . "/" (or "\" in windows). outpufn; */
  /* Length = strlen(outputdir) + 1 + strlen(outputfn) + 5 (For safety)*/
  outputpath=(char*) ckd_calloc(1,strlen(outputdir)+strlen(outputfn)+6);


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
