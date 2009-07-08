/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.7  2006/02/22  17:46:26  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH:
 * 1, Used new log function in logs3
 * 2, Fixed dox-doc.
 * 
 *
 * Revision 1.5.4.1  2005/07/05 21:28:57  arthchan2003
 * 1, Merged from HEAD. 2, Remove redundant keyword in cont_mgau.
 *
 * Revision 1.6  2005/07/05 13:12:39  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.5  2005/06/21 19:23:35  arthchan2003
 * 1, Fixed doxygen documentation. 2, Added $ keyword.
 *
 * Revision 1.5  2005/05/03 04:09:09  archan
 * Implemented the heart of word copy search. For every ci-phone, every word end, a tree will be allocated to preserve its pathscore.  This is different from 3.5 or below, only the best score for a particular ci-phone, regardless of the word-ends will be preserved at every frame.  The graph propagation will not collect unused word tree at this point. srch_WST_propagate_wd_lv2 is also as the most stupid in the century.  But well, after all, everything needs a start.  I will then really get the results from the search and see how it looks.
 *
 * Revision 1.4  2005/04/21 23:50:26  archan
 * Some more refactoring on the how reporting of structures inside kbcore_t is done, it is now 50% nice. Also added class-based LM test case into test-decode.sh.in.  At this moment, everything in search mode 5 is already done.  It is time to test the idea whether the search can really be used.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added tmat_free to free allocated memory 
 *
 * 29-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added tmat_chk_1skip(), and made tmat_chk_uppertri() public.
 * 
 * 10-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Made tmat_dump() public.
 * 
 * 11-Mar-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started based on original S3 implementation.
 */


#include <string.h>

#include "tmat.h"
#include "bio.h"
#include "vector.h"
#include "logs3.h"


#define TMAT_PARAM_VERSION		"1.0"


void
tmat_dump(tmat_t * tmat, FILE * fp)
{
    int32 i, src, dst;

    for (i = 0; i < tmat->n_tmat; i++) {
        fprintf(fp, "TMAT %d = %d x %d\n", i, tmat->n_state,
                tmat->n_state + 1);
        for (src = 0; src < tmat->n_state; src++) {
            for (dst = 0; dst <= tmat->n_state; dst++)
                fprintf(fp, " %12d", tmat->tp[i][src][dst]);
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }
    fflush(fp);
}


/*
 * Check model tprob matrices that they conform to upper-triangular assumption;
 * i.e. no "backward" transitions allowed.
 */
int32
tmat_chk_uppertri(tmat_t * tmat)
{
    int32 i, src, dst;

    /* Check that each tmat is upper-triangular */
    for (i = 0; i < tmat->n_tmat; i++) {
        for (dst = 0; dst < tmat->n_state; dst++)
            for (src = dst + 1; src < tmat->n_state; src++)
                if (tmat->tp[i][src][dst] > S3_LOGPROB_ZERO)
                    return -1;
    }

    return 0;
}


int32
tmat_chk_1skip(tmat_t * tmat)
{
    int32 i, src, dst;

    for (i = 0; i < tmat->n_tmat; i++) {
        for (src = 0; src < tmat->n_state; src++)
            for (dst = src + 3; dst <= tmat->n_state; dst++)
                if (tmat->tp[i][src][dst] > S3_LOGPROB_ZERO)
                    return -1;
    }

    return 0;
}


tmat_t *
tmat_init(const char *file_name, float64 tpfloor, int32 breport, logmath_t *logmath)
{
    char tmp;
    int32 n_src, n_dst;
    FILE *fp;
    int32 byteswap, chksum_present;
    uint32 chksum;
    float32 **tp;
    int32 i, j, k, tp_per_tmat;
    char **argname, **argval;
    tmat_t *t;


    if (breport) {
        E_INFO("Reading HMM transition probability matrices: %s\n",
               file_name);
    }

    t = (tmat_t *) ckd_calloc(1, sizeof(tmat_t));
    t->logmath = logmath;

    if ((fp = fopen(file_name, "rb")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);

    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr(fp, &argname, &argval, &byteswap) < 0)
        E_FATAL("bio_readhdr(%s) failed\n", file_name);

    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
        if (strcmp(argname[i], "version") == 0) {
            if (strcmp(argval[i], TMAT_PARAM_VERSION) != 0)
                E_WARN("Version mismatch(%s): %s, expecting %s\n",
                       file_name, argval[i], TMAT_PARAM_VERSION);
        }
        else if (strcmp(argname[i], "chksum0") == 0) {
            chksum_present = 1; /* Ignore the associated value */
        }
    }
    bio_hdrarg_free(argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #tmat, #from-states, #to-states, arraysize */
    if ((bio_fread(&(t->n_tmat), sizeof(int32), 1, fp, byteswap, &chksum)
         != 1)
        || (bio_fread(&n_src, sizeof(int32), 1, fp, byteswap, &chksum) !=
            1)
        || (bio_fread(&n_dst, sizeof(int32), 1, fp, byteswap, &chksum) !=
            1)
        || (bio_fread(&i, sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
        E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if (t->n_tmat >= MAX_S3TMATID)
        E_FATAL("%s: #tmat (%d) exceeds limit (%d)\n", file_name,
                t->n_tmat, MAX_S3TMATID);
    if (n_dst != n_src + 1)
        E_FATAL("%s: #from-states(%d) != #to-states(%d)-1\n", file_name,
                n_src, n_dst);
    t->n_state = n_src;

    if (i != t->n_tmat * n_src * n_dst) {
        E_FATAL
            ("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
             file_name, i, t->n_tmat, n_src, n_dst);
    }

    /* Allocate memory for tmat data */
    t->tp =
        (int32 ***) ckd_calloc_3d(t->n_tmat, n_src, n_dst, sizeof(int32));

    /* Temporary structure to read in the float data */
    tp = (float32 **) ckd_calloc_2d(n_src, n_dst, sizeof(float32));

    /* Read transition matrices, normalize and floor them, and convert to logs3 domain */
    tp_per_tmat = n_src * n_dst;
    for (i = 0; i < t->n_tmat; i++) {
        if (bio_fread(tp[0], sizeof(float32), tp_per_tmat, fp,
                      byteswap, &chksum) != tp_per_tmat) {
            E_FATAL("fread(%s) (arraydata) failed\n", file_name);
        }

        /* Normalize and floor */
        for (j = 0; j < n_src; j++) {
            if (vector_sum_norm(tp[j], n_dst) == 0.0)
                E_WARN("Normalization failed for tmat %d from state %d\n",
                       i, j);
            vector_nz_floor(tp[j], n_dst, tpfloor);
            vector_sum_norm(tp[j], n_dst);

            /* Convert to logs3.  Take care of special case when tp = 0.0! */
            for (k = 0; k < n_dst; k++)
                t->tp[i][j][k] =
                    (tp[j][k] == 0.0) ? S3_LOGPROB_ZERO : logs3(logmath, tp[j][k]);
        }
    }

    ckd_free_2d((void **) tp);

    if (chksum_present)
        bio_verify_chksum(fp, byteswap, chksum);

    if (fread(&tmp, 1, 1, fp) == 1)
        E_ERROR("Non-empty file beyond end of data\n");

    fclose(fp);


    if (tmat_chk_uppertri(t) < 0)
        E_FATAL("Tmat not upper triangular\n");

    return t;
}

void
tmat_report(tmat_t * t)
{
    E_INFO_NOFN("Initialization of tmat_t, report:\n");
    E_INFO_NOFN("Read %d transition matrices of size %dx%d\n",
                t->n_tmat, t->n_state, t->n_state + 1);
    E_INFO_NOFN("\n");

}

/* 
 *  RAH, Free memory allocated in tmat_init ()
 */
void
tmat_free(tmat_t * t)
{
    if (t) {
        if (t->tp)
            ckd_free_3d((void ***) t->tp);
        ckd_free((void *) t);
    }
}


#if _TMAT_TEST_
/* RAH, April 26th, 2001, opened file tmat_test.out and added tmat_free(t) call, there are no memory leaks here */
main(int32 argc, char *argv[])
{
    tmat_t *t;
    float64 flr;
    FILE *fp;

    if (argc < 3)
        E_FATAL("Usage: %s tmat floor\n", argv[0]);
    if (sscanf(argv[2], "%lf", &flr) != 1)
        E_FATAL("Usage: %s tmat floor\n", argv[0]);

    fp = fopen("tmat_test.out", "wt");
    if (!fp) {
        fprintf(stderr, "Unable to topen tmat_test.out for writing\n");
        exit(-1);
    }

    logs3_init((float64) 1.0001, 1, 1);

    t = tmat_init(argv[1], flr);

    tmat_dump(t, fp);
    tmat_free(t);
}
#endif
