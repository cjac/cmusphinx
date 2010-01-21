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
 * HISTORY
 * 
 * 05-Nov-98  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		dict_load now terminates program if input dictionary 
 *              contains errors.
 * 
 * 21-Nov-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		Bugfix: Noise dictionary was not being considered in figuring
 *              dictionary size.
 * 
 * 18-Nov-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		Added ability to modify pronunciation of an existing word in
 *              dictionary (in dict_add_word()).
 * 
 * 10-Aug-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 *		Added check for word already existing in dictionary in 
 *              dict_add_word().
 * 
 * 27-May-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		Included Bob Brennan's personaldic handling (similar to 
 *              oovdic).
 * 
 * 11-Apr-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 *		Made changes to replace_dict_entry to handle the addition of
 * 		alternative pronunciations (linking in alt, wid, fwid fields).
 * 
 * 02-Apr-97  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 		Caused a fatal error if max size exceeded, instead of realloc.
 * 
 * 08-Dec-95  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 	Added function dict_write_oovdict().
 * 
 * 06-Dec-95  M K Ravishankar (rkm@cs.cmu.edu) at Carnegie-Mellon University
 * 	Added functions dict_next_alt() and dict_pron().
 * 
 * Revision 8.5  94/10/11  12:32:03  rkm
 * Minor changes.
 * 
 * Revision 8.4  94/07/29  11:49:59  rkm
 * Changed handling of OOV subdictionary (no longer alternatives to <UNK>).
 * Added placeholders for dynamic addition of words to dictionary.
 * Added dict_add_word () for adding new words to dictionary.
 * 
 * Revision 8.3  94/04/14  15:08:31  rkm
 * Added function dictid_to_str().
 * 
 * Revision 8.2  94/04/14  14:34:11  rkm
 * Added OOV words sub-dictionary.
 * 
 * Revision 8.1  94/02/15  15:06:26  rkm
 * Basically the same as in v7; includes multiple start symbols for
 * the LISTEN project.
 * 
 * 11-Feb-94  M K Ravishankar (rkm) at Carnegie-Mellon University
 * 	Added multiple start symbols for the LISTEN project.
 * 
 *  9-Sep-92  Fil Alleva (faa) at Carnegie-Mellon University
 *	Added special silences for start_sym and end_sym.
 *	These special silences
 *	(SILb and SILe) are CI models and as such they create a new context,
 *	however since no triphones model these contexts explicity they are
 *	backed off to silence, which is the desired context. Therefore no
 *	special measures are take inside the decoder to handle these
 *	special silences.
 * 14-Oct-92  Eric Thayer (eht) at Carnegie Mellon University
 *	added Ravi's formal declarations for dict_to_id() so int32 -> pointer
 *	problem doesn't happen on DEC Alpha
 * 14-Oct-92  Eric Thayer (eht) at Carnegie Mellon University
 *	added Ravi's changes to make calls into hash.c work properly on Alpha
 *	
 */

/* System headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* SphinxBase headers. */
#include <prim_type.h>
#include <cmd_ln.h>
#include <ckd_alloc.h>
#include <pio.h>
#include <hash_table.h>
#include <err.h>
#include <strfuncs.h>
#include <glist.h>

/* Local headers. */
#include "dict.h"

#ifdef DEBUG
#define DFPRINTF(x)		fprintf x
#else
#define DFPRINTF(x)
#endif

#define QUIT(x)		{fprintf x; exit(-1);}

static dict_entry_t *_new_dict_entry(dict_t *dict,
                                     char *word_str,
                                     char *pronoun_str);
static void _dict_list_add(dict_t * dict, dict_entry_t * entry);
static int dict_load(dict_t * dict, mdef_t *mdef,
                     char const *filename, int32 * word_id);

#define MAX_PRONOUN_LEN 	150

static int32
get_dict_size(char const *file)
{
    FILE *fp;
    __BIGSTACKVARIABLE__ char line[1024];
    int32 n;

    if ((fp = fopen(file, "r")) == NULL)
        return -1;
    for (n = 0;; n++)
        if (fgets(line, sizeof(line), fp) == NULL)
            break;
    fclose(fp);

    return n;
}

int32
dict_ciphone_id(dict_t *dict, char const *phone)
{
    if (dict->mdef)
        return mdef_ciphone_id(dict->mdef, (char *)phone);
    else {
        int32 pid;

        if (hash_table_lookup_int32(dict->ciphones, phone, &pid) < 0) {
            pid = dict->ciphone_count;
            if (dict->ciphone_str == NULL)
                dict->ciphone_str = ckd_calloc(1, sizeof(char *));
            else
                dict->ciphone_str = ckd_realloc(dict->ciphone_str,
                                                (pid + 1) * sizeof(char *));
            dict->ciphone_str[pid] = ckd_salloc(phone);
            (void)hash_table_enter_int32(dict->ciphones, dict->ciphone_str[pid], pid);
            ++dict->ciphone_count;
        }
        return pid;
    }
}

char const *
dict_ciphone_str(dict_t *dict, int32 pid)
{
    if (dict->mdef)
        return mdef_ciphone_str(dict->mdef, pid);
    else {
        return dict->ciphone_str[pid];
    }
}

int32
dict_n_ciphone(dict_t *dict)
{
    if (dict->mdef)
        return mdef_n_ciphone(dict->mdef);
    else {
        return dict->ciphone_count;
    }
}

dict_t *
dict_init(cmd_ln_t *config, mdef_t *mdef)
{
    dict_t *dict = ckd_calloc(1, sizeof(*dict));
    int32 word_id = 0, j;
    dict_entry_t *entry;
    void *val;
    char const *filename, *n_filename;

    dict->config = config;
    dict->mdef = mdef;
    if (mdef == NULL) 
        dict->ciphones = hash_table_new(40, HASH_CASE_NO);
    dict->ciphone_count = 0;
    dict->ciphone_str = NULL;
    dict->dict_entry_alloc = listelem_alloc_init(sizeof(dict_entry_t));
    filename = cmd_ln_str_r(config, "-dict");
    n_filename = cmd_ln_str_r(config, "-fdict");

    /*
     * Find size of dictionary and set hash and list table size hints.
     * (Otherwise, the simple-minded PC malloc library goes berserk.)
     */
    if ((j = get_dict_size(filename)) < 0){
        E_ERROR("Failed to open dictionary file %s\n", filename);
        dict_free(dict);
        return NULL;
    }
    if (n_filename)
        j += get_dict_size(n_filename);
    j += 3;                     /* </s>, <s> and <sil> */
    if (dict->dict)
        hash_table_free(dict->dict);
    if (cmd_ln_boolean_r(config, "-dictcase"))
        dict->dict = hash_table_new(j, HASH_CASE_YES);
    else
        dict->dict = hash_table_new(j, HASH_CASE_NO);

    /* Load dictionaries */
    if (dict_load(dict, mdef, filename, &word_id) != 0) {
        dict_free(dict);
        return NULL;
    }

    /* Make sure that <s>, </s>, and <sil> are always in the dictionary. */
    if (hash_table_lookup(dict->dict, "</s>", &val) != 0) {
        char pronstr[5];
        /*
         * Check if there is a special end silence phone.
         */
        if (-1 == dict_ciphone_id(dict, "SILe")) {
            strcpy(pronstr, "SIL");
            entry = _new_dict_entry(dict, "</s>", pronstr);
            if (!entry) {
                E_ERROR("Failed to add </s>(SIL) to dictionary\n");
                dict_free(dict);
                return NULL;
            }
        }
        else {
            E_INFO("Using special end silence for </s>\n");
            strcpy(pronstr, "SILe");
            entry =
                _new_dict_entry(dict, "</s>", pronstr);
        }
        _dict_list_add(dict, entry);
        hash_table_enter(dict->dict, entry->word, (void *)(long)word_id);
        entry->wid = word_id;
        word_id++;
    }

    dict->config = config;
    /* Mark the start of filler words */
    dict->filler_start = word_id;

    /* Add the standard start symbol (<s>) if not already in dict */
    if (hash_table_lookup(dict->dict, "<s>", &val) != 0) {
        char pronstr[5];
        /*
         * Check if there is a special begin silence phone.
         */
        if (-1 == dict_ciphone_id(dict, "SILb")) {
            strcpy(pronstr, "SIL");
            entry =
                _new_dict_entry(dict, "<s>", pronstr);
            if (!entry) {
                E_ERROR("Failed to add <s>(SIL) to dictionary\n");
                dict_free(dict);
                return NULL;
            }
        }
        else {
            E_INFO("Using special begin silence for <s>\n");
            strcpy(pronstr, "SILb");
            entry =
                _new_dict_entry(dict, "<s>", pronstr);
            if (!entry) {
                E_ERROR("Failed to add <s>(SILb) to dictionary\n");
                dict_free(dict);
                return NULL;
            }
        }
        _dict_list_add(dict, entry);
        hash_table_enter(dict->dict, entry->word, (void *)(long)word_id);
        entry->wid = word_id;
        word_id++;
    }

    /* Finally create a silence word if it isn't there already. */
    if (hash_table_lookup(dict->dict, "<sil>", &val) != 0) {
        char pronstr[4];

        strcpy(pronstr, "SIL");
        entry = _new_dict_entry(dict, "<sil>", pronstr);
        if (!entry) {
            E_ERROR("Failed to add <sil>(SIL) to dictionary\n");
            dict_free(dict);
            return NULL;
        }
        _dict_list_add(dict, entry);
        hash_table_enter(dict->dict, entry->word, (void *)(long)word_id);
        entry->wid = word_id;
        word_id++;
    }

    if (n_filename) {
        if (dict_load(dict, mdef, n_filename, &word_id) != 0) {
            dict_free(dict);
            return NULL;
        }
    }

    return dict;
}

void
dict_free(dict_t * dict)
{
    int32 i;
    int32 entry_count;
    dict_entry_t *entry;

    if (dict == NULL)
        return;

    entry_count = dict->dict_entry_count;

    for (i = 0; i < entry_count; i++) {
        entry = dict->dict_list[i];
        ckd_free(entry->word);
        ckd_free(entry->phone_ids);
        ckd_free(entry->ci_phone_ids);
    }
    for (i = 0; i < dict->ciphone_count; ++i) {
        ckd_free(dict->ciphone_str[i]);
    }
    ckd_free(dict->ciphone_str);
    listelem_alloc_free(dict->dict_entry_alloc);
    ckd_free(dict->dict_list);
    ckd_free(dict->ci_index);
    if (dict->ciphones)
        hash_table_free(dict->ciphones);
    if (dict->dict)
        hash_table_free(dict->dict);
    ckd_free(dict);
}

static int
dict_load(dict_t * dict, mdef_t *mdef,
          char const *filename, int32 *word_id)
{
    __BIGSTACKVARIABLE__ char dict_str[1024];
    __BIGSTACKVARIABLE__ char pronoun_str[1024];
    dict_entry_t *entry;
    FILE *fs;
    int32 start_wid = *word_id;
    int32 err = 0;

    if ((fs = fopen(filename, "r")) == NULL)
        return -1;

    pronoun_str[0] = '\0';
    while (EOF != fscanf(fs, "%s%[^\n]\n", dict_str, pronoun_str)) {
        int32 wid;
        /* Check for duplicate before we do anything. */
        if (hash_table_lookup_int32(dict->dict, dict_str, &wid) == 0) {
            E_WARN("Skipping duplicate definition of %s\n", dict_str);
            continue;
        }
        entry = _new_dict_entry(dict, dict_str, pronoun_str);
        if (!entry) {
            E_ERROR("Failed to add %s to dictionary\n", dict_str);
            err = 1;
            continue;
        }

        if (hash_table_enter_int32(dict->dict, entry->word, *word_id) != *word_id) {
            E_ERROR("Failed to add %s to dictionary hash!\n", entry->word);
            err = 1;
            continue;
        }
        _dict_list_add(dict, entry);
        entry->wid = *word_id;
        pronoun_str[0] = '\0';
        /*
         * Look for words of the form ".*(#)". These words are
         * alternate pronunciations. Also look for phrases
         * concatenated with '_'.
         */
        {
            char *p = strrchr(dict_str, '(');

            /*
             * For alternate pron. the last car of the word must be ')'
             * This will handle the case where the word is something like
             * "(LEFT_PAREN"
             */
            if (dict_str[strlen(dict_str) - 1] != ')')
                p = NULL;

            if (p != NULL) {
                void *wid;

                *p = '\0';
                if (hash_table_lookup(dict->dict, dict_str, &wid) != 0) {
                    E_ERROR
                        ("Missing first pronunciation for [%s]\nThis means that e.g. [%s(2)] was found with no [%s]\nPlease correct the dictionary and re-run.\n",
                         dict_str, dict_str, dict_str);
                    return -1;
                }
                DFPRINTF((stdout,
                          "Alternate transcription for [%s](wid = %d)\n",
                          entry->word, (long)wid));
                entry->wid = (long)wid;
                {
                    while (dict->dict_list[(long)wid]->alt >= 0)
                        wid = (void *)(long)dict->dict_list[(long)wid]->alt;
                    dict->dict_list[(long)wid]->alt = *word_id;
                }
            }
        }

        *word_id = *word_id + 1;
    }

    E_INFO("%6d = words in file [%s]\n", *word_id - start_wid, filename);

    if (fs)
        fclose(fs);

    return err;
}

int32
dict_to_id(dict_t * dict, char const *dict_str)
{
    int32 dictid;

    if (hash_table_lookup_int32(dict->dict, dict_str, &dictid) < 0)
        return NO_WORD;
    return dictid;
}

static dict_entry_t *
_new_dict_entry(dict_t *dict, char *word_str, char *pronoun_str)
{
    dict_entry_t *entry;
    __BIGSTACKVARIABLE__ char *phone[MAX_PRONOUN_LEN];
    __BIGSTACKVARIABLE__ int32 ciPhoneId[MAX_PRONOUN_LEN];
    __BIGSTACKVARIABLE__ int32 triphone_ids[MAX_PRONOUN_LEN];
    int32 pronoun_len = 0;
    int32 i;
    __BIGSTACKVARIABLE__ char position[256];         /* phone position */
    mdef_t *mdef = dict->mdef;

    memset(position, 0, sizeof(position));      /* zero out the position matrix */

    position[0] = 'b';          /* First phone is at begginging */

    while (1) {
        int n;
        char delim;

	if (pronoun_len >= MAX_PRONOUN_LEN) {
	    E_ERROR("'%s': Too many phones for bogus hard-coded limit (%d), skipping\n",
		    word_str, MAX_PRONOUN_LEN);
	    return NULL;
	}
        n = nextword(pronoun_str, " \t", &phone[pronoun_len], &delim);
        if (n < 0)
            break;
        pronoun_str = phone[pronoun_len] + n + 1;
        /*
         * An '&' in the phone string indicates that this is a word break and
         * and that the previous phone is in the end of word position and the
         * next phone is the begining of word position
         */
        if (phone[pronoun_len][0] == '&') {
            position[pronoun_len - 1] = WORD_POSN_END;
            position[pronoun_len] = WORD_POSN_BEGIN;
            continue;
        }
        ciPhoneId[pronoun_len] = dict_ciphone_id(dict, phone[pronoun_len]);
        if (ciPhoneId[pronoun_len] == -1) {
            E_ERROR("'%s': Unknown phone '%s'\n", word_str,
                    phone[pronoun_len]);
            return NULL;
        }
        pronoun_len++;
        if (delim == '\0')
            break;
    }

    position[pronoun_len - 1] = WORD_POSN_END;    /* Last phone is at the end */

    /*
     * If the position marker sequence 'ee' appears or 'se' appears
     * the sequence should be '*s'.
     */

    if (position[0] == WORD_POSN_END)     /* Also handle single phone word case */
        position[0] = WORD_POSN_SINGLE;

    for (i = 0; i < pronoun_len - 1; i++) {
        if (((position[i] == WORD_POSN_END)
             || (position[i] == WORD_POSN_SINGLE)) &&
            (position[i + 1] == WORD_POSN_END))
            position[i + 1] = WORD_POSN_SINGLE;
    }

    if (mdef && pronoun_len >= 2) {
        i = 0;

        triphone_ids[i] = mdef_phone_id(mdef,
                                        dict_ciphone_id(dict, phone[i]),
                                        dict_ciphone_id(dict, "SIL"),
                                        dict_ciphone_id(dict, phone[i+1]),
                                        WORD_POSN_BEGIN);
        if (triphone_ids[i] < 0)
            triphone_ids[i] = dict_ciphone_id(dict, phone[i]);
        triphone_ids[i] = mdef_pid2ssid(mdef, triphone_ids[i]);
        assert(triphone_ids[i] >= 0);

        for (i = 1; i < pronoun_len - 1; i++) {
            triphone_ids[i] = mdef_phone_id(mdef,
                                                dict_ciphone_id(dict, phone[i]),
                                                dict_ciphone_id(dict, phone[i-1]),
                                                dict_ciphone_id(dict, phone[i+1]),
                                                position[i]);
            if (triphone_ids[i] < 0)
                triphone_ids[i] = dict_ciphone_id(dict, phone[i]);
            triphone_ids[i] = mdef_pid2ssid(mdef, triphone_ids[i]);
            assert(triphone_ids[i] >= 0);
        }

        triphone_ids[i] = mdef_phone_id(mdef,
                                        dict_ciphone_id(dict, phone[i]),
                                        dict_ciphone_id(dict, phone[i-1]),
                                        dict_ciphone_id(dict, "SIL"),
                                        position[i]);
        if (triphone_ids[i] < 0)
            triphone_ids[i] = dict_ciphone_id(dict, phone[i]);
        triphone_ids[i] = mdef_pid2ssid(mdef, triphone_ids[i]);
        assert(triphone_ids[i] >= 0);
    }

    /*
     * It's too hard to model both contexts so I choose to model only
     * the left context.
     */
    if (mdef && pronoun_len == 1) {
        triphone_ids[0] = dict_ciphone_id(dict,phone[0]);
        triphone_ids[0] = mdef_pid2ssid(mdef,triphone_ids[0]);
    }

    entry = listelem_malloc(dict->dict_entry_alloc);
    entry->word = ckd_salloc(word_str);
    entry->len = pronoun_len;
    entry->alt = -1;
    if (pronoun_len != 0) {
        entry->ci_phone_ids =
            (int32 *) ckd_calloc((size_t) pronoun_len, sizeof(int32));
        memcpy(entry->ci_phone_ids, ciPhoneId,
               pronoun_len * sizeof(int32));
        entry->phone_ids =
            (int32 *) ckd_calloc((size_t) pronoun_len, sizeof(int32));
        memcpy(entry->phone_ids, triphone_ids,
               pronoun_len * sizeof(int32));
    }
    else {
        E_WARN("%s has no pronounciation, will treat as dummy word\n",
               word_str);
    }

    return (entry);
}

static void
_dict_list_add(dict_t * dict, dict_entry_t * entry)
/*------------------------------------------------------------*/
{
    if (!dict->dict_list)
        dict->dict_list = (dict_entry_t **)
            ckd_calloc(hash_table_size(dict->dict), sizeof(dict_entry_t *));

    if (dict->dict_entry_count >= hash_table_size(dict->dict)) {
        E_WARN("dict size (%d) exceeded\n", hash_table_size(dict->dict));
        dict->dict_list = (dict_entry_t **)
            ckd_realloc(dict->dict_list,
                        (hash_table_size(dict->dict) + 16) * sizeof(dict_entry_t *));
    }

    dict->dict_list[dict->dict_entry_count++] = entry;
}

int32
dict_get_num_main_words(dict_t * dict)
{
    /* FIXME FIXME: Relies on a particular ordering of the dictionary. */
    return dict_to_id(dict, "</s>");
}

int32
dict_pron(dict_t * dict, int32 w, int32 ** pron)
{
    *pron = dict->dict_list[w]->ci_phone_ids;
    return (dict->dict_list[w]->len);
}

int32
dict_next_alt(dict_t * dict, int32 w)
{
    return (dict->dict_list[w]->alt);
}

int32
dict_is_filler_word(dict_t * dict, int32 wid)
{
    return (wid >= dict->filler_start);
}
