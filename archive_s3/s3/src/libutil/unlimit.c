/*
 * unlimit.c -- "unlimit" the memory usage of program.
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
 * 03-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Copied from Sphinx-II sources.
 */


#if ((! WIN32) && (! _HPUX_SOURCE))
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "unlimit.h"


/*
 * The limit is set to the maximum of your system.
 */
void unlimit ( void )
{
#if ((! WIN32) && (! _HPUX_SOURCE))
  struct rlimit rl;

  getrlimit(RLIMIT_DATA,&rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_DATA,&rl);
#endif
}
