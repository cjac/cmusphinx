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

#ifndef _AC_LMFUNC_IMPL_H_
#define _AC_LMFUNC_IMPL_H_

#include "ac_hash.h"
#include "../liblmest/toolkit.h"
#include "../libs/general.h"
#include "../libs/pc_general.h"


int text2wfreq_impl(FILE* infp, FILE* outfp, int init_nwords, int verbosity);

int wfreq2vocab_impl(FILE* ifp, FILE* ofp, int cutoff, int vocab_size, int num_recs, int verbosity);

int read_vocab(char* vocab, 
	       int verbosity,
	       struct idngram_hash_table* vocabulary,
	       int M
	       );

int  read_txt2ngram_buffer(FILE* infp, 
			   struct idngram_hash_table *vocabulary, 
			   int32 verbosity,
			   wordid_t *buffer,
			   int buffer_size,
			   unsigned int n,
			   char* tempfiles_directory,
			   char* temp_file_root,
			   char* temp_file_ext,
			   FILE* temp_file
			   );

#endif
