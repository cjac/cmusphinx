/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * adaptor.c -- Wrapper for structures of adaptation. 
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2005/06/21  17:59:44  arthchan2003
 * Log: Implementation and Interface of adapt_am_t, a wrapper of
 * adaptation capabability of Sphinx 3.  It takes the responsibility from
 * kb_t to manage regA, regB and mllr_nclass.  Interfaces are not fully
 * completed. So "pointer" symtom code still appears in kb.c
 * 
 * Revision 1.5  2005/06/19 19:41:21  archan
 * Sphinx3 to s3.generic: Added multiple regression class for single stream MLLR. Enabled MLLR for livepretend and decode.
 *
 * Revision 1.4  2005/05/11 06:10:37  archan
 * Code for lattice and back track pointer table dumping is now wrapped in reg_result_dump.  The function is shared across mode 4 and mode 5.  Possibly later for mode 3 and mode 6 as well.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 25-Mar-2005  Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon Unversity
 *              First started. 
 */

#include "adaptor.h"

#define ADAPT_FILE_NAME_LENGTH 1024

adapt_am_t* adapt_am_init()
{
  adapt_am_t* ad=(adapt_am_t *) ckd_calloc (1, sizeof(adapt_am_t));

  ad->prevmllrfn=(char*)ckd_calloc(ADAPT_FILE_NAME_LENGTH,sizeof(char));
  ad->prevmllrfn[0]='\0';

  return ad;
}

void adapt_am_free(adapt_am_t* ad)
{
  if(ad){
    if(ad->prevmllrfn){
      ckd_free((void*)ad->prevmllrfn);
    }
    ckd_free((void*)ad);
  }
}

