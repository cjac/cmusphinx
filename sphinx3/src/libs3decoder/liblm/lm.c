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
 * lm.c -- Disk-based backoff word trigram LM module.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: lm.c,v $
 * Revision 1.20  2006/03/03 20:02:38  arthchan2003
 * Removed C++ styles comment. This will make options -ansi and -std=c89 happy
 *
 * Revision 1.19  2006/03/02 22:11:56  arthchan2003
 * Fixed dox-doc.
 *
 * Revision 1.18  2006/03/01 20:03:55  arthchan2003
 * Do encoding conversion when the encodings are different. This will avoid a lot of weird characters.
 *
 * Revision 1.17  2006/02/24 13:38:08  arthchan2003
 * Added lm_read, it is a simple version of lm_read_advance.
 *
 * Revision 1.16  2006/02/23 04:16:29  arthchan2003
 * Merged from SPHINX3_5_2_RCI_IRII_BRANCH:
 * Splited the original lm.c into five parts,
 * a, lm.c - a controller of other subroutines.
 * b, lm_3g.c - implement TXT-based lm operations
 * c, lm_3g_dmp.c - implement DMP-based lm operations
 * d, lm_attfsm.c - implement FSM-based lm operations
 * e, lmset.c - implement sets of lm.
 *
 *
 * Revision 1.14.4.9  2006/01/16 19:56:37  arthchan2003
 * 1, lm_rawscore doesn't need a language weight, 2, Support dumping the LM in FST format.  This code used Yannick Esteve's and LIUM code.
 *
 * Revision 1.14.4.8  2005/11/17 06:18:49  arthchan2003
 * Added a string encoding conversion routine in lm.c. Currently it only works for converting hex to its value.
 *
 * Revision 1.14.4.7  2005/10/17 04:49:13  arthchan2003
 * Free resource of lm_t and lmset_t correctly.
 *
 * Revision 1.14.4.6  2005/09/07 23:30:26  arthchan2003
 * Changed error message for LM dump.
 *
 * Revision 1.14.4.5  2005/08/02 21:10:18  arthchan2003
 * Added function declaration for lm_read_dump.
 *
 * Revision 1.14.4.4  2005/07/17 05:24:23  arthchan2003
 * (Incomplete) Added lm_arbitrary.[ch], an arbitrary n-gram data structure.  Far from completed. Don't expect too much.
 *
 * Revision 1.14.4.3  2005/07/13 01:44:17  arthchan2003
 * 1, Moved text formatted LM code into lm_3g.c, 2 Changed lm_read such that it will work with both TXT file format and DMP file format. 3,  Added function lm_write to handle lm writing.
 *
 * Revision 1.14.4.2  2005/07/05 21:31:25  arthchan2003
 * Merged from HEAD.
 *
 * Revision 1.15  2005/07/05 13:12:37  dhdfu
 * Add new arguments to logs3_init() in some tests, main_ep
 *
 * Revision 1.14.4.1  2005/07/03 22:58:56  arthchan2003
 * tginfo and membg 's memory were not deallocated at all. This change fixed it.
 *
 * Revision 1.14  2005/06/21 22:24:02  arthchan2003
 * Log. In this change, I introduced a new interface for lm ,which is
 * call lmset_t. lmset_t wraps up multiple lm, n_lm, n_alloclm into the
 * same structure and handle LM initialization (lm_init) switching,
 * (lmset_curlm_widx), delete LM (lmset_delete_lm).  The internal
 * structure is called lmarray and is an array of pointers of lm.  The
 * current lm is always maintained and pointed by a pointer called cur_lm
 * . This substantially clarify the structure of the code.  At this
 * check-in, not every core function of lmset is completed.
 * e.g. lmset_add_lm because that required testing of several LM reading
 * routines and could be quite time-consuming.
 *
 * Log. Another notable change is the fact dict2lmwid map is started to
 * be part of the LM. The reason of this is clearly described inside the
 * code. Don't want to repeat here.
 *
 * Log. The new interface has been already used broadly in both Sphinx
 * 3.0 and sphinx 3.x family of tools.
 *
 * Revision 1.4  2005/06/18 03:22:28  archan
 * Add lmset_init. A wrapper function of various LM initialization and initialize an lmset It is now used in decode, livepretend, dag and astar.
 *
 * Revision 1.3  2005/06/17 23:44:40  archan
 * Sphinx3 to s3.generic, 1, Support -lmname in decode and livepretend.  2, Wrap up the initialization of dict2lmwid to lm initialization. 3, add Dave's trick in LM switching in mode 4 of the search.
 *
 * Revision 1.2  2005/05/10 21:21:53  archan
 * Three functionalities added but not tested. Code on 1) addition/deletion of LM in mode 4. 2) reading text-based LM 3) Converting txt-based LM to dmp-based LM.
 *
 * Revision 1.1  2005/05/04 06:08:07  archan
 * Refactor all lm routines except fillpen.c into ./libs3decoder/liblm/ . This will be equivalent to ./lib/liblm in future.
 *
 * Revision 1.6  2005/05/04 04:02:24  archan
 * Implementation of lm addition, deletion in (mode 4) time-switching tree implementation of search.  Not yet tested. Just want to keep up my own momentum.
 *
 * Revision 1.5  2005/04/20 03:37:59  archan
 * LM code changes: functions are added to set, add and delete LM from the lmset, change the legacy lmset data structure to contain n_lm and n_alloc_lm.
 *
 * Revision 1.4  2005/03/30 16:28:34  archan
 * delete test-full.log alog
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Adding lm_free() to free allocated memory
 * 
 * 30-Dec-2000  Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Removed language weight application to wip. To maintain
 *		comparability between s3decode and current decoder. Does
 *		not affect decoding performance.
 *
 * 23-Feb-2000	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Bugfix: Applied language weight to word insertion penalty.
 * 
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.access_type; made lm_wid externally visible.
 * 
 * 24-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added lm_t.log_bg_seg_sz and lm_t.bg_seg_sz.
 * 
 * 13-Feb-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Creating from original S3 version.
 */


#include <string.h>

#include "lm.h"
#include "bio.h"
#include "logs3.h"
#include "wid.h"
#include "encoding.h"

/*ARCHAN, 20041112: NOP, NO STATIC VARIABLES! */

extern lm_t *lm_read_txt(const char *filename, /**< The file name */
                         const int lminmemory,  /**< Whether using in memory LM */
                         int *err_no, /**< Input/Output: Depends on the problem that LM
					reading encounters, it could be errors
					from -2 (LM_OFFSET_TOO_LARGE) to
					-15 (LM_CANNOT_ALLOCATE).  Please checkout
					lm.h for details. 
				     */
                         int32 isforced32bit, /** Input: normally, we should let lm_read_txt
						 to decide whether a file is 32 bit or not. 
						 When the lm_read_txt couldn't decide that before
						 reading or if more specificially when we hit
						 the LM segment size problems. Then this bit
						 will alter the reading behavior to 32 bit. 
					     */
                         logmath_t *logmath
    );

extern lm_t *lm_read_dump(const char *file,  /**< The file name*/
                          int lminmemory,  /**< Whether using in memory LM */
                          logmath_t *logmath,
	                  int expect_dmp     /**< Show error if header not found */
    );


int32 lm3g_dump(char const *file,   /**< the file name */
                lm_t * model,       /**< the langauge model for output */
                char const *lmfile,   /**< the lm file name */
                int32 mtime,   /**< LM file modification date */
                int32 noBits   /**< Number of bits of DMP format */
    );

/**
   Writer of lm in ARPA text format
 */
int32 lm_write_arpa_text(lm_t * lmp, /**< the pointer of the language model */
                         const char *outputfn, /**< the output file name */
                         const char *inputenc, /**< The input encoding method */
                         const char *outputenc /**< The output encoding method */
    );

/**
   Writer of lm in FST format
 */

int32 lm_write_att_fsm(lm_t * lm, /**< the languauge model pointer */
                       const char *filename/**< output file name */
    );


/**
   The function to return whether an LM should be 32bit or not.
   It is decided by whether we are using 32bit mode DMP.  Or whether
   it is LMTXT_VERSION but with more than 0xffff words.  The final
   criterion is when LMFORCE_TXT32VERSION. 
 */
int32
lm_is32bits(lm_t * lm)
{
    if (lm->version == LMDMP_VERSION_TG_32BIT)
        return 1;
    if (lm->version == LMFORCED_TXT32VERSION)
        return 1;
    if (lm->version == LMTXT_VERSION && lm->n_ug > LM_LEGACY_CONSTANT)
        return 1;
    if (lm->version == LMFST_VERSION && lm->n_ug > LM_LEGACY_CONSTANT)
        return 1;

    return 0;
}


int32
lm_get_classid(lm_t * model, const char *name)
{
    int32 i;

    if (!model->lmclass)
        return BAD_LMCLASSID;

    for (i = 0; i < model->n_lmclass; i++) {
        if (strcmp(lmclass_getname(model->lmclass[i]), name) == 0)
            return (i + LM_CLASSID_BASE);
    }
    return BAD_LMCLASSID;
}



void
lm_null_struct(lm_t * lm)
{
    lm->name = NULL;
    lm->wordstr = NULL;

    lm->ug = NULL;
    lm->bg = NULL;
    lm->tg = NULL;
    lm->membg = NULL;
    lm->tginfo = NULL;
    lm->tgcache = NULL;
    lm->dict2lmwid = NULL;

    lm->bg32 = NULL;
    lm->tg32 = NULL;
    lm->membg32 = NULL;
    lm->tginfo32 = NULL;
    lm->tgcache32 = NULL;

    lm->bgprob = NULL;
    lm->tgprob = NULL;
    lm->tgbowt = NULL;

    lm->tg_segbase = NULL;
    lm->lmclass = NULL;
    lm->inclass_ugscore = NULL;
    lm->logmath = NULL;
}

/* Apply unigram weight; should be part of LM creation, but... */
static void
lm_uw(lm_t * lm, float64 uw)
{
    int32 i, loguw, loguw_, loguniform, p1, p2;

    /* Interpolate unigram probs with uniform PDF, with weight uw */
    loguw = logs3(lm->logmath, uw);
    loguw_ = logs3(lm->logmath, 1.0 - uw);
    loguniform = logs3(lm->logmath, 1.0 / (lm->n_ug - 1));   /* Skipping S3_START_WORD */

    for (i = 0; i < lm->n_ug; i++) {
        if (strcmp(lm->wordstr[i], S3_START_WORD) != 0) {
            p1 = lm->ug[i].prob.l + loguw;
            p2 = loguniform + loguw_;
            lm->ug[i].prob.l = logmath_add(lm->logmath, p1, p2);
        }
    }
}


static void
lm2logs3(lm_t * lm, float64 uw)
{
    int32 i;

    for (i = 0; i < lm->n_ug; i++) {
        lm->ug[i].prob.l = logmath_log10_to_log(lm->logmath, lm->ug[i].prob.f);

        /* This prevent underflow if the backoff value is too small 
           It happens sometimes in cmu-lmtk V3's lm_combine. 
         */

        if (lm->ug[i].bowt.f < MIN_PROB_F)
            lm->ug[i].bowt.f = MIN_PROB_F;

        lm->ug[i].bowt.l = logmath_log10_to_log(lm->logmath, lm->ug[i].bowt.f);
    }

    lm_uw(lm, uw);

    for (i = 0; i < lm->n_bgprob; i++)
        lm->bgprob[i].l = logmath_log10_to_log(lm->logmath, lm->bgprob[i].f);

    if (lm->n_tg > 0) {
        for (i = 0; i < lm->n_tgprob; i++)
            lm->tgprob[i].l = logmath_log10_to_log(lm->logmath, lm->tgprob[i].f);
        for (i = 0; i < lm->n_tgbowt; i++) {

            if (lm->tgbowt[i].f < MIN_PROB_F)
                lm->tgbowt[i].f = MIN_PROB_F;

            lm->tgbowt[i].l = logmath_log10_to_log(lm->logmath, lm->tgbowt[i].f);
        }
    }
}


void
lm_set_param(lm_t * lm, float64 lw, float64 wip)
{
    int32 i, iwip;
    float64 f;

    if (lw <= 0.0)
        E_FATAL("lw = %e\n", lw);
    if (wip <= 0.0)
        E_FATAL("wip = %e\n", wip);
#if 0                           /* No lang weight on wip */
    iwip = logs3(lm->logmath, wip) * lw;
#endif
    iwip = logs3(lm->logmath, wip);

    f = lw / lm->lw;

    for (i = 0; i < lm->n_ug; i++) {
        lm->ug[i].prob.l =
            (int32) ((lm->ug[i].prob.l - lm->wip) * f) + iwip;
        lm->ug[i].bowt.l = (int32) (lm->ug[i].bowt.l * f);
    }

    for (i = 0; i < lm->n_bgprob; i++)
        lm->bgprob[i].l = (int32) ((lm->bgprob[i].l - lm->wip) * f) + iwip;

    if (lm->n_tg > 0) {
        for (i = 0; i < lm->n_tgprob; i++)
            lm->tgprob[i].l =
                (int32) ((lm->tgprob[i].l - lm->wip) * f) + iwip;
        for (i = 0; i < lm->n_tgbowt; i++)
            lm->tgbowt[i].l = (int32) (lm->tgbowt[i].l * f);
    }

    lm->lw = (float32) lw;
    lm->wip = iwip;
}


int32
lm_add_wordlist(lm_t * lm,      /**< In/Out: a modified LM structure */
                dict_t * dict,      /**< In: a dictionary */
                const char *filename       /**< In: a file that contains a
					list of word one wants to
					add*/
    )
{
    FILE *fp;
    char string[1024];
    char word[1024];
    int32 n;

    fp = NULL;
    if ((fp = fopen(filename, "r")) == NULL) {
        E_ERROR("Cannot open file %s\n", filename);
        return LM_FAIL;
    }

    while (fgets(string, sizeof(string), fp) != NULL) {
        n = sscanf(string, "%s", word);
        if (n != 1) {
            E_INFO
                ("Detecting more than 1 word in one line. Only using the first word. \n");
            return LM_FAIL;
        }
        E_INFO("%s\n", word);
        if (lm_add_word_to_ug(lm, dict, word) == LM_FAIL)
            E_INFO("Fail to add word %s into the unigram\n", word);
    }

    if (lm == NULL) {
        E_ERROR("LM pointer is NULL.  lm_add_wordlist failed.\n");
        return LM_FAIL;
    }

    fclose(fp);
    return LM_SUCCESS;
}

/*
  INCOMPLETE
 */
int32
lm_add_word_to_ug(lm_t * lm,      /**<In/Out: a modified LM structure */
                  dict_t * dict,      /**< In: an initialized dictionary structure */
                  const char *newword       /**< In: a new word */
    )
{
    s3wid_t w;
    s3lmwid_t lwid;
    void *id;
    int32 classid = BAD_LMCLASSID;

  /** ARCHAN 20060320
      Add a word into the unigram. 
      look up the dictionary and see whether it exists in the dictionary
      Looks alike with wid.c's logic at this point.  

      We also avoid the addition of classes at this point because that
      could complicated things quite a lot */

  /** Reallocate the size of lm->ug, lm->wordstr
      Update the value lm->n_ug, lm->max_ug;
   */

    if (hash_table_lookup(lm->HT, newword, &id) == 0) {
        E_WARN("The word %s already exists in the language model \n",
               newword);
        return LM_FAIL;
    }

    lm->n_ug = lm->n_ug + 1;
    lm->max_ug = lm->n_ug;

    E_INFO("lm->n_ug %d\n", lm->n_ug);
    lm->ug = (ug_t *) ckd_realloc(lm->ug, (lm->n_ug + 1) * sizeof(ug_t));       /* Yes, +2 look at NewUnigramModel(n_ug+1) */
    lm->wordstr =
        (char **) ckd_realloc(lm->wordstr, (lm->n_ug) * sizeof(char *));

  /** Reallocate the size of lm->membg 
      and lm->tginfo
  */

    if (!lm->is32bits) {
        lm->membg =
            (membg_t *) ckd_realloc(lm->membg,
                                    (lm->n_ug) * sizeof(membg_t));
        lm->tginfo =
            (tginfo_t **) ckd_realloc(lm->tginfo,
                                      (lm->n_ug) * sizeof(tginfo_t *));
        lm->tginfo[lm->n_ug - 1] = NULL;
    }
    else {
        lm->membg32 =
            (membg32_t *) ckd_realloc(lm->membg32,
                                      (lm->n_ug) * sizeof(membg32_t));
        lm->tginfo32 =
            (tginfo32_t **) ckd_realloc(lm->tginfo32,
                                        (lm->n_ug) * sizeof(tginfo32_t *));
        lm->tginfo32[lm->n_ug - 1] = NULL;
    }


    E_WARN("Invoke incomplete lm_add_word_to_ug\n");

  /** Insert the entry into lm->ug and lm->wordstr */

    /* 
       This part is not compeleted, prob.f should be the second best
       unigram probability.  This is a fairly standard that was used by
       Dragon and also recommended by Roni. 
     */

    lm->ug[lm->n_ug].prob.f = -99.0;
    lm->ug[lm->n_ug].bowt.f = -99.0;
    lm->ug[lm->n_ug].dictwid = lm->n_ug;        /* See the comment in ug_t, this is not exactly correct
                                                   externally application needs to set it again. 
                                                 */

    /* Supposingly, the bigram should follow the unigram order.
       Because, we have no bigram inserted in this case, the
       unigram.firstbg will just follow the previous one.  */

    lm->ug[lm->n_ug].firstbg = lm->ug[lm->n_ug - 1].firstbg;

    lm->wordstr[lm->n_ug - 1] = (char *) ckd_salloc(newword);

    hash_table_enter(lm->HT, lm->wordstr[lm->n_ug - 1], (void *)(long)(lm->n_ug - 1));

    if (dict != NULL) {
                  /** If dictionary is initialized and used in this context */
    /** Insert the mapping from LM WID to dictionary Word ID  */
        w = dict_wordid(dict, newword);

        if (lm->lmclass)
            classid = lm_get_classid(lm, newword);

        lwid = lm->dict2lmwid[w];

        E_INFO("%d\n", lwid);

        if (IS_S3WID(w)) {
            if ((lm->lmclass) && (classid != BAD_LMCLASSID)) {
                E_ERROR("%s is both a word and an LM class name\n",
                        lm_wordstr(lm, lm->n_ug - 1));
                return LM_FAIL;
            }
            else {
                if (dict_filler_word(dict, w))
                    E_ERROR("Filler dictionary word '%s' found in LM\n",
                            lm_wordstr(lm, lm->n_ug - 1));

                if (w != dict_basewid(dict, w)) {
                    E_ERROR
                        ("LM word '%s' is an alternative pronunciation in dictionary\n",
                         lm_wordstr(lm, lm->n_ug - 1));

                    w = dict_basewid(dict, w);
                    lm_lmwid2dictwid(lm, lm->n_ug - 1) = w;
                }

                for (; IS_S3WID(w); w = dict_nextalt(dict, w))
                    lm->dict2lmwid[w] = (s3lmwid32_t) (lm->n_ug - 1);
            }
        }
        else {
            E_ERROR
                ("Thew new word is not in the dictionary.  We will not do anything in this case\n");
            return LM_FAIL;
        }

    }
    return LM_SUCCESS;
}

lm_t *
lm_read(const char *file, const char *lmname, cmd_ln_t *config, logmath_t *logmath)
{
    return lm_read_advance(file,
                           lmname,
                           cmd_ln_float32_r(config, "-lw"),
                           cmd_ln_float32_r(config, "-wip"),
                           cmd_ln_float32_r(config, "-uw"), 0, NULL, 1, logmath);
}

lm_t *
lm_read_advance(const char *file, const char *lmname, float64 lw,
                float64 wip, float64 uw, int32 ndict, const char *fmt,
                int32 applyWeight, logmath_t *logmath)
{
    return lm_read_advance2(file, lmname, lw, wip, uw, ndict, fmt, applyWeight, 0, logmath);
}

lm_t *
lm_read_advance2(const char *file, const char *lmname, float64 lw,
                 float64 wip, float64 uw, int32 ndict, const char *fmt,
                 int32 applyWeight, int lminmemory, logmath_t *logmath)
{
    int32 i, u;
    lm_t *lm;
    int32 err_no;

    if (!file)
        E_FATAL("No LM file\n");
    if (lw <= 0.0)
        E_FATAL("lw = %e\n", lw);
    if (wip <= 0.0)
        E_FATAL("wip = %e\n", wip);
    if ((uw < 0.0) || (uw > 1.0))
        E_FATAL("uw = %e\n", uw);

    /* HACK: At this part, one should check whether the LM name is being used already */

    E_INFO("LM read('%s', lw= %.2f, wip= %.2f, uw= %.2f)\n", file, lw, wip,
           uw);
    E_INFO("Reading LM file %s (LM name \"%s\")\n", file, lmname);

    /* First it will try to decide whether the file a .DMP file */
    /* ARCHAN: We should provide function pointer implementation at here. */
    if (fmt == NULL) {
        /**Automatically decide the LM format */
        lm = lm_read_dump(file, lminmemory, logmath, 0);
        if (lm == NULL) {
            E_INFO("In lm_read, LM is not a DMP file. Trying to read it as a txt file\n");
            if (lminmemory == 0) {
                lminmemory = 1;
            }
            lm = lm_read_txt(file, lminmemory, &err_no, 0, logmath); /* Not forcing 32bit LM */
            if (lm == NULL) {
                if (err_no == LM_OFFSET_TOO_LARGE) {
                    E_INFO
                        ("In lm read, LM is not a DMP, it is likely to be a ARPA format file. But the LM hits the limit of legacy 16 bit format. Force LM reading to 32bit now\n");

                    /* This only happens when both TXT & DMP format reading have problems */
                    lm = lm_read_txt(file, lminmemory, &err_no, 1, logmath);      /* Now force 32bit LM */
                    if (lm == NULL) {
                        E_INFO
                            ("Panic: In lm_read, LM is not DMP format, it is likely to be ARPA format and hits legacy 16 bit format problem. But when forcing to 32bit LM, problem still couldn't be solved.\n");
                        return NULL;
                    }
                }
                else {
                    E_INFO("Lm is both not DMP and TXT format\n");
                    return NULL;
                }
            }
        }
    }
    else if (!strcmp(fmt, "TXT")) {
        lm = lm_read_txt(file, lminmemory, &err_no, 0, logmath);  /* Not forcing 32bit LM */
        if (lm == NULL) {
            if (err_no == LM_OFFSET_TOO_LARGE) {
                E_INFO
                    ("In lm read, LM is not a DMP, it is likely to be a ARPA format file. But the LM hits the limit of legacy 16 bit format. Force LM reading to 32bit now\n");

                /* This only happens when both TXT & DMP format reading have problems */
                lm = lm_read_txt(file, lminmemory, &err_no, 1, logmath);  /* Now force 32bit LM */
                if (lm == NULL) {
                    E_INFO
                        ("Panic: In lm_read, LM is not DMP format, it is likely to be ARPA format and hits legacy 16 bit format problem. But when forcing to 32bit LM, problem still couldn't be solved.\n");
                    return NULL;
                }
            }
            else {
                E_INFO("LM is not in TXT format\n");
                return NULL;
            }
        }

    }
    else if (!strcmp(fmt, "DMP")) {
        lm = lm_read_dump(file, lminmemory, logmath, 1);
        if (lm == NULL) {
            E_INFO
                ("In lm_read, a DMP format reader is called, but lm cannot be read, Diagnosis: LM is corrupted or not enough memory.\n");
            return NULL;
        }
    }
    else if (!strcmp(fmt, "TXT32")) {
        lm = lm_read_txt(file, lminmemory, &err_no, 1, logmath);
        if (lm == NULL) {
            E_INFO("In lm_read, failed to read lm in txt format. .\n");
            return NULL;
        }
    }
    else {
        E_INFO("Unknown format (%s) is specified\n", fmt);
        return NULL;
    }

    /* the following code is for MMIE training
       lqin 2010-03 */
    lm->ugonly = cmd_ln_boolean("-ugonly");
    if (lm->ugonly == TRUE)
        E_INFO("Only use Unigram to compute LM score in decoding\n");

    lm->bgonly = cmd_ln_boolean("-bgonly");
    if (lm->bgonly == TRUE)
        E_INFO("Only use Bigram to compute LM score in decoding\n");

    if (lm->ugonly && lm->bgonly) {
        E_FATAL("Both -ugonly and -bgonly are set to YES\n");
    }
    /* end */

    lm->name = ckd_salloc(lmname);
    lm->inputenc = IND_BADENCODING;
    lm->outputenc = IND_BADENCODING;

    lm->is32bits = lm_is32bits(lm);

    E_INFO("The LM routine is operating at %d bits mode\n",
           lm->is32bits ? 32 : 16);

    /* Initialize the fast trigram cache, with all entries invalid */
    if (lm->n_tg > 0) {
        if (lm->is32bits) {
            lm->tgcache32 =
                (lm_tgcache_entry32_t *) ckd_calloc(LM_TGCACHE_SIZE,
                        sizeof
                        (lm_tgcache_entry32_t));
            for (i = 0; i < LM_TGCACHE_SIZE; i++)
                lm->tgcache32[i].lwid[0] = (s3lmwid32_t) BAD_LMWID(lm);
        }
        else {
            lm->tgcache =
                (lm_tgcache_entry_t *) ckd_calloc(LM_TGCACHE_SIZE,
                        sizeof(lm_tgcache_entry_t));
            for (i = 0; i < LM_TGCACHE_SIZE; i++)
                lm->tgcache[i].lwid[0] = (s3lmwid_t) BAD_LMWID(lm);
        }
    }

    if (applyWeight) {
        lm2logs3(lm, uw);       /* Applying unigram weight; convert to logs3 values */

        /* Apply the new lw and wip values */
        lm->lw = 1.0;           /* The initial settings for lw and wip */
        lm->wip = 0;            /* logs3(1.0) */
        lm_set_param(lm, lw, wip);
    }


    assert(lm);
    /* Set the size of dictionary */
    lm->dict_size = ndict;
    /*    E_INFO("lm->dict %d\n",lm->dict_size); */
    for (u = 0; u < lm->n_ug; u++)
        lm->ug[u].dictwid = BAD_S3WID;


    return lm;
}

/*
  This convert every string in the lm from lmp->inputenc to
  lm->outputenc.  This function assumes the caller has checked the
  encoding schemes appropriateness. 

  (Caution!) At 20051115, the method is specific and only support hex
  to value conversion.  The code also hasn't considered that output
  encoding requires a longer length of string than the input encoding.
 */
static void
lm_convert_encoding(lm_t * lmp)
{
    int i;

    E_INFO("Encoding Conversion\n");
    for (i = 0; i < lmp->n_ug; i++) {
#if 0
        E_INFO("%s\n", lmp->wordstr[i]);
#endif

        if (ishex(lmp->wordstr[i])) {
            hextocode(lmp->wordstr[i]);
        }

#if 0
        E_INFO("%s\n", lmp->wordstr[i]);
#endif
    }
}

int32
lm_write_advance(lm_t * lmp, const char *outputfn, const char *filename,
                 const char *fmt, const char *inputenc, char *outputenc)
{
    /* This might be duplicated with the caller checking but was done for extra safety. */

    assert(encoding_resolve(inputenc, outputenc));

    lmp->inputenc = encoding_str2ind(inputenc);
    lmp->outputenc = encoding_str2ind(outputenc);

    if (lmp->inputenc != lmp->outputenc) {
        E_INFO("Did I come here?\n");
        lm_convert_encoding(lmp);
    }

    if (!strcmp(fmt, "TXT")) {

        return lm_write_arpa_text(lmp, outputfn, inputenc, outputenc);

    }
    else if (!strcmp(fmt, "DMP")) {

        /* set mtime to be zero because sphinx3 has no mechanism to check
           whether the file is generated earlier (at least for now.) */

        if (lm_is32bits(lmp)) {
            E_INFO
                ("16 bit DMP format is specified but LM is decided to be 32 bit mode. (May be it has segment size which is large than 64k or programmer forced it).\n",
                 LM_LEGACY_CONSTANT);
            E_INFO("Now use 32 bits format.\n");
            return lm3g_dump(outputfn, lmp, filename, 0, 32);
        }
        else {
            return lm3g_dump(outputfn, lmp, filename, 0, 16);
        }


    }
    else if (!strcmp(fmt, "DMP32")) {

        /* set mtime to be zero because sphinx3 has no mechanism to check
           whether the file is generated earlier (at least for now.) */

        return lm3g_dump(outputfn, lmp, filename, 0, 32);

    }
    else if (!strcmp(fmt, "FST")) {

        E_WARN("Invoke un-tested ATT-FSM writer\n");
        return lm_write_att_fsm(lmp, outputfn);

    }
    else {

        E_INFO("Unknown format (%s) is specified\n", fmt);
        return LM_FAIL;
    }
}

int32
lm_write(lm_t * lmp, const char *outputfn, const char *filename, const char *fmt)
{
    return lm_write_advance(lmp, outputfn, filename, fmt, "iso8859-1",
                            "iso8859-1");
}


/*
 * Free stale bigram and trigram info, those not used since last reset.
 */
void
lm_cache_reset(lm_t * lm)
{
    int32 i, n_bgfree, n_tgfree;
    tginfo_t *tginfo, *next_tginfo, *prev_tginfo;
    tginfo32_t *tginfo32, *next_tginfo32, *prev_tginfo32;
    int32 is32bits;

    n_bgfree = n_tgfree = 0;


    /* ARCHAN: RAH only short-circult this function only */
    if (lm->isLM_IN_MEMORY)     /* RAH We are going to short circuit this if we are running with the lm in memory */
        return;

    is32bits = lm->is32bits;

    if ((lm->n_bg > 0) && (!lm->bg)) {  /* Disk-based; free "stale" bigrams */

        if (is32bits) {
            for (i = 0; i < lm->n_ug; i++) {
                if (lm->membg32[i].bg32 && (!lm->membg32[i].used)) {
                    lm->n_bg_inmem -=
                        lm->ug[i + 1].firstbg - lm->ug[i].firstbg;

                    ckd_free(lm->membg32[i].bg32);
                    lm->membg32[i].bg32 = NULL;
                    n_bgfree++;
                }

                lm->membg32[i].used = 0;
            }
        }
        else {
            for (i = 0; i < lm->n_ug; i++) {
                if (lm->membg[i].bg && (!lm->membg[i].used)) {
                    lm->n_bg_inmem -=
                        lm->ug[i + 1].firstbg - lm->ug[i].firstbg;

                    ckd_free(lm->membg[i].bg);
                    lm->membg[i].bg = NULL;
                    n_bgfree++;
                }

                lm->membg[i].used = 0;
            }
        }
    }

    if (lm->n_tg > 0) {
        if (is32bits) {
            for (i = 0; i < lm->n_ug; i++) {
                prev_tginfo32 = NULL;
                for (tginfo32 = lm->tginfo32[i]; tginfo32;
                     tginfo32 = next_tginfo32) {
                    next_tginfo32 = tginfo32->next;

                    if (!tginfo32->used) {
                        if ((!lm->tg32) && tginfo32->tg32) {
                            lm->n_tg_inmem -= tginfo32->n_tg;
                            ckd_free(tginfo32->tg32);
                            n_tgfree++;
                        }

                        ckd_free(tginfo32);
                        if (prev_tginfo32)
                            prev_tginfo32->next = next_tginfo32;
                        else
                            lm->tginfo32[i] = next_tginfo32;
                    }
                    else {
                        tginfo32->used = 0;
                        prev_tginfo32 = tginfo32;
                    }
                }
            }
        }
        else {
            for (i = 0; i < lm->n_ug; i++) {
                prev_tginfo = NULL;
                for (tginfo = lm->tginfo[i]; tginfo; tginfo = next_tginfo) {
                    next_tginfo = tginfo->next;

                    if (!tginfo->used) {
                        if ((!lm->tg) && tginfo->tg) {
                            lm->n_tg_inmem -= tginfo->n_tg;
                            ckd_free(tginfo->tg);
                            n_tgfree++;
                        }

                        free(tginfo);
                        if (prev_tginfo)
                            prev_tginfo->next = next_tginfo;
                        else
                            lm->tginfo[i] = next_tginfo;
                    }
                    else {
                        tginfo->used = 0;
                        prev_tginfo = tginfo;
                    }
                }
            }
        }
    }

    if ((n_tgfree > 0) || (n_bgfree > 0)) {
        E_INFO("%d tg frees, %d in mem; %d bg frees, %d in mem\n",
               n_tgfree, lm->n_tg_inmem, n_bgfree, lm->n_bg_inmem);
    }
}


void
lm_cache_stats_dump(lm_t * lm)
{
    E_INFO
        ("%9d tg(), %9d tgcache, %8d bo; %5d fills, %8d in mem (%.1f%%)\n",
         lm->n_tg_score, lm->n_tgcache_hit, lm->n_tg_bo, lm->n_tg_fill,
         lm->n_tg_inmem, (lm->n_tg_inmem * 100.0) / (lm->n_tg + 1));
    E_INFO("%8d bg(), %8d bo; %5d fills, %8d in mem (%.1f%%)\n",
           lm->n_bg_score, lm->n_bg_bo, lm->n_bg_fill, lm->n_bg_inmem,
           (lm->n_bg_inmem * 100.0) / (lm->n_bg + 1));

    lm->n_tgcache_hit = 0;
    lm->n_tg_fill = 0;
    lm->n_tg_score = 0;
    lm->n_tg_bo = 0;
    lm->n_bg_fill = 0;
    lm->n_bg_score = 0;
    lm->n_bg_bo = 0;
}


int32
lm_ug_score(lm_t * lm, s3lmwid32_t lwid, s3wid_t wid)
{
    if (NOT_LMWID(lm, lwid) || (lwid >= lm->n_ug))
        E_FATAL("Bad argument (%d) to lm_ug_score\n", lwid);

    lm->access_type = 1;

    if (lm->inclass_ugscore)
        return (lm->ug[lwid].prob.l + lm->inclass_ugscore[wid]);
    else
        return (lm->ug[lwid].prob.l);
}

int32
lm_ug_exists(lm_t * lm, s3lmwid32_t lwid)
{
    if (NOT_LMWID(lm, lwid) || (lwid >= lm->n_ug))
        return 0;
    else
        return 1;
}


int32
lm_uglist(lm_t * lm, ug_t ** ugptr)
{
    *ugptr = lm->ug;
    return (lm->n_ug);
}


/* This create a mapping from either the unigram or words in a class*/
int32
lm_ug_wordprob(lm_t * lm, dict_t * dict, int32 th, wordprob_t * wp)
{
    int32 i, j, n, p;
    s3wid_t w, dictid;
    lmclass_t *lmclass;
    lmclass_word_t *lm_cw;
    n = lm->n_ug;

    for (i = 0, j = 0; i < n; i++) {
        w = lm->ug[i].dictwid;
        if (IS_S3WID(w)) {      /*Is w>0? Then it can be either wid or class id */
            if (w < LM_CLASSID_BASE) {  /*It is just a word */
                if ((p = lm->ug[i].prob.l) >= th) {
                    wp[j].wid = w;
                    wp[j].prob = p;
                    j++;
                }
            }
            else {              /* It is a class */
                lmclass = LM_CLASSID_TO_CLASS(lm, w);   /* Get the class */
                lm_cw = lmclass_firstword(lmclass);
                while (lmclass_isword(lm_cw)) {
                    dictid = lmclass_getwid(lm_cw);

                    /*E_INFO("Lookup dict_id using dict_basewid %d\n",dictid); */
                    if (IS_S3WID(dictid)) {
                        if (dictid != dict_basewid(dict, dictid)) {
                            dictid = dict_basewid(dict, dictid);
                        }
                        if ((p =
                             lm->ug[i].prob.l +
                             lm->inclass_ugscore[dictid]) >= th) {
                            wp[j].wid = dictid;
                            wp[j].prob = lm->ug[i].prob.l;
                            j++;
                        }
                    }
                    else {
                        E_INFO("Word %s cannot be found \n",
                               lmclass_getword(lm_cw));
                    }

                    lm_cw = lmclass_nextword(lmclass, lm_cw);

                }
            }
        }
    }

    return j;
}


/*
 * Load bigrams for the given unigram (LMWID) lw1 from disk into memory
 */
static void
load_bg(lm_t * lm, s3lmwid32_t lw1)
{
    int32 i, n, b;
    bg_t *bg = NULL;
    bg32_t *bg32 = NULL;

    int32 mem_sz;
    int32 is32bits;

    b = lm->ug[lw1].firstbg;    /* Absolute first bg index for ug lw1 */
    n = lm->ug[lw1 + 1].firstbg - b;    /* Not including guard/sentinel */

    is32bits = lm->is32bits;
    mem_sz = is32bits ? sizeof(bg32_t) : sizeof(bg_t);

    if (lm->isLM_IN_MEMORY) {   /* RAH, if LM_IN_MEMORY, then we don't need to go get it. */
        if (is32bits)
            bg32 = lm->membg32[lw1].bg32 = &lm->bg32[b];
        else
            bg = lm->membg[lw1].bg = &lm->bg[b];
    }
    else {
        if (is32bits)
            bg32 = lm->membg32[lw1].bg32 =
                (bg32_t *) ckd_calloc(n + 1, mem_sz);
        else
            bg = lm->membg[lw1].bg = (bg_t *) ckd_calloc(n + 1, mem_sz);

        if (fseek(lm->fp, lm->bgoff + b * mem_sz, SEEK_SET) < 0)
            E_FATAL_SYSTEM("fseek failed\n");


        /* Need to read n+1 because obtaining tg count for one bg also depends on next bg */
        if (is32bits) {
            if (fread(bg32, mem_sz, n + 1, lm->fp) != (size_t) (n + 1))
                E_FATAL("fread failed\n");
            if (lm->byteswap) {
                for (i = 0; i <= n; i++)
                    swap_bg32(&(bg32[i]));
            }
        }
        else {
            if (fread(bg, mem_sz, n + 1, lm->fp) != (size_t) (n + 1))
                E_FATAL("fread failed\n");
            if (lm->byteswap) {
                for (i = 0; i <= n; i++)
                    swap_bg(&(bg[i]));
            }
        }
    }
    lm->n_bg_fill++;
    lm->n_bg_inmem += n;
}


#define BINARY_SEARCH_THRESH	16

/* Locate a specific bigram within a bigram list */
int32
find_bg(bg_t * bg, int32 n, s3lmwid32_t w)
{
    int32 i, b, e;

    /* Binary search until segment size < threshold */
    b = 0;
    e = n;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (bg[i].wid < w)
            b = i + 1;
        else if (bg[i].wid > w)
            e = i;
        else
            return i;
    }

    /* Linear search within narrowed segment */
    for (i = b; (i < e) && (bg[i].wid != w); i++);
    return ((i < e) ? i : -1);
}

/* Locate a specific bigram within a bigram list */
int32
find_bg32(bg32_t * bg, int32 n, s3lmwid32_t w)
{
    int32 i, b, e;

    /* Binary search until segment size < threshold */
    b = 0;
    e = n;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (bg[i].wid < w)
            b = i + 1;
        else if (bg[i].wid > w)
            e = i;
        else
            return i;
    }

    /* Linear search within narrowed segment */
    for (i = b; (i < e) && (bg[i].wid != w); i++);
    return ((i < e) ? i : -1);
}


/*** Begin lm_bglist*/
int32
lm_bglist(lm_t * lm, s3lmwid32_t w1, bg_t ** bgptr, int32 * bowt)
{
    int32 n;

    if (NOT_LMWID(lm, w1) || (w1 >= lm->n_ug))
        E_FATAL("Bad w1 argument (%d) to lm_bglist\n", w1);

    n = (lm->n_bg > 0) ? lm->ug[w1 + 1].firstbg - lm->ug[w1].firstbg : 0;

    if (n > 0) {
        if (!lm->membg[w1].bg)
            load_bg(lm, w1);
        lm->membg[w1].used = 1;

        *bgptr = lm->membg[w1].bg;
        *bowt = lm->ug[w1].bowt.l;
    }
    else {
        *bgptr = NULL;
        *bowt = 0;
    }

    return (n);
}

int32
lm_bg32list(lm_t * lm, s3lmwid32_t w1, bg32_t ** bgptr, int32 * bowt)
{
    int32 n;

    if (NOT_LMWID(lm, w1) || (w1 >= lm->n_ug))
        E_FATAL("Bad w1 argument (%d) to lm_bglist\n", w1);

    n = (lm->n_bg > 0) ? lm->ug[w1 + 1].firstbg - lm->ug[w1].firstbg : 0;

    if (n > 0) {
        if (!lm->membg32[w1].bg32)
            load_bg(lm, w1);
        lm->membg32[w1].used = 1;

        *bgptr = lm->membg32[w1].bg32;
        *bowt = lm->ug[w1].bowt.l;
    }
    else {
        *bgptr = NULL;
        *bowt = 0;
    }

    return (n);
}

/*** End lm_bglist*/

/*
 *  This function look-ups the bigram score of p(lw2|lw1)
 *  The information for lw2 and w2 are repeated because the
 *  class based language model implementation uses the dictionary wid
 *  to look up the within class probability.
 */
int32
lm_bg_score(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2, s3wid_t w2)
{
    int32 i, n, score;
    bg_t *bg = NULL;
    bg32_t *bg32 = NULL;
    int32 is32bits;

    is32bits = lm->is32bits;

    /* the following code is modified for MMIE training
       lqin 2010-03 */
    if ((lm->n_bg == 0) || (NOT_LMWID(lm, lw1)) || (lm->ugonly == TRUE))
        return (lm_ug_score(lm, lw2, w2));
    /* end */

    lm->n_bg_score++;

    if (NOT_LMWID(lm, lw2) || (lw2 >= lm->n_ug))
        E_FATAL("Bad lw2 argument (%d) to lm_bg_score\n", lw2);

    n = lm->ug[lw1 + 1].firstbg - lm->ug[lw1].firstbg;

    if (n > 0) {
        if (is32bits) {
            if (!lm->membg32[lw1].bg32)
                load_bg(lm, lw1);
            lm->membg32[lw1].used = 1;
            bg32 = lm->membg32[lw1].bg32;
            i = find_bg32(bg32, n, lw2);
        }
        else {
            if (!lm->membg[lw1].bg)
                load_bg(lm, lw1);
            lm->membg[lw1].used = 1;
            bg = lm->membg[lw1].bg;
            i = find_bg(bg, n, lw2);
        }
    }
    else
        i = -1;

    if (i >= 0) {
        if (is32bits)
            score = lm->bgprob[bg32[i].probid].l;
        else
            score = lm->bgprob[bg[i].probid].l;

        if (lm->inclass_ugscore) {      /*Only add within class prob if class information exists.
                                           Is actually ok to just add the score because if the word
                                           is not within-class. The returning scores will be 0. I just
                                           love to safe-guard it :-).
                                         */
            score += lm->inclass_ugscore[w2];
        }

        lm->access_type = 2;
    }
    else {
        lm->n_bg_bo++;
        lm->access_type = 1;
        score = lm->ug[lw1].bowt.l + lm->ug[lw2].prob.l;
        if (lm->inclass_ugscore) {      /*Only add within class prob if class information exists.
                                           Is actually ok to just add the score because if the word
                                           is not within-class. The returning scores will be 0. I just
                                           love to safe-guard it :-).
                                         */
            score += lm->inclass_ugscore[w2];
        }
    }

    return (score);
}

int32
lm_bg_exists(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2)
{
    int32 i, n, score;
    bg_t *bg = NULL;
    bg32_t *bg32 = NULL;
    int32 is32bits;

    is32bits = lm->is32bits;

    if ((lm->n_bg == 0) || (NOT_LMWID(lm, lw1)))
        return 0;

    if (NOT_LMWID(lm, lw2) || (lw2 >= lm->n_ug))
        return 0;

    n = lm->ug[lw1 + 1].firstbg - lm->ug[lw1].firstbg;

    if (n > 0) {
        if (is32bits) {
            if (!lm->membg32[lw1].bg32)
                load_bg(lm, lw1);
            lm->membg32[lw1].used = 1;
            bg32 = lm->membg32[lw1].bg32;
            i = find_bg32(bg32, n, lw2);
        }
        else {
            if (!lm->membg[lw1].bg)
                load_bg(lm, lw1);
            lm->membg[lw1].used = 1;
            bg = lm->membg[lw1].bg;

            i = find_bg(bg, n, lw2);
        }
    }
    else
        i = -1;

    if (i >= 0)
        return 1;
    else
        return 0;


    return (score);
}


static void
load_tg(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2)
{
    int32 i, n, b;
    int32 t = -1;               /* Let's make sure that if t isn't initialized after the
                                 * "if" statement below, it makes things go bad */
    bg_t *bg = NULL;
    bg32_t *bg32 = NULL;
    tg_t *tg = NULL;
    tg32_t *tg32 = NULL;
    tginfo_t *tginfo = NULL;
    tginfo32_t *tginfo32 = NULL;
    int32 mem_sz_tg, mem_sz_tginfo;
    int32 is32bits;

    is32bits = lm->is32bits;
    mem_sz_tg = is32bits ? sizeof(tg32_t) : sizeof(tg_t);
    mem_sz_tginfo = is32bits ? sizeof(tginfo32_t) : sizeof(tginfo_t);

    /* First allocate space for tg information for bg lw1,lw2 */

    if (is32bits) {
        tginfo32 = (tginfo32_t *) ckd_malloc(mem_sz_tginfo);
        tginfo32->w1 = lw1;
        tginfo32->tg32 = NULL;
        tginfo32->next = lm->tginfo32[lw2];
        lm->tginfo32[lw2] = tginfo32;
    }
    else {
        tginfo = (tginfo_t *) ckd_malloc(mem_sz_tginfo);
        tginfo->w1 = lw1;
        tginfo->tg = NULL;
        tginfo->next = lm->tginfo[lw2];
        lm->tginfo[lw2] = tginfo;
    }

    /* Locate bigram lw1,lw2 */

    b = lm->ug[lw1].firstbg;
    n = lm->ug[lw1 + 1].firstbg - b;


    /* Make sure bigrams for lw1, if any, loaded into memory */
    if (n > 0) {
        if (is32bits) {
            if (!lm->membg32[lw1].bg32)
                load_bg(lm, lw1);
            lm->membg32[lw1].used = 1;
            bg32 = lm->membg32[lw1].bg32;
        }
        else {
            if (!lm->membg[lw1].bg)
                load_bg(lm, lw1);
            lm->membg[lw1].used = 1;
            bg = lm->membg[lw1].bg;
        }
    }

    /* At this point, n = #bigrams for lw1 */
    if (n > 0
        && (i =
            is32bits ? find_bg32(bg32, n, lw2) : find_bg(bg, n,
                                                         lw2)) >= 0) {

        /*      if(i<0){
           E_INFO("What is the value of i %d, lw2 %d\n",i,lw2);
           } */

        if (i >= 0) {
            if (is32bits)
                tginfo32->bowt = lm->tgbowt[bg32[i].bowtid].l;
            else
                tginfo->bowt = lm->tgbowt[bg[i].bowtid].l;


            /* Find t = Absolute first trigram index for bigram lw1,lw2 */
            b += i;             /* b = Absolute index of bigram lw1,lw2 on disk */
            t = lm->tg_segbase[b >> lm->log_bg_seg_sz];
            t += is32bits ? bg32[i].firsttg : bg[i].firsttg;

            /*      E_INFO("%d %d\n",lm->tg_segbase[b >> lm->log_bg_seg_sz],t); */
            /* Find #tg for bigram w1,w2 */
            n = lm->tg_segbase[(b + 1) >> lm->log_bg_seg_sz];
            n += is32bits ? bg32[i + 1].firsttg : bg[i + 1].firsttg;
            n -= t;

            if (is32bits)
                tginfo32->n_tg = n;
            else
                tginfo->n_tg = n;

        }

    }
    else {                      /* No bigram w1,w2 */

        if (is32bits) {
            tginfo32->bowt = 0;
            n = tginfo32->n_tg = 0;
        }
        else {
            tginfo->bowt = 0;
            n = tginfo->n_tg = 0;
        }
    }

    /* "t" has not been assigned any meanigful value, so if you use it
     * beyond this point, make sure it's been properly assigned.
     */
    /*  assert (t != -1); */

    /* At this point, n = #trigrams for lw1,lw2.  Read them in */

    if (lm->isLM_IN_MEMORY) {
        /* RAH, already have this in memory */
        if (n > 0) {
            assert(t != -1);
            if (is32bits)
                tg32 = tginfo32->tg32 = &lm->tg32[t];
            else
                tg = tginfo->tg = &lm->tg[t];
        }
    }
    else {
        if (n > 0) {

            if (is32bits)
                tg32 = tginfo32->tg32 =
                    (tg32_t *) ckd_calloc(n, mem_sz_tg);
            else
                tg = tginfo->tg = (tg_t *) ckd_calloc(n, mem_sz_tg);


            if (fseek(lm->fp, lm->tgoff + t * mem_sz_tg, SEEK_SET) < 0)
                E_FATAL_SYSTEM("fseek failed\n");


            if (is32bits) {
                if (fread(tg32, mem_sz_tg, n, lm->fp) != (size_t) n)
                    E_FATAL("fread(tg32, %d at %d) failed\n", n,
                            lm->tgoff);
                if (lm->byteswap) {
                    for (i = 0; i < n; i++) {
                        SWAP_INT32(&(tg32[i].wid));
                        SWAP_INT32(&(tg32[i].probid));
                    }
                }
            }
            else {
                if (fread(tg, mem_sz_tg, n, lm->fp) != (size_t) n)
                    E_FATAL("fread(tg, %d at %d) failed\n", n, lm->tgoff);
                if (lm->byteswap) {
                    for (i = 0; i < n; i++) {
                        SWAP_INT16(&(tg[i].wid));
                        SWAP_INT16(&(tg[i].probid));
                    }
                }
            }
        }
    }
    lm->n_tg_fill++;
    lm->n_tg_inmem += n;
}


/* Similar to find_bg */
int32
find_tg(tg_t * tg, int32 n, s3lmwid32_t w)
{
    int32 i, b, e;

    b = 0;
    e = n;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (tg[i].wid < w)
            b = i + 1;
        else if (tg[i].wid > w)
            e = i;
        else
            return i;
    }

    for (i = b; (i < e) && (tg[i].wid != w); i++);
    return ((i < e) ? i : -1);
}

int32
find_tg32(tg32_t * tg, int32 n, s3lmwid32_t w)
{
    int32 i, b, e;

    b = 0;
    e = n;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (tg[i].wid < w)
            b = i + 1;
        else if (tg[i].wid > w)
            e = i;
        else
            return i;
    }

    for (i = b; (i < e) && (tg[i].wid != w); i++);
    return ((i < e) ? i : -1);
}


int32
lm_tglist(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2, tg_t ** tgptr,
          int32 * bowt)
{
    tginfo_t *tginfo, *prev_tginfo;

    if (lm->n_tg <= 0) {
        *tgptr = NULL;
        *bowt = 0;
        return 0;
    }

    if (NOT_LMWID(lm, lw1) || (lw1 >= lm->n_ug))
        E_FATAL("Bad lw1 argument (%d) to lm_tglist\n", lw1);
    if (NOT_LMWID(lm, lw2) || (lw2 >= lm->n_ug))
        E_FATAL("Bad lw2 argument (%d) to lm_tglist\n", lw2);

    prev_tginfo = NULL;
    for (tginfo = lm->tginfo[lw2]; tginfo; tginfo = tginfo->next) {
        if (tginfo->w1 == lw1)
            break;
        prev_tginfo = tginfo;
    }

    if (!tginfo) {
        load_tg(lm, lw1, lw2);
        tginfo = lm->tginfo[lw2];
    }
    else if (prev_tginfo) {
        prev_tginfo->next = tginfo->next;
        tginfo->next = lm->tginfo[lw2];
        lm->tginfo[lw2] = tginfo;
    }
    tginfo->used = 1;

    *tgptr = tginfo->tg;
    *bowt = tginfo->bowt;

    return (tginfo->n_tg);
}

int32
lm_tg32list(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2, tg32_t ** tgptr,
            int32 * bowt)
{
    tginfo32_t *tginfo32, *prev_tginfo32;

    if (lm->n_tg <= 0) {
        *tgptr = NULL;
        *bowt = 0;
        return 0;
    }

    if (NOT_LMWID(lm, lw1) || (lw1 >= lm->n_ug))
        E_FATAL("Bad lw1 argument (%d) to lm_tglist\n", lw1);
    if (NOT_LMWID(lm, lw2) || (lw2 >= lm->n_ug))
        E_FATAL("Bad lw2 argument (%d) to lm_tglist\n", lw2);

    prev_tginfo32 = NULL;
    for (tginfo32 = lm->tginfo32[lw2]; tginfo32; tginfo32 = tginfo32->next) {
        if (tginfo32->w1 == lw1)
            break;
        prev_tginfo32 = tginfo32;
    }

    if (!tginfo32) {
        load_tg(lm, lw1, lw2);
        tginfo32 = lm->tginfo32[lw2];
    }
    else if (prev_tginfo32) {
        prev_tginfo32->next = tginfo32->next;
        tginfo32->next = lm->tginfo32[lw2];
        lm->tginfo32[lw2] = tginfo32;
    }
    tginfo32->used = 1;

    *tgptr = tginfo32->tg32;
    *bowt = tginfo32->bowt;

    return (tginfo32->n_tg);
}

/*
 *  This function look-ups the trigram score of p(lw3|lw2,lw1)
 *  and compute the in-class ug probability of w3.
 *  The information for lw3 and w3 are repeated because the
 *  class based language model implementation uses the dictionary wid
 *  to look up the within class probability.
 */
int32
lm_tg_score(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2, s3lmwid32_t lw3,
            s3wid_t w3)
{
    int32 i, h, n, score, inclass_ugscore = 0;
    tg_t *tg;
    tginfo_t *tginfo, *prev_tginfo;
    tg32_t *tg32;
    tginfo32_t *tginfo32, *prev_tginfo32;
    int32 is32bits;

    tg = NULL;
    tginfo = prev_tginfo = NULL;

    tg32 = NULL;
    tginfo32 = prev_tginfo32 = NULL;


    is32bits = lm->is32bits;

    /*    E_INFO("lw1 %d, lw2 %d, lw3 %d is32bits %d BAD_LMWID %d\n",lw1,lw2,lw3,is32bits, BAD_LMWID(lm)); */

    /* the following code is modified for MMIE training
       lqin 2010-03 */
    if ((lm->n_tg == 0) || (NOT_LMWID(lm, lw1)) || (lm->bgonly == TRUE) || (lm->ugonly == TRUE))
        return (lm_bg_score(lm, lw2, lw3, w3));
    /* end */

    lm->n_tg_score++;

    /*    E_INFO("lw1 %d, lw2 %d, lw3 %d is32bits %d BAD_LMWID %d\n",lw1,lw2,lw3,is32bits, BAD_LMWID(lm)); */

    if (NOT_LMWID(lm, lw1) || (lw1 >= lm->n_ug))
        E_FATAL("Bad lw1 argument (%d) to lm_tg_score\n", lw1);
    if (NOT_LMWID(lm, lw2) || (lw2 >= lm->n_ug))
        E_FATAL("Bad lw2 argument (%d) to lm_tg_score\n", lw2);
    if (NOT_LMWID(lm, lw3) || (lw3 >= lm->n_ug))
        E_FATAL("Bad lw3 argument (%d) to lm_tg_score\n", lw3);

    /* Lookup tgcache first; compute hash(lw1, lw2, lw3) */
    h = ((lw1 & 0x000003ff) << 21) + ((lw2 & 0x000003ff) << 11) +
        (lw3 & 0x000007ff);
    h %= LM_TGCACHE_SIZE;


    if (lm->inclass_ugscore)
        inclass_ugscore = lm->inclass_ugscore[w3];

    /*
     * Have to add within class score to the cached language score since
     *  the former cannot be cached.
     */
    if (is32bits) {
        if ((lm->tgcache32[h].lwid[0] == lw1) &&
            (lm->tgcache32[h].lwid[1] == lw2) &&
            (lm->tgcache32[h].lwid[2] == lw3)) {

            lm->n_tgcache_hit++;
            return lm->tgcache32[h].lscr + inclass_ugscore;
        }

        prev_tginfo32 = NULL;
        for (tginfo32 = lm->tginfo32[lw2]; tginfo32;
             tginfo32 = tginfo32->next) {
            if (tginfo32->w1 == lw1)
                break;
            prev_tginfo32 = tginfo32;
        }

    }
    else {
        if ((lm->tgcache[h].lwid[0] == lw1) &&
            (lm->tgcache[h].lwid[1] == lw2) &&
            (lm->tgcache[h].lwid[2] == lw3)) {

            lm->n_tgcache_hit++;
            return lm->tgcache[h].lscr + inclass_ugscore;
        }

        prev_tginfo = NULL;
        for (tginfo = lm->tginfo[lw2]; tginfo; tginfo = tginfo->next) {
            if (tginfo->w1 == lw1)
                break;
            prev_tginfo = tginfo;
        }
    }

    if (is32bits) {
        if (!tginfo32) {
            load_tg(lm, lw1, lw2);
            tginfo32 = lm->tginfo32[lw2];
        }
        else if (prev_tginfo32) {
            prev_tginfo32->next = tginfo32->next;
            tginfo32->next = lm->tginfo32[lw2];
            lm->tginfo32[lw2] = tginfo32;
        }
        tginfo32->used = 1;
    }
    else {
        if (!tginfo) {
            load_tg(lm, lw1, lw2);
            tginfo = lm->tginfo[lw2];
        }
        else if (prev_tginfo) {
            prev_tginfo->next = tginfo->next;
            tginfo->next = lm->tginfo[lw2];
            lm->tginfo[lw2] = tginfo;
        }
        tginfo->used = 1;
    }


    /* Trigrams for w1,w2 now in memory; look for w1,w2,w3 */
    if (is32bits) {
        n = tginfo32->n_tg;
        tg32 = tginfo32->tg32;
        assert(tginfo32);
    }
    else {
        n = tginfo->n_tg;
        tg = tginfo->tg;
        assert(tginfo);
    }

    if (is32bits)
        i = find_tg32(tg32, n, lw3);
    else
        i = find_tg(tg, n, lw3);

    if (i >= 0) {
        if (is32bits)
            score = lm->tgprob[tg32[i].probid].l;
        else
            score = lm->tgprob[tg[i].probid].l;

        score += inclass_ugscore;
        lm->access_type = 3;
    }
    else {
        lm->n_tg_bo++;
        score = is32bits ? tginfo32->bowt : tginfo->bowt;
        score += lm_bg_score(lm, lw2, lw3, w3);
    }

    /*
     * Have to subtract within class score since it cannot be cached
     */
    if (is32bits) {
        lm->tgcache32[h].lwid[0] = lw1;
        lm->tgcache32[h].lwid[1] = lw2;
        lm->tgcache32[h].lwid[2] = lw3;
        lm->tgcache32[h].lscr = score - inclass_ugscore;
    }
    else {
        lm->tgcache[h].lwid[0] = lw1;
        lm->tgcache[h].lwid[1] = lw2;
        lm->tgcache[h].lwid[2] = lw3;
        lm->tgcache[h].lscr = score - inclass_ugscore;
    }


#if 0
    printf("      %5d %5d -> %8d\n", lw1, lw2, score);
    /* ENABLE this when you suspect the lm routine produce abnormal scores */
    if (score > 0) {
        E_INFO
            ("score %d >0 lm->ug[lw1].bowt.l %d lm_ug[lw2].prob.l %d, lw1 %d lw2 %d i, %d\n",
             score, lm->ug[lw1].bowt.l, lm->ug[lw2].bowt.l, lw1, lw2, i);
    }
#endif

    return (score);
}

int32
lm_tg_exists(lm_t * lm, s3lmwid32_t lw1, s3lmwid32_t lw2, s3lmwid32_t lw3)
{
    int32 i, n;
    tg_t *tg;
    tginfo_t *tginfo, *prev_tginfo;
    tg32_t *tg32;
    tginfo32_t *tginfo32, *prev_tginfo32;


    int32 is32bits;

    tg = NULL;
    tginfo = prev_tginfo = NULL;
    tg32 = NULL;
    tginfo32 = prev_tginfo32 = NULL;
    is32bits = lm->is32bits;

    if ((lm->n_tg == 0) || (NOT_LMWID(lm, lw1)))
        return 0;

    if (NOT_LMWID(lm, lw1) || (lw1 >= lm->n_ug))
        return 0;
    if (NOT_LMWID(lm, lw2) || (lw2 >= lm->n_ug))
        return 0;
    if (NOT_LMWID(lm, lw3) || (lw3 >= lm->n_ug))
        return 0;

    if (is32bits) {
        prev_tginfo32 = NULL;
        for (tginfo32 = lm->tginfo32[lw2]; tginfo32;
             tginfo32 = tginfo32->next) {
            if (tginfo32->w1 == lw1)
                break;
            prev_tginfo32 = tginfo32;
        }
    }
    else {
        prev_tginfo = NULL;
        for (tginfo = lm->tginfo[lw2]; tginfo; tginfo = tginfo->next) {
            if (tginfo->w1 == lw1)
                break;
            prev_tginfo = tginfo;
        }
    }

    if (is32bits) {
        if (!tginfo32) {
            load_tg(lm, lw1, lw2);
            tginfo32 = lm->tginfo32[lw2];
        }
        else if (prev_tginfo32) {
            prev_tginfo32->next = tginfo32->next;
            tginfo32->next = lm->tginfo32[lw2];
            lm->tginfo32[lw2] = tginfo32;
        }
        tginfo32->used = 1;
        /* Trigrams for w1,w2 now in memory; look for w1,w2,w3 */
        n = tginfo32->n_tg;
        tg32 = tginfo32->tg32;
        assert(tginfo32);
    }
    else {
        if (!tginfo) {
            load_tg(lm, lw1, lw2);
            tginfo = lm->tginfo[lw2];
        }
        else if (prev_tginfo) {
            prev_tginfo->next = tginfo->next;
            tginfo->next = lm->tginfo[lw2];
            lm->tginfo[lw2] = tginfo;
        }
        tginfo->used = 1;
        /* Trigrams for w1,w2 now in memory; look for w1,w2,w3 */
        n = tginfo->n_tg;
        tg = tginfo->tg;
        assert(tginfo);
    }

    if (is32bits)
        i = find_tg32(tg32, n, lw3);
    else
        i = find_tg(tg, n, lw3);

    if (i >= 0)
        return 1;
    else
        return 0;
}


s3lmwid32_t
lm_wid(lm_t * lm, const char *word)
{
    int32 i;

    for (i = 0; i < lm->n_ug; i++)
        if (strcmp(lm->wordstr[i], word) == 0)
            return ((s3lmwid32_t) i);

    return BAD_LMWID(lm);
}

void
lm_free(lm_t * lm)
{
    int i;
    tginfo_t *tginfo;
    tginfo32_t *tginfo32;

    if (lm->fp)
        fclose(lm->fp);

    ckd_free((void *) lm->ug);

    for (i = 0; i < lm->n_ug; i++)
        ckd_free((void *) lm->wordstr[i]);      /*  */
    ckd_free((void *) lm->wordstr);

    if (lm->n_bg > 0) {
        if (lm->bg || lm->bg32) {       /* Memory-based; free all bg */
            if (lm->bg)
                ckd_free(lm->bg);
            if (lm->bg32)
                ckd_free(lm->bg32);

            if (lm->membg)
                ckd_free(lm->membg);
            if (lm->membg32)
                ckd_free(lm->membg32);
        }
        else {                  /* Disk-based; free in-memory bg */
            if (lm->membg) {
                for (i = 0; i < lm->n_ug; ++i)
                    ckd_free(lm->membg[i].bg);
                ckd_free(lm->membg);
            }
            if (lm->membg32) {
                for (i = 0; i < lm->n_ug; ++i)
                    ckd_free(lm->membg32[i].bg32);
                ckd_free(lm->membg32);
            }
        }

        ckd_free(lm->bgprob);
    }

    if (lm->n_tg > 0) {
        if (lm->tg)
            ckd_free((void *) lm->tg);
        if (lm->tg32)
            ckd_free((void *) lm->tg32);

        if (lm->tginfo) {
            for (i = 0; i < lm->n_ug; i++) {
                if (lm->tginfo[i] != NULL) {
                    /* Free the whole linked list of tginfo. */
                    while (lm->tginfo[i]) {
                        tginfo = lm->tginfo[i];
                        lm->tginfo[i] = tginfo->next;
                        if (!lm->isLM_IN_MEMORY)
                            ckd_free(tginfo->tg);
                        ckd_free((void *) tginfo);
                    }
                }
            }
            ckd_free((void *) lm->tginfo);
        }
        if (lm->tginfo32) {
            for (i = 0; i < lm->n_ug; i++) {
                if (lm->tginfo32[i] != NULL) {
                    while (lm->tginfo32[i]) {
                        tginfo32 = lm->tginfo32[i];
                        lm->tginfo32[i] = tginfo32->next;
                        if (!lm->isLM_IN_MEMORY)
                            ckd_free(tginfo32->tg32);
                        ckd_free((void *) tginfo32);
                    }
                }
            }
            ckd_free((void *) lm->tginfo32);
        }



        if (lm->tgcache)
            ckd_free((void *) lm->tgcache);
        if (lm->tgcache32)
            ckd_free((void *) lm->tgcache32);

        ckd_free((void *) lm->tg_segbase);
        ckd_free((void *) lm->tgprob);
        ckd_free((void *) lm->tgbowt);
    }

    if (lm->lmclass) {
        for (i = 0; i < lm->n_lmclass; ++i)
            lmclass_free(lm->lmclass[i]);
        ckd_free(lm->lmclass);
    }

    if (lm->inclass_ugscore) {
        ckd_free(lm->inclass_ugscore);
    }

    if (lm->HT) {
        hash_table_free(lm->HT);
    }

    if (lm->dict2lmwid) {
        ckd_free(lm->dict2lmwid);
    }

    if (lm->name)
        ckd_free(lm->name);

    ckd_free((void *) lm);
}

static void
copy_bgt_to_bg32t(bg_t * b16, bg32_t * b32)
{
    b32->wid = (s3lmwid32_t) b16->wid;
    b32->probid = (uint32) b16->probid;
    b32->bowtid = (uint32) b16->bowtid;
    b32->firsttg = (uint32) b16->firsttg;
}

void
copy_bg_to_bg32(lm_t * lm)
{
    int i;
    assert(lm->bg32 == NULL);
    lm->bg32 = (bg32_t *) ckd_calloc(lm->n_bg + 1, sizeof(bg32_t));

    for (i = 0; i <= lm->n_bg; i++)
        copy_bgt_to_bg32t(&(lm->bg[i]), &(lm->bg32[i]));
}

static void
copy_bg32t_to_bgt(bg32_t * b32, bg_t * b16)
{
    assert(b32->wid <= LM_LEGACY_CONSTANT);
    b16->wid = (s3lmwid_t) b32->wid;
    b16->probid = (uint16) b32->probid;
    b16->bowtid = (uint16) b32->bowtid;
    b16->firsttg = (uint16) b32->firsttg;
}

void
copy_bg32_to_bg(lm_t * lm)
{
    int i;
    assert(lm->bg == NULL);
    lm->bg = (bg_t *) ckd_calloc(lm->n_bg + 1, sizeof(bg_t));

    for (i = 0; i <= lm->n_bg; i++)
        copy_bg32t_to_bgt(&(lm->bg32[i]), &(lm->bg[i]));

}

void
swap_bg(bg_t * b16)
{
    SWAP_INT16(&(b16->wid));
    SWAP_INT16(&(b16->probid));
    SWAP_INT16(&(b16->bowtid));
    SWAP_INT16(&(b16->firsttg));
}

void
swap_bg32(bg32_t * b32)
{
    SWAP_INT32(&(b32->wid));
    SWAP_INT32(&(b32->probid));
    SWAP_INT32(&(b32->bowtid));
    SWAP_INT32(&(b32->firsttg));
}

static void
copy_tgt_to_tg32t(tg_t * t16, tg32_t * t32)
{
    t32->wid = (s3lmwid32_t) t16->wid;
    t32->probid = (uint32) t16->probid;
}



void
copy_tg_to_tg32(lm_t * lm)
{
    int i;
    assert(lm->tg32 == NULL);
    lm->tg32 = (tg32_t *) ckd_calloc(lm->n_tg, sizeof(tg32_t));

    for (i = 0; i < lm->n_tg; i++)
        copy_tgt_to_tg32t(&(lm->tg[i]), &(lm->tg32[i]));

}

static void
copy_tg32t_to_tgt(tg32_t * t32, tg_t * t16)
{
    t16->wid = (s3lmwid_t) t32->wid;
    t16->probid = (uint32) t32->probid;
}


void
copy_tg32_to_tg(lm_t * lm)
{
    int i;
    assert(lm->tg == NULL);
    lm->tg = (tg_t *) ckd_calloc(lm->n_tg, sizeof(tg_t));

    for (i = 0; i < lm->n_tg; i++)
        copy_tg32t_to_tgt(&(lm->tg32[i]), &(lm->tg[i]));

}

void
swap_tg(tg_t * t16)
{
    SWAP_INT16(&(t16->wid));
    SWAP_INT16(&(t16->probid));
}

void
swap_tg32(tg32_t * t32)
{
    SWAP_INT32(&(t32->wid));
    SWAP_INT32(&(t32->probid));
}


int32
lm_rawscore(lm_t * lm, int32 score)
{
    score -= lm->wip;
    score /= lm->lw;

    return score;
}

void
lm_convert_structure(lm_t * model, int32 is32bits)
{
    /* Convert the data structure */
    if (is32bits) {             /* Convert from 16 bits to 32 bits */
        if (model->n_bg > 0) {
            if (model->bg32 == NULL) {
                assert(model->bg != NULL);
                copy_bg_to_bg32(model);
            }
        }
        if (model->n_tg > 0) {
            if (model->tg32 == NULL) {
                assert(model->tg != NULL);
                copy_tg_to_tg32(model);
            }
        }
    }
    else {                      /* Convert from 32 bits to 16 bits */
        if (model->n_bg > 0) {
            if (model->bg == NULL) {
                assert(model->bg32 != NULL);
                copy_bg32_to_bg(model);
            }
        }
        if (model->n_tg > 0) {
            if (model->tg == NULL) {
                assert(model->tg32 != NULL);
                copy_tg32_to_tg(model);
            }
        }
    }

    if (is32bits) {
        if (model->bg > 0)
            assert(model->bg32 != NULL);
        if (model->tg > 0)
            assert(model->tg32 != NULL);
    }
    else {
        if (model->bg > 0)
            assert(model->bg != NULL);
        if (model->tg > 0)
            assert(model->tg != NULL);
    }

}



#if (_LM_TEST_)
static int32
sentence_lmscore(lm_t * lm, const char *line)
{
    char *word[1024];
    s3lmwid32_t w[1024];
    int32 nwd, score, tgscr;
    int32 i, j;

    if ((nwd = str2words(line, word, 1020)) < 0)
        E_FATAL("Increase word[] and w[] arrays size\n");

    w[0] = BAD_LMWID(lm);
    w[1] = lm_wid(lm, S3_START_WORD);
    if (NOT_LMWID(lm, w[1]))
        E_FATAL("Unknown word: %s\n", S3_START_WORD);

    for (i = 0; i < nwd; i++) {
        w[i + 2] = lm_wid(lm, word[i]);
        if (NOT_LMWID(lm, w[i + 2])) {
            E_ERROR("Unknown word: %s\n", word[i]);
            return 0;
        }
    }

    w[i + 2] = lm_wid(lm, S3_FINISH_WORD);
    if (NOT_LMWID(lm, w[i + 2]))
        E_FATAL("Unknown word: %s\n", S3_FINISH_WORD);

    score = 0;
    for (i = 0, j = 2; i <= nwd; i++, j++) {
        tgscr = lm_tg_score(lm, w[j - 2], w[j - 1], w[j]);
        score += tgscr;
        printf("\t%10d %s\n", tgscr, lm->wordstr[w[j]]);
    }

    return (score);
}


main(int32 argc, char *argv[])
{
    char line[4096];
    int32 score, k;
    lm_t *lm;

    if (argc < 2)
        E_FATAL("Usage: %s <LMdumpfile>\n", argv[0]);

    logs3_init(1.0001, 1, 1);
    lm = lm_read(argv[1], 9.5, 0.2);

    if (1) {                    /* Short cut this so we can test for memory leaks */
        for (;;) {
            printf("> ");
            if (fgets(line, sizeof(line), stdin) == NULL)
                break;

            score = sentence_lmscore(lm, line);

            k = strlen(line);
            if (line[k - 1] == '\n')
                line[k - 1] = '\0';
            printf("LMScr(%s) = %d\n", line, score);
        }
    }                           /*  */
    lm_free(lm);
    exit(0);
}
#endif
