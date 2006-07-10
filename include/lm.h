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
 * $Log: lm.h,v $
 * Revision 1.16  2006/03/02 22:10:36  arthchan2003
 * Add *g_write into the code.
 *
 * Revision 1.15  2006/02/28 22:26:51  egouvea
 * Moved definition of lm_wid() outside of the #if 0/#endif block, so
 * it's declared.
 *
 * Revision 1.14  2006/02/24 13:38:08  arthchan2003
 * Added lm_read, it is a simple version of lm_read_advance.
 *
 * Revision 1.13  2006/02/23 04:16:29  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH:
 * Splited the original lm.c into five parts,
 * a, lm.c - a controller of other subroutines.
 * b, lm_3g.c - implement TXT-based lm operations
 * c, lm_3g_dmp.c - implement DMP-based lm operations
 * d, lm_attfsm.c - implement FSM-based lm operations
 * e, lmset.c - implement sets of lm.
 *
 * Revision 1.12.4.3  2006/01/16 19:56:37  arthchan2003
 * 1, lm_rawscore doesn't need a language weight, 2, Support dumping the LM in FST format.  This code used Yannick Esteve's and LIUM code.
 *
 * Revision 1.12.4.2  2005/11/17 06:15:22  arthchan2003
 * Added input-encoding and output-encoding into the lm structure.
 *
 * Revision 1.12.4.1  2005/07/13 01:46:22  arthchan2003
 * 1, Fixed dox-doc, 2, Added more documentation for major functions such as lm_read and lm_write.
 *
 * Revision 1.12  2005/06/21 22:24:02  arthchan2003
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
#if 0
} /* Fool Emacs into not indenting things. */
#endif

#define LM_DICTWID_BADMAP	-16000		/** An illegal mapping */
#define LM_CLASSID_BASE		0x01000000	/** Interpreted as LMclass ID */

/** Upper limit of the words of Sphinx 3.X */
#define LM_LEGACY_CONSTANT      BAD_S3LMWID          /**< =65535 (~65k), this is introduced 
							since 1996 when Ravi first wrote Sphinx 3.0. It
							was with us since. 
						     */

#define LM_SPHINX_CONSTANT      BAD_S3LMWID32      /**< (4 billion), ARCHAN: this is introduced by in Sphinx 3.6
						      during the time of Release Candidate I (2006 March). The caveat of using
						      this constant is that it is much hard to detect byte-swapping problem.
						      in general. Also, if the world has more than 10000 cities, each has 1 million
						      roads name. We are stuck in this case. I assume this will happen in 
						      year3001. 
						   */


#define LM_CLASSID_TO_CLASS(m,i)	((m)->lmclass[(i)-LM_CLASSID_BASE])

#define MIN_PROB_F       -99.0  /**< The minimum value of probabilities and
                                   backoff weights. When changing, notice
                                   that both s2 and s3 may transform this 
                                   number to very small integer (say -2e-31)
                                   This will easily cause integer wrap 
                                   around.  -99 is chosen for that reason. 
                                */

#define LM_ALLOC_BLOCK      16  /** The number of LMs to allocate at a time.                                    
                                 */

/**
   Sucess and error message. 
 */
#define LM_SUCCESS           1  /**< Constant that indicates an operation succeed 
                                 */
#define LM_FAIL              0  /**< Constant that define an operation failed.  */
#define LM_NOT_FOUND        -1  /**< Constant which indicate an LM couldn't be 
                                   found */
#define LM_OFFSET_TOO_LARGE -2  /**< Constant where the 16 bit LM was
                                   used, but th tgcount is larger than
                                   LM_LEGACY_CONSTANT (65535). This
                                   breaks addressing scheme in the
                                   current LM.
                                */
#define LM_NO_DATA_MARK     -3  /**< When reading text-based LM,
                                   return thisif we see no data
                                   mark  */
#define LM_UNKNOWN_NG       -4  /**< When reading the header of LM, if
                                   there is unknown K for K-gram */
#define LM_BAD_LM_COUNT     -5  /**< When reading LM, if count is bad,
                                   return this msg */
#define LM_UNKNOWN_WORDS    -6  /**< When an unknown word is found
                                   during LM readin, return this
                                   message */
#define LM_BAD_BIGRAM       -7  /**< A bad bigram, it could be word
                                   ids larger than # of unigram, it
                                   could be word id smaller than 0.
                                   It could also be bigram out of
                                   bound.
                                */
#define LM_BAD_TRIGRAM      -8  /**< A bad trigram, it could be word
                                   ids larger than # of unigram, it
                                   could be word id smaller than 0.
                                   It could also be bigram out of
                                   bound.
                                */
#define LM_BAD_QUADGRAM     -9  /**< (RESERVED BUT NOT USED) A bad
                                   quadgram (4-gram), it could be word
                                   ids larger than # of unigram, it
                                   could be word id smaller than 0.
                                   It could also be bigram out of
                                   bound.
                                */
#define LM_BAD_QUINGRAM     -10  /**< (RESERVED BUT NOT USED) A bad
                                    quingram (5-gram), it could be
                                    word ids larger than # of unigram,
                                    it could be word id smaller than
                                    0.  It could also be bigram out of
                                    bound.  BTW, there is no need to
                                    remind me the mixed use of
                                    quadgram and quingram is stupid
                                    English.  I read Manning and
                                    Schultze.
                                 */
#define LM_BAD_NGRAM       -11  /**< (RESERVED BUT NOT USED) A bad
                                   n-gram.  generalization of message
                                   -7 to -10. In our case, we don't
                                   make the message as specific as
                                   possible.
                                 */
#define LM_TOO_MANY_NGRAM  -12  /**< When reading LM, if the number of
                                   n-grams is more than the number
                                   specified header.  return this
                                   header */
#define LM_NO_MINUS_1GRAM  -13  /**< When reading n-gram, if the
                                   corresponding (n-1)-gram doesn't
                                   exists, return this message. */
#define LM_FILE_NOT_FOUND  -14  /**< When couldn't find the LM file,
                                   return this message */
#define LM_CANNOT_ALLOCATE -15  /**< When cannot allocate tables in LM 
                                   return this message */

/** Versioning of LM */
#define LMDMP_VERSIONNULL 0   /**< VERSION 0 is oldest, in the past, we
				 used to use the version number to
				 store the number of unigram, you will
				 see logic that said vn > LMDMP_VERSIONNULL				 
			      */
				 
#define LMDMP_VERSION_TG_16BIT -1 /**< VERSION 1 is the simplest DMP file which
				     is trigram or lower which used 16 bits in
				     bigram and trigram.*/

#define LMDMP_VERSION_TG_16BIT_V2 -2 /**< VERSION 2 means legacy VERSION 1 DMP file
					which has log_bg_seg_sz != 9*/

#define LMDMP_VERSION_TG_32BIT -3 /**< VERSION 3 is the 32 bit
				     extension of VERSION 1 but
				     the bigram and trigram are
				     represented by 32 bits data
				     structure */

#define LMTXT_VERSION         1000 /**< VERSION 1000 is the text-based LM */
#define LMFST_VERSION         1001 /**< VERSION 1001 is the FST-based LM */
#define LMFORCED_TXT32VERSION 1002 /**< VERSION 1002 is the internal version of
                                      text-based LM. The difference betwwen
                                      1002 and 1000 is that 1002 will assume
                                      LM is 32bits.  This fact is used in 
                                      lm_is32bits(lm)
                                   */


#define NO_WORD	-1

#include "s3types.h"
#include "lmclass.h"
#include "dict.h"
#include "encoding.h"

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

/** \file lm.h
    \brief Language model 

    This is the header file for language model support in Sphinx 3. 
    Sphinx 3 supports language model in 4 formats. The four formats are

    ARPA format: First appear in Sphinx 2. We port it to Sphinx 3 in
    3.X (X=6)

    DMP : Sphinx 3 slow and fast used it, so does later in Sphinx 3.X
    (X>4)

    DMP32 : We start to break the limit of number of words of
    65535. This is the first LM file format in Sphinx 3.X that could
    capture 4 billion words in the language model
     
    FST: In AT&T format, we start to support in 3.X (X=6).

    At 20060302
    we can only read and used ARPA, DMP-based format in the decoder. 
    we can write ARPA, DMP, DMP32 and FST file format. 
*/
  
/** \struct lmlog_t
    \brief Log quantities represented in either floating or integer format 
*/
typedef union {
    float32 f; /**< The floating point component */
    int32 l;   /**< The integer component */
} lmlog_t;



/** \struct sorted_entry_t
    \brief single entry used in the linked list structure of lm reading
*/
  
typedef struct sorted_entry_s {
    lmlog_t val;		/**< value being kept in this node */
    uint32 lower;	/**< index of another entry.  All descendants down
			   this path have their val < this node's val.
			   0 => no son exists (0 is root index) */
    uint32 higher;	/**< index of another entry.  All descendants down
			   this path have their val > this node's val
			   0 => no son exists (0 is root index) */
} sorted_entry_t;

/** \struct sorted_list_t
 *
 * \brief The sorted list used lm reading.  list is a (64K long) array.  The first entry is the root of the tree and is created during initialization.
 */
typedef struct {
    sorted_entry_t *list; /**< Beginnig of the list  */
    int32 free;		/**< first free element in list */
} sorted_list_t;

/** \struct ug_t
 * \brief A unigram structure
 * Please see 
 */
typedef struct {
    s3wid_t dictwid;	/**< Dictionary word id, or BAD_S3WID if unknown.  However, the LM
                           module merely sets this field to BAD_S3WID.  It is upto the
                           application to fill in this field (HACK!!), so that this
                           module can be independent of a dictionary. */
    lmlog_t prob;       /**< Unigram probability */
    lmlog_t bowt;
    int32 firstbg;	/**< 1st bigram entry on disk */
} ug_t;

/** \struct bg_t
 * \brief A bigram structure
 */

typedef struct {
    s3lmwid_t wid;	/**< LM wid (index into lm_t.ug) */
    uint16 probid;      /**< Index into array of actualy bigram probs*/
    uint16 bowtid;      /**< Index into array of actualy bigram backoff wts */
    uint16 firsttg;     /**< 1st trigram entry on disk (see tg_segbase below) */
} bg_t;


/** \struct bg32_t 
 * \brief A bigram structure which has 32 bits. 
 */
typedef struct {
    s3lmwid32_t wid;	/**< LM wid (index into lm_t.ug) */
    uint32 probid;      /**< Index into array of actualy bigram probs*/
    uint32 bowtid;      /**< Index into array of actualy bigram backoff wts */
    uint32 firsttg;     /**< 1st trigram entry on disk (see tg_segbase below) */
} bg32_t;


/** \struct tg_t
 * \brief A trigram structure
 */

typedef struct {
    s3lmwid_t wid;	/**< LM wid (index into lm_t.ug) */
    uint16 probid;      /**< Index into array of actualy trigram probs*/
} tg_t;


/** \struct tg32_t
 * \brief A 32 bits version of tg_t 
 */

typedef struct {
    s3lmwid32_t wid;	/**< LM wid (index into lm_t.ug) */
    uint32 probid;      /**< Index into array of actualy trigram probs*/
} tg32_t;


/** \struct membg_t
 *  \brief Management of in-memory bigrams.  Not used if all bigrams in memory.
 */
typedef struct {
    bg_t *bg;		/**< Bigrams for a specific unigram; see lm_t.membg */
    int32 used;		/**< Whether used since last lm_reset.  If not used, at the next
                           lm_reset bg are freed */
} membg_t;

/** \struct membg32_t
 *
 * \brief A 32 bits version of membg_t
 */
typedef struct {
    bg32_t *bg32;		/**< Bigrams for a specific unigram; see lm_t.membg */
    int32 used;		/**< Whether used since last lm_reset.  If not used, at the next
                           lm_reset bg are freed */
} membg32_t;


/**
 * \struct tginfo_t
 * \brief trigram cache that enhance locating trigram for a given bigram (w_1,w_2)
 *
 * The following trigram information cache eliminates most traversals of 1g->2g->3g
 * tree to locate trigrams for a given bigram (w1,w2).  The organization is optimized
 * for locality of access.  All bigrams (*,w2) for a given w2, for which trigrams have
 * been accessed "recently", form a linear linked list, pointed to by lm_t.tginfo[w2].
 * If disk-based, all trigrams for the given bg loaded upon request.  Cached info (and
 * tg if disk-based) freed at lm_reset if not used since last such reset.
 */
typedef struct tginfo_s {
    s3lmwid_t w1;		/**< w1 component of bigram w1,w2.  All bigrams with
				   same w2 linked together. */
    int32 n_tg;			/**< #tg for parent bigram w1,w2 */
    tg_t *tg;			/**< Trigrams for w1,w2 */
    int32 bowt;			/**< tg bowt for w1,w2 */
    int32 used;			/**< whether used since last lm_reset */
    struct tginfo_s *next;	/**< Next w1 with same parent w2 */
} tginfo_t;

/**
 * \struct tginfo32_t
 * \brief 32 bit version of tginfo
 *
 */
typedef struct tginfo32_s {
    s3lmwid32_t w1;		/**< w1 component of bigram w1,w2.  All bigrams with
				   same w2 linked together. */
    int32 n_tg;			/**< #tg for parent bigram w1,w2 */
    tg32_t *tg32;			/**< Trigrams for w1,w2 */
    int32 bowt;			/**< tg bowt for w1,w2 */
    int32 used;			/**< whether used since last lm_reset */
    struct tginfo32_s *next;	/**< Next w1 with same parent w2 */
} tginfo32_t;


/*
 * \struct lm_tgcache_entry_t
 * Entries in a fast and dirty cache for trigram lookups.  See lm_t.tgcache.
 */
typedef struct {
    s3lmwid_t lwid[3];		/**< 0 = oldest, 2 = newest (i.e., P(2|0,1)) */
    int32 lscr;			/**< LM score for above trigram */
} lm_tgcache_entry_t;


/*
 * \struct lm_tgcache_entry32_t
 * \brief 32 bit version of lm_tg_cache_entry
 */
typedef struct {
    s3lmwid32_t lwid[3];		/**< 0 = oldest, 2 = newest (i.e., P(2|0,1)) */
    int32 lscr;			/**< LM score for above trigram */
} lm_tgcache_entry32_t;



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
 * Of course, having multiple arrays of things will somedays caused
 * problems.
 *
 * The resolution is that we observed that the dict2lm map mostly
 * changed when the lm needs to change. Also, the fact that the
 * dictionary pronounciation itself seldom changes. That is partially
 * caused by the fact we don't have too much research on So at the
 * end, that is why it makes sense to let the lm to own a dict2lm.
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

/*
 * Comments by RKM
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

/** \struct lm_t
 * \brief The language model.
 * All unigrams are read into memory on initialization.
 * Bigrams and trigrams read in on demand.
 */
typedef struct lm_s {
    char *name ;        /**< The name of the LM */
    int32 n_ug;         /**< #unigrams in LM */
    int32 n_bg;         /**< #bigrams in entire LM */
    int32 n_tg;         /**< #trigrams in entire LM */
    int32 max_ug;       /**< To which n_ug can grow with dynamic addition of words */

    int32 n_ng;           /**< if unigram, n_ng=1, if bigram n_bg=2 and so one */
    
    char **wordstr;	/**< The LM word list (in unigram order) */
    
    
    uint32 log_bg_seg_sz;/**< See big comment above */
    uint32 bg_seg_sz;

    ug_t *ug;           /**< Unigrams */

    /* 20040225 ARCHAN : Data structure to maintain dictionary information */
    /* Data structure for dictionary to LM words look up mapping */
    /* 20060306 ARCHAN: Change this to a 32 bits data structure */
    s3lmwid32_t *dict2lmwid; /**< a mapping from dictionary word to LM word */
    s3lmwid32_t startlwid;	/**< S3_START_WORD id, if it exists */
    s3lmwid32_t finishlwid;	/**< S3_FINISH_WORD id, if it exists */

    bg_t *bg;		/**< NULL iff disk-based */
    tg_t *tg;		/**< NULL iff disk-based */
    membg_t *membg;	/**< membg[w1] = bigrams for lm wid w1 (used iff disk-based) */
    tginfo_t **tginfo;	/**< tginfo[w2] = fast trigram access info for bigrams (*,w2) */


    lm_tgcache_entry_t *tgcache; /**< <w0,w1,w2> hashed to an entry into
                                    this array.  Only the last trigram
                                    mapping to any * given hash entry is
                                    kept in that entry.  (The cache
                                    doesn't have to be super-efficient.)
                                 */


    /**************************/


    bg32_t *bg32;		/**< Bigram 32 bits, NULL iff disk-based */
    tg32_t *tg32;		/**< Trigram 32 bits NULL iff disk-based */
    membg32_t *membg32;	/**< membg 32bits membg[w1] = bigrams for lm wid w1 (used iff disk-based) */
    tginfo32_t **tginfo32;	/**< tginfo 32bits tginfo[w2] = fast trigram access info for bigrams (*,w2) */

    lm_tgcache_entry32_t *tgcache32; /** tgcache 32 bits */

    /**************************/
    
    lmlog_t *bgprob;    /**< Table of actual bigram probs */
    lmlog_t *tgprob;    /**< Table of actual trigram probs */
    lmlog_t *tgbowt;    /**< Table of actual trigram backoff weights */
    int32 *tg_segbase;  /**< tg_segbase[i>>lm_t.log_bg_seg_sz] = index of 1st
                           trigram for bigram segment (i>>lm_t.log_bg_seg_sz) */
    int32 n_bgprob;
    int32 n_tgprob;
    int32 n_tgbowt;

    FILE *fp;
    int32 byteswap;     /**< Whether this file is in the WRONG byte order */
    int32 bgoff;        /**< BG offsets into DMP file (used iff disk-based) */
    int32 tgoff;        /**< TG offsets into DMP file (used iff disk-based) */

    float32 lw;		/**< Language weight currently in effect for this LM */
    int32 wip;          /**< logs3(word insertion penalty) in effect for this LM */
    
    
    /* Statistics */
    int32 n_bg_fill;    /**< #bg fill operations */
    int32 n_bg_inmem;   /**< #bg in memory */
    int32 n_bg_score;   /**< #bg_score operations */
    int32 n_bg_bo;	/**< #bg_score ops backed off to ug */
    int32 n_tg_fill;	/**< Similar stats for trigrams */
    int32 n_tg_inmem;   /**< #tg in memory */
    int32 n_tg_score;   /**< #tg_score operations */
    int32 n_tg_bo;      /**< #tg_score ops backed off to bg */
    int32 n_tgcache_hit;  /**< # of trigram cache hit ops backed off to bg */
    
    int32 access_type;	/**< Updated on every lm_{tg,bg,ug}_score call to reflect the kind of
                           n-gram accessed: 3 for 3-gram, 2 for 2-gram and 1 for 1-gram */


    int32 isLM_IN_MEMORY;  /**< Whether LM in in memory, it is a property, potentially it means
                              the code could allow you some model to be disk-based, some are not. */

    int32 dict_size;  /**< Only used in class-based LM, because class-based LM is addressed in 
                         the dictionary space. */
    hash_table_t *HT;		/**<  hash table for word-string->word-id map */

  
    /* Data structure that maintains the class information */
    lmclass_t *lmclass;   /**< LM class for this LM */
    int32 n_lmclass;      /**< # LM class */
    int32 *inclass_ugscore; /**< An array of inter-class unigram probability */


    int32 inputenc ; /**< Input encoding method */
    int32 outputenc ; /**< Output encoding method */
    int32 version;  /**< The version number of LM, in particular, this is the version that recently
                       read in. 
                    */
    int32 is32bits; /**< Whether the current LM is 32 bits or not. Derived from version and n_ug*/

    /* Arrays of unique bigram probs and bo-wts, and trigram probs */
    sorted_list_t sorted_prob2; /**< Temporary Variable: Sorted list */
    sorted_list_t sorted_bowt2; /**< Temporary Variable: Sorted list */
    sorted_list_t sorted_prob3; /**< Temporary Variable: Sorted list */
    int32 max_sorted_entries; /**< Temporary Variable: 2x the maximum size of the MAX_SORTED_ENTRIES*/

} lm_t;



/** \struct lmset_t
    \brief Structure for multiple LM, provide operations for addition/deletion/read
    Structure for multiple, named LMs, started from s2
*/
typedef struct lmset_s {
    lm_t **lmarray;  /**< 1 dimensional array of pointers of lm_t */
    lm_t *cur_lm; /**< TEMPORARY VARIABLE: The current LM */

    int32 cur_lm_idx; /**< TEMPORARY VARIABLE : The current LM index */
    int32 n_lm;       /**< number of LM */
    int32 n_alloc_lm; /**< number of allocated LM */
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


/** \struct wordprob_t
    \brief Generic structure that could be used at any n-gram level 
*/
typedef struct {
    s3wid_t wid;	/**< NOTE: dictionary wid; may be BAD_S3WID if not available */
    int32 prob;         /**< The probability */
} wordprob_t;
  

/** A wrapper function of controlling the behavior of LM initialization 
 * 
 * (ARCHAN 20050617) lmset_init controls the behavior how the lmset
 * which is an array of lm was initialized by different command-line
 * arguments.  lmfile and lmctlfile are mutually exclusive.  Each
 * will invoke one reading functions.  
 * 
 * In the case of -lmfile is specified.  A lmset with one single lm
 * (or lmset->n_lm=1) will be returned. The single lm's name will be
 * called lmname.
 *
 * In the case of -lmctlfile is specified. A lmset with multiple lms
 * will be returned. The number of lm will depend on the number of
 * lm specified by -lmctlfile.  For the format, please read the
 * current format of -lmctlfile in lm.c
 *
 * ctl_lm is the equivalent of -ctl for lm.  When -ctl_lm is not
 * specified in command-line (ctl_lm is NULL). Then either lm with
 * name lmname will be used as the default lm.  If lmname is NULL, then
 * the first lm will be named as the "default"
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
 * ARCHAN 20050711 -lminmemory is the only global variable that
 * control the code and we haven't explicitly specify it.  Currently,
 * if the LM is DMP, both -lminmeory=0 or -lminmeory=1 could be used. 
 * if the LM is txt-base, only -lminmemory=1 is accepted. (This will
 * be changed in future.)
 *
 *
 * ARCHAN 20050705: A survival guide for this part of the code.  Our
 * language mode code is unnecessarily complicated and is mainly
 * caused by the fact the way we specified class-based LM and
 * multiple LM are inter-dependent. For example, one could specify a
 * multiple LMs file (i.e. lmctlfile) and have no classes.  However,
 * if one would like to specify class information even with a single
 * LM, one need to use a multiple LM file format (i.e. lmctlfile).
 *
 * This difficulty is well-observed in the period of Sphinx
 * 3.4-3.6. That might imply that a new LM format is needed if we
 * want to sustain this part of the development.
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
		       float64 uw,          /**< The unigram weight */
		       const char *lmdumpdir /**< In: LM dump dir */
    );

/**
 * Read the LM control file. **Usually**, it is also a class-based LM, 
 */

lmset_t* lmset_read_ctl(const char * ctlfile,/**< Control file name */
			dict_t* dict,  /**< In: Dictionary */
			float64 lw,	/**< In: Language weight */
			float64 wip,	/**< In: Word insertion penalty */
			float64 uw,    /**< In: Unigram weight */
			char* lmdumpdir /**< In: LMdumpdir */
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
 * @return a pointer of the name string.  No memory is allocated. 
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
		 s3lmwid32_t w1,	/**< In: LM word id of the first of a 2-word history */
		 s3lmwid32_t w2,	/**< In: LM word id of the second of the 2-word history */
		 tg_t **tg,	/**< Out: *tg = array of trigrams for <w1,w2> */
		 int32 *bowt	/**< Out: *bowt = backoff-weight for <w1, w2> */
    );

int32 lm_tg32list (lm_t *lmp,	/**< In: LM being queried */
		   s3lmwid32_t w1,	/**< In: LM word id of the first of a 2-word history */
		   s3lmwid32_t w2,	/**< In: LM word id of the second of the 2-word history */
		   tg32_t **tg,	/**< Out: *tg = array of trigrams for <w1,w2> */
		   int32 *bowt	/**< Out: *bowt = backoff-weight for <w1, w2> */
    );

/**
 * Return the bigram followers for the given word w.
 * Return value: #bigrams in returned list.
 */
int32 lm_bglist (lm_t *lmp,	/**< In: LM being queried */
		 s3lmwid32_t w,	/**< In: LM word id of the 1-word history */
		 bg_t **bg,	/**< Out: *bg = array of bigrams for w */
		 int32 *bowt	/**< Out: *bowt = backoff-weight for w */
    );

int32 lm_bg32list (lm_t *lmp,	/**< In: LM being queried */
		   s3lmwid32_t w,	/**< In: LM word id of the 1-word history */
		   bg32_t **bg,	/**< Out: *bg = array of bigrams for w */
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
		     s3lmwid32_t w,	/**< In: LM word ID of the 1-word history */
		     int32 th,		/**< In: If a prob (logs3, langwt-ed) < th, ignore it */
		     wordprob_t *wp,	/**< In/Out: Array to be filled; caller must have
					   allocated this array */
		     int32 *bowt	/**< Out: *bowt = backoff-weight associated with w */
    );

#endif

/* Return LM word ID for the given string, or BAD_LMWID(lm) if not available */
s3lmwid32_t lm_wid (lm_t *lm, char *wd);

/**
   Set all pointers to NULL in the lm
*/
void lm_null_struct(lm_t* lm 
    );

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
  


/* 20040227: This also account the in-class probability of wid*/
/** Return unigram score for the given word */
int32 lm_ug_score (lm_t *lmp,  /**< In: LM begin queried */
		   s3lmwid32_t lwid, /**< LM ID for the word */
		   s3wid_t wid     /**< Dict ID for the word */
    );

  
int32 lm_ug_exists(lm_t* lm ,  /**< LM */
		   s3lmwid32_t lwid /**< LM ID for the word */
    );
  
/*
 * Return bigram score for the given two word sequence.  If w1 is BAD_LMWID(lm), return
 * lm_ug_score (w2).
 * 20040227: This also account for the in-class probability of w2. 
 */
int32 lm_bg_score (lm_t *lmp, /**< In: LM begin queried */
		   s3lmwid32_t lw1, 
		   s3lmwid32_t lw2,
		   s3wid_t w2);


/**
   Whether a certain bigram exists. 
*/
int32 lm_bg_exists (lm_t *lm,  /**< In: LM */
		    s3lmwid32_t lw1,  
		    s3lmwid32_t lw2   
    );

/**
 * Return trigram score for the given three word sequence.  If w1 is BAD_LMWID(lm), return
 * lm_bg_score (w2, w3).  If both lw1 and lw2 are BAD_LMWID(lm), return lm_ug_score (lw3).
 * 
 * 20040227: This also account for the in-class probability of w3. 
 */
int32 lm_tg_score (lm_t *lmp,  /**< In: LM begin queried */
		   s3lmwid32_t lw1, 
		   s3lmwid32_t lw2, 
		   s3lmwid32_t lw3, 
		   s3wid_t w3);


/**
   Whether a certain trigram exists. 
*/
int32 lm_tg_exists (lm_t *lm,  /**< In: LM */
		    s3lmwid32_t lw1,  
		    s3lmwid32_t lw2,
		    s3lmwid32_t lw3
    );

/**
 * Set the language-weight and insertion penalty parameters for the LM, after revoking
 * any earlier set of such parameters.
 *
 * WARNING!! This function doesn't prevent underflow of values.  Make sure you call
 * safe lm2logs3 before it. 
 */
void lm_set_param (lm_t *lm,  /**< In: the LM */
		   float64 lw,  /**< In: the langauage weight */
		   float64 wip  /**< In: the word insertion penalty */
    );


int32 lm_rawscore (lm_t *lm,  /**< In: the LM */
		   int32 score
    );



/** LM cache related */
void lm_cache_reset (lm_t *lmp /**< In: the LM */
    );

/** LM cache statistic dumping */
void lm_cache_stats_dump (lm_t *lmp /**< In: the LM */
    );

/** 
 * A simple version of reading in a LM 
 *
 * lm_read is a simple version of lm_read_advance.  It will assume
 * language weight, word insertion penalty and unigram weight to be
 * automatically applied.  There is also no class-based LM (so
 * ndict=0).  Format is set to NULL, so the program will determine
 * it automatically. 
 */

lm_t * lm_read ( 
    const char *file,	/**< In: LM file being read */
    const char *lmname   /**<In: LM name*/
    );
/**
 * Read an LM file, it will automatically decide whether the file is
 * a DUMP file or a txt file. Then call lm_read_txt and lm_read_dump
 * (non-public functions) correspondingly.  Currently the code is
 * not aware about OOV.  
 *
 * lw, wip, uw and ndict are mainly used for recognition purpose.
 * When lm_read is used for other purpose, one could just used dummy
 * setting.  recommended one is lw=1.0,wip=0.1,uw=1.0 and
 * ndict=0. These are very useful when lm_read is just used as
 * reading the LM.  
 *
 * If applyweight is 0, lw,wip, uw will not be apply the LM at all.
 * This will allow users to just call the LM routine without
 * initializing other modules (such as logs3_init).
 * 
 * If applyweight is 1, then logs3_init must be called before lm_read. 
 * This is usually the case when kb_init is called before the code. 
 *
 * fmt now could be either "TXT", "DMP" and "TXT32" or just
 * NULL. If it is NULL, the LM format will be automatically
 * determined.  If it is specified as "TXT" or "DMP", the
 * corresponding lm reader will be called. In such a case, it is
 * important for the users to know what he/she is doing.
 * (Unfortunately, this is mostly not true. ) 
 * In the case of "TXT32", a text LM will be forced to 32bit mode. 
 *
 * ndict is the dictionary size of the application.  This is needed
 * because class-based LM are addressed in the dictionary wid-space
 * instead of lm wid-space. If class-based LM is not used, just set
 * this to zero.
 *
 * Note: there are two defense mechanisms of lm_read_advance. 
 * First of all, if no fmt is specified, it will start to read
 * the lm in the order of DMP->TXT. Second, if txt format
 * is specified but LM is found to hit the 16bit legacy segments
 * limit, it will automatically switch to read TXT32 LM
 *
 * @return pointer to LM structure created.
 */
lm_t *lm_read_advance (const char *file,	/**< In: LM file being read */
		       const char *lmname,   /**<In: LM name*/
		       float64 lw,	/**< In: Language weight */
		       float64 wip,	/**< In: Word insertion penalty */
		       float64 uw,	/**< In: Unigram weight (interpolation with uniform distr.) */
		       int32 ndict,    /**< In: Number of dictionary entry.  We need that because
					  class-based LM is addressed in dictionary word ID space. 
				       */  
		       char* fmt,       /**< In: file format of the LM, it is
					   now either "TXT", "DMP" and NULL,
					   if NULL, file format is
					   automaticaly determined */
		       int32 applyweight      /**< In: whether lw,wip, uw should be 
						 applied to the lm or not */
    );

/**
   Simple writing of an LM file, the input and output encoding will
   assume to be iso8859-1. Call lm_write. To convert encoding, please use
   lm_write_advance. 
*/
int32 lm_write(lm_t *model, /** In: the pointer LM we want to output */
	       const char *outputfile, /**< In: the output file name */
	       const char *filename, /**< In: the LM file name  */
	       char *fmt   /**< In: LM file format, it is now either "TXT" or "DMP" */
    );
  
/**
   Writing of an LM file with advanced options such as encoding support. 
   Called by lm_write. 
     
   fmt now could be TXT, DMP, FST
     
   inputenc and outputenc could now be iso8859-1, gb2312-hex, gb2312.  
   Not every pair of conversion works.  
     
   Current input/output encodings support list. 
   0: iso8859-1
   1: gb2312-hex
   2: gb2312

   -: do nothing
   n: doesn't make sense or not compatible
   x: not supported yet
   y: supported

   i\o 0 1 2
   0 - n n
   1 n - y
   2 n x -

   When we have 4 encoding types: This document should be
   implemented as a data structure.

   This conversion table is copied from encoding.c, please take a
   look the latest support in encoding.c
*/

int32 lm_write_advance(lm_t *model, /**< In: the pointer LM we want to output */
		       const char *outputfile, /**< In: the output file name */
		       const char *filename, /**< In: the LM file name  */
		       char *fmt,   /**< In: LM file format, it is now either "TXT", "DMP", "FST" */
		       char* inputenc, /**< In: Input encoding type */
		       char* outputenc /**< Out: Output encoding type */
    );

/* RAH, added code for freeing allocated memory 
 */
/**
   Deallocate the language model. 
*/
void lm_free (lm_t *lm /**< In: a LM structure */
    );

/**
   Add word list to the LM 
   For each word in the file, call lm_add_wordlist. 
   The file is assume to have a format like this:
   <word1> 
   <word2>
   <word3>
   <word4>
     
   If the lmwid2dictid mapping is not updated, or the dictionary
   itself is not used in the context.  Just specify dict=NULL;
     
*/
int32 lm_add_wordlist(lm_t *lm, /**< In/Out: a modified LM structure */
		      dict_t *dict, /**< In: an initialized dictionary structure 
				       Used to update 
				    */
		      char* filename /**< In: a file that contains a
					list of word one wants to
					add*/
    );

/**
   Add a word to the LM 

   look up the dictionary and see whether it exists in the dictionary
   Looks alike with wid.c's logic at this point.  

   (Incomplete!) Not fully tested in the situation for on-line
   recognition.
     
   We also avoid the addition of classes at this point because that
   could complicated things quite a lot. 
*/
int32 lm_add_word_to_ug(lm_t *lm, /**< In/Out: a modified LM structure */
			dict_t *dict, /**< In: an initialized dictionary structure 
					 Used to update lmwid2dictid mapping. 
				      */
			char* newword /**<In: a pointer of a new word */
    );
/** 
    Get class ID given a LM. 
*/
int32 lm_get_classid (lm_t *model, /**< In: LM file being queried*/
		      char *name   /**< In: The name of the class */
    );

/**
 * Explicity convert structure from 16bit -> 32bit or 32bit to 16bit. 
 */
void lm_convert_structure(lm_t *model, /**< In: LM file being used */
			  int32 is32bits 
    );	

/**
   Check whether the model is operating at 32 bits 
*/
int32 lm_is32bits(lm_t* model);

/**
   Write of UG structure
*/
void ug_write(FILE* fp,  /**< A file pointer */
	      ug_t* ug   /**< A pointer of the ug_t structure */
    );
/**
   Write of BG structure
*/
void bg_write(FILE* fp, /**< A file pointer */
	      bg_t* bg  /**< A pointer of the bg_t structure */
    );

/**
   Write of BG (32bits) structure
*/
void bg32_write(FILE* fp, /**< A file pointer */
		bg32_t* bg  /**< A pointer of the bg32_t structure */
    );

/**
   Write of TG structure
*/
  
void tg_write(FILE* fp, /**< A file pointer */
	      tg_t* tg  /**< A pointer of the tg_t structure */
    );

/**
   Write of TG (32bits) structure
*/
  
void tg32_write(FILE* fp, /**< A file pointer */
		tg32_t* tg  /**< A pointer of the tg32_t structure */
    );


/**
   Convert the 16 bit bigram structure to 32 bit
*/
void copy_bg_to_bg32(lm_t *lm /**< LM */
    );

/**
   Convert the 32 bit bigram structure to 16 bit
*/

void copy_bg32_to_bg(lm_t *lm /**< LM */
    );

/**
   Convert the 16 bit trigram structure to 32 bit
*/
void copy_tg_to_tg32(lm_t *lm /**< LM */
    );

/**
   Convert the 32 bit trigram structure to 16 bit
*/

void copy_tg32_to_tg(lm_t *lm /**< LM */
    );

/**
   Swap 16 bits bigram
*/
void swap_bg(bg_t* bg);
  

/**
   Swap 32 bits bigram
*/
void swap_bg32(bg32_t* bg);

/**
   Swap 16 bits trigram
*/
void swap_tg(tg_t* tg);
  

/**
   Swap 32 bits trigram
*/
void swap_tg32(tg32_t* tg);

int32 find_bg (bg_t *bg,  /**< In: The bigram */
	       int32 n, 
	       s3lmwid32_t w
    );

int32 find_bg32 (bg32_t *bg,  /**< In: The bigram */
		 int32 n, 
		 s3lmwid32_t w
    );


int32 find_tg (tg_t *tg, /**< In: The trigram */
	       int32 n, s3lmwid32_t w);

int32 find_tg32 (tg32_t *tg, /**< In: The trigram */
		 int32 n, s3lmwid32_t w);
  
/* Macro versions of access functions */
#define LM_TGPROB(lm,tgptr)	((lm)->tgprob[(tgptr)->probid].l)
#define LM_BGPROB(lm,bgptr)	((lm)->bgprob[(bgptr)->probid].l)
#define LM_UGPROB(lm,ugptr)	((ugptr)->prob.l)
#define LM_RAWSCORE(lm,score)	((score - (lm)->wip) / ((lm)->lw))
#define LM_DICTWID(lm,lmwid)     ((lm)->ug[(lmwid)].dictwid)

/** 
    Create a new unigram table
*/
ug_t *NewUnigramTable (int32 n_ug /**< Number of unigram */
    );


#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
