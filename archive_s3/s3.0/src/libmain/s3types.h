/*
 * s3types.h -- Types specific to s3 decoder.
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
 * 23-Apr-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed MAX_CIPID from 128 to 127.
 * 
 * 12-Jul-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#ifndef _LIBMAIN_S3TYPES_H_
#define _LIBMAIN_S3TYPES_H_


#include <libutil/prim_type.h>


/*
 * Size definitions for more semantially meaningful units.
 * Illegal value definitions, limits, and tests for specific types.
 */

/* CI phone id */
typedef int8		s3cipid_t;
#define BAD_CIPID	((s3cipid_t) -1)
#define NOT_CIPID(p)	((p)<0)
#define IS_CIPID(p)	((p)>=0)
#define MAX_CIPID	127

/* Phone id (triphone or ciphone) */
typedef int32		s3pid_t;
#define BAD_PID		((s3pid_t) -1)
#define NOT_PID(p)	((p)<0)
#define IS_PID(p)	((p)>=0)
#define MAX_PID		((int32)0x7fffffff)

/* Transition matrix id; perhaps as many as pids */
typedef int32		s3tmatid_t;
#define BAD_TMATID	((s3tmatid_t) -1)
#define NOT_TMATID(t)	((t)<0)
#define IS_TMATID(t)	((t)>=0)
#define MAX_TMATID	((int32)0x7fffffff)

/* Dictionary word id */
typedef int32		s3wid_t;
#define BAD_WID		((s3wid_t) -1)
#define NOT_WID(w)	((w)<0)
#define IS_WID(w)	((w)>=0)
#define MAX_WID		((int32)0x7fffffff)

/* LM word id (uint16 for conserving space) */
typedef uint16		s3lmwid_t;
#define BAD_LMWID	((s3lmwid_t) 0xffff)
#define NOT_LMWID(w)	((w)==BAD_LMWID)
#define IS_LMWID(w)	((w)!=BAD_LMWID)
#define MAX_LMWID	((uint32)0xfffe)

/* Senone id */
typedef int16   	s3senid_t;
#define BAD_SENID	((s3senid_t) -1)
#define NOT_SENID(s)	((s)<0)
#define IS_SENID(s)	((s)>=0)
#define MAX_SENID	((int32)0x7fff)

/* Mixture-gaussian codebook id */
typedef int16   	s3mgauid_t;
#define BAD_MGAUID	((s3mgauid_t) -1)
#define NOT_MGAUID(m)	((m)<0)
#define IS_MGAUID(m)	((m)>=0)
#define MAX_MGAUID	((int16)0x7fff)

/* Frame id (must be SIGNED integer) */
typedef int16   	s3frmid_t;
#define BAD_FRMID	((s3frmid_t) -1)
#define NOT_FRMID(f)	((f)<0)
#define IS_FRMID(f)	((f)>=0)
#define MAX_FRMID	((int32)0x7fff)

/* Lattice entry id */
typedef int32		s3latid_t;
#define BAD_LATID	((s3latid_t) -1)
#define NOT_LATID(l)	((l)<0)
#define IS_LATID(l)	((l)>=0)
#define MAX_LATID	((int32)0x7fffffff)


#endif
