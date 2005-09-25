/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * gau_reader.c -- Gaussian Selection Map File Reader Routine. 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1.2.1  2005/09/25  20:05:31  arthchan2003
 * Second New Tool of September: gauvq_read. It can be used to generate Gaussian Selection map.
 * 
 *
 * 28-Dec-2003	Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 * 		First created
 */

#include <libutil/libutil.h>

#include "s3types.h"
#include "cmdln_macro.h"
#include "gs.h"

#include "math.h"

static arg_t defn[] = {
  common_application_properties_command_line_macro()
  gaussian_selection_command_line_macro()
  { NULL, ARG_INT32, NULL, NULL }
};

int32 main (int32 argc, char *argv[])
{
  gs_t *gs=NULL;
  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",defn);
  unlimit ();

  gs_display(cmd_ln_str("-gs"),gs);

#if (! WIN32)
  system ("ps auxwww | grep gs_read");
#endif
  
  cmd_ln_appl_exit();

  return 0;
}
