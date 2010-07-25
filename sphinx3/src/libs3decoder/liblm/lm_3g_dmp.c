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

/*
 * lm_3g_dmp.c -- DMP format LM manipulation.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: lm_3g_dmp.c,v $
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
 * Merged the logic in share/lm3g2dmp to here.  It will take care the situation when log_bg_seg_sz is different. (Must be an old format Ravi played with in the past). This will match the reading code also generalize the old sphinx 2's logic a little bit.
 *
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert lm to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.1  2005/07/17 05:23:25  arthchan2003
 * added lm_3g_dmp.c and lmset.c, split it out from lm.c to avoid overcrowding situation in it.
 *
 *
 *
 */

#include <string.h>

#include <sphinxbase/bio.h>

#include <lm.h>
#include <s3types.h>

/**< ARCHAN 20060302:
   
Please do not change it.  Legacy code use this string to match
   the header of the LM DMP model.  If we change it, lm3g_read_dump
   won't work. 
*/
const char *darpa_hdr = "Darpa Trigram LM";


#define IS32BITS 1
#define IS16BITS 0

static void
fwrite_int32(FILE * fp, int32 val)
{
    REVERSE_SENSE_SWAP_INT32(val);
    fwrite(&val, sizeof(int32), 1, fp);
}

static void
fwrite_ug(FILE * fp, ug_t * ug)
{
    ug_t tmp_ug = *ug;

    REVERSE_SENSE_SWAP_INT32(tmp_ug.dictwid);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.prob.l);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.bowt.l);
    REVERSE_SENSE_SWAP_INT32(tmp_ug.firstbg);
    fwrite(&tmp_ug, sizeof(ug_t), 1, fp);
}

static void
fwrite_bg(FILE * fp, bg_t * bg)
{
    bg_t tmp_bg = *bg;

    REVERSE_SENSE_SWAP_INT16(tmp_bg.wid);
    REVERSE_SENSE_SWAP_INT16(tmp_bg.probid);
    REVERSE_SENSE_SWAP_INT16(tmp_bg.bowtid);
    REVERSE_SENSE_SWAP_INT16(tmp_bg.firsttg);
    fwrite(&tmp_bg, sizeof(bg_t), 1, fp);
}

static void
fwrite_bg32(FILE * fp, bg32_t * bg)
{
    bg32_t tmp_bg = *bg;

    REVERSE_SENSE_SWAP_INT32(tmp_bg.wid);
    REVERSE_SENSE_SWAP_INT32(tmp_bg.probid);
    REVERSE_SENSE_SWAP_INT32(tmp_bg.bowtid);
    REVERSE_SENSE_SWAP_INT32(tmp_bg.firsttg);
    fwrite(&tmp_bg, sizeof(bg32_t), 1, fp);
}

static void
fwrite_tg(FILE * fp, tg_t * tg)
{
    tg_t tmp_tg = *tg;

    REVERSE_SENSE_SWAP_INT16(tmp_tg.wid);
    REVERSE_SENSE_SWAP_INT16(tmp_tg.probid);
    fwrite(&tmp_tg, sizeof(tg_t), 1, fp);
}

static void
fwrite_tg32(FILE * fp, tg32_t * tg)
{
    tg32_t tmp_tg = *tg;

    REVERSE_SENSE_SWAP_INT32(tmp_tg.wid);
    REVERSE_SENSE_SWAP_INT32(tmp_tg.probid);
    fwrite(&tmp_tg, sizeof(tg32_t), 1, fp);
}


/** Please look at the definition of 
 */
static char const *fmtdesc[] = {
    "BEGIN FILE FORMAT DESCRIPTION",
    "Header string length (int32) and string (including trailing 0)",
    "Original LM filename string-length (int32) and filename (including trailing 0)",
    "(int32) version number (present iff value <= 0)",
    "(int32) original LM file modification timestamp (iff version# present)",
    "(int32) string-length and string (including trailing 0) (iff version# present)",
    "... previous entry continued any number of times (iff version# present)",
    "(int32) 0 (terminating sequence of strings) (iff version# present)",
    "(int32) log_bg_seg_sz (present iff different from default value of LOG2_BG_SEG_SZ)",
    "(int32) lm_t.ucount (must be > 0)",
    "(int32) lm_t.bcount",
    "(int32) lm_t.tcount",
    "lm_t.ucount+1 unigrams (including sentinel)",
    "lm_t.bcount+1 bigrams (including sentinel 64 bits (bg_t) each if version=-1/-2, 128 bits (bg32_t) each if version=-3",
    "lm_t.tcount trigrams (present iff lm_t.tcount > 0 32 bits (tg_t) each if version=-1/-2, 64 bits (tg32_t) each if version=-3)",
    "(int32) lm_t.n_prob2",
    "(int32) lm_t.prob2[]",
    "(int32) lm_t.n_bo_wt2 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.bo_wt2[] (present iff lm_t.tcount > 0)",
    "(int32) lm_t.n_prob3 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.prob3[] (present iff lm_t.tcount > 0)",
    "(int32) (lm_t.bcount+1)/BG_SEG_SZ+1 (present iff lm_t.tcount > 0)",
    "(int32) lm_t.tseg_base[] (present iff lm_t.tcount > 0)",
    "(int32) Sum(all word string-lengths, including trailing 0 for each)",
    "All word strings (including trailing 0 for each)",
    "END FILE FORMAT DESCRIPTION",
    NULL,
};

void
lm3g_dump_write_header(FILE * fp)
{
    int32 k;
    k = strlen(darpa_hdr) + 1;
    fwrite_int32(fp, k);
    fwrite(darpa_hdr, sizeof(char), k, fp);
}

void
lm3g_dump_write_lm_filename(FILE * fp, const char *lmfile)
{
    int32 k;

    k = strlen(lmfile) + 1;
    fwrite_int32(fp, k);
    fwrite(lmfile, sizeof(char), k, fp);

}

void
lm3g_dump_write_version(FILE * fp, lm_t * model, int32 mtime,
                        int32 is32bits)
{
    if (!is32bits) {
        if (model->log_bg_seg_sz != LOG2_BG_SEG_SZ) {   /* Hack!! */
            E_WARN("log_bg_seg_sz is different from default");
            fwrite_int32(fp, LMDMP_VERSION_TG_16BIT_V2);        /* version # */
        }
        else {
            fwrite_int32(fp, LMDMP_VERSION_TG_16BIT);   /* version # */
        }
    }
    else
        fwrite_int32(fp, LMDMP_VERSION_TG_32BIT);       /* version # */

    fwrite_int32(fp, mtime);
}

void
lm3g_dump_write_ngram_counts(FILE * fp, lm_t * model)
{
    fwrite_int32(fp, model->n_ug);
    fwrite_int32(fp, model->n_bg);
    fwrite_int32(fp, model->n_tg);
}

void
lm3g_dump_write_fmtdesc(FILE * fp)
{
    int32 i, k;
    long pos;

    /* Write file format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
        k = strlen(fmtdesc[i]) + 1;
        fwrite_int32(fp, k);
        fwrite(fmtdesc[i], sizeof(char), k, fp);
    }
    /* Pad it out in order to achieve 32-bit alignment */
    pos = ftell(fp);
    k = pos & 3;
    if (k) {
        fwrite_int32(fp, 4-k);
        fwrite("!!!!", 1, 4-k, fp);
    }
    fwrite_int32(fp, 0);
}

void
lm3g_dump_write_unigram(FILE * fp, lm_t * model)
{
    int32 i;
    for (i = 0; i <= model->n_ug; i++)
        fwrite_ug(fp, &(model->ug[i]));

}


void
lm3g_dump_write_bigram(FILE * fp, lm_t * model, int32 is32bits)
{
    int32 i;
    for (i = 0; i <= model->n_bg; i++) {
        if (is32bits)
            fwrite_bg32(fp, &(model->bg32[i]));
        else
            fwrite_bg(fp, &(model->bg[i]));
    }

}

void
lm3g_dump_write_trigram(FILE * fp, lm_t * model, int32 is32bits)
{
    int32 i;
    for (i = 0; i < model->n_tg; i++) {
        if (is32bits)
            fwrite_tg32(fp, &(model->tg32[i]));
        else
            fwrite_tg(fp, &(model->tg[i]));
    }
}

void
lm3g_dump_write_bgprob(FILE * fp, lm_t * model)
{
    int32 i;
    fwrite_int32(fp, model->n_bgprob);
    for (i = 0; i < model->n_bgprob; i++)
        fwrite_int32(fp, model->bgprob[i].l);
}

void
lm3g_dump_write_tgbowt(FILE * fp, lm_t * model)
{
    int32 i;
    fwrite_int32(fp, model->n_tgbowt);
    for (i = 0; i < model->n_tgbowt; i++)
        fwrite_int32(fp, model->tgbowt[i].l);
}

void
lm3g_dump_write_tgprob(FILE * fp, lm_t * model)
{
    int32 i;
    fwrite_int32(fp, model->n_tgprob);
    for (i = 0; i < model->n_tgprob; i++)
        fwrite_int32(fp, model->tgprob[i].l);
}

void
lm3g_dump_write_tg_segbase(FILE * fp, lm_t * model)
{
    int32 i, k;
    k = (model->n_bg + 1) / BG_SEG_SZ + 1;
    fwrite_int32(fp, k);
    for (i = 0; i < k; i++)
        fwrite_int32(fp, model->tg_segbase[i]);
}

void
lm3g_dump_write_wordstr(FILE * fp, lm_t * model)
{
    int32 i, k;
    k = 0;
    for (i = 0; i < model->n_ug; i++)
        k += strlen(model->wordstr[i]) + 1;
    fwrite_int32(fp, k);
    for (i = 0; i < model->n_ug; i++)
        fwrite(model->wordstr[i], sizeof(char),
               strlen(model->wordstr[i]) + 1, fp);
}

int32
lm3g_dump(char const *file,         /**< the file name */
          lm_t * model,             /**< the langauge model for output */
          char const *lmfile,         /**< the */
          int32 mtime,         /**< LM file modification date */
          int32 noBits         /**< Number of bits of DMP format */
    )
{
    FILE *fp;
    int32 is32bits;

    if (noBits != 16 && noBits != 32) {
        E_ERROR("No of Bits specified is not 16 or 32\n");
        return LM_FAIL;
    }

    is32bits = (noBits == 32);

    if (!is32bits && model->n_ug > LM_LEGACY_CONSTANT) {
        E_ERROR
            ("Number of words is larger than %d, but 16 bits models were used\n",
             LM_LEGACY_CONSTANT);
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

    lm3g_dump_write_header(fp);
    lm3g_dump_write_lm_filename(fp, lmfile);
    lm3g_dump_write_version(fp, model, mtime, is32bits);

    /* Write version# and LM file modification date */
    lm3g_dump_write_fmtdesc(fp);

    /* HACK!! Write only if different from previous version */
    if (model->log_bg_seg_sz != LOG2_BG_SEG_SZ)
        fwrite_int32(fp, model->log_bg_seg_sz);

    lm3g_dump_write_ngram_counts(fp, model);

    if (!is32bits && model->n_ug > LM_LEGACY_CONSTANT) {
        E_ERROR
            ("This is a 16-bit language model, but there are more than 65535 unigrams in it.");
        return LM_FAIL;
    }

    lm3g_dump_write_unigram(fp, model);

    /**
       20060302 ARCHAN 
       This part is where the 16/32 bits differ
     */

    lm_convert_structure(model, is32bits);
    lm3g_dump_write_bigram(fp, model, is32bits);
    lm3g_dump_write_trigram(fp, model, is32bits);

    /**************************************/

    lm3g_dump_write_bgprob(fp, model);

    if (model->n_tg > 0) {
        lm3g_dump_write_tgbowt(fp, model);
        lm3g_dump_write_tgprob(fp, model);
        lm3g_dump_write_tg_segbase(fp, model);
    }

    lm3g_dump_write_wordstr(fp, model);

    fclose(fp);
    return LM_SUCCESS;
}


static int32
lm_fread_int32(lm_t * lm)
{
    int32 val;

    if (fread(&val, sizeof(int32), 1, lm->fp) != 1)
        E_FATAL("fread failed\n");
    if (lm->byteswap)
        SWAP_INT32(&val);
    return (val);
}


/**   
      20060303: ARCHAN

      lm_read_dump_header will read in the DMP format. What it will do
      is to compare the value read in with the darpa_hdr ("Darpa
      Trigram LM").  If it matches, that means there is no byte
      swap. If it doesn't, we will try to swap the value and match the
      header again.  If it still doesn't work, that means something is
      wrong. (e.g. Format problem of the DMP file).  

      This process will also allow us to know the byte-order of the
      DMP file. Swapping could then automatically done in the code. 
 */
static int32
lm_read_dump_header(lm_t * lm,             /**< The LM */
                    const char *file              /**< The file we are reading */
    )
{
    int32 k;
    char str[1024];

    /* Standard header string-size; set byteswap flag based on this */
    if (fread(&k, sizeof(int32), 1, lm->fp) != 1)
        E_FATAL("fread(%s) failed\n", file);

    if ((size_t) k == strlen(darpa_hdr) + 1)
        lm->byteswap = 0;
    else {
        SWAP_INT32(&k);
        if ((size_t) k == strlen(darpa_hdr) + 1)
            lm->byteswap = 1;
        else {
            SWAP_INT32(&k);
            E_INFO("Bad magic number: %d(%08x), not an LM dumpfile??\n", k,
                   k);
            return LM_FAIL;
        }
    }

    /* Read and verify standard header string */
    if (fread(str, sizeof(char), k, lm->fp) != (size_t) k) {
        E_ERROR("fread(%s) failed\n", file);
        return LM_FAIL;
    }
    if (strncmp(str, darpa_hdr, k) != 0) {
        E_ERROR("Bad header\n");
        return LM_FAIL;
    }

    return LM_SUCCESS;

}

static int32
lm_read_lmfilename(lm_t * lm,             /**< The LM */
                   const char *file              /**< The file we are reading */
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
   20060303 ARCHAN:

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
 */

static int32
lm_read_dump_ver_nug(lm_t * lm,             /**< The LM*/
                     const char *file              /**< The file we are reading */
    )
{
    int32 k;
    char str[1024];

    /* Version#.  If present (must be <= 0); otherwise it's actually the unigram count */
    lm->version = lm_fread_int32(lm);

    if (lm->version <= 0) {
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

        /* Read log_bg_seg_sz if present */

        /* ARCHAN 20060304
           use lm->version == -2 (LMDMP_VERSION_TG_16BIT_V2) instead of lm->version <2,
           This is different from share's version
         */
        if (lm->version == LMDMP_VERSION_TG_16BIT_V2) {
            k = lm_fread_int32(lm);
            if ((k < 1) || (k > 15)) {
                E_ERROR("log2(bg_seg_sz) %d outside range 1..15 \n", k);
                return LM_FAIL;
            }
            lm->log_bg_seg_sz = k;
        }
        else {
            lm->log_bg_seg_sz = LOG2_BG_SEG_SZ; /* Default */
        }

        /* Read #ug */
        lm->n_ug = lm_fread_int32(lm);

    }
    else {
        /* oldest dump file version has no version# or any of the above */
        if (lm->version > lm->n_ug) {
            E_ERROR("LM.ucount(%d) out of range [1..%d]\n", lm->version,
                    lm->n_ug);
            return LM_FAIL;
        }

        /* No version number, actually a unigram count */
        lm->n_ug = lm->version;
        lm->log_bg_seg_sz = LOG2_BG_SEG_SZ;     /* Default */
    }


    lm->is32bits = lm_is32bits(lm);
    if ((lm->n_ug <= 0) || (lm->n_ug >= MAX_LMWID(lm))) {
        E_ERROR("Bad #ug: %u (must be >0, <%u) Version %d\n", lm->n_ug,
                MAX_LMWID(lm), lm->version);
        return LM_FAIL;
    }

    lm->bg_seg_sz = 1 << lm->log_bg_seg_sz;

    if (lm->version == LMDMP_VERSION_TG_32BIT) {
        E_INFO("Reading LM in 32 bits format\n");
    }
    else if (lm->version > LMDMP_VERSIONNULL ||
             lm->version == LMDMP_VERSION_TG_16BIT ||
             lm->version == LMDMP_VERSION_TG_16BIT_V2) {
        E_INFO("Reading LM in 16 bits format\n");
    }

    return LM_SUCCESS;
}

static int32
lm_read_dump_ng_counts(lm_t * lm, const char *file)
{
    /* #bigrams */
    lm->n_bg = lm_fread_int32(lm);
    if (lm->n_bg < 0) {
        E_ERROR("Bad #bigrams: %d\n", lm->n_bg);
        return LM_FAIL;
    }

    /* #trigrams */
    lm->n_tg = lm_fread_int32(lm);
    if (lm->n_tg < 0) {
        E_ERROR("Bad #trigrams: %d\n", lm->n_tg);
        return LM_FAIL;
    }

    if (lm->n_bg > 0)
        lm->n_ng = 2;

    if (lm->n_tg > 0)
        lm->n_ng = 3;

    return LM_SUCCESS;
}


static int32
lm_read_dump_ug(lm_t * lm, const char *file)
{
    int32 i;

    assert(lm->n_ug > 0);

    /* Read ug; remember sentinel ug at the end! */
    lm->ug = (ug_t *) ckd_calloc(lm->n_ug + 1, sizeof(ug_t));
    if (fread(lm->ug, sizeof(ug_t), lm->n_ug + 1, lm->fp) !=
        (size_t) (lm->n_ug + 1)) {
        E_ERROR("unigram fread(%s) failed\n", file);
        return LM_FAIL;
        /*        E_FATAL("fread(%s) failed\n", file); */
    }

    if (lm->byteswap) {
        for (i = 0; i <= lm->n_ug; i++) {
            SWAP_INT32(&(lm->ug[i].prob.l));
            SWAP_INT32(&(lm->ug[i].bowt.l));
            SWAP_INT32(&(lm->ug[i].firstbg));
        }
    }
    E_INFO("Read %8d unigrams [in memory]\n", lm->n_ug);
    return LM_SUCCESS;
}



/**
   Reading bigram in the DMP format. 

   When lm->isLM_IN_MEMORY is turned on.  A memory space will be
   allocated based.  Recorded the offset of bigram. Then the lm will be
   read from the file in one piece (lm->n_bg+1 *sizeof(bg_t)

   When lm->isLM_IN_MEMORY is turned off, we will just skip
   (lm->n_bg+1 * sizeof(bg_t)) byte memory and recorded the offset of
   bigram. In this case, the program will be operated in disk mode.

   ARCHAN 20060304, First introduced 32 bits reading.  This is whether
   the code is 32bit or not, lm->bg32 or lm->bg (16bits) will be used.
 */
static int32
lm_read_dump_bg(lm_t * lm,             /**< LM */
                const char *file,              /**< file we are reading */
                int32 is32bits                 /**< Is it a 32 bits reading? */
    )
{
    int32 i;
    int32 mem_sz;
    void *lmptr;
    assert(lm->n_bg > 0);

    mem_sz = is32bits ? sizeof(bg32_t) : sizeof(bg_t);
    lmptr = NULL;

  /** Allocate memory */
    if (lm->isLM_IN_MEMORY) {   /* Remember the sentinel */
        if ((lmptr = ckd_calloc(lm->n_bg + 1, mem_sz)) == NULL) {
            E_ERROR
                ("Failed to allocate memory with size %d for bigram reading. Size of each bigram is %d bytes\n",
                 lm->n_bg + 1, mem_sz);
            return LM_FAIL;
        }
    }
    else {
        lmptr = NULL;
    }

    if (lm->n_bg > 0) {

        lm->bgoff = ftell(lm->fp);

        if (lm->isLM_IN_MEMORY) {
            if (is32bits) {
                lm->bg32 = (bg32_t *) lmptr;
                fread(lm->bg32, lm->n_bg + 1, mem_sz, lm->fp);
                if (lm->byteswap) {
                    for (i = 0; i <= lm->n_bg; i++)
                        swap_bg32(&(lm->bg32[i]));
                }
            }
            else {
                lm->bg = (bg_t *) lmptr;
                fread(lm->bg, lm->n_bg + 1, mem_sz, lm->fp);
                if (lm->byteswap) {
                    for (i = 0; i <= lm->n_bg; i++)
                        swap_bg(&(lm->bg[i]));
                }
            }

            E_INFO("Read %8d bigrams [in memory]\n", lm->n_bg);
        }
        else {
            fseek(lm->fp, (lm->n_bg + 1) * mem_sz, SEEK_CUR);
            E_INFO("%8d bigrams [on disk]\n", lm->n_bg);
        }

    }

    return LM_SUCCESS;
}

/*
  
  Similar to lm_read_dump_bg, note instead of lm->n_tg+1, we are
  working on lm->n_tg here. 
  @see lm_read_dump_bg
 */

static int32
lm_read_dump_tg(lm_t * lm,             /**< LM */
                const char *file,              /**< file we are reading */
                int is32bits               /**< Whether the data structure is 32 bits */
    )
{
    int32 i;
    int32 mem_sz;
    void *lmptr;
    /* Number of Trigrams might be zero 
     */


    assert(lm->n_tg >= 0);

    mem_sz = is32bits ? sizeof(tg32_t) : sizeof(tg_t);
    lmptr = NULL;

    if (lm->isLM_IN_MEMORY && lm->n_tg > 0) {
        if ((lmptr = ckd_calloc(lm->n_tg + 1, mem_sz)) == NULL) {
            E_ERROR
                ("Fail to allocate memory with size %d for trigram reading.  Each trigram with mem_sz\n",
                 lm->n_tg + 1, mem_sz);
            return LM_FAIL;
        }

    }
    else
        lmptr = NULL;

    if (lm->n_tg > 0) {         /* Read bigrams; remember sentinel at the end */

        lm->tgoff = ftell(lm->fp);

        if (lm->isLM_IN_MEMORY) {
            if (is32bits) {
                lm->tg32 = (tg32_t *) lmptr;
                fread(lm->tg32, lm->n_tg, mem_sz, lm->fp);
                if (lm->byteswap) {
                    for (i = 0; i <= lm->n_tg - 1; i++) {
                        swap_tg32(&(lm->tg32[i]));
                    }
                }
            }
            else {
                lm->tg = (tg_t *) lmptr;
                fread(lm->tg, lm->n_tg, mem_sz, lm->fp);
                if (lm->byteswap) {
                    for (i = 0; i <= lm->n_tg - 1; i++) {
                        swap_tg(&(lm->tg[i]));
                    }
                }
            }

            E_INFO("Read %8d trigrams [in memory]\n", lm->n_tg);
        }
        else {
            fseek(lm->fp, (lm->n_tg) * mem_sz, SEEK_CUR);
            E_INFO("%8d bigrams [on disk]\n", lm->n_tg);
        }
    }
    return LM_SUCCESS;
}

static int32
lm_read_dump_calloc_membg_tginfo(lm_t * lm, const char *file, int is32bits)
{
    void *lmptr, *lmptr2;
    int32 mem_sz, mem_sz2;

    lmptr = lmptr2 = NULL;
    mem_sz = is32bits ? sizeof(membg32_t) : sizeof(membg_t);
    mem_sz2 = is32bits ? sizeof(tginfo32_t *) : sizeof(tginfo_t *);

    if (lm->n_bg > 0) {
        if ((lmptr = ckd_calloc(lm->n_ug, mem_sz)) == NULL) {
            E_ERROR("Failed to allocate memory for membg.\n");
            return LM_FAIL;
        }
    }

    if (lm->n_tg > 0) {
        if ((lmptr2 = ckd_calloc(lm->n_ug, mem_sz2)) == NULL) {
            E_ERROR("Failed to allocate memory for tginfo.\n");
            return LM_FAIL;
        }
    }

    if (is32bits) {
        lm->membg32 = (membg32_t *) lmptr;
        lm->tginfo32 = (tginfo32_t **) lmptr2;
    }
    else {
        lm->membg = (membg_t *) lmptr;
        lm->tginfo = (tginfo_t **) lmptr2;
    }
    return LM_SUCCESS;

}

static int32
lm_read_dump_bgprob(lm_t * lm, const char *file, int32 is32bits)
{
    int32 i;
    uint32 upper_limit;

    upper_limit = is32bits ? LM_SPHINX_CONSTANT : LM_LEGACY_CONSTANT;
    /*  E_INFO("%d upper_limit\n",upper_limit); */
    if (lm->n_bg > 0) {
        /* Bigram probs table size */
        lm->n_bgprob = lm_fread_int32(lm);
        if ((lm->n_bgprob <= 0) || (lm->n_bgprob > upper_limit)) {
            E_ERROR("Bad bigram prob table size: %d\n", lm->n_bgprob);
            return LM_FAIL;
        }

        /* Allocate and read bigram probs table */
        lm->bgprob = (lmlog_t *) ckd_calloc(lm->n_bgprob, sizeof(lmlog_t));
        if (fread(lm->bgprob, sizeof(lmlog_t), lm->n_bgprob, lm->fp) !=
            (size_t) lm->n_bgprob) {
            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < lm->n_bgprob; i++)
                SWAP_INT32(&(lm->bgprob[i].l));
        }

        E_INFO("%8d bigram prob entries\n", lm->n_bgprob);
    }
    return LM_SUCCESS;

}

static int32
lm_read_dump_tgbowt(lm_t * lm, const char *file, int32 is32bits)
{
    int32 i;
    uint32 upper_limit;

    upper_limit = is32bits ? LM_SPHINX_CONSTANT : LM_LEGACY_CONSTANT;

    if (lm->n_tg > 0) {
        /* Trigram bowt table size */
        lm->n_tgbowt = lm_fread_int32(lm);
        if ((lm->n_tgbowt <= 0) || (lm->n_tgbowt > upper_limit)) {
            E_ERROR("Bad trigram bowt table size: %d\n", lm->n_tgbowt);
            return LM_FAIL;
        }

        /* Allocate and read trigram bowt table */
        lm->tgbowt = (lmlog_t *) ckd_calloc(lm->n_tgbowt, sizeof(lmlog_t));
        if (fread(lm->tgbowt, sizeof(lmlog_t), lm->n_tgbowt, lm->fp) !=
            (size_t) lm->n_tgbowt) {

            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < lm->n_tgbowt; i++)
                SWAP_INT32(&(lm->tgbowt[i].l));
        }
        E_INFO("%8d trigram bowt entries\n", lm->n_tgbowt);
    }
    return LM_SUCCESS;
}

static int32
lm_read_dump_tgprob(lm_t * lm, const char *file, int32 is32bits)
{
    int32 i;
    uint32 upper_limit;

    upper_limit = is32bits ? LM_SPHINX_CONSTANT : LM_LEGACY_CONSTANT;

    if (lm->n_tg > 0) {
        lm->n_tgprob = lm_fread_int32(lm);
        if ((lm->n_tgprob <= 0) || (lm->n_tgprob > upper_limit)) {
            E_ERROR("Bad trigram bowt table size: %d\n", lm->n_tgprob);
            return LM_FAIL;
        }

        /* Allocate and read trigram bowt table */
        lm->tgprob = (lmlog_t *) ckd_calloc(lm->n_tgprob, sizeof(lmlog_t));
        if (fread(lm->tgprob, sizeof(lmlog_t), lm->n_tgprob, lm->fp) !=
            (size_t) lm->n_tgprob) {
            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < lm->n_tgprob; i++)
                SWAP_INT32(&(lm->tgprob[i].l));
        }
        E_INFO("%8d trigram prob entries\n", lm->n_tgprob);
    }

    return LM_SUCCESS;
}

/*
  The only function which doesn't require switching in lm_read_dump
 */
static int32
lm_read_dump_tg_segbase(lm_t * lm, const char *file)
{
    int i, k;
    if (lm->n_tg > 0) {
        /* Trigram seg table size */
        k = lm_fread_int32(lm);
        if (k != (lm->n_bg + 1) / lm->bg_seg_sz + 1) {
            E_ERROR("Bad trigram seg table size: %d\n", k);
            return LM_FAIL;
        }

        /* Allocate and read trigram seg table */
        lm->tg_segbase = (int32 *) ckd_calloc(k, sizeof(int32));
        if (fread(lm->tg_segbase, sizeof(int32), k, lm->fp) != (size_t) k) {
            E_ERROR("fread(%s) failed\n", file);
            return LM_FAIL;
        }
        if (lm->byteswap) {
            for (i = 0; i < k; i++)
                SWAP_INT32(&(lm->tg_segbase[i]));
        }
        E_INFO("%8d trigram segtable entries (%d segsize)\n", k,
               lm->bg_seg_sz);
    }
    return LM_SUCCESS;
}

static int32
lm_read_dump_wordstr(lm_t * lm, const char *file, int32 is32bits)
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

    if (j != lm->n_ug) {
        E_ERROR("Bad #words: %d\n", j);
        return LM_FAIL;
    }


    startwid = endwid = (s3lmwid32_t) BAD_LMWID(lm);


    lm->wordstr = (char **) ckd_calloc(lm->n_ug, sizeof(char *));
    j = 0;
    for (i = 0; i < lm->n_ug; i++) {
        if (strcmp(tmp_word_str + j, S3_START_WORD) == 0)
            startwid = i;
        else if (strcmp(tmp_word_str + j, S3_FINISH_WORD) == 0)
            endwid = i;

        lm->wordstr[i] = (char *) ckd_salloc(tmp_word_str + j);

        hash_table_enter(lm->HT, lm->wordstr[i], (void *)(long)i);

        j += strlen(tmp_word_str + j) + 1;
    }
    free(tmp_word_str);
    E_INFO("%8d word strings\n", i);

    /* Force ugprob(<s>) = MIN_PROB_F */
    if (IS_LMWID(lm, startwid)) {
        lm->ug[startwid].prob.f = MIN_PROB_F;
        lm->startlwid = startwid;
    }

    /* Force bowt(</s>) = MIN_PROB_F */
    if (IS_LMWID(lm, endwid)) {
        lm->ug[endwid].bowt.f = MIN_PROB_F;
        lm->finishlwid = endwid;
    }
    else {
        E_WARN("No </s> in LM!\n");
    }

    return LM_SUCCESS;
}


/**
  The core of reading reading the data structure from the LM file.  It
  also depends the version to operate.  Here is a summary of what's
  going on in each version. 

  1, In version >0, version=-1(LMDMP_VERSION_TG_16BIT),
  -2(LMDMP_VERSION_TG_16BIT_V2),
  
  The code will read the file using the following sequence. 
  -read unigram (*_dump_ug)
  -read bigram  (*_dump_bg)
  -read trigram (*_dump_tg)
  -create mem bigram
  -create trigram info
  -read the actual bigram probability (*_dump_bgprob)
  -read the actual trigram backoff weight (*_dump_tgbowt)
  -read the actual trigram probability  (*_dump_tgprob)
  -read the actual trigram segment base.  (*_dump_tgsegbase)
  -read the word str into the code. 

  bigram, trigram, membg, tg_info are all in 16 bits.  unigram in
  Sphinx 2, Sphinx 3.x (x<4) legacy are already 32 bits. 

  bgprob, tgbowt, tgprob, tgsegbase are arrays, their size are all
  controlled by a number which in int32.  We are cool here. 

  2, In version = -3 (LMDMP_VERSION_TG_32BIT)

  The code will read the file using the following sequence. 

  -read unigram (*_dump_ug)
  -read bigram in 32 bits  (*_dump_bg)
  -read trigram in 32 bits (*_dump_tg)
  -create mem bigram in 32 bits
  -create trigram info in 32 bits. 
  -read the actual bigram probability (*_dump_bgprob)
  -read the actual trigram backoff weight (*_dump_tgbowt)
  -read the actual trigram probability  (*_dump_tgprob)
  -read the actual trigram segment base.  (*_dump_tgsegbase)
  -read the word str into the code. 

  At here, all data structure will use 32 bits data structures or
  address arrays as int32 arrays. However because legacy
  implementation check the size in bgprob, tgbow, tgprob. I conformed
  to this coding style.  So except, _dump_ug and _dump_tgsegbase. All
  the code are now having is32bits arguments.  But the major difference
  between the two readings are mainly on _dump_bg and _dump_tg


  On coding :

  Each LM DMP versions will just show out all the routines used.  We
  are aware that you could optimize it.  Please don't because it will
  kill readability in future.
  
  We also want to support LIUM's lm format and a general n-gram format
  in my mind. We will see. 

 */

static int32
lm_read_dump_ng(lm_t * lm, const char *file)
{

    if (lm->version == LMDMP_VERSION_TG_16BIT ||
        lm->version == LMDMP_VERSION_TG_16BIT_V2 ||
        lm->version >= LMDMP_VERSIONNULL) {

        if (lm_read_dump_ug(lm, file) == LM_FAIL) {
            E_ERROR("Error in reading unigram. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_bg(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading bigram. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tg(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading trigram. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_calloc_membg_tginfo(lm, file, IS16BITS) ==
            LM_FAIL) {
            E_ERROR
                ("Error in allocating memory bigram and trigram info. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_bgprob(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading bigram probability. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tgbowt(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading trigram back off weight. \n");

            return LM_FAIL;
        }

        if (lm_read_dump_tgprob(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading trigram probability. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tg_segbase(lm, file) == LM_FAIL) {
            E_ERROR("Error in reading trigram segment base. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_wordstr(lm, file, IS16BITS) == LM_FAIL) {
            E_ERROR("Error in reading the word str.  \n");
            return LM_FAIL;
        }
    }
    else if (lm->version == LMDMP_VERSION_TG_32BIT) {

        if (lm_read_dump_ug(lm, file) == LM_FAIL) {
            E_ERROR("Error in reading unigram. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_bg(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading bigram. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tg(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading trigram. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_calloc_membg_tginfo(lm, file, IS32BITS) ==
            LM_FAIL) {
            E_ERROR
                ("Error in allocating memory bigram and trigram info. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_bgprob(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading bigram probability. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tgbowt(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading trigram back off weight. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tgprob(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading trigram probability. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_tg_segbase(lm, file) == LM_FAIL) {
            E_ERROR("Error in reading trigram segment base. \n");
            return LM_FAIL;
        }

        if (lm_read_dump_wordstr(lm, file, IS32BITS) == LM_FAIL) {
            E_ERROR("Error in reading the word str.  \n");
            return LM_FAIL;
        }

    }
    else {
        E_ERROR("Error, Format %d is unknown\n", lm->version);
        return LM_FAIL;
    }

    return LM_SUCCESS;
}

/**
 * Read LM dump (<lmname>.DMP) file and make it the current LM.
 * Same interface as lm_read except that the filename refers to a .DMP file.
 */
lm_t *
lm_read_dump(const char *file,        /**< The file name*/
             int lminmemory,        /**< Whether using in memory LM */
             logmath_t *logmath,
             int expect_dmp     /**< Show error if header not found */
    )
{
    lm_t *lm;

    lm = (lm_t *) ckd_calloc(1, sizeof(lm_t));

    lm_null_struct(lm);

    lm->isLM_IN_MEMORY = lminmemory;
    lm->n_ng = 1;
    lm->logmath = logmath;


    if ((lm->fp = fopen(file, "rb")) == NULL)
        E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file);

    /** Read header and compare byte order */
    if (lm_read_dump_header(lm, file) == LM_FAIL) {
	if (expect_dmp)
            E_ERROR("Error in reading the header of the DUMP file. \n");
        fclose(lm->fp);
        ckd_free(lm);
        return NULL;
    }

    /** Read the full path of file name of lm */
    if (lm_read_lmfilename(lm, file) == LM_FAIL) {
        E_ERROR("Error in reading the file name of lm. \n");
        fclose(lm->fp);
        ckd_free(lm);
        return NULL;
    }

    /** Read the version number and number of unigram */
    if (lm_read_dump_ver_nug(lm, file) == LM_FAIL) {
        E_ERROR
            ("Error in reading the version name and number of unigram. \n");
        fclose(lm->fp);
        ckd_free(lm);
        return NULL;
    }

    /** Reading the count of ngrams. */

    if (lm_read_dump_ng_counts(lm, file) == LM_FAIL) {
        E_ERROR("Error in reading the ngram counts.  \n");
        fclose(lm->fp);
        ckd_free(lm);
        return NULL;
    }

    lm->HT = hash_table_new(lm->n_ug, HASH_CASE_YES);


    /** Reading the ngrams, the meat of the code. Also decide how
	different versions of LM are read in.
     */

    if (lm_read_dump_ng(lm, file) == LM_FAIL) {
        E_ERROR("Error in reading the ngram.  \n");
        fclose(lm->fp);
        hash_table_free(lm->HT);
        ckd_free(lm);
        return NULL;
    }


    return lm;
}
