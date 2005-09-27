/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 * ctxt_table.c -- Building a context table , split from flat_fwd.c (or fwd.c)
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 14-Jul-05    ARCHAN (archan@cs.cmu.edu) at Carnegie Mellon Unversity 
 *              First created it. 
 *
 * $Log$
 * Revision 1.1.2.2  2005/09/27  07:39:17  arthchan2003
 * Added ctxt_table_free.
 * 
 * Revision 1.1.2.1  2005/09/25 19:08:25  arthchan2003
 * Move context table from search to here.
 *
 * Revision 1.1.2.4  2005/09/07 23:32:03  arthchan2003
 * 1, Added get_lcpid in parrallel with get_rcpid. 2, Also fixed small mistakes in the macro.
 *
 * Revision 1.1.2.3  2005/07/24 01:32:54  arthchan2003
 * Flush the output of the cross word triphone in ctxt_table.c
 *
 * Revision 1.1.2.2  2005/07/17 05:42:27  arthchan2003
 * Added super-detailed comments ctxt_table.h. Also added dimension to the arrays that stores all context tables.
 *
 * Revision 1.1.2.1  2005/07/15 07:48:32  arthchan2003
 * split the hmm (whmm_t) and context building process (ctxt_table_t) from the the flat_fwd.c
 *
 *
 */

#include <ctxt_table.h>
#include <ckd_alloc.h>

static s3pid_t *tmp_xwdpid ;  /**< Temporary array used during the creation of lexical triphones lists */
static int8 *word_start_ci; 
static int8 *word_end_ci;   

void dump_xwdpidmap (xwdpid_t **x, mdef_t *mdef)
{
    s3cipid_t b, c1, c2;
    s3pid_t p;

    for (b = 0; b < mdef->n_ciphone; b++) {
	if (! x[b])
	    continue;
	
	for (c1 = 0; c1 < mdef->n_ciphone; c1++) {
	    if (! x[b][c1].cimap)
		continue;
	    
	    printf ("n_pid(%s, %s) = %d\n",
		    mdef_ciphone_str(mdef, b), mdef_ciphone_str(mdef, c1),
		    x[b][c1].n_pid);

	    for (c2 = 0; c2 < mdef->n_ciphone; c2++) {
		p = x[b][c1].pid[x[b][c1].cimap[c2]];
		printf ("  %10s %5d\n", mdef_ciphone_str(mdef, c2), p);
	    }
	}
    }
    fflush(stdout);
}


/**
 * Utility function for building cross-word pid maps.  Compresses cross-word pid list
 * to unique ones.
 */
int32 xwdpid_compress (s3pid_t p, s3pid_t *pid, s3cipid_t *map, s3cipid_t ctx,
		       int32 n, 
		       mdef_t* mdef /**<The model definition */
		       )
{
    s3senid_t *senmap, *prevsenmap;
    int32 s;
    s3cipid_t i;
    int32 n_state;
    n_state=mdef->n_emit_state +1 ;

    senmap = mdef->phone[p].state;
    
    for (i = 0; i < n; i++) {
	if (mdef->phone[p].tmat != mdef->phone[pid[i]].tmat)
	    continue;

	prevsenmap = mdef->phone[pid[i]].state;
	for (s = 0; (s < n_state-1) && (senmap[s] == prevsenmap[s]); s++);
	

	if (s == n_state-1) {
	    /* This state sequence same as a previous ones; just map to it */
	    map[ctx] = i;
	    return n;
	}
    }

    /* This state sequence different from all previous ones; allocate new entry */
    map[ctx] = n;
    pid[n] = p;
    
    return (n+1);
}



/**
 * Given base b, and right context rc, build left context cross-word triphones map
 * for all left context ciphones.  Compress map to unique list.
 */
void build_lcpid (ctxt_table_t *ct, s3cipid_t b, s3cipid_t rc, mdef_t *mdef)
{
    s3cipid_t lc, *map;
    s3pid_t p;
    int32 n;
    
    map = (s3cipid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3cipid_t));
    
    n = 0;
    for (lc = 0; lc < mdef->n_ciphone; lc++) {
	p = mdef_phone_id_nearest (mdef, b, lc, rc, WORD_POSN_BEGIN);
	if ((! mdef->ciphone[b].filler) && word_end_ci[lc] &&
	    mdef_is_ciphone(mdef, p))
	    ct->n_backoff_ci++;
	
	n = xwdpid_compress (p, tmp_xwdpid, map, lc, n,mdef);
    }

    /* Copy/Move to lcpid */
    ct->lcpid[b][rc].cimap = map;
    ct->lcpid[b][rc].n_pid = n;
    ct->lcpid[b][rc].pid = (s3pid_t *) ckd_calloc (n, sizeof(s3pid_t));
    memcpy (ct->lcpid[b][rc].pid, tmp_xwdpid, n*sizeof(s3pid_t));
}


/**
 * Given base b, and left context lc, build right context cross-word triphones map
 * for all right context ciphones.  Compress map to unique list.
 */
void build_rcpid (ctxt_table_t *ct, s3cipid_t b, s3cipid_t lc, mdef_t *mdef)
{
    s3cipid_t rc, *map;
    s3pid_t p;
    int32 n;
    
    map = (s3cipid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3cipid_t));
    
    n = 0;
    for (rc = 0; rc < mdef->n_ciphone; rc++) {
	p = mdef_phone_id_nearest (mdef, b, lc, rc, WORD_POSN_END);
	if ((! mdef->ciphone[b].filler) && word_start_ci[rc] &&
	    mdef_is_ciphone(mdef, p))
	    ct->n_backoff_ci++;

	n = xwdpid_compress (p, tmp_xwdpid, map, rc, n,mdef);
    }

    /* Copy/Move to rcpid */
    ct->rcpid[b][lc].cimap = map;
    ct->rcpid[b][lc].n_pid = n;
    ct->rcpid[b][lc].pid = (s3pid_t *) ckd_calloc (n, sizeof(s3pid_t));
    memcpy (ct->rcpid[b][lc].pid, tmp_xwdpid, n*sizeof(s3pid_t));
}

/**
 * Given base b for a single-phone word, build context cross-word triphones map
 * for all left and right context ciphones.
 */
void build_lrcpid (ctxt_table_t *ct, s3cipid_t b, mdef_t *mdef)
{
    s3cipid_t rc, lc;
    
    for (lc = 0; lc < mdef->n_ciphone; lc++) {
	ct->lrcpid[b][lc].pid = (s3pid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3pid_t));
	ct->lrcpid[b][lc].cimap = (s3cipid_t *) ckd_calloc (mdef->n_ciphone,sizeof(s3cipid_t));
							
	
	for (rc = 0; rc < mdef->n_ciphone; rc++) {
	    ct->lrcpid[b][lc].cimap[rc] = rc;
	    ct->lrcpid[b][lc].pid[rc] = mdef_phone_id_nearest (mdef, b, lc, rc,
							   WORD_POSN_SINGLE);
	    if ((! mdef->ciphone[b].filler) &&
		word_start_ci[rc] && word_end_ci[lc] &&
		mdef_is_ciphone(mdef, ct->lrcpid[b][lc].pid[rc]))
		ct->n_backoff_ci++;
	}
	ct->lrcpid[b][lc].n_pid = mdef->n_ciphone;
    }
}




/**
 * Build within-word triphones sequence for each word.  The extreme ends are not needed
 * since cross-word modelling is used for those.  (See lcpid, rcpid, lrcpid.)
 */

void build_wwpid (ctxt_table_t* ct, dict_t *dict, mdef_t *mdef )
{
    s3wid_t w;
    int32 pronlen, l;
    s3cipid_t b, lc, rc;
    
    E_INFO ("Building within-word triphones\n");
    ct->n_backoff_ci=0;

    ct->wwpid = (s3pid_t **) ckd_calloc (dict->n_word, sizeof(s3pid_t *));
    for (w = 0; w < dict->n_word; w++) {
	pronlen = dict->word[w].pronlen;
	if (pronlen >= 3)
	    ct->wwpid[w] = (s3pid_t *) ckd_calloc (pronlen-1, sizeof(s3pid_t));
	else
	    continue;
	
	lc = dict->word[w].ciphone[0];
	b = dict->word[w].ciphone[1];
	for (l = 1; l < pronlen-1; l++) {
	    rc = dict->word[w].ciphone[l+1];
	    ct->wwpid[w][l] = mdef_phone_id_nearest (mdef, b, lc, rc, WORD_POSN_INTERNAL);
	    if ((! mdef->ciphone[b].filler) && mdef_is_ciphone(mdef, ct->wwpid[w][l]))
		ct->n_backoff_ci++;
	    
	    lc = b;
	    b = rc;
	}
#if 0
	printf ("%-25s ", dict->word[w].word);
	for (l = 1; l < pronlen-1; l++)
	    printf (" %5d", wwpid[w][l]);
	printf ("\n");
#endif
    }

    E_INFO("%d within-word triphone instances mapped to CI-phones\n", ct->n_backoff_ci);

}

/**
 * Build cross-word triphones map for the entire dictionary.
 */
void build_xwdpid_map (ctxt_table_t* ct, dict_t *dict, mdef_t *mdef)
{
    s3wid_t w;
    int32 pronlen;
    s3cipid_t b, lc, rc;

    ct->n_backoff_ci = 0;

    /* Build cross-word triphone models */
    E_INFO ("Building cross-word triphones\n");
    
    word_start_ci = (int8 *) ckd_calloc (mdef->n_ciphone, sizeof(int8));
    word_end_ci = (int8 *) ckd_calloc (mdef->n_ciphone, sizeof(int8));

    /* Mark word beginning and ending ciphones that occur in given dictionary */
    for (w = 0; w < dict->n_word; w++) {
	word_start_ci[dict->word[w].ciphone[0]] = 1;
	word_end_ci[dict->word[w].ciphone[dict->word[w].pronlen-1]] = 1;
    }
    ct->lcpid = (xwdpid_t **) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t *));
    ct->rcpid = (xwdpid_t **) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t *));
    ct->lrcpid = (xwdpid_t **) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t *));

    for (w = 0; w < dict->n_word; w++) {
	pronlen = dict->word[w].pronlen;
	if (pronlen > 1) {
	    /* Multi-phone word; build rcmap and lcmap if not already present */

	    b = dict->word[w].ciphone[pronlen-1];
	    lc = dict->word[w].ciphone[pronlen-2];
	    if (! ct->rcpid[b])
		ct->rcpid[b] = (xwdpid_t *) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t));
	    if (! ct->rcpid[b][lc].cimap)
		build_rcpid (ct, b, lc, mdef);
	    b = dict->word[w].ciphone[0];
	    rc = dict->word[w].ciphone[1];
	    if (! ct->lcpid[b])
		ct->lcpid[b] = (xwdpid_t *) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t));
	    if (! ct->lcpid[b][rc].cimap)
		build_lcpid (ct, b, rc, mdef);

	} else {
	    /* Single-phone word; build lrcmap if not already present */
	    b = dict->word[w].ciphone[0];
	    if (! ct->lrcpid[b]) {
		ct->lrcpid[b] = (xwdpid_t *) ckd_calloc (mdef->n_ciphone, sizeof(xwdpid_t));
		build_lrcpid (ct, b, mdef);
	    }
	}
    }

    ckd_free (word_start_ci);
    ckd_free (word_end_ci);
    E_INFO("%d cross-word triphones mapped to CI-phones\n", ct->n_backoff_ci);

#if 0
    E_INFO ("LCXWDPID\n");
    dump_xwdpidmap (ct->lcpid,mdef);
    
    E_INFO ("RCXWDPID\n");
    dump_xwdpidmap (ct->rcpid,mdef);

    E_INFO ("LRCXWDPID\n");
    dump_xwdpidmap (ct->lrcpid,mdef);
#endif
}

ctxt_table_t* ctxt_table_init( dict_t *dict,mdef_t *mdef)
{
  ctxt_table_t *ct;
  ct= (ctxt_table_t*) ckd_calloc(1,sizeof(ctxt_table_t));

  tmp_xwdpid = (s3pid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3pid_t));

  build_wwpid(ct, dict, mdef);
  build_xwdpid_map (ct, dict, mdef)  ;

  ckd_free (tmp_xwdpid);
  return ct;
}

void ctxt_table_free(ctxt_table_t *ct)
{
  if(ct->lcpid)
    ckd_free(ct->lcpid);
  
  if(ct->rcpid)
    ckd_free(ct->rcpid);

  if(ct->lrcpid)
    ckd_free(ct->lrcpid);

  if(ct->wwpid)
    ckd_free(ct->wwpid);

}

s3cipid_t *get_rc_cimap (ctxt_table_t *ct, s3wid_t w,dict_t *dict)
{
    int32 pronlen;
    s3cipid_t b, lc;
    
    pronlen = dict->word[w].pronlen;
    b = dict->word[w].ciphone[pronlen-1];
    if (pronlen == 1) {
	/* No known left context.  But all cimaps (for any l) are identical; pick one */
	return (ct->lrcpid[b][0].cimap);
    } else {
	lc = dict->word[w].ciphone[pronlen-2];
	return (ct->rcpid[b][lc].cimap);
    }
}

s3cipid_t *get_lc_cimap (ctxt_table_t *ct, s3wid_t w,dict_t *dict)
{
    int32 pronlen;
    s3cipid_t b, rc;
    
    pronlen = dict->word[w].pronlen;
    b = dict->word[w].ciphone[0];
    if (pronlen == 1) {
	/* No known right context.  But all cimaps (for any l) are identical; pick one */
	return (ct->lrcpid[b][0].cimap);
    } else {
	rc = dict->word[w].ciphone[1];
	return (ct->lcpid[b][rc].cimap);
    }
}


void get_rcpid (ctxt_table_t *ct, s3wid_t w, s3pid_t **pid, int32 *npid,dict_t *dict)
{
    int32 pronlen;
    s3cipid_t b, lc;
    
    pronlen = dict->word[w].pronlen;
    assert (pronlen > 1);
    
    b = dict->word[w].ciphone[pronlen-1];
    lc = dict->word[w].ciphone[pronlen-2];
    
    *pid = ct->rcpid[b][lc].pid;
    *npid = ct->rcpid[b][lc].n_pid;
}

void get_lcpid (ctxt_table_t *ct, s3wid_t w, s3pid_t **pid, int32 *npid,dict_t *dict)
{
    int32 pronlen;
    s3cipid_t b, rc;
    
    pronlen = dict->word[w].pronlen;
    assert (pronlen > 1);
    
    b = dict->word[w].ciphone[0];
    rc = dict->word[w].ciphone[1];
    
    *pid = ct->lcpid[b][rc].pid;
    *npid = ct->lcpid[b][rc].n_pid;
}


int32 get_rc_npid (ctxt_table_t *ct, s3wid_t w,dict_t *dict)
{
    int32 pronlen;
    s3cipid_t b, lc;
    
    pronlen = dict->word[w].pronlen;
    b = dict->word[w].ciphone[pronlen-1];
    assert(ct);
    assert(ct->lrcpid);
    if (pronlen == 1) {
	/* No known left context.  But all cimaps (for any l) are identical; pick one */
	return (ct->lrcpid[b][0].n_pid);
    } else {
	lc = dict->word[w].ciphone[pronlen-2];
	return (ct->rcpid[b][lc].n_pid);
    }
}



