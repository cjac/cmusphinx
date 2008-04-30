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
 * idngram2lm.c -implement method of idngram manipulation
 *
 * $Log: idngram2lm.c,v $
 * Revision 1.2  2006/04/13 17:33:26  archan
 * 0, This particular change enable 32bit LM creation in ARPA format.  1, rationalized/messed up the data type, (Careful, with reading and writing for 8-byte data structure, they are not exactly working at this point.) 2, all switches in idngram2lm is changed to be implemented by the disc_meth object.
 *
 * Revision 1.1  2006/04/02 23:52:11  archan
 * Added routines to specialize in methods of idngram and ngram.
 *
 */

#include "pc_general.h"   // from libs
#include "general.h"   // from libs

void show_idngram_nlines(int nlines, int verbosity)
{
  if (nlines % 20000 == 0) {
    if (nlines % 1000000 == 0)
      pc_message(verbosity,2,".\n");
    else 
      pc_message(verbosity,2,".");
  }
}

void show_idngram_corruption_mesg()
{
  quit(-1,"Error in idngram stream. This is most likely to be caused by trying to read\na gzipped file as if it were uncompressed. Ensure that all gzipped files have\na .gz extension. Other causes might be confusion over whether the file is in\nascii or binary format.\n");
}

