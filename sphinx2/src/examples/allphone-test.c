/*
 * allphone-test.c -- Test of a simple driver for allphone decoding.o
 * 
 * HISTORY
 *
 * 08-Feb-2000  Kevin Lenzo <lenzo@cs.cmu.edu> at Carnegie Mellon University
 *              Changed wording and routine name.
 * 
 * 
 * 11-Sep-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "s2types.h"
#include "fbs.h"


/*
 * This application reads in filenames, one at a time, and performs allphone
 * recognition on each.  The result is written to stdout.  (But note that the
 * recognition engine also writes a log to stdout.  So a real application might
 * prefer to write the results to a file instead.)
 */

int
main (int argc, char *argv[])
{
    char utt[4096];
    search_hyp_t *h;
    
    /* argc, argv: The usual argument for batchmode allphone decoding. */
    fbs_init (argc, argv);

    for (;;) {
	printf ("Audio filename (without extension): ");
	if (scanf ("%s", utt) != 1)
	    break;

	printf ("%s:\n", utt);

	h = uttproc_allphone_file (utt);
	
	for (; h; h = h->next)
	    printf ("%4d %4d %s\n", h->sf, h->ef, h->word);
	printf ("\n");
    }
    
    fbs_end();
    return 0;
}



