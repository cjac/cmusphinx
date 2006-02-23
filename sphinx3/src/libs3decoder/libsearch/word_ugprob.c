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
 * word_ugprob.c -- Unigram word probability structure 
 *
 * $Log$
 * Revision 1.2  2006/02/23  05:12:23  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: split word_ugprob's routine from flat_fwd
 * 
 * Revision 1.1.2.1  2005/09/18 01:17:07  arthchan2003
 * Add word_ugprob_t that is separated from flat_fwd.c
 *
 */

#include "word_ugprob.h"

word_ugprob_t**  init_word_ugprob(mdef_t *_mdef, lm_t *_lm, dict_t *_dict)
{
  /* WARNING! _dict and dict are two variables.*/

  s3wid_t w;
  s3cipid_t ci;
  int32 n_ug, ugprob;
  ug_t *ugptr;
  word_ugprob_t *wp, *prevwp;
  word_ugprob_t** wugp;

  wugp = (word_ugprob_t **) ckd_calloc (_mdef->n_ciphone,
					sizeof(word_ugprob_t *));
  n_ug = lm_uglist (_lm,&ugptr);
  for (; n_ug > 0; --n_ug, ugptr++) {
    if ((w = ugptr->dictwid) == _dict->startwid)
      continue;

    ugprob = LM_UGPROB(_lm, ugptr);

    for (; IS_S3WID(w); w = _dict->word[w].alt) {
      ci = _dict->word[w].ciphone[0];
      prevwp = NULL;
      for (wp = wugp[ci]; wp && (wp->ugprob >= ugprob); wp = wp->next)
	prevwp = wp;
      wp = (word_ugprob_t *) listelem_alloc (sizeof(word_ugprob_t));
      wp->wid = w;
      wp->ugprob = ugprob;
      if (prevwp) {
	wp->next = prevwp->next;
	prevwp->next = wp;
      } else {
	wp->next = wugp[ci];
	wugp[ci] = wp;
      }
    }
  }
  return wugp;
}
