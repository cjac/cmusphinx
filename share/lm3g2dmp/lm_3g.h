/*
 * lm_3g.h - darpa standard trigram language model header file
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
 * 02-Apr-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added lm3g_raw_score() and lm_t.invlw.
 * 		Changed lm_{u,b,t}g_score to lm3g_{u,b,t}g_score.
 * 
 * 01-Jul-95	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added tginfo_t to help speed up find_bg() and find_tg() searches.
 * 
 * $Log$
 * Revision 1.1  2002/11/11  17:42:40  egouvea
 * Initial import of lm3g2dmp from Ravi's files.
 * 
 * Revision 1.1.1.1  2000/02/28 18:34:44  rkm
 * Imported Sources
 *
 * Revision 6.5  93/10/27  17:47:04  rkm
 * *** empty log message ***
 * 
 * Revision 6.4  93/10/15  15:02:39  rkm
 * *** empty log message ***
 * 
 */

#ifndef _LM_3G_H_
#define _LM_3G_H_


#include "primtype.h"
#include "hash.h"


/* log quantities represented in either floating or integer format */
typedef union {
    float f;
    int32 l;
} log_t;

typedef struct unigram_s {
    int32 wid;		/* dict word-id */
    log_t prob1;
    log_t bo_wt1;
    int32 bigrams;	/* index of 1st entry in lm_t.bigrams[] */
} unigram_t, UniGram, *UniGramPtr;

/*
 * To conserve space, bigram info is kept in many tables.  Since the number
 * of distinct values << #bigrams, these table indices can be 16-bit values.
 * prob2 and bo_wt2 are such indices, but keeping trigram index is less easy.
 * It is supposed to be the index of the first trigram entry for each bigram.
 * But such an index cannot be represented in 16-bits, hence the following
 * segmentation scheme: Partition bigrams into segments of BG_SEG_SZ
 * consecutive entries, such that #trigrams in each segment <= 2**16 (the
 * corresponding trigram segment).  The bigram_t.trigrams value is then a
 * 16-bit relative index within the trigram segment.  A separate table--
 * lm_t.tseg_base--has the index of the 1st trigram for each bigram segment.
 */
#define BG_SEG_SZ	512	/* chosen so that #trigram/segment <= 2**16 */
#define LOG_BG_SEG_SZ	9

typedef struct bigram_s {
    u_int16 wid;	/* dict word-id */
    u_int16 prob2;	/* index into array of actual bigram probs */
    u_int16 bo_wt2;	/* index into array of actual bigram backoff wts */
    u_int16 trigrams;	/* index of 1st entry in lm_t.trigrams[],
			   RELATIVE TO its segment base (see above) */
} bigram_t, BiGram, *BiGramPtr;

/*
 * As with bigrams, trigram prob info kept in a separate table for conserving
 * memory space.
 */
typedef struct trigram_s {
    u_int16 wid;	/* dict word-id */
    u_int16 prob3;	/* index into array of actual trigram probs */
} trigram_t;


/*
 * The following trigram information cache eliminates most traversals of 1g->2g->3g
 * tree to locate trigrams for a given bigram (lw1,lw2).  The organization is optimized
 * for locality of access (to the same lw1), given lw2.
 */
typedef struct tginfo_s {
    int32 w1;			/* lw1 component of bigram lw1,lw2.  All bigrams with
				   same lw2 linked together (see lm_t.tginfo). */
    int32 n_tg;			/* #tg for parent bigram lw1,lw2 */
    int32 bowt;                 /* tg bowt for lw1,lw2 */
    int32 used;			/* whether used since last lm_reset */
    trigram_t *tg;		/* Trigrams for lw1,lw2 */
    struct tginfo_s *next;      /* Next lw1 with same parent lw2; NULL if none. */
} tginfo_t;


/*
 * The language model.
 * Bigrams for each unigram are contiguous.  Bigrams for unigram i+1 come
 * immediately after bigrams for unigram i.  So, no need for a separate count
 * of bigrams/unigram; it is enough to know the 1st bigram for each unigram.
 * But an extra dummy unigram entry needed at the end to terminate the last
 * real entry.
 * Similarly, trigrams for each bigram are contiguous and trigrams for bigram
 * i+1 come immediately after trigrams for bigram i, and an extra dummy bigram
 * entry is required at the end.
 */
typedef struct lm_s {
    unigram_t *unigrams;
    bigram_t  *bigrams;	/* for entire LM */
    trigram_t *trigrams;/* for entire LM */
    log_t *prob2;	/* table of actual bigram probs */
    int32 n_prob2;	/* prob2 size */
    log_t *bo_wt2;	/* table of actual bigram backoff weights */
    int32 n_bo_wt2;	/* bo_wt2 size */
    log_t *prob3;	/* table of actual trigram probs */
    int32 n_prob3;	/* prob3 size */
    int32 *tseg_base;	/* tseg_base[i>>LOG_BG_SEG_SZ] = index of 1st
			   trigram for bigram segment (i>>LOG_BG_SEG_SZ) */
    int32 *dictwid_map; /* lexicon word-id to ILM word-id map */
    int32 max_ucount;	/* To which ucount can grow with dynamic addition of words */
    int32 ucount;	/* #unigrams in LM */
    int32 bcount;	/* #bigrams in entire LM */
    int32 tcount;	/* #trigrams in entire LM */
    int32 dict_size;	/* #words in lexicon */

    double lw;          /* language weight */
    double invlw;       /* 1.0/language weight */
    double uw;          /* unigram weight */
    int32 log_wip;      /* word insertion penalty */
    
    tginfo_t **tginfo;	/* tginfo[lw2] is head of linked list of trigram information for
			   some cached subset of bigrams (*,lw2). */
    
    hash_t HT;		/* hash table for word-string->word-id map */
} lm_t, *LM;

#define UG_WID(m,u)		((m)->unigrams[u].wid)
#define UG_PROB_F(m,u)		((m)->unigrams[u].prob1.f)
#define UG_BO_WT_F(m,u)		((m)->unigrams[u].bo_wt1.f)
#define UG_PROB_L(m,u)		((m)->unigrams[u].prob1.l)
#define UG_BO_WT_L(m,u)		((m)->unigrams[u].bo_wt1.l)
#define FIRST_BG(m,u)		((m)->unigrams[u].bigrams)
#define LAST_BG(m,u)		(FIRST_BG((m),(u)+1)-1)
#define DICT2LM_WID(m,d)	((m)->dictwid_map[d])

#define BG_WID(m,b)		((m)->bigrams[b].wid)
#define BG_PROB_F(m,b)		((m)->prob2[(m)->bigrams[b].prob2].f)
#define BG_BO_WT_F(m,b)		((m)->bo_wt2[(m)->bigrams[b].bo_wt2].f)
#define BG_PROB_L(m,b)		((m)->prob2[(m)->bigrams[b].prob2].l)
#define BG_BO_WT_L(m,b)		((m)->bo_wt2[(m)->bigrams[b].bo_wt2].l)
#define TSEG_BASE(m,b)		((m)->tseg_base[(b)>>LOG_BG_SEG_SZ])
#define FIRST_TG(m,b)		(TSEG_BASE((m),(b))+((m)->bigrams[b].trigrams))
#define LAST_TG(m,b)		(FIRST_TG((m),(b)+1)-1)

#define TG_WID(m,t)		((m)->trigrams[t].wid)
#define TG_PROB_F(m,t)		((m)->prob3[(m)->trigrams[t].prob3].f)
#define TG_PROB_L(m,t)		((m)->prob3[(m)->trigrams[t].prob3].l)

#define ILLEGAL_WID		65535

/* ----Interface---- */

void	lmSetStartSym (/* char *sym */);
void	lmSetEndSym (/* char *sym */);
lm_t *	NewModel (/* int32 n_ug, n_bg, n_tg, n_dict */);
int32   lm_add_word (lm_t *model, int32 dictwid);
void	lm_add (/* char *name, lm_t *model, double lw, double uw, double wip */);
int32	lm_set_current (char *name);
lm_t *	lm_get_current ();
char *	get_current_lmname ();
lm_t *  lm_name2lm (char *name);
int32	get_n_lm ();
int32	dictwd_in_lm (/* int32 wid */);

int32	lm3g_tg_score (int32 w1, int32 w2, int32 w3);
int32	lm3g_bg_score (int32 w1, int32 w2);
int32	lm3g_ug_score (int32 w);
int32	lm3g_raw_score (int32 score);

void	lm3g_cache_reset ();
void	lm3g_cache_stats_dump ();
void	lm_next_frame ();

#endif /* _LM_3G_H_ */
