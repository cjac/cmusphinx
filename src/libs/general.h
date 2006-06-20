/* GENERAL.H  */
/*=====================================================================
                =======   COPYRIGHT NOTICE   =======
Copyright (C) 1994, Carnegie Mellon University and Ronald Rosenfeld.
All rights reserved.

This software is made available for research purposes only.  It may be
redistributed freely for this purpose, in full or in part, provided
that this entire copyright notice is included on any copies of this
software and applications and derivations thereof.

This software is provided on an "as is" basis, without warranty of any
kind, either expressed or implied, as to any matter including, but not
limited to warranty of fitness of purpose, or merchantability, or
results obtained from use of this software.
======================================================================*/

#ifndef _GENERAL_H_
#define _GENERAL_H_

#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define CMU_SLM_VERSION  "CMU-Cambridge SLM Toolkit, Version 3 alpha"

/* the following should be made machine-dependent */
typedef short int16;

FILE *rr_fopen(char *filename, char *mode);
void *rr_fseek(FILE *fp, int offset, int mode, char *description);

void *rr_fread(char *ptr, int elsize, size_t n_elem, FILE *fp,
	       char *header, int not_more);
void *rr_fwrite(char *ptr, int elsize, size_t n_elem, FILE *fp, char *header);

char *rr_malloc(size_t n_bytes);
char *rr_calloc(size_t nelem, size_t elsize);
int  rr_filesize(int fd);
int  rr_feof(FILE *fp);
char *salloc(char *str);
int  rr_fexists(char *path);
FILE *rr_iopen(char *path);
void *rr_iclose(FILE *fp);
FILE *rr_oopen(char *path);
void *rr_oclose(FILE *fp);
void parse_line(char *line, int mwords, int canonize,
    char **pword_begin, char **pword_end, int *p_nwords, int *p_overflow);
int quit(int rc, char *msg, ...);

typedef char   Boolean;
typedef int    cluster_t;

#define BAD_LOG_PROB -99.999
#define BAD_PROB 1e-99

#ifdef THIRTYTWOBITS

#warning "CMU-Cambridge LM Toolkit V3 alpha is compiled under 32 bits mode.  (Support more than 65536 words in unigram but it's less memory efficient)"
typedef unsigned int wordid_t;

typedef long long int ngram_sz_t;
typedef unsigned long long int us_ngram_sz_t;

/*typedef int ngram_sz_t;
  typedef unsigned int us_ngram_sz_t;*/

typedef unsigned int fof_sz_t;

#define MAX_WORDID 40000000  /* ARCHAN, 40 million now, this could be safely changed.  */
#define KEY 65000
#define DEFAULT_COUNT_TABLE_SIZE 65535

#define BBO_FILE_VERSION 060402
#else

/*
#warning "CMU-Cambridge LM Toolkit V3 alpha is compiled under 16 bits mode.  (Support less than 65536 words in unigram but it's more memory-efficient.)"
*/
typedef unsigned short wordid_t;
typedef int ngram_sz_t;
typedef unsigned int us_ngram_sz_t;
typedef unsigned short fof_sz_t;

#define MAX_WORDID 65535 
#define KEY 65000
#define DEFAULT_COUNT_TABLE_SIZE 65535
#define BBO_FILE_VERSION 970314
#endif

typedef us_ngram_sz_t us_vocab_sz_t;
typedef ngram_sz_t vocab_sz_t; /* tie vocab_sz_t with ngram_sz_t */
typedef int   int32;

#ifndef MIN
#define MIN(X,Y)  ( ((X)<(Y)) ? (X) : (Y))
#endif
#ifndef MAX
#define MAX(X,Y)  ( ((X)>(Y)) ? (X) : (Y))
#endif

#define LOG_BASE	9.9995e-5
#define MIN_LOG		-690810000
#define LOG(x) ((x == 0.0) ? MIN_LOG : ((x > 1.0) ?			    \
				 	(int) ((log (x) / LOG_BASE) + 0.5) :\
				 	(int) ((log (x) / LOG_BASE) - 0.5)))
#define EXP(x)  (exp ((double) (x) * LOG_BASE))

#define MAX_UNIGRAM 65535 /* ARCHAN, to change this, several data types need to change from short to X (X=int or long long) */

/* the following are for the benefit of vararg-less environments */

#define quit0(rc,msg) {fprintf(stderr,msg); exit(rc);}
#define quit1(rc,msg,i1) {fprintf(stderr,msg,i1); exit(rc);}
#define quit2(rc,msg,i1,i2) {fprintf(stderr,msg,i1,i2); exit(rc);}
#define quit3(rc,msg,i1,i2,i3) {fprintf(stderr,msg,i1,i2,i3); exit(rc);}
#define quit4(rc,msg,i1,i2,i3,i4) {fprintf(stderr,msg,i1,i2,i3,i4); exit(rc);}

#define  MAX_WORDS_PER_DOC 65534

/*#ifdef DMALLOC
#include "dmalloc.h"
#endif*/


#endif  
