/*
 * mdef.c -- HMM model definition: base (CI) phones and triphones
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
 * 13-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added mdef_phone_str().
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Allowed mdef_phone_id_nearest to return base phone id if either
 * 		left or right context (or both) is undefined.
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


/*
 * Major assumptions:
 *   All phones have same #states, same topology.
 *   Every phone has exactly one non-emitting, final state--the last one.
 *   CI phones must appear first in model definition file.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <s3.h>

#include "s3types.h"
#include "mdef.h"

#define MODEL_DEF_VERSION	"0.3"


static mdef_t *mdef = NULL;	/* The model definition structure */

static int32 max_ciphone;
static int32 max_phone;

static char *wpos_name = WPOS_NAME;
static char *delim = " \t\n";


static void ciphone_add (mdef_t *m, char *ci, s3pid_t p)
{
    if (mdef->n_ciphone >= max_ciphone)
	E_FATAL("Max CI phones (%d) exceeded\n", max_ciphone);
    
    /* Make sure pid being assigned is the next free ciphone */
    assert (p == mdef->n_ciphone);

    /* Note ciphone name */
    mdef->ciphone[p].name = (char *) ckd_salloc (ci);
    if (hash_enter (mdef->ciphone_ht, mdef->ciphone[p].name, p) < 0)
	E_FATAL("hash_enter(%s) failed\n", mdef->ciphone[p].name);
    
    mdef->n_ciphone++;
}


static ph_lc_t *find_ph_lc (ph_lc_t *lclist, s3cipid_t lc)
{
    ph_lc_t *lcptr;

    for (lcptr = lclist; lcptr && (lcptr->lc != lc); lcptr = lcptr->next);
    return lcptr;
}


static ph_rc_t *find_ph_rc (ph_rc_t *rclist, s3cipid_t rc)
{
    ph_rc_t *rcptr;

    for (rcptr = rclist; rcptr && (rcptr->rc != rc); rcptr = rcptr->next);
    return rcptr;
}


static int32 triphone_add (mdef_t *m,
			   s3cipid_t ci, s3cipid_t lc, s3cipid_t rc, word_posn_t wpos,
			   s3pid_t p)
{
    ph_lc_t *lcptr;
    ph_rc_t *rcptr;

    if (mdef->n_phone >= max_phone)
	E_FATAL("Max phones (%d) exceeded\n", max_phone);
    
    assert (p == mdef->n_phone);

    /* Fill in phone[p] information */
    mdef->phone[p].ci = ci;
    mdef->phone[p].lc = lc;
    mdef->phone[p].rc = rc;
    mdef->phone[p].wpos = wpos;

    if (p >= max_ciphone) {
	/* Create <ci,lc,rc,wpos> -> p mapping */
	if ((lcptr = find_ph_lc (m->wpos_ci_lclist[wpos][ci], lc)) == NULL) {
	    lcptr = (ph_lc_t *) ckd_calloc (1, sizeof(ph_lc_t));
	    lcptr->lc = lc;
	    lcptr->next = m->wpos_ci_lclist[wpos][ci];
	    m->wpos_ci_lclist[wpos][ci] = lcptr;
	}
	if ((rcptr = find_ph_rc (lcptr->rclist, rc)) != NULL)
	    return -1;	/* Duplicate triphone */
	
	rcptr = (ph_rc_t *) ckd_calloc (1, sizeof(ph_rc_t));
	rcptr->rc = rc;
	rcptr->pid = p;
	rcptr->next = lcptr->rclist;
	lcptr->rclist = rcptr;
    }

    mdef->n_phone++;

    return 0;
}


s3cipid_t mdef_ciphone_id (mdef_t *m, char *ci)
{
    int32 id;
    
    assert (m);
    assert (ci);
    
    if (hash_lookup (m->ciphone_ht, ci, &id) < 0)
	return (BAD_CIPID);
    return ((s3cipid_t) id);
}


char *mdef_ciphone_str (mdef_t *m, s3cipid_t id)
{
    assert (m);
    assert ((id >= 0) && (id < mdef->n_ciphone));
    
    return (m->ciphone[id].name);
}


int32 mdef_phone_str (mdef_t *m, s3pid_t pid, char *buf)
{
    assert (m);
    assert ((pid >= 0) && (pid < mdef->n_phone));
    
    buf[0] = '\0';
    if (pid < mdef->n_ciphone)
	sprintf (buf, "%s", mdef_ciphone_str (m, (s3cipid_t) pid));
    else {
	sprintf (buf, "%s %s %s %c",
		 mdef_ciphone_str(m, m->phone[pid].ci),
		 mdef_ciphone_str(m, m->phone[pid].lc),
		 mdef_ciphone_str(m, m->phone[pid].rc),
		 wpos_name[m->phone[pid].wpos]);
    }
    return 0;
}


s3pid_t mdef_phone_id (mdef_t *m, 
		       s3cipid_t ci, s3cipid_t lc, s3cipid_t rc, word_posn_t wpos)
{
    ph_lc_t *lcptr;
    ph_rc_t *rcptr;
    
    assert (m);
    assert ((ci >= 0) && (ci < mdef->n_ciphone));
    assert ((lc >= 0) && (lc < mdef->n_ciphone));
    assert ((rc >= 0) && (rc < mdef->n_ciphone));
    assert ((wpos >= 0) && (wpos < N_WORD_POSN));

    if (((lcptr = find_ph_lc (m->wpos_ci_lclist[wpos][ci], lc)) == NULL) ||
	((rcptr = find_ph_rc (lcptr->rclist, rc)) == NULL))
	return (BAD_PID);

    return (rcptr->pid);
}


s3pid_t mdef_phone_id_nearest (mdef_t *m, 
			       s3cipid_t b, s3cipid_t l, s3cipid_t r, word_posn_t pos)
{
    word_posn_t tmppos;
    s3pid_t p;
    s3cipid_t newl, newr;
    
    assert (m);
    assert ((b >= 0) && (b < m->n_ciphone));
    assert ((pos >= 0) && (pos < N_WORD_POSN));

    if ((NOT_CIPID(l)) || (NOT_CIPID(r)))
	return ((s3pid_t) b);
    
    assert ((l >= 0) && (l < m->n_ciphone));
    assert ((r >= 0) && (r < m->n_ciphone));
    
    p = mdef_phone_id (m, b, l, r, pos);
    if (IS_PID(p))
	return p;
    
    /* Exact triphone not found; backoff to other word positions */
    for (tmppos = 0; tmppos < N_WORD_POSN; tmppos++) {
	if (tmppos != pos) {
	    p = mdef_phone_id (m, b, l, r, tmppos);
	    if (IS_PID(p))
		return p;
	}
    }
    
    /* Nothing yet; backoff to silence phone if non-silence filler context */
    if (IS_CIPID(m->sil)) {
	newl = m->ciphone[l].filler ? m->sil : l;
	newr = m->ciphone[r].filler ? m->sil : r;
	if ((newl != l) || (newr != r)) {
	    p = mdef_phone_id (m, b, newl, newr, pos);
	    if (IS_PID(p))
		return p;
	    
	    for (tmppos = 0; tmppos < N_WORD_POSN; tmppos++) {
		if (tmppos != pos) {
		    p = mdef_phone_id (m, b, newl, newr, tmppos);
		    if (IS_PID(p))
			return p;
		}
	    }
	}
    }
    
    /* Nothing yet; backoff to base phone */
    return ((s3pid_t) b);
}


int32 mdef_phone_components (mdef_t *m,
			     s3pid_t p,
			     s3cipid_t *b,
			     s3cipid_t *l,
			     s3cipid_t *r,
			     word_posn_t *pos)
{
    assert (m);
    assert ((p >= 0) && (p < mdef->n_phone));

    *b = mdef->phone[p].ci;
    *l = mdef->phone[p].lc;
    *r = mdef->phone[p].rc;
    *pos = mdef->phone[p].wpos;

    return 0;
}


s3cipid_t mdef_is_ciphone (mdef_t *m, s3pid_t p)
{
    assert (m);
    assert ((p >= 0) && (p < mdef->n_phone));
    
    return ((p < m->n_ciphone) ? 1 : 0);
}


static int32 parse_tmat_senmap (mdef_t *m, char *line, int32 lineno, s3pid_t p)
{
    int32 wlen, n, s;
    char *word;
    char tmp;

    /* Read transition matrix id */
    if ((sscanf (line, "%d%n", &n, &wlen) != 1) || (n < 0))
        E_FATAL("Line %d: Missing or bad transition matrix id\n", lineno);
    m->phone[p].tmat = n;
    if (mdef->n_tmat <= n)
	E_FATAL("Line %d: tmat-id(%d) > #tmat in header(%d)\n",
		lineno, n, mdef->n_tmat);
    line += wlen;
    
    /* Read senone mappings for each emitting state */
    for (n = 0; n < mdef->n_emit_state; n++) {
        if ((sscanf (line, "%d%n", &s, &wlen) != 1) || (s < 0))
	    E_FATAL("Line %d: Missing or bad state[%d]->senone mapping\n", lineno, n);
	m->phone[p].state[n] = s;
	if (p < max_ciphone) {
	    if (mdef->n_ci_sen <= s)
		E_FATAL("Line %d: CI-senone-id(%d) > #CI-senone in header(%d)\n",
			lineno, s, mdef->n_ci_sen);
	}
	if (mdef->n_sen <= s)
	    E_FATAL("Line %d: Senone-id(%d) > #senone in header(%d)\n",
		    lineno, s, mdef->n_sen);

	line += wlen;
    }

    /* Check for the last non-emitting state N */
    if (((wlen = nextword (line, delim, &word, &tmp)) < 0) ||
	(strcmp (word, "N") != 0))
        E_FATAL("Line %d: Missing non-emitting state spec\n", lineno);
    line = word+wlen;
    word[wlen] = tmp;

    /* Check for end of line */
    if ((wlen = nextword (line, delim, &word, &tmp)) >= 0)
        E_FATAL("Line %d: Non-empty beyond non-emitting final state\n", lineno);

    return 0;
}


static int32 parse_base_line (mdef_t *m, char *line, int32 lineno, s3pid_t p)
{
    int32 wlen, n;
    char *word;
    s3cipid_t ci;
    char tmp;

    /* Read base phone name */
    if ((wlen = nextword (line, delim, &word, &tmp)) < 0)	/* Empty line */
	E_FATAL("Line %d: Incomplete base phone line\n", lineno);

    /* Make sure it's not a duplicate */
    ci = mdef_ciphone_id (m, word);
    if (IS_CIPID(ci))
        E_FATAL("Line %d: Duplicate base phone: %s\n", lineno, word);

    /* Add ciphone to ciphone table with id p */
    ciphone_add (m, word, p);
    ci = p;

    /* Restore original delimiter to word */
    line = word+wlen;
    word[wlen] = tmp;

    /* Read and skip "-" for lc, rc, wpos */
    for (n = 0; n < 3; n++) {
        if ((wlen = nextword (line, delim, &word, &tmp)) < 0)
	    E_FATAL("Line %d: Incomplete base phone line\n", lineno);
	if ((wlen != 1) || (word[0] != '-'))
	    E_FATAL("Line %d: %s instead of '-' in base phone line\n", word, lineno);
	line = word+wlen;
	word[wlen] = tmp;
    }
    
    /* Read filler attribute, if present */
    if ((wlen = nextword (line, delim, &word, &tmp)) < 0)
	E_FATAL("Line %d: Incomplete base phone line\n", lineno);
    if (strcmp (word, "filler") == 0)
        m->ciphone[ci].filler = 1;
    else if (strcmp (word, "n/a") == 0)
        m->ciphone[ci].filler = 0;
    else
        E_FATAL("Line %d: Illegal attribute string: %s\n", lineno, word);
    line = word+wlen;
    word[wlen] = tmp;

    if (triphone_add (m, ci, BAD_CIPID, BAD_CIPID, WORD_POSN_UNDEFINED, p) < 0)
	E_FATAL("Line %d: Duplicate/Bad triphone\n", lineno);

    /* Parse remainder of line: transition matrix and state->senone mappings */
    parse_tmat_senmap (m, line, lineno, p);
    
    return 0;
}


static int32 parse_tri_line (mdef_t *m, char *line, int32 lineno, s3pid_t p)
{
    int32 wlen;
    char *word;
    s3cipid_t ci, lc, rc;
    word_posn_t wpos;
    char tmp;

    /* Read base phone name */
    if ((wlen = nextword (line, delim, &word, &tmp)) < 0)	/* Empty line */
	E_FATAL("Line %d: Incomplete triphone line\n", lineno);
    ci = mdef_ciphone_id (m, word);
    if (NOT_CIPID(ci))
        E_FATAL("Line %d: Unknown base phone in triphone: %s\n", lineno, word);
    line = word+wlen;
    word[wlen] = tmp;

    /* Read lc */
    if ((wlen = nextword (line, delim, &word, &tmp)) < 0)
        E_FATAL("Line %d: Incomplete triphone line\n", lineno);
    lc = mdef_ciphone_id (m, word);
    if (NOT_CIPID(lc))
        E_FATAL("Line %d: Unknown left context in triphone: %s\n", lineno, word);
    line = word+wlen;
    word[wlen] = tmp;

    /* Read rc */
    if ((wlen = nextword (line, delim, &word, &tmp)) < 0)
        E_FATAL("Line %d: Incomplete triphone line\n", lineno);
    rc = mdef_ciphone_id (m, word);
    if (NOT_CIPID(rc))
        E_FATAL("Line %d: Unknown left context in triphone: %s\n", lineno, word);
    line = word+wlen;
    word[wlen] = tmp;
    
    /* Read tripone word-position within word */
    if (((wlen = nextword (line, delim, &word, &tmp)) < 0) ||
	(word[1] != '\0'))
        E_FATAL("Line %d: Missing or bad triphone word-position spec\n", lineno);
    switch (word[0]) {
    case 'b': wpos = WORD_POSN_BEGIN; break;
    case 'e': wpos = WORD_POSN_END; break;
    case 's': wpos = WORD_POSN_SINGLE; break;
    case 'i': wpos = WORD_POSN_INTERNAL; break;
    default: E_FATAL("Line %d: Bad word-position spec: %s\n", lineno, word);
    }
    line = word+wlen;
    word[wlen] = tmp;

    /* Read filler attribute, if present.  Must match base phone attribute */
    if ((wlen = nextword (line, delim, &word, &tmp)) < 0)
	E_FATAL("Line %d: Incomplete base phone line\n", lineno);
    if (((strcmp (word, "filler") == 0) && (m->ciphone[ci].filler)) ||
	((strcmp (word, "n/a") == 0) && (! m->ciphone[ci].filler))) {
    } else
        E_FATAL("Line %d: Bad attribute string: %s\n", lineno, word);
    line = word+wlen;
    word[wlen] = tmp;
    
    if (triphone_add (m, ci, lc, rc, wpos, p) < 0)
	E_FATAL("Line %d: Duplicate/Bad triphone\n", lineno);

    /* Parse remainder of line: transition matrix and state->senone mappings */
    parse_tmat_senmap (m, line, lineno, p);
    
    return 0;
}


static int32 noncomment_line(char *line, int32 size, int32 *n_read, FILE *fp)
{
    while (fgets (line, size, fp) != NULL) {
        (*n_read)++;
        if (line[0] != '#')
	    return 0;
    }
    return -1;
}


/*
 * Initialize phones (ci and triphones) and state->senone mappings from .mdef file.
 */
mdef_t *mdef_init (char *mdeffile)
{
    FILE *fp;
    int32 n_ci, n_tri, n_map, n;
    char tag[1024], buf[1024];
    int32 n_read;
    s3senid_t *senmap;
    s3pid_t p;
    int32 s, ci, cd;
    
    E_INFO("Reading model definition: %s\n", mdeffile);

    assert (! mdef);
    mdef = (mdef_t *) ckd_calloc (1, sizeof(mdef_t));
    
    if ((fp = fopen(mdeffile, "r")) == NULL)
        E_FATAL("fopen(%s,r) failed\n", mdeffile);

    n_read = 0;

    if (noncomment_line(buf, sizeof(buf), &n_read, fp) < 0)
        E_FATAL("Empty file: %s\n", mdeffile);

    if (strncmp(buf, MODEL_DEF_VERSION, strlen(MODEL_DEF_VERSION)) != 0)
        E_FATAL("Line %d: Version error.  Expecing %s, but read %s\n",
		n_read, MODEL_DEF_VERSION, buf);

    /* Read #base phones, #triphones, #senone mappings defined in header */
    n_ci = -1;
    n_tri = -1;
    n_map = -1;
    mdef->n_ci_sen = -1;
    mdef->n_sen = -1;
    mdef->n_tmat = -1;
    do {
	if (noncomment_line(buf, sizeof(buf), &n_read, fp) < 0)
	    E_FATAL("Incomplete header\n");

	if ((sscanf(buf, "%d %s", &n, tag) != 2) || (n < 0))
	    E_FATAL("Line %d: Error in reading header\n", n_read);

	if (strcmp(tag, "n_base") == 0)
	    n_ci = n;
	else if (strcmp(tag, "n_tri") == 0)
	    n_tri = n;
	else if (strcmp(tag, "n_state_map") == 0)
	    n_map = n;
	else if (strcmp(tag, "n_tied_ci_state") == 0)
	    mdef->n_ci_sen = n;
	else if (strcmp(tag, "n_tied_state") == 0)
	    mdef->n_sen = n;
	else if (strcmp(tag, "n_tied_tmat") == 0)
	    mdef->n_tmat = n;
	else
	    E_FATAL("Line %d: Unknown tag: %s\n", n_read, tag);
    } while ((n_ci < 0) || (n_tri < 0) || (n_map < 0) ||
	     (mdef->n_ci_sen < 0) || (mdef->n_sen < 0) || (mdef->n_tmat < 0));
    if ((n_ci == 0) || (mdef->n_ci_sen == 0) || (mdef->n_tmat == 0) ||
	(mdef->n_ci_sen > mdef->n_sen))
        E_FATAL("Incorrect information in header\n");
    if (n_ci >= MAX_CIPID)
	E_FATAL("%s: #CI phones (%d) >= %d limit\n", mdeffile, n_ci, MAX_CIPID);
    if (n_ci + n_tri >= MAX_PID)
	E_FATAL("%s: #Phones (%d) >= %d limit\n", mdeffile, n_ci+n_tri, MAX_PID);
    if (mdef->n_sen >= MAX_SENID)
	E_FATAL("%s: #senones (%d) >= %d limit\n", mdeffile, mdef->n_sen, MAX_SENID);
    if (mdef->n_tmat >= MAX_TMATID)
	E_FATAL("%s: #tmats (%d) >= %d limit\n", mdeffile, mdef->n_tmat, MAX_TMATID);
    
    mdef->n_emit_state = (n_map / (n_ci+n_tri)) - 1;
    if ((mdef->n_emit_state+1) * (n_ci+n_tri) != n_map)
        E_FATAL("Header error: n_state_map not a multiple of n_ci*n_tri\n");

    /* Initialize ciphone info */
    max_ciphone = n_ci;
    mdef->ciphone_ht = hash_new ("ciphone", n_ci);
    mdef->ciphone = (ciphone_t *) ckd_calloc (n_ci, sizeof(ciphone_t));
    mdef->n_ciphone = 0;

    /* Initialize phones info (ciphones + triphones) */
    max_phone = n_ci + n_tri;
    mdef->phone = (phone_t *) ckd_calloc (max_phone, sizeof(phone_t));
    mdef->n_phone = 0;
    
    /* Allocate space for state->senone map for each phone */
    senmap = (s3senid_t *) ckd_calloc (max_phone * mdef->n_emit_state, sizeof(s3senid_t));
    for (p = 0; p < max_phone; p++)
        mdef->phone[p].state = senmap + (p * mdef->n_emit_state);

    /* Allocate initial space for <ci,lc,rc,wpos> -> pid mapping */
    mdef->wpos_ci_lclist = (ph_lc_t ***) ckd_calloc_2d (N_WORD_POSN,
							max_ciphone,
							sizeof(ph_lc_t *));

    /*
     * Read base phones and triphones.  They'll simply be assigned a running sequence
     * number as their "phone-id".  If the phone-id < n_ci, it's a ciphone.
     */

    /* Read base phones */
    for (p = 0; p < n_ci; p++) {
        if (noncomment_line(buf, sizeof(buf), &n_read, fp) < 0)
	    E_FATAL("Premature EOF(%s)\n", mdeffile);
        parse_base_line (mdef, buf, n_read, p);
    }
    mdef->sil = mdef_ciphone_id (mdef, SILENCE_CIPHONE);
    
    /* Read triphones, if any */
    for (; p < max_phone; p++) {
        if (noncomment_line(buf, sizeof(buf), &n_read, fp) < 0)
	    E_FATAL("Premature EOF(%s)\n", mdeffile);
        parse_tri_line (mdef, buf, n_read, p);
    }

    if (noncomment_line(buf, sizeof(buf), &n_read, fp) >= 0)
	E_ERROR("Line %d: File continues beyond expected size\n", n_read);

    /* Build CD senones to CI senones map */
    mdef->cd2cisen = (s3senid_t *) ckd_calloc (mdef->n_sen, sizeof(s3senid_t));
    for (s = 0; s < mdef->n_ci_sen; s++)		/* CI senones */
	mdef->cd2cisen[s] = (s3senid_t) s;
    for (p = n_ci; p < max_phone; p++) {		/* CD senones */
	for (s = 0; s < mdef->n_emit_state; s++) {
	    cd = mdef->phone[p].state[s];
	    ci = mdef->phone[mdef->phone[p].ci].state[s];
	    mdef->cd2cisen[cd] = (s3senid_t) ci;
	}
    }

    E_INFO("%d CI-phones, %d CD-phones, %d emitting states/phone, %d sen, %d CI-sen\n",
	   mdef->n_ciphone, mdef->n_phone - mdef->n_ciphone, mdef->n_emit_state,
	   mdef->n_sen, mdef->n_ci_sen);

    fclose (fp);

    return mdef;
}


mdef_t *mdef_getmdef ( void )
{
    return (mdef);
}
