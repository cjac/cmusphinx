/*
 * mdef.h -- HMM model definition: base (CI) phones and triphones
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
 * 13-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added mdef_phone_str().
 * 
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _LIBFBS_MDEF_H_
#define _LIBFBS_MDEF_H_


#include "s3types.h"

#include <libutil/libutil.h>


typedef enum {
    WORD_POSN_BEGIN = 0,	/* Beginning phone of word */
    WORD_POSN_END = 1,		/* Ending phone of word */
    WORD_POSN_SINGLE = 2,	/* Single phone word (i.e. begin & end) */
    WORD_POSN_INTERNAL = 3,	/* Internal phone of word */
    WORD_POSN_UNDEFINED = 4	/* Undefined value, used for initial conditions, etc */
} word_posn_t;

#define WPOS_NAME	"besiu"	/* Printable code for each word position above */
#define N_WORD_POSN	4	/* total # of word positions (excluding undefined) */


/* CI phone information */
typedef struct {
    char *name;
    int32 filler;		/* Whether a filler phone */
} ciphone_t;

/*
 * Triphone information, including base phones as a subset.  For the latter, lc, rc
 * and wpos are non-existent.
 */
typedef struct {
    s3senid_t *state;		/* State->senone mappings */
    s3tmatid_t tmat;		/* Transition matrix id */
    s3cipid_t ci, lc, rc;	/* Base, left, right context ciphones */
    word_posn_t wpos;		/* Word position */
} phone_t;

/*
 * Structures needed for mapping <ci,lc,rc,wpos> into pid.
 */
typedef struct ph_rc_s {
    s3cipid_t rc;
    s3pid_t pid;
    struct ph_rc_s *next;
} ph_rc_t;

typedef struct ph_lc_s {
    s3cipid_t lc;
    ph_rc_t *rclist;	/* ptr to list of rc for parent <wpos,ci>, this lc */
    struct ph_lc_s *next;
} ph_lc_t;


/* The main model definition structure */
typedef struct {
    hash_t ciphone_ht;		/* Hash table for mapping ciphone strings to ids */
    ciphone_t *ciphone;		/* CI-phone information for all ciphones */
    phone_t *phone;		/* Information for all ciphones and triphones */

    s3cipid_t sil;		/* SILENCE_CIPHONE id */
    
    int32 n_ciphone;		/* #basephones */
    int32 n_phone;		/* #basephones + #triphones */
    int32 n_emit_state;		/* #emitting states / phone */

    int32 n_ci_sen;		/* #CI senones */
    int32 n_sen;		/* #senones (CI+CD) */
    int32 n_tmat;		/* #transition matrices */
    s3senid_t *cd2cisen;	/* Parent CI-senone id for each senone */
    
    ph_lc_t ***wpos_ci_lclist;	/* wpos_ci[wpos][ci] = list of lc for <wpos,ci>.
				   wpos_ci[wpos][ci][lc].rclist = list of rc for
				   <wpos,ci,lc>.  Entries for only the known triphones
				   are created, to conserve space. */
} mdef_t;


/* -------------------------------- INTERFACE -------------------------------- */


/*
 * Initialize the phone structure from the given model definition file.
 * Return value: pointer to the phone structure created.
 * It should be treated as a READ-ONLY structure.
 */
mdef_t *mdef_init (char *mdeffile);

/*
 * Return value: pointer to the model definition structure (assuming only one exists).
 * It should be treated as a READ-ONLY structure.
 */
mdef_t *mdef_getmdef ( void );


/* Return value: phone id for the given constituents if found, else BAD_PID */
s3pid_t mdef_phone_id (mdef_t *m,		/* In: Model structure being queried */
		       s3cipid_t b,		/* In: base ciphone id */
		       s3cipid_t l,		/* In: left context ciphone id */
		       s3cipid_t r,		/* In: right context ciphone id */
		       word_posn_t pos);	/* In: Word position */

/*
 * Like phone_id, but backs off to other word positions if exact triphone not found.
 * Ultimately, backs off to base phone id.  Thus, it should never return BAD_PID.
 */
s3pid_t mdef_phone_id_nearest (mdef_t *m,	/* In: Model structure being queried */
			       s3cipid_t b,	/* In: base ciphone id */
			       s3cipid_t l,	/* In: left context ciphone id */
			       s3cipid_t r,	/* In: right context ciphone id */
			       word_posn_t pos);	/* In: Word position */

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

/* Return value: ciphone id for the given ciphone string name */
s3cipid_t mdef_ciphone_id (mdef_t *m,		/* In: Model structure being queried */
			   char *ciphone);	/* In: ciphone for which id wanted */

/* Return value: READ-ONLY ciphone string name for the given ciphone id */
char *mdef_ciphone_str (mdef_t *m,		/* In: Model structure being queried */
			s3cipid_t ci);		/* In: ciphone id for which name wanted */

/* Return 1 if given triphone argument is a ciphone, 0 if not, -1 if error */
s3cipid_t mdef_is_ciphone (mdef_t *m,		/* In: Model structure being queried */
			   s3pid_t p);		/* In: triphone id being queried */

/*
 * Create a phone string for the given phone (base or triphone) id in the given buf.
 * Return value: 0 if successful, -1 if error.
 */
int32 mdef_phone_str (mdef_t *m,		/* In: Model structure being queried */
		      s3pid_t pid,		/* In: phone id being queried */
		      char *buf);		/* Out: On return, buf has the string */

#endif	/* _MDEF_H_ */
