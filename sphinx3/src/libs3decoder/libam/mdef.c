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
 * mdef.c -- HMM model definition: base (CI) phones and triphones
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
 * Revision 1.11  2006/03/17  23:35:42  egouvea
 * mdef_is_cisenone failed if only ci phones were used. Changed condition to return 0 if senid is higher than n_sen, satisfying the loops in approx_cont_mgau
 * 
 * Revision 1.10  2006/02/22 16:52:51  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed memory leaks in mdef. 2,  Fixed $, 3, Fixed dox-doc.
 *
 * Revision 1.9.4.1  2005/07/03 22:54:09  arthchan2003
 * move st2senmap into mdef_t, it was not properly freed before. \n
 *
 * Revision 1.9  2005/06/21 18:47:39  arthchan2003
 * Log. 1, Added breport flag to mdef_init, 2, implemented reporting functions to
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added $Log$
 * Revision 1.11  2006/03/17  23:35:42  egouvea
 * mdef_is_cisenone failed if only ci phones were used. Changed condition to return 0 if senid is higher than n_sen, satisfying the loops in approx_cont_mgau
 * 
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added Revision 1.10  2006/02/22 16:52:51  arthchan2003
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed memory leaks in mdef. 2,  Fixed $, 3, Fixed dox-doc.
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added
 *
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 19.Apr-2001  Ricky Houghton, added code for free allocated memory
 *
 * 14-Oct-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added mdef_sseq2sen_active().
 * 
 * 06-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		In mdef_phone_id(), added backing off to silence phone context from filler
 * 		context if original triphone not found.
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added senone-sequence id (ssid) to phone_t and appropriate functions to
 * 		maintain it.  Instead, moved state sequence info to mdef_t.
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "mdef.h"
#include "ckd_alloc.h"
#include "err.h"


#define MODEL_DEF_VERSION	"0.3"

void
mdef_dump(FILE * fp, mdef_t * m)
{
    int32 i, j;
    int32 ssid;
    char buf[1024];

    fprintf(fp, "%d ciphone\n", m->n_ciphone);
    fprintf(fp, "%d phone\n", m->n_phone);
    fprintf(fp, "%d emitstate\n", m->n_emit_state);
    fprintf(fp, "%d cisen\n", m->n_ci_sen);
    fprintf(fp, "%d sen\n", m->n_sen);
    fprintf(fp, "%d tmat\n", m->n_tmat);

    for (i = 0; i < m->n_phone; i++) {
        mdef_phone_str(m, i, buf);
        ssid = m->phone[i].ssid;

        fprintf(fp, "%3d %5d", m->phone[i].tmat, ssid);
        for (j = 0; j < m->n_emit_state; j++)
            fprintf(fp, " %5d", m->sseq[ssid][j]);
        fprintf(fp, "\t");
        for (j = 0; j < m->n_emit_state; j++)
            fprintf(fp, " %3d", m->cd2cisen[m->sseq[ssid][j]]);
        fprintf(fp, "\t%s\n", buf);
    }

    fflush(fp);
}


#if 0
int32
mdef_hmm_cmp(mdef_t * m, s3pid_t p1, s3pid_t p2)
{
    int32 i;

    if (m->phone[p1].tmat != m->phone[p2].tmat)
        return -1;

    for (i = 0; i < m->n_emit_state; i++)
        if (m->phone[p1].state[i] != m->phone[p2].state[i])
            return -1;

    return 0;
}
#endif


static void
ciphone_add(mdef_t * m, const char *ci, s3pid_t p)
{
    assert(p < m->n_ciphone);

    m->ciphone[p].name = (char *) ckd_salloc(ci);       /* freed in mdef_free */
    if (hash_table_enter(m->ciphone_ht, m->ciphone[p].name, (void *)(long)p) != (void *)(long)p)
        E_FATAL("hash_table_enter(%s) failed; duplicate CIphone?\n",
                m->ciphone[p].name);
}


static ph_lc_t *
find_ph_lc(ph_lc_t * lclist, s3cipid_t lc)
{
    ph_lc_t *lcptr;

    for (lcptr = lclist; lcptr && (lcptr->lc != lc); lcptr = lcptr->next);
    return lcptr;
}


static ph_rc_t *
find_ph_rc(ph_rc_t * rclist, s3cipid_t rc)
{
    ph_rc_t *rcptr;

    for (rcptr = rclist; rcptr && (rcptr->rc != rc); rcptr = rcptr->next);
    return rcptr;
}


static void
triphone_add(mdef_t * m,
             s3cipid_t ci, s3cipid_t lc, s3cipid_t rc, word_posn_t wpos,
             s3pid_t p)
{
    ph_lc_t *lcptr;
    ph_rc_t *rcptr;

    assert(p < m->n_phone);

    /* Fill in phone[p] information (state and tmat mappings added later) */
    m->phone[p].ci = ci;
    m->phone[p].lc = lc;
    m->phone[p].rc = rc;
    m->phone[p].wpos = wpos;

    /* Create <ci,lc,rc,wpos> -> p mapping if not a CI phone */
    if (p >= m->n_ciphone) {
        if ((lcptr = find_ph_lc(m->wpos_ci_lclist[wpos][(int) ci], lc))
            == NULL) {
            lcptr = (ph_lc_t *) ckd_calloc(1, sizeof(ph_lc_t)); /* freed at mdef_free, I believe */
            lcptr->lc = lc;
            lcptr->next = m->wpos_ci_lclist[wpos][(int) ci];
            m->wpos_ci_lclist[wpos][(int) ci] = lcptr;  /* This is what needs to be freed */
        }
        if ((rcptr = find_ph_rc(lcptr->rclist, rc)) != NULL) {
            char buf[4096];

            mdef_phone_str(m, rcptr->pid, buf);
            E_FATAL("Duplicate triphone: %s\n", buf);
        }

        rcptr = (ph_rc_t *) ckd_calloc(1, sizeof(ph_rc_t));     /* freed in mdef_free, I believe */
        rcptr->rc = rc;
        rcptr->pid = p;
        rcptr->next = lcptr->rclist;
        lcptr->rclist = rcptr;
    }
}


s3cipid_t
mdef_ciphone_id(mdef_t * m, const char *ci)
{
    void *id;

    assert(m);
    assert(ci);

    if (hash_table_lookup(m->ciphone_ht, ci, &id) < 0)
        return (BAD_S3CIPID);
    return ((s3cipid_t)(long)id);
}


const char *
mdef_ciphone_str(mdef_t * m, s3cipid_t id)
{
    assert(m);
    assert((id >= 0) && (id < m->n_ciphone));

    return (m->ciphone[(int) id].name);
}


int32
mdef_phone_str(mdef_t * m, s3pid_t pid, char *buf)
{
    char *wpos_name;

    assert(m);
    assert((pid >= 0) && (pid < m->n_phone));
    wpos_name = WPOS_NAME;

    buf[0] = '\0';
    if (pid < m->n_ciphone)
        sprintf(buf, "%s", mdef_ciphone_str(m, (s3cipid_t) pid));
    else {
        sprintf(buf, "%s %s %s %c",
                mdef_ciphone_str(m, m->phone[pid].ci),
                mdef_ciphone_str(m, m->phone[pid].lc),
                mdef_ciphone_str(m, m->phone[pid].rc),
                wpos_name[m->phone[pid].wpos]);
    }
    return 0;
}


s3pid_t
mdef_phone_id(mdef_t * m,
              s3cipid_t ci, s3cipid_t lc, s3cipid_t rc, word_posn_t wpos)
{
    ph_lc_t *lcptr;
    ph_rc_t *rcptr;
    s3cipid_t newl, newr;

    assert(m);
    assert((ci >= 0) && (ci < m->n_ciphone));
    assert((lc >= 0) && (lc < m->n_ciphone));
    assert((rc >= 0) && (rc < m->n_ciphone));
    assert((wpos >= 0) && (wpos < N_WORD_POSN));

    if (((lcptr =
          find_ph_lc(m->wpos_ci_lclist[wpos][(int) ci], lc)) == NULL)
        || ((rcptr = find_ph_rc(lcptr->rclist, rc)) == NULL)) {
        /* Not found; backoff to silence context if non-silence filler context */
        if (NOT_S3CIPID(m->sil))
            return BAD_S3PID;

        newl = m->ciphone[(int) lc].filler ? m->sil : lc;
        newr = m->ciphone[(int) rc].filler ? m->sil : rc;
        if ((newl == lc) && (newr == rc))
            return BAD_S3PID;

        return (mdef_phone_id(m, ci, newl, newr, wpos));
    }

    return (rcptr->pid);
}


s3pid_t
mdef_phone_id_nearest(mdef_t * m,
                      s3cipid_t b, s3cipid_t l, s3cipid_t r,
                      word_posn_t pos)
{
    word_posn_t tmppos;
    s3pid_t p;
    s3cipid_t newl, newr;
    char *wpos_name;

    assert(m);
    assert((b >= 0) && (b < m->n_ciphone));
    assert((pos >= 0) && (pos < N_WORD_POSN));

    if ((NOT_S3CIPID(l)) || (NOT_S3CIPID(r)))
        return ((s3pid_t) b);

    assert((l >= 0) && (l < m->n_ciphone));
    assert((r >= 0) && (r < m->n_ciphone));

    p = mdef_phone_id(m, b, l, r, pos);
    if (IS_S3PID(p))
        return p;

    /* Exact triphone not found; backoff to other word positions */
    for (tmppos = 0; tmppos < N_WORD_POSN; tmppos++) {
        if (tmppos != pos) {
            p = mdef_phone_id(m, b, l, r, tmppos);
            if (IS_S3PID(p))
                return p;
        }
    }

    /* Nothing yet; backoff to silence phone if non-silence filler context */
    if (IS_S3CIPID(m->sil)) {
        newl = m->ciphone[(int) l].filler ? m->sil : l;
        newr = m->ciphone[(int) r].filler ? m->sil : r;
        if ((newl != l) || (newr != r)) {
            p = mdef_phone_id(m, b, newl, newr, pos);
            if (IS_S3PID(p))
                return p;

            for (tmppos = 0; tmppos < N_WORD_POSN; tmppos++) {
                if (tmppos != pos) {
                    p = mdef_phone_id(m, b, newl, newr, tmppos);
                    if (IS_S3PID(p))
                        return p;
                }
            }
        }
    }

    /* Nothing yet; backoff to base phone */
    if ((m->n_phone > m->n_ciphone) && (!m->ciphone[(int) b].filler)) {
        wpos_name = WPOS_NAME;
#if 0
        E_WARN("Triphone(%s,%s,%s,%c) not found; backing off to CIphone\n",
               mdef_ciphone_str(m, b),
               mdef_ciphone_str(m, l),
               mdef_ciphone_str(m, r), wpos_name[pos]);
#endif
    }
    return ((s3pid_t) b);
}


int32
mdef_phone_components(mdef_t * m,
                      s3pid_t p,
                      s3cipid_t * b,
                      s3cipid_t * l, s3cipid_t * r, word_posn_t * pos)
{
    assert(m);
    assert((p >= 0) && (p < m->n_phone));

    *b = m->phone[p].ci;
    *l = m->phone[p].lc;
    *r = m->phone[p].rc;
    *pos = m->phone[p].wpos;

    return 0;
}


int32
mdef_is_ciphone(mdef_t * m, s3pid_t p)
{
    assert(m);
    assert((p >= 0) && (p < m->n_phone));

    return ((p < m->n_ciphone) ? 1 : 0);
}

int32
mdef_is_cisenone(mdef_t * m, s3senid_t s)
{
    assert(m);
    if (s >= m->n_sen) {
        return 0;
    }
    assert(s >= 0);
    return ((s == m->cd2cisen[s]) ? 1 : 0);
}


/* Parse tmat and state->senone mappings for phone p and fill in structure */
static void
parse_tmat_senmap(mdef_t * m, const char *line, int32 off, s3pid_t p)
{
    int32 wlen, n, s;
    char word[1024];
    const char *lp;

    lp = line + off;

    /* Read transition matrix id */
    if ((sscanf(lp, "%d%n", &n, &wlen) != 1) || (n < 0))
        E_FATAL("Missing or bad transition matrix id: %s\n", line);
    m->phone[p].tmat = n;
    if (m->n_tmat <= n)
        E_FATAL("tmat-id(%d) > #tmat in header(%d): %s\n", n, m->n_tmat,
                line);
    lp += wlen;

    /* Read senone mappings for each emitting state */
    for (n = 0; n < m->n_emit_state; n++) {
        if ((sscanf(lp, "%d%n", &s, &wlen) != 1) || (s < 0))
            E_FATAL("Missing or bad state[%d]->senone mapping: %s\n", n,
                    line);

        /*20040821 ARCHAN, This line is added to allow 3.x/3.0 compatability. */
        m->phone[p].state[n] = s;

        if ((p < m->n_ciphone) && (m->n_ci_sen <= s))
            E_FATAL("CI-senone-id(%d) > #CI-senones(%d): %s\n", s,
                    m->n_ci_sen, line);
        if (m->n_sen <= s)
            E_FATAL("Senone-id(%d) > #senones(%d): %s\n", s, m->n_sen,
                    line);

        m->sseq[p][n] = s;
        lp += wlen;
    }

    /* Check for the last non-emitting state N */
    if ((sscanf(lp, "%s%n", word, &wlen) != 1) || (strcmp(word, "N") != 0))
        E_FATAL("Missing non-emitting state spec: %s\n", line);
    lp += wlen;

    /* Check for end of line */
    if (sscanf(lp, "%s%n", word, &wlen) == 1)
        E_FATAL("Non-empty beyond non-emitting final state: %s\n", line);
}


static void
parse_base_line(mdef_t * m, const char *line, s3pid_t p)
{
    int32 wlen, n;
    char word[1024];
    const char *lp;
    s3cipid_t ci;

    lp = line;

    /* Read base phone name */
    if (sscanf(lp, "%s%n", word, &wlen) != 1)
        E_FATAL("Missing base phone name: %s\n", line);
    lp += wlen;

    /* Make sure it's not a duplicate */
    ci = mdef_ciphone_id(m, word);
    if (IS_S3CIPID(ci))
        E_FATAL("Duplicate base phone: %s\n", line);

    /* Add ciphone to ciphone table with id p */
    ciphone_add(m, word, p);
    ci = (s3cipid_t) p;

    /* Read and skip "-" for lc, rc, wpos */
    for (n = 0; n < 3; n++) {
        if ((sscanf(lp, "%s%n", word, &wlen) != 1)
            || (strcmp(word, "-") != 0))
            E_FATAL("Bad context info for base phone: %s\n", line);
        lp += wlen;
    }

    /* Read filler attribute, if present */
    if (sscanf(lp, "%s%n", word, &wlen) != 1)
        E_FATAL("Missing filler atribute field: %s\n", line);
    lp += wlen;
    if (strcmp(word, "filler") == 0)
        m->ciphone[(int) ci].filler = 1;
    else if (strcmp(word, "n/a") == 0)
        m->ciphone[(int) ci].filler = 0;
    else
        E_FATAL("Bad filler attribute field: %s\n", line);

    triphone_add(m, ci, BAD_S3CIPID, BAD_S3CIPID, WORD_POSN_UNDEFINED, p);

    /* Parse remainder of line: transition matrix and state->senone mappings */
    parse_tmat_senmap(m, line, lp - line, p);
}


static void
parse_tri_line(mdef_t * m, const char *line, s3pid_t p)
{
    int32 wlen;
    char word[1024];
    const char *lp;
    s3cipid_t ci, lc, rc;
    word_posn_t wpos = WORD_POSN_BEGIN;

    lp = line;

    /* Read base phone name */
    if (sscanf(lp, "%s%n", word, &wlen) != 1)
        E_FATAL("Missing base phone name: %s\n", line);
    lp += wlen;

    ci = mdef_ciphone_id(m, word);
    if (NOT_S3CIPID(ci))
        E_FATAL("Unknown base phone: %s\n", line);

    /* Read lc */
    if (sscanf(lp, "%s%n", word, &wlen) != 1)
        E_FATAL("Missing left context: %s\n", line);
    lp += wlen;
    lc = mdef_ciphone_id(m, word);
    if (NOT_S3CIPID(lc))
        E_FATAL("Unknown left context: %s\n", line);

    /* Read rc */
    if (sscanf(lp, "%s%n", word, &wlen) != 1)
        E_FATAL("Missing right context: %s\n", line);
    lp += wlen;
    rc = mdef_ciphone_id(m, word);
    if (NOT_S3CIPID(rc))
        E_FATAL("Unknown right  context: %s\n", line);

    /* Read tripone word-position within word */
    if ((sscanf(lp, "%s%n", word, &wlen) != 1) || (word[1] != '\0'))
        E_FATAL("Missing or bad word-position spec: %s\n", line);
    lp += wlen;
    switch (word[0]) {
    case 'b':
        wpos = WORD_POSN_BEGIN;
        break;
    case 'e':
        wpos = WORD_POSN_END;
        break;
    case 's':
        wpos = WORD_POSN_SINGLE;
        break;
    case 'i':
        wpos = WORD_POSN_INTERNAL;
        break;
    default:
        E_FATAL("Bad word-position spec: %s\n", line);
    }

    /* Read filler attribute, if present.  Must match base phone attribute */
    if (sscanf(lp, "%s%n", word, &wlen) != 1)
        E_FATAL("Missing filler attribute field: %s\n", line);
    lp += wlen;
    if (((strcmp(word, "filler") == 0) && (m->ciphone[(int) ci].filler)) ||
        ((strcmp(word, "n/a") == 0) && (!m->ciphone[(int) ci].filler))) {
        /* Everything is fine */
    }
    else
        E_FATAL("Bad filler attribute field: %s\n", line);

    triphone_add(m, ci, lc, rc, wpos, p);

    /* Parse remainder of line: transition matrix and state->senone mappings */
    parse_tmat_senmap(m, line, lp - line, p);
}


static void
sseq_compress(mdef_t * m)
{
    hash_table_t *h;
    s3senid_t **sseq;
    int32 n_sseq;
    int32 p, j, k;
    glist_t g;
    gnode_t *gn;
    hash_entry_t *he;

    k = m->n_emit_state * sizeof(s3senid_t);

    h = hash_table_new(m->n_phone, HASH_CASE_YES);
    n_sseq = 0;

    /* Identify unique senone-sequence IDs.  BUG: tmat-id not being considered!! */
    for (p = 0; p < m->n_phone; p++) {
        /* Add senone sequence to hash table */
	if ((j = (long)
             hash_table_enter_bkey(h, (char *) (m->sseq[p]), k,
				   (void *)(long)n_sseq)) == n_sseq)
            n_sseq++;

        m->phone[p].ssid = j;
    }

    /* Generate compacted sseq table */
    sseq = (s3senid_t **) ckd_calloc_2d(n_sseq, m->n_emit_state, sizeof(s3senid_t));    /* freed in mdef_free() */

    g = hash_table_tolist(h, &j);
    assert(j == n_sseq);

    for (gn = g; gn; gn = gnode_next(gn)) {
        he = (hash_entry_t *) gnode_ptr(gn);
        j = (int32)(long)hash_entry_val(he);
        memcpy(sseq[j], hash_entry_key(he), k);
    }
    glist_free(g);

    /* Free the old, temporary senone sequence table, replace with compacted one */
    ckd_free_2d((void **) m->sseq);
    m->sseq = sseq;
    m->n_sseq = n_sseq;

    hash_table_free(h);
}


static int32
noncomment_line(char *line, int32 size, FILE * fp)
{
    while (fgets(line, size, fp) != NULL) {
        if (line[0] != '#')
            return 0;
    }
    return -1;
}


/*
 * Initialize phones (ci and triphones) and state->senone mappings from .mdef file.
 */
mdef_t *
mdef_init(const char *mdeffile, int32 breport)
{
    FILE *fp;
    int32 n_ci, n_tri, n_map, n;
    char tag[1024], buf[1024];
    s3senid_t **senmap;
    /*    s3senid_t *tempsenmap; */

    s3pid_t p;
    int32 s, ci, cd;
    mdef_t *m;

    if (!mdeffile)
        E_FATAL("No mdef-file\n");

    if (breport)
        E_INFO("Reading model definition: %s\n", mdeffile);

    m = (mdef_t *) ckd_calloc(1, sizeof(mdef_t));       /* freed in mdef_free */

    if ((fp = fopen(mdeffile, "r")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,r) failed\n", mdeffile);

    if (noncomment_line(buf, sizeof(buf), fp) < 0)
        E_FATAL("Empty file: %s\n", mdeffile);

    if (strncmp(buf, MODEL_DEF_VERSION, strlen(MODEL_DEF_VERSION)) != 0)
        E_FATAL("Version error: Expecing %s, but read %s\n",
                MODEL_DEF_VERSION, buf);

    /* Read #base phones, #triphones, #senone mappings defined in header */
    n_ci = -1;
    n_tri = -1;
    n_map = -1;
    m->n_ci_sen = -1;
    m->n_sen = -1;
    m->n_tmat = -1;
    do {
        if (noncomment_line(buf, sizeof(buf), fp) < 0)
            E_FATAL("Incomplete header\n");

        if ((sscanf(buf, "%d %s", &n, tag) != 2) || (n < 0))
            E_FATAL("Error in header: %s\n", buf);

        if (strcmp(tag, "n_base") == 0)
            n_ci = n;
        else if (strcmp(tag, "n_tri") == 0)
            n_tri = n;
        else if (strcmp(tag, "n_state_map") == 0)
            n_map = n;
        else if (strcmp(tag, "n_tied_ci_state") == 0)
            m->n_ci_sen = n;
        else if (strcmp(tag, "n_tied_state") == 0)
            m->n_sen = n;
        else if (strcmp(tag, "n_tied_tmat") == 0)
            m->n_tmat = n;
        else
            E_FATAL("Unknown header line: %s\n", buf);
    } while ((n_ci < 0) || (n_tri < 0) || (n_map < 0) ||
             (m->n_ci_sen < 0) || (m->n_sen < 0) || (m->n_tmat < 0));

    if ((n_ci == 0) || (m->n_ci_sen == 0) || (m->n_tmat == 0)
        || (m->n_ci_sen > m->n_sen))
        E_FATAL("%s: Error in header\n", mdeffile);

    /* Check typesize limits */
    if (n_ci >= MAX_S3CIPID)
        E_FATAL("%s: #CI phones (%d) exceeds limit (%d)\n", mdeffile, n_ci,
                MAX_S3CIPID);
    if (n_ci + n_tri >= MAX_S3PID)
        E_FATAL("%s: #Phones (%d) exceeds limit (%d)\n", mdeffile,
                n_ci + n_tri, MAX_S3PID);
    if (m->n_sen >= MAX_S3SENID)
        E_FATAL("%s: #senones (%d) exceeds limit (%d)\n", mdeffile,
                m->n_sen, MAX_S3SENID);
    if (m->n_tmat >= MAX_S3TMATID)
        E_FATAL("%s: #tmats (%d) exceeds limit (%d)\n", mdeffile,
                m->n_tmat, MAX_S3TMATID);

    m->n_emit_state = (n_map / (n_ci + n_tri)) - 1;
    if ((m->n_emit_state + 1) * (n_ci + n_tri) != n_map)
        E_FATAL
            ("Header error: n_state_map not a multiple of n_ci*n_tri\n");

    /* Initialize ciphone info */
    m->n_ciphone = n_ci;
    m->ciphone_ht = hash_table_new(n_ci, 1);  /* With case-insensitive string names *//* freed in mdef_free */
    m->ciphone = (ciphone_t *) ckd_calloc(n_ci, sizeof(ciphone_t));     /* freed in mdef_free */

    /* Initialize phones info (ciphones + triphones) */
    m->n_phone = n_ci + n_tri;
    m->phone = (phone_t *) ckd_calloc(m->n_phone, sizeof(phone_t));     /* freed in mdef_free */

    /* Allocate space for state->senone map for each phone */
    senmap = (s3senid_t **) ckd_calloc_2d(m->n_phone, m->n_emit_state, sizeof(s3senid_t));      /* freed in mdef_free */
    m->sseq = senmap;           /* TEMPORARY; until it is compressed into just the unique ones */


    /**CODE DUPLICATION!*****************************************************************************************************/
    /* Flat decoder-specific */
    /* Allocate space for state->senone map for each phone */

    /* ARCHAN 20040820, this sacrifice readability and may cause pointer
       problems in future. However, this is a less evil than
       duplication of code.  This is trick point all the state mapping
       to the global mapping and avoid duplicated memory.  
     */

    /* S3 xwdpid_compress will compress the below list phone list. 
     */

    /* ARCHAN, this part should not be used when one of the recognizer is used. */
    m->st2senmap =
        (s3senid_t *) ckd_calloc(m->n_phone * m->n_emit_state,
                                 sizeof(s3senid_t));
    for (p = 0; p < m->n_phone; p++)
        m->phone[p].state = m->st2senmap + (p * m->n_emit_state);
    /******************************************************************************************************/


    /* Allocate initial space for <ci,lc,rc,wpos> -> pid mapping */
    m->wpos_ci_lclist = (ph_lc_t ***) ckd_calloc_2d(N_WORD_POSN, m->n_ciphone, sizeof(ph_lc_t *));      /* freed in mdef_free */

    /*
     * Read base phones and triphones.  They'll simply be assigned a running sequence
     * number as their "phone-id".  If the phone-id < n_ci, it's a ciphone.
     */

    /* Read base phones */
    for (p = 0; p < n_ci; p++) {
        if (noncomment_line(buf, sizeof(buf), fp) < 0)
            E_FATAL("Premature EOF reading CIphone %d\n", p);
        parse_base_line(m, buf, p);
    }
    m->sil = mdef_ciphone_id(m, S3_SILENCE_CIPHONE);

    /* Read triphones, if any */
    for (; p < m->n_phone; p++) {
        if (noncomment_line(buf, sizeof(buf), fp) < 0)
            E_FATAL("Premature EOF reading phone %d\n", p);
        parse_tri_line(m, buf, p);
    }

    if (noncomment_line(buf, sizeof(buf), fp) >= 0)
        E_ERROR("Non-empty file beyond expected #phones (%d)\n",
                m->n_phone);

    /* Build CD senones to CI senones map */
    if (m->n_ciphone * m->n_emit_state != m->n_ci_sen)
        E_FATAL
            ("#CI-senones(%d) != #CI-phone(%d) x #emitting-states(%d)\n",
             m->n_ci_sen, m->n_ciphone, m->n_emit_state);
    m->cd2cisen = (s3senid_t *) ckd_calloc(m->n_sen, sizeof(s3senid_t));        /* freed in mdef_free */

    m->sen2cimap = (s3cipid_t *) ckd_calloc(m->n_sen, sizeof(s3cipid_t));       /* freed in mdef_free */

    for (s = 0; s < m->n_sen; s++)
        m->sen2cimap[s] = BAD_S3CIPID;
    for (s = 0; s < m->n_ci_sen; s++) { /* CI senones */
        m->cd2cisen[s] = (s3senid_t) s;
        m->sen2cimap[s] = s / m->n_emit_state;
    }
    for (p = n_ci; p < m->n_phone; p++) {       /* CD senones */
        for (s = 0; s < m->n_emit_state; s++) {
            cd = m->sseq[p][s];
            ci = m->sseq[(int) m->phone[p].ci][s];
            m->cd2cisen[cd] = (s3senid_t) ci;
            m->sen2cimap[cd] = m->phone[p].ci;
        }
    }

    sseq_compress(m);
    fclose(fp);

    return m;
}

void
mdef_report(mdef_t * m)
{
    E_INFO_NOFN("Initialization of mdef_t, report:\n");
    E_INFO_NOFN
        ("%d CI-phone, %d CD-phone, %d emitstate/phone, %d CI-sen, %d Sen, %d Sen-Seq\n",
         m->n_ciphone, m->n_phone - m->n_ciphone, m->n_emit_state,
         m->n_ci_sen, m->n_sen, m->n_sseq);
    E_INFO_NOFN("\n");

}

void
mdef_sseq2sen_active(mdef_t * mdef, uint8 * sseq, uint8 * sen)
{
    int32 ss, i;
    s3senid_t *sp;

    for (ss = 0; ss < mdef_n_sseq(mdef); ss++) {
        if (sseq[ss]) {
            sp = mdef->sseq[ss];
            for (i = 0; i < mdef_n_emit_state(mdef); i++)
                sen[sp[i]] = 1;
        }
    }
}

/* RAH 4.23.01, Need to step down the ->next list to see if there are
   any more things to free
 */



/* RAH 4.19.01, Attempt to free memory that was allocated within this module
   I have not verified that all the memory has been freed. I've taken only a 
   reasonable effort for now.
   RAH 4.24.01 - verified that all memory is released.
 */
void
mdef_free_recursive_lc(ph_lc_t * lc)
{
    if (lc == NULL)
        return;

    if (lc->rclist)
        mdef_free_recursive_rc(lc->rclist);

    if (lc->next)
        mdef_free_recursive_lc(lc->next);

    ckd_free((void *) lc);
}

void
mdef_free_recursive_rc(ph_rc_t * rc)
{
    if (rc == NULL)
        return;

    if (rc->next)
        mdef_free_recursive_rc(rc->next);

    ckd_free((void *) rc);
}


/* RAH, Free memory that was allocated in mdef_init 
   Rational purify shows that no leaks exist
 */

void
mdef_free(mdef_t * m)
{
    int i, j;

    if (m) {
        if (m->sen2cimap)
            ckd_free((void *) m->sen2cimap);
        if (m->cd2cisen)
            ckd_free((void *) m->cd2cisen);

        /* RAH, go down the ->next list and delete all the pieces */
        for (i = 0; i < N_WORD_POSN; i++)
            for (j = 0; j < m->n_ciphone; j++)
                if (m->wpos_ci_lclist[i][j]) {
                    mdef_free_recursive_lc(m->wpos_ci_lclist[i][j]->next);
                    mdef_free_recursive_rc(m->wpos_ci_lclist[i][j]->
                                           rclist);
                }

        for (i = 0; i < N_WORD_POSN; i++)
            for (j = 0; j < m->n_ciphone; j++)
                if (m->wpos_ci_lclist[i][j])
                    ckd_free((void *) m->wpos_ci_lclist[i][j]);


        if (m->wpos_ci_lclist)
            ckd_free_2d((void *) m->wpos_ci_lclist);
        if (m->sseq)
            ckd_free_2d((void *) m->sseq);
        /* Free phone context */
        if (m->phone)
            ckd_free((void *) m->phone);
        if (m->ciphone_ht)
            hash_table_free(m->ciphone_ht);

        for (i = 0; i < m->n_ciphone; i++) {
            if (m->ciphone[i].name)
                ckd_free((void *) m->ciphone[i].name);
        }


        if (m->ciphone)
            ckd_free((void *) m->ciphone);

        if (m->st2senmap)
            ckd_free((void *) m->st2senmap);

        ckd_free((void *) m);
    }
}
