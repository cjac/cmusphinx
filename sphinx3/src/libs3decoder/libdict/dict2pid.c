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
 * dict2pid.c -- Triphones for dictionary
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.6  2005/06/21  21:03:49  arthchan2003
 * 1, Introduced a reporting routine. 2, Fixed doyxgen documentation, 3, Added  keyword.
 * 
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:46  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 14-Sep-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added dict2pid_comsseq2sen_active().
 * 
 * 04-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "dict2pid.h"
#include "logs3.h"


  /** \file dict2pid.c
   * \brief Implementation of dict2pid
   */

/**
 * Build a glist of triphone senone-sequence IDs (ssids) derivable from [b][r] at the word
 * begin position.  If no triphone found in mdef, include the ssid for basephone b.
 * Return the generated glist.
 */
static glist_t ldiph_comsseq (mdef_t *mdef, int32 b, int32 r)
{
    int32 l, p, ssid;
    glist_t g;
    
    g = NULL;
    for (l = 0; l < mdef_n_ciphone(mdef); l++) {
	p = mdef_phone_id (mdef, (s3cipid_t)b, (s3cipid_t)l, (s3cipid_t)r, WORD_POSN_BEGIN);
	
	if (IS_S3PID(p)) {
	    ssid = mdef_pid2ssid(mdef, p);
	    if (! glist_chkdup_int32 (g, ssid))
		g = glist_add_int32 (g, ssid);
	}
    }
    if (! g)
	g = glist_add_int32 (g, mdef_pid2ssid(mdef, b));
    
    return g;
}


/**
 * Build a glist of triphone senone-sequence IDs (ssids) derivable from [r][b] at the word
 * end position.  If no triphone found in mdef, include the ssid for basephone b.
 * Return the generated glist.
 */
static glist_t rdiph_comsseq (mdef_t *mdef, int32 b, int32 l)
{
    int32 r, p, ssid;
    glist_t g;
    
    g = NULL;
    for (r = 0; r < mdef_n_ciphone(mdef); r++) {
	p = mdef_phone_id (mdef, (s3cipid_t)b, (s3cipid_t)l, (s3cipid_t)r, WORD_POSN_END);
	
	if (IS_S3PID(p)) {
	    ssid = mdef_pid2ssid(mdef, p);
	    if (! glist_chkdup_int32 (g, ssid))
		g = glist_add_int32 (g, ssid);
	}
    }
    if (! g)
	g = glist_add_int32 (g, mdef_pid2ssid(mdef, b));
    
    return g;
}


/**
 * Build a glist of triphone senone-sequence IDs (ssids) derivable from [b] as a single
 * phone word.  If no triphone found in mdef, include the ssid for basephone b.
 * Return the generated glist.
 */
static glist_t single_comsseq (mdef_t *mdef, int32 b)
{
    int32 l, r, p, ssid;
    glist_t g;
    
    g = NULL;
    for (l = 0; l < mdef_n_ciphone(mdef); l++) {
	for (r = 0; r < mdef_n_ciphone(mdef); r++) {
	    p = mdef_phone_id (mdef, (s3cipid_t)b, (s3cipid_t)l, (s3cipid_t)r, WORD_POSN_SINGLE);
	    
	    if (IS_S3PID(p)) {
		ssid = mdef_pid2ssid(mdef, p);
		if (! glist_chkdup_int32 (g, ssid))
		    g = glist_add_int32 (g, ssid);
	    }
	}
    }
    if (! g)
	g = glist_add_int32 (g, mdef_pid2ssid(mdef, b));
    
    return g;
}


/**
 * Build a glist of triphone senone-sequence IDs (ssids) derivable from [b] as a single
 * phone word, with a given left context l.  If no triphone found in mdef, include the ssid
 * for basephone b.  Return the generated glist.
 */
static glist_t single_lc_comsseq (mdef_t *mdef, int32 b, int32 l)
{
    int32 r, p, ssid;
    glist_t g;
    
    g = NULL;
    for (r = 0; r < mdef_n_ciphone(mdef); r++) {
	p = mdef_phone_id (mdef, (s3cipid_t)b, (s3cipid_t)l, (s3cipid_t)r, WORD_POSN_SINGLE);
	
	if (IS_S3PID(p)) {
	    ssid = mdef_pid2ssid(mdef, p);
	    if (! glist_chkdup_int32 (g, ssid))
		g = glist_add_int32 (g, ssid);
	}
    }
    if (! g)
	g = glist_add_int32 (g, mdef_pid2ssid(mdef, b));
    
    return g;
}


/**
 * Convert the glist of ssids to a composite sseq id.  Return the composite ID.
 */
static s3ssid_t ssidlist2comsseq (glist_t g, mdef_t *mdef, dict2pid_t *dict2pid,
				  hash_table_t *hs,	/* For composite states */
				  hash_table_t *hp)	/* For composite senone seq */
{
    int32 i, j, n, s, ssid;
    s3senid_t **sen;
    s3senid_t *comsenid;
    gnode_t *gn;
    
    n = glist_count (g);
    if (n <= 0)
	E_FATAL("Panic: length(ssidlist)= %d\n", n);
    
    /* Space for list of senones for each state, derived from the given glist */
    sen = (s3senid_t **) ckd_calloc (mdef_n_emit_state (mdef), sizeof(s3senid_t *));
    for (i = 0; i < mdef_n_emit_state (mdef); i++) {
	sen[i] = (s3senid_t *) ckd_calloc (n+1, sizeof(s3senid_t));
	sen[i][0] = BAD_S3SENID;	/* Sentinel */
    }
    /* Space for composite senone ID for each state position */
    comsenid = (s3senid_t *) ckd_calloc (mdef_n_emit_state (mdef), sizeof(s3senid_t));
    
    for (gn = g; gn; gn = gnode_next(gn)) {
	ssid = gnode_int32 (gn);
	
	/* Expand ssid into individual states (senones); insert in sen[][] if not present */
	for (i = 0; i < mdef_n_emit_state (mdef); i++) {
	    s = mdef->sseq[ssid][i];
	    
	    for (j = 0; (IS_S3SENID(sen[i][j])) && (sen[i][j] != s); j++);
	    if (NOT_S3SENID(sen[i][j])) {
		sen[i][j] = s;
		sen[i][j+1] = BAD_S3SENID;
	    }
	}
    }
    
    /* Convert senones list for each state position into composite state */
    for (i = 0; i < mdef_n_emit_state (mdef); i++) {
	for (j = 0; IS_S3SENID(sen[i][j]); j++);
	assert (j > 0);
	
	j = hash_enter_bkey (hs, (char *)(sen[i]), j*sizeof(s3senid_t), dict2pid->n_comstate);
	if (j == dict2pid->n_comstate)
	    dict2pid->n_comstate++;	/* New composite state */
	else
	    ckd_free ((void *) sen[i]);
	
	comsenid[i] = j;
    }
    ckd_free (sen);
    
    /* Convert sequence of composite senids to composite sseq ID */
    j = hash_enter_bkey (hp, (char *)comsenid, mdef->n_emit_state * sizeof(s3senid_t),
			 dict2pid->n_comsseq);
    if (j == dict2pid->n_comsseq) {
	dict2pid->n_comsseq++;
	if (dict2pid->n_comsseq >= MAX_S3SENID)
	    E_FATAL("#Composite sseq limit(%d) reached; increase MAX_S3SENID\n",
		    dict2pid->n_comsseq);
    } else
	ckd_free ((void *) comsenid);
    
    return ((s3ssid_t)j);
}


/* RAH 4.16.01 This code has several leaks that must be fixed */
dict2pid_t *dict2pid_build (mdef_t *mdef, dict_t *dict)
{
    dict2pid_t *dict2pid;
    s3ssid_t *internal, **ldiph, **rdiph, *single;
    int32 pronlen;
    hash_table_t *hs, *hp;
    glist_t g;
    gnode_t *gn;
    s3senid_t *sen;
    hash_entry_t *he;
    int32 *cslen;
    int32 i, j, b, l, r, w, n, p;
    
    E_INFO("Building PID tables for dictionary\n");

    dict2pid = (dict2pid_t *) ckd_calloc (1, sizeof(dict2pid_t));
    dict2pid->internal = (s3ssid_t **) ckd_calloc (dict_size(dict), sizeof(s3ssid_t *));
    dict2pid->ldiph_lc = (s3ssid_t ***) ckd_calloc_3d (mdef->n_ciphone,
						       mdef->n_ciphone,
						       mdef->n_ciphone,
						       sizeof(s3ssid_t));
    dict2pid->single_lc = (s3ssid_t **) ckd_calloc_2d (mdef->n_ciphone,
						       mdef->n_ciphone,
						       sizeof(s3ssid_t));
    dict2pid->n_comstate = 0;
    dict2pid->n_comsseq = 0;
    
    hs = hash_new (mdef->n_ciphone * mdef->n_ciphone * mdef->n_emit_state, HASH_CASE_YES);
    hp = hash_new (mdef->n_ciphone * mdef->n_ciphone, HASH_CASE_YES);
    
    for (w = 0, n = 0; w < dict_size(dict); w++) {
	pronlen = dict_pronlen(dict, w);
	if (pronlen < 0)
	    E_FATAL("Pronunciation-length(%s)= %d\n", dict_wordstr(dict, w), pronlen);
	n += pronlen;
    }

    internal = (s3ssid_t *) ckd_calloc (n, sizeof(s3ssid_t));
    
    /* Temporary */
    ldiph = (s3ssid_t **) ckd_calloc_2d (mdef->n_ciphone, mdef->n_ciphone, sizeof(s3ssid_t));
    rdiph = (s3ssid_t **) ckd_calloc_2d (mdef->n_ciphone, mdef->n_ciphone, sizeof(s3ssid_t));
    single = (s3ssid_t *) ckd_calloc (mdef->n_ciphone, sizeof(s3ssid_t));
    for (b = 0; b < mdef->n_ciphone; b++) {
	for (l = 0; l < mdef->n_ciphone; l++) {
	    for (r = 0; r < mdef->n_ciphone; r++)
		dict2pid->ldiph_lc[b][r][l] = BAD_S3SSID;
	    
	    dict2pid->single_lc[b][l] = BAD_S3SSID;
	    
	    ldiph[b][l] = BAD_S3SSID;
	    rdiph[b][l] = BAD_S3SSID;
	}
	single[b] = BAD_S3SSID;
    }
    
    for (w = 0; w < dict_size(dict); w++) {
	dict2pid->internal[w] = internal;
	pronlen = dict_pronlen(dict,w);
	
	if (pronlen >= 2) {
	    b = dict_pron(dict, w, 0);
	    r = dict_pron(dict, w, 1);
	    if (NOT_S3SSID(ldiph[b][r])) {
		g = ldiph_comsseq(mdef, b, r);
		ldiph[b][r] = ssidlist2comsseq (g, mdef, dict2pid, hs, hp);
		glist_free (g);
		
		for (l = 0; l < mdef_n_ciphone(mdef); l++) {
		    p = mdef_phone_id_nearest (mdef, (s3cipid_t)b, (s3cipid_t)l, (s3cipid_t)r, WORD_POSN_BEGIN);
		    dict2pid->ldiph_lc[b][r][l] = mdef_pid2ssid(mdef, p);
		}
	    }
	    internal[0] = ldiph[b][r];
	    
	    for (i = 1; i < pronlen-1; i++) {
		l = b;
		b = r;
		r = dict_pron(dict, w, i+1);
		
		p = mdef_phone_id_nearest(mdef, (s3cipid_t)b, (s3cipid_t)l, (s3cipid_t)r, WORD_POSN_INTERNAL);
		internal[i] = mdef_pid2ssid(mdef, p);
	    }
	    
	    l = b;
	    b = r;
	    if (NOT_S3SSID(rdiph[b][l])) {
		g = rdiph_comsseq(mdef, b, l);
		rdiph[b][l] = ssidlist2comsseq (g, mdef, dict2pid, hs, hp);
		glist_free (g);
	    }
	    internal[pronlen-1] = rdiph[b][l];
	} else if (pronlen == 1) {
	    b = dict_pron(dict, w, 0);
	    if (NOT_S3SSID(single[b])) {
		g = single_comsseq(mdef, b);
		single[b] = ssidlist2comsseq (g, mdef, dict2pid, hs, hp);
		glist_free (g);
		
		for (l = 0; l < mdef_n_ciphone(mdef); l++) {
		    g = single_lc_comsseq(mdef, b, l);
		    dict2pid->single_lc[b][l] = ssidlist2comsseq (g, mdef, dict2pid, hs, hp);
		    glist_free (g);
		}
	    }
	    internal[0] = single[b];
	}
	
	internal += pronlen;
    }
    
    ckd_free_2d ((void **) ldiph);
    ckd_free_2d ((void **) rdiph);
    ckd_free ((void *) single);
    
    /* Allocate space for composite state table */
    cslen = (int32 *) ckd_calloc (dict2pid->n_comstate, sizeof(int32));
    g = hash_tolist(hs, &n);
    assert (n == dict2pid->n_comstate);
    n = 0;
    for (gn = g; gn; gn = gnode_next(gn)) {
	he = (hash_entry_t *) gnode_ptr (gn);
	sen = (s3senid_t *) hash_entry_key(he);
	for (i = 0; IS_S3SENID(sen[i]); i++);
	
	cslen[hash_entry_val(he)] = i+1;	/* +1 for terminating sentinel */
	
	n += (i+1);
    }
    dict2pid->comstate = (s3senid_t **) ckd_calloc (dict2pid->n_comstate, sizeof(s3senid_t *));
    sen = (s3senid_t *) ckd_calloc (n, sizeof(s3senid_t));
    for (i = 0; i < dict2pid->n_comstate; i++) {
	dict2pid->comstate[i] = sen;
	sen += cslen[i];
    }
    
    /* Build composite state table from hash table hs */
    for (gn = g; gn; gn = gnode_next(gn)) {
	he = (hash_entry_t *) gnode_ptr (gn);
	sen = (s3senid_t *) hash_entry_key(he);
	i = hash_entry_val(he);
	
	for (j = 0; j < cslen[i]; j++)
	    dict2pid->comstate[i][j] = sen[j];
	assert (sen[j-1] == BAD_S3SENID);

	ckd_free ((void *)sen);
    }
    ckd_free (cslen);
    glist_free (g);
    hash_free (hs);
    
    /* Allocate space for composite sseq table */
    dict2pid->comsseq = (s3senid_t **) ckd_calloc (dict2pid->n_comsseq, sizeof(s3senid_t *));
    g = hash_tolist (hp, &n);
    assert (n == dict2pid->n_comsseq);
    
    /* Build composite sseq table */
    for (gn = g; gn; gn = gnode_next(gn)) {
	he = (hash_entry_t *) gnode_ptr (gn);
	i = hash_entry_val(he);
	dict2pid->comsseq[i] = (s3senid_t *) hash_entry_key(he);
    }
    glist_free (g);
    hash_free (hp);
    
    /* Weight for each composite state */
    dict2pid->comwt = (int32 *) ckd_calloc (dict2pid->n_comstate, sizeof(int32));
    for (i = 0; i < dict2pid->n_comstate; i++) {
	sen = dict2pid->comstate[i];
	
	for (j = 0; IS_S3SENID(sen[j]); j++);
#if 0
	/* if comstate i has N states, its weight= (1/N^2) (Major Hack!!) */
	dict2pid->comwt[i] = - (logs3 ((float64)j) << 1);
#else
	/* if comstate i has N states, its weight= 1/N */
	dict2pid->comwt[i] = - logs3 ((float64)j);
#endif
    }
    
    
    return dict2pid;
}

void dict2pid_report(dict2pid_t* d2p)
{
  E_INFO_NOFN("Initialization of dict2pid_t, report:\n");
  E_INFO_NOFN("%d composite states; %d composite sseq\n",
	      d2p->n_comstate, d2p->n_comsseq);
  E_INFO_NOFN("\n");


}

void dict2pid_comsenscr (dict2pid_t *d2p, int32 *senscr, int32 *comsenscr)
{
    int32 i, j;
    int32 best;
    s3senid_t *comstate, k;
    
    for (i = 0; i < d2p->n_comstate; i++) {
	comstate = d2p->comstate[i];
	
	best = senscr[comstate[0]];
	for (j = 1;; j++) {
	    k = comstate[j];
	    if (NOT_S3SENID(k))
		break;
	    if (best < senscr[k])
		best = senscr[k];
	}
	
	comsenscr[i] = best + d2p->comwt[i];
    }
}


void dict2pid_comsseq2sen_active (dict2pid_t *d2p, mdef_t *mdef, int32 *comssid, int32 *sen)
{
    int32 ss, cs, i, j;
    s3senid_t *csp, *sp;	/* Composite state pointer */
    
    for (ss = 0; ss < d2p->n_comsseq; ss++) {
	if (comssid[ss]) {
	    csp = d2p->comsseq[ss];

	    for (i = 0; i < mdef_n_emit_state(mdef); i++) {
		cs = csp[i];
		sp = d2p->comstate[cs];
		
		for (j = 0; IS_S3SENID(sp[j]); j++)
		    sen[sp[j]] = 1;
	    }
	}
    }
}


void dict2pid_dump (FILE *fp, dict2pid_t *d2p, mdef_t *mdef, dict_t *dict)
{
    int32 w, p, pronlen;
    int32 i, j, b, l, r;
    
    fprintf (fp, "# INTERNAL (wd comssid ssid ssid ... ssid comssid)\n");
    for (w = 0; w < dict_size(dict); w++) {
	fprintf (fp, "%30s ", dict_wordstr(dict, w));
	
	pronlen = dict_pronlen(dict, w);
	for (p = 0; p < pronlen; p++)
	    fprintf (fp, " %5d", d2p->internal[w][p]);
	fprintf (fp, "\n");
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "# LDIPH_LC (b r l ssid)\n");
    for (b = 0; b < mdef_n_ciphone(mdef); b++) {
	for (r = 0; r < mdef_n_ciphone(mdef); r++) {
	    for (l = 0; l < mdef_n_ciphone(mdef); l++) {
		if (IS_S3SSID(d2p->ldiph_lc[b][r][l]))
		    fprintf (fp, "%6s %6s %6s %5d\n",
			     mdef_ciphone_str (mdef, (s3cipid_t)b),
			     mdef_ciphone_str (mdef, (s3cipid_t)r),
			     mdef_ciphone_str (mdef, (s3cipid_t)l),
			     d2p->ldiph_lc[b][r][l]); /* RAH, ldiph_lc is returning an int32, %d expects an int16 */
	    }
	}
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "# SINGLE_LC (b l comssid)\n");
    for (b = 0; b < mdef_n_ciphone(mdef); b++) {
	for (l = 0; l < mdef_n_ciphone(mdef); l++) {
	    if (IS_S3SSID(d2p->single_lc[b][l]))
		fprintf (fp, "%6s %6s %5d\n",
			 mdef_ciphone_str (mdef, (s3cipid_t)b),
			 mdef_ciphone_str (mdef, (s3cipid_t)l),
			 d2p->single_lc[b][l]);	/* RAH, single_lc is returning an int32, %d expects an int16 */
	}
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "# SSEQ %d (senid senid ...)\n", mdef->n_sseq);
    for (i = 0; i < mdef->n_sseq; i++) {
	fprintf (fp, "%5d ", i);
	for (j = 0; j < mdef_n_emit_state(mdef); j++)
	    fprintf (fp, " %5d", mdef->sseq[i][j]);
	fprintf (fp, "\n");
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "# COMSSEQ %d (comstate comstate ...)\n", d2p->n_comsseq);
    for (i = 0; i < d2p->n_comsseq; i++) {
	fprintf (fp, "%5d ", i);
	for (j = 0; j < mdef_n_emit_state(mdef); j++)
	    fprintf (fp, " %5d", d2p->comsseq[i][j]);
	fprintf (fp, "\n");
    }
    fprintf (fp, "#\n");
    
    fprintf (fp, "# COMSTATE %d (senid senid ...)\n", d2p->n_comstate);
    for (i = 0; i < d2p->n_comstate; i++) {
	fprintf (fp, "%5d ", i);
	for (j = 0; IS_S3SENID(d2p->comstate[i][j]); j++)
	    fprintf (fp, " %5d", d2p->comstate[i][j]);
	fprintf (fp, "\n");
    }
    fprintf (fp, "#\n");
    fprintf (fp, "# END\n");
    
    fflush (fp);
}
