/* ====================================================================
 * Copyright (c) 1987-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/* 
 *------------------------------------------------------------*
 * DESCRIPTION
 *	Read tied distribution hmms on behalf of fbs.
 *------------------------------------------------------------*
 * HISTORY
 *
 * 23-Jan-01    H J Fox (hjf@cs.brown.edu) at Brown University
 *              Hacked to run under Solaris 8 - flipped the byte swap
 *              condition.
 * 
 * Revision 1.4  2001/01/25 19:36:29  lenzo
 * Fixing some memory leaks
 *
 * Revision 1.3  2000/12/12 23:01:42  lenzo
 * Rationalizing libs and names some more.  Split a/d and fe libs out.
 *
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:50  lenzo
 * Initial import of sphinx2
 *
 * Revision 8.4  94/05/10  10:46:37  rkm
 * Added original .map file timestamp info to map dump file.
 * 
 * Revision 8.3  94/04/22  13:51:27  rkm
 * *** empty log message ***
 * 
 * 14-Apr-94  M K Ravishankar (rkm@cs) at Carnegie-Mellon University
 * 	Added map-file dump and load facility for faster startup.
 * 
 *  2-Nov-91  Fil Alleva (faa) at Carnegie-Mellon University
 *	Massive changes for fbs.
 *
 * 16-Mar-87 Hsiao-wuen Hon (Hon)  at Carnegie Mellon University.
 *	Initial version.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#include "s2types.h"
#include "CM_macros.h"
#include "basic_types.h"
#include "list.h"
#include "hash.h"
#include "phone.h"
#include "search_const.h"
#include "msd.h"
#include "magic.h"
#include "log.h"
#include "cviterbi4.h"
#include "smmap4f.h"
#include "dict.h"
#include "lmclass.h"
#include "lm_3g.h"
#include "kb.h"
#include "hmm_tied_r.h"
#include "logmsg.h"

#define QUIT(x)		{fprintf x; exit(-1);}

#define MAX_ALPHABET		256

#define HAS_ARC(fs,ts,S)	S->topo[((fs * S->stateCnt) + ts) / 32] & \
				(1 << ((fs * S->stateCnt) + ts) % 32)

int32 totalDists;		/* Total number of distributions */
int32 *numDists;		/* Number of senones for each of the CI
				 * and WD base phone units.
				 */
int32 *numDPDists;		/* Number of DiPhone senones. These are
				 * generated from the appropriate cd units.
				 */
static int32 **distMap;		/* Distribution map
				 */
static int32 *ssIdMap;		/* Senone sequence Id Map.
				 * ssIdMap[phone_id] == the senone sequence id.
				 */
static int32 numSSeq = 0;	/* Number of unique senone sequences
				 */

int32 *Out_Prob0;
int32 *Out_Prob1;
int32 *Out_Prob2;
int32 *Out_Prob3;
int32 *Out_Prob4;

/*
 * Out_Prob<k> compressed to 16 bits.  We lose a few significant bits, but it
 * doesn't affect the word accuracy.
 */
extern int16 *out_prob_16b[];
int16 *out_prob_16b[4];		/* for the 4 features */

/*
 * Out_Prob<k> compressed to 8 bits, by clustering all senone probs for each
 * codeword to 256 unique values.  See OPDF_8BIT_T definition for details.
 */

typedef struct {
    int32 **prob;           /* 2-D array, #codewords x 256 */
    unsigned char **id;     /* 2-D array, #codewords x #senones */
} OPDF_8BIT_T;

extern OPDF_8BIT_T out_prob_8b[];
OPDF_8BIT_T out_prob_8b[4];	/* for the 4 features */

static void dist_read (
	char const *file,
	int32 expected,
	int32 useCiDistsOnly,
	int32 *o1, char const *Code_Ext1,
	int32 *o2, char const *Code_Ext2,
	int32 *o3, char const *Code_Ext3,
	int32 *o4, char const *Code_Ext4);
static void hmm_tied_bin_parse (FILE *fp, SMD *smd,
				double transSmooth,
				int32 numAlphaExpected,
				int norm,
				double arcWeight, 
				int doByteSwap,
				char const *hmmName);
static void add_senone (int32 s1, int32 s2);
static void compute_diphone_senones(void);
static void remove_all_members (int32 s);
static void zero_senone (int32 s);
static void normalize_dists (int numAlphabet, double SmoothMin);
static void normalize_out (register int32 *out,
			   double weight, int32 numAlphabet);
static void normalize_trans (SMD *smd, SMD_R *smd_r, double weight);
static void transpose (int32 *pa, int32 const *a, int32 rows, int32 cols);
static void insert_floor (register int32 *out, int32 min,
			  int32 numAlphabet);

static int hmmArcNormalize (SMD *smd, SMD_R *smd_r,
			    double transSmooth, double arcWeight);

/* FIXME: ARGH!  These are exactly the opposite of the similarly named
   macros in "byteorder.h".  And then we also have a set of functions
   which also swap bytes.  */
/* put a define __BIG_ENDIAN__ 1 here for suns/solaris */
#if (__BIG_ENDIAN__)
#define SWAP_W(x)	x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define SWAP_L(x)	x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
    			      (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )
#else
#define SWAP_W(x)
#define SWAP_L(x)
#endif

/* FIXME: Yes, this one swaps unconditionally, and it's a
   function... !@#$%@!#$ (from util.c) */
extern void swapLong(int32 *intp);

/* FIXME: needs a header file */
extern int areadint (char *file, int **data_ref, int *length_ref);


static int fread_int32(FILE *fp, int min, int max, char const *name)
{
    int k;
    
    if (fread (&k, sizeof (int), 1, fp) != 1)
	QUIT((stdout, "%s(%d): fread(%s) failed\n", __FILE__, __LINE__, name));
    SWAP_L(k);
    if ((min > k) || (max < k))
	QUIT((stdout, "%s(%d): %s outside range [%d,%d]\n", __FILE__, __LINE__, name, min, max));
    return (k);
}

static size_t fwrite_int32 (FILE *fp, int val)
{
    SWAP_L(val);
    return fwrite (&val, sizeof(int), 1, fp);
}

/*
 * load_senone_dists_8bits:  Allocate space for and load precompiled senone prob
 * distributions file.
 * The precompiled file contains senone probs smoothed, normalized and compressed
 * according to the definition of type OPDF_8BIT_T.
 */
static void
load_senone_dists_8bits(OPDF_8BIT_T p[],	/* Output probs, clustered */
			int32 r, int32 c,	/* #rows, #cols */
			char const *file,	/* dumped probs */
			char const *dir)	/* original hmm directory */
{
    FILE *fp;
    char line[1000];
    size_t n;
    int32 i;
    
    log_info("%s(%d): Loading HMMs from dump file %s\n",
	     __FILE__, __LINE__, file);
    
    fp = CM_fopen (file, "rb");

    /* Read title size, title */
    n = fread_int32 (fp, 1, 999, "Title length");
    if (fread (line, sizeof(char), n, fp) != n)
	QUIT((stdout, "%s(%d): Cannot read title\n", __FILE__, __LINE__));
    if (line[n-1] != '\0')
	QUIT((stdout, "%s(%d): Bad title in dump file\n", __FILE__, __LINE__));
    log_info("%s(%d): %s\n", __FILE__, __LINE__, line);
    
    /* Read header size, header */
    n = fread_int32 (fp, 1, 999, "Header length");
    if (fread (line, sizeof(char), n, fp) != n)
	QUIT((stdout, "%s(%d): Cannot read header\n", __FILE__, __LINE__));
    if (line[n-1] != '\0')
	QUIT((stdout, "%s(%d): Bad header in dump file\n", __FILE__, __LINE__));
    
#if 0
    if (strcmp (line, dir) != 0) {
	log_info("%s(%d): ***** WARNING *****\n", __FILE__, __LINE__);
	log_info("    HMM DIRECTORY NAME IN DUMPFILE HEADER: %s\n", line);
	log_info("    INCONSISTENT WITH -hmmdir ARGUMENT:    %s\n\n", dir);
    }
#endif

    /* Skip any other header strings until string length = 0 */
    for (;;) {
	n = fread_int32 (fp, 0, 999, "string length");
	if (n == 0)
	    break;
	if (fread (line, sizeof(char), n, fp) != n)
	    QUIT((stdout, "%s(%d): Cannot read header\n", __FILE__, __LINE__));
    }
    
    /* Read #codewords, #pdfs */
    fread_int32 (fp, r, r, "#codewords");
    fread_int32 (fp, c, c, "#pdfs");
    
    /* Allocate memory for pdfs */
    for (i = 0; i < 4; i++) {
	p[i].prob = (int32 **) CM_2dcalloc (r, 256, sizeof(int32));
	p[i].id = (unsigned char **) CM_2dcalloc (r, c, sizeof(unsigned char));
    }
    
    /* Read pdf values and ids */
    for (n = 0; n < 4; n++) {
	for (i = 0; i < r; i++) {
	    if (fread (p[n].prob[i], sizeof(int32), 256, fp) != 256)
		QUIT((stdout, "%s(%d): fread failed\n", __FILE__, __LINE__));
#if (__BIG_ENDIAN__)
	    {
		int j;
		for (j = 0; j < 256; j++) {
		    SWAP_L(p[n].prob[i][j]);
		}
	    }
#endif
	    if (fread (p[n].id[i], sizeof (unsigned char), c, fp) != (size_t) c)
		QUIT((stdout, "%s(%d): fread failed\n", __FILE__, __LINE__));
	}
    }
    
    fclose (fp);
}

static void
dump_probs(int32 *p0, int32 *p1,
	   int32 *p2, int32 *p3,	/* pdfs, may be transposed */
	   int32 r, int32 c,		/* rows, cols */
	   char const *file,		/* output file */
	   char const *dir)		/* **ORIGINAL** HMM directory */
{
    FILE *fp;
    int32 i, k;
    static char const *title = "V6 Senone Probs, Smoothed, Normalized";
    
    log_info("%s(%d): Dumping HMMs to dump file %s\n",
	     __FILE__, __LINE__, file);
    if ((fp = fopen (file, "wb")) == NULL) {
	log_error("%s(%d): fopen(%s,wb) failed\n", __FILE__, __LINE__, file);
	return;
    }
    
    /* Write title size and title (directory name) */
    k = strlen (title)+1;	/* including trailing null-char */
    fwrite_int32 (fp, k);
    fwrite (title, sizeof(char), k, fp);
    
    /* Write header size and header (directory name) */
    k = strlen (dir)+1;		/* including trailing null-char */
    fwrite_int32 (fp, k);
    fwrite (dir, sizeof(char), k, fp);
    
    /* Write 0, terminating header strings */
    fwrite_int32 (fp, 0);
    
    /* Write #rows, #cols; this also indicates whether pdfs already transposed */
    fwrite_int32 (fp, r);
    fwrite_int32 (fp, c);

    /* Write pdfs */
    for (i = 0; i < r*c; i++)
	fwrite_int32 (fp, p0[i]);
    for (i = 0; i < r*c; i++)
	fwrite_int32 (fp, p1[i]);
    for (i = 0; i < r*c; i++)
	fwrite_int32 (fp, p2[i]);
    for (i = 0; i < r*c; i++)
	fwrite_int32 (fp, p3[i]);
    
    fclose (fp);
}

void read_dists (
	char const *distDir,
	char const *Code_Ext0, char const *Code_Ext1,
	char const *Code_Ext2, char const *Code_Ext3,
	int32 numAlphabet,
	double SmoothMin,
	int32 useCiDistsOnly)
/*------------------------------------------------------------*
 * DESCRIPTION
 *	Out_ProbN is organized as follows.
 *		numAlphabet log_probabilities
 *		    foreach of the numDists[i]
 *			for i=0 to numCiWdPhones
 */
{
    int32               i, osofar;
    int32		numExpected;
    char                file[256];
    int32               numCiWdPhones = phoneCiCount () + phoneWdCount();
    char		*dumpfile;
    
    if (useCiDistsOnly)
	log_info ("ONLY using CI Senones\n");

    totalDists = 0;
    for (i = 0; i < numCiWdPhones; i++)
	totalDists += numDists[i];

    if (kb_get_senprob_size() == 8) {
	if ((dumpfile = kb_get_senprob_dump_file()) == NULL)
	    QUIT((stdout, "%s(%d): Precompiled 8-bit senone probs dump file not specified\n    Compression to be done offline\n", __FILE__, __LINE__));
	load_senone_dists_8bits (out_prob_8b, numAlphabet, totalDists, dumpfile, distDir);
	return;
    }

    Out_Prob1 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob2 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob3 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob4 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob0 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));

    /*
     * Read in the Senones here.
     */
    for (osofar = i = 0; i < numCiWdPhones; i++) {
	/*
	 * The numDPDists are not in the files, we generate them below.
	 */
	numExpected = (numDists[i] - numDPDists[i]) * numAlphabet;
	sprintf (file, "%s/%s", distDir, phone_from_id(i));
	dist_read (file, numExpected, useCiDistsOnly,
		&Out_Prob0[osofar], Code_Ext0,
		&Out_Prob1[osofar], Code_Ext1,
		&Out_Prob2[osofar], Code_Ext2,
		&Out_Prob3[osofar], Code_Ext3);
	osofar += numDists[i] * numAlphabet;
    }
  
    /*
     * Compute the senones cooresponding to the diphones if there are any.
     */
    compute_diphone_senones();

    /*
     * Normalize and insert the floor.
     */
    normalize_dists (numAlphabet, SmoothMin);

    /*
     * Transpose the matricies so that all the entries cooresponding
     * to a particular codeword are in the same row.
     */
    transpose (Out_Prob4, Out_Prob3, totalDists, numAlphabet);
    transpose (Out_Prob3, Out_Prob2, totalDists, numAlphabet);
    transpose (Out_Prob2, Out_Prob1, totalDists, numAlphabet);
    transpose (Out_Prob1, Out_Prob0, totalDists, numAlphabet);
    free (Out_Prob0);

    if ((dumpfile = kb_get_senprob_dump_file()) != NULL)
	dump_probs(Out_Prob1, Out_Prob2, Out_Prob3, Out_Prob4,
		   numAlphabet, totalDists, dumpfile, distDir);
}

#if 0
/*
 * dist_min_max: find min and max prob values in the given 32-bit prob
 * distr table.
 */
static void
dist_min_max(int32 *dist, /* prob distr table */
	     int32 sz,	  /* size of distr table */
	     int32 *min, int32 *max)	/* ptrs to return values */
{
    int32 i, maxi = 0;

    /* FIXME: signedness?! */
    *min = (int32) 0x7fffffff;
    *max = (int32) 0x80000000;
    for (i = 0; i < sz; i++) {
	if (*min > *dist)
	    *min = *dist;
	if (*max < *dist) {
	    *max = *dist;
	    maxi = i;
	}
	dist++;
    }
    log_info("  min = %d, max[%d] = %d\n", *min, maxi, *max);
}

/*
 * COMPRESS_SEN_DISTS_16BITS: compress 32-bit senone prob values to 16-bit values.
 * Compress in-place.  Free the remaining space.
 * Compression scheme:
 *     if (int32-value fits in int16) int16-value = int32-value;
 *     else int16-value = (-(int32-value) >> shift),
 * where shift = #bit-shifts is determined from max(abs(min(int32-values))).
 * Return shift.
 */
static int32
compress_sen_dists_16bits (int32 num_alphabet)
{
    int32 i, sz, min, max, minmin, absmin, shift;

    log_info ("%s(%d): Compressing senone probs to 16 bits\n", __FILE__, __LINE__);
    
    sz = totalDists * num_alphabet;
    
    out_prob_16b[0] = (int16 *) Out_Prob1;
    out_prob_16b[1] = out_prob_16b[0] + sz;
    out_prob_16b[2] = (int16 *) Out_Prob2;
    out_prob_16b[3] = out_prob_16b[2] + sz;

    dist_min_max (Out_Prob1, sz, &min, &max);
    if (max > 0)
	QUIT((stderr, "%s(%d): Cannot compress; dist1 range = %d..%d\n", __FILE__, __LINE__, min, max));
    minmin = min;
    
    dist_min_max (Out_Prob2, sz, &min, &max);
    if (max > 0)
	QUIT((stderr, "%s(%d): Cannot compress; dist2 range = %d..%d\n", __FILE__, __LINE__, min, max));
    if (minmin > min)
	minmin = min;
    
    dist_min_max (Out_Prob3, sz, &min, &max);
    if (max > 0)
	QUIT((stderr, "%s(%d): Cannot compress; dist3 range = %d..%d\n", __FILE__, __LINE__, min, max));
    if (minmin > min)
	minmin = min;
    
    dist_min_max (Out_Prob4, sz, &min, &max);
    if (max > 0)
	QUIT((stderr, "%s(%d): Cannot compress; dist4 range = %d..%d\n", __FILE__, __LINE__, min, max));
    if (minmin > min)
	minmin = min;

    /* find compression factor (#bits to shift-right |minmin| to fit in 16 bits) */
    absmin = -minmin;
    for (shift = 0; absmin > 32768; shift++)
	absmin >>= 1;
    
    /* compress in[] */
    for (i = 0; i < sz; i++)
	out_prob_16b[0][i] = (Out_Prob1[i] < -32768) ?
	    (-Out_Prob1[i]) >> shift : Out_Prob1[i];
    for (i = 0; i < sz; i++)
	out_prob_16b[1][i] = (Out_Prob2[i] < -32768) ?
	    (-Out_Prob2[i]) >> shift : Out_Prob2[i];
    for (i = 0; i < sz; i++)
	out_prob_16b[2][i] = (Out_Prob3[i] < -32768) ?
	    (-Out_Prob3[i]) >> shift : Out_Prob3[i];
    for (i = 0; i < sz; i++)
	out_prob_16b[3][i] = (Out_Prob4[i] < -32768) ?
	    (-Out_Prob4[i]) >> shift : Out_Prob4[i];
    
    free (Out_Prob3);
    free (Out_Prob4);

    log_info ("%s(%d): %d LSBs lost in compression\n", __FILE__, __LINE__, shift);

    return (shift);
}
#endif /* 0 */

void readDistsOnly (
	char const *distDir,
	char const *Code_Ext0, char const *Code_Ext1,
	char const *Code_Ext2, char const *Code_Ext3,
	int32 numAlphabet,
	int32 useCiDistsOnly)
/*------------------------------------------------------------*
 * DESCRIPTION
 *	Only read the dists, don't normalize or transpose
 */
{
    int32                 i, osofar;
    int32		numExpected;
    char                file[256];
    int32                numCiWdPhones = phoneCiCount () + phoneWdCount();

    totalDists = 0;
    for (i = 0; i < numCiWdPhones; i++)
	totalDists += numDists[i];

    Out_Prob0 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob1 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob2 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));
    Out_Prob3 = (int32 *) CM_calloc (totalDists * numAlphabet, sizeof(int32));

    osofar = 0;

    for (i = 0; i < numCiWdPhones; i++) {
	numExpected = numDists[i] * numAlphabet;
	sprintf (file, "%s/%s", distDir, phone_from_id(i));
	dist_read (file, numExpected, useCiDistsOnly,
		&Out_Prob0[osofar], Code_Ext0,
		&Out_Prob1[osofar], Code_Ext1,
		&Out_Prob2[osofar], Code_Ext2,
		&Out_Prob3[osofar], Code_Ext3);

	osofar += numExpected;
    }
}

static void
normalize_dists (int numAlphabet, double SmoothMin)
/*------------------------------------------------------------*
 * DESCRIPTION
 */
{
    int32                 i, k, smooth_min, osofar;
    int32                numCiWdPhones = phoneCiCount () + phoneWdCount ();

    totalDists = 0;
    for (i = 0; i < numCiWdPhones; i++)
	totalDists += numDists[i];

    smooth_min = LOG (SmoothMin);
    osofar = 0;

    for (i = 0; i < numCiWdPhones; i++) {
	for (k = 0; k < numDists[i]; k++) {
	    normalize_out (&Out_Prob0[osofar], 1.0, numAlphabet);
	    insert_floor (&Out_Prob0[osofar], smooth_min, numAlphabet);
	    normalize_out (&Out_Prob0[osofar], 1.0, numAlphabet);

	    normalize_out (&Out_Prob1[osofar], 1.0, numAlphabet);
	    insert_floor (&Out_Prob1[osofar], smooth_min, numAlphabet);
	    normalize_out (&Out_Prob1[osofar], 1.0, numAlphabet);

	    normalize_out (&Out_Prob2[osofar], 1.0, numAlphabet);
	    insert_floor (&Out_Prob2[osofar], smooth_min, numAlphabet);
	    normalize_out (&Out_Prob2[osofar], 1.0, numAlphabet);

	    normalize_out (&Out_Prob3[osofar], 1.0, numAlphabet);
	    insert_floor (&Out_Prob3[osofar], smooth_min, numAlphabet);
	    normalize_out (&Out_Prob3[osofar], 1.0, numAlphabet);

	    osofar += numAlphabet;
	}
    }
}

static void
transpose (int32 *pa, int32 const *a, int32 rows, int32 cols)
{
    int32	i, j, idx, pidx;
    /*
     * Permute the codes
     */
    for (idx = i = 0; i < cols; i++) {
	pidx = i;
	for (j = 0; j < rows; j++, pidx += cols, idx++) {
	    pa[idx] = a[pidx];
	}
    }
}

void
remap (SMD *smdV)	/* smd pointer vector */
/*------------------------------------------------------------*
 * DESCRIPTION
 *	Remap the distributions in the smd's.
 */
{
    int32 		i, j;

    for (i = 0; i < numSSeq; i++) {
	for (j = 0; j < TRANS_CNT; j++) {
	    smdV[i].dist[j] = distMap[i][smdV[i].dist[j]];
  	}
    }
    free (distMap);
}

typedef struct {
    int32 name;
    int32 idx;
} ARC;

static int
cmp_arc (void const *a, void const *b)
{
    return (((ARC *)a)->name - ((ARC *)b)->name);
}

void
hmm_tied_read_bin (char const *dir_list,   /* directory search list */
		   char const *file,	   /* tied dist hmm file name */
		   SMD *smd,		   /* smd struct to fill */
		   double transSmooth, 	   /* Trans smoothing floor */
		   int32 numAlphaExpected, /* Expected size of alphabet */
		   int norm,		   /* normalize the arcs ? */
		   double arcWeight)	   /* transition weight */
/*------------------------------------------------------------*
 * Read a single hmm file.
 */
{
    FILE               *fp;
    int32              magic, tmp;
    int                doByteSwap = FALSE;

    fp = CM_fopenp (dir_list, file, "rb");

    CM_fread (&magic, sizeof (int32), 1, fp);

    if (magic != TIED_DIST) {
	swapLong (&magic);
	if (magic != TIED_DIST) {
	    log_error ("%s(%d): in %s, magic = %d expected %d\n", __FILE__, __LINE__, 
		  file, magic, TIED_DIST);
	    exit (-1);
	}
	else
	    doByteSwap = TRUE;
    }

    hmm_tied_bin_parse (fp, smd, transSmooth, numAlphaExpected, norm, arcWeight,
			doByteSwap, file);

    fread (&tmp, sizeof (int32), 1, fp);
    if (fread (&tmp, sizeof (int32), 1, fp) != 0) {
	log_error ("%s(%d): EOF not encountered in %s\n", __FILE__, __LINE__, file);
	exit (-1);
    }
    fclose (fp);
}

void
hmm_tied_read_big_bin (char const *dir_list,/* directory search list */
		       char const *file,   /* tied dist hmm file name */
		       SMD   *smds,	   /* smd structs to fill */
		       double transSmooth, /* Trans smoothing floor */
		       int32  numAlphaExpected,	/* Expected size of alphabet */
		       int    norm,	   /* normalize the arcs ? */
		       double arcWeight)   /* transition weight */
/*------------------------------------------------------------*
 * Read a big hmm file.
 */
{
    FILE               *fp;
    int32                magic, pid;
    int32                doByteSwap = FALSE;
    char		hmmName[256];
    SMD			dummySmd;
    SMD			*smd;
    int32		parsed = 0;

    fp = CM_fopenp (dir_list, file, "rb");

    while (TRUE) {
	int32 i;

	if (0 == fread (&magic, sizeof (int32), 1, fp)) {
	    if (parsed == 0)
		log_info("%s(%d): file [%s] is empty\n", 
			 __FILE__, __LINE__, file);
	    break;
  	}
  	parsed++;

	if (magic != BIG_HMM) {
	    swapLong (&magic);
	    if (magic != BIG_HMM) {
	        char str[256];
	        /*
	         * Maybey this a little hmm file ?
	         */
	        fclose (fp);
	        strcpy (str, file);
	        *strrchr (str, '.') = '\0';
	        pid = phone_to_id (str, TRUE);
	        hmm_tied_read_bin (dir_list, file, &smds[hmm_pid2sid(pid)], transSmooth,
			       numAlphaExpected, norm, arcWeight);
	        return;
	    }
	    else {
	        doByteSwap = TRUE;
	    }
	}

        /*
         * Parse the name of the next hmm
         */
 	memset (hmmName, 0, 256);
	for (i = 0; i < 256; i++) {
	    hmmName[i] = fgetc(fp);
	    if ((hmmName[i] == EOF) || (hmmName[i] == '\0'))
		break;
	}

	/*
	 * Reached EOF cleanly
	 */
  	if ((i == 0) && feof(fp))
	    break;

	/* make sure we parsed an hmmName
	 */	    
	if ((i == sizeof(hmmName)) || (hmmName[i] != '\0')) {
	    log_error ("%s(%d): failed to parse hmmName [%s] from [%s]\n",
		     __FILE__, __LINE__, hmmName, file);
	    exit (-1);
	}

        pid = phone_to_id (hmmName, TRUE);
	if (pid == NO_PHONE) {
	    log_warn ("Ignoring this phone\n");
	    smd = &dummySmd;
	}
        else {
	    smd = &smds[hmm_pid2sid(pid)];
	}
	
	hmm_tied_bin_parse (fp, smd, transSmooth,
			    numAlphaExpected, norm, arcWeight,
			    doByteSwap, file);
    }

    fclose (fp);
}

static void
hmm_tied_bin_parse (FILE     *fp,		   /* file pointer, to next hmm */
		    SMD      *smd,		   /* smd struct to fill */
		    double    transSmooth,	   /* Trans smoothing floor */
		    int32     numAlphaExpected,	   /* Expected size of alphabet */
		    int	      norm,		   /* normalize the arcs ? */
		    double    arcWeight,	   /* transition weight */
		    int	      doByteSwap,	   /* Byte swap the data? */
		    char const *hmmName)	   /* the name of this hmm */
/*------------------------------------------------------------*
 * FORMAT
 *	NAME		value	size	meaning
 *	magic		-10	4	Indicates tied distributions
 *	numAlphabet	256	4	Size of code book
 *	num_omatrix	  5	4	Number output distrubutions
 *	num_states	  6	4	Number of states in the model
 *	num_initial	  1	4	Number start states
 *	initail[]	  ?	4*num_initial 	The state numbers
 *	num_final	  1	4	Number of final states
 *	final[]		  ?     4*num_final	The state numbers
 *	num_arcs	 14     4	Number of directed arcs
 *      struct arc	  ?     16*Num Arcs	The arcs
 *	    from
 *	    to
 *	    prob
 *	    dist_num	
 */
{
    int32                i;
    int32                numOMatrix, numInitial, numFinal, numArcs;
    int32                numAlphabet;
    ARC                 arcs[MAX_ARCS];
    int32                distBuf[MAX_ARCS];
    int32                tpBuf[MAX_ARCS];
    SMD_R		smd_tmp;
    SMD_R		*smd_r = &smd_tmp;

    memset (smd_r, 0, sizeof(SMD_R));

    CM_fread ((char *) &numAlphabet, sizeof (int32), 1, fp);
    if (doByteSwap)
	swapLong (&numAlphabet);

    if (numAlphabet != numAlphaExpected) {
	log_error ("%s(%d): in %s, VQ size != %d\n", hmmName, numAlphaExpected,
		__FILE__, __LINE__);
	exit (-1);
    }

    CM_fread (&numOMatrix, sizeof (int32), 1, fp);
    if (doByteSwap)
	swapLong (&numOMatrix);

    CM_fread (&smd_r->stateCnt, sizeof (int32), 1, fp);
    if (doByteSwap)
	swapLong (&smd_r->stateCnt);

    if (smd_r->stateCnt != (HMM_LAST_STATE+1)) {
	log_error ("%s(%d): Unexpected state count = %d, in %s\n",
	         __FILE__, __LINE__, smd_r->stateCnt, hmmName);
	exit (-1);
    }

    CM_fread (&numInitial, sizeof (int32), 1, fp);
    if (doByteSwap)
	swapLong (&numInitial);

    if (numInitial != 1) {
	log_error ("%s(%d): Unexpected num. initial states = %d, in %s\n",
	        __FILE__, __LINE__, numInitial, hmmName);
	exit (-1);
    }

    for (i = 0; i < numInitial; i++) {
	int32                state;

	CM_fread (&state, sizeof (int32), 1, fp);
	if (doByteSwap)
	    swapLong (&state);

	if (state != 0) {
	    log_error ("%s(%d): Unexpected initial state = %d, in %s\n",
		    __FILE__, __LINE__, numInitial, hmmName);
	    exit (-1);
	}
    }

    CM_fread (&numFinal, sizeof (int32), 1, fp);
    if (doByteSwap)
	swapLong (&numFinal);

    if (numFinal != 1) {
	log_error ("%s(%d): Unexpected num. final states = %d, in %s\n",
	        __FILE__, __LINE__, numFinal, hmmName);
	exit (-1);
    }

    for (i = 0; i < numFinal; i++) {
	int32                state;

	CM_fread (&state, sizeof (int32), 1, fp);
	if (doByteSwap)
	    swapLong (&state);

	if (state != HMM_LAST_STATE) {
	    log_error ("%s(%d): Unexpected final state = %d, in %s\n",
		    __FILE__, __LINE__, numArcs, hmmName);
	    exit (-1);
	}
    }

    CM_fread (&numArcs, sizeof (int32), 1, fp);
    if (doByteSwap)
	swapLong (&numArcs);

    if (numArcs != TRANS_CNT) {
	log_error ("%s(%d): Unexpected number of arcs = %d, in %s\n",
	        __FILE__, __LINE__, numArcs, hmmName);
	exit (-1);
    }

    for (i = 0; i < numArcs; i++) {
	int32                from, to, prob, dist;

	CM_fread (&from, sizeof (int32), 1, fp);
	CM_fread (&to, sizeof (int32), 1, fp);
	CM_fread (&prob, sizeof (int32), 1, fp);
	CM_fread (&dist, sizeof (int32), 1, fp);
	if (doByteSwap) {
	    swapLong (&from);
	    swapLong (&to);
	    swapLong (&prob);
	    swapLong (&dist);
	}

	/*
  	 * Check for bad distributions
	 */
	if ((dist >= numOMatrix) ||
	    (dist < 0 && dist != NULL_TRANSITION)) {
	    log_error ("%s(%d): Illegal out_prob_index = %d, arc %d, in %s\n",
		    __FILE__, __LINE__, dist, i, hmmName);
	    exit (-1);
	}

	if ((from >= smd_r->stateCnt) || (from < 0) ||
	    (to >= smd_r->stateCnt) || (to < 0)) {
	    log_error ("%s(%d): Illegal arc(%d) from(%d)->to(%d) in %s\n",
		    __FILE__, __LINE__, i, from, to, hmmName);
	    exit (-1);
	}

	/*
	 * Set up the topology 
	 */
	{
	    int32                bit = ((from * smd_r->stateCnt) + to) % 32;
	    int32                word = ((from * smd_r->stateCnt) + to) / 32;

	    smd_r->topo[word] |= (1 << bit);
	}

	arcs[i].name = (from << 16) | to;
	arcs[i].idx = i;

	distBuf[i] = dist;
	tpBuf[i] = prob;
    }

    /*
     * Sort the ARCS in lexical order (from,to) and reorder distBuf and tpBuf
     * into the smd structure.
     */
    qsort (arcs, numArcs, sizeof (ARC), cmp_arc);

    for (i = 0; i < numArcs; i++) {
	smd->dist[i] = distBuf[arcs[i].idx];
	smd->tp[i] = tpBuf[arcs[i].idx];
    }

    if (norm)
	if (hmmArcNormalize (smd, smd_r, transSmooth, arcWeight) < 0) {
	    log_error ("%s(%d): Problem with trans probs in %s\n", __FILE__, __LINE__, hmmName);
	    exit (-1);
	}
}

static int
hmmArcNormalize (SMD *smd, SMD_R *smd_r, double transSmooth, double arcWeight)
{
    int32 logTransSmooth = LOG (transSmooth);
    int32 i;

    normalize_trans (smd, smd_r, arcWeight);

    for (i = 0; i < TRANS_CNT; i++)
	if (smd->tp[i] < logTransSmooth)
	    smd->tp[i] = logTransSmooth;

    normalize_trans (smd, smd_r, arcWeight);

    for (i = 0; i < TRANS_CNT; i++)
	if ((smd->tp[i] > 0) || (smd->tp[i] < MIN_LOG))
	    return (-1);

    return (0);
}

static void
normalize_out (register int32 *out, double weight, int32 numAlphabet)
{
    int32 sum = MIN_LOG;
    register int32 i;

    for (i = 0; i < numAlphabet; i++)
	sum = ADD (sum, out[i]);
    if (weight == 1.0)
	for (i = 0; i < numAlphabet; i++)
	    if (out[i] <= MIN_LOG)
		out[i] = MIN_LOG;
	    else
		out[i] -= sum;
    else
	for (i = 0; i < numAlphabet; i++)
	    if (out[i] <= MIN_LOG)
		out[i] = MIN_LOG;
	    else
		out[i] = (int32) (((double) out[i] - sum) * weight);
}


static void
insert_floor (register int32 *out, int32 min, int32 numAlphabet)
{
    register int32 i;

    for (i = 0; i < numAlphabet; i++)
	if (out[i] < min)
	    out[i] = min;
}

static void
normalize_trans (SMD *smd, SMD_R *smd_r, double weight)
{
    int32 		fs, ts;
    int32                arc = 0;

    for (fs = 0; fs < smd_r->stateCnt; fs++) {
	int32                denom = MIN_LOG;
	int32                save_arc = arc;

	for (ts = 0; ts < smd_r->stateCnt; ts++) {
	    if (HAS_ARC (fs, ts, smd_r)) {
		denom = ADD (denom, smd->tp[arc]);
		arc++;
	    }
	}
	arc = save_arc;
	for (ts = 0; ts < smd_r->stateCnt; ts++) {
	    if (HAS_ARC (fs, ts, smd_r)) {
		if (smd->tp[arc] <= MIN_LOG)
		    smd->tp[arc] = MIN_LOG;
		else
		    smd->tp[arc] = (smd->tp[arc] - denom) * weight;
		arc++;
	    }
	}
    }
}

static int
cmp_sseq (void const *a, void const *b)
/*--------------------*
 * Compare Senone Sequences
 */
{
    int32	i;

    for (i = 0; i < NUMDISTRTYPES; i++) {
        if (distMap[*(int32 *)a][i] != distMap[*(int32 *)b][i]) {
	    return (distMap[*(int32 *)a][i] - distMap[*(int32 *)b][i]);
	}
    }
    return (0);
}

static int
cmp_dmap (void const *a, void const *b)
/*--------------------*
 * Compare Senone Sequences
 */
{
    int32	i;

    for (i = 0; i < NUMDISTRTYPES; i++) {
        if ((*(int const **)a)[i] != (*(int const **)b)[i]) {
	    return ((*(int const **)a)[i] - (*(int const **)b)[i]);
	}
    }
    return (0);
}


/*
 * HACK!!  This routine is full of hacks.
 * Assumes that each line is of the form: triphone<state-name> id, where triphone begins
 * on 1st char of line, and state-name is a 1-digit char.
 * Return EOF if end of file, 1 otherwise.
 */
static int32
read_map_line(FILE *fp, char *line,
	      int32 linesize, int32 *name, int32 *id)
{
    char *lp;

    if (fgets(line, linesize, fp) == NULL)
        return (EOF);

    /* Find triphone */
    for (lp = line; (*lp != '<') && (*lp != '\0'); lp++);
    if (*lp == '\0')
        QUIT((stdout, "%s(%d): **ERROR** Cannot find <state>: %s\n",
	      __FILE__, __LINE__, line));
    *lp = '\0';
    ++lp;
    
    /* Find state name */
    *name = *lp - '0';
    lp += 2;
    
    /* read global id */
    if (sscanf(lp, "%d", id) != 1)
	QUIT((stdout, "%s(%d): **ERROR** Cannot read senone id\n", __FILE__, __LINE__));
    
    return (1);
}

static int
eq_dist (int32 *a, int32 *b)
{
    int32	i;

    for (i = 0; i < NUMDISTRTYPES; i++) {
        if (a[i] != b[i]) {
	    return 0;
	}
    }
    return 1;
}

void read_map (char const *map_file, int32 compress)
/*------------------------------------------------------------*
 * DESCRIPTION
 *
 * The format of the map file looks like the following:
 *		AE(DH,TD)<0>    1
 *		AE(DH,TD)<1>    2
 *		AE(Z,V)b<1>     2
 * Where
 *	AE 		is the phone
 *	(DH,TD)b 	is the context
 *	<0>		is the local distribution name
 *	1		is the distribution id, numbering starts at 1.
 *
 * Parameters
 *	compress	- if compress is true then sort the distMap
 *			  and delete duplicates.
 */
{
    FILE               *fp;
    int32                i, j;
    char		triphone[256];
    int32	        distName, distId, triphoneId, ciPhoneId;
    int32		numCiPhones = phoneCiCount ();
    int32		numWdPhones = phoneWdCount ();
    int32                numCiWdPhones = phoneCiCount () + phoneWdCount();
    int32		numPhones = phone_count ();
    
    fp = CM_fopen (map_file, "r");
    
    numDists = (int32 *) CM_calloc (numCiPhones+numWdPhones, sizeof(int32));
    numDPDists = (int32 *) CM_calloc (numCiPhones+numWdPhones, sizeof(int32));
    distMap = (int32 **) CM_2dcalloc (numPhones, NUMDISTRTYPES, sizeof(int32));

    while (read_map_line (fp, triphone, sizeof(triphone), &distName, &distId) != EOF)
    {
 	triphoneId = phone_to_id (triphone, TRUE);
	if (triphoneId < 0)
	    QUIT((stderr, "%s(%d): cannot find triphone %s\n", __FILE__, __LINE__, triphone));

 	ciPhoneId = phone_id_to_base_id (triphoneId);

	distMap[triphoneId][distName] = distId - 1;

	/*
	 * Keep track of the number of distributions for each class of
	 * phonemes
	 */
	if (numDists[ciPhoneId] < distId)
	    numDists[ciPhoneId] = distId;
    }

    /*
     * Add in the context independent distributions and the
     * the with in word dists
     */
    for (i = 0; i < numPhones; i++) {
	int32 phoneType = phone_type(i);
	int32 offset = -1;

	if (phoneType == PT_CDPHONE)	/* these phones are mapped */
	    continue;
	if (phoneType == PT_CDDPHONE)	/* these phones are mapped */
	    continue;
	if (phoneType == PT_DIPHONE)	/* these are handle below */
	    continue;
	if (phoneType == PT_DIPHONE_S) 	/* these are handle below */
	    continue;
	
	if (phoneType == PT_CIPHONE) {
	    offset = numDists[i];
	    numDists[i] += NUMDISTRTYPES;
	}
	
	if (phoneType == PT_WWPHONE)
	    offset = 0;
	if (phoneType >= PT_WWCPHONE)
	    offset = (phoneType - PT_WWCPHONE) * NUMDISTRTYPES;

	if (offset == -1) {
	    log_warn ("%s(%d): Ignoring unknown phone type %d\n",
		     __FILE__, __LINE__, phoneType);
	    continue;
	}

	/*
	 * They are the last NUMDISTRTYPES in the file
	 */
	for (j = 0; j < NUMDISTRTYPES; j++) {
	    distMap[i][j] = j + offset;
	}
    }

    /*
     * Add in the distributions for the diphones if there are any
     */
    for (i = 0; i < numPhones; i++) {
	int32 phoneType = phone_type(i);
	int32 offset = -1;
	int32 baseid = phone_id_to_base_id (i);

	/* we only care about diphones */
	if ((phoneType != PT_DIPHONE) && (phoneType != PT_DIPHONE_S))
	    continue;
	
	offset = numDists[baseid];
	
	/*
	 * There are NUMDISTRTYPES dists per diphone
	 */
	for (j = 0; j < NUMDISTRTYPES; j++) {
	    distMap[i][j] = j + offset;
	}
	numDPDists[baseid] += NUMDISTRTYPES;
	numDists[baseid] += NUMDISTRTYPES;
    }

    /*
     * Fix up the counts for WWPHONE's
     */
    for (i = 0; i < numPhones; i++) {
	int32 phoneType = phone_type(i);

        if (phoneType == PT_WWPHONE)
	    numDists[i] = NUMDISTRTYPES * phone_len (i);
    }

    fclose (fp);

    totalDists = 0;
    for (i = 0; i < numCiWdPhones; i++)
	totalDists += numDists[i];

    /*
     * Convert the distMap entries to be indexed from 0 rather than from
     * 0 for each basephone.
     */
    {
        int32 		i, j;
        int32           numCiWdPhones = phoneCiCount () + phoneWdCount();
        int32		numPhones = phone_count();
	int32		*distIndexBase;


        distIndexBase = (int32 *) CM_calloc (numCiWdPhones, sizeof(int32));
        distIndexBase[0] = 0;
        for (i = 1; i < numCiWdPhones; i++)
	    distIndexBase[i] = distIndexBase[i-1] + numDists[i-1];

        for (i = 0; i < numPhones; i++) {
	    for (j = 0; j < NUMDISTRTYPES; j++) {
	        distMap[i][j] += distIndexBase[phone_id_to_base_id(i)];
	        if ((distMap[i][j] > totalDists) || (distMap[i][j] < 0)) {
		    log_error ("%s(%d): distMap[%d][%d] == %d\n",
			    __FILE__, __LINE__, i, j, distMap[i][j]);
		    exit (-1);
		}
	    }
  	}
	free (distIndexBase);
    }

    /*
     * Allocate Senone Sequence Id table
     */
    ssIdMap = (int32 *) CM_calloc (numPhones, sizeof(int32));

    /*
     * Sort distPermTab[] (distribution permutation table)
     * according to senone sequences in distMap
     */
    if (compress)  {
	int32	i, j, id;
        int32    numPhones = phone_count();
	/*
	 * Temporary permutation table
	 */
	int32 	*pTab = (int32 *) CM_calloc (numPhones, sizeof(int32));
	

	for (i = 0; i < numPhones; i++) {
	    pTab[i] = i;
	}

	/*
	 * Sort pTab according to distMap then sort distMap.
	 */
	qsort (pTab, numPhones, sizeof(int32), cmp_sseq);
	qsort (distMap, numPhones, sizeof(int32 *), cmp_dmap);

	/*
	 * No compress distMap (removing duplicates) and write
	 * the ssIdTab.
	 */
	for (id = 0, i = 0, j = 0; j < numPhones; j++) {
	   if (eq_dist (distMap[i], distMap[j])) {
		ssIdMap[pTab[j]] = id;
	   }
	   else {
		id++;
		ssIdMap[pTab[j]] = id;
		distMap[id] = distMap[j];
		i = j;
	   }
	}
	free (pTab);
	numSSeq = id+1;

	log_info("Read Map: %d phones map to %d unique senone sequences\n",
		 numPhones, numSSeq);
    }
    else {
	int32 i;

	/*
	 * Create the default map when no compression is done
	 */
	numSSeq = phone_count();
	for (i = 0; i < numPhones; i++) {
	    ssIdMap[i] = i;
	}
    }
}

static void dist_read (
	char const *file,
	int32 expected,
	int32 useCiDistsOnly,
	int32 *o1, char const *Code_Ext1,
	int32 *o2, char const *Code_Ext2,
	int32 *o3, char const *Code_Ext3,
	int32 *o4, char const *Code_Ext4)
{
    int32                *iptr, numints;
    char		filename[128];

    sprintf (filename, "%s.%s", file, Code_Ext1);
    /* FIXME: areadint() and friends need to be prototyped somewhere */
    areadint (filename, &iptr, &numints);
    if (((numints != expected) && (! useCiDistsOnly))    ||
	((numints < (NUMDISTRTYPES * MAX_ALPHABET)) && useCiDistsOnly))
    {
	log_error ("%s(%d): %s length trouble (%d expected, read %d)\n",
	        __FILE__, __LINE__, filename, expected, numints);
	exit(-1);
    }
    /*
     * If useCiDistsOnly then copy only the context independent senones.
     */
    if (useCiDistsOnly)
	memcpy (o1, &iptr[numints - (NUMDISTRTYPES * MAX_ALPHABET)],
	       expected * sizeof (int32));
    else
        memcpy (o1, iptr, numints * sizeof (int32));
    free (iptr);

    sprintf (filename, "%s.%s", file, Code_Ext2);
    areadint (filename, &iptr, &numints);
    if (((numints != expected) && (! useCiDistsOnly))    ||
	((numints < (NUMDISTRTYPES * MAX_ALPHABET)) && useCiDistsOnly))
    {
	log_error ("%s(%d): %s length trouble (%d expected, read %d)\n",
	        __FILE__, __LINE__, filename, expected, numints);
	exit(-1);
    }
    /*
     * If useCiDistsOnly then copy only the context independent senones.
     */
    if (useCiDistsOnly)
	memcpy (o2, &iptr[numints - (NUMDISTRTYPES * MAX_ALPHABET)],
	       expected * sizeof (int32));
    else
        memcpy (o2, iptr, numints * sizeof (int32));
    free (iptr);

    sprintf (filename, "%s.%s", file, Code_Ext3);
    areadint (filename, &iptr, &numints);
    if (((numints != expected) && (! useCiDistsOnly))    ||
	((numints < (NUMDISTRTYPES * MAX_ALPHABET)) && useCiDistsOnly))
    {
	log_error ("%s(%d): %s length trouble (%d expected, read %d)\n",
	        __FILE__, __LINE__, filename, expected, numints);
	exit(-1);
    }
    /*
     * If useCiDistsOnly then copy only the context independent senones.
     */
    if (useCiDistsOnly)
	memcpy (o3, &iptr[numints - (NUMDISTRTYPES * MAX_ALPHABET)],
	       expected * sizeof (int32));
    else
        memcpy (o3, iptr, numints * sizeof (int32));
    free (iptr);

    sprintf (filename, "%s.%s", file, Code_Ext4);
    areadint (filename, &iptr, &numints);
    if (((numints != expected) && (! useCiDistsOnly))    ||
	((numints < (NUMDISTRTYPES * MAX_ALPHABET)) && useCiDistsOnly))
    {
	log_error ("%s(%d): %s length trouble (%d expected, read %d)\n",
	        __FILE__, __LINE__, filename, expected, numints);
	exit(-1);
    }
    /*
     * If useCiDistsOnly then copy only the context independent senones.
     */
    if (useCiDistsOnly)
	memcpy (o4, &iptr[numints - (NUMDISTRTYPES * MAX_ALPHABET)],
	       expected * sizeof (int32));
    else
        memcpy (o4, iptr, numints * sizeof (int32));
    free (iptr);
}

#define MAX_MEMBERS 256

int32 sets[NUMDISTRTYPES][MAX_MEMBERS];
int32 set_size[NUMDISTRTYPES];

static void
add_member (int32 m, int32 s)
{
    sets[s][set_size[s]] = m;
    set_size[s]++;
}

static int32 isa_member (int32 m, int32 s)
{
    int32 i;
    for (i = 0; i < set_size[s]; i++) {
	if (sets[s][i] == m)
	    return TRUE;
    }
    return FALSE;
}

static void remove_all_members (int32 s)
{
    set_size[s] = 0;
}

int32
hmm_num_sseq (void)
/*------------------------------------------------------------*
 * Return number of unique senone sequences.
 * If the number is 0 we call this a fatal error.
 */
{
    if (numSSeq == 0) {
	log_error ("%s(%d): numSSeq (number of senone sequences is 0\n", __FILE__, __LINE__);
	exit (-1);
    }
    return numSSeq;
}

int32
hmm_pid2sid (int32 pid)
/*------------------------------------------------------------*
 * Convert a phone id to a senone sequence id\
 */
{
    return ssIdMap[pid];
}

static void compute_diphone_senones(void)
{
    int32 pid, cpid;
    int32 j, k, s;
    char pstr[64];
    int32 phone_cnt = phone_count();
    int32 ci_phone_cnt = phoneCiCount();

    for (pid = 0; pid < phone_cnt; pid++) {
	int32 p_type = phone_type(pid);
	char const *di_pstr = phone_from_id(pid);

	if (p_type == PT_DIPHONE) {
	   /*  printf ("DiPhone %s\n", di_pstr); */
	    for (s = 0; s < NUMDISTRTYPES; s++) {
		remove_all_members(s);
		zero_senone (distMap[ssIdMap[pid]][s]);
	    }
 	    for (j = 0; j < ci_phone_cnt; j++) {
	        sprintf (pstr, di_pstr, phone_from_id(j));
		cpid = phone_to_id (pstr, FALSE);
	  	if (cpid != NO_PHONE) {
		    for (s = 0; s < NUMDISTRTYPES; s++) {
			if (!isa_member(distMap[ssIdMap[cpid]][s], s)) {
/*
   printf ("\tAdd senone %d, state %d, phone %s\n",
	   distMap[ssIdMap[cpid]][s], s, pstr);
*/
		            add_senone (distMap[ssIdMap[pid]][s],
				        distMap[ssIdMap[cpid]][s]);
			    add_member (distMap[ssIdMap[cpid]][s], s);
			}
		    }
		}
	    }
	}
	else
	if (p_type == PT_DIPHONE_S) {
/*
  printf ("DiPhone %s\n", di_pstr);
*/
	    for (s = 0; s < NUMDISTRTYPES; s++)  {
		remove_all_members(s);
		zero_senone (distMap[ssIdMap[pid]][s]);
	    }
 	    for (j = 0; j < ci_phone_cnt; j++) {
 		for (k = 0; k < ci_phone_cnt; k++) {
	            sprintf (pstr, di_pstr, phone_from_id(j), phone_from_id(k));
		    cpid = phone_to_id (pstr, FALSE);
	  	    if (cpid != NO_PHONE) {
		        for (s = 0; s < NUMDISTRTYPES; s++) {
			    if (!isa_member(distMap[ssIdMap[cpid]][s], s)) {
/*
   printf ("\tAdd senone %d, state %d, phone %s\n",
	   distMap[ssIdMap[cpid]][s], s, pstr);
*/
		                add_senone (distMap[ssIdMap[pid]][s],
					    distMap[ssIdMap[cpid]][s]);
			        add_member (distMap[ssIdMap[cpid]][s], s);
			    }
			}
		    }
		}
	    }
	}
    } 
}

static void add_senone (int32 s1, int32 s2)
{
    int32 i, j, e1;
    int16 *at = Addition_Table;
    int32  ts = Table_Size;

    e1 = (MAX_ALPHABET * s1) + MAX_ALPHABET;
    for (j = MAX_ALPHABET * s2, i = MAX_ALPHABET * s1; i < e1; i++, j++) {
	FAST_ADD (Out_Prob0[i], Out_Prob0[i], Out_Prob0[j], at, ts);
	FAST_ADD (Out_Prob1[i], Out_Prob1[i], Out_Prob1[j], at, ts);
	FAST_ADD (Out_Prob2[i], Out_Prob2[i], Out_Prob2[j], at, ts);
	FAST_ADD (Out_Prob3[i], Out_Prob3[i], Out_Prob3[j], at, ts);
    }
}

static void zero_senone (int32 s)
{
    int32 i, end;

    end = (MAX_ALPHABET * s) + MAX_ALPHABET;
    for (i = MAX_ALPHABET * s; i < end; i++) {
	Out_Prob0[i] = MIN_LOG;
	Out_Prob1[i] = MIN_LOG;
	Out_Prob2[i] = MIN_LOG;
	Out_Prob3[i] = MIN_LOG;
    }
}


int32
senid2pid (int32 senid)
{
    int32 p, k, nph;
    
    k = 0;
    nph = phoneCiCount();
    for (p = 0; p < nph; p++) {
	k += numDists[p];
	if (senid < k)
	    return p;
    }
    return (-1);
}


int32 *
hmm_get_psen ( void )
{
    return numDists;
}
