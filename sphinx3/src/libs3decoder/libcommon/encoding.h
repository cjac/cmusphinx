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
 * encoding.h -- Take care of text encoding issue
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

#ifndef ENCODING
#define ENCODING

#include "stdlib.h"
#include <s3types.h> 

#define IND_ISO88591  0
#define ISO88591  "iso8859-1"
#define IND_GB2312HEX 1
#define GB2312HEX "gb2312-hex"
#define IND_GB2312 2
#define GB2312    "gb2312"

#define IND_BADENCODING -1
#define BADENCODING  "BAD_ENCODING"

/**
   Get encoding index from encoding scheme string. 
   @return the index of the encoding scheme
 */

int encoding_str2ind(const char *enc /**< In: Input encoding */
		     );

/**
   Resolve whether encoding is legitimate 
   @return whether the two encoding could be resolved. 
 */
int encoding_resolve(char* inputenc,  /**< In: Input encoding */
		     char *outputenc  /**< In: Input encoding */
		     );

/**
   Convert hex to code. 
 */
void hextocode(char* src /**< In/Out: Input and output string where
			   in-place conversion took place */
	      );

int ishex(char* str);
#endif
