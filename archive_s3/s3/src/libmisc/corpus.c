/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * corpus.c -- Corpus-file related misc functions.
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
 * 25-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if (_SUN4)
#include <unistd.h>
#endif
#include <math.h>

#include <libutil/libutil.h>
#include "corpus.h"


corpus_t *corpus_load_headid (char *file)
{
    FILE *fp;
    int32 ispipe;
    char line[16384], wd[4096], *id;
    int32 k, n;
    corpus_t *corp;
    
    E_INFO("Loading corpus (%s)\n", file);
    
    if ((fp = fopen_compchk(file, &ispipe)) == NULL)
	E_FATAL_SYSTEM("fopen_compchk(%s,r) failed\n", file);

    corp = (corpus_t *) ckd_calloc (1, sizeof(corpus_t));
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	/* Skip comment or empty lines */
	if ((line[0] != '#') && (sscanf (line, "%s%n", wd, &k) == 1))
	    n++;
    }
    rewind (fp);
    
    corp->ht = hash_new ("corpus", n);
    corp->n = 0;
    corp->str = (char **) ckd_calloc (n, sizeof(char *));
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	/* Skip comment or empty lines */
	if ((line[0] == '#') || (sscanf (line, "%s%n", wd, &k) != 1))
	    continue;

	corp->str[n] = ckd_salloc (line+k);
	id = ckd_salloc (wd);
	if (hash_enter (corp->ht, id, n) < 0)
	    E_FATAL("corpus_load(%s) failed\n", file);
	
	n++;
    }
    corp->n = n;
    
    fclose_comp (fp, ispipe);
    
    E_INFO("%d entries loaded\n", n);

    return corp;
}


char *corpus_lookup (corpus_t *corp, char *id)
{
    int32 n;
    
    if (hash_lookup (corp->ht, id, &n) < 0)
	return NULL;

    assert ((n >= 0) && (n < corp->n));
    return (corp->str[n]);
}


#if _CORPUS_TEST_
main (int32 argc, char *argv[])
{
    corpus_t *c;
    char id[4096], *str;
    
    if (argc != 2)
	E_FATAL("Usage: %s corpusfile\n", argv[0]);
    
    c = corpus_load_headid (argv[1]);
    for (;;) {
	printf ("> ");
	scanf ("%s", id);
	
	str = corpus_lookup (c, id);
	if (str == NULL)
	    printf ("%s Not found\n");
	else
	    printf ("%s: %s\n", id, str);
    }
}
#endif
