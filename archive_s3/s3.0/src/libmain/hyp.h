/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * hyp.h -- Hypothesis structures.
 *
 * 
 * HISTORY
 * 
 * 27-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_HYP_H_
#define _LIBMAIN_HYP_H_


typedef struct {
    int32 id;		/* Could be s3wid_t, s3cipid_t, etc.  To be interpreted only by
			   the client. */
    int32 sf, ef;	/* Start/end frames, inclusive, for this segment */
    int32 ascr;		/* Segment acoustic score */
    int32 lscr;		/* LM score for transition to this segment (if applicable) */
    int32 scr;		/* Total segment score (ascr+lscr) */
} hyp_t;


/*
 * Log the given hypothesis to the given file.  (Kind of a hack!  Might not be the right
 * interface for all situations.)
 */
void hyp_log (FILE *fp,		/* In/Out: File to log to */
	      glist_t hyplist,	/* In: glist of hyp nodes */
	      char *(*func)(void *kb, int32 id),
	   			/* In: Function that provides a string name for the ID in
				   Viterbi history nodes. */
	      void *kb);	/* In: Auxiliary data structure, if any, needed by the
				   above func */

/*
 * Free the given glist of hyp nodes, including the hyp nodes themselves.
 */
void hyp_myfree (glist_t hyplist);


#endif
