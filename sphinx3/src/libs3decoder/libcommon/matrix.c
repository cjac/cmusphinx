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
 * matrix.c
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

#include "matrix.h"
#include <math.h>

matrix_t* matrix_init(int32 r, int32 c, int32 mode)
{
  matrix_t *m;
  m=ckd_calloc(1,sizeof(matrix_t));

  m->MODE=mode;
  m->r=r;
  m->c=c;
  m->f=NULL;
  m->i=NULL;
  
  assert(mode==MATRIX_FLOAT||mode==MATRIX_INT);

  if(mode==MATRIX_FLOAT)
    m->f=ckd_calloc2d(r,c,MATRIX_FLOAT);
  else if(mode==MATRIX_FLOAT)
    m->i=ckd_calloc2d(r,c,MATRIX_INT);
  else
    E_ERROR("Deep panic, even after assertion, unknown mode still appears\n");

  return m;
}

matrix_t* matrix_dup(matrix_t *a)
{
  matrix_t *m;
  int32 i,j;
  m=ckd_calloc(1,sizeof(matric_t));
  m->MODE=a->MODE;
  m->r=a->r;
  m->c=a->c;
  m->f=NULL;
  m->i=NULL;

  assert(mode==MATRIX_FLOAT||mode==MATRIX_INT);
  if(mode==MATRIX_FLOAT){
    m->f=ckd_calloc2d(m->r,m->c,MATRIX_FLOAT);
    for(i=0;i<m->r;i++){
      for(j=0;j<m->c;j++){
	m->f[i][j]=a->f[i][j];
      }
    }
  }
  else if(mode==MATRIX_FLOAT){
    m->i=ckd_calloc2d(m->r,m->c,MATRIX_INT);
    for(i=0;i<m->r;i++){
      for(j=0;j<m->c;j++){
	m->i[i][j]=a->i[i][j];
      }
    }
  }
  else
    E_ERROR("Deep panic, even after assertion, unknown mode still appears\n");
  
}

void matrix_free(matrix_t *m)
{
  if(m!=NULL){
    if(MODE==MATRIX_FLOAT){
      if(m->f)
	ckd_free2d(m->f);
    }else if(MODE==MATRIX_INT){
      if(m->i)
	ckd_free2d(m->i);
    }else
      E_WARN("See invalid mode in matrix_free\n");

    ckd_free(m);
  }
}

/* What if overflow? */
int32 matrix_add(matrix_t *a,matrix_t *b)
{
  int32 i,j;
  /* Check whether the two matrices a and b could be added */
  if(a->r!=b->r ||a->c!=b->c){
    E_ERROR("Matrice sizes mismatch in addition");
    return MATRIX_OPERATION_FAIL;
  }

  if(a->MODE != b->MODE){
    E_ERROR("Matrice has different mode of operation");
    return MATRIX_OPERATION_FAIL;
  }

  for(i=0;i<a->r;i++){
    for(j=0;j<a->c;j++){
      if(mode==MATRIX_FLOAT)
	a->f[i][j]+=b->f[i][j];
      else if (mode==MATRIX_INT)
	a->i[i][j]+=b->i[i][j];
    }
  }
  return MATRIX_OPERATAION_SUCCESS;
}

/* What if overflow? */
int32 matrix_scale_fl(matrix_t *a,float32 scale){
  int32 i,j;
  if(mode!=MATRIX_FLOAT){
    E_ERROR("Scaling in a wrong mode");
    return MATRIX_OPERATION_FAIL;
  }

  assert(a->i==NULL);
  for(i=0;i<a->r;i++){
    for(j=0;j<a->c;j++){
      a->f[i][j]*=scale;
    }
  }
  return MATRIX_OPERATION_SUCCESS;
}

/* What if overflow? */
int32 matrix_scale_int(matrix_t *a,int32 scale){
  int32 i,j;
  if(mode!=MATRIX_INT){
    E_ERROR("Scaling in a wrong mode");
    return MATRIX_OPERATION_FAIL;
  }

  assert(a->f==NULL);
  for(i=0;i<a->r;i++){
    for(j=0;j<a->c;j++){
      a->i[i][j]*=scale;
    }
  }
  
  return MATRIX_OPERATION_SUCCESS;
}

int32 matrix_print(matrix_t *a, int32 description)
{
  int32 i,j;
  if(description) {
    printf("%7.4s ","");
    for(j=0;j<a->c ;j++)
      printf("%7.4s%d%s ","[",j,"]");
  }

  for(i=0;i<a->r;i++){
    for(j=0;j<a->c;j++){
      if(description)
	printf("%7.4d ",i);
      if(a->MODE==MATRIX_FLOAT)
	printf("%7.4f ",a->f[i][j]);
      else if(a->MODE==MATRIX_INT)
	printf("%7.4d ",a->i[i][j]);
      else
	E_INFO("Panic\n");
    }
    printf("\n");
  }
  fflush(stderr);
}

/* What if overflow?*/
/*
  The O(n^3) algorithm. In the case of square matrix, we could use
  Strassen's algorithm and could get O(n^2.82). 
 */
int32  matrix_multiply(matrix_t *a,matrix_t *b, matrix_t** c)
{
  int32 i,j,k;
  matrix_t *m;

  m=*c;

  if(a->c!=b->r){
    E_ERROR("Number of columns in a is not equal to number of rows in b\n");
    return MATRIX_OPERATION_FAIL;
  }

  if(a->r!=c->r||b->c!=c->c){
    E_ERROR("The output matrix doesn't have the correct dimension\n");
    return MATRIX_OPERATION_FAIL;
  }

  if(a->MODE != b->MODE){
    E_ERROR("Matrice has different mode of operation");
    return MATRIX_OPERATION_FAIL;
  }

  if(m==NULL)
    m=matrix_init(a->r,b->c,a->MODE);

  for(i=0;i<a->r;i++){
    for(j=0;j<b->c;j++){
      if(a->MODE==MATRIX_FLOAT){
	m->f[i][j]=0;
	for(k=0;k<a->c;k++){
	  m->f[i][j]+=a->f[i][k]*b->f[k][j];
	}
      }else if(a->MODE==MATRIX_INT){
	m->i[i][j]=0;
	for(k=0;k<a->c;k++){
	  m->i[i][j]+=a->i[i][k]*b->i[k][j];
	}
      }else{
	E_INFO("Panic\n");
      }
    }
  }
  return MATRIX_OPERATION_SUCCESS;
}

int32 matrix_is_square(matrix_t *m)
{
  assert(m);
  return (m->r==m->c);
}


/** The numerical accuracy is not good enough in general. Consider to use float64 */
int32 matrix_lupsolve(matrix_t *L, matrix_t *U, matrix_t *P, matrix_t* b, vector_t **x)
{
  int32 i,j;
  int32 n;
  matrix_t *perm_b; /* permuted b*/
  matrix_t *y;
  vector_t *ans;
  float32 sum;

  /* First do sanity checking */
  if( !matrix_is_square(L)||!matrix_is_sqaure(U)||!matrix_is_sqaure(P))
  {
    E_ERROR("Either one of L, U or the permutation matrix is not a square matrix\n");
    return MATRIX_OPERATION_FAIL;
  }
  
  /* FIXME , only assertion on b and x*/
  assert(b);
  assert(b->c==1);
  assert(*x);

  if(L->r!=U->r||U->r!=P->r||P->r!=b->r){
    E_ERROR("Dimension mismatch in L, U and P, L->r %d, U->r %d, P->r %d, b->r %d\n",L->r,U->r,P->r, b->r);
    return MATRIX_OPERATION_FAIL;
  }
  
  n=L->r;

  assert(P->MODE==MATRIX_FLOAT);
  perm_b=matrix_init(P->r,1,MATRIX_FLOAT);
  y=matrix_init(P->r,1,MATRIX_FLOAT);
  
  /* First create the permuted b matrix. Of course, there are cheaper ways to do it. */
  matrix_multply(P,b,&perm_b);
  
  /* Now the meat, backward and forward subsitution*/
  /* Forward subsitutation*/
  for(i=0;i<n;i++){
    sum=0;
    for(j=0;j<i;j++){
      sum+=L[i][j]* y[j][0];
    }
    y[i][0]=perm_b[i][0] - sum;
  }
  
  /* Backward subsitutation*/
  for(i=n-1;i>=0;i--){
    sum=0;
    for(j=i+1;j<n;j++){
      sum+=U[i][j] * ans[j];
    }
    ans[i]= y[i][0] - sum / U[i][i] ;
  }
  
  /* Now the answer will be found in *x;*/
  matrix_free(perm_b);
  matrix_free(y);
  return MATRIX_OPERATION_SUCCESS;
}



/** This only works for semi-definite matrix, if a_{ii} or the upper
    element of Schur components is zero, this algorithm doens't
    work. In general, one should use matrix_lupdecompose in general. 
*/

int32 matrix_ludecompose(matrix_t *A, matrix_t **L_mat, matrix_t **U_mat)
{
  int32 n;
  int32 i, j, k;
  matrix *dupA;
  matrix *L, *U;

  L=*L_mat;
  U=*U_mat;

  assert(A);
  assert(L);
  assert(U);

  if(!matrix_is_square(A) || !matrix_is_square(L) || !matrix_is_square(U) ){
    E_ERROR("Either one of matrix A, L or U is not square matrix\n");
    return MATRIX_OPERATION_FAIL;
  }

  if(A->r!=L->r ||L->r!=U->r){
    E_ERROR("Dimensiont mismatch in A, L, U  A->r %d, L->r %d, U->r %d\n",A->r,L->r, U->r);
    return MATRIX_OPERATION_FAIL;
  }

  assert(A->MODE==MATRIX_FLOAT);
  dupA=matrix_dup(A);

  n=A->r;

  for(k=0;k<n;k++){
    U[k][k]=dupA[k][k];
    for(i=k+1;i<n;i++){
      L[i][k] = dupA[i][k]/U[k][k]; /* That's just A[k][k] again */
      U[k][i] = dupA[k][i];
    }

    for(i=k+1;i<n;i++){ /* This part compute the Schur's component */
      for(j=k+1;j<n;j++){
	dupA[i][j]=dupA[i][j] - L[i][k] * U[k][j];
      }
    }
  }

  matrix_free(dupA);
}

int32 matrix_row_exchange(matrix_t *A, int32 rowA, int32 rowB)
{
  int32 ind;
  int32 tmp_i,tmp_f;
  
  if(A->MODE==MATRIX_FLOAT){
    for(ind=0;ind<A->c;ind++){
      tmp_i=A->i[rowA][ind];
      A->i[rowA][ind]=A->i[rowB][ind];
      A->i[rowB][ind]=tmp_i;
    }
  }else if (A->MODE==MATRIX_INT){
    for(ind=0;ind<A->c;ind++){
      tmp_f=A->f[rowA][ind];
      A->f[rowA][ind]=A->f[rowB][ind];
      A->f[rowB][ind]=tmp_f;
    }
  }
    
}

int32 matrix_col_exchange(matrix_t *A, int32 colA, int32 colB)
{
  int32 ind;
  int32 tmp_i,tmp_f;
  
  if(A->MODE==MATRIX_FLOAT){
    for(ind=0;ind<A->r;ind++){
      tmp_i=A->i[ind][colA];
      A->i[ind][colA]=A->i[ind][colB];
      A->i[ind][colB]=tmp_i;
    }
  }else if (A->MODE==MATRIX_INT){
    for(ind=0;ind<A->r;ind++){
      tmp_i=A->f[ind][colA];
      A->f[ind][colA]=A->f[ind][colB];
      A->f[ind][colB]=tmp_f;
    }
  }

}


int32 matrix_lupdecompose(matrix_t *A, matrix_t **L_mat, matrix_t **U_mat, int32 *P)
{
  int32 n;
  int32 i, j, k;
  int32 min,loc;
  int32 tmp;
  matrix *dupA;
  matrix *L, *U;

  L=*L_mat;
  U=*U_mat;

  assert(A);
  assert(L);
  assert(U);
  assert(P);

  if(!matrix_is_square(A) || !matrix_is_square(L) || !matrix_is_square(U)){
    E_ERROR("Either one of matrix A, L , U or P is not square matrix\n");
    return MATRIX_OPERATION_FAIL;
  }

  if(A->r!=L->r ||L->r!=U->r || U->r ){
    E_ERROR("Dimensiont mismatch in A, L, U and P, A->r %d, L->r %d, U->r %d\n",A->r,L->r, U->r);
    return MATRIX_OPERATION_FAIL;
  }

  dupA=matrix_dup(A);

  n=A->r;

  assert(A->MODE==MATRIX_FLOAT);
  for(i=0;i<n;i++)
    P[i]=i;
  
  for(k=0;k<n;k++){
    min=0;
    for(i=k;i<n;i++){
      /* This part will find the permutation matrix */
      if(fabs(a[i][k]) > min){
	min = fabs(a[i][k]);
	loc = k;
      }

      if(min==0){
	E_ERROR("Matrix is singular \n");
	return MATRIX_OPERATION_FAIL;
      }
      
      /* Exchange Permutation */
      tmp=P[k];
      P[k]=P[loc];
      P[loc]=tmp;

      matrix_row_exchange(dupA,k,loc);
      
      for(i = k + 1 ; i < n ; i++){
	dupA[i][k] = dupA[i][k] / dupA[k][k];

	for(j= k+1 ; j < n ; j++){
	  dup[i][j] = dupA[i][j] - dupA[i][k] * dupA[k][j];
	}
      }

    }
  }

  matrix_free(dupA);

}
