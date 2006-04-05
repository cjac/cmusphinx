/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * word_ugprob.h -- Unigram word probability structure 
 *
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:12:23  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: split word_ugprob's routine from flat_fwd
 *
 * Revision 1.1.2.1  2005/09/18 01:17:07  arthchan2003
 * Add word_ugprob_t that is separated from flat_fwd.c
 *
 */

#include "s3types.h"
#include "mdef.h"
#include "lm.h"
#include "dict.h"

#ifndef WORD_UGPROB
#define WORD_UGPROB
/**
 * \struct word_ugprob_t
 *
 * Unigrams re-organized for faster unigram word transitions.  Words
 * partitioned by their first CI phone and ordered in descending
 * unigram probability within each partition.
 */
typedef struct word_ugprob_s {
  s3wid_t wid;        /**< Word ID */
  int32 ugprob;     /**< Unigram probability */
  struct word_ugprob_s *next;   /**< Nex unigram probability*/
} word_ugprob_t;

/**
 * Initialize word_ugprobability 
 */
word_ugprob_t**  init_word_ugprob(mdef_t *_mdef, 
				  lm_t *_lm, 
				  dict_t *_dict
				  );


#endif /*WORD_UGPROB*/
