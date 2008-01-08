/*---------------------------------------------------

miscella.cpp

for miscellaneous purpose

by X.W.

----------------------------------------------------*/

/*
 *
 * $Log: miscella.c,v
 */


#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "miscella.h"

typedef char* CHARPTR;

int AddShareCount(void* pMem) {
  return ++*(int*)((char*)(pMem)-4);
}

int GetShareCount(void* pMem) {
  return *(int*)((char*)(pMem)-4);
}
void FreeShare(void* pMem) {
  if (--*(int*)((char*)(pMem)-4)==0) free((char*)(pMem)-4);
}
void DeleteArray(void* pMem)
{
   FreeShare(pMem);
}

void __ErrorMsg(char* msg,char* file,int line)
{
  char mess[512];
  sprintf(mess,"%s\nin File %s at line %d\n",msg,file,line);
  assert(0);
}

void* NewArray(size_t dim1,size_t dim2,size_t len)
{
  char* pMem;
  CHARPTR* p;
  char*	q;
  long a,size;
  size_t i;

  
  size=(long)len*(long)dim1*(long)dim2+(long)dim1*(long)sizeof(char*);
  if ((pMem=(char*)AllocShare(size))==NULL) return NULL;
  
  p=(CHARPTR*)pMem;
  q=(char*)(pMem+(long)dim1*(long)sizeof(void*));
  
  a=(long)len*(long)dim2;
  
  for (i=dim1;i>0;i--) {
    *(p++)=q;
    q+=a;
  }
  return pMem;
}

void* AllocShare(unsigned long size)
{
  void* pMem;
  if ((pMem=malloc(size+4))==NULL) return NULL;
  *((int*)pMem)=1;
  return (char*)pMem+4;
}
