/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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
 * senone.c -- Mixture density weights associated with each tied state.
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
 * $Log$
 * Revision 1.2  2002/12/03  23:02:44  egouvea
 * Updated slow decoder with current working version.
 * Added copyright notice to Makefiles, *.c and *.h files.
 * Updated some of the documentation.
 * 
 * Revision 1.1.1.1  2002/12/03 20:20:46  robust
 * Import of s3decode.
 *
 * 
 * 06-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added handling of .semi. and .cont. special cases for senone-mgau
 * 		mapping.
 * 
 * 20-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed senone_mixw_read and senone_mgau_map_read to use the new
 *		libio/bio_fread functions.
 * 
 * 20-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Modified senone_eval to accommodate both normal and transposed
 *		senone organization.
 * 
 * 13-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added implementation of senone_mgau_map_read.
 * 		Added senone_eval_all() optimized for the semicontinuous case.
 * 
 * 12-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#include "senone.h"
#include "logs3.h"

#include <libio/libio.h>
#include <libutil/libutil.h>

#include <string.h>
#include <assert.h>


#define MIXW_PARAM_VERSION	"1.0"
#define SPDEF_PARAM_VERSION	"1.2"


static int32 senone_mgau_map_read (senone_t *s, char *file_name)
{
    FILE *fp;
    int32 byteswap, chksum_present, n_gauden_present;
    uint32 chksum;
    int32 i;
    char eofchk;
    char **argname, **argval;
    float32 v;
    
    E_INFO("Reading senone gauden-codebook map file: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    n_gauden_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], SPDEF_PARAM_VERSION) != 0) {
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
		       file_name, argval[i], SPDEF_PARAM_VERSION);
	    }
	    
	    /* HACK!! Convert version# to float32 and take appropriate action */
	    if (sscanf (argval[i], "%f", &v) != 1)
		E_FATAL("%s: Bad version no. string: %s\n", file_name, argval[i]);

	    n_gauden_present = (v > 1.1) ? 1 : 0;
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;
    
    /* Read #gauden (if version matches) */
    if (n_gauden_present) {
	if (bio_fread (&(s->n_gauden), sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	    E_FATAL("fread(%s) (#gauden) failed\n", file_name);
    }
    
    /* Read 1d array data */
    if (bio_fread_1d ((void **)(&s->mgau), sizeof(s3mgauid_t), &(s->n_sen), fp,
		      byteswap, &chksum) < 0) {
	E_FATAL("bio_fread_1d(%s) failed\n", file_name);
    }
    
    /* Infer n_gauden if not present in this version */
    if (! n_gauden_present) {
	s->n_gauden = 1;
	for (i = 0; i < s->n_sen; i++)
	    if (s->mgau[i] >= s->n_gauden)
		s->n_gauden = s->mgau[i]+1;
    }
    
    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read %d->%d senone-codebook mappings\n", s->n_sen, s->n_gauden);

    return S3_SUCCESS;
}


static int32 senone_mixw_read(senone_t *s, char *file_name)
{
    char eofchk;
    FILE *fp;
    int32 byteswap, chksum_present;
    uint32 chksum;
    float32 *pdf;
    int32 i, f, c, p, n_err;
    char **argname, **argval;

    E_INFO("Reading senone mixture weights: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], MIXW_PARAM_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
			file_name, argval[i], MIXW_PARAM_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #senones, #features, #codewords, arraysize */
    if ((bio_fread (&(s->n_sen),  sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(s->n_feat), sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(s->n_cw),   sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&i,   sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
	E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if (i != s->n_sen * s->n_feat * s->n_cw) {
	E_FATAL("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
		file_name, i, s->n_sen, s->n_feat, s->n_cw);
    }
    
    /*
     * Compute #LSB bits to be dropped to represent mixwfloor with 8 bits.
     * All PDF values will be truncated (in the LSB positions) by these many bits.
     */
    if ((s->mixwfloor <= 0.0) || (s->mixwfloor >= 1.0))
	E_FATAL("mixwfloor (%e) not in range (0, 1)\n", s->mixwfloor);
    p = logs3(s->mixwfloor);
    for (s->shift = 0, p = -p; p >= 256; s->shift++, p >>= 1);
    E_INFO("Truncating senone logs3(pdf) values by %d bits, to 8 bits\n", s->shift);

    /*
     * Allocate memory for senone PDF data.  Organize normally or transposed depending on
     * s->n_gauden.
     */
    if (s->n_gauden > 1) {
	s->pdf = (senprob_t ***) ckd_calloc_3d (s->n_sen, s->n_feat, s->n_cw,
						sizeof(senprob_t));
    } else {
	s->pdf = (senprob_t ***) ckd_calloc_3d (s->n_feat, s->n_cw, s->n_sen,
						sizeof(senprob_t));
    }
    
    /* Temporary structure to read in floats */
    pdf = (float32 *) ckd_calloc (s->n_cw, sizeof(float32));

    /* Read senone probs data, normalize, floor, convert to logs3, truncate to 8 bits */
    n_err = 0;
    for (i = 0; i < s->n_sen; i++) {
	for (f = 0; f < s->n_feat; f++) {
	    if (bio_fread((void *)pdf, sizeof(float32), s->n_cw, fp, byteswap, &chksum)
		!= s->n_cw) {
		E_FATAL("bio_fread(%s) (arraydata) failed\n", file_name);
	    }
	    
	    /* Normalize and floor */
	    if (vector_normalize (pdf, s->n_cw) != S3_SUCCESS)
		n_err++;
	    vector_floor (pdf, s->n_cw, s->mixwfloor);
	    vector_normalize (pdf, s->n_cw);

	    /* Convert to logs3, truncate to 8 bits, and store in s->pdf */
	    for (c = 0; c < s->n_cw; c++) {
		p = -(logs3(pdf[c]));
		p += (1 << (s->shift-1)) - 1;	/* Rounding before truncation */

		if (s->n_gauden > 1)
		    s->pdf[i][f][c] = (p < (255 << s->shift)) ? (p >> s->shift) : 255;
		else
		    s->pdf[f][c][i] = (p < (255 << s->shift)) ? (p >> s->shift) : 255;
	    }
	}
    }
    if (n_err > 0)
	E_WARN("Weight normalization failed for %d senones\n", n_err);

    ckd_free (pdf);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read mixture weights for %d senones: %d features x %d codewords\n",
	   s->n_sen, s->n_feat, s->n_cw);
    
    return S3_SUCCESS;
}


senone_t *senone_init (char *mixwfile, char *sen2mgau_map_file, float32 mixwfloor)
{
    senone_t *s;
    int32 n, i;
    
    assert (sen2mgau_map_file);
    
    s = (senone_t *) ckd_calloc (1, sizeof(senone_t));
    s->mixwfloor = mixwfloor;

    if (strcmp (sen2mgau_map_file, ".semi.") == 0)
	s->n_gauden = 1;
    else if (strcmp (sen2mgau_map_file, ".cont.") == 0)
	s->n_gauden = 2;	/* HACK!! Dummy value >1 for the moment; fixed below */
    else {
	senone_mgau_map_read (s, sen2mgau_map_file);
	n = s->n_sen;
    }
    
    senone_mixw_read (s, mixwfile);
    
    if (strcmp (sen2mgau_map_file, ".semi.") == 0) {
	/* All-to-1 senones-codebook mapping */
	s->mgau = (s3mgauid_t *) ckd_calloc (s->n_sen, sizeof(s3mgauid_t));
    } else if (strcmp (sen2mgau_map_file, ".cont.") == 0) {
	/* 1-to-1 senone-codebook mapping */
	if (s->n_sen <= 1)
	    E_FATAL("#senone=%d; must be >1\n", s->n_sen);
	
	s->mgau = (s3mgauid_t *) ckd_calloc (s->n_sen, sizeof(s3mgauid_t));
	for (i = 0; i < s->n_sen; i++)
	    s->mgau[i] = i;

	s->n_gauden = s->n_sen;
    } else {
	if (s->n_sen != n)
	    E_FATAL("#senones inconsistent: %d in %s; %d in %s\n",
		    n, sen2mgau_map_file, s->n_sen, mixwfile);
    }
    
    return s;
}


/*
 * Compute senone score for one senone.
 * NOTE:  Remember that senone PDF tables contain SCALED logs3 values.
 * NOTE:  Remember also that PDF data may be transposed or not depending on s->n_gauden.
 */
int32 senone_eval (senone_t *s, s3senid_t id, gauden_dist_t **dist, int32 n_top)
{
    int32 scr;		/* total senone score */
    int32 fscr;		/* senone score for one feature */
    int32 fwscr;	/* senone score for one feature, one codeword */
    int32 f, t;
    gauden_dist_t *fdist;
    
    assert ((id >= 0) && (id < s->n_sen));
    assert ((n_top > 0) && (n_top <= s->n_cw));

    scr = 0;

    for (f = 0; f < s->n_feat; f++) {
	fdist = dist[f];

	/* Top codeword for feature f */
	fscr = (s->n_gauden > 1) ?
	    fdist[0].dist - (s->pdf[id][f][fdist[0].id] << s->shift) :	/* untransposed */
	    fdist[0].dist - (s->pdf[f][fdist[0].id][id] << s->shift);	/* transposed */
	
	/* Remaining of n_top codewords for feature f */
	for (t = 1; t < n_top; t++) {
	    fwscr = (s->n_gauden > 1) ?
		fdist[t].dist - (s->pdf[id][f][fdist[t].id] << s->shift) :
		fdist[t].dist - (s->pdf[f][fdist[t].id][id] << s->shift);
	    fscr = logs3_add (fscr, fwscr);
	}
	
	scr += fscr;
    }

    return scr;
}


/*
 * Optimized for special case of all senones sharing one codebook (perhaps many features).
 * In particular, the PDF tables are transposed in memory.
 */
void senone_eval_all (senone_t *s, gauden_dist_t **dist, int32 n_top, int32 *senscr)
{
    int32 i, f, k, cwdist, scr;
    static int32 *featscr = NULL;
    senprob_t *pdf;
    
    assert (s->n_gauden == 1);
    assert ((n_top > 0) && (n_top <= s->n_cw));
    
    if ((s->n_feat > 1) && (! featscr))
	featscr = (int32 *) ckd_calloc (s->n_sen, sizeof(int32));
    
    /* Feature 0 */
    /* Top-N codeword 0 */
    cwdist = dist[0][0].dist;
    pdf = s->pdf[0][dist[0][0].id];

    for (i = 0; i < s->n_sen; i++)
	senscr[i] = cwdist - (pdf[i] << s->shift);

    /* Remaining top-N codewords */
    for (k = 1; k < n_top; k++) {
	cwdist = dist[0][k].dist;
	pdf = s->pdf[0][dist[0][k].id];
	
	for (i = 0; i < s->n_sen; i++) {
	    scr = cwdist - (pdf[i] << s->shift);
	    senscr[i] = logs3_add (senscr[i], scr);
	}
    }
    
    /* Remaining features */
    for (f = 1; f < s->n_feat; f++) {
	/* Top-N codeword 0 */
	cwdist = dist[f][0].dist;
	pdf = s->pdf[f][dist[f][0].id];
	
	for (i = 0; i < s->n_sen; i++)
	    featscr[i] = cwdist - (pdf[i] << s->shift);
	
	/* Remaining top-N codewords */
	for (k = 1; k < n_top; k++) {
	    cwdist = dist[f][k].dist;
	    pdf = s->pdf[f][dist[f][k].id];
	    
	    for (i = 0; i < s->n_sen; i++) {
		scr = cwdist - (pdf[i] << s->shift);
		featscr[i] = logs3_add (featscr[i], scr);
	    }
	}
	
	for (i = 0; i < s->n_sen; i++)
	    senscr[i] += featscr[i];
    }
}
