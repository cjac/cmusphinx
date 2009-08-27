/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/** \file lm_ng_dmp.c
    \brief DMP format LM manipulation
*/

/*
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: lm_ng_dmp.c,v $
 * 2008/11/5  P. Deleglise and Y. Esteve
 * Adapted the code to manage large files (off_t 64 bits instead of int32)
 *
 * 2008/10/20  N. Coetmeur, supervised by Y. Esteve
 * When reading a dump file without trigrams, read it as a trigram dump file
 *
 * 2008/08/21  N. Coetmeur, supervised by Y. Esteve
 * Add conditionnal statements (#ifndef _READ_DUMP_ONLY_) for parts of code that we must
 * deactivating for use LM model with our others tools.
 *
 * 2008/06/27  N. Coetmeur, supervised by Y. Esteve
 * Adjust comments for compatibility with Doxygen 1.5.6
 *
 * 2008/06/26  N. Coetmeur, supervised by Y. Esteve
 * Adapt LM functions for working with N-grams for each N value
 * (i.e for a trigram file N=3, for a quadrigram file N=4 and so on...).
 * - Some functions are renamed:
 *   > lm3g_dump* functions renamed lmng_dump*
 *   > lm_read_dump_ng renamed lm_read_dump_all_ng
 *   > lm_read_dump_calloc_membg_tginfo renamed lm_read_dump_calloc_membg_nginfo
 * - Many functions are replaced by another one working for N-grams,
 *  N given in parameter (not for fwrite_*g*):
 *   > lm3g_dump_write_unigram, lm3g_dump_write_bigram, lm3g_dump_write_trigram
 *    are raplaced by lmng_dump_write_ngram
 *   > lm3g_dump_write_bgprob, lm3g_dump_write_tgprob are raplaced by
 *     lmng_dump_write_ngprob
 *   > lm3g_dump_write_tgbowt is replaced by lmng_dump_write_ngbowt
 *   > lm3g_dump_write_tg_segbase is replaced by lmng_dump_write_ng_segbase
 *   > lm_read_dump_bg, lm_read_dump_tg are replaced by lm_read_dump_ng
 *   > lm_read_dump_bgprob, lm_read_dump_tgprob are raplaced by
 *     lm_read_dump_ngprob
 *   > lm_read_dump_tgbowt is replaced by lm_read_dump_ngbowt
 *   > lm_read_dump_tg_segbase is replaced by lm_read_dump_ng_segbase
 *   > fwrite_bg, fwrite_bg32 are raplaced by
 *     fwrite_ng, fwrite_ng32 (n>=2 and n<=maxN-1)
 *   > fwrite_tg, fwrite_tg32 are raplaced by fwrite_lg, fwrite_lg32
 *    and work for the last N level in LM file
 *    (i.e don't write bowtid and firstnng fields)
 * - A part of fmtdesc table string is dynamically written in the
 *  lmng_dump_write_fmtdesc function depending on the max N value in the LM file
 * - The Darpa header string changed :
 *   > darpa_hdr is renamed darpa_tg_hdr
 *   > two headers are adding : darpa_qg_hdr (for a 4-gram file LM, already
 *    existing in other application) and darpa_ng_hdr (a template for a
 *    5-gram and more file LM, including a decimal value that is the max N
 *    value in the file)
 *   > the lm_read_dump_header function set the max N level
 *    depending on the header
 * - Miscellaneous changes:
 *   > the lm_read_dump_ver_nug function return (by pointers) version and n_ug
 *    values (instead of set this values in a lm_t structure pointer)
 *   > the lm_read_dump_ng_counts function takes the max N value in parameter
 *
 * Revision 1.4  2006/03/03 00:42:36  egouvea
 * In bio.h, definition of REVERSE_SWAP_... depends on WORDS_BIGENDIAN,
 * since __BIG_ENDIAN__ isn't defined.
 *
 * In lm_3g_dmp.c, swap bigram and trigram values if needed.
 *
 * In lm_convert regresssion test, allow for tolerance (< 0.0002) when
 * comparing the results.
 *
 * Revision 1.3  2006/03/02 00:35:08  arthchan2003
 * Merged the logic in share/lm3g2dmp to here. It will take care the situation
 * when log_bg_seg_sz is different. (Must be an old format Ravi played with in
 * the past). This will match the reading code also generalize the old
 * sphinx 2's logic a little bit.
 *
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert LM to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.1  2005/07/17 05:23:25  arthchan2003
 * Added lm_3g_dmp.c and lmset.c, split it out from lm.c
 * to avoid overcrowding situation in it.
 *
 *
 */

#include "lm.h"
#include "s3types.h"
#include "bio.h"


/** \name Number of bits */
/*\{*/
#define IS32BITS 1
#define IS16BITS 0
/*\}*/


/**
   \name N-gram headers

   \note ARCHAN: Please do not change it.
   Legacy code use this string to match the header of the LM DMP model.
   If we change it, lm_read_dump won't work. 
*/
/*\{*/
const char *darpa_ng_hdr = "Darpa %d-gram LM";      /**< pattern for N >= 5 */
const char *darpa_qg_hdr = "Darpa Quadrigram LM";   /**<  string for N == 4 */
const char *darpa_tg_hdr = "Darpa Trigram LM";      /**<  string for N <= 3 */
/*\}*/


#ifndef _READ_DUMP_ONLY_

/**
    \brief File format description

    Written in DMP files.
*/
static char const *fmtdesc[] = {
 "BEGIN FILE FORMAT DESCRIPTION",
 "Header string length (int32) and string (including trailing 0)",
 "Original LM filename string-length (int32) and filename (including trailing 0)",
 "(int32) version number (present if value <= 0)",
 "(int32) original LM file modification timestamp (if version present)",
 "(int32) string-length and string (including trailing 0) (if version present)",
 "... previous entry continued any number of times (if version present)",
 "(int32) 0 (terminating sequence of strings) (if version present)",
 "(int32) log_ng_seg_sz (N-2 values for a N-gram file, present if at least one value is different from default value of LOG2_NG_SEG_SZ)",
 "(int32) lm_t.1count (must be > 0)",
 "(int32) lm_t.2count",
 NULL,
};

#endif


/**
  \brief Read a 32-bits integer value
    \return Value read
*/
static int32
lm_fread_int32  (lm_t * lm  /**< In: language model */
                 )
{
    int32 val;

    if (fread(&val, sizeof(int32), 1, lm->fp) != 1) {
        perror ("error");
        E_FATAL ("fread failed\n");
    }
    if (lm->byteswap)
        SWAP_INT32(&val);
    return (val);
}


/**   
    \brief Read header in DMP file

    
    \note 20060303: ARCHAN

    lm_read_dump_header will read in the DMP format. What it will do
    is to compare the value read in with the darpa_hdr ("Darpa
    Trigram LM").  If it matches, that means there is no byte
    swap. If it doesn't, we will try to swap the value and match the
    header again.  If it still doesn't work, that means something is
    wrong. (e.g. Format problem of the DMP file).  

    This process will also allow us to know the byte-order of the
    DMP file. Swapping could then automatically done in the code. 


    \return Error code
*/
static int32
lm_read_dump_header (lm_t * lm,         /**< In/Out: language model */
                     const char *file,  /**< In: the file we are reading */
                     int32 *max_ng      /**< Out: N-gram format in header */
                     )
{
    int32 hsz, k = 0;
    char str[26], str2[26];

    /* Standard header string-size */
    if (fread(&hsz, sizeof(int32), 1, lm->fp) != 1)
        E_FATAL("fread(%s) failed\n", file);


    /* Read and verify standard header string */

    do {
        str[k] = fgetc(lm->fp);
        if (ferror(lm->fp) || feof(lm->fp)) {
            E_ERROR("fgetc(%s) failed\n", file);
            return LM_FAIL;
        }
        k++;
    } while ((str[k-1] != '\0') && (k <= 26));
    str[k-1] = '\0';

    if (sscanf(str, darpa_ng_hdr, max_ng) == 1) {
        sprintf(str2, darpa_ng_hdr, *max_ng);
        if (strncmp(str, str2, k) != 0)
            (*max_ng) = 0;
        /* else : N-gram header */
    }
    else
        (*max_ng) = 0;

    if ((*max_ng) <= 0) {
        if (strncmp(str, darpa_qg_hdr, k) != 0) {
            if (strncmp(str, darpa_tg_hdr, k) != 0) {
                E_ERROR("Bad header\n");
                return LM_FAIL;
            }
            else /* trigram header */
                *max_ng = 3;
        }
        else /* quadrigram header */
            *max_ng = 4;
    }


    /* Set byteswap flag based on standard header string-size */
    if ((size_t) hsz == k)
        lm->byteswap = 0;
    else {
        SWAP_INT32(&hsz);
        if ((size_t) hsz == k)
            lm->byteswap = 1;
        else {
            SWAP_INT32(&hsz);
            E_INFO("Bad magic number: %d(%08x), not an LM dumpfile??\n", hsz,
                   hsz);
            return LM_FAIL;
        }
    }

    return LM_SUCCESS;

}

/**
  \brief Read the file name in DMP file
      \return Error code
*/
static int32
lm_read_lmfilename  (lm_t       * lm,   /**< In: language model */
                     const char * file  /**< In: the file we are reading */
                     )
{
    int32 k;
    char str[1024];

    /* Original LM filename string size and string */
    k = lm_fread_int32(lm);
    if ((k < 1) || (k > 1024)) {
        E_ERROR("Bad original filename size: %d\n", k);
        return LM_FAIL;
    }
    if (fread(str, sizeof(char), k, lm->fp) != (size_t) k) {
        E_ERROR("fread(%s) failed\n", file);
        return LM_FAIL;
    }

    return LM_SUCCESS;
}

/**
   \brief Read version and number of unigrams in DMP file


   \note 20060303 ARCHAN:

   lm_read_dump_ver_nug read in the version number and number of
   unigram from a LM dump file. They are related because of legacy.
   Here is a survey of what's going on in our past routines at
   timestamp 20060303.

   Before Sphinx 3.X (X<4), the routines of reading DMP format of LM
   have appeared in 3 places.  First place is Sphinx 2's lm3g_load
   which doesn't take care of version=LMDMP_VERSION_TG_16BIT_V2.  The
   second place is share's lm3g_load which takes care of
   version=LMDMP_VERSION_TG_16BIT (-1),
   version=LMDMP_VERSION_TG_16BIT_V2 (-2) and version >
   LMDMP_VERSIONNULL (0). The last one is Sphinx 3 which is
   essentially a quick hack of Sphinx 2's version. (* Note because of
   the legacy naming system version > 0 here is actually the oldest
   version)

   What is in the version then? From the source code, you could
   backtrace the story.  At the beginning, the version number is used
   to store the number of unigram.  Hence, it is a number which can be
   larger than LMDMP_VERSIONNULL (0).  

   However, quickly, the programmer found that it doesn't make sense
   to do that.  Hence, version  soon appear. version here really
   mean the version number of the LM.

   Here is one small problem, the programmer found that log_bg_seg_sz
   needs to be changed.  So he decides to introduce
   version=LMDMP_VERSION_TG_32BIT. i.e. a version that doesn't follow
   the current default value of log_bg_seg_sz (=9)

   At 20060303, the current code assume all versions
   <LMDMP_VERSION_TG_32BIT are equivalent. This is likely to change
   because we might need to introduce version 3, 4 and 5.


   \return Error code
*/
static int32
lm_read_dump_ver_nug    (lm_t       * lm,       /**< In/Out: language model */
                         const char * file,     /**< In: the file read */
                         int32        max_Ng,   /**< In: maximum N-gram level */
                         int32      * version,  /**< Out: LM version */
                         int32      * n_ug      /**< Out: number of unigrams */
                         )
{
    int32 k;
    uint32 i;
    char str[1024];

    /* Allocate memory for log_ng_seg_sz */
    if ( (lm->log_ng_seg_sz = (uint32 *) ckd_calloc (max_Ng, sizeof(uint32)))
            == NULL ) {
        E_ERROR("Can't alloctae memory for log2(ng_seg_sz)\n");
        return LM_FAIL;
    }

    
    /* Version number.
      If present (must be <= 0); otherwise it's actually the unigram count */
    (*version) = lm_fread_int32(lm);

    if ((*version) <= 0) {
        /* Read and skip orginal file timestamp; 
           ARCHAN: Unlike the sphinx2's code, currently, the timestamp
           is not compared in Sphinx 3. 
         */
        k = lm_fread_int32(lm);

        /* Read and skip format description */
        for (;;) {
            if ((k = lm_fread_int32(lm)) == 0)
                break;
            if (fread(str, sizeof(char), k, lm->fp) != (size_t) k) {
                E_ERROR("fread(%s) failed\n", file);
                return LM_FAIL;
            }
        }

        /* Read log_ng_seg_sz if present */

        /* ARCHAN 20060304
           use lm->version == -2 (LMDMP_VERSION_TG_16BIT_V2) instead of
               lm->version  <  2, this is different from share's version
         */
        if ((*version) == LMDMP_VERSION_16BIT_V2) {
            for (i = 2; i < max_Ng; i++) {
                k = lm_fread_int32(lm);
                if ((k < 1) || (k > 15)) {
                    E_ERROR("log2(%dg_seg_sz) %d outside range 1..15 \n", i, k);
                    return LM_FAIL;
                }
                lm->log_ng_seg_sz[i-1] = k;
            }
        }
        else
            for (i = 2; i < max_Ng; i++)
                lm->log_ng_seg_sz[i-1] = LOG2_NG_SEG_SZ; /* Default */

        /* Read number of unigrams */
        (*n_ug) = lm_fread_int32(lm);

    }
    else {
        /* oldest dump file version has no version number or any of the above */

        /* No version number, actually a unigram count */
        (*n_ug) = (*version);
        for (i = 2; i < max_Ng; i++)
            lm->log_ng_seg_sz[i-1] = LOG2_NG_SEG_SZ;  /* Default */
    }


    if ((*version) == LMDMP_VERSION_32BIT) {
        if (((*n_ug) <= 0) || ((*n_ug) >= MAX_S3LMWID32)) {
            E_ERROR("Bad number of unigrams: %u (must be >0, <%u) Version %d\n",
                    (*n_ug), MAX_S3LMWID32, (*version));
            return LM_FAIL;
        }

        E_INFO("Reading LM in 32 bits format\n");
    }
    else {
        if (((*n_ug) <= 0) || ((*n_ug) >= MAX_S3LMWID)) {
            E_ERROR("Bad number of unigrams: %u (must be >0, <%u) Version %d\n",
                    (*n_ug), MAX_S3LMWID, (*version));
            return LM_FAIL;
        }
        
        if ((*version) > LMDMP_VERSIONNULL ||
            (*version) == LMDMP_VERSION_16BIT ||
            (*version) == LMDMP_VERSION_16BIT_V2) {
                E_INFO("Reading LM in 16 bits format\n");
        }
    }

    return LM_SUCCESS;
}

/**
  \brief Read the N-gram counts in DMP file (N >= 2)
      
      \return Error code
*/
static int32
lm_read_dump_ng_counts  (lm_t       * lm,       /**< In/Out: language model */
                         const char * file,     /**< In: the file read */
                         int32        max_ng    /**< In: maximum N-gram level */
                         )
{
    uint32 i;

    for (i = 2; i <= max_ng; i++) {
        lm->n_ng[i-1] = lm_fread_int32(lm);
        if (lm->n_ng[i-1] < 0) {
            E_ERROR("Bad number of %d-grams: %d\n", i, lm->n_ng[i-1]);
            return LM_FAIL;
        }

        if (lm->n_ng[i-1] > 0)
            lm->max_ng = i;
    }

    return LM_SUCCESS;
}

/**
  \brief Read unigrams in DMP file
      \return Error code
*/
static int32
lm_read_dump_ug (lm_t       * lm,   /**< In/Out: language model */
                 const char * file  /**< In: the file read */
                 )
{
    int32 i;

    assert(lm->n_ng[0] > 0);

    /* Read unigrams; remember unigram sentinel at the end! */
    lm->ug = (ug_t *) ckd_calloc(lm->n_ng[0] + 1, sizeof(ug_t));
    if (fread(lm->ug, sizeof(ug_t), lm->n_ng[0] + 1, lm->fp) !=
        (size_t) (lm->n_ng[0] + 1)) {
        E_ERROR("1-gram fread(%s) failed\n", file);
        return LM_FAIL;
    }

    if (lm->byteswap) {
        for (i = 0; i <= lm->n_ng[0]; i++) {
            SWAP_INT32(&(lm->ug[i].dictwid));
            SWAP_INT32(&(lm->ug[i].prob.l));
            SWAP_INT32(&(lm->ug[i].bowt.l));
            SWAP_INT32(&(lm->ug[i].firstbg));
        }
    }
    E_INFO("Read %8d 1-grams [in memory]\n", lm->n_ng[0]);
    return LM_SUCCESS;
}

/**
  \brief Allocate memory for tables to locating N-grams
      \return Error code
*/
static int32
lm_read_dump_calloc_membg_nginfo(lm_t       * lm, /**< In/Out: language model */
                                 const char * file, /**< In: the file read */
                                 int          is32bits /**< In: is LM in 32-bits
                                                                       or not */
                                 )
{
    int32 mem_sz, mem_sz2;
    void *lmptr = NULL;
    uint32 i;

    mem_sz = is32bits ? sizeof(membg32_t) : sizeof(membg_t);
    mem_sz2 = is32bits ? sizeof(nginfo32_t *) : sizeof(nginfo_t *);

    /* membg */
    if (lm->n_ng[1] > 0) {
        if ((lmptr = ckd_calloc(lm->n_ng[0], mem_sz)) == NULL) {
            E_ERROR ("Failed to allocate memory for membg\n");
            return LM_FAIL;
        }
    }
    if (is32bits)
        lm->membg32 = (membg32_t *) lmptr;
    else
        lm->membg = (membg_t *) lmptr;

    /* nginfo */
    for (i = 3; i <= lm->max_ng; i++)
    {
        lmptr = NULL;
        if (lm->n_ng[i-1] > 0) {
            if ((lmptr = ckd_calloc(lm->n_ng[0], mem_sz2)) == NULL) {
                E_ERROR ("Failed to allocate memory for %dginfo\n", i);
                return LM_FAIL;
            }
        }
        if (is32bits)
            lm->nginfo32[i-1] = (nginfo32_t **) lmptr;
        else
            lm->nginfo[i-1] = (nginfo_t **) lmptr;
    }

    return LM_SUCCESS;
}

/**
  \brief Read N-gram probabilities in DMP file
      \return Error code
*/
static int32
lm_read_dump_ngprob (lm_t       * lm,       /**< In/Out: language model */
                     uint32       N,        /**< In: N-gram level (N>=2) */
                     const char * file,     /**< In: the file read */
                     int32        is32bits  /**< In: is LM in 32-bits or not */
                     )
{
    int32 i;
    uint32 upper_limit;

    if (N < 2) {
        E_ERROR("lm_read_dump_ngprob don't work with N=%d\n", N);
        return LM_FAIL;
    }

    upper_limit = is32bits ? LM_SPHINX_CONSTANT : LM_LEGACY_CONSTANT;

    if (lm->n_ng[N-1] > 0) {
        /* N-gram probabilities table size */
        lm->n_ngprob[N-1] = lm_fread_int32(lm);
        if ((lm->n_ngprob[N-1] <= 0) || (lm->n_ngprob[N-1] > upper_limit)) {
            E_ERROR ("Bad %d-gram probabilities table size: %d\n",
                     N, lm->n_ngprob[N-1]);
            return LM_FAIL;
        }

        /* Allocate and read N-gram probabilities table */
        lm->ngprob[N-1] = (lmlog_t *)
                            ckd_calloc(lm->n_ngprob[N-1], sizeof(lmlog_t));
        if (fread(lm->ngprob[N-1], sizeof(lmlog_t), lm->n_ngprob[N-1], lm->fp)
                != (size_t) lm->n_ngprob[N-1]) {
            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < lm->n_ngprob[N-1]; i++)
                SWAP_INT32(&(lm->ngprob[N-1][i].l));
        }

        E_INFO("%8d %d-gram probability entries\n", lm->n_ngprob[N-1], N);
    }

    return LM_SUCCESS;
}

/**
  \brief Read N-gram back-off weight in DMP file
      \return Error code
*/
static int32
lm_read_dump_ngbowt (lm_t       * lm,       /**< In/Out: language model */
                     uint32       N,        /**< In: N-gram level (N>=3) */
                     const char * file,     /**< In: the file read */
                     int32        is32bits  /**< In: is LM in 32 bits or not */
                     )
{
    int32 i;
    uint32 upper_limit;

    if (N < 3) {
        E_ERROR("lm_read_dump_ngbowt don't work with N=%d\n", N);
        return LM_FAIL;
    }

    upper_limit = is32bits ? LM_SPHINX_CONSTANT : LM_LEGACY_CONSTANT;

    if (lm->n_ng[N-1] > 0) {
        /* N-gram back-off weights table size */
        lm->n_ngbowt[N-1] = lm_fread_int32(lm);
        if ((lm->n_ngbowt[N-1] <= 0) || (lm->n_ngbowt[N-1] > upper_limit)) {
            E_ERROR ("Bad %d-gram back-off weights table size: %d\n",
                     N, lm->n_ngbowt[N-1]);
            return LM_FAIL;
        }

        /* Allocate and read N-gram back-off weights table */
        lm->ngbowt[N-1] = (lmlog_t *) ckd_calloc(lm->n_ngbowt[N-1], sizeof(lmlog_t));
        if (fread(lm->ngbowt[N-1], sizeof(lmlog_t), lm->n_ngbowt[N-1], lm->fp)
                != (size_t) lm->n_ngbowt[N-1]) {

            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < lm->n_ngbowt[N-1]; i++)
                SWAP_INT32(&(lm->ngbowt[N-1][i].l));
        }
        E_INFO("%8d %d-gram back-off weight entries\n", lm->n_ngbowt[N-1], N);
    }
    return LM_SUCCESS;
}

/**
  \brief Read segments base in DMP file
      \return Error code
*/
static int32
lm_read_dump_ng_segbase (lm_t       * lm,   /**< In/Out: language model */
                         uint32       N,    /**< In: N-gram level (N>=3) */
                         const char * file  /**< In: the file read */
                         )
{
    int i, k;

    if (N < 3) {
        E_ERROR("lm_read_dump_ng_segbase don't work with N=%d\n", N);
        return LM_FAIL;
    }

    if (lm->n_ng[N-1] > 0) {
        /* N-gram segments table size */
        k = lm_fread_int32(lm);
        if (k != (lm->n_ng[N-2] + 1) / (1 << (lm->log_ng_seg_sz[N-2])) + 1) {
            E_ERROR("Bad %d-gram segments table size: %d\n", N, k);
            return LM_FAIL;
        }

        /* Allocate and read N-gram segments table */
        lm->ng_segbase[N-1] = (int32 *) ckd_calloc(k, sizeof(int32));
        if (fread(lm->ng_segbase[N-1], sizeof(int32), k, lm->fp)
                != (size_t) k) {
            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < k; i++)
                SWAP_INT32(&(lm->ng_segbase[N-1][i]));
        }
        E_INFO("%8d %d-gram segments table entries (%d segment size)\n", k,
               N, 1 << (lm->log_ng_seg_sz[N-2]));
    }
    return LM_SUCCESS;
}

/**
  \brief Read word strings in DMP file
      \return Error code
*/
static int32
lm_read_dump_wordstr    (lm_t       * lm,       /**< In/Out: language model */
                         const char * file,     /**< In: the file read */
                         int32        is32bits  /**< In: is LM in 32-bits */
                         )
{
    int32 i, j, k;
    char *tmp_word_str;
    s3lmwid32_t startwid, endwid;

    /* Read word string names */
    k = lm_fread_int32(lm);
    if (k <= 0) {
        E_ERROR("Bad wordstrings size: %d\n", k);
        return LM_FAIL;
    }

    tmp_word_str = (char *) ckd_calloc(k, sizeof(char));
    if (fread(tmp_word_str, sizeof(char), k, lm->fp) != (size_t) k) {
        E_ERROR("fread(%s) failed\n", file);
        return LM_FAIL;
    }

    /* First make sure string just read contains n_ug words (PARANOIA!!) */
    for (i = 0, j = 0; i < k; i++)
        if (tmp_word_str[i] == '\0')
            j++;

    if (j != lm->n_ng[0]) {
        E_ERROR("Bad number of words: %d\n", j);
        return LM_FAIL;
    }


    startwid = endwid = (s3lmwid32_t) BAD_LMWID(lm);


    lm->wordstr = (char **) ckd_calloc(lm->n_ng[0], sizeof(char *));
    j = 0;
    for (i = 0; i < lm->n_ng[0]; i++) {
        if (strcmp(tmp_word_str + j, S3_START_WORD) == 0)
            startwid = i;
        else if (strcmp(tmp_word_str + j, S3_FINISH_WORD) == 0)
            endwid = i;

        lm->wordstr[i] = (char *) ckd_salloc(tmp_word_str + j);

#ifndef _READ_DUMP_ONLY_
        hash_table_enter(lm->HT, lm->wordstr[i], (void *)(long)i);
#endif

        j += strlen(tmp_word_str + j) + 1;
    }
    free(tmp_word_str);
    E_INFO("%8d word strings\n", i);

    /* Force prob(<s>) = MIN_PROB_F */
    if (IS_LMWID(lm, startwid)) {
        lm->ug[startwid].prob.f = MIN_PROB_F;
        lm->startlwid = startwid;
    }

    /* Force bowt(</s>) = MIN_PROB_F */
    if (IS_LMWID(lm, endwid)) {
        lm->ug[endwid].bowt.f = MIN_PROB_F;
        lm->finishlwid = endwid;
    }

    return LM_SUCCESS;
}


/**
   \brief Reading a N-gram level in DMP file

   When lm->isLM_IN_MEMORY is turned on.  A memory space will be
   allocated based.  Recorded the offset of bigram.

   When lm->isLM_IN_MEMORY is turned off, we will just skip
   (lm->n_ng[N-1]+1 * sizeof(ng)) byte memory and recorded the offset of
   N-gram. In this case, the program will be operated in disk mode.

   \note ARCHAN 20060304, First introduced 32 bits reading. This is whether
   the code is 32bit or not, lm->ng32 or lm->ng (16bits) will be used.
       
   \return Error code
*/
static int32
lm_read_dump_ng (lm_t       * lm,       /**< In/Out: language model */
                 uint32       N,        /**< In: N-gram level */
                 const char * file,     /**< In: the file read */
                 int          is32bits  /**< In: is LM in 32-bits or not */
                 )
{
    int32 i, n_ng_r;
    int32 mem_sz_a, mem_sz_r;
    void *lmptr;

    /* Number of N-grams (N>=3) might be zero */

    if (N >= 2) {
        if (N >= 3)
            assert(lm->n_ng[N-1] >= 0);
        else
            assert(lm->n_ng[N-1] > 0);
    }
    else /* N == 1 */
        return lm_read_dump_ug (lm, file);

    mem_sz_a = is32bits ? sizeof(ng32_t) : sizeof(ng_t);
    lmptr = NULL;

    if ((lm->max_ng > N) || (N < 3))
        mem_sz_r = mem_sz_a;
    else
        /* for read a N-gram file without (N+1)gram information */
        mem_sz_r = is32bits ? SIZE_OF_NG32_WITHOUT_NNG
                    : SIZE_OF_NG16_WITHOUT_NNG;

    if (lm->isLM_IN_MEMORY) {
        if ((lmptr = ckd_calloc(lm->n_ng[N-1] + 1, mem_sz_a)) == NULL) {
            E_ERROR (
             "Fail to allocate memory with size %d for %d-grams reading. Each %d-gram with mem_sz %d\n"
                     , lm->n_ng[N-1] + 1, N, N, mem_sz_a);
            return LM_FAIL;
        }
    }
    else
        lmptr = NULL;

    if (lm->n_ng[N-1] > 0) {
        lm->ngoff[N-1] = ftello(lm->fp);

        /* no N-gram sentinel in files without (N+1)gram information */
        n_ng_r = lm->n_ng[N-1] - (((lm->max_ng>N)||(N<3)) ? 0 : 1);

        if (lm->isLM_IN_MEMORY) {
            if (is32bits) {
                lm->ng32[N-1] = (ng32_t *) lmptr;
                for (i = 0; i <= n_ng_r; i++) {
                    fread(&(lm->ng32[N-1][i]), 1, mem_sz_r, lm->fp);
                    if (lm->byteswap) {
                        swap_ng32(&(lm->ng32[N-1][i]));
                    }
                }
            }
            else {
                lm->ng[N-1] = (ng_t *) lmptr;
                for (i = 0; i <= n_ng_r; i++) {
                    fread(&(lm->ng[N-1][i]), 1, mem_sz_r, lm->fp);
                    if (lm->byteswap) {
                        swap_ng(&(lm->ng[N-1][i]));
                    }
                }
            }

            E_INFO("Read %8d %d-grams [in memory]\n", lm->n_ng[N-1], N);
        }
        else {
            fseeko(lm->fp, ((off_t)(n_ng_r + 1)) * mem_sz_r, SEEK_CUR);
            E_INFO("%8d %d-grams [on disk]\n", lm->n_ng[N-1], N);
        }
    }
    return LM_SUCCESS;
}

/**
  \brief Read all N-gram levels informations in DMP file
  
  The core of reading the data structure from the LM file.  It
  also depends the version to operate.  Here is a summary of what's
  going on in each version.                                                   \n
                                                                              \n
  1, In version >0, version=-1(LMDMP_VERSION_16BIT),
  -2(LMDMP_VERSION_16BIT_V2),                                                 \n
                                                                              \n
  The code will read the file using the following sequence.                   \n
  -read unigram (*_dump_ug)                                                   \n
  -read bigram  (*_dump_bg)                                                   \n
  -for N=3 to max_ng :                                                        \n
    -read N-gram (*_dump_ng)                                                  \n
  -create mem bigram                                                          \n
  -for N=3 to max_ng :                                                        \n
    -create N-gram informations                                               \n
  -read the actual bigram probability (*_dump_ngprob)                         \n
  -for N=3 to max_ng :                                                        \n
    -read the actual N-gram back-off weight (*_dump_ngbowt)                   \n
    -read the actual N-gram probability  (*_dump_ngprob)                      \n
    -read the actual N-gram segment base.  (*_dump_ngsegbase)                 \n
  -read the word str into the code.                                           \n
                                                                              \n
  N-grams, membg, ng_infos are all in 16 bits.
  unigram in Sphinx 2, Sphinx 3.x (x<4) legacy are already 32 bits.

  ngprobs, ngbowts, ngsegbases are arrays,
  their size are all controlled by a number which in int32, we are cool here. \n
                                                                              \n
  2, In version = -3 (LMDMP_VERSION_PG_32BIT)                                 \n
                                                                              \n
  The code will read the file using the following sequence.                   \n
  -read unigram (*_dump_ug)                                                   \n
  -read bigram in 32 bits  (*_dump_ng)                                        \n
  -for N=3 to max_ng :                                                        \n
    -read N-gram in 32 bits (*_dump_ng)                                       \n
  -create mem bigram in 32 bits                                               \n
  -for N=3 to max_ng :                                                        \n
    -create N-gram info in 32 bits.                                           \n
  -read the actual bigram probability (*_dump_ngprob)                         \n
  -for N=3 to max_ng :                                                        \n
    -read the actual N-gram back-off weight (*_dump_ngbowt)                   \n
    -read the actual N-gram probability  (*_dump_ngprob)                      \n
    -read the actual N-gram segment base.  (*_dump_ngsegbase)                 \n
  -read the word str into the code.                                           \n
                                                                              \n
  At here, all data structure will use 32-bits data structures or
  address arrays as int32 arrays. However because legacy implementation
  check the size in ngprob, ngbowt.                                           \n
  I conformed to this coding style.  So except, _dump_ug, _dump_ng_segbase.
  All the code are now having is32bits arguments.  But the major difference
  between the two readings are mainly on _dump_ng.
                                                                              \n
                                                                              \n
  On coding :                                                                 \n

  Each LM DMP versions will just show out all the routines used.  We
  are aware that you could optimize it.  Please don't because it will
  kill readability in future.                                                 \n


  \return Error code
*/
static int32
lm_read_dump_all_ng (lm_t       * lm,   /**< In/Out: language model */
                     const char * file  /**< In: the file read */
                     )
{
    uint32 i;

    if (lm->version == LMDMP_VERSION_16BIT ||
        lm->version == LMDMP_VERSION_16BIT_V2 ||
        lm->version >= LMDMP_VERSIONNULL) {

        for (i = 1; i <= lm->max_ng; i++)
            if (lm_read_dump_ng(lm, i, file, IS16BITS) == LM_FAIL) {
                E_ERROR("Error in reading %d-grams\n", i);
                return LM_FAIL;
            }

        if (lm_read_dump_calloc_membg_nginfo(lm, file, IS16BITS) ==
            LM_FAIL) {
            E_ERROR
                ("Error in allocating memory for N-gram informations\n");
            return LM_FAIL;
        }

        if (lm_read_dump_ngprob(lm, 2, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading 2-gram probability\n");
            return LM_FAIL;
        }

        for (i = 3; i <= lm->max_ng; i++) {
            if (lm_read_dump_ngbowt(lm, i, file, IS16BITS) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram back-off weight\n", i);

                return LM_FAIL;
            }

            if (lm_read_dump_ngprob(lm, i, file, IS16BITS) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram probability\n", i);
                return LM_FAIL;
            }

            if (lm_read_dump_ng_segbase(lm, i, file) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram segment base\n", i);
                return LM_FAIL;
            }
        }

        if (lm_read_dump_wordstr(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading the word strings\n");
            return LM_FAIL;
        }
    }
    else if (lm->version == LMDMP_VERSION_32BIT) {

        for (i = 1; i <= lm->max_ng; i++)
            if (lm_read_dump_ng(lm, i, file, IS32BITS) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram\n", i);
                return LM_FAIL;
            }

        if (lm_read_dump_calloc_membg_nginfo(lm, file, IS32BITS) ==
            LM_FAIL) {
            E_ERROR
                ("Error in allocating memory for N-gram informations\n");
            return LM_FAIL;
        }

        if (lm_read_dump_ngprob(lm, 2, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading 2-gram probability\n");
            return LM_FAIL;
        }

        for (i = 3; i <= lm->max_ng; i++) {
            if (lm_read_dump_ngbowt(lm, i, file, IS32BITS) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram-back-off weight\n", i);
                return LM_FAIL;
            }

            if (lm_read_dump_ngprob(lm, i, file, IS32BITS) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram probability\n", i);
                return LM_FAIL;
            }

            if (lm_read_dump_ng_segbase(lm, i, file) == LM_FAIL) {
                E_ERROR("Error in reading %d-gram segment base\n", i);
                return LM_FAIL;
            }
        }

        if (lm_read_dump_wordstr(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading the word strings\n");
            return LM_FAIL;
        }

    }
    else {
        E_ERROR("Error, Format %d is unknown\n", lm->version);
        return LM_FAIL;
    }

    return LM_SUCCESS;
}

#ifndef _READ_DUMP_ONLY_

/**
  \brief Write a 32-bits integer value
*/
static void
fwrite_int32    (FILE * fp, /**< In: file stream */
                 int32  val /**< In: value to write */
                 )
{
    REVERSE_SENSE_SWAP_INT32(val);
    fwrite(&val, sizeof(int32), 1, fp);
}

/**
  \brief Write an unigram
*/
static void
fwrite_ug   (FILE * fp, /**< In: file stream */
             ug_t * ug  /**< In: unigram to write */
             )
{
    ug_t tmp_ug = *ug;

    REVERSE_SENSE_SWAP_INT32(tmp_ug.dictwid);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.prob.l);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.bowt.l);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.firstbg);
    fwrite(&tmp_ug, sizeof(ug_t), 1, fp);
}

/**
  \brief Write a 16-bits N-gram (not last level)
*/
static void
fwrite_ng   (FILE * fp, /**< In: file stream */
             ng_t * ng  /**< In: N-gram to write */
             )
{
    ng_t tmp_ng = *ng;

    REVERSE_SENSE_SWAP_INT16(tmp_ng.wid);
    REVERSE_SENSE_SWAP_INT16(tmp_ng.probid);
    REVERSE_SENSE_SWAP_INT16(tmp_ng.bowtid);
    REVERSE_SENSE_SWAP_INT16(tmp_ng.firstnng);
    fwrite(&tmp_ng, sizeof(ng_t), 1, fp);
}

/**
  \brief Write a 32-bits N-gram (not last level)
*/
static void
fwrite_ng32 (FILE   * fp,   /**< In: file stream */
             ng32_t * ng    /**< In: N-gram to write */
             )
{
    ng32_t tmp_ng = *ng;

    REVERSE_SENSE_SWAP_INT32(tmp_ng.wid);
    REVERSE_SENSE_SWAP_INT32(tmp_ng.probid);
    REVERSE_SENSE_SWAP_INT32(tmp_ng.bowtid);
    REVERSE_SENSE_SWAP_INT32(tmp_ng.firstnng);
    fwrite(&tmp_ng, sizeof(ng32_t), 1, fp);
}

/**
  \brief Write a 16-bits last level N-gram
    
  Write only word and probability IDs
*/
static void
fwrite_lg   (FILE * fp, /**< In: file stream */
             ng_t * ng  /**< In: N-gram to write */
             )
{
    ng_t tmp_ng = *ng;

    REVERSE_SENSE_SWAP_INT16(tmp_ng.wid);
    REVERSE_SENSE_SWAP_INT16(tmp_ng.probid);
    fwrite(&tmp_ng, SIZE_OF_NG16_WITHOUT_NNG, 1, fp);
}

/**
  \brief Write a 32-bits last level N-gram
    
  Write only word and probability IDs
*/
static void
fwrite_lg32 (FILE   * fp,   /**< In: file stream */
             ng32_t * ng    /**< In: N-gram to write */
             )
{
    ng32_t tmp_ng = *ng;

    REVERSE_SENSE_SWAP_INT32(tmp_ng.wid);
    REVERSE_SENSE_SWAP_INT32(tmp_ng.probid);
    fwrite(&tmp_ng, SIZE_OF_NG32_WITHOUT_NNG, 1, fp);
}

#endif



/**
   \brief Read a DMP file (<-lmname->.DMP dump LM format)
    \return Pointer to LM structure created
 */
lm_t *
lm_read_dump    (const  char *  file,       /**< In: file name */
                        int     lminmemory  /**< In: whether LM is in memory */
                 )
{
    lm_t *lm;
    int32 max_ng, version, n_ug;


    /* Allocate memory for LM */

    if ( (lm = (lm_t *) ckd_calloc(1, sizeof(lm_t))) == NULL ) {
        E_ERROR("Error in allocating memory for LM\n");
        return NULL;
    }

    lm_null_struct(lm);

    lm->isLM_IN_MEMORY = lminmemory;
    lm->max_ng = 1;


    if ((lm->fp = fopen(file, "rb")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file);

    /* Read header and compare byte order */
    if (lm_read_dump_header(lm, file, &max_ng) == LM_FAIL) {
        E_ERROR("Error in reading the header of the DUMP file\n");
        lm_free(lm);
        return NULL;
    }
    
    /* Read the full path of file name of lm */
    if (lm_read_lmfilename(lm, file) == LM_FAIL) {
        E_ERROR("Error in reading the file name of LM\n");
        lm_free(lm);
        return NULL;
    }

    /* Read the version number and number of unigram */
    if (lm_read_dump_ver_nug(lm, file, max_ng, &version, &n_ug) == LM_FAIL) {
        E_ERROR("Error in reading the version name and number of unigram\n");
        lm_free(lm);
        return NULL;
    }

    /* Allocate memory for LM with max Ng level value */
    if (lm_init_n_ng (lm, max_ng, n_ug, version, lminmemory) == LM_FAIL) {
        E_ERROR("Error in allocating memory for LM with max Ng level\n");
        lm_free(lm);
        return NULL;
    }


    /* Reading the count of N-grams */

    if (lm_read_dump_ng_counts(lm, file, max_ng) == LM_FAIL) {
        E_ERROR("Error in reading the N-gram counts\n");
        lm_free(lm);
        return NULL;
    }

#ifndef _READ_DUMP_ONLY_
    lm->HT = hash_table_new(n_ug, HASH_CASE_YES);
#endif


    /* Reading the N-grams, the meat of the code. Also decide how
       different versions of LM are read in. */

    if (lm_read_dump_all_ng(lm, file) == LM_FAIL) {
        E_ERROR("Error in reading the N-gram\n");
        lm_free(lm);
        return NULL;
    }


    return lm;
}


#ifndef _READ_DUMP_ONLY_

/**
  \brief Write header in DMP file
*/
void
lmng_dump_write_header  (FILE * fp,     /**< In: file stream */
                         uint32 max_N   /**< In: maximum N-gram level */
                         )
{
    char str[26];
    int32 k;

    if (max_N <= 3)
        sprintf (str, darpa_tg_hdr);
    else if (max_N == 4)
        sprintf (str, darpa_qg_hdr);
    else /* max_N >= 5 */
        sprintf (str, darpa_ng_hdr, max_N);
    k = strlen(str) + 1;

    fwrite_int32(fp, k);
    fwrite(str, sizeof(char), k, fp);
}

/**
  \brief Write file name in DMP file
*/
void
lmng_dump_write_lm_filename (FILE       * fp,       /**< In: file stream */
                             const char * lmfile    /**< In: file name */
                             )
{
    int32 k;

    k = strlen(lmfile) + 1;
    fwrite_int32(fp, k);
    fwrite(lmfile, sizeof(char), k, fp);

}

/**
  \brief Write segment sizes, version number and modification date in DMP file
*/
void
lmng_dump_write_version (FILE * fp,         /**< In: file stream */
                         lm_t * model,      /**< In: language model */
                         int32  mtime,      /**< In: modification date */
                         int32  is32bits    /**< In: is LM in 32-bits or not */
                         )
{
    uint32 i;

    if (!is32bits) {
        for (i = 2; (model->log_ng_seg_sz[i-1] == LOG2_NG_SEG_SZ)
                    && ((i < model->max_ng) || (i < 3)); i++);
        if ((i < model->max_ng) || (i < 3)) {
            E_WARN("a log_ng_seg_sz is different from default");
            fwrite_int32(fp, LMDMP_VERSION_16BIT_V2);   /* version number */
        }
        else {
            fwrite_int32(fp, LMDMP_VERSION_16BIT);   /* version number */
        }
    }
    else
        fwrite_int32(fp, LMDMP_VERSION_32BIT);       /* version number */

    fwrite_int32(fp, mtime);
}

/**
  \brief Write N-gram counts in DMP file
*/
void
lmng_dump_write_ngram_counts    (FILE * fp,     /**< In: file stream */
                                 lm_t * model   /**< In: language model */
                                 )
{
    uint32 i;
    
    for (i = 1; (i <= model->max_ng) || (i <= 3); i++)
        fwrite_int32(fp, model->n_ng[i-1]);
}

/**
  \brief Write format description in DMP file
*/
void
lmng_dump_write_fmtdesc (FILE * fp,     /**< In: file stream */
                         uint32 max_N   /**< In: maximum N-gram level */
                         )
{
    char str[191];
    uint32 i, k = 0;
    long posd, posf;
    
    /* write file format for trigrams or more */
    if (max_N < 3)
        max_N = 3;

    
    /* Write file format description into header */

    for (i = 0; fmtdesc[i] != NULL; i++) {
        k = strlen(fmtdesc[i]) + 1;
        fwrite_int32(fp, k);
        fwrite(fmtdesc[i], sizeof(char), k, fp);
    }

    for (i = 3; i <= max_N; i++) {
        sprintf (str, "(int32) lm_t.%dcount", i);
        k = strlen(str) + 1;
        fwrite_int32(fp, k);
        fwrite(str, sizeof(char), k, fp);
    }

    for (i = 1; i <= max_N; i++) {
        posd = ftell(fp);
        fwrite_int32(fp, k);
        /* the right value of the string length will be written later */

        if (i == 1) {
            sprintf (str, "lm_t.1count+1 1-grams (");
            k = strlen(str);
            fwrite(str, sizeof(char), k, fp);
        }
        if (i == 2) {
            sprintf (str, "lm_t.2count+1 2-grams (");
            k = strlen(str);
            fwrite(str, sizeof(char), k, fp);
        }
        if ( (i >= 3) && (i < max_N) ) {
            sprintf(str,"lm_t.%dcount+1 %d-grams (present if lm_t.%dcount > 0 ",
                    i, i, i);
            k = strlen(str);
            fwrite(str, sizeof(char), k, fp);
        }

        if (i >= max_N) {
            sprintf (str,
             "lm_t.%dcount %d-grams (present if lm_t.%dcount > 0 32 bits (SIZE_OF_NG16_WITHOUT_NNG) each if version=-1/-2, 64 bits (SIZE_OF_NG32_WITHOUT_NNG) each if version=-3)"
                     , i, i, i);
            k = strlen(str);
            fwrite(str, sizeof(char), k+1, fp);
        }
        else {/* i < max_N */
            sprintf (str, "including sentinel");
            k += strlen(str);
            fwrite(str, sizeof(char), strlen(str), fp);
        }

        if (i == 1) {
            k += 1;
            fwrite(")", sizeof(char), 2, fp);
        }
        if ( (i >= 2) && (i < max_N) ) {
            sprintf (str,
             " 64 bits (ng_t) each if version=-1/-2, 128 bits (ng32_t) each if version=-3)"
                     );
            k += strlen(str);
            fwrite(str, sizeof(char), strlen(str)+1, fp);
        }

        /* write the good string length */
        posf = ftell(fp);
        if (fseek(fp, posd, SEEK_SET) < 0)
            E_FATAL_SYSTEM("fseek failed\n");
        fwrite_int32(fp, k+1);
        if (fseek(fp, posf, SEEK_SET) < 0)
            E_FATAL_SYSTEM("fseek failed\n");
    }

    sprintf (str, "(int32) lm_t.n_prob2");
    k = strlen(str) + 1;
    fwrite_int32(fp, k);
    fwrite(str, sizeof(char), k, fp);

    sprintf (str, "(int32) lm_t.prob2[]");
    k = strlen(str) + 1;
    fwrite_int32(fp, k);
    fwrite(str, sizeof(char), k, fp);

    for (i = 2; i <= max_N; i++) {
        if (i >= 3) {
            sprintf (str, "(int32) lm_t.n_prob%d (present if lm_t.%dcount > 0)",
                     i, i);
            k = strlen(str) + 1;
            fwrite_int32(fp, k);
            fwrite(str, sizeof(char), k, fp);

            sprintf (str, "(int32) lm_t.prob%d[] (present if lm_t.%dcount > 0)",
                     i, i);
            k = strlen(str) + 1;
            fwrite_int32(fp, k);
            fwrite(str, sizeof(char), k, fp);

            sprintf (str,
             "(int32) (lm_t.%dcount+1)/NG_SEG_SZ+1 (present if lm_t.%dcount > 0)"
                     , i-1, i);
            k = strlen(str) + 1;
            fwrite_int32(fp, k);
            fwrite(str, sizeof(char), k, fp);

            sprintf (str,
                     "(int32) lm_t.%dseg_base[] (present if lm_t.%dcount > 0)",
                     i, i);
            k = strlen(str) + 1;
            fwrite_int32(fp, k);
            fwrite(str, sizeof(char), k, fp);
        }

        if (i < max_N) {
            sprintf (str,"(int32) lm_t.n_bo_wt%d (present if lm_t.%dcount > 0)",
                     i, i+1);
            k = strlen(str) + 1;
            fwrite_int32(fp, k);
            fwrite(str, sizeof(char), k, fp);

            sprintf (str,"(int32) lm_t.bo_wt%d[] (present if lm_t.%dcount > 0)",
                     i, i+1);
            k = strlen(str) + 1;
            fwrite_int32(fp, k);
            fwrite(str, sizeof(char), k, fp);
        }
    }

    sprintf (str,
           "(int32) Sum(all word string-lengths, including trailing 0 for each)"
             );
    k = strlen(str) + 1;
    fwrite_int32(fp, k);
    fwrite(str, sizeof(char), k, fp);

    sprintf (str, "All word strings (including trailing 0 for each)");
    k = strlen(str) + 1;
    fwrite_int32(fp, k);
    fwrite(str, sizeof(char), k, fp);

    sprintf (str, "END FILE FORMAT DESCRIPTION");
    k = strlen(str) + 1;
    fwrite_int32(fp, k);
    fwrite(str, sizeof(char), k, fp);


    /* Pad it out in order to achieve 32-bit alignment */
    posf = ftell(fp);
    k = posf & 3;
    if (k) {
        fwrite_int32(fp, 4-k);
        fwrite("!!!!", 1, 4-k, fp);
    }
    fwrite_int32(fp, 0);
}

/**
  \brief Write a N-gram level in DMP file
*/
void
lmng_dump_write_ngram   (FILE * fp,         /**< In: file stream */
                         lm_t * model,      /**< In: language model */
                         uint32 N,          /**< In: N-gram level */
                         int32  is32bits    /**< In: is LM in 32-bits or not */
                         )
{
    int32 i;

    if (N >= 2) {
        /* write bigrams or more */
        if ((N < model->max_ng) || (N < 3)) {
            for (i = 0; i <= model->n_ng[N-1]; i++) {
                if (is32bits)
                    fwrite_ng32(fp, &(model->ng32[N-1][i]));
                else
                    fwrite_ng(fp, &(model->ng[N-1][i]));
            }
        }
        else {
            /* max N-gram level, no sentinel */
            for (i = 0; i < model->n_ng[N-1]; i++) {
                if (is32bits)
                    fwrite_lg32(fp, &(model->ng32[N-1][i]));
                else
                    fwrite_lg(fp, &(model->ng[N-1][i]));
            }
        }
    }
    else /* N == 1 */
        /* write unigrams */
        for (i = 0; i <= model->n_ng[0]; i++)
            fwrite_ug(fp, &(model->ug[i]));
}

/**
  \brief Write probabilities in DMP file
*/
void
lmng_dump_write_ngprob  (FILE * fp,     /**< In: file stream */
                         lm_t * model,  /**< In: language model */
                         uint32 N       /**< In: N-gram level (N>=2) */
                         )
{
    int32 i;

    if (N < 2)
        return;

    fwrite_int32(fp, model->n_ngprob[N-1]);
    for (i = 0; i < model->n_ngprob[N-1]; i++)
        fwrite_int32(fp, model->ngprob[N-1][i].l);
}

/**
  \brief Write back-off weights in DMP file
*/
void
lmng_dump_write_ngbowt  (FILE * fp,     /**< In: file stream */
                         lm_t * model,  /**< In: language model */
                         uint32 N       /**< In: N-gram level (N>=3) */
                         )
{
    int32 i;

    if (N < 3)
        return;

    fwrite_int32(fp, model->n_ngbowt[N-1]);
    for (i = 0; i < model->n_ngbowt[N-1]; i++)
        fwrite_int32(fp, model->ngbowt[N-1][i].l);
}

/**
  \brief Write segments base in DMP file
*/
void
lmng_dump_write_ng_segbase  (FILE * fp,     /**< In: file stream */
                             lm_t * model,  /**< In: language model */
                             uint32 N       /**< In: N-gram level (N>=3) */
                             )
{
    int32 i, k;

    if (N < 3)
        return;

    k = (model->n_ng[N-2] + 1) / NG_SEG_SZ + 1;
    fwrite_int32(fp, k);
    for (i = 0; i < k; i++)
        fwrite_int32(fp, model->ng_segbase[N-1][i]);
}

/**
  \brief Write word strings in DMP file
*/
void
lmng_dump_write_wordstr (FILE * fp,     /**< In: file stream */
                         lm_t * model   /**< In: language model */
                         )
{
    int32 i, k;
    k = 0;
    for (i = 0; i < model->n_ng[0]; i++)
        k += strlen(model->wordstr[i]) + 1;
    fwrite_int32(fp, k);
    for (i = 0; i < model->n_ng[0]; i++)
        fwrite(model->wordstr[i], sizeof(char),
               strlen(model->wordstr[i]) + 1, fp);
}

int32
lmng_dump   (char const * file,     /**< In: the path name */
             lm_t       * model,    /**< In: language model for output */
             char const * lmfile,   /**< In: the file name */
             int32        mtime,    /**< In: LM file modification date */
             int32        noBits    /**< In: number of bits of DMP format */
             )
{
    FILE *fp;
    uint32 i, is32bits;

    if (noBits != 16 && noBits != 32) {
        E_ERROR("No of Bits specified is not 16 or 32\n");
        return LM_FAIL;
    }

    is32bits = (noBits == 32);

    if (!is32bits && model->n_ng[0] > LM_LEGACY_CONSTANT) {
        E_ERROR (
             "Number of words is larger than %d, but 16 bits models were used\n"
                 , LM_LEGACY_CONSTANT);
        return LM_FAIL;
    }
    /* 
     * If is32bits, 
     */

    E_INFO("Dumping LM to %s\n", file);
    if ((fp = fopen(file, "wb")) == NULL) {
        E_ERROR("Cannot create file %s\n", file);
        return LM_FAIL;
    }

    lmng_dump_write_header(fp, model->max_ng);
    lmng_dump_write_lm_filename(fp, lmfile);
    lmng_dump_write_version(fp, model, mtime, is32bits);

    /* Write version number and LM file modification date */
    lmng_dump_write_fmtdesc(fp, model->max_ng);

    /* HACK!! Write only if different from previous version */
    for (i = 2; (model->log_ng_seg_sz[i-1] == LOG2_NG_SEG_SZ)
                && ((i < model->max_ng) || (i < 3)); i++);
    if ((i < model->max_ng) || (i < 3))
        /* a log_ng_seg_sz is different from default */
        for (i = 2; i < model->max_ng; i++)
            fwrite_int32(fp, model->log_ng_seg_sz[i-1]);

    lmng_dump_write_ngram_counts(fp, model);

    if (!is32bits && model->n_ng[0] > LM_LEGACY_CONSTANT) {
        E_ERROR (
         "The model is a 16 bits' one but the number of unigrams has more than 65535 words (> 16 bits)"
                 );
        return LM_FAIL;
    }

    /*
       20060302 ARCHAN 
       This part is where the 16/32 bits differ
    */

    lm_convert_structure(model, is32bits);
    
    for (i = 1; i <= model->max_ng; i++)
        lmng_dump_write_ngram(fp, model, i, is32bits);

    /**************************************/

    lmng_dump_write_ngprob(fp, model, 2);

    for (i = 3; i <= model->max_ng; i++)
        if (model->n_ng[i-1] > 0) {
            lmng_dump_write_ngbowt(fp, model, i);
            lmng_dump_write_ngprob(fp, model, i);
            lmng_dump_write_ng_segbase(fp, model, i);
        }

    lmng_dump_write_wordstr(fp, model);

    fclose(fp);
    return LM_SUCCESS;
}

#endif
