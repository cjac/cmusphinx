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
 * encoding.c -- Take care of text encoding issue
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2005 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY 
 * $Log$
 * Revision 1.2  2006/02/22  18:45:02  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Added encoding.[ch].  This is
 * a simple interface to convert text from one format to another.
 * Currently, it only support iso8859-1, gb2312 and gb2312-hex.
 * 
 * Revision 1.1.2.1  2005/11/17 06:08:39  arthchan2003
 * Added a simple interface for text encoding conversion.
 *
 */

#include "encoding.h"
#include <ctype.h>


int encoding_str2ind(const char *enc)
{
  if(!strcmp(ISO88591,enc)){
    return IND_ISO88591;
  }else if (!strcmp(GB2312HEX,enc)){
    return IND_GB2312HEX;
  }else if (!strcmp(GB2312,enc)){
    return IND_GB2312;
  }else{
    return IND_BADENCODING;
  }
}

/*
  Current input/output encodings support list. 
  0: iso8859-1
  1: gb2312-hex
  2: gb2312

  -: do nothing
  n: doesn't make sense or not compatible
  x: not supported yet
  y: supported

i\o 0 1 2
  0 - n n
  1 n - y
  2 n x -

  When we have 4 encoding types: This document should be implemented as a data structure. 
 */

int encoding_resolve(char* inputenc, char *outputenc)
{
  int32 inputidx, outputidx;
  inputidx =encoding_str2ind(inputenc);
  outputidx=encoding_str2ind(outputenc);

  if(inputidx==outputidx){ /* Coding type is the same: No conflict and do nothing*/
    return 1;
  }else if(inputidx==IND_ISO88591 || outputidx==IND_ISO88591){ 
    E_ERROR("Ascii coding type cannot interconvert with others coding type at this point\n");
    return 0;
  }else if(inputidx==IND_GB2312HEX && outputidx==IND_GB2312){
    return 1;
  }else if(inputidx==IND_GB2312 && outputidx==IND_GB2312HEX){
    E_ERROR("Input coding type %s, output coding type %s, Not Supported\n", inputenc, outputenc);
    return 0;
  }else{
    E_ERROR("Unknown types. Input type %s, output type %s\n",inputenc, outputenc);
    return 0;
  }
}

static int hextoval(char c)
{
  if(isdigit(c)){
    return c - '0';
  }else if(c=='a'|| c=='A'){
    return 10;
  }else if(c=='b'|| c=='B'){
    return 11;
  }else if(c=='c'|| c=='C'){
    return 12;
  }else if(c=='d'|| c=='D'){
    return 13;
  }else if(c=='e'|| c=='E'){
    return 14;
  }else if(c=='f'|| c=='F'){
    return 15;
  }
  return -1;
}

int ishex(char* str)
{
  int i;
  for(i=0;str[i]!='\0';i++){
    if(!(isdigit(str[i]) ||
	 (str[i]=='a' || str[i]=='A') ||
	 (str[i]=='b' || str[i]=='B') ||
	 (str[i]=='c' || str[i]=='C') ||
	 (str[i]=='d' || str[i]=='D') ||
	 (str[i]=='e' || str[i]=='E') ||
	 (str[i]=='f' || str[i]=='F')
	 )){

      return 0;
    }
  }

  if(strlen(str)%2==1)  /* Check if its length is an even number */
    return 0;
  
  return 1;
}

/*
  An in-place conversion scheme, src would first be the hex code, then
  it would be convert to its value space. 
  For example string "AABB" would be turn into value sequence 110, 121
 */
void hextocode(char* src)
{
  int i;
  int length;

  assert(ishex(src));

  length=strlen(src);  
  for(i=0;src[i]!='\0';i+=2){
#if 0
    printf("%c%c\n",src[i],src[i+1]);
    printf("%d %d\n",hextoval(src[i]),hextoval(src[i+1]));
    printf("%d\n", hextoval(src[i]) * 16 + hextoval(src[i+1]));
#endif
    src[i/2]= hextoval(src[i]) * 16 + hextoval(src[i+1]);
  }
  src[(length/2)]=0;

#if 0
  printf("\n");
#endif
}

/**
   Check whether the text are written in hex code.  Implementation
   assumes non-contagious encoding scheme and it is insensitive to
   case. It also take no responsibility of conversion. 
 */



