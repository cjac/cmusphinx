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
 * vithist.h -- Viterbi search history.
 *
 *
 * HISTORY
 * 
 * 03-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added vithist_sort().
 * 
 * 25-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Modified vithist_append() interface to include lmhist.
 * 
 * 24-Feb-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added lmhist field to vithist_t.
 * 
 * 16-Jul-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_VITHIST_H_
#define _LIBMAIN_VITHIST_H_


#include <libutil/libutil.h>
#include "s3types.h"


/* A single node in the Viterbi history */
typedef struct vithist_s {
    int32 id;		/* Could be s3wid_t, s3cipid_t, or anything else; interpreted by the
			   caller or client */
    int32 frm;		/* Frame at which this history ends */
    int32 scr;		/* Total path score for for this history */
    struct vithist_s *hist;	/* Predecessor history entry for backtracking */
    struct vithist_s *lmhist;	/* Auxiliary Viterbi history field; typically used for locating
				   LM-visible history node if this node is a filler word node.
				   (Recall that filler words are transparent to the LM.) */
} vithist_t;


/*
 * Create a vithist node out of the given information and update the given Viterbi
 * history list.  Return value: the updated history list of vithist_t nodes; i.e.,
 * glist_t->data.ptr is of type (vithist_t *).  (Note that the Viterbi history is
 * traced back via vithist_s.hist info in the glist data, not the glist itself.
 */
glist_t vithist_append (glist_t hlist,	/* In/Out: History list to be updated */
			int32 id,	/* In: ID info in new history node */
			int32 frm,	/* In: Frame info in new history node */
			int32 score,	/* In: Score info in new history node */
			vithist_t *hist,/* In: Viterbi predecessor for the new node */
			vithist_t *lmhist);	/* In: LM-visible history node, if any.  If NULL,
						   the lmhist field of the newly created vithist
						   node points to itself. */

/*
 * Sort the given Viterbi nodes list in descending order of score.
 * Return value: A new sorted glist if successful; NULL if any error.  The old list is not
 * altered.
 */
glist_t vithist_sort (glist_t vithist_list);	/* In: Vithist nodes to sort by score */


/*
 * Backtrack through a Viterbi history starting from the given point, build a glist
 * of hyp_t nodes for the resulting path.
 * Return value: the glist of hyp_t nodes created.  Caller is responsible for finally
 * freeing this list (using glist_free or glist_myfree(,sizeof(hyp_t))).
 */
glist_t vithist_backtrace (vithist_t *hist,
			   int32 *senscale);

/*
 * Log the entire Viterbi history to the given file.  (Kind of a hack!  Might not be the right
 * interface for all situations.)
 * Return value: #entries logged.
 */
int32 vithist_log (FILE *fp,		/* In/Out: File to log to */
		   glist_t *vithist,	/* In: vithist[f] = glist of history nodes created @fr f */
		   int32 nfr,		/* #Frames valid in vithist[] */
		   char *(*func)(void *kb, int32 id),
		   			/* In: Function that provides a string name for the ID in
					   Viterbi history nodes. */
		   void *kb);		/* In: Auxiliary data structure, if any, needed by the
					   above func */
#endif
