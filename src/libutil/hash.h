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
 * hash.h -- Hash table module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 05-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Removed hash_key2hash().  Added hash_enter_bkey() and hash_lookup_bkey(),
 * 		and len attribute to hash_entry_t.
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added hash_key2hash().
 * 
 * 18-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Included case sensitive/insensitive option.
 * 
 * 08-31-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


/** \file hash.h
 * \brief Hash table implementation
 *
 * This hash tables are intended for associating an integer "value" with a
 * char string "key", (e.g., an ID with a word string).  Subsequently,
 * one can retrieve the integer value by providing the string key.
 * (The reverse functionality--obtaining the string given the
 * value--is not provided with the hash table module.)  
 */

#ifndef _LIBUTIL_HASH_H_
#define _LIBUTIL_HASH_H_

#include "prim_type.h"
#include "glist.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * The hash table structures.
   * Each hash table is identified by a hash_table_t structure.  hash_table_t.table is
   * pre-allocated for a user-controlled max size, and is initially empty.  As new
   * entries are created (using hash_enter()), the empty entries get filled.  If multiple
   * keys hash to the same entry, new entries are allocated and linked together in a
   * linear list.
   */

typedef struct hash_entry_s {
    const char *key;		/** Key string, NULL if this is an empty slot.
				   NOTE that the key must not be changed once the entry
				   has been made. */
    int32 len;			/** Key-length; the key string does not have to be a C-style NULL
				   terminated string; it can have arbitrary binary bytes */
    int32 val;			/** Value associated with above key */
    struct hash_entry_s *next;	/** For collision resolution */
} hash_entry_t;

typedef struct {
    hash_entry_t *table;	/**Primary hash table, excluding entries that collide */
    int32 size;			/** Primary hash table size, (is a prime#); NOTE: This is the
				   number of primary entries ALLOCATED, NOT the number of valid
				   entries in the table */
    uint8 nocase;		/** Whether case insensitive for key comparisons */
} hash_table_t;


  /** Access macros */
#define hash_entry_val(e)	((e)->val)
#define hash_entry_key(e)	((e)->key)
#define hash_entry_len(e)	((e)->len)
#define hash_table_size(h)	((h)->size)


  /**
   * Allocate a new hash table for a given expected size.
   * Return value: READ-ONLY handle to allocated hash table.
   */
hash_table_t *
hash_new (int32 size,		/* In: Expected #entries in the table */
	  int32 casearg);	/* In: Whether case insensitive for key comparisons.
				   Use values provided below */
#define HASH_CASE_YES		0
#define HASH_CASE_NO		1


  /**
 * Free the specified hash table; the caller is responsible for freeing the key strings
 * pointed to by the table entries.
 */
void hash_free (hash_table_t *h);


  /**
   * Try to add a new entry with given key and associated value to hash table h.  If key doesn't
   * already exist in hash table, the addition is successful, and the return value is val.  But
   * if key already exists, return its existing associated value.  (The hash table is unchanged;
   * it is upto the caller to resolve the conflict.)
   */
int32
hash_enter (hash_table_t *h,	/* In: Handle of hash table in which to create entry */
	    const char *key,	/* In: C-style NULL-terminated key string for the new entry */
	    int32 val);		/* In: Value to be associated with above key */

  /**
   * Like hash_enter, but with an explicitly specified key length, instead of a NULL-terminated,
   * C-style key string.  So the key string is a binary key (or bkey).  Hash tables containing
   * such keys should be created with the HASH_CASE_YES option.  Otherwise, the results are
   * unpredictable.
   */
int32
hash_enter_bkey (hash_table_t *h,	/* In: Handle of hash table in which to create entry */
		 const char *key,	/* In: Key buffer */
		 int32 len,		/* In: Length of above key buffer */
		 int32 val);		/* In: Value to be associated with above key */

/*
 * Lookup hash table h for given key and return the associated value in *val.
 * Return value: 0 if key found in hash table, else -1.
 */
int32
hash_lookup (hash_table_t *h,	/* In: Handle of hash table being searched */
	     const char *key,	/* In: C-style NULL-terminated string whose value is sought */
	     int32 *val);	/* Out: *val = value associated with key */

  /**
   * Like hash_lookup, but with an explicitly specified key length, instead of a NULL-terminated,
   * C-style key string.  So the key string is a binary key (or bkey).  Hash tables containing
   * such keys should be created with the HASH_CASE_YES option.  Otherwise, the results are
   * unpredictable.
   */
int32
hash_lookup_bkey (hash_table_t *h,	/* In: Handle of hash table being searched */
		  const char *key,	/* In: Key buffer */
		  int32 len,		/* In: Length of above key buffer */
		  int32 *val);		/* Out: *val = value associated with key */

  /**
   * Build a glist of valid hash_entry_t pointers from the given hash table.  Return the list.
   */
glist_t hash_tolist (hash_table_t *h,	/* In: Hash table from which list is to be generated */
		     int32 *count);	/* Out: #entries in the list */

#ifdef __cplusplus
}
#endif

#endif
