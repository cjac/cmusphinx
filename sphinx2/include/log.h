/*
 * log.h -- LOG based probability ops
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
 * 16-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Fil Alleva's original.
 */


#ifndef _LOG_H_
#define _LOG_H_

#include <math.h>

extern int16 Addition_Table[];
extern int32 Table_Size;

#define BASE		1.0001
#define LOG_BASE	9.9995e-5
#define MIN_LOG		-690810000
#define MIN_DOUBLE	1e-300
/*
 * Note that we are always dealing with values between 0.0 and 1.0 so the
 * log(x) is < 0.0 so the rounding uses -0.5 instead of +0.5
 */
#define LOG(x)	((x == 0.0) ? MIN_LOG :					\
			      ((x > 1.0) ?				\
				 (int32) ((log (x) / LOG_BASE) + 0.5) :	\
				 (int32) ((log (x) / LOG_BASE) - 0.5)))

#define EXP(x)	(exp ((double) (x) * LOG_BASE))

#define ADD(x,y)	((x) > (y) ?					\
			 (((y) <= MIN_LOG || (x) - (y) >= Table_Size) ?	\
			  (x) : Addition_Table[(x) - (y)] + (x))	\
			 :						\
			 (((x) <= MIN_LOG || (y) - (x) >= Table_Size) ?	\
			  (y) : Addition_Table[(y) - (x)] + (y)))

#define FAST_ADD(res, x, y, table, table_size)		\
{							\
	int32 _d = (x) - (y);				\
	if (_d > 0) { /* x >= y */			\
		if (_d >= (table_size))			\
			res = (x);			\
		else					\
			res = (table)[_d] + (x);	\
	} else { /* x < y */				\
		if (-_d >= (table_size))		\
			res = (y);			\
		else					\
			res = (table)[-_d] + (y);	\
	}						\
}

/*
 * Note that we always deal with quantities < 0.0, there for we round
 * using -0.5 instead of +0.5
 */ 
#define LOG10TOLOG(x)	((int32)((x * (2.30258509 / LOG_BASE)) - 0.5))

#endif /* _LOG_H_ */
