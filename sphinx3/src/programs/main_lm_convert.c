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
 * $Log: main_lm_convert.c,v $
 * Revision 1.4  2006/03/01 00:11:34  egouvea
 * Added MS Visual C projects for each of the missing executables, fixed
 * compilation problems. Code now compiles in MS .NET, with several
 * warnings such as "signed/unsigned mismatch" and "conversion ...,
 * possible loss of data", which we normally ignore.
 *
 * Revision 1.3  2006/02/24 13:43:43  arthchan2003
 * Temporarily removed allphone's compilation. used lm_read_advance in several cases.
 *
 * Revision 1.2  2006/02/24 03:50:30  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Added application lm_convert.
 *
 * Revision 1.1.2.5  2006/01/16 20:29:52  arthchan2003
 * Changed -ltsoov to -lts_mismatch. Changed lm_rawscore interface. Change from cmd_ln_access to cmd_ln_str.
 *
 * Revision 1.1.2.4  2005/11/17 06:48:58  arthchan2003
 * Support simple encoding conversion in lm_convert.
 *
 * Revision 1.1.2.3  2005/07/18 23:21:24  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.1.2.2  2005/07/17 06:00:21  arthchan2003
 * Added default argument in main_lm_convert.c, so the code will not die when -outputfmt is specified as nothing.
 *
 * Revision 1.1.2.1  2005/07/13 01:20:40  arthchan2003
 * Added lm_convert that could inter-convert DMP and plain-text format of language model file.
 *
 *
 */

#include <string.h>

#include <sphinxbase/info.h>
#include <sphinxbase/cmd_ln.h>

#include "lm.h"
#include "s3types.h"
#include "cmdln_macro.h"
#include "encoding.h"

static arg_t arg[] = {
    common_application_properties_command_line_macro(),
    {"-i",
     REQARG_STRING,
     NULL,
     "Input file"},
    {"-o",
     REQARG_STRING,
     NULL,
     "Output file, if not specified. the output format name will be used a suffix. "},
    {"-ifmt",
     REQARG_STRING,
     "TXT",
     "Input LM format. TXT (ARPA LM Format), DMP (Sphinx 3 efficient LM format bits), TXT32 (ARPA format but force the data structure to represent in 32 bits)."},
    {"-ofmt",
     ARG_STRING,
     "DMP",
     "Output LM format: TXT (ARPA LM Format), DMP (Sphinx 3 efficient LM format bits), DMP32 (Sphinx 3 efficient LM format forced 32 bits)."},
    /* Also FST for AT&T file format but the option is hidden because it
       is not well tested */
    /* Also DMP32 for LM which has 32 bits. This is yet another option I
       hired because we want to hide the format issue from the user. */
    {"-ienc",
     ARG_STRING,
     "iso8859-1",
     "Input encoding (pls. make sure this match with -outputenc), input could be: iso8859-1 (superset of ascii), gb2312-hex (hex code of gb2312)"},
    {"-oenc",
     ARG_STRING,
     "iso8859-1",
     "Output encoding (pls. make sure this match with -inputenc), output could be iso8859-1 (superset of ascii), gb2312"},
    {"-odir",
     ARG_STRING,
     ".",
     "Output Directory."},
    {NULL, ARG_INT32, NULL, NULL}
};



int
main(int argc, char *argv[])
{
    const char *inputfn;
    const char *outputfn;
    char *local_outputfn;
    const char *inputfmt;
    const char *outputfmt;
    const char *inputenc;
    const char *outputenc;
    const char *outputdir;
    char *outputpath;
    int outputfnfree = FALSE;
    lm_t *lm;
    char separator[2];
    cmd_ln_t *config;

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", arg);

    config = cmd_ln_get();

    inputfn = NULL;
    outputfn = local_outputfn = NULL;
    inputfmt = NULL;
    outputfmt = NULL;
    outputdir = NULL;

    inputfn = cmd_ln_str_r(config, "-i");
    outputfn = cmd_ln_str_r(config, "-o");

    inputfmt = cmd_ln_str_r(config, "-ifmt");
    outputfmt = cmd_ln_str_r(config, "-ofmt");

    inputenc = cmd_ln_str_r(config, "-ienc");
    outputenc = cmd_ln_str_r(config, "-oenc");

    outputdir = cmd_ln_str_r(config, "-odir");

    if (!strcmp(inputfmt, outputfmt) && !strcmp(inputenc, outputenc))
        E_FATAL
            ("Input and Output file formats and encodings are the same (%s, %s). Do nothing\n",
             inputfmt, inputenc);

    if (!encoding_resolve
        (cmd_ln_str_r(config, "-ienc"), cmd_ln_str_r(config, "-oenc")))
        E_FATAL
            ("Input and output encoding types is either not compatible or the conversion is not supported. Forced exit\n");


    /* Read LM */
    if ((lm =
         lm_read_advance2(inputfn, "default", 1.0, 0.1, 1.0, 0, inputfmt,
                          0, 1, NULL)) == NULL)
        E_FATAL("Fail to read inputfn %s in inputfmt %s\n", inputfn,
                inputfmt);

    if (outputfn == NULL) {
      /* Length = strlen(inputfn) + 1 + strlen(outputfmt) + 5 (For safety) */
      outputfn = local_outputfn = (char *) ckd_calloc(strlen(inputfn) + strlen(outputfmt) + 5, sizeof(char));
      sprintf(local_outputfn, "%s.%s", inputfn, outputfmt);
      outputfnfree = TRUE;
    }

    /* Outputpath = outputdir . "/" (or "\" in windows). outputfn; */
    /* Length = strlen(outputdir) + 1 + strlen(outputfn) + 5 (For safety) */
    outputpath =
      (char *) ckd_calloc(strlen(outputdir) + strlen(outputfn) + 6, sizeof(char));


#if WIN32
    strcpy(separator, "\\");
#else
    strcpy(separator, "/");
#endif

    sprintf(outputpath, "%s%s%s", outputdir, separator, outputfn);
    lm_write(lm, outputpath, inputfn, outputfmt);


    if (local_outputfn) {
      ckd_free(local_outputfn);
    }
    ckd_free(outputpath);

    lm_free(lm);
    cmd_ln_free_r(config);
    return 0;
}
