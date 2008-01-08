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
 * ac_parsetext.c - utility functions used in parse different type of
 * file formats.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2006 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: ac_parsetext.c,v $
 * Revision 1.1  2006/03/31 04:07:50  archan
 * Tied 6 instance of line warning to ac_parsetext.[ch].
 *
 *
 */

#include <string.h>
#include "ac_parsetext.h"

void warn_on_wrong_vocab_comments(char *sent_str){
  if (strncmp(sent_str,"#",1)==0) {
    fprintf(stderr,"\n\n===========================================================\n");
    fprintf(stderr,":\nWARNING: line assumed NOT a comment:\n");
    fprintf(stderr,     ">>> %s <<<\n",sent_str);
    fprintf(stderr,     "         '%s' will be included in the vocabulary.\n",sent_str);
    fprintf(stderr,     "         (comments must start with '##')\n");
    fprintf(stderr,"===========================================================\n\n");
  }
}

void warn_on_repeated_words(char *str){
  fprintf(stderr,"======================================================\n");
  fprintf(stderr,"WARNING: word %s is repeated in the vocabulary.\n",str);
  fprintf(stderr,"=======================================================\n");
}
