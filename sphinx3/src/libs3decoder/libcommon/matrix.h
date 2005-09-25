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
 * matrix.h
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 *
 * $Log$
 * Revision 1.1.2.1  2005/09/19  05:17:37  arthchan2003
 * (Not compiled) A matrix toolbox.
 * 
 *
 */
#include "vector.h"

#define MATRIX_FLOAT 1
#define MATRIX_INT 2

#define MATRIX_OPERATION_FAIL -1
#define MATRIX_OPERATION_SUCCESS 1

typedef struct {
  fpoint_t **f; /**< The floating point data */
  ipoint_t **i; /**< The integer data */
  int32 r;  /**< The number of rows */
  int32 c;  /**< The number of columns */
  int32 MODE; /**< 1, using floating point, 2, using integer */
} matrix_t ;


matrix_t* matrix_init(int32 r,  /**< Input: number of row */
		      int32 c,  /**< Output: number of row */
		      int32 mode /**< The mode of operation */
		      );

int32 matrix_is_square(matrix_t *m /**< Input: The matrix */
		       );
void matrix_free(matrix_t *m /**< Input: The matrix which will be freed */
		 );

int32 matrix_add(matrix_t* a, /**< Input: A and Output: A+B*/
		 matrix_t* b  /**< Input: B*/
		 );

int32 matrix_print(matrix *a,  /**< Input: A */
		   int32 description /**< Description */
		   );


int32 matrix_lupsolve(matrix *L, /**< The L matrix */
		      matrix *U, /**< The U matrix */
		      matrix *P, /**< The permutation matrix */
		      matrix* b, /**< The b vector */
		      matrix **x /**< The final answer */
		      );
