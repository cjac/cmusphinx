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
 * $Log$
 * Revision 1.12  2005/06/21  22:24:02  arthchan2003
 * Log. In this change, I introduced a new interface for lm ,which is
 * call lmset_t. lmset_t wraps up multiple lm, n_lm, n_alloclm into the
 * same structure and handle LM initialization (lm_init) switching,
 * (lmset_curlm_widx), delete LM (lmset_delete_lm).  The internal
 * structure is called lmarray and is an array of pointers of lm.  The
 * current lm is always maintained and pointed by a pointer called cur_lm
 * . This substantially clarify the structure of the code.  At this
 * check-in, not every core function of lmset is completed.
 * e.g. lmset_add_lm because that required testing of several LM reading
 * routines and could be quite time-consuming.
 * 
 * Log. Another notable change is the fact dict2lmwid map is started to
 * be part of the LM. The reason of this is clearly described inside the
 * code. Don't want to repeat here.
 * 
 * Log. The new interface has been already used broadly in both Sphinx
 * 3.0 and sphinx 3.x family of tools.
 * 
 * Revision 1.5  2005/06/18 03:22:28  archan
 * Add lmset_init. A wrapper function of various LM initialization and initialize an lmset It is now used in decode, livepretend, dag and astar.
 *
 * Revision 1.4  2005/06/17 23:44:40  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.3  2005/06/13 04:02:59  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.2  2005/05/10 21:21:54  archan
 * Three functionalities added but not tested. Code on 1) addition/deletion of LM in mode 4. 2) reading text-based LM 3) Converting txt-based LM to dmp-based LM.
 *
 * Revision 1.1  2005/05/04 06:08:07  archan
 * Refactor all lm routines except fillpen.c into ./libs3decoder/liblm/ . This will be equivalent to ./lib/liblm in future.
 *
 * Revision 1.6  2005/05/04 04:02:24  archan
 * Implementation of lm addition, deletion in (mode 4) time-switching tree implementation of search.  Not yet tested. Just want to keep up my own momentum.
 *
 * Revision 1.5  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.4  2005/04/20 03:37:59  archan
 * LM code changes: functions are added to set, add and delete LM from the lmset, change the legacy lmset data structure to contain n_lm and n_alloc_lm.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
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

#define LM_DICTWID_BADMAP	-16000		/** An illegal mapping */
#define LM_CLASSID_BASE		0x01000000	/** Interpreted as LMclass ID */
#define LM_CLASSID_TO_CLASS(m,i)	((m)->lmclass[(i)-LM_CLASSID_BASE])
#define MIN_PROB_F -99.0
#define LM_ALLOC_BLOCK 16 

#define LM_NOT_FOUND  -1 /** Constant which indicate an LM couldn't be found */
#define LM_SUCCESS 1 /** Constant that indicates an operation succeed */
#define LM_FAIL 0 /** Constant that define an operation failed.  */

#define NO_WORD	-1

#include "s3types.h"
#include "lmclass.h"
#include "dict.h"

  /** \file lm.h
     \brief Language model

   */

  
  /** Log quantities represented in either floating or integer format */
  typedef union {
    float32 f;
    int32 l;
  } lmlog_t;


  /*
   * ARCHAN 20050503: comment copied from Sphinx 2
   * Bigram probs and bo-wts, and trigram probs are kept in separate tables
   * rather than within the bigram_t and trigram_t structures.  These tables
   * hold unique prob and bo-wt values, and can be < 64K long (see lm_3g.h).
   * The following tree structure is used to construct these tables of unique
   * values.  Whenever a new value is read from the LM file, the sorted tree
   * structure is searched to see if the value already exists, and inserted
   * if not found.
   */

  typedef struct sorted_entry_s {
    lmlog_t val;		/** value being kept in this node */
    uint16 lower;	/** index of another entry.  All descendants down
			   this path have their val < this node's val.
			   0 => no son exists (0 is root index) */
    uint16 higher;	/** index of another entry.  All descendants down
			   this path have their val > this node's val
			   0 => no son exists (0 is root index) */
  } sorted_entry_t;

  /*
   * The sorted list.  list is a (64K long) array.  The first entry is the
   * root of the tree and is created during initialization.
   */
  typedef struct {
    sorted_entry_t *list;
    int32 free;		/** first free element in list */
  } sorted_list_t;

typedef struct {
    s3wid_t dictwid;	/** Dictionary word id, or BAD_S3WID if unknown.  However, the LM
			   module merely sets this field to BAD_S3WID.  It is upto the
			   application to fill in this field (HACK!!), so that this
			   module can be independent of a dictionary. */
    lmlog_t prob;
    lmlog_t bowt;
    int32 firstbg;	/** 1st bigram entry on disk */
} ug_t;

typedef struct {
    s3lmwid_t wid;	/** LM wid (index into lm_t.ug) */
    uint16 probid;
    uint16 bowtid;
    uint16 firsttg;     /** 1st trigram entry on disk (see tg_segbase below) */
} bg_t;

typedef struct {
    s3lmwid_t wid;	/** LM wid (index into lm_t.ug) */
    uint16 probid;
} tg_t;


  /**
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

/* 
 * A note on lm/dict/dict2lm.   -ARCHAN 20050616
 * 
 * In older versions of sphinx3 (<s3.4). dict2lm is a separate object
 * from lm and dict.  A kb actually owns a dict2lm so programer will
 * read the lm.  This seprates the initalization of lm and dict2lm and
 * it makes a lot of sense if there is **only one** lm and **only one
 * dict2lm. 
 * 
 * However, when multiple LMs and switching of them is required.
 * Then, the problem of the above architecture starts to show up.  For
 * example, 
 *  lmset=lm_read_ctl ();
 *  for(i=0;i<kb->n_lm;i++){
 *   dict2lmwid[i]=wid_dict_lm_map
 *  }
 * At the same time, one will also have an array of lms (lmset[i]) for 
 * corresponding dict2lm[i]!
 *
 * Of course, having multiple arrays of things will somedays caused problems.  
 *
 * The resolution is that we observed that the dict2lm map mostly changed when the lm 
 * needs to change. Also, the fact that the dictionary pronounciation itself seldom
 * changes. That is partially caused by the fact we don't have too much research on 
 * So at the end, that is why it makes sense to let the lm to own a dict2lm. 
 * 
 * What if we also allow the dictionary to change? That is a tough
 * question.  In that case perhaps, we should still inventory of sets
 * of lm and dict2lm and allow lm to store a pointer of dict2lm.  Once
 * there are changes in dict, programmer will be responsible to update
 * dict2lm. (Storing pointers will allow programmers not to update
 * everything but just lms corresponding to a particular dict.)  I
 * guess in that case it will be sign of having a wrapper that control
 * both lm and dict together.
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

  /**
 * The language model.
 * All unigrams are read into memory on initialization.
 * Bigrams and trigrams read in on demand.
 */
typedef struct lm_s {
  char *name ;
    int32 n_ug;         /** #unigrams in LM */
    int32 n_bg;         /** #bigrams in entire LM */
    int32 n_tg;         /** #trigrams in entire LM */
    int32 max_ug;       /** To which n_ug can grow with dynamic addition of words */
    
    char **wordstr;	/** The LM word list (in unigram order) */
    
    s3lmwid_t startlwid;	/* S3_START_WORD id, if it exists */
    s3lmwid_t finishlwid;	/* S3_FINISH_WORD id, if it exists */
    
    int32 log_bg_seg_sz;/** See big comment above */
    int32 bg_seg_sz;
    
    ug_t *ug;           /** Unigrams */
    bg_t *bg;		/** NULL iff disk-based */
    tg_t *tg;		/** NULL iff disk-based */
    membg_t *membg;	/** membg[w1] = bigrams for lm wid w1 (used iff disk-based) */
    tginfo_t **tginfo;	/** tginfo[w2] = fast trigram access info for bigrams (*,w2) */
    
    lmlog_t *bgprob;    /** Table of actual bigram probs */
    lmlog_t *tgprob;    /** Table of actual trigram probs */
    lmlog_t *tgbowt;    /** Table of actual trigram backoff weights */
    int32 *tg_segbase;  /** tg_segbase[i>>lm_t.log_bg_seg_sz] = index of 1st
			   trigram for bigram segment (i>>lm_t.log_bg_seg_sz) */
    int32 n_bgprob;
    int32 n_tgprob;
    int32 n_tgbowt;

    FILE *fp;
    int32 byteswap;     /** Whether this file is in the WRONG byte order */
    int32 bgoff;        /** BG and TG offsets into DMP file (used iff disk-based) */
    int32 tgoff;

    float32 lw;		/** Language weight currently in effect for this LM */
    int32 wip;          /** logs3(word insertion penalty) in effect for this LM */
    
  /**
     * <w0,w1,w2> hashed to an entry into this array.  Only the last trigram mapping to any
     * given hash entry is kept in that entry.  (The cache doesn't have to be super-efficient.)
     */
    lm_tgcache_entry_t *tgcache;
    
    /* Statistics */
    int32 n_bg_fill;    /** #bg fill operations */
    int32 n_bg_inmem;   /** #bg in memory */
    int32 n_bg_score;   /** #bg_score operations */
    int32 n_bg_bo;	/** #bg_score ops backed off to ug */
    int32 n_tg_fill;	/** Similar stats for trigrams */
    int32 n_tg_inmem;
    int32 n_tg_score;
    int32 n_tg_bo;
    int32 n_tgcache_hit;
    
    int32 access_type;	/** Updated on every lm_{tg,bg,ug}_score call to reflect the kind of
			   n-gram accessed: 3 for 3-gram, 2 for 2-gram and 1 for 1-gram */



  hash_table_t HT;		/**  hash table for word-string->word-id map */

  /* 20040225 ARCHAN : Data structure to maintain dictionary information */
  /* Data structure for dictionary to LM words look up mapping */
  s3lmwid_t *dict2lmwid; /** a mapping from dictionary word to LM word */
  
  /* Data structure to maintain the class information */
  int32 dict_size;	/** CURRENTLY NOT USED: #words in lexicon */
  /* Data structure that maintains the class information */
  lmclass_t *lmclass;   /** LM class for this LM */
  int32 n_lmclass;      /** # LM class */
  int32 *inclass_ugscore;

  char **word_str;   /** Temporary Variable: Words in LM; used only for building internal LM from LM file */

  /* Arrays of unique bigram probs and bo-wts, and trigram probs */
  sorted_list_t sorted_prob2; /** Temporary Variable: Sorted list */
  sorted_list_t sorted_bowt2; /** Temporary Variable: Sorted list */
  sorted_list_t sorted_prob3; /** Temporary Variable: Sorted list */

} lm_t;



  /** Structure for multiple, named LMs, started from s2*/
typedef struct lmset_s {
  lm_t **lmarray;  /** 1 dimensional array of pointers of lm_t */
  lm_t *cur_lm; /** TEMPORARY VARIABLE: The current LM */

  int32 cur_lm_idx; /** TEMPORARY VARIABLE : The current LM index */
  int32 n_lm;       /** number of LM */
  int32 n_alloc_lm; /** number of allocated LM */
} lmset_t;

  /** Access macros; not meant for arbitrary use */
#define lm_lmwid2dictwid(lm,u)	((lm)->ug[u].dictwid)
#define lm_n_ug(lm)		((lm)->n_ug)
#define lm_n_bg(lm)		((lm)->n_bg)
#define lm_n_tg(lm)		((lm)->n_tg)
#define lm_wordstr(lm,u)	((lm)->wordstr[u])
#define lm_startwid(lm)		((lm)->startlwid)
#define lm_finishwid(lm)	((lm)->finishlwid)
#define lm_access_type(lm)	((lm)->access_type)


  /** Generic structure that could be used at any n-gram level */
typedef struct {
    s3wid_t wid;	/* NOTE: dictionary wid; may be BAD_S3WID if not available */
    int32 prob;
} wordprob_t;


  /** 
      Get class ID given a LM. 
   */
  int32 lm_get_classid (lm_t *model, char *name);
  
  /**
   * Read an LM (dump) file; return pointer to LM structure created.
   */
  lm_t *lm_read (const char *file,	/**< In: LM file being read */
		 float64 lw,	/**< In: Language weight */
		 float64 wip,	/**< In: Word insertion penalty */
		 float64 uw	/**< In: Unigram weight (interpolation with uniform distr.) */
		 );


  /**
   * Read an LM (txt) file  (Not Regression Tested)
   */
  lm_t *lm_read_txt(const char *filename, /**<In: LM file being read */
		    const char *lmname,   /**<In: LM name*/
		    float64 lw,           /**<In: Language weight */
		    float64 wip,          /**<In: word insertion penalty */
		    float64 uw            /**<In: Unigram weight (interpolation with uniform distr.) */
		    );


  /** A wrapper function of controlling the behavior of LM initialization 
   * 
   * (ARCHAN 20050617) lmset_init controls the behavior how the lmset
   * which is an array of lm was initialized by different command-line
   * arguments.  lmfile and lmctlfile are mutually exclusive.  Each
   * will invoke one reading functions.  
   * 
   * In the case of -lmfile is specified.  A lmset with one single lm
   * (or lmset->n_lm=1) will be returned. The lm's name is called
   * "default".
   *
   * In the case of -lmctlfile is specified. A lmset with multiple lms
   * will be returned. The number of lm will depend on the number of
   * lm specified by -lmctlfile.  For the format, please read the
   * current format of -lmctlfile in lm.c
   *
   * ctl_lm is the equivalent of -ctl for lm.  When -ctl_lm is not
   * specified in command-line (ctl_lm is NULL). Then either lm with
   * name lmname will be used as the default lm.  If lmname is NULL, then
   * the first lm will be as the default
   *
   * lmdumpdir is currently not used. It is there for backward
   * compatibility purpose. 
   *
   * lw,wip,uw are language weight, word insertion pernalty and
   * unigram weight. Their values are crucial to computation of the
   * language model score. Therefore, the programmer is urged to
   * carefully set these three values and also be careful of the 
   * order. 
   * 
   * dict is assumed to be a pre-initialized dict_t structure which is
   * used in deriving the mapping between the dictionary word and the
   * lm words
   *
   */

  lmset_t* lmset_init(char* lmfile,  /**< The lm file name, lmfile and lmctlfile are mutally exclusive */
		      char* lmctlfile, /**< The file that specified multiple LMs and class information, lmfile and lmctlfile are mutually exclusive */
		      char* ctl_lm,    /**< The control file that describes which lm to use for a particular utterance*/
		      char* lmname,    /**< The LM name to use if ctl_lm is not specified  */
		      char* lmdumpdir, /**< Currently not used */
		      float32 lw,      /**< Language model weight */
		      float32 wip,     /**< Word insertion penalty */
		      float32 uw,      /**< Unigram weight */
		      dict_t *dict     /**< A pre-initialized dict_t structure */
		      );


  /* It is still a sore point: To have two interfaces for two different
     type of input.  Some of the code is still duplicated.  Changing
     one doesn't the other one will be changed
   */
  /** 
   *  Read a single LM into the lmset. 
   */
  lmset_t* lmset_read_lm(const char *lmfile, /**< In: The LM file */
			 dict_t *dict,       /**< In: A pre-initialized dictionary file*/
			 const char *lmname, /**< In: The LM name */
			 float64 lw,         /**< The language weight */
			 float64 wip,        /**< The word insertion penalty */
			 float64 uw          /**< The unigram weight */
			 );

  /**
   * Read the LM control file, also initialize kb->lm
   */

  lmset_t* lmset_read_ctl(const char * ctlfile,/**< Control file name */
			  dict_t* dict,  /**< In: Dictionary */
			  float64 lw,	/**< In: Language weight */
			  float64 wip,	/**< In: Word insertion penalty */
			  float64 uw,    /**< In: Unigram weight */
			  char* lmdumpdir, /**< In: LMdumpdir */
			  int32 dict_size  /**< In: dictionary size */
			  );	

  /**
   * Get an LM by index. 
   */
  lm_t* lmset_get_lm_widx(lmset_t *lms,  /**< In: The set of LM */
			  int32 lmidx    /**< In: LM index */
			  );

  /**
   * Get an LM by name
   * @return a pointer of the LM with name lmname
   */
  lm_t* lmset_get_lm_wname(lmset_t *lms,  /**< In: The set of LM */
			   const char *lmname   /**< In: The LM name */
			   );

  /**
   * Set the current LM with index 
   */
  void lmset_set_curlm_widx(lmset_t *lms, /**< In: The set of LM */
		       int32 lmidx   /**< In: LM index */
		       );

  /**
   * Set the current LM with name
   */
  void lmset_set_curlm_wname(lmset_t *lms, /**< In: The set of LM */
			const char *lmname   /**< In: The LM name */
			);
  
  /**
   * Convert name to index
   */
  int32 lmset_name_to_idx(lmset_t *lms, /**< In: The set of LM */
			  const char *lmname /**< In: The LM name */
			  );

  /**
   * Convert index to name
   * @return a pointer of
   */

  char* lmset_idx_to_name(lmset_t *lms, /**< In: The set of LM */
			  int32 lmidx /**< In: LM index */
			  );


  /** 
   * Add a new lm into the lmset. Notice that lms->n_lm will be added by 1
   */
  
  void lmset_add_lm(lmset_t *lms,  /**< In/Out : The set of LM */
		    lm_t *lm,      /**< In : The input LM */
		    const char* lmname /**< In: The lm name */
		    );

  /**
   * Delete a LM with lmname. Notice that lms->n_lm will be subtracted by 1
   */
  
  void lmset_delete_lm(lmset_t *lms, /**< In/Out : The set of LM */
		       const char *lmname /**< The lm name */
		       );

  /**
   * Free the lmset data structure
   */
  void lmset_free(lmset_t *lms /**< In: The set of LM */
		  );

  /**
   * Return trigram followers for given two words.  Both w1 and w2 must be valid.
   * Return value: #trigrams in returned list.
   */
int32 lm_tglist (lm_t *lmp,	/**< In: LM being queried */
		 s3lmwid_t w1,	/**< In: LM word id of the first of a 2-word history */
		 s3lmwid_t w2,	/**< In: LM word id of the second of the 2-word history */
		 tg_t **tg,	/**< Out: *tg = array of trigrams for <w1,w2> */
		 int32 *bowt	/**< Out: *bowt = backoff-weight for <w1, w2> */
		 );

  /**
 * Return the bigram followers for the given word w.
 * Return value: #bigrams in returned list.
 */
int32 lm_bglist (lm_t *lmp,	/**< In: LM being queried */
		 s3lmwid_t w,	/**< In: LM word id of the 1-word history */
		 bg_t **bg,	/**< Out: *bg = array of bigrams for w */
		 int32 *bowt	/**< Out: *bowt = backoff-weight for w */
		 );


#if 0 /*Obsolete and it will cause conflict the code, so comment for now*/
/*
 * Somewhat like lm_bglist, but fill up a wordprob_t array from the bigram list found, instead
 * of simply returning the bglist.  The wordprob array contains dictionary word IDs.  But note
 * that only the base IDs are entered; the caller is responsible for filling out the alternative
 * pronunciations.
 * Return value:  #entries filled in the wordprob array.
 */
int32 lm_bg_wordprob(lm_t *lm,		/**< In: LM being queried */
		     s3lmwid_t w,	/**< In: LM word ID of the 1-word history */
		     int32 th,		/**< In: If a prob (logs3, langwt-ed) < th, ignore it */
		     wordprob_t *wp,	/**< In/Out: Array to be filled; caller must have
					   allocated this array */
		     int32 *bowt	/**< Out: *bowt = backoff-weight associated with w */
		     );
#endif

  /**
 * Like lm_bg_wordprob, but for unigrams.
 * Return value:  #entries filled in the wordprob array.
 */
  int32 lm_ug_wordprob(lm_t *lm, /**< In: LM being queried */
		       dict_t *dict, /**< In : The dictionary */
		     int32 th,
		       wordprob_t *wp /**< In/out: Array to be filled */
		     );

  /** Return the unigrams in LM.  Return value: #unigrams in returned list. */
int32 lm_uglist (lm_t *lmp,	/**< In: LM being queried */
		 ug_t **ug	/**< Out: *ug = unigram array */
		 );


/* Return unigram score for the given word */
/* 20040227: This also account the in-class probability of wid*/
int32 lm_ug_score (lm_t *lmp, 
		   s3lmwid_t lwid,
		   s3wid_t wid);


/*
 * Return bigram score for the given two word sequence.  If w1 is BAD_S3LMWID, return
 * lm_ug_score (w2).
 * 20040227: This also account the in-class probability of w2. 
 */
int32 lm_bg_score (lm_t *lmp, 
		   s3lmwid_t lw1, 
		   s3lmwid_t lw2,
		   s3wid_t w2);


/*
 * Return trigram score for the given three word sequence.  If w1 is BAD_S3LMWID, return
 * lm_bg_score (w2, w3).  If both lw1 and lw2 are BAD_S3LMWID, return lm_ug_score (lw3).
 * 
 * 20040227: This also account the in-class probability of w3. 
 */
int32 lm_tg_score (lm_t *lmp, 
		   s3lmwid_t lw1, 
		   s3lmwid_t lw2, 
		   s3lmwid_t lw3, 
		   s3wid_t w3);


/*
 * Set the language-weight and insertion penalty parameters for the LM, after revoking
 * any earlier set of such parameters.
 */
void lm_set_param (lm_t *lm, float64 lw, float64 wip);


int32 lm_rawscore (lm_t *lm, int32 score, float64 lwf);

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
#define LM_DICTWID(lm,lmwid)     ((lm)->ug[(lmwid)].dictwid)

#ifdef __cplusplus
}
#endif

#endif
