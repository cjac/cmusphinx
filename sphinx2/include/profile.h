/*
 * profile.h -- Profiling (timing and frequency counts) code
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1995 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 16-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <s2types.h>

int32 timer_new (char *name);
void  timer_resume (int32 id);
void  timer_pause (int32 id);
int32 timer_accum_time (int32 id);
char *timer_name (int32 id);
void  timer_reset (int32 id);
void  timer_reset_all ( void );

int32 counter_new (char *name);
void  counter_increment (int32 id, int32 inc);
int32 counter_value (int32 id);
char *counter_name (int32 id);
void  counter_reset (int32 id);
void  counter_reset_all ( void );

#endif
