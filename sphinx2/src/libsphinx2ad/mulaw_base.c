/*
 * mulaw.c -- u-Law audio routines.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2001/12/07  04:27:35  lenzo
 * License cleanup.  Remove conditions on the names.  Rationale: These
 * conditions don't belong in the license itself, but in other fora that
 * offer protection for recognizeable names such as "Carnegie Mellon
 * University" and "Sphinx."  These changes also reduce interoperability
 * issues with other licenses such as the Mozilla Public License and the
 * GPL.  This update changes the top-level license files and removes the
 * old license conditions from each of the files that contained it.
 * All files in this collection fall under the copyright of the top-level
 * LICENSE file.
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


#include "s2types.h"
#include "ad.h"
#include "mulaw.h"


void ad_mu2li (int16 *out, unsigned char *in, int32 n_samp)
{
    int32 i;
    
    for (i = 0; i < n_samp; i++)
	out[i] = muLaw[in[i]];
}
