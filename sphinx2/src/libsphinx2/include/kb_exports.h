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
 *	This is the interface exported by kb.c
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

#include <dict.h>
#include <lm_3g.h>
#include <msd.h>

extern kb_get_aw_tpron();
extern kb_get_num_models ();
extern kb_get_num_dist ();
extern kb_get_num_model_instances ();
extern SMD  *kb_get_models ();
extern char **kb_get_word_list ();
extern char **kb_get_phone_list ();
extern int32 *kb_get_codebook_0_dist ();
extern int32 *kb_get_codebook_1_dist ();
extern int32 *kb_get_codebook_2_dist ();
extern int32 *kb_get_codebook_3_dist ();
extern int32 kb_get_start_word_id ();
extern int32 kb_get_finish_word_id ();
extern int32 kb_get_silence_word_id ();
extern int32 **kb_get_word_transitions ();
extern dictT *kb_get_word_dict ();
extern LM     kb_get_lang_model ();
extern char  *kb_get_lm_start_sym();
extern char  *kb_get_lm_end_sym();
extern char  *kb_get_word_str (int32 wid);
extern int32  kb_get_word_id (char *word);
extern int32 dict_maxsize ( void );
extern	void	kbAddGrammar(char *fileName, char *grammarName);
extern	char	*kb_get_oovdic();
extern	char	*kb_get_personaldic();

#endif  _KB_EXPORTS_H_
