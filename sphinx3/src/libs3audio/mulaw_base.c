/*
 * mulaw.c -- u-Law audio routines.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2001/12/11  04:10:16  lenzo
 * License.
 * 
 * Revision 1.1.1.1  2001/12/03 16:01:45  egouvea
 * Initial import of sphinx3
 *
 * Revision 1.1  2000/12/12 22:55:55  lenzo
 * Separate out a/d lib.
 *
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:52  lenzo
 * Initial import of sphinx2
 *
 *
 * 
 * 22-Jul-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Sunil's version.
 */


#include "s3types.h"
#include "ad.h"
#include "mulaw.h"


void ad_mu2li (int16 *out, unsigned char *in, int32 n_samp)
{
    int32 i;
    
    for (i = 0; i < n_samp; i++)
	out[i] = muLaw[in[i]];
}
