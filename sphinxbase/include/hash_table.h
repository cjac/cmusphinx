/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * $Log: hash.h,v $
 * Revision 1.7  2005/06/22 03:04:01  arthchan2003
 * 1, Implemented hash_delete and hash_display, 2, Fixed doxygen documentation, 3, Added  keyword.
 *
 * Revision 1.8  2005/05/24 01:10:54  archan
 * Fix a bug when the value only appear in the hash but there is no chain.   Also make sure that prev was initialized to NULL. All success cases were tested, but not tested with the deletion is tested.
 *
 * Revision 1.7  2005/05/24 00:12:31  archan
 * Also add function prototype for hash_display in hash.h
 *
 * Revision 1.4  2005/05/03 04:09:11  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.3  2005/03/30 01:22:48  archan
 * Fixed mistakes in last updates. Add
 *
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
 * This hash tables are intended for associating a pointer/integer
 * "value" with a char string "key", (e.g., an ID with a word string).
 * Subsequently, one can retrieve the value by providing the string
 * key.  (The reverse functionality--obtaining the string given the
 * value--is not provided with the hash table module.)
 */

/**
 * A note by ARCHAN at 20050510: Technically what we use is so-called
 * "hash table with buckets" which is very nice way to deal with
 * external hashing.  There are definitely better ways to do internal
 * hashing (i.e. when everything is stored in the memory.)  In Sphinx
 * 3, this is a reasonable practice because hash table is only used in
 * lookup in initialization or in lookups which is not critical for
 * speed.
 */

/**
 * Another note by ARCHAN at 20050703: To use this data structure
 * properly, it is very important to realize that the users are
 * required to handle memory allocation of the C-style keys.  The hash
 * table will not make a copy of the memory allocated for any of the
 * C-style key. It will not allocate memory for it. It will not delete
 * memory for it.  As a result, the following code sniplet will cause
 * memory leak.  
 *
 * while (1){
 * str=(char*)ckd_calloc(str_length,sizeof(char*))
 * if(hash_enter(ht,str,id)!=id){ printf("fail to add key str %s with val id %d\n",str,id)} 
 * }
 *
 */

/**
 * A note by dhuggins on 20061010: Changed this to use void * instead
 * of int32 as the value type, so that arbitrary objects can be
 * inserted into a hash table (in a way that won't crash on 64-bit
 * machines ;)
 */

#ifndef _LIBUTIL_HASH_H_
#define _LIBUTIL_HASH_H_

#include "prim_type.h"
#include "glist.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
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
	size_t len;			/** Key-length; the key string does not have to be a C-style NULL
					    terminated string; it can have arbitrary binary bytes */
	void *val;			/** Value associated with above key */
	struct hash_entry_s *next;	/** For collision resolution */
} hash_entry_t;

typedef struct {
	hash_entry_t *table;	/**Primary hash table, excluding entries that collide */
	int32 size;		/** Primary hash table size, (is a prime#); NOTE: This is the
				    number of primary entries ALLOCATED, NOT the number of valid
				    entries in the table */
	int32 inuse;		/** Number of valid entries in the table. */
	int32 nocase;		/** Whether case insensitive for key comparisons */
} hash_table_t;


/** Access macros */
#define hash_entry_val(e)	((e)->val)
#define hash_entry_key(e)	((e)->key)
#define hash_entry_len(e)	((e)->len)
#define hash_table_inuse(h)	((h)->inuse)
#define hash_table_size(h)	((h)->size)


/**
 * Allocate a new hash table for a given expected size.
 * Return value: READ-ONLY handle to allocated hash table.
 */
hash_table_t * hash_table_new(int32 size,	/**< In: Expected #entries in the table */
                              int32 casearg  	/**< In: Whether case insensitive for key
                                                   comparisons. When 1, case is insentitive,
                                                   0, case is sensitive. */
    );

#define HASH_CASE_YES		0
#define HASH_CASE_NO		1

/**
 * Free the specified hash table; the caller is responsible for freeing the key strings
 * pointed to by the table entries.
 */
void hash_table_free(hash_table_t *h /**< In: Handle of hash table to free */
    );


/**
 * Try to add a new entry with given key and associated value to hash table h.  If key doesn't
 * already exist in hash table, the addition is successful, and the return value is val.  But
 * if key already exists, return its existing associated value.  (The hash table is unchanged;
 * it is up to the caller to resolve the conflict.)
 */
void *hash_table_enter(hash_table_t *h, /**< In: Handle of hash table in which to create entry */
                       const char *key, /**< In: C-style NULL-terminated key string
                                           for the new entry */
                       void *val	  /**< In: Value to be associated with above key */
    );

/**
 * Add a new entry with given key and value to hash table h.  If the
 * key already exists, its value is replaced with the given value, and
 * the previous value is returned, otherwise val is returned.
 */
void *hash_table_replace(hash_table_t *h, /**< In: Handle of hash table in which to create entry */
                         const char *key, /**< In: C-style NULL-terminated key string
                                             for the new entry */
                         void *val	  /**< In: Value to be associated with above key */
    );


/**
 * Delete an entry with given key and associated value to hash table
 * h.  Return the value associated with the key (NULL if it did not exist)
 */

void *hash_table_delete(hash_table_t *h,    /**< In: Handle of hash table in
                                               which a key will be deleted */
                        const char *key     /**< In: C-style NULL-terminated
                                               key string for the new entry */
	);

/**
 * Delete all entries from a hash_table.
 */
void hash_table_empty(hash_table_t *h    /**< In: Handle of hash table */
    );

/**
 * Like hash_table_enter, but with an explicitly specified key length,
 * instead of a NULL-terminated, C-style key string.  So the key
 * string is a binary key (or bkey).  Hash tables containing such keys
 * should be created with the HASH_CASE_YES option.  Otherwise, the
 * results are unpredictable.
 */
void *hash_table_enter_bkey(hash_table_t *h,	/**< In: Handle of hash table
                                                   in which to create entry */
                              const char *key,	/**< In: Key buffer */
                              size_t len,	/**< In: Length of above key buffer */
                              void *val		/**< In: Value to be associated with above key */
	);

/*
 * Lookup hash table h for given key and return the associated value in *val.
 * Return value: 0 if key found in hash table, else -1.
 */
int32 hash_table_lookup(hash_table_t *h,	/**< In: Handle of hash table being searched */
                        const char *key,	/**< In: C-style NULL-terminated string whose value is sought */
                        void **val	  	/**< Out: *val = value associated with key */
	);

/**
 * Like hash_lookup, but with an explicitly specified key length, instead of a NULL-terminated,
 * C-style key string.  So the key string is a binary key (or bkey).  Hash tables containing
 * such keys should be created with the HASH_CASE_YES option.  Otherwise, the results are
 * unpredictable.
 */
int32 hash_table_lookup_bkey(hash_table_t *h,	/**< In: Handle of hash table being searched */
                             const char *key,	/**< In: Key buffer */
                             size_t len,	/**< In: Length of above key buffer */
                             void **val		/**< Out: *val = value associated with key */
	);

/**
 * Build a glist of valid hash_entry_t pointers from the given hash table.  Return the list.
 */
glist_t hash_table_tolist(hash_table_t *h,	/**< In: Hash table from which list is to be generated */
                          int32 *count		/**< Out: #entries in the list */
	);

/**
 * Display a hash-with-chaining representation on the screen.
 * Currently, it will only works for situation where hash_enter was
 * used to enter the keys. 
 */
void  hash_table_display(hash_table_t *h, /**< In: Hash table to display */
                         int32 showkey    /**< In: Show the string or not,
                                             Use 0 if hash_enter_bkey was
                                             used. */
	);

#ifdef __cplusplus
}
#endif

#endif
