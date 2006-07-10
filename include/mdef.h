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
 * mdef.h -- HMM model definition: base (CI) phones and triphones
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
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.13  2006/02/22 16:52:51  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed memory leaks in mdef. 2,  Fixed $, 3, Fixed dox-doc.
 *
 * Revision 1.12.4.2  2005/07/05 05:47:59  arthchan2003
 * Fixed dox-doc. struct level of documentation are included.
 *
 * Revision 1.12.4.1  2005/07/03 22:54:09  arthchan2003
 * move st2senmap into mdef_t, it was not properly freed before. \n
 *
 * Revision 1.12  2005/06/21 18:47:39  arthchan2003
 * Log. 1, Added breport flag to mdef_init, 2, implemented reporting functions to
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added Revision 1.13  2006/02/22 16:52:51  arthchan2003
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added Merged from SPHINX3_5_2_RCI_IRII_BRANCH: 1, Fixed memory leaks in mdef. 2,  Fixed $, 3, Fixed dox-doc.
 * mdef_report. 3, Fixed doxygen-style documentation. 4, Added
 *
 * Revision 1.5  2005/06/13 04:02:55  archan
 * Fixed most doxygen-style documentation under libs3decoder.
 *
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 19.Apr-2001  Ricky Houghton, added code for free allocated memory
 * 
 * 14-Oct-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added mdef_sseq2sen_active().
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added senone-sequence id (ssid) to phone_t and appropriate functions to
 * 		maintain it.  Instead, moved state sequence info to mdef_t.
 * 
 * 13-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added mdef_phone_str().
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _S3_MDEF_H_
#define _S3_MDEF_H_

#include "s3types.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** \file mdef.h
 * \brief Model definition 
 */

/** \enum word_posn_t
 * \brief Union of different type of word position
 */

typedef enum {
    WORD_POSN_BEGIN = 0,	/**< Beginning phone of word */
    WORD_POSN_END = 1,		/**< Ending phone of word */
    WORD_POSN_SINGLE = 2,	/**< Single phone word (i.e. begin & end) */
    WORD_POSN_INTERNAL = 3,	/**< Internal phone of word */
    WORD_POSN_UNDEFINED = 4	/**< Undefined value, used for initial conditions, etc */
} word_posn_t;
#define N_WORD_POSN	4	/**< total # of word positions (excluding undefined) */
#define WPOS_NAME	"besiu"	/**< Printable code for each word position above */


/**
   \struct ciphone_t
   \brief CI phone information 
*/
typedef struct {
    char *name;                 /**< The name of the CI phone */
    int32 filler;		/**< Whether a filler phone; if so, can be substituted by
				   silence phone in left or right context position */
} ciphone_t;

/**
 * \struct phone_t
 * \brief Triphone information, including base phones as a subset.  For the latter, lc, rc and wpos are non-existent.
 */
typedef struct {
    s3ssid_t ssid;		/**< State sequence (or senone sequence) ID, considering the
				   n_emit_state senone-ids are a unit.  The senone sequences
				   themselves are in a separate table */
    s3tmatid_t tmat;		/**< Transition matrix id */
    s3cipid_t ci, lc, rc;	/**< Base, left, right context ciphones */
    word_posn_t wpos;		/**< Word position */
    s3senid_t *state;           /**< State->senone mappings */
    
} phone_t;

/**
 * \struct ph_rc_t
 * \brief Structures needed for mapping <ci,lc,rc,wpos> into pid.  (See mdef_t.wpos_ci_lclist below.)  (lc = left context; rc = right context.)
 * NOTE: Both ph_rc_t and ph_lc_t FOR INTERNAL USE ONLY.
 */
typedef struct ph_rc_s {
    s3cipid_t rc;		/**< Specific rc for a parent <wpos,ci,lc> */
    s3pid_t pid;		/**< Triphone id for above rc instance */
    struct ph_rc_s *next;	/**< Next rc entry for same parent <wpos,ci,lc> */
} ph_rc_t;

/**
 * \struct ph_lc_t
 * \brief Structures for storing the left context. 
 */

typedef struct ph_lc_s {
    s3cipid_t lc;		/**< Specific lc for a parent <wpos,ci> */
    ph_rc_t *rclist;		/**< rc list for above lc instance */
    struct ph_lc_s *next;	/**< Next lc entry for same parent <wpos,ci> */
} ph_lc_t;


/** The main model definition structure */
/**
   \struct mdef_t
   \brief strcture for storing the model definition. 
*/
typedef struct {
    int32 n_ciphone;		/**< #basephones actually present */
    int32 n_phone;		/**< #basephones + #triphones actually present */
    int32 n_emit_state;		/**< #emitting states per phone */
    int32 n_ci_sen;		/**< #CI senones; these are the first */
    int32 n_sen;		/**< #senones (CI+CD) */
    int32 n_tmat;		/**< #transition matrices */
    
    hash_table_t *ciphone_ht;	/**< Hash table for mapping ciphone strings to ids */
    ciphone_t *ciphone;		/**< CI-phone information for all ciphones */
    phone_t *phone;		/**< Information for all ciphones and triphones */
    s3senid_t **sseq;		/**< Unique state (or senone) sequences in this model, shared
                                   among all phones/triphones */
    int32 n_sseq;		/**< No. of unique senone sequences in this model */
    
    s3senid_t *cd2cisen;	/**< Parent CI-senone id for each senone; the first
				   n_ci_sen are identity mappings; the CD-senones are
				   contiguous for each parent CI-phone */
    s3cipid_t *sen2cimap;	/**< Parent CI-phone for each senone (CI or CD) */
    int32 *ciphone2n_cd_sen;	/**< #CD-senones for each parent CI-phone */
    
    s3cipid_t sil;		/**< SILENCE_CIPHONE id */
    
    ph_lc_t ***wpos_ci_lclist;	/**< wpos_ci_lclist[wpos][ci] = list of lc for <wpos,ci>.
                                   wpos_ci_lclist[wpos][ci][lc].rclist = list of rc for
                                   <wpos,ci,lc>.  Only entries for the known triphones
                                   are created to conserve space.
                                   (NOTE: FOR INTERNAL USE ONLY.) */
  
    s3senid_t *st2senmap; /**< A mapping from State to senone. Only used
                             in sphinx 3.0 HACK!, In general, there is
                             only need for either one of st2senmap or
                             sseq. 
                          */
} mdef_t;

/** Access macros; not meant for arbitrary use */
#define mdef_is_fillerphone(m,p)	((m)->ciphone[p].filler)
#define mdef_n_ciphone(m)		((m)->n_ciphone)
#define mdef_n_phone(m)			((m)->n_phone)
#define mdef_n_sseq(m)			((m)->n_sseq)
#define mdef_n_emit_state(m)		((m)->n_emit_state)
#define mdef_n_sen(m)			((m)->n_sen)
#define mdef_n_tmat(m)			((m)->n_tmat)
#define mdef_pid2ssid(m,p)		((m)->phone[p].ssid)
#define mdef_pid2tmatid(m,p)		((m)->phone[p].tmat)
#define mdef_silphone(m)		((m)->sil)
#define mdef_sen2cimap(m)		((m)->sen2cimap)
#define mdef_cd2cisen(m)		((m)->cd2cisen)

/**
 * Initialize the phone structure from the given model definition file.
 * It should be treated as a READ-ONLY structure.
 * @return pointer to the phone structure created.
 */
mdef_t *mdef_init (char *mdeffile, /**< In: Model definition file */
		   int32 breport   /**< In: whether to report the progress or not */
    );


/** 
    Get the ciphone id given a string name
    @return ciphone id for the given ciphone string name 
*/
s3cipid_t mdef_ciphone_id (mdef_t *m,		/**< In: Model structure being queried */
			   char *ciphone	/**< In: ciphone for which id wanted */
    );

/** 
    Get the phone string given the ci phone id.
    @return: READ-ONLY ciphone string name for the given ciphone id 
*/
const char *mdef_ciphone_str (mdef_t *m,	/**< In: Model structure being queried */
			      s3cipid_t ci	/**< In: ciphone id for which name wanted */
    );

/** 
    Decide whether the phone is ci phone.
    @return 1 if given triphone argument is a ciphone, 0 if not, -1 if error 
*/
int32 mdef_is_ciphone (mdef_t *m,		/**< In: Model structure being queried */
		       s3pid_t p		/**< In: triphone id being queried */
    );

/**
   Decide whether the senone is a senone for a ci phone, or a ci senone
   @return 1 if a given senone is a ci senone
*/  
int32 mdef_is_cisenone(mdef_t *m,               /**< In: Model structure being queried */
		       s3senid_t s            /**< In: senone id being queried */
    );

/** 
    Decide the phone id given the left, right and base phones. 
    @return: phone id for the given constituents if found, else BAD_S3PID 
*/
s3pid_t mdef_phone_id (mdef_t *m,		/**< In: Model structure being queried */
		       s3cipid_t b,		/**< In: base ciphone id */
		       s3cipid_t l,		/**< In: left context ciphone id */
		       s3cipid_t r,		/**< In: right context ciphone id */
		       word_posn_t pos	/**< In: Word position */
    );

/**
 * Like phone_id, but backs off to other word positions if exact triphone not found.
 * Also, non-SILENCE_PHONE filler phones back off to SILENCE_PHONE.
 * Ultimately, backs off to base phone id.  Thus, it should never return BAD_S3PID.
 */
s3pid_t mdef_phone_id_nearest (mdef_t *m,	/**< In: Model structure being queried */
			       s3cipid_t b,	/**< In: base ciphone id */
			       s3cipid_t l,	/**< In: left context ciphone id */
			       s3cipid_t r,	/**< In: right context ciphone id */
			       word_posn_t pos	/**< In: Word position */
    );

/**
 * Create a phone string for the given phone (base or triphone) id in the given buf.
 * @return 0 if successful, -1 if error.
 */
int32 mdef_phone_str (mdef_t *m,		/**< In: Model structure being queried */
		      s3pid_t pid,		/**< In: phone id being queried */
		      char *buf		/**< Out: On return, buf has the string */
    );

/**
 * Obtain phone components: inverse of mdef_phone_id().
 * @return 0 if successful, -1 otherwise.
 */
int32 mdef_phone_components (mdef_t *m,		/**< In: Model structure being queried */
			     s3pid_t p,		/**< In: triphone id being queried */
			     s3cipid_t *b,	/**< Out: base ciphone id */
			     s3cipid_t *l,	/**< Out: left context ciphone id */
			     s3cipid_t *r,	/**< Out: right context ciphone id */
			     word_posn_t *pos	/**< Out: Word position */
    );

/**
 * Compare the underlying HMMs for two given phones (i.e., compare the two transition
 * matrix IDs and the individual state(senone) IDs).
 * @return 0 iff the HMMs are identical, -1 otherwise.
 */
int32 mdef_hmm_cmp (mdef_t *m,			/**< In: Model being queried */
		    s3pid_t p1, /**< In: One of the two triphones being compared */
		    s3pid_t p2	/**< In: One of the two triphones being compared */
    );

/**
 * From the given array of active senone-sequence flags, mark the corresponding senones that
 * are active.  Caller responsible for allocating sen[], and for clearing it, if necessary.
 */
void mdef_sseq2sen_active (mdef_t *mdef,        /**< In: The model definition */
			   int32 *sseq,		/**< In: sseq[ss] is != 0 iff senone-sequence ID
						   ss is active */
			   int32 *sen		/**< In/Out: Set sen[s] to non-0 if so indicated
						   by any active senone sequence */
    );

/** For debugging: dump the mdef_t structure out. */
void mdef_dump (FILE *fp,  /**< In: a file pointer */
		mdef_t *m  /**< In: a model definition structure */
    );

/** Report the model definition's parameters */
void mdef_report(mdef_t *m /**<  In: model definition structure */
    );

/** RAH, For freeing memory */
void mdef_free_recursive_lc (ph_lc_t *lc /**< In: A list of left context */
    );
void mdef_free_recursive_rc (ph_rc_t *rc /**< In: A list of right context */
    );

/** Free an mdef_t */
void mdef_free (mdef_t *mdef /**< In : The model definition*/
    );


#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
