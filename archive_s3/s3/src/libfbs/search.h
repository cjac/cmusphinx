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
 * search.h -- All exported search-related functions and data structures.
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
 * $Log$
 * Revision 1.2  2002/12/03  23:02:44  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 * 
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 07-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *  		Added onlynodes argument to dag_dump().
 * 
 * 12-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed fwd_sen_active to flag active senones instead of building a list
 * 		of them.
 *  
 * 24-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dag_search().
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added function fwd_sen_active() to obtain list of active senones in
 * 		current frame.
 * 
 * 04-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LIBFBS_SEARCH_H_
#define _LIBFBS_SEARCH_H_

typedef struct hyp_s {
    char     *word;		/* READ-ONLY item!! */
    s3wid_t   wid;
    s3frmid_t sf;
    s3frmid_t ef;
    int32     ascr;
    int32     lscr;
    int32     pscr;
    struct hyp_s *next;
} hyp_t;


/* ---------------- Forward Viterbi search related functions ---------------- */


void fwd_init ( void );

void fwd_start_utt (char *uttid);	/* In, READ-ONLY argument: utterance id */

/*
 * Called at the beginning of a frame to build a list of active senones (any senone used
 * by active HMMs in that frame.
 * Return value: #active senones in the returned list senlist[].
 */
void fwd_sen_active (int8 *senlist,	/* Out: Upon return senlist[s] TRUE iff
					   s active in current frame.
					   Caller allocates senlist[] array. */
		      int32 n_sen);	/* In: Size of senlist[] array */

/*
 * One frame of forward Viterbi beam search.
 * Return value: best path score of all the states evaluated in the frame.
 */
int32 fwd_frame (int32 *senscr);/* Step search one frame forward;
				   senscr: In: array of senone scores this frame */

hyp_t *fwd_end_utt ( void );	/* Wind up utterance and return final result */


/* ---------------- DAG related functions ---------------- */


/* DAG built from forward pass lattice */
int32 dag_build ( void );

/* Dump DAG to file for postprocessing */
int32 dag_dump (char *dir, int32 onlynodes, char *uttid);

/* Best path search through DAG from fwdvit word lattice */
hyp_t *dag_search (char *uttid);

/* Destroy DAG */
int32 dag_destroy ( void );


#endif
