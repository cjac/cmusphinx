/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Adding lm_free() to free allocated memory
 * 
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.access_type; made lm_wid externally visible.
 * 
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.log_bg_seg_sz and lm_t.bg_seg_sz.
 * 
 * 13-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *              Created from original S3 version.
 */


#ifndef _S3_LM_H_
#define _S3_LM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LM_DICTWID_BADMAP	-16000		/* An illegal mapping */
#define LM_CLASSID_BASE		0x01000000	/* Interpreted as LMclass ID */
#define LM_CLASSID_TO_CLASS(m,i)	((m)->lmclass[(i)-LM_CLASSID_BASE])
#define MIN_PROB_F -99.0

#include <libutil/libutil.h>
#include "s3types.h"
#include "lmclass.h"
#include "dict.h"


/* Log quantities represented in either floating or integer format */
typedef union {
    float32 f;
    int32 l;
} lmlog_t;

typedef struct {
    s3wid_t dictwid;	/* Dictionary word id, or BAD_S3WID if unknown.  However, the LM
			   module merely sets this field to BAD_S3WID.  It is upto the
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
 * Entries in a fast and dirty cache for trigram lookups.  See lm_t.tgcache.
 */
typedef struct {
    s3lmwid_t lwid[3];		/* 0 = oldest, 2 = newest (i.e., P(2|0,1)) */
    int32 lscr;			/* LM score for above trigram */
} lm_tgcache_entry_t;


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

#define LM_TGCACHE_SIZE		100003	/* A prime no. (hopefully it IS one!) */
/* 20040211 ARCHAN: Yes! Indeed it is a prime */


/*
 *  The structure for control of LM
 */
/* Never used!*/
#if 0
typedef struct lm_ctl_s{
  char *classInfoFn;
  char **gramfnlist;
  char **gramlist;
} lm_ctl_t;
#endif

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
    
    s3lmwid_t startlwid;	/* S3_START_WORD id, if it exists */
    s3lmwid_t finishlwid;	/* S3_FINISH_WORD id, if it exists */
    
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
    
    /*
     * <w0,w1,w2> hashed to an entry into this array.  Only the last trigram mapping to any
     * given hash entry is kept in that entry.  (The cache doesn't have to be super-efficient.)
     */
    lm_tgcache_entry_t *tgcache;
    
    /* Statistics */
    int32 n_bg_fill;    /* #bg fill operations */
    int32 n_bg_inmem;   /* #bg in memory */
    int32 n_bg_score;   /* #bg_score operations */
    int32 n_bg_bo;	/* #bg_score ops backed off to ug */
    int32 n_tg_fill;	/* Similar stats for trigrams */
    int32 n_tg_inmem;
    int32 n_tg_score;
    int32 n_tg_bo;
    int32 n_tgcache_hit;
    
    int32 access_type;	/* Updated on every lm_{tg,bg,ug}_score call to reflect the kind of
			   n-gram accessed: 3 for 3-gram, 2 for 2-gram and 1 for 1-gram */

  /* 20040225 ARCHAN : Data structure to maintain dictionary information */
  /* Data structure for dictionary to LM words look up mapping */
  s3lmwid_t *dict2lmwid; 
  
  /* Data structure to maintain the class information */
  int32 dict_size;	/* #words in lexicon */
  /* Data structure that maintains the class information */
  lmclass_t *lmclass;
  int32 n_lmclass;
  int32 *inclass_ugscore;
  
} lm_t;

/*20040222 ARCHAN: Structure for multiple, named LMs, copied from s2*/
typedef struct lmset_s {
  char *name;
  lm_t *lm;
} lmset_t;

/* Access macros; not meant for arbitrary use */
#define lm_lmwid2dictwid(lm,u)	((lm)->ug[u].dictwid)
#define lm_n_ug(lm)		((lm)->n_ug)
#define lm_n_bg(lm)		((lm)->n_bg)
#define lm_n_tg(lm)		((lm)->n_tg)
#define lm_wordstr(lm,u)	((lm)->wordstr[u])
#define lm_startwid(lm)		((lm)->startlwid)
#define lm_finishwid(lm)	((lm)->finishlwid)
#define lm_access_type(lm)	((lm)->access_type)


/* Generic structure that could be used at any n-gram level */
typedef struct {
    s3wid_t wid;	/* NOTE: dictionary wid; may be BAD_S3WID if not available */
    int32 prob;
} wordprob_t;


int32 lm_get_classid (lm_t *model, char *name);
/*
 * Read an LM (dump) file; return pointer to LM structure created.
 */
lm_t *lm_read (char *file,	/* In: LM file being read */
	       float64 lw,	/* In: Language weight */
	       float64 wip,	/* In: Word insertion penalty */
	       float64 uw);	/* In: Unigram weight (interpolation with uniform distr.) */

/*
 * Read the LM control file, also initialize kb->lm
 */

lmset_t* lm_read_ctl(char * ctlfile,/* Control file name */
		 dict_t* dict,  /* In: Dictionary */
		 float64 lw,	/* In: Language weight */
		 float64 wip,	/* In: Word insertion penalty */
		 float64 uw,    /* In: Unigram weight */
		 char* lmdumpdir, /* In: LMdumpdir */
		 int32* n_lm,    /* In/Out: number of LM */
		 int32* n_alloclm, /* In/Out: number of allocated LM */
		     int32 dict_size  /* In: dictionary size */
		     );	


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


#if 0 /*Obsolete and it will cause conflict the code, so comment for now*/
/*
 * Somewhat like lm_bglist, but fill up a wordprob_t array from the bigram list found, instead
 * of simply returning the bglist.  The wordprob array contains dictionary word IDs.  But note
 * that only the base IDs are entered; the caller is responsible for filling out the alternative
 * pronunciations.
 * Return value:  #entries filled in the wordprob array.
 */
int32 lm_bg_wordprob(lm_t *lm,		/* In: LM being queried */
		     s3lmwid_t w,	/* In: LM word ID of the 1-word history */
		     int32 th,		/* In: If a prob (logs3, langwt-ed) < th, ignore it */
		     wordprob_t *wp,	/* In/Out: Array to be filled; caller must have
					   allocated this array */
		     int32 *bowt);	/* Out: *bowt = backoff-weight associated with w */
#endif

/*
 * Like lm_bg_wordprob, but for unigrams.
 * Return value:  #entries filled in the wordprob array.
 */
int32 lm_ug_wordprob(lm_t *lm,
		     dict_t *dict,
		     int32 th,
		     wordprob_t *wp);

/* Return the unigrams in LM.  Return value: #unigrams in returned list. */
int32 lm_uglist (lm_t *lmp,	/* In: LM being queried */
		 ug_t **ug);	/* Out: *ug = unigram array */


/* Return unigram score for the given word */
/* 20040227: This also account the in-class probability of wid*/
int32 lm_ug_score (lm_t *lmp, s3lmwid_t lwid,s3wid_t wid);


/*
 * Return bigram score for the given two word sequence.  If w1 is BAD_S3LMWID, return
 * lm_ug_score (w2).
 * 20040227: This also account the in-class probability of w2. 
 */
int32 lm_bg_score (lm_t *lmp, s3lmwid_t lw1, s3lmwid_t lw2,s3wid_t w2);


/*
 * Return trigram score for the given three word sequence.  If w1 is BAD_S3LMWID, return
 * lm_bg_score (w2, w3).  If both lw1 and lw2 are BAD_S3LMWID, return lm_ug_score (lw3).
 * 
 * 20040227: This also account the in-class probability of w3. 
 */
int32 lm_tg_score (lm_t *lmp, s3lmwid_t lw1, s3lmwid_t lw2, s3lmwid_t lw3, s3wid_t w3);


/*
 * Set the language-weight and insertion penalty parameters for the LM, after revoking
 * any earlier set of such parameters.
 */
void lm_set_param (lm_t *lm, float64 lw, float64 wip);


/* Return LM word ID for the given string, or BAD_S3LMWID if not available */
s3lmwid_t lm_wid (lm_t *lm, char *wd);


/* LM cache related */
void lm_cache_reset (lm_t *lmp);
void lm_cache_stats_dump (lm_t *lmp);

/* RAH, added code for freeing allocated memory */
void lm_free (lm_t *lm);



/* Macro versions of access functions */
#define LM_TGPROB(lm,tgptr)	((lm)->tgprob[(tgptr)->probid].l)
#define LM_BGPROB(lm,bgptr)	((lm)->bgprob[(bgptr)->probid].l)
#define LM_UGPROB(lm,ugptr)	((ugptr)->prob.l)
#define LM_RAWSCORE(lm,score)	((score - (lm)->wip) / ((lm)->lw))


#ifdef __cplusplus
}
#endif

#endif
