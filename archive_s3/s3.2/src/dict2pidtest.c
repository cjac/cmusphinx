/*
 * dict2pidtest.c
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
 * 05-May-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include "dict2pid.h"
#include "logs3.h"


static void usage (char *pgm)
{
    E_INFO("Usage: %s mdeffile dictfile fillerfile\n", pgm);
    exit(0);
}


main (int32 argc, char *argv[])
{
    mdef_t *mdef;
    dict_t *dict;
    dict2pid_t *d2p;
    
    if (argc != 4)
	usage(argv[0]);
    
    logs3_init (1.0003);
    
    mdef = mdef_init (argv[1]);
    dict = dict_init (mdef, argv[2], argv[3], 0);
    
    d2p = dict2pid_build (mdef, dict);
    dict2pid_dump (stdout, d2p, mdef, dict);
}
