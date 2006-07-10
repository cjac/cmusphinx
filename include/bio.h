/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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
 * bio.h -- Sphinx-3 binary file I/O functions.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.10  2006/03/03 00:42:36  egouvea
 * In bio.h, definition of REVERSE_SWAP_... depends on WORDS_BIGENDIAN,
 * since __BIG_ENDIAN__ isn't defined.
 *
 * In lm_3g_dmp.c, swap bigram and trigram values if needed.
 *
 * In lm_convert regresssion test, allow for tolerance (< 0.0002) when
 * comparing the results.
 *
 * Revision 1.9  2006/02/22 18:49:05  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: Move the swapping function from fe.h to bio.h
 *
 * Revision 1.8.4.2  2005/09/25 19:00:05  arthchan2003
 * Added all swap functions from fe.h
 *
 * Revision 1.8.4.1  2005/07/17 05:19:20  arthchan2003
 * Added SWAP_FLOAT64
 *
 * Revision 1.8  2005/06/21 20:40:46  arthchan2003
 * 1, Fixed doxygen documentation, 2, Add the $ keyword.
 *
 * Revision 1.5  2005/06/13 04:02:57  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.4  2005/05/10 21:21:52  archan
 * Three functionalities added but not tested. Code on 1) addition/deletion of LM in mode 4. 2) reading text-based LM 3) Converting txt-based LM to dmp-based LM.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 28-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _S3_BIO_H_
#define _S3_BIO_H_

#include <s3types.h>

/** \file bio.h
 * \brief Cross platform binary IO to process files in sphinx3 format. 
 * 
 * Note by ARCHAN at 20050717, the following swapper may suffer from
 * byte alignment in different machines. CMU LM Tk v2's mips.h might
 * be a better choice. 
 */

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

#define BYTE_ORDER_MAGIC	(0x11223344)

/** Macro to byteswap an int16 variable.  x = ptr to variable */
#define SWAP_INT16(x)	*(x) = ((0x00ff & (*(x))>>8) | (0xff00 & (*(x))<<8))

/** Macro to byteswap an int32 variable.  x = ptr to variable */
#define SWAP_INT32(x)	*(x) = ((0x000000ff & (*(x))>>24) | \
				(0x0000ff00 & (*(x))>>8) | \
				(0x00ff0000 & (*(x))<<8) | \
				(0xff000000 & (*(x))<<24))

/** Macro to byteswap a float32 variable.  x = ptr to variable */
#define SWAP_FLOAT32(x)	SWAP_INT32((int32 *) x)



/** HACK! using CMU LM ToolKit V1's swapper .*/
#define SWAP_FLOAT64(x) { int *low  = (int *) (x), \
                            *high = (int *) (x) + 1, temp;\
                        SWAP_INT32(low);  SWAP_INT32(high);\
                        temp = *low; *low = *high; *high = temp;}

/* ARCHAN: Old my! another set of swapping function. From the route fe.h!! */

#define SWAPW(x)        *(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
#define SWAPL(x)        *(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
                        (0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))
#define SWAPF(x)        SWAPL((int *) x)

/** "reversed senses" SWAP, ARCHAN: This is still incorporated in
    Sphinx 3 because lm3g2dmp used it.  Don't think that I am very
    happy with it. */

#if ((__BIG_ENDIAN__) || (WORDS_BIGENDIAN))
#define REVERSE_SENSE_SWAP_INT16(x)  x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define REVERSE_SENSE_SWAP_INT32(x)  x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
                         (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )
#else
#define REVERSE_SENSE_SWAP_INT16(x)
#define REVERSE_SENSE_SWAP_INT32(x)

#endif




/**
 * Read binary file format header: has the following format
 *     s3
 *     <argument-name> <argument-value>
 *     <argument-name> <argument-value>
 *     ...
 *     endhdr
 *     4-byte byte-order word used to find file byte ordering relative to host machine.
 * Lines beginning with # are ignored.
 * Memory for name and val allocated by this function; use bio_hdrarg_free to free them.
 * @return: 0 if successful, -1 otherwise.
 */
int32 bio_readhdr (FILE *fp,		/**< In: File to read */
		   char ***name,	/**< Out: array of argument name strings read */
		   char ***val,		/**< Out: corresponding value strings read */
		   int32 *swap	/**< Out: file needs byteswapping iff (*swap) */
    );
/**
 * Write a simple binary file header, containing only the version string.  Also write
 * the byte order magic word.
 * @return: 0 if successful, -1 otherwise.
 */
int32 bio_writehdr_version (FILE *fp,  /**< Output: File to write */
			    char *version /**< Input: A string of version */
    );


/**
 * Free name and value strings previously allocated and returned by bio_readhdr.
 */
void bio_hdrarg_free (char **name,	/**< In: Array previously returned by bio_readhdr */
		      char **val	/**< In: Array previously returned by bio_readhdr */
    );

/**
 * Like fread but perform byteswapping and accumulate checksum (the 2 extra arguments).
 * But unlike fread, returns -1 if required number of elements (n_el) not read; also,
 * no byteswapping or checksum accumulation is performed in that case.
 */
int32 bio_fread (void *buf,
		 int32 el_sz,
		 int32 n_el,
		 FILE *fp,              /**< In: An input file pointer */
		 int32 swap,		/**< In: Byteswap iff (swap != 0) */
		 uint32 *chksum	/**< In/Out: Accumulated checksum */
    );

/**
 * Read a 1-d array (fashioned after fread):
 *     4-byte array size (returned in n_el)
 *     memory allocated for the array and read (returned in buf)
 * Byteswapping and checksum accumulation performed as necessary.
 * Fails fatally if expected data not read.
 * Return value: #array elements allocated and read; -1 if error.
 */
int32 bio_fread_1d (void **buf,		/**< Out: contains array data; allocated by this
					   function; can be freed using ckd_free */
		    int32 el_sz,	/**< In: Array element size */
		    int32 *n_el,	/**< Out: #array elements allocated/read */
		    FILE *fp,		/**< In: File to read */
		    int32 sw,		/**< In: Byteswap iff (swap != 0) */
		    uint32 *ck	/**< In/Out: Accumulated checksum */
    );

/**
 * Read and verify checksum at the end of binary file.  Fails fatally if there is
 * a mismatch.
 */
void bio_verify_chksum (FILE *fp,	/**< In: File to read */
			int32 byteswap,	/**< In: Byteswap iff (swap != 0) */
			uint32 chksum	/**< In: Value to compare with checksum in file */
    );

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
