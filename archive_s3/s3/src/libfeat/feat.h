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
 * feat.h -- Cepstral features computation.
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
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added sf, ef parameters to s2mfc_read().
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added feat_cepsize().
 * 
 * 09-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Moved constant declarations to feat.c.
 * 
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _LIBFEAT_FEAT_H_
#define _LIBFEAT_FEAT_H_


#include <libutil/prim_type.h>


/*
 * Initialize feature module to use the selected type of feature stream.  One-time only
 * initialization at the beginning of the program.
 */
void feat_init (char *type);		/* In: type of feature stream */

/*
 * Return input mfc vector size.
 */
int32 feat_cepsize ( int32 veclen);

/*
 * Return feature vector size for each feature stream.
 * Return value: #feature streams.
 */
int32 feat_featsize (int32 **size);	/* Out: Upon return, *size is a ptr to the array,
					   allocated by this function, of feature stream
					   sizes.  It must NOT be modified or freed. */

/*
 * Each feature frame is computed from a window of 2W+1 input mfc vectors:
 *   mfc[-W]..mfc[W] centered around mfc[0].
 * Return value: W.
 */
int32 feat_window_size ( void );


/*
 * Compute feature vectors from a window of mfc frames.
 */
void feat_cep2feat (float32 **mfc,	/* In: Array of 2W+1 mfc vectors, CENTERED on
					   FEATURE WINDOW; ie, feature vectors computed
					   from mfc vectors mfc[-W]..mfc[W]. */
		    float32 **feat);	/* Out: feat[f] = output vector for feature
					   stream f.  Caller must allocate this space */

/*
 * Read Sphinx-II format mfc file.
 * Return value: #frames read.
 */
int32 s2mfc_read (char *file,		/* In: mfc file to be read */
		  int32 sf,		/* In: Start/end frames, inclusive, of segment read */
		  int32 ef,
		  float32 ***mfc, 	/* Out: On return, *mfc is a ptr to the 2-D array
					   of mfc vectors read.  The 2-D array is allocated
					   by this function (and re-used upon successive
					   calls).  Caller MUST NOT free it. */
		  int32  veclen);       /* Length of base mfc feature vector
					   (before deltas and double deltas) */

#endif
