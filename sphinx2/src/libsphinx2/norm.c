/* norm.c - feature normalization
 * 
 * HISTORY
 * 
 * 20-Aug-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Extended normalization to C0.
 * 
 |------------------------------------------------------------*
 | $Header$
 |------------------------------------------------------------*
 | Description
 |	norm_mean()	- compute the mean of the input vectors
 |			  and then subtract the mean from the
 |			  input vectors. Leave coefficient 0
 |			  untouched.
 */

#include <stdlib.h>

#include <s2types.h>

#ifdef DEBUG
#define dprintf(x)	printf x
#else
#define dprintf(x)
#endif

void
norm_mean(float	*vec,		/* the data */
	  int32	nvec,		/* number of vectors (frames) */
	  int32	veclen)		/* number of elements (coefficients) per vector */
{
    static double      *mean = 0;
    float              *data;
    int32               i, f;

    if (mean == 0)
	mean = (double *) calloc (veclen, sizeof (double));

    for (i = 0; i < veclen; i++)
	mean[i] = 0.0;

    /*
     * Compute the sum
     */
    for (data = vec, f = 0; f < nvec; f++, data += veclen) {
	for (i = 0; i < veclen; i++)
	    mean[i] += data[i];
    }

    /*
     * Compute the mean
     */
    dprintf(("Mean Vector\n"));
    for (i = 0; i < veclen; i++) {
	mean[i] /= nvec;
	dprintf(("[%d]%.3f, ", i, mean[i]));
    }
    dprintf(("\n"));
    
    /*
     * Normalize the data
     */
    for (data = vec, f = 0; f < nvec; f++, data += veclen) {
	for (i = 0; i < veclen; i++)
	    data[i] -= mean[i];
    }
}
