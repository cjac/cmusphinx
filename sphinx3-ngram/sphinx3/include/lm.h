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
 * 2008/11/5  P. Deleglise and Y. Esteve
 * Adapted the code to manage large files (off_t 64 bits instead of int32)
 *
 * 2008/08/21  N. Coetmeur, supervised by Y. Esteve
 * Add conditionnal statements (#ifndef _READ_DUMP_ONLY_) for parts of code that we must
 * deactivating for use LM model with our others tools.
 *
 * 2008/06/27  N. Coetmeur, supervised by Y. Esteve
 * Adjust comments for compatibility with Doxygen 1.5.6
 *
 * 2008/06/20  N. Coetmeur, supervised by Y. Esteve
 * Adapt LM structure for working with N-grams for each N value
 * (i.e for a trigram file N=3, for a quadrigram file N=4 and so on...).
 * - Added many tables (dynamically allocated) of N values, index from 0 to N-1
 *  (for clarify the code, all tables have N elements, although sometime 0 and 1
 *  indexes are not used) in replacement of old fields:
 *   --------------------------------------------------------------
 *   |             old fields             |    new table field    |
 *   --------------------------------------------------------------
 *   | n_ug     n_bg         n_tg         |          n_ng         |
 *   |        log_bg_seg_sz               |        log_ng_seg_sz  |
 *   |            bg           tg         |            ng         |
 *   |                         tginfo     |            nginfo     |
 *   |                         tgcache    |            ngcache    |
 *   |            bg32         tg32       |            ng32       |
 *   |                         tginfo32   |            nginfo32   |
 *   |                         tgcache32  |            ngcache32  |
 *   |            bgprob       tgprob     |            ngprob     |
 *   |                         tgbowt     |            ngbowt     |
 *   |                         tg_segbase |            ng_segbase |
 *   |          n_bgprob     n_tgprob     |          n_ngprob     |
 *   |                       n_tgbowt     |          n_ngbowt     |
 *   |            bgoff        tgoff      |            ngoff      |
 *   |          n_bg_fill    n_tg_fill    |          n_ng_fill    |
 *   |          n_bg_inmem   n_tg_inmem   |          n_ng_inmem   |
 *   |          n_bg_score   n_tg_score   |          n_ng_score   |
 *   |          n_bg_bo      n_tg_bo      |          n_ng_bo      |
 *   | sorted_prob2 sorted_prob3          | sorted_probn          |
 *   | sorted_bowt2                       | sorted_bowtn          |
 *   --------------------------------------------------------------
 * - Group some structures (and macros):
 *   > bg_t and tg_t both became ng_t
 *   > bg32_t, tg32_t became ng32_t (and two define values are created for the
 *    size of the le last N-gram level structure (without bowt_id and firstnng))
 *   > macros lm_n_ug(lm), lm_n_bg(lm), lm_n_tg(lm) became lm_n_ng(lm,n)
 *   > macros LM_BGPROB(lm,bgp) and LM_TGPROB(lm,tgp) became LM_NGPROB(lm,n,ngp)
 * - Change some names:
 *   > in lm_t structure, old field n_ng is renamed max_ng
 *   > version values LMDMP_VERSION_TG_16BIT / 16BIT_V2, LMDMP_VERSION_TG_32BIT
 *           are renamed LMDMP_VERSION_16BIT / 16BIT_V2, LMDMP_VERSION_32BIT   
 *   > structures tginfo_t, tginfo32_t, lm_tgcache_entry_t, lm_tgcache_entry32_t
 *    are renamed nginfo_t, nginfo32_t, lm_ngcache_entry_t, lm_ngcache_entry32_t
 *    and now used a table of words dynamically allocated
 *   > define values LOG2_BG_SEG_SZ, BG_SEG_SZ, LM_TGCACHE_SIZE
 *       are renamed LOG2_NG_SEG_SZ, NG_SEG_SZ, LM_NGCACHE_SIZE
 * - Replace functions with 'ug', 'bg' and 'tg' in their name by one function
 *  (with 'ng' in the name) with the N value as a parameter.
 *  [ few exceptions: lm_ug_wordprob, lm_uglist, ug_write ]
 * - New function lm_init_n_ng: allocate memory for tables in LM with the max Ng
 *  level value
 * - lm_write and lm_write_advance functions now used separately output
 *  directory and output file name in parameters instead of the complete full
 *  path in previous version
 * - Replace 'lmclass_t' in lm_t structure by 'struct lmclass_s' for
 *  compatibility of lm.h with another program that we used
 *
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

#include <stdio.h>

#include <logmath.h>
#include <hash_table.h>
#include <cmd_ln.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/* LIUM */
#define _FILE_OFFSET_BITS 64
#include "s3types.h"
#include "lmclass.h"
#include "dict.h"

/* LIUM */
#ifndef _READ_DUMP_ONLY_
#include "encoding.h"
#else
#define S3DECODER_EXPORT
#endif

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
/* LIUM */
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
				 
#define LMDMP_VERSION_16BIT -1 /**< VERSION 1 is the simplest DMP file which
				     is trigram or lower which used 16 bits in
				     bigram and trigram.*/

#define LMDMP_VERSION_16BIT_V2 -2 /**< VERSION 2 means legacy VERSION 1 DMP file
					which has log_bg_seg_sz != 9*/

#define LMDMP_VERSION_32BIT -3 /**< VERSION 3 is the 32 bit
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
                                      lm_is32bits(version, n_ng[0])
                                   */

#define LM_DICTWID_BADMAP	-16000		/** An illegal mapping */
#define LM_CLASSID_BASE		0x01000000	/** Interpreted as LMclass ID */
#define MIN_PROB_F       -99.0  /**< The minimum value of probabilities and
								backoff weights. When changing, notice
								that both s2 and s3 may transform this 
								number to very small integer (say -2e-31)
								This will easily cause integer wrap 
								around.  -99 is chosen for that reason. */
#define LM_ALLOC_BLOCK      16  /** The number of LMs to allocate at a time. */
#define NO_WORD				-1

/* LIUM */
/** Access macros; not meant for arbitrary use */
#define LM_CLASSID_TO_CLASS(m,i)	((m)->lmclass[(i)-LM_CLASSID_BASE])
#define lm_lmwid2dictwid(lm,u)	((lm)->ug[u].dictwid)
#define lm_n_ng(lm,n)		((lm)->n_ng[n-1])
#define lm_n_ug(lm)			((lm)->n_ng[0])
#define lm_n_bg(lm)			((lm)->n_ng[1])
#define lm_n_tg(lm)			((lm)->n_ng[2])
#define lm_wordstr(lm,u)	((lm)->wordstr[u])
#define lm_startwid(lm)		((lm)->startlwid)
#define lm_finishwid(lm)	((lm)->finishlwid)
#define lm_access_type(lm)	((lm)->access_type)
#define LM_NGPROB(lm,n,ngptr)	((lm)->ngprob[n-1][(ngptr)->probid].l)
#define LM_TGPROB(lm,tgptr)	((lm)->ngprob[2][(tgptr)->probid].l)
#define LM_BGPROB(lm,bgptr)	((lm)->ngprob[1][(bgptr)->probid].l)
#define LM_UGPROB(lm,ugptr)	((ugptr)->prob.l)
#define LM_RAWSCORE(lm,score)	((score - (lm)->wip) / ((lm)->lw))
#define LM_DICTWID(lm,lmwid)     ((lm)->ug[(lmwid)].dictwid)

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
  
/** \union lmlog_t
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

/* LIUM */
/** \struct ng_t
 *  \brief A N-gram structure
 */
typedef struct {
    s3lmwid_t   wid;      /**< LM wid (index into lm_t.ug) */
    uint16      probid;   /**< Index into array of actualy N-gram probability */
    uint16      bowtid;   /**< Index into array of actualy
						   N-gram back-off weights */
    uint16      firstnng; /**< First next N-gram ((N+1)gram) entry on disk
						   \see lm_t.ng_segbase */
} ng_t;

/* LIUM */
/** \brief Size of a 16-bits last N-gram level
 \details Used for read a 16-bits N-gram file without (N+1)gram information.
 */
#define SIZE_OF_NG16_WITHOUT_NNG (sizeof(s3lmwid_t)+sizeof(uint16))

/* LIUM */
/** \struct ng32_t 
 * \brief A N-gram structure which has 32 bits. 
 */
typedef struct {
    s3lmwid32_t wid;      /**< LM wid (index into lm_t.ug) */
    uint32      probid;   /**< Index into array of actualy N-gram probability */
    uint32      bowtid;   /**< Index into array of actualy
						   N-gram back-off weights */
    uint32      firstnng; /**< First next N-gram ((N+1)gram) entry on disk
						   \see lm_t.ng_segbase */
} ng32_t;

/* LIUM */
/** \brief Size of a 32-bits last N-gram level
 \details Used for read a 32-bits N-gram file without (N+1)gram information.
 */
#define SIZE_OF_NG32_WITHOUT_NNG (sizeof(s3lmwid32_t)+sizeof(uint32))

/* LIUM */
/** \struct membg_t
 *  \brief Management of in-memory bigrams.  Not used if all bigrams in memory.
 */
typedef struct {
    ng_t *bg;		/**< Bigrams for a specific unigram; see lm_t.membg */
    int32 used;		/**< Whether used since last lm_reset.  If not used, at the next
                           lm_reset bg are freed */
} membg_t;

/* LIUM */
/** \struct membg32_t
 *
 * \brief A 32 bits version of membg_t
 */
typedef struct {
    ng32_t *bg32;		/**< Bigrams for a specific unigram; see lm_t.membg */
    int32 used;		/**< Whether used since last lm_reset.  If not used, at the next
                           lm_reset bg are freed */
} membg32_t;

/* LIUM */
/**
 * \struct nginfo_t
 * \brief N-gram cache that enhance locating N-gram for a given (N-1)gram
 *
 * The following N-gram information cache eliminates most traversals of
 * 1g->...->ng tree to locate N-grams for a given (N-1)gram (w1...wN-1).  The
 * organization is optimized for locality of access.  All (N-1)grams (*,wN-1)
 * for a given wN-1, for which N-grams have been accessed "recently", form a
 * linear linked list, pointed to by lm_t.nginfo[n][wN-1]. If disk-based, all
 * N-grams for the given (N-1)g loaded upon request.  Cached info (and ng if
 * disk-based) freed at lm_reset if not used since last such reset.
 */
typedef struct nginfo_t {
    s3lmwid_t       *w;    /**< \brief w1..wN-2 component of (N-1)gram w1..wN-1
							\details All (N-1)grams with same wN-1 are
							linked together. */
    int32            n_ng; /**< Number of N-grams for
							parent (N-1)gram w1...wN-1 */
    ng_t            *ng;   /**< N-grams for w1...wN-1 */
    int32            bowt; /**< N-gram back-off weight for w1...wN-1 */
    int32            used; /**< Whether used since last lm_reset */
    struct nginfo_t *next; /**< Next (w1...wN-2) with same parent wn */
} nginfo_t;

/* LIUM */
/**
 * \struct nginfo32_t
 * \brief 32 bit version of nginfo_t
 * \see nginfo_t
 */
typedef struct nginfo32_t {
    s3lmwid32_t       *w;   /**< \brief w1..wN-2 component of (N-1)gram w1..wN-1
							 \details All (N-1)grams with same wN-1
							 linked together. */
    int32              n_ng; /**< Number of N-grams for
							  parent (N-1)gram w1...wN-1 */
    ng32_t            *ng32; /**< N-grams for w1...wN-1 */
    int32              bowt; /**< N-gram back-off weight for w1...wN-1 */
    int32              used; /**< Whether used since last lm_reset */
    struct nginfo32_t *next; /**< Next (w1...wN-2) with same parent wn */
} nginfo32_t;

/* LIUM */
/**
 * \struct lm_ngcache_entry_t
 * \brief Entries in a fast and dirty cache for N-gram lookups
 * \see lm_t.ngcache.
 */
typedef struct {
    s3lmwid_t  *lwid;   /**< N words id for the N-grams */
    int32       lscr;   /**< LM score for above N-gram */
} lm_ngcache_entry_t;

/* LIUM */
/**
 * \struct lm_ngcache_entry32_t
 * \brief 32 bit version of lm_ngcache_entry_t
 * \see lm_ngcache_entry_t
 */
typedef struct {
    s3lmwid32_t    *lwid;   /**< N words id for the N-grams */
    int32           lscr;   /**< LM score for above N-gram */
} lm_ngcache_entry32_t;

/* LIUM */
#define LM_NGCACHE_SIZE 100003  /**< Size of cache : a prime number */

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
#define LOG2_NG_SEG_SZ  9	
#define NG_SEG_SZ       (1 << (LOG2_NG_SEG_SZ))

/* 20040211 ARCHAN: Yes! Indeed it is a prime */

/* LIUM */
/** \struct lm_t
 *  \brief The language model
 *
 * All unigrams are read into memory on initialization.
 * Bigrams, trigrams and more read in on demand.
 */
typedef struct {
    /** \name File informations */
    /*\{*/
    char   *name;        /**< \brief The name of the LM */
    FILE   *fp;          /**< \brief The file pointer */
    int32   version;     /**< \brief The version number of LM
						  \details In particular, this is the version that
						  recently read in. */
    int32   byteswap;    /**< \brief Whether this file is in the
						  WRONG byte order */
    int32   is32bits;    /**< \brief Whether the current LM is 32-bits or not */
    int32   isLM_IN_MEMORY; /**< \brief Whether LM in in memory
							 \details It is a property, potentially it means
							 the code could allow you some model to be
							 disk-based, some are not. */
    int32   inputenc;    /**< \brief Input encoding method */
    int32   outputenc;   /**< \brief Output encoding method */
    int32   access_type; /**< \brief Kind of N-gram accessed
						  \details Updated on every ::lm_ng_score call:
						  1 for 1-gram, 2 for 2-gram and so on. */
    /*\}*/
	
    /** \name N-grams informations */
    /*\{*/
    uint32      max_ng;     /**< \brief Max N-gram level in LM file
							 \details If bigram file max_ng=2, if trigram
							 file max_ng=3 and so on. */
    int32       max_ug;     /**< \brief To which n_ng[0] can grow with dynamic
							 addition of words */
    int32      *n_ng;       /**< \brief n_ng[N-1]: N-grams count in entire LM */
    ug_t       *ug;         /**< \brief Table of Unigrams */
    ng_t      **ng;         /**< \brief ng[N-1]: Table of N-grams (N>=2) */
    ng32_t    **ng32;		/**< \brief ng32[N-1]: Table of 32-bits N-grams
							 (N>=2) */
    s3lmwid32_t startlwid;  /**< \brief S3_START_WORD id, if it exists */
    s3lmwid32_t finishlwid; /**< \brief S3_FINISH_WORD id, if it exists */
    off_t      *ngoff;      /**< \brief ngoff[N-1] : N-grams offsets into
							 DMP file (used iff disk-based) */
    /*\}*/
	
    /** \name Segments */
    /*\{*/
    uint32 *log_ng_seg_sz; /**< \brief log_ng_seg_sz[N-1] : Table of
							log2(Ngram_segment_size) (N>=2) */
    int32 **ng_segbase;    /**< \brief ng_segbase[N-1][i>>(log_ng_seg_sz[N-2])]:
							absolute index of first N-gram corresponding
							to i (N-1)gram absolute index */
    /*\}*/
	
    /** \name Words */
    /*\{*/
    char          **wordstr;  /**< \brief The LM word list (in unigram order) */
    hash_table_t   *HT; /**< \brief Hash table for word-string to word-id map */
    /*\}*/
	
    /** \name Probabilities */
    /*\{*/
    float32         lw;                 /**< \brief Language weight currently
										 in effect for this LM */
    int32           wip;                /**< \brief logs3(word insertion
										 penalty) in effect for this LM */
    int32          *n_ngprob;           /**< \brief n_ngprob[N-1]: Number of
										 N-grams probalities (N>=2) */
    lmlog_t       **ngprob;             /**< \brief ngprob[N-1] : Table of
										 N-grams probalities (N>=2) */
    sorted_list_t  *sorted_probn;       /**< \brief Sorted list of unique
										 N-grams probalities (N>=2) */
    int32          *n_ngbowt;           /**< \brief n_ngbowt[N-1] : Number of
										 N-grams back-off weights (N>=3) */
    lmlog_t       **ngbowt;             /**< \brief ngbowt[N-1] : Table of
										 N-grams back-off weights (N>=3) */
    sorted_list_t  *sorted_bowtn;       /**< \brief Sorted list of unique
										 N-grams back-off weights (N>=3) */
    int32           max_sorted_entries; /**< \brief Twice the maximum size of
										 MAX_SORTED_ENTRIES */
    /*\}*/
	
    /** \name N-grams locating */
    /*\{*/
    membg_t        *membg;     /**< \brief membg[w1]: Bigram for LM word id w1
								(used if disk-based) */
    membg32_t      *membg32;   /**< \brief membg[w1]: 32-bits bigram for LM
								word id w1 (used iff disk-based) */
    nginfo_t     ***nginfo;    /**< \brief nginfo[N-1][wN-1]: Fast N-gram
								access info for (N-1)grams (*,wN-1) */
    nginfo32_t   ***nginfo32;  /**< \brief nginfo32[N-1][wN-1]: Fast 32-bits
								N-gram access info for (N-1)grams (*,wN-1) */
    /*\}*/
	
    /** \name Cache */
    /*\{*/
    lm_ngcache_entry_t    **ngcache;    /**< \brief N-gram cache
										 \details (w1...wN) hashed to an
										 entry into ngcache[N-1] array. Only
										 the last N-gram mapping to any given
										 hash entry is kept in that entry.
										 (The cache doesn't have to be
										 super-efficient.) */
    lm_ngcache_entry32_t  **ngcache32;  /**< \brief 32-bits N-gram cache
										 \see ngcache */
	/*\}*/
    /** \name Dictionary */
    /*\{*/
    /**
     * \brief A mapping from dictionary word to LM word
     * 
     * \note
     * ARCHAN 20050616
     * 
     * In older versions of sphinx3 (<s3.4). dict2lm is a separate object
     * from LM and dictionary.  A kb actually owns a dict2lm so programer will
     * read the LM.  This seprates the initalization of LM and dict2lm and
     * it makes a lot of sense if there is **only one** LM and **only one
     * dict2lm. 
     * 
     * However, when multiple LMs and switching of them is required.
     * Then, the problem of the above architecture starts to show up.  For
     * example, 
     *  lmset=lm_read_ctl ();
     *  for(i=0;i<kb->n_lm;i++){
     *   dict2lmwid[i]=wid_dict_lm_map
     *  }
     * At the same time, one will also have an array of LMs (lmset[i]) for 
     * corresponding dict2lm[i]!
     *
     * Of course, having multiple arrays of things will somedays caused
     * problems.
     *
     * The resolution is that we observed that the dict2lm map mostly
     * changed when the LM needs to change. Also, the fact that the
     * dictionary pronounciation itself seldom changes. That is partially
     * caused by the fact we don't have too much research on So at the
     * end, that is why it makes sense to let the LM to own a dict2lm.
     * 
     * What if we also allow the dictionary to change? That is a tough
     * question.  In that case perhaps, we should still inventory of sets
     * of LM and dict2lm and allow LM to store a pointer of dict2lm.  Once
     * there are changes in dict, programmer will be responsible to update
     * dict2lm. (Storing pointers will allow programmers not to update
     * everything but just LMs corresponding to a particular dict.)  I
     * guess in that case it will be sign of having a wrapper that control
     * both LM and dictionary together.
     */
    s3lmwid32_t    *dict2lmwid;
    int32           dict_size;  /**< \brief Dictionary size
								 \details Only used in class-based LM, because class-based LM 
								 is addressed in the dictionary space. */
    /*\}*/
	
    /** \name LM Class */
    /*\{*/
    /* Data structure that maintains the class information */
    int32               n_lmclass;          /**< \brief Number of LM class */
    struct lmclass_s  **lmclass;            /**< \brief LM class for this LM */
    int32              *inclass_ugscore;    /**< \brief An array of inter-class
											 unigram probability */
    /*\}*/
	
    /** \name Statistics */
    /*\{*/
    int32   *n_ng_fill;     /**< \brief n_ng_fill[N-1]: Number of N-gram fill
							 operations */
    int32   *n_ng_inmem;    /**< \brief n_ng_inmem[N-1]: Number of N-grams in
							 memory */
    int32   *n_ng_score;    /**< \brief n_ng_score[N-1]: Number of N-gram score
							 operations */
    int32   *n_ng_bo;	    /**< \brief n_ng_bo[N-1]: Number of N-gram score
							 operations backed off to (N-1)g */
    int32   *n_ngcache_hit; /**< \brief n_ngcache_hit[N-1]: Number of N-gram
							 cache hit operations backed off to (N-1)gram */
    /*\}*/
    logmath_t *logmath;
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
S3DECODER_EXPORT
lmset_t* lmset_init(const char* lmfile,  /**< The lm file name, lmfile and lmctlfile are mutally exclusive */
		    const char* lmctlfile, /**< The file that specified multiple LMs and class information, lmfile and lmctlfile are mutually exclusive */
		    const char* ctl_lm,    /**< The control file that describes which lm to use for a particular utterance*/
		    const char* lmname,    /**< The LM name to use if ctl_lm is not specified  */
		    const char* lmdumpdir, /**< Currently not used */
		    float32 lw,      /**< Language model weight */
		    float32 wip,     /**< Word insertion penalty */
		    float32 uw,      /**< Unigram weight */
		    dict_t *dict,     /**< A pre-initialized dict_t structure */
		    logmath_t *logmath
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
		       const char *lmdumpdir, /**< In: LM dump dir */
		       logmath_t *logmath
    );

/**
 * Read the LM control file. **Usually**, it is also a class-based LM, 
 */

lmset_t* lmset_read_ctl(const char * ctlfile,/**< Control file name */
			dict_t* dict,  /**< In: Dictionary */
			float64 lw,	/**< In: Language weight */
			float64 wip,	/**< In: Word insertion penalty */
			float64 uw,    /**< In: Unigram weight */
			const char* lmdumpdir, /**< In: LMdumpdir */
			logmath_t *logmath
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
S3DECODER_EXPORT
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
S3DECODER_EXPORT
void lmset_free(lmset_t *lms /**< In: The set of LM */
    );

/* LIUM */
/**
 \brief Return N-gram (N>=2) followers for given N-1 words
 All w1...wN-1 must be valid.
 \return Number of N-grams in returned list
 */
int32
lm_nglist   (lm_t * lmp,	 /**< In: LM being queried */
             uint32 N,	     /**< In: N-gram */
             s3lmwid32_t * w,/**< In: N-1 LM word ids of the N-1-word history */
             ng_t ** ng,	 /**< Out: *Ng = array of N-grams for <w1...wN-1> */
		     int32 * bowt	 /**< Out: *bowt = back-off weight for <w1...w3> */
             );

/* LIUM */
/**
 \brief Return 32-bits N-gram (N>=2) followers for given N-1 words
 All w1...wN-1 must be valid.
 \return Number of N-grams in returned list
 */
int32
lm_ng32list (lm_t * lmp,      /**< In: LM being queried */
             uint32 N,	      /**< In: N-gram */
		     s3lmwid32_t * w, /**< In: N-1 LM word IDs of the N-1-word history */
		     ng32_t ** ng,	  /**< Out: *ng = array of N-grams for <w1...wN-1> */
		     int32 * bowt	  /**< Out: *bowt = back-off weight for <w1...w3> */
             );

/* Return LM word ID for the given string, or BAD_LMWID(lm) if not available */
s3lmwid32_t lm_wid (lm_t *lm, const char *wd);

/**
   Set all pointers to NULL in the lm
*/
void lm_null_struct(lm_t* lm 
    );

/* LIUM */
/**
 \brief Allocate memory for tables in LM
 \note Call ::lm_null_struct function before calling this function with a new LM.
 \return Error code
 */
int32
lm_init_n_ng    (lm_t *lm,      /**< In/Out: allocated LM */
                 uint32 max_Ng, /**< In: maximum N-gram level in LM */
                 int32 n_ug,    /**< In: number of unigrams */
                 int32 version, /**< In: LM file version number */
                 int32 inmemory /**< In: is LM must be in memory or not */
                 );

/**
 * Like lm_bg_wordprob, but for unigrams.
 * Return value:  \#entries filled in the wordprob array.
 */
int32 lm_ug_wordprob(lm_t *lm, /**< In: LM being queried */
		     dict_t *dict, /**< In : The dictionary */
		     int32 th,
		     wordprob_t *wp /**< In/out: Array to be filled */
    );

/** Return the unigrams in LM.  Return value: \#unigrams in returned list. */
int32 lm_uglist (lm_t *lmp,	/**< In: LM being queried */
		 ug_t **ug	/**< Out: *ug = unigram array */
    );

/* LIUM */
/**
 * \brief Compute N-gram score for the given N words sequence
 *
 * Example for a trigram :
 *  if lwid[0] is BAD_LMWID(lm), return lm_ng_score(n=2, [lwid[1], lwid[2]])
 *  if lwid[0] and lwid[1] are BAD_LMWID(lm), return lm_ng_score(n=1, [lwid[2]])
 *
 *  \return The score
 */
int32
lm_ng_score (lm_t *lmp,         /**< In: LM begin queried */
             uint32 N,          /**< In: N-gram level */
             s3lmwid32_t *lwid, /**< In: array of N LM ID for the words */
             s3wid_t wn         /**< In: dictionary ID for the word */
             );

/* LIUM */
/**
 \brief Whether a certain N-gram exists
 \return True if N-gram exists
 */
int32
lm_ng_exists    (lm_t* lm,          /**< In: the LM */
                 uint32 N,          /**< In: N-gram level */
                 s3lmwid32_t *lwid  /**< In: array of N LM ID for the words */
                 );

/* LIUM */
/**
 \brief Load a N-gram in memory
 */
void
load_ng (lm_t *lm,          /**< In/Out: the LM */
         uint32 N,          /**< In: N-gram level (N>=2) */
         s3lmwid32_t *lwid  /**< In: array of N-1 LM ID for the words */
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


S3DECODER_EXPORT
int32 lm_rawscore (lm_t *lm,  /**< In: the LM */
		   int32 score
    );



/** LM cache related */
S3DECODER_EXPORT
void lm_cache_reset (lm_t *lmp /**< In: the LM */
    );

/** LM cache statistic dumping */
S3DECODER_EXPORT
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
    const char *lmname,  /**<In: LM name*/
    cmd_ln_t *config,
    logmath_t *logmath);

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
		       const char* fmt,       /**< In: file format of the LM, it is
					   now either "TXT", "DMP" and NULL,
					   if NULL, file format is
					   automaticaly determined */
		       int32 applyweight,      /**< In: whether lw,wip, uw should be 
						 applied to the lm or not */
		       logmath_t *logmath
    );

S3DECODER_EXPORT
lm_t *lm_read_advance2(const char *file,	/**< In: LM file being read */
		       const char *lmname,   /**<In: LM name*/
		       float64 lw,	/**< In: Language weight */
		       float64 wip,	/**< In: Word insertion penalty */
		       float64 uw,	/**< In: Unigram weight (interpolation with uniform distr.) */
		       int32 ndict,    /**< In: Number of dictionary entry.  We need that because
					  class-based LM is addressed in dictionary word ID space. 
				       */  
		       const char* fmt,       /**< In: file format of the LM, it is
					   now either "TXT", "DMP" and NULL,
					   if NULL, file format is
					   automaticaly determined */
		       int32 applyweight,      /**< In: whether lw,wip, uw should be 
                                                  applied to the lm or not */
                       int lminmemory, /**< In: Whether LM is read into memory */
		       logmath_t *logmath
    );

/* LIUM */
/**
   Simple writing of an LM file, the input and output encoding will
   assume to be iso8859-1. Call lm_write. To convert encoding, please use
   lm_write_advance. 
*/
S3DECODER_EXPORT
int32 lm_write(lm_t *model, /** In: the pointer LM we want to output */
		   const char *outputdir, /**< In: the output directory */
	       const char *outputfn, /**< In: the output file name */
	       const char *filename, /**< In: the LM file name  */
	       const char *fmt   /**< In: LM file format, it is now either "TXT" or "DMP" */
    );

/* LIUM */
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

   i\\o 0 1 2
   0 - n n
   1 n - y
   2 n x -

   When we have 4 encoding types: This document should be
   implemented as a data structure.

   This conversion table is copied from encoding.c, please take a
   look the latest support in encoding.c
*/

int32 lm_write_advance(lm_t *model, /**< In: the pointer LM we want to output */
			   const char *outputdir, /**< In: the output directory */
		       const char *outputfn, /**< In: the output file name */
		       const char *filename, /**< In: the LM file name  */
		       const char *fmt,   /**< In: LM file format, it is now either "TXT", "DMP", "FST" */
		       const char* inputenc, /**< In: Input encoding type */
		       char* outputenc /**< Out: Output encoding type */
    );

/* RAH, added code for freeing allocated memory 
 */
/**
   Deallocate the language model. 
*/
S3DECODER_EXPORT
void lm_free (lm_t *lm /**< In: a LM structure */
    );

/**
   Add word list to the LM 
   For each word in the file, call lm_add_wordlist. 
   The file is assume to have a format like this:
   \<word1\> 
   \<word2\>
   \<word3\>
   \<word4\>
     
   If the lmwid2dictid mapping is not updated, or the dictionary
   itself is not used in the context.  Just specify dict=NULL;
     
*/
int32 lm_add_wordlist(lm_t *lm, /**< In/Out: a modified LM structure */
		      dict_t *dict, /**< In: an initialized dictionary structure 
				       Used to update 
				    */
		      const char* filename /**< In: a file that contains a
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
			const char* newword /**<In: a pointer of a new word */
    );
/** 
    Get class ID given a LM. 
*/
int32 lm_get_classid (lm_t *model, /**< In: LM file being queried*/
		      const char *name   /**< In: The name of the class */
    );

/**
 * Explicity convert structure from 16bit -> 32bit or 32bit to 16bit. 
 */
void lm_convert_structure(lm_t *model, /**< In: LM file being used */
			  int32 is32bits 
    );	

/**
 \brief Check whether the model is operating at 32-bits 
 \return True if 32-bits
 */
int32
lm_is32bits (int32 version, int32 n_ug);

/**
   Write of UG structure
*/
void ug_write(FILE* fp,  /**< A file pointer */
	      ug_t* ug   /**< A pointer of the ug_t structure */
    );

/* LIUM */
/**
 \brief Write a N-gram structure
 */
void
ng_write    (FILE* fp,  /**< In: file pointer */
             ng_t* ng,  /**< In: pointer to a N-gram (N>=2) */
             uint32 Nc, /**< In: current N level (3 for a 3-grams) */
             uint32 N   /**< In: max N level in LM (4 for a 4-grams LM file) */
             );

/* LIUM */
/**
 \brief Write a 32-bits N-gram structure
 */
void
ng32_write  (FILE * fp,     /**< In: file pointer */
             ng32_t * ng,   /**< In: pointer to a 32-bits N-gram (N>=2) */
             uint32 Nc,     /**< In: current N level (3 for a 3-grams) */
             uint32 N       /**< In: max N level in LM
							 (4 for a 4-grams LM file) */
             );

/* LIUM */
/**
 \brief Convert the 16 bit N-gram structure to 32 bit
 */
void
copy_ng_to_ng32 (lm_t * lm, /**< In/Out: LM */
                 uint32 N   /**< In: N level (3 for a 3-grams) */
                 );

/* LIUM */
/**
 \brief Convert the 32 bit N-gram structure to 16 bit
 */
void
copy_ng32_to_ng (lm_t * lm, /**< In/Out: LM */
                 uint32 N   /**< In: N level (3 for a 3-grams) */
                 );

/* LIUM */
/**
 \brief Swap a 16-bits N-gram
 */
void
swap_ng (ng_t* ng   /**< In/Out: the N-gram */
         );

/* LIUM */
/**
 \brief Swap a 32-bits N-gram
 */
void
swap_ng32   (ng32_t* ng /**< In/Out: the N-gram */
             );

/* LIUM */
/**
 \brief Locate a specific N-gram within a N-gram list
 \return Index of N-gram found
 */
int32
find_ng (ng_t *ng,      /**< In: The N-gram list */
         int32 nsearch, /**< In: Maximum number of N-grams */ 
         s3lmwid32_t w  /**< In: The last word of N-gram */
         );

/* LIUM */
/**
 \brief Locate a specific 32-bits N-gram within a N-gram list
 \return Index of N-gram found
 */
int32
find_ng32   (ng32_t *ng,    /**< In: the 32-bits N-gram list */
             int32 nsearch, /**< In: maximum number of N-grams */  
             s3lmwid32_t w  /**< In: the last word of N-gram */
             );

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
