/*
 * hash.h -- Hash table related
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 16-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Fil Alleva's original.
 */


#ifndef _HASH_H_
#define _HASH_H_

#include <sys/types.h>

typedef struct {
	caddr_t	val;			/* Associated Object */
	char const *obj;		/* Hash Object */
} hent_t;


typedef struct {
    int32	size_hint;		/* For initial allocation */
    int32	size;			/* Number entries in the table */
    int32	inuse;			/* Number entires in use */
    hent_t	*tab;			/* The table */
} hash_t;

list_t *hash_to_list(hash_t *ht);
int32 hash_add(hash_t *ht, char const *sym, caddr_t val);
int32 hash_lookup(hash_t *ht, char const *sym, caddr_t *val);
int hash_free(hash_t *ht);

#endif /* _HASH_H_ */
