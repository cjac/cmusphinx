/*
 * lm.h - Disk/memory based word-trigram backoff LM
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
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.log_bg_seg_sz and lm_t.bg_seg_sz.
 * 
 * 13-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *              Created from original S3 version.
 */


#ifndef _LIBMAIN_LM_H_
#define _LIBMAIN_LM_H_


#include "s3types.h"


/* Log quantities represented in either floating or integer format */
typedef union {
    float32 f;
    int32 l;
} lmlog_t;

typedef struct {
    s3wid_t dictwid;	/* Dictionary word id, or BAD_WID if unknown.  However, the LM
			   module merely sets this field to BAD_WID.  It is upto the
			   application to fill in this field (HACK!!), so that this
			   module can be independent of a dictionary. */
    lmlog_t prob;
    lmlog_t bowt;
    int32 firstbg;	/* 1st bigram entry on disk */
} ug_t;

typedef struct {
    s3lmwid_t wid;	/* LM wid (index into lm_t.ug) */
    uint16 probid;
    uint16 bowtid;
    uint16 firsttg;     /* 1st trigram entry on disk (see tg_segbase below) */
} bg_t;

typedef struct {
    s3lmwid_t wid;	/* LM wid (index into lm_t.ug) */
    uint16 probid;
} tg_t;


/*
 * Management of in-memory bigrams.  Not used if all bigrams in memory.
 */
typedef struct {
    bg_t *bg;		/* Bigrams for a specific unigram; see lm_t.membg */
    int32 used;		/* Whether used since last lm_reset.  If not used, at the next
			   lm_reset bg are freed */
} membg_t;

/*
 * The following trigram information cache eliminates most traversals of 1g->2g->3g
 * tree to locate trigrams for a given bigram (w1,w2).  The organization is optimized
 * for locality of access.  All bigrams (*,w2) for a given w2, for which trigrams have
 * been accessed "recently", form a linear linked list, pointed to by lm_t.tginfo[w2].
 * If disk-based, all trigrams for the given bg loaded upon request.  Cached info (and
 * tg if disk-based) freed at lm_reset if not used since last such reset.
 */
typedef struct tginfo_s {
    s3lmwid_t w1;		/* w1 component of bigram w1,w2.  All bigrams with
				   same w2 linked together. */
    int32 n_tg;			/* #tg for parent bigram w1,w2 */
    tg_t *tg;			/* Trigrams for w1,w2 */
    int32 bowt;			/* tg bowt for w1,w2 */
    int32 used;			/* whether used since last lm_reset */
    struct tginfo_s *next;	/* Next w1 with same parent w2 */
} tginfo_t;


/*
 * To conserve space, bg/tg probs/ptrs kept in many tables.  Since the number of
 * distinct prob values << #bg/#tg, these table indices can be easily fit into
 * 16 bits.  bgprob and bgbowt are such indices.  The firsttg entry for a bigram
 * is harder.  It is supposed to be the index of the first trigram entry for each
 * bigram.  But #tg can be >> 2^16.  Hence the following segmentation scheme:
 * Partition bigrams into segments of lm_t.bg_seg_sz consecutive entries, such that
 * #trigrams in each segment <= 2**16 (the corresponding trigram segment).  The
 * bigram_t.firsttg value is then a 16-bit relative index within the trigram
 * segment.  A separate table--lm_t.tg_segbase--has the absolute index of the
 * 1st trigram for each segment.
 */

/* Default values for lm_t.log_bg_seg.sz */
#define LOG2_BG_SEG_SZ  9	
#define BG_SEG_SZ       (1 << (LOG2_BG_SEG_SZ))


/*
 * The language model.
 * All unigrams are read into memory on initialization.
 * Bigrams and trigrams read in on demand.
 */
typedef struct lm_s {
    int32 n_ug;         /* #unigrams in LM */
    int32 n_bg;         /* #bigrams in entire LM */
    int32 n_tg;         /* #trigrams in entire LM */
    int32 max_ug;       /* To which n_ug can grow with dynamic addition of words */
    
    char **wordstr;	/* The LM word list (in unigram order) */
    
    int32 log_bg_seg_sz;/* See big comment above */
    int32 bg_seg_sz;
    
    ug_t *ug;           /* Unigrams */
    bg_t *bg;		/* NULL iff disk-based */
    tg_t *tg;		/* NULL iff disk-based */
    membg_t *membg;	/* membg[w1] = bigrams for lm wid w1 (used iff disk-based) */
    tginfo_t **tginfo;	/* tginfo[w2] = fast trigram access info for bigrams (*,w2) */
    
    lmlog_t *bgprob;    /* Table of actual bigram probs */
    lmlog_t *tgprob;    /* Table of actual trigram probs */
    lmlog_t *tgbowt;    /* Table of actual trigram backoff weights */
    int32 *tg_segbase;  /* tg_segbase[i>>lm_t.log_bg_seg_sz] = index of 1st
			   trigram for bigram segment (i>>lm_t.log_bg_seg_sz) */
    int32 n_bgprob;
    int32 n_tgprob;
    int32 n_tgbowt;

    FILE *fp;
    int32 byteswap;     /* Whether this file is in the WRONG byte order */
    int32 bgoff;        /* BG and TG offsets into DMP file (used iff disk-based) */
    int32 tgoff;

    float32 lw;		/* Language weight currently in effect for this LM */
    int32 wip;          /* logs3(word insertion penalty) in effect for this LM */

    /* Statistics */
    int32 n_bg_fill;    /* #bg fill operations */
    int32 n_bg_inmem;   /* #bg in memory */
    int32 n_bg_score;   /* #bg_score operations */
    int32 n_bg_bo;	/* #bg_score ops backed off to ug */
    int32 n_tg_fill;	/* Similar stats for trigrams */
    int32 n_tg_inmem;
    int32 n_tg_score;
    int32 n_tg_bo;
} lm_t;


/*
 * Read an LM (dump) file; return pointer to LM structure created.
 */
lm_t *lm_read (char *file,	/* In: LM file being read */
	       float64 lw,	/* In: Language weight */
	       float64 wip);	/* In: Word insertion penalty */

/*
 * Return trigram followers for given two words.  Both w1 and w2 must be valid.
 * Return value: #trigrams in returned list.
 */
int32 lm_tglist (lm_t *lmp,	/* In: LM being queried */
		 s3lmwid_t w1,	/* In: LM word id of the first of a 2-word history */
		 s3lmwid_t w2,	/* In: LM word id of the second of the 2-word history */
		 tg_t **tg,	/* Out: *tg = array of trigrams for <w1,w2> */
		 int32 *bowt);	/* Out: *bowt = backoff-weight for <w1, w2> */

/*
 * Return the bigram followers for the given word w.
 * Return value: #bigrams in returned list.
 */
int32 lm_bglist (lm_t *lmp,	/* In: LM being queried */
		 s3lmwid_t w,	/* In: LM word id of the 1-word history */
		 bg_t **bg,	/* Out: *bg = array of bigrams for w */
		 int32 *bowt);	/* Out: *bowt = backoff-weight for w */


/* Return the unigrams in LM.  Return value: #unigrams in returned list. */
int32 lm_uglist (lm_t *lmp,	/* In: LM being queried */
		 ug_t **ug);	/* Out: *ug = unigram array */


/* Return unigram score for the given word */
int32 lm_ug_score (lm_t *lmp, s3lmwid_t w);


/*
 * Return bigram score for the given two word sequence.  If w1 is BAD_LMWID, return
 * lm_ug_score (w2).
 */
int32 lm_bg_score (lm_t *lmp, s3lmwid_t w1, s3lmwid_t w2);


/*
 * Return trigram score for the given three word sequence.  If w1 is BAD_LMWID, return
 * lm_bg_score (w2, w3).  If both w1 and w2 are BAD_LMWID, return lm_ug_score (w3).
 */
int32 lm_tg_score (lm_t *lmp, s3lmwid_t w1, s3lmwid_t w2, s3lmwid_t w3);


/*
 * Set the language-weight and insertion penalty parameters for the LM, after revoking
 * any earlier set of such parameters.
 */
void lm_set_param (lm_t *lm, float64 lw, float64 wip);


/* LM cache related */
void lm_cache_reset (lm_t *lmp);
void lm_cache_stats_dump (lm_t *lmp);


/* Macro versions of access functions */
#define LM_TGPROB(lm,tgptr)	((lm)->tgprob[(tgptr)->probid].l)
#define LM_BGPROB(lm,bgptr)	((lm)->bgprob[(bgptr)->probid].l)
#define LM_UGPROB(lm,ugptr)	((ugptr)->prob.l)
#define LM_RAWSCORE(lm,score)	((score - (lm)->wip) / ((lm)->lw))


#endif
