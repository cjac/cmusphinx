/*
 * byteorder.h -- Byte swapping ordering macros.
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

/* in place byte order conversion
   nothing is promised to be returned
   currently only works for suns and Vax MIPS machines
 */

#if defined(mips) || defined(__alpha) || defined(WIN32) || (! __BIG_ENDIAN__)
#define SWAPBYTES

#define SWAPW(x)	*(x) = ((0xff & (*(x))>>8) | (0xff00 & (*(x))<<8))
#define SWAPL(x)	*(x) = ((0xff & (*(x))>>24) | (0xff00 & (*(x))>>8) |\
			(0xff0000 & (*(x))<<8) | (0xff000000 & (*(x))<<24))
#define SWAPF(x)	SWAPL((int *) x)
#define SWAPP(x)	SWAPL((int *) x)
#define SWAPD(x)	{ int *low = (int *) (x), *high = (int *) (x) + 1,\
			      temp;\
			  SWAPL(low);  SWAPL(high);\
			  temp = *low; *low = *high; *high = temp;}

#else	/* don't need byte order conversion, do nothing */

#define SWAPW(x)
#define SWAPL(x)
#define SWAPF(x)
#define SWAPP(x)
#define SWAPD(x)

#endif


