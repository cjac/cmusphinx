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



/* Generates a pointer to an array of size (vocab_size+1) of flags
   indicating whether or not there should be forced backing off from
   the vocab item, and returns a pointer to it. */

#include <stdio.h>
#include <string.h>
#include "ac_parsetext.h"   // from libs
#include "evallm.h"

fb_info *gen_fb_list(sih_t *vocab_ht,
		     vocab_sz_t vocab_size,
		     char **vocab,
		     flag *context_cue,
		     flag backoff_from_unk_inc,
		     flag backoff_from_unk_exc,
		     flag backoff_from_ccs_inc,
		     flag backoff_from_ccs_exc,
		     char *fb_list_filename) {

  fb_info *fb_list;
  int i;
  FILE *fb_list_file;
  char current_fb_word[500];
  char inc_or_exc[500];
  vocab_sz_t current_fb_id;
  char wlist_entry[1024];

  fb_list = (fb_info *) rr_calloc(vocab_size+1,sizeof(fb_info));

  if (backoff_from_unk_inc) {
    fb_list[0].backed_off = 1;
    fb_list[0].inclusive = 1;
  }

  if (backoff_from_unk_exc) {
    fb_list[0].backed_off = 1;
    fb_list[0].inclusive = 0;
  }

  if (backoff_from_ccs_inc || backoff_from_ccs_exc) {
    for (i=0;i<=(int) vocab_size;i++) {
      if (context_cue[i]) {
	fb_list[i].backed_off = 1;
	if (backoff_from_ccs_inc) 
	  fb_list[i].inclusive = 1;
	else 
	  fb_list[i].inclusive = 0;
      }
    }
  }

  if (strcmp(fb_list_filename,"")) {
    fb_list_file = rr_iopen(fb_list_filename);
    while (fgets (wlist_entry, sizeof (wlist_entry),fb_list_file)) {
      if (strncmp(wlist_entry,"##",2)==0) continue;
      sscanf (wlist_entry, "%s %s",current_fb_word,inc_or_exc);

      warn_on_wrong_vocab_comments(wlist_entry);

      if (sih_lookup(vocab_ht,current_fb_word,&current_fb_id) == 0)
	fprintf(stderr,"Error : %s in the forced backoff list does not appear in the vocabulary.",current_fb_word);

      if (inc_or_exc[0] == 'i' || inc_or_exc[0] == 'I') {
	fb_list[current_fb_id].inclusive = 1;
	fb_list[current_fb_id].backed_off = 1;
      }else {
	if (inc_or_exc[0] == 'e' || inc_or_exc[0] == 'E') {
	  fb_list[current_fb_id].inclusive = 0;
	  fb_list[current_fb_id].backed_off = 1;
	}else 
	  fprintf(stderr,"Error in line of forced back-off list file.\nLine is : %s\n",wlist_entry);
      }
    }
    rr_iclose(fb_list_file);
  }

  return (fb_list);
}


