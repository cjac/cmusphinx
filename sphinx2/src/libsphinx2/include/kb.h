/* ====================================================================
 * Copyright (c) 1989-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 *------------------------------------------------------------*
 * DESCRIPTION
 *	Interface to Sphinx-II global knowledge base
 *-------------------------------------------------------------*
 * HISTORY
 * 
 * 27-May-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added Bob Brennan's declarations kbAddGrammar() and kb_get_personaldic().
 * 
 * 02-Apr-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added dict_maxsize().
 * 
 * Spring 89, Fil Alleva (faa) at Carnegie Mellon
 *	Created.
 */

#ifndef _KB_EXPORTS_H_
#define _KB_EXPORTS_H_

int32 kb_get_total_dists(void);
int32 kb_get_aw_tprob(void);
int32 kb_get_num_models(void);
int32 kb_get_num_dist(void);
int32 kb_get_num_model_instances(void);
int32 kb_get_num_words(void);
int32 kb_get_num_codebooks(void);
SMD  *kb_get_models(void);
char **kb_get_phone_list(void);
int32 *kb_get_codebook_0_dist(void);
int32 *kb_get_codebook_1_dist(void);
int32 *kb_get_codebook_2_dist(void);
int32 *kb_get_codebook_3_dist(void);
int32 kb_get_dist_prob_bytes(void);
int32 kb_get_start_word_id(void);
int32 kb_get_finish_word_id(void);
int32 kb_get_silence_word_id(void);
int32 **kb_get_word_transitions(void);
dictT *kb_get_word_dict(void);
LM     kb_get_lang_model(void);
int   kb_get_darpa_lm_flag(void);
int   kb_get_no_lm_flag(void);
char const *kb_get_lm_start_sym(void);
char const *kb_get_lm_end_sym(void);
char  *kb_get_word_str(int32 wid);
int32  kb_get_word_id(char const *word);
int32  dict_maxsize(void);
void   kbAddGrammar(char const *fileName, char const *grammarName);
char  *kb_get_dump_dir(void);
char  *kb_get_senprob_dump_file(void);
char  *kb_get_startsym_file(void);
int32  kb_get_senprob_size(void);
char  *kb_get_oovdic(void);
char  *kb_get_personaldic(void);
double kb_get_oov_ugprob(void);
int32  kb_get_max_new_oov(void);

void kb (int argc, char *argv[], float ip, float lw, float pip);


#endif /* _KB_EXPORTS_H_ */
