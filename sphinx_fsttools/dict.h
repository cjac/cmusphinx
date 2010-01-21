/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

#ifndef _DICT_H_
#define _DICT_H_

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/* SphinxBase headers. */
#include <hash_table.h>
#include <glist.h>
#include <listelem_alloc.h>
#include <cmd_ln.h>

/* Local headers. */
#include "mdef.h"

#define NO_WORD		-1

/* DICT_ENTRY
 *------------------------------------------------------------*
 * DESCRIPTION
 * 	Each dictionary entry consists of the word pronunciation
 * and list of next words in this grammar for this word.
 */ 
typedef struct dict_entry {
    char	       *word;		/* Ascii rep of the word */
    int32	       *phone_ids;	/* gt List of the of the Phones */
    int32	       *ci_phone_ids;	/* context independent phone ids */
    int16               len;		/* Number of Phones in the word */
    int16		mpx;		/* Is this a multiplexed entry? */
    int32		wid;		/* Actual word id */
    int32		alt;		/* Alt word idx */
}                   dict_entry_t;

typedef struct dict_s {
    cmd_ln_t            *config;
    mdef_t		*mdef;
    hash_table_t 	*dict;
    hash_table_t 	*ciphones;
    char 		**ciphone_str;
    int32		ciphone_count;
    int32 		dict_entry_count;
    listelem_alloc_t    *dict_entry_alloc;
    dict_entry_t	**dict_list;
    int32		ci_index_len;	 	/* number of indecies */
    int32		*ci_index;		/* Index to each group */
    int32		filler_start;		/* Start of filler words */
} dict_t;

dict_t *dict_init(cmd_ln_t *config, mdef_t *mdef);
void dict_free(dict_t *dict);

#define DICT_SILENCE_WORDSTR	"<sil>"

int32 dict_to_id(dict_t *dict, char const *dict_str);
int32 dict_pron (dict_t *dict, int32 w, int32 **pron);
int32 dict_next_alt (dict_t *dict, int32 w);
int32 dict_get_num_main_words (dict_t *dict);

/* Return TRUE if the given wid is a filler word, FALSE otherwise */
int32 dict_is_filler_word (dict_t *dict, int32 wid);

int32 dict_ciphone_id(dict_t *dict, char const *phone);
char const *dict_ciphone_str(dict_t *dict, int32 pid);
int32 dict_n_ciphone(dict_t *dict);

#define dict_word_str(d,x)	((x == NO_WORD) ? "" : d->dict_list[x]->word)

#define dict_base_str(d,x)	((x == NO_WORD) ? "" :	\
				   d->dict_list[d->dict_list[x]->wid]->word)
#define dict_base_wid(d,x)	((x == NO_WORD) ? NO_WORD :	\
				   d->dict_list[x]->wid)

#define dict_pronlen(dict,wid)	((dict)->dict_list[wid]->len)
#define dict_ciphone(d,w,p)	((d)->dict_list[w]->ci_phone_ids[p])
#define dict_phone(d,w,p)	((d)->dict_list[w]->phone_ids[p])
#define dict_mpx(d,w)		((d)->dict_list[w]->mpx)

#define dict_n_words(d)		((d)->dict_entry_count)


#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif
#endif
