/*
 * mdef.h -- HMM model definition: base (CI) phones and triphones
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
 * 13-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added mdef_phone_str().
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _LIBMAIN_MDEF_H_
#define _LIBMAIN_MDEF_H_


#include "s3types.h"


typedef enum {
    WORD_POSN_BEGIN = 0,	/* Beginning phone of word */
    WORD_POSN_END = 1,		/* Ending phone of word */
    WORD_POSN_SINGLE = 2,	/* Single phone word (i.e. begin & end) */
    WORD_POSN_INTERNAL = 3,	/* Internal phone of word */
    WORD_POSN_UNDEFINED = 4	/* Undefined value, used for initial conditions, etc */
} word_posn_t;
#define N_WORD_POSN	4	/* total # of word positions (excluding undefined) */
#define WPOS_NAME	"besiu"	/* Printable code for each word position above */


/* CI phone information */
typedef struct {
    char *name;
    int32 filler;		/* Whether a filler phone; if so, can be substituted by
				   silence phone in left or right context position */
} ciphone_t;

/*
 * Triphone information, including base phones as a subset.  For the latter, lc, rc
 * and wpos are non-existent.
 */
typedef struct {
    s3senid_t *state;		/* State->senone mappings (senone == tied-state;
				   state must have mdef_t.n_emit_state elements) */
    s3tmatid_t tmat;		/* Transition matrix id */
    s3cipid_t ci, lc, rc;	/* Base, left, right context ciphones */
    word_posn_t wpos;		/* Word position */
} phone_t;

/*
 * Structures needed for mapping <ci,lc,rc,wpos> into pid.  (See mdef_t.wpos_ci_lclist
 * below.)  (lc = left context; rc = right context.)
 * NOTE: Both ph_rc_t and ph_lc_t FOR INTERNAL USE ONLY.
 */
typedef struct ph_rc_s {
    s3cipid_t rc;		/* Specific rc for a parent <wpos,ci,lc> */
    s3pid_t pid;		/* Triphone id for above rc instance */
    struct ph_rc_s *next;	/* Next rc entry for same parent <wpos,ci,lc> */
} ph_rc_t;

typedef struct ph_lc_s {
    s3cipid_t lc;		/* Specific lc for a parent <wpos,ci> */
    ph_rc_t *rclist;		/* rc list for above lc instance */
    struct ph_lc_s *next;	/* Next lc entry for same parent <wpos,ci> */
} ph_lc_t;


/* The main model definition structure */
typedef struct {
    hash_table_t *ciphone_ht;	/* Hash table for mapping ciphone strings to ids */
    ciphone_t *ciphone;		/* CI-phone information for all ciphones */
    phone_t *phone;		/* Information for all ciphones and triphones */
    
    int32 n_ciphone;		/* #basephones actually present */
    int32 n_phone;		/* #basephones + #triphones actually present */
    int32 n_emit_state;		/* #emitting states per phone */
    
    int32 n_ci_sen;		/* #CI senones; these are the first */
    int32 n_sen;		/* #senones (CI+CD) */
    int32 n_tmat;		/* #transition matrices */

    s3senid_t *cd2cisen;	/* Parent CI-senone id for each senone; the first
				   n_ci_sen are identity mappings; the CD-senones are
				   contiguous for each parent CI-phone */
    s3cipid_t *sen2ciphone;	/* Parent CI-phone for each senone (CI or CD) */
    int32 *ciphone2n_cd_sen;	/* #CD-senones for each parent CI-phone */
    
    s3cipid_t sil;		/* SILENCE_CIPHONE id */
    
    ph_lc_t ***wpos_ci_lclist;	/* wpos_ci_lclist[wpos][ci] = list of lc for <wpos,ci>.
				   wpos_ci_lclist[wpos][ci][lc].rclist = list of rc for
				   <wpos,ci,lc>.  Only entries for the known triphones
				   are created to conserve space.
				   (NOTE: FOR INTERNAL USE ONLY.) */
} mdef_t;


/*
 * Initialize the phone structure from the given model definition file.
 * Return value: pointer to the phone structure created.
 * It should be treated as a READ-ONLY structure.
 */
mdef_t *mdef_init (char *mdeffile);


/* Return value: ciphone id for the given ciphone string name */
s3cipid_t mdef_ciphone_id (mdef_t *m,		/* In: Model structure being queried */
			   char *ciphone);	/* In: ciphone for which id wanted */

/* Return value: READ-ONLY ciphone string name for the given ciphone id */
const char *mdef_ciphone_str (mdef_t *m,	/* In: Model structure being queried */
			      s3cipid_t ci);	/* In: ciphone id for which name wanted */

/* Return 1 if given triphone argument is a ciphone, 0 if not, -1 if error */
int32 mdef_is_ciphone (mdef_t *m,		/* In: Model structure being queried */
		       s3pid_t p);		/* In: triphone id being queried */

/* Return value: phone id for the given constituents if found, else BAD_PID */
s3pid_t mdef_phone_id (mdef_t *m,		/* In: Model structure being queried */
		       s3cipid_t b,		/* In: base ciphone id */
		       s3cipid_t l,		/* In: left context ciphone id */
		       s3cipid_t r,		/* In: right context ciphone id */
		       word_posn_t pos);	/* In: Word position */

/*
 * Like phone_id, but backs off to other word positions if exact triphone not found.
 * Also, non-SILENCE_PHONE filler phones back off to SILENCE_PHONE.
 * Ultimately, backs off to base phone id.  Thus, it should never return BAD_PID.
 */
s3pid_t mdef_phone_id_nearest (mdef_t *m,	/* In: Model structure being queried */
			       s3cipid_t b,	/* In: base ciphone id */
			       s3cipid_t l,	/* In: left context ciphone id */
			       s3cipid_t r,	/* In: right context ciphone id */
			       word_posn_t pos);	/* In: Word position */

/*
 * Create a phone string for the given phone (base or triphone) id in the given buf.
 * Return value: 0 if successful, -1 if error.
 */
int32 mdef_phone_str (mdef_t *m,		/* In: Model structure being queried */
		      s3pid_t pid,		/* In: phone id being queried */
		      char *buf);		/* Out: On return, buf has the string */

/*
 * Obtain phone components: inverse of mdef_phone_id().
 * Return value: 0 if successful, -1 otherwise.
 */
int32 mdef_phone_components (mdef_t *m,		/* In: Model structure being queried */
			     s3pid_t p,		/* In: triphone id being queried */
			     s3cipid_t *b,	/* Out: base ciphone id */
			     s3cipid_t *l,	/* Out: left context ciphone id */
			     s3cipid_t *r,	/* Out: right context ciphone id */
			     word_posn_t *pos);	/* Out: Word position */

/*
 * Compare the underlying HMMs for two given phones (i.e., compare the two transition
 * matrix IDs and the individual state(senone) IDs).
 * Return value: 0 iff the HMMs are identical, -1 otherwise.
 */
int32 mdef_hmm_cmp (mdef_t *m,			/* In: Model being queried */
		    s3pid_t p1, s3pid_t p2);	/* In: The two triphones being compared */


/* Some common access functions (macros) */

/* Return TRUE iff p is a fillerphone; p must be a CI-phone */
#define mdef_is_fillerphone(m,p)	((m)->ciphone[p].filler)
#define mdef_n_ciphone(m)		((m)->n_ciphone)
#define mdef_n_emit_state(m)		((m)->n_emit_state)


#endif
