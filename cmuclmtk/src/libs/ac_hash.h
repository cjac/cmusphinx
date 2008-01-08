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

#ifndef _AC_HASH_H_
#define _AC_HASH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#define MAX_STRING_LENGTH 501

struct node {
  char *word;
  int count;
  struct node *next;
};

struct hash_table {
  int size;
  struct node **chain;
};

struct node *new_node( char *key );

void new_hashtable( struct hash_table *table, int M );

int update_chain( struct node *t, char *key );

int hash( char *key, int M );

void print( FILE* outfp, struct hash_table *table );

void update( struct hash_table *table, char *key, int verbosity );

int nearest_prime(int num);

struct idngram_node {
  char *word;
  wordid_t ind;
  struct idngram_node *next;
};

struct idngram_hash_table {
  int size;
  struct idngram_node **chain;
};

wordid_t index2(struct idngram_hash_table *vocab, char *word);

void add_to_idngram_hashtable( struct idngram_hash_table *table,
			       unsigned long position,
			       char *vocab_item,
			       wordid_t ind);

int idngram_hash( char *key, int M );

void new_idngram_hashtable( struct idngram_hash_table *table, int M );

#endif
