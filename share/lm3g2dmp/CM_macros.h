/* CM_macros.h
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 *------------------------------------------------------------*
 * DESCRIPTION
 *	CMPSL macros to make error handling a little easier.
 *------------------------------------------------------------*
 * HISTORY
 * Spring 89, Fil Alleva (faa) Carnegie Mellon
 *	Created
 */

#ifndef _CM_MACROS_H_
#define _CM_MACROS_H_

#include "primtype.h"


/* CM_fopen
 *------------------------------------------------------------*
 */ 
#define CM_fopen(name,mode)	_CM_fopen((name),(mode),__FILE__,__LINE__)

/* CM_fopenp
 *------------------------------------------------------------*
 */ 
#define CM_fopenp(dirl,name,mode)    _CM_fopenp((dirl),(name),(mode),__FILE__,__LINE__)


/* CM_fread
 *------------------------------------------------------------*
 */ 
#define CM_fread(ptr,size,cnt,stream)			\
{							\
    char mesg[1024];					\
    if (fread(ptr,size,cnt,stream) != cnt) {		\
	sprintf (mesg, "%s(%d): fread failed",		\
		 __FILE__, __LINE__);			\
	perror (mesg);					\
	exit (-1);					\
    }							\
}


/* CM_calloc
 *------------------------------------------------------------*
 */ 
#define CM_calloc(cnt,size)	_CM_calloc((cnt),(size),__FILE__,__LINE__)


/* CM_2dcalloc
 *------------------------------------------------------------*
 */ 
#define CM_2dcalloc(rcnt,ccnt,size) _CM_2dcalloc((rcnt),(ccnt),(size),__FILE__,__LINE__)

				
/* CM_3dcalloc
 *------------------------------------------------------------*
 */ 
#define CM_3dcalloc(lcnt,rcnt,ccnt,size) \
			_CM_3dcalloc((lcnt),(rcnt),(ccnt),(size),__FILE__,__LINE__)

				
/* CM_recalloc
 *------------------------------------------------------------*
 */ 
#define CM_recalloc(ptr,cnt,size)     _CM_recalloc((ptr),(cnt),(size),__FILE__,__LINE__)


extern FILE *_CM_fopen();
extern FILE *_CM_fopenp();

extern char *_CM_calloc();
extern char *_CM_2dcalloc();
extern char *_CM_3dcalloc();
extern char *_CM_recalloc();

#endif  /* _CM_MACROS_H_ */
