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
 * tmat.c
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
 * 20-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed tmat_read to use the new libio/bio_fread functions.
 * 
 * 13-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created, liberally borrowed from Eric Thayer's S3 trainer.
 */


#include "tmat.h"
#include "vector.h"
#include "logs3.h"

#include <libio/libio.h>
#include <libutil/libutil.h>

#include <assert.h>
#include <string.h>


#define TMAT_PARAM_VERSION		"1.0"


static tmat_t *tmat;		/* The set of transition probability matrices */


#if 0
void tmat_dump (float32 ***tmat, int32 d1, int32 d2, int32 d3)
{
    int32 i, j, k;
    
    for (i = 0; i < d1; i++) {
	E_INFO ("TMAT %d:\n", i);
	for (j = 0; j < d2; j++) {
	    for (k = 0; k < d3; k++)
		printf (" %.3e", tmat[i][j][k]);
	    printf ("\n");
	}
	printf ("\n");
    }
}
#endif


static int32 tmat_read(tmat_t *t, char *file_name)
{
    char version[1024], tmp;
    int32 row_dim;
    FILE *fp;
    int32 byteswap, chksum_present;
    uint32 chksum;
    float32 **tp;
    int32 i, j, k, tp_per_tmat;
    char **argname, **argval;

    E_INFO("Reading HMM transition matrix: %s\n", file_name);

    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], TMAT_PARAM_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
			file_name, argval[i], TMAT_PARAM_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;
    
    chksum = 0;
    
    /* Read #tmat, #from-states, #to-states, arraysize */
    if ((bio_fread (&(t->n_tmat),  sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&row_dim,      sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(t->n_state), sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&i, sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
	E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if (t->n_state != row_dim+1) {
	E_FATAL("%s: #from-states(%d) != #to-states(%d)-1\n",
		file_name, row_dim, t->n_state);
    }
    if (i != t->n_tmat * row_dim * t->n_state) {
	E_FATAL("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
		file_name, i, t->n_tmat, row_dim, t->n_state);
    }

    /* Allocate memory for tmat data */
    t->tp = (int32 ***) ckd_calloc_3d (t->n_tmat, t->n_state-1, t->n_state, sizeof(int32));

    /* Temporary structure to read in the float data */
    tp = (float32 **) ckd_calloc_2d (t->n_state-1, t->n_state, sizeof(float32));

    /* Read transition matrices, normalize and floor them, and convert to logs3 domain */
    tp_per_tmat = (t->n_state-1) * t->n_state;
    for (i = 0; i < t->n_tmat; i++) {
	if (bio_fread (tp[0], sizeof(float32), tp_per_tmat, fp,
		       byteswap, &chksum) != tp_per_tmat) {
	    E_FATAL("fread(%s) (arraydata) failed\n", file_name);
	}
	
	/* Normalize and floor */
	for (j = 0; j < t->n_state-1; j++) {
	    if (vector_normalize (tp[j], t->n_state) != S3_SUCCESS)
		E_ERROR("Normalization failed for tmat %d from state %d\n", i, j);
	    vector_nz_floor (tp[j], t->n_state, t->tpfloor);
	    vector_normalize (tp[j], t->n_state);

	    /* Convert to logs3.  Take care of special case when tp = 0.0! */
	    for (k = 0; k < t->n_state; k++) {
		t->tp[i][j][k] = (tp[j][k] == 0.0) ? LOGPROB_ZERO : logs3(tp[j][k]);
	    }
	}
    }

    ckd_free_2d ((void **) tp);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);

    if (fread (&tmp, 1, 1, fp) == 1)
	E_ERROR("Non-empty file beyond end of data\n");

    fclose(fp);

    E_INFO("Read %d transition matrices of size %dx%d\n",
	   t->n_tmat, t->n_state-1, t->n_state);

    return S3_SUCCESS;
}


#if 0
static void tmat_dump (tmat_t *t)
{
    int32 i, from, to;
    for (i = 0; i < t->n_tmat; i++) {
	printf ("[%3d]\n", i);
	for (from = 0; from < t->n_state-1; from++) {
	    printf ("\t");
	    for (to = 0; to < t->n_state; to++)
		printf (" %12d", t->tp[i][from][to]);
	    printf ("\n");
	}
	printf ("\n");
    }
}
#endif


/*
 * Check model tprob matrices that they conform to upper-diagonal assumption.
 */
int32 tmat_chk_uppertri (tmat_t *tmat)
{
    int32 i, from, to;
    
    /* Check that each tmat is upper-triangular */
    for (i = 0; i < tmat->n_tmat; i++) {
	for (to = 0; to < tmat->n_state-1; to++)
	    for (from = to+1; from < tmat->n_state-1; from++)
		if (tmat->tp[i][from][to] > LOGPROB_ZERO) {
		    E_ERROR("Tmat %d not upper triangular\n", i);
		    return -1;
		}
    }

    return 0;
}


tmat_t *tmat_init (char *tmatfile, float32 tpfloor)
{
    tmat = (tmat_t *) ckd_calloc (1, sizeof(tmat_t));
    tmat->tpfloor = tpfloor;

    tmat_read (tmat, tmatfile);
    
    return tmat;
}


tmat_t *tmat_gettmat ( void )
{
    return tmat;
}
