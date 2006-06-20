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
  $Log: mergeidngram.c,v $
  Revision 1.6  2006/06/19 21:02:08  archan
  Changed license from the original research-only license to the BSD license.

  Revision 1.5  2006/04/15 03:42:23  archan
  Make sure the 8-byte value is fixing.  Typecast all pointers to (char*)

  Revision 1.4  2006/04/13 17:36:37  archan
  0, This particular change enable 32bit LM creation in ARPA format.  Binary reading and writing are more complicated issues.  I will try to use the next 3 days to tackle them.  1, idngram2lm has been significantly rewritten. We start to see the most important 150 lines in LM counting code. (line 676 to 833 in v1.9)

 */

#include <stdio.h>
#include <stdlib.h>
#include "../libs/pc_general.h"
#include "../liblmest/toolkit.h"
#include "../liblmest/ngram.h"
#include "../libs/general.h"

int n;
flag ascii_in;
flag ascii_out;

void procComLine( int *argc, char **argv );
void printUsage( char *name );
int cmp_ngram( ngram *ng1, ngram *ng2 );
extern int get_ngram(FILE *id_ngram_fp, ngram *ng, flag ascii);

/* write ngram in either ascii or binary */
void write_ngram( FILE *id_ngram_fp, ngram *ng, flag ascii )
{
  int i;
  
  if ( ascii ) {
    for( i = 0; i < n; i++ ) {
      if ( fprintf( stdout, "%hu ", ng->id_array[i] ) < 0 ) 
	quit( -1, "error writing ascii ngram\n" );
    }
    if ( fprintf( stdout, "%d\n", ng->count ) < 0 )
      quit( -1, "error writing ascii ngram\n" );
  }else {
    for ( i = 0; i < n; i++ ) {
      rr_fwrite((char*) &ng->id_array[i], sizeof( id__t ), 1, id_ngram_fp,
		 "binary ngram" );
    }
    rr_fwrite( (char*) &ng->count, sizeof( int ), 1, id_ngram_fp,
	       "binary ngram" );
  }
}

/* process the command line */
void procComLine( int *argc, char **argv ) 
{
  int i;

  n = 3;
  ascii_in = 0;
  ascii_out = 0;

  i = *argc - 1 ;
  while( i > 0 ) {

    /* handle a request for help */
    if( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "-help" ) ) {
      printUsage( argv[0] ) ;
      exit( 1 ) ;
    }

    /* specify n */
    if( !strcmp( argv[i], "-n" ) ) {
      n = atoi( argv[i+1] ) ;
      updateArgs( argc, argv, i+1 ) ;
      updateArgs( argc, argv, i ) ;
    }
    
    /* input files in ascii */
    if( !strcmp( argv[i], "-ascii_input" ) ) {
      ascii_in = 1;
      updateArgs( argc, argv, i ) ;
    }

    /* input files in ascii */
    if( !strcmp( argv[i], "-ascii_output" ) ) {
      ascii_out = 1;
      updateArgs( argc, argv, i ) ;
    }

    i--;
  }
}
   
/* show command line usage */ 
void printUsage( char *name )
{
  fprintf( stderr, "%s: merge idngram files.\n", name );
  fprintf( stderr, "Usage:\n%s [options] .idngram_1 ... .idngram_N > .idngram\n", name );
  fprintf( stderr, "  -n 3           \tn in n-gram \n" );
  fprintf( stderr, "  -ascii_input   \tinput files are ascii\n" );
  fprintf( stderr, "  -ascii_output  \toutput files are ascii\n" );
  exit(1);
}

/* compare two ngrams */
int cmp_ngram( ngram *ng1, ngram *ng2 )
{
  int i;

  if ( ng1->n != ng2->n )
    quit( -1, "Error: n-grams have different n!\n" );

  for( i = 0; i < ng1->n; i++ ) {
    if ( ng1->id_array[i] < ng2->id_array[i] ) return( -1 );
    if ( ng1->id_array[i] > ng2->id_array[i] ) return( 1 );
  }
  return( 0 );
}
    
int main( int argc, char **argv )
{
  FILE **fin;
  ngram *ng;
  ngram outng;
  flag *done, finished;
  int i, j, nfiles;

  /* Process the command line */
  report_version(&argc,argv);
  procComLine( &argc, argv ) ;
  if( argc < 2 ) {
    printUsage( argv[0] ) ;
    exit( 1 ) ;
  }
  nfiles = argc - 1;

  /* allocate memory */
  fin = (FILE **) rr_malloc( sizeof( FILE *) * nfiles );
  done = (flag *) rr_malloc( sizeof( flag ) * nfiles );
  ng = (ngram *) rr_malloc( sizeof( ngram ) * nfiles );

  for( i = 0; i < nfiles; i++ ) {
    ng[i].id_array = (id__t *) rr_calloc( n, sizeof( id__t ) );
    ng[i].n = n;
  }
  outng.id_array = (id__t *) rr_calloc( n, sizeof( id__t ) );
  outng.n = n;

  /* open the input files */
  for( i = 0; i < nfiles; i++ )
    fin[i] = rr_iopen( argv[i+1] );

  /* read first ngram from each file */
  for( i = 0; i < nfiles; i++ ) {
    done[i] = 0;
    if ( !get_ngram( fin[i], &ng[i], ascii_in ) )
      done[i] = 1;
  }

  finished = 0;
  while ( !finished ) {
  /* set outng to max possible */
    for( i = 0; i < n; i++ )
      outng.id_array[i] = MAX_VOCAB_SIZE;
    
    /* find smallest ngram */
    for( i = 0; i < nfiles; i++ ) {
      if ( !done[i] ) 
	if ( cmp_ngram( &outng, &ng[i] ) > 0 ) 
	  for( j = 0; j < n; j++ ) outng.id_array[j] = ng[i].id_array[j];
    }
    
    outng.count = 0;
    for( i = 0; i < nfiles; i++ ) {
      if ( !done[i] ) {
	/* add counts of equal ngrams */
	if ( cmp_ngram( &outng, &ng[i] ) == 0 ) {
	  outng.count += ng[i].count;
	  if ( !get_ngram( fin[i], &ng[i], ascii_in ) ) {
	    /* check if all files done */
	    done[i] = 1;
	    finished = 1;
	    for( j = 0; j < nfiles; j++ ) 
	      if ( ! done[j] ) finished = 0;
	  }
	}
      }
    }

    write_ngram( stdout, &outng, ascii_out );
  }

  for( i = 0; i < nfiles; i++ )
    rr_iclose( fin[i] );

  fprintf(stderr,"mergeidngram : Done.\n");

  return( 0 );
}

