/*
 * hyp.c -- Hypothesis structures.
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
 * 27-Aug-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>


#include "hyp.h"


void hyp_log (FILE *fp, glist_t hyplist, char *(*func)(void *kb, int32 id), void *kb)
{
    gnode_t *gn;
    hyp_t *h;
    int32 scr;
    
    fprintf (fp, " %5s %5s %11s %s\n", "Start", "End", "Score", "Name");
    
    for (gn = hyplist, scr = 0; gn; gn = gnode_next(gn), scr += h->scr) {
	h = (hyp_t *) gnode_ptr(gn);

	fprintf (fp, " %5d %5d %11d %s\n", h->sf, h->ef, h->scr,
		 func ? (*func)(kb, h->id) : "");
    }
    fprintf (fp, " Total Score %11d\n", scr);
    
    fflush (fp);
}


void hyp_myfree (glist_t hyplist)
{
    glist_myfree (hyplist, sizeof(hyp_t));
}
