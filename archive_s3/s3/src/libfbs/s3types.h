/*
 * s3types.h -- Types specific to s3 decoder.
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
 * 12-Jul-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBFBS_S3TYPES_H_
#define _LIBFBS_S3TYPES_H_


#include <libutil/prim_type.h>


/*
 * Size definitions for more semantially meaningful units.
 */
typedef int8	s3cipid_t;	/* Ci phone id */
typedef int32	s3pid_t;	/* Phone id (triphone or ciphone) */
typedef int32	s3wid_t;	/* Dictionary word id */
typedef uint16	s3lmwid_t;	/* LM word id (uint16 for conserving space) */
typedef int32	s3latid_t;	/* Lattice entry id */
typedef int16   s3frmid_t;	/* Frame id (must be SIGNED integer) */
typedef uint16  s3senid_t;	/* Senone id */
typedef int32   s3mgauid_t;	/* Mixture-gaussian codebook id */
typedef int32	s3tmatid_t;	/* Transition matrix id; there can be as many as pids */

/*
 * Illegal value definitions and tests for specific types.
 */
#define BAD_CIPID	((s3cipid_t) -1)
#define NOT_CIPID(p)	((p)<0)
#define IS_CIPID(p)	((p)>=0)
#define MAX_CIPID	127

#define BAD_PID		((s3pid_t) -1)
#define NOT_PID(p)	((p)<0)
#define IS_PID(p)	((p)>=0)
#define MAX_PID		((int32)0x7fffffff)

#define BAD_TMATID	((s3tmatid_t) -1)
#define NOT_TMATID(t)	((t)<0)
#define IS_TMATID(t)	((t)>=0)
#define MAX_TMATID	((int32)0x7fffffff)

#define BAD_WID		((s3wid_t) -1)
#define NOT_WID(w)	((w)<0)
#define IS_WID(w)	((w)>=0)

#define BAD_LMWID	((s3lmwid_t) 0xffff)
#define NOT_LMWID(w)	((w)==BAD_LMWID)
#define IS_LMWID(w)	((w)!=BAD_LMWID)

#define BAD_LATID	((s3latid_t) -1)
#define NOT_LATID(l)	((l)<0)
#define IS_LATID(l)	((l)>=0)

#define BAD_FRMID	((s3frmid_t) -1)
#define NOT_FRMID(f)	((f)<0)
#define IS_FRMID(f)	((f)>=0)

#define BAD_SENID	((s3senid_t) 0xffff)
#define NOT_SENID(s)	((s)==BAD_SENID)
#define IS_SENID(s)	((s)!=BAD_SENID)
#define MAX_SENID	((uint32)0x0000fffe)

#define BAD_MGAUID	((s3mgauid_t) -1)
#define NOT_MGAUID(m)	((m)<0)
#define IS_MGAUID(m)	((m)>=0)


#endif
