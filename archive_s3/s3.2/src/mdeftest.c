/*
 * mdeftest.c --
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 30-Apr-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include "mdef.h"


main (int32 argc, char *argv[])
{
    mdef_t *m;
    
    if ((argc != 2) && (argc != 3))
	E_FATAL("Usage: %s mdeffile [dump]\n", argv[0]);
    
    m = mdef_init (argv[1]);
    
    if (argc == 3)
	mdef_dump (stdout, m);

    exit(0);
}
