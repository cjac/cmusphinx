/*
 * lm.h - Wrapper for all language models under one integrated look (ideally!).
 *
 * HISTORY
 * 
 * 03-Apr-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _LM_H_
#define _LM_H_


/* LM scores, given sequences of dictionary base wid */
int32	lm_tg_score (int32 w1, int32 w2, int32 w3);
int32	lm_bg_score (int32 w1, int32 w2);
int32	lm_ug_score (int32 w);

/* One-time initialization of cache LM */
void lm_cache_lm_init ( void );


/*
 * Add a unigram (dictionary word id w) to cache LM (if doesn't exceed ugprob thresh).
 * The LM (and decoder) must be quiescent during this operation.
 */
void lm_cache_lm_add_ug (int32 w);


/*
 * Add a bigram (dictionary word id w1,w2) to cache LM.
 * The LM (and decoder) must be quiescent during this operation.
 */
void lm_cache_lm_add_bg (int32 w1, int32 w2);

void lm_cache_lm_dump (char *file);
void lm_cache_lm_load (char *file);


#endif
