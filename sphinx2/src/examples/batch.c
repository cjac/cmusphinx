/*
 * batch.c -- dummy (stub) user interface module for batch mode operation.
 * 
 * HISTORY
 * 
 * 17-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Copied from an earlier version
 */

#include <stdio.h>
#include "s2types.h"
#include "fbs.h"

int
main (int32 argc, char *argv[])
{
    fbs_init (argc, argv);
    /* That is it; fbs_init takes care of batch-mode processing */

    fbs_end ();
    return 0;
}
