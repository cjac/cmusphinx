/*
 * lm.h - Disk/memory based word-trigram backoff LM
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
 * 25-Jun-95    M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *              Created.
 */


#ifndef _LIBFBS_LM_H_
#define _LIBFBS_LM_H_


#include <libutil/prim_type.h>

#include "s3types.h"


/* Log quantities represented in either floating or integer format */
typedef union {
    float f;
    int32 l;
} lmlog_t;

typedef struct {
    s3wid_t dictwid;
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
 * Partition bigrams into segments of BG_SEG_SZ consecutive entries, such that
 * #trigrams in each segment <= 2**16 (the corresponding trigram segment).  The
 * bigram_t.firsttg value is then a 16-bit relative index within the trigram
 * segment.  A separate table--lm_t.tg_segbase--has the absolute index of the
 * 1st trigram for each segment.
 */
#define BG_SEG_SZ       512     /* chosen so that #trigram/segment <= 2**16 */
#define LOG_BG_SEG_SZ   9	/* HACK!! Hardwired segment size!! */

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
    
    ug_t *ug;           /* Unigrams */
    bg_t *bg;		/* NULL iff disk-based */
    tg_t *tg;		/* NULL iff disk-based */
    membg_t *membg;	/* membg[w1] = bigrams for lm wid w1 (used iff disk-based) */
    tginfo_t **tginfo;	/* tginfo[w2] = fast trigram access info for bigrams (*,w2) */
    
    lmlog_t *bgprob;    /* Table of actual bigram probs */
    lmlog_t *tgprob;    /* Table of actual trigram probs */
    lmlog_t *tgbowt;    /* Table of actual trigram backoff weights */
    int32 *tg_segbase;  /* tg_segbase[i>>LOG_BG_SEG_SZ] = index of 1st
			   trigram for bigram segment (i>>LOG_BG_SEG_SZ) */
    int32 n_bgprob;
    int32 n_tgprob;
    int32 n_tgbowt;

    FILE *fp;
    int32 byteswap;     /* Whether this file is in the WRONG byte order */
    int32 bgoff;        /* BG and TG offsets into DMP file (used iff disk-based) */
    int32 tgoff;

    float lw;           /* Language weight */
    float uw;           /* Unigram weight */
    int32 wip;          /* Word insertion penalty (LOG) */
    
    char **wordstr;
    
    int32 startwid;     /* LM wid */
    int32 endwid;       /* LM wid */

    /* Statistics */
    int32 n_bg_fill;    /* #bg fill operations */
    int32 n_bg_inmem;   /* #bg in memory */
    int32 n_bg_score;   /* #bg_score operations */
    int32 n_bg_bo;	/* #bg_score ops backed off to ug */
    int32 n_tg_fill;
    int32 n_tg_inmem;
    int32 n_tg_score;
    int32 n_tg_bo;
} lm_t;


typedef struct {
    char *name;
    lm_t *lm;
} lmset_t;


/* Return ptr to the currently active LM */
lm_t *lm_current ( void );


/*
 * Return trigram followers for given two words.  Return value: #trigrams in returned list.
 */
int32 lm_tglist (s3wid_t bw1,	/* In: Dictionary base wid of the 2-word history */
		 s3wid_t bw2,	/* Both must be valid and exist in the LM */
		 tg_t **tg,	/* Out: *tg = array of trigrams for <bw1,bw2> */
		 int32 *bowt);	/* Out: *bowt = backoff-weight for <bw1, bw2> */

/*
 * Return bigram followers for the given word.  Return value: #bigrams in returned list.
 */
int32 lm_bglist (s3wid_t w,	/* In: Dictionary base wid */
		 bg_t **bg,	/* Out: *bg = array of bigrams for w */
		 int32 *bowt);	/* Out: *bowt = backoff-weight for w */


/* Return unigrams in LM.  Return value: #unigrams in returned list. */
int32 lm_uglist (ug_t **ug);	/* Out: *ug = unigram array */

/* Return the trigram prob (actually logs3(prob)) for the given trigram */
int32 lm_tgprob (tg_t *tg);

/* Return the bigram prob (actually logs3(prob)) for the given bigram */
int32 lm_bgprob (bg_t *bg);

/*
 * Return unigram score for the given word.
 */
int32 lm_ug_score (s3wid_t w);

/*
 * Return bigram score for the given two word sequence.  If w1 is BAD_WID, return
 * lm_ug_score (w2).
 */
int32 lm_bg_score (s3wid_t w1, s3wid_t w2);

/*
 * Return trigram score for the given three word sequence.  If w1 is BAD_WID, return
 * lm_bg_score (w2, w3).  If both w1 and w2 are BAD_WID, return lm_ug_score (w3).
 */
int32 lm_tg_score (s3wid_t w1, s3wid_t w2, s3wid_t w3);

/* Return the dictionary word id for given LM wid */
s3wid_t lm_dictwid (s3lmwid_t w);

/* Read an LM (dump) file */
int32 lm_read (char *file, char *name, float64 lw, float64 uw, float64 wip);

/* LM cache related */
void lm_cache_reset ( void );
void lm_cache_stats_dump ( void );


/* Macro versions of access functions */
#define LM_DICTWID(lm,lmwid)	((lm)->ug[(lmwid)].dictwid)
#define LM_TGPROB(lm,tgptr)	((lm)->tgprob[(tgptr)->probid].l)
#define LM_BGPROB(lm,bgptr)	((lm)->bgprob[(bgptr)->probid].l)
#define LM_UGPROB(lm,ugptr)	((ugptr)->prob.l)


#endif /* _LM_H_ */
