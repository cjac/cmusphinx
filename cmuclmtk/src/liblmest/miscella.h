/*--------------------------------------------
		head file of miscella.cpp
		
			written by X.W.


---------------------------------------------*/


#ifndef __MISCELLA_H__
#define __MISCELLA_H__

#include <stdio.h>
#include <stdlib.h>

#ifndef BOOL
#define BOOL	int
#define TRUE	1
#define FALSE	0
#endif

void __ErrorMsg(char* msg,char* file,int line);
#define Error(msg)	__ErrorMsg(msg,__FILE__,__LINE__)

void* AllocShare(unsigned long size);
int AddShareCount(void* pMem);
int GetShareCount(void* pMem);
void FreeShare(void* pMem);
void DeleteArray(void* pMem);

void* NewArray(size_t dim1,size_t dim2,size_t len);

#endif
