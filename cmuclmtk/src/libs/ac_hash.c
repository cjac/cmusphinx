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

/*
  Largely derived just from the code obtained from text2wfreq.c -AC
 */

#include "ac_hash.h"
#include "pc_general.h"
#include "general.h"

/* create a new node, and sets the count to 1 */
struct node *new_node( char *key )
{
  struct node *x;

  x = (struct node *) rr_malloc( sizeof( struct node ) );
  x->word = (char *) rr_malloc( (strlen( key ) + 1) * sizeof( char ) );
  strcpy( x->word, key );
  x->count = 1;
  return x;
}

/* create hash table */
void new_hashtable( struct hash_table *table, int M )
{
  int i;
  
  table->size = M;
  table->chain = (struct node **) rr_malloc( M * sizeof( struct node *) );
  for( i = 0; i < M; i++ ) {
    table->chain[i] = new_node( "HEAD_NODE" );
    table->chain[i]->next = (struct node *) NULL;
  }
}

/* update linked list */
int update_chain( struct node *t, char *key )
{
  struct node *x;
  int score;

  while( t->next != NULL ) {
    score = strcmp( key, t->next->word ); 
    /* move to next node */
    if ( score > 0 ) t = t->next;
    /* update node */
    else if ( score == 0 ) {
      t->next->count++;
      return 1;
    }
    /* add new node */
    else {
      x = new_node( key );
      x->next = t->next;
      t->next = x;
      return 0;
    }
  }
  /* add node at end */
  x = new_node( key );
  x->next = (struct node *) NULL;
  t->next = x;
  return 0;
}


/* generate a hash table address from a variable length character */
/* string - from R. Sedgewick, "Algorithms in C++". */
int hash( char *key, int M )
{
  unsigned int h; 
  char *t = key;

  for( h = 0; *t; t++ )
    h = ( 64 * h + *t ) % M;
  return h;
}

/* print contents of linked list */
static void print_chain(FILE* outfp, struct node *t )
{
  t = t->next;  /* don't print head node */
  while ( t != NULL ) {
    fprintf(outfp, "%s %d\n", t->word, t->count );
    t = t->next;
  }
}

/* print hash table contents */
void print(FILE* outfp, struct hash_table *table )
{
  int i;
  for( i = 0; i < table->size; i++ )
    print_chain(outfp, table->chain[i] );
}

/* update hash table contents */
void update( struct hash_table *table, char *key, int verbosity )
{
  int chain;
  
  chain = hash( key, table->size );
  if ( chain < 0 || chain >= table->size ) {
    pc_message(verbosity,1,"WARNING : invalid hash address.\n");
    pc_message(verbosity,1,"%s ignored\n", key );
    return;
  }
  update_chain( table->chain[ chain ], key );
}

/* Hashing functions, by Gary Cook (gdc@eng.cam.ac.uk).  Could use the
   sih functions used in idngrma2lm, but these are much faster. */

/* return the nearest prime not smaller than 'num' */
int nearest_prime(int num)
{
  int div;
  int num_has_divisor = 1;
  
  if ( num / 2 * 2 == num ) num++; 
  for (; num_has_divisor; num += 2 ) {
     num_has_divisor=0;
     for ( div = 3; div <= num / 3; div++ ) {
        if ( ( num / div) * div == num ) {
           num_has_divisor = 1;
           break;
        }
     }
  }
  num -= 2;
  return( num );
}


/** 
    ARCHAN 20060122 This is a customized implementation of hash table
    for text2idngram and wngram2idngram. 
 */

/* create a new node, and sets the index to ind */
struct idngram_node *idngram_new_node( char *key ,wordid_t ind)
{
  struct idngram_node *x;

  x = (struct idngram_node *) rr_malloc( sizeof( struct idngram_node ) );
  x->word = (char *) rr_malloc( (strlen( key ) + 1) * sizeof( char ) );
  strcpy( x->word, key );
  x->ind = ind;
  return x;
}

/* generate a hash table address from a variable length character */
/* string - from R. Sedgewick, "Algorithms in C++". */
int idngram_hash( char *key, int M )
{
  unsigned int h; 
  char *t = key;

  for( h = 0; *t; t++ )
    h = ( 256 * h + *t ) % M;

  return h;
}

/* create hash table */
void new_idngram_hashtable( struct idngram_hash_table *table, int M )
{
  int i;
  
  table->size = M;
  table->chain = (struct idngram_node **) rr_malloc( M * sizeof( struct idngram_node *) );
  for( i = 0; i < M; i++ ) {
    table->chain[i] = idngram_new_node( "HEAD_NODE" , 0);
    table->chain[i]->next = (struct idngram_node *) NULL;
  }

  /*  for(i = 0; i< M ; i++){
    fprintf(stderr, "%d %s ind %d, %d\n",i,table->chain[i]->word, table->chain[i]->ind, table->chain[i]->next);
    fflush(stderr);
    }*/
}

/* update linked list */
int idngram_update_chain( struct idngram_node *t, char *key ,wordid_t ind)
{
  struct idngram_node *x;

  /* Move to end of list */ 

  while( t->next != NULL )
    t = t->next;

  /* add node at end */
  x = idngram_new_node( key,ind );
  x->next = (struct idngram_node *) NULL;
  t->next = x;
  return 0;
}


void add_to_idngram_hashtable( struct idngram_hash_table *table,
			       unsigned long position,
			       char *vocab_item,
			       wordid_t ind) {

  idngram_update_chain( table->chain[position], vocab_item,ind );
}

wordid_t index2(struct idngram_hash_table *vocab,
		char *word) {
  
  unsigned long chain;
  struct idngram_node *chain_pos;

  chain = idngram_hash( word, vocab->size );
  if ( chain >= vocab->size ) {
    fprintf( stderr, "WARNING : invalid hash address\n" );
    fprintf( stderr, "%s ignored\n", word );
    return(0);
  }

  chain_pos = vocab->chain[chain];
  while (chain_pos->next != NULL) {
    if (strcmp(word,chain_pos->next->word) ) {
      fflush(stderr);
      chain_pos = chain_pos->next;
    }else
      return (chain_pos->next->ind);
  }
  return (0);

}

