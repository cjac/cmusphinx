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
/** \file lm.c
 \brief Language model implementation
 
 This is the implementation file for language model support in Sphinx 3.
 */
/* **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log: lm.c,v $
 * 2008/11/5  P. Deleglise and Y. Esteve
 * Adapted the code to manage large files (off_t 64 bits instead of int32)
 *
 * 2008/08/21  N. Coetmeur, supervised by Y. Esteve
 * Add conditionnal statements (#ifndef _READ_DUMP_ONLY_) for parts of code that we must
 * deactivating for use LM model with our others tools.
 *
 * 2008/06/27  N. Coetmeur, supervised by Y. Esteve
 * Adjust comments for compatibility with Doxygen 1.5.6
 *
 * 2008/06/24  N. Coetmeur, supervised by Y. Esteve
 * load_ng function became non-static.
 *
 * 2008/06/20  N. Coetmeur, supervised by Y. Esteve
 * Adapt LM functions for working with N-grams for each N value
 * (i.e for a trigram file N=3, for a quadrigram file N=4 and so on...).
 *   - Replace functions with 'ug', 'bg' and 'tg' in their name by one function
 *    (with 'ng' in the name) with the N value as a parameter.
 *    [ few exceptions : lm_uglist, lm_ug_wordprob, load_bg, lm_add_word_to_ug ]
 *   - New function lm_init_n_ng: allocate memory for tables in LM with the max
 *   Ng level value
 *   - lm_write and lm_write_advance functions now used separately output
 *    directory and output file name in parameters instead of the complete full
 *    path in previous version, the full path is compute in lm_write_advance for
 *    add the good extension for the output format
 *   - load_bg is now only called by load_ng
 *   - lm3g_dump is renamed lmng_dump
 *   - lm_is32bits use version and number of unigrams in parameters instead of a
 *   lm_t structure pointer
 * Verify if lmclass is allocated in lm_ug_wordprob
 *
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

/** Search thresh used by find_ng and find_ng32 functions */
#define BINARY_SEARCH_THRESH	16


/*ARCHAN, 20041112: NOP, NO STATIC VARIABLES! */

/**
 \brief Read an Arpa-based LM file
 
 (200060708) At this point, it is not memory leak-free. 
 
 \return An initialized LM if everything is alright,
 NULL if something goes wrong
 */
extern lm_t *lm_read_txt(const char *filename, /**< The file name */
                         int32 lminmemory,  /**< Whether using in memory LM */
                         int32 *err_no, /**< Input/Output: Depends on the problem that LM
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

/**
 \brief Read a DMP file (<-lmname->.DMP dump LM format)
 \return Pointer to LM structure created
 */
extern lm_t *lm_read_dump(const char *file,  /**< The file name*/
                          int lminmemory,  /**< Whether using in memory LM */
                          logmath_t *logmath
    );

/* LIUM */
/**
 \brief Write a LM in Dump file format
 \return Error code
 */
int32 lmng_dump(char const *file,   /**< the file name */
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

/* LIUM */
/* Apply unigram weight; should be part of LM creation, but... */
static void
lm_uw(lm_t * lm, float64 uw)
{
    int32 i, loguw, loguw_, loguniform, p1, p2;
	
    /* Interpolate unigram probs with uniform PDF, with weight uw */
    loguw = logs3(lm->logmath, uw);
    loguw_ = logs3(lm->logmath, 1.0 - uw);
    loguniform = logs3(lm->logmath, 1.0 / (lm->n_ng[0] - 1));   /* Skipping S3_START_WORD */
	
    for (i = 0; i < lm->n_ng[0]; i++) {
        if (strcmp(lm->wordstr[i], S3_START_WORD) != 0) {
            p1 = lm->ug[i].prob.l + loguw;
            p2 = loguniform + loguw_;
            lm->ug[i].prob.l = logmath_add(lm->logmath, p1, p2);
        }
    }
}

/* LIUM */
/** \brief Convert all probabilities to logs3 values */
static void
lm2logs3(lm_t * lm, float64 uw)
{
    uint32 i, n;
	
	/* Unigrams */
    for (i = 0; i < lm->n_ng[0]; i++) {
        lm->ug[i].prob.l = logmath_log10_to_log(lm->logmath, lm->ug[i].prob.f);
		
        /* This prevent underflow if the backoff value is too small 
		 It happens sometimes in cmu-lmtk V3's lm_combine. 
         */
		
        if (lm->ug[i].bowt.f < MIN_PROB_F)
            lm->ug[i].bowt.f = MIN_PROB_F;
		
        lm->ug[i].bowt.l = logmath_log10_to_log(lm->logmath, lm->ug[i].bowt.f);
    }
	
    lm_uw(lm, uw);
	
	/* Bigrams */
    for (i = 0; i < lm->n_ngprob[1]; i++)
        lm->ngprob[1][i].l = logmath_log10_to_log(lm->logmath, lm->ngprob[1][i].f);
	
	/* Trigrams to (max_ng)grams */
	for (n = 3; n <= lm->max_ng; n++) {
		if (lm->n_ng[n-1] > 0) {
			for (i = 0; i < lm->n_ngprob[n-1]; i++)
				lm->ngprob[n-1][i].l = logmath_log10_to_log(lm->logmath, lm->ngprob[n-1][i].f);
			for (i = 0; i < lm->n_ngbowt[n-1]; i++) {
			
				if (lm->ngbowt[n-1][i].f < MIN_PROB_F)
					lm->ngbowt[n-1][i].f = MIN_PROB_F;
			
				lm->ngbowt[n-1][i].l = logmath_log10_to_log(lm->logmath, lm->ngbowt[n-1][i].f);
			}
		}
	}
}

/* LIUM */
/*
 * Load bigrams for the given unigram (LMWID) lw1 from disk into memory
 */
static void
load_bg(lm_t * lm, s3lmwid32_t lw1)
{
    int32 i, n, b;
    ng_t *bg = NULL;
    ng32_t *bg32 = NULL;
	
    int32 mem_sz;
    int32 is32bits;
	
    b = lm->ug[lw1].firstbg;    /* Absolute first bg index for ug lw1 */
    n = lm->ug[lw1 + 1].firstbg - b;    /* Not including guard/sentinel */
	
    is32bits = lm->is32bits;
    mem_sz = is32bits ? sizeof(ng32_t) : sizeof(ng_t);
	
    if (lm->isLM_IN_MEMORY) {   /* RAH, if LM_IN_MEMORY, then we don't need to go get it. */
        if (is32bits)
            bg32 = lm->membg32[lw1].bg32 = &lm->ng32[1][b];
        else
            bg = lm->membg[lw1].bg = &lm->ng[1][b];
    }
    else {
        if (is32bits)
            bg32 = lm->membg32[lw1].bg32 =
			(ng32_t *) ckd_calloc(n + 1, mem_sz);
        else
            bg = lm->membg[lw1].bg = (ng_t *) ckd_calloc(n + 1, mem_sz);
		
        if (fseeko(lm->fp, lm->ngoff[1] + ((off_t)b) * mem_sz, SEEK_SET) < 0)
            E_FATAL_SYSTEM("fseek failed\n");
		
		
        /* Need to read n+1 because obtaining tg count for one bg also depends on next bg */
        if (is32bits) {
            if (fread(bg32, mem_sz, n + 1, lm->fp) != (size_t) (n + 1))
                E_FATAL("fread failed\n");
            if (lm->byteswap) {
                for (i = 0; i <= n; i++)
                    swap_ng32(&(bg32[i]));
            }
        }
        else {
            if (fread(bg, mem_sz, n + 1, lm->fp) != (size_t) (n + 1))
                E_FATAL("fread failed\n");
            if (lm->byteswap) {
                for (i = 0; i <= n; i++)
                    swap_ng(&(bg[i]));
            }
        }
    }
    lm->n_ng_fill[1]++;
    lm->n_ng_inmem[1] += n;
}

/* LIUM */
/**
 \brief Copy a 16-bits N-gram to a 32-bits N-gram
 */
static void
copy_ngt_to_ng32t(ng_t * n16, ng32_t * n32)
{
    n32->wid = (s3lmwid32_t) n16->wid;
    n32->probid = (uint32) n16->probid;
    n32->bowtid = (uint32) n16->bowtid;
    n32->firstnng = (uint32) n16->firstnng;
}

/* LIUM */
/**
 \brief Copy a 32-bits N-gram to a 16-bits N-gram
 */
static void
copy_ng32t_to_ngt(ng32_t * n32, ng_t * n16)
{
    assert(n32->wid <= LM_LEGACY_CONSTANT);
    n16->wid = (s3lmwid_t) n32->wid;
    n16->probid = (uint16) n32->probid;
    n16->bowtid = (uint16) n32->bowtid;
    n16->firstnng = (uint16) n32->firstnng;
}

/* LIUM */
#ifndef _READ_DUMP_ONLY_
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
    for (i = 0; i < lmp->n_ng[0]; i++) {
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
#endif

/* LIUM */
#if (_LM_TEST_)
/**
 \brief Compute the score of a sentence
 \return the score
 */
static int32
sentence_lmscore(lm_t * lm, const char *line)
{
    char *word[1024];
    s3lmwid32_t w[1024];
    int32 nwd, score, ngscr;
    uint32 i, j, N;
	
    if ((nwd = str2words(line, word, 1020)) < 0)
        E_FATAL("Increase word[] and w[] arrays size\n");
	
	for (N = 1; N <= (lm->max_ng - 2); N++)
		w[N-1] = BAD_LMWID(lm);
	
    w[N-2] = lm_wid(lm, S3_START_WORD);
    if (NOT_LMWID(lm, w[N-2]))
        E_FATAL("Unknown word: %s\n", S3_START_WORD);
	
    for (i = 0; i < nwd; i++) {
        w[i + N-1] = lm_wid(lm, word[i]);
        if (NOT_LMWID(lm, w[i + N-1])) {
            E_ERROR("Unknown word: %s\n", word[i]);
            return 0;
        }
    }
	
    w[i + N-1] = lm_wid(lm, S3_FINISH_WORD);
    if (NOT_LMWID(lm, w[i + N-1]))
        E_FATAL("Unknown word: %s\n", S3_FINISH_WORD);
	
    score = 0;
    for (i = 0, j = (N - 1); i <= nwd; i++, j++) {
        ngscr = lm_ng_score(lm, N, &(w[j - N+1]), w[j]);
        score += ngscr;
        printf("\t%10d %s\n", ngscr, lm->wordstr[w[j]]);
    }
	
    return (score);
}
#endif

/* LIUM */
/**
   The function to return whether an LM should be 32bit or not.
   It is decided by whether we are using 32bit mode DMP.  Or whether
   it is LMTXT_VERSION but with more than 0xffff words.  The final
   criterion is when LMFORCE_TXT32VERSION. 
 */
int32
lm_is32bits(int32 version, int32 n_ug)
{
    if (version == LMDMP_VERSION_32BIT)
        return 1;
    if (version == LMFORCED_TXT32VERSION)
        return 1;
    if (version == LMTXT_VERSION && n_ug > LM_LEGACY_CONSTANT)
        return 1;
    if (version == LMFST_VERSION && n_ug > LM_LEGACY_CONSTANT)
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

/* LIUM */
void
lm_null_struct(lm_t * lm)
{
    lm->name = NULL;
    lm->wordstr = NULL;

	lm->n_ng = NULL;
    lm->ug = NULL;
    lm->ng = NULL;
    lm->membg = NULL;
    lm->nginfo = NULL;
    lm->ngcache = NULL;
    lm->dict2lmwid = NULL;

    lm->ng32 = NULL;
    lm->membg32 = NULL;
    lm->nginfo32 = NULL;
    lm->ngcache32 = NULL;

	lm->n_ngprob = NULL;
    lm->ngprob = NULL;
    lm->n_ngbowt = NULL;
    lm->ngbowt = NULL;

    lm->ng_segbase = NULL;
    lm->lmclass = NULL;
    lm->inclass_ugscore = NULL;
	
	lm->fp = NULL;
    lm->ngoff = NULL;
    
    lm->n_ng_fill = NULL;
    lm->n_ng_inmem = NULL;
    lm->n_ng_score = NULL;
    lm->n_ng_bo = NULL;
    lm->n_ngcache_hit = NULL;
    lm->sorted_probn = NULL;
    lm->sorted_bowtn = NULL;
	
    lm->HT = NULL;
	
    lm->logmath = NULL;
}

/* LIUM */
/*
 Allocate memory for tables in LM with the max Ng level value
 
 Note : call lm_null_struct function before calling this function with a new LM
 */
int32
lm_init_n_ng(lm_t *lm, uint32 max_Ng, int32 n_ug, int32 version, int32 inmemory)
{
    lm->max_ng = max_Ng;
    lm->version = version;
    lm->isLM_IN_MEMORY = inmemory;
    lm->is32bits = lm_is32bits(version, n_ug);
	
    if (lm->n_ng == NULL)
        if ( (lm->n_ng = (int32 *) ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    lm->n_ng[0] = n_ug;
	
    if (lm->log_ng_seg_sz == NULL)
        if ( (lm->log_ng_seg_sz = (uint32 *)
              ckd_calloc (max_Ng, sizeof(uint32))) == NULL )
            return LM_FAIL;
	
    if (lm->is32bits) {
        if (inmemory)
            if (lm->ng32 == NULL)
                if ( (lm->ng32 = (ng32_t **)
                      ckd_calloc (max_Ng, sizeof(ng32_t *))) == NULL )
                    return LM_FAIL;
		
        if (lm->nginfo32 == NULL)
            if ( (lm->nginfo32 = (nginfo32_t ***)
                  ckd_calloc (max_Ng, sizeof(nginfo32_t **))) == NULL )
                return LM_FAIL;
		
        if (lm->ngcache32 == NULL)
            if ( (lm->ngcache32 = (lm_ngcache_entry32_t **)
                  ckd_calloc (max_Ng, sizeof(lm_ngcache_entry32_t *))) == NULL )
                return LM_FAIL;
    }
    else {
        if (inmemory)
            if (lm->ng == NULL)
                if ( (lm->ng = (ng_t **)
                      ckd_calloc (max_Ng, sizeof(ng_t *))) == NULL )
                    return LM_FAIL;
		
        if (lm->nginfo == NULL)
            if ( (lm->nginfo = (nginfo_t ***)
                  ckd_calloc (max_Ng, sizeof(nginfo_t **))) == NULL )
                return LM_FAIL;
		
        if (lm->ngcache == NULL)
            if ( (lm->ngcache = (lm_ngcache_entry_t **)
                  ckd_calloc (max_Ng, sizeof(lm_ngcache_entry_t *))) == NULL )
                return LM_FAIL;
    }
	
    if (lm->ngprob == NULL)
        if ( (lm->ngprob = (lmlog_t **)
              ckd_calloc (max_Ng, sizeof(lmlog_t *))) == NULL )
            return LM_FAIL;
	
    if (lm->ngbowt == NULL)
        if ( (lm->ngbowt = (lmlog_t **)
              ckd_calloc (max_Ng, sizeof(lmlog_t *))) == NULL )
            return LM_FAIL;
	
    if (lm->ng_segbase == NULL)
        if ( (lm->ng_segbase = (int32 **)
              ckd_calloc (max_Ng, sizeof(int32 *))) == NULL )
            return LM_FAIL;
	
    if (lm->n_ngprob == NULL)
        if ( (lm->n_ngprob = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->n_ngbowt == NULL)
        if ( (lm->n_ngbowt = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->ngoff == NULL)
        if ( (lm->ngoff = (off_t *)
              ckd_calloc (max_Ng, sizeof(off_t))) == NULL )
            return LM_FAIL;
    
    if (lm->n_ng_fill == NULL)
        if ( (lm->n_ng_fill = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->n_ng_inmem == NULL)
        if ( (lm->n_ng_inmem = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->n_ng_score == NULL)
        if ( (lm->n_ng_score = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->n_ng_bo == NULL)
        if ( (lm->n_ng_bo = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->n_ngcache_hit == NULL)
        if ( (lm->n_ngcache_hit = (int32 *)
              ckd_calloc (max_Ng, sizeof(int32))) == NULL )
            return LM_FAIL;
    
    if (lm->sorted_probn == NULL)
        if ( (lm->sorted_probn = (sorted_list_t *)
              ckd_calloc (max_Ng, sizeof(sorted_list_t))) == NULL )
            return LM_FAIL;
    
    if (lm->sorted_bowtn == NULL)
        if ( (lm->sorted_bowtn = (sorted_list_t *)
              ckd_calloc (max_Ng, sizeof(sorted_list_t))) == NULL )
            return LM_FAIL;
    
    return LM_SUCCESS;
}

/* LIUM */
void
lm_set_param(lm_t * lm, float64 lw, float64 wip)
{
    uint32 i, n, iwip;
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

	/* Unigrams */
    for (i = 0; i < lm->n_ng[0]; i++) {
        lm->ug[i].prob.l =
            (int32) ((lm->ug[i].prob.l - lm->wip) * f) + iwip;
        lm->ug[i].bowt.l = (int32) (lm->ug[i].bowt.l * f);
    }

	/* Bigrams */
    for (i = 0; i < lm->n_ngprob[1]; i++)
        lm->ngprob[1][i].l = (int32) ((lm->ngprob[1][i].l - lm->wip) * f) + iwip;

	/* Trigrams to (max_ng)grams */
	for (n = 3; n <= lm->max_ng; n++) {
		if (lm->n_ng[n-1] > 0) {
			for (i = 0; i < lm->n_ngprob[n-1]; i++)
				lm->ngprob[n-1][i].l =
					(int32) ((lm->ngprob[n-1][i].l - lm->wip) * f) + iwip;
			for (i = 0; i < lm->n_ngbowt[n-1]; i++)
				lm->ngbowt[n-1][i].l = (int32) (lm->ngbowt[n-1][i].l * f);
		}
	}
		
    lm->lw = (float32) lw;
    lm->wip = iwip;
}

lm_t *
lm_read_advance(const char *file, const char *lmname, float64 lw,
                float64 wip, float64 uw, int32 ndict, const char *fmt,
                int32 applyWeight, logmath_t *logmath)
{
    return lm_read_advance2(file, lmname, lw, wip, uw, ndict, fmt, applyWeight, 0, logmath);
}

/* LIUM */
lm_t *
lm_read_advance2(const char *file, const char *lmname, float64 lw,
                 float64 wip, float64 uw, int32 ndict, const char *fmt,
                 int32 applyWeight, int lminmemory, logmath_t *logmath)
{
    uint32 i, n;
    lm_t *lm;
#ifndef _READ_DUMP_ONLY_
    int32 err_no;
#endif
	
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
        lm = lm_read_dump(file, lminmemory, logmath);
        if (lm == NULL) {
#ifdef _READ_DUMP_ONLY_
            E_INFO("In lm_read, LM is not a DMP file\n");
            return NULL;
#else
            E_INFO("In lm_read, LM is not a DMP file. Trying to read it as a txt file\n");
            if (lminmemory == 0) {
                E_WARN("On-disk LM not supported for text files, reading it into memory.\n");
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
#endif
        }
    }
#ifndef _READ_DUMP_ONLY_
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
#endif
    else if (!strcmp(fmt, "DMP")) {
        lm = lm_read_dump(file, lminmemory, logmath);
        if (lm == NULL) {
            E_INFO
			("In lm_read, a DMP format reader is called, but lm cannot be read, Diagnosis: LM is corrupted or not enough memory.\n");
            return NULL;
        }
    }
#ifndef _READ_DUMP_ONLY_
    else if (!strcmp(fmt, "TXT32")) {
        lm = lm_read_txt(file, lminmemory, &err_no, 1, logmath);
        if (lm == NULL) {
            E_INFO("In lm_read, failed to read lm in txt format. .\n");
            return NULL;
        }
    }
#endif
    else {
        E_INFO("Unknown format (%s) is specified\n", fmt);
        return NULL;
    }
	
	
    lm->name = ckd_salloc(lmname);
#ifndef _READ_DUMP_ONLY_
    lm->inputenc = IND_BADENCODING;
    lm->outputenc = IND_BADENCODING;
#endif
	
    lm->is32bits = lm_is32bits(lm->version, lm->n_ng[0]);
	
    E_INFO("The LM routine is operating at %d bits mode\n",
           lm->is32bits ? 32 : 16);
	
    /* Initialize the fast trigram cache, with all entries invalid */
	if (lm->is32bits) {
		for (n = 3; n <= lm->max_ng; n++) {
			lm->ngcache32[n-1] =
			(lm_ngcache_entry32_t *) ckd_calloc(LM_NGCACHE_SIZE,
												sizeof
												(lm_ngcache_entry32_t));
			for (i = 0; i < LM_NGCACHE_SIZE; i++) {
				lm->ngcache32[n-1][i].lwid =
					(s3lmwid32_t *) ckd_calloc(n, sizeof(s3lmwid32_t));
				lm->ngcache32[n-1][i].lwid[0] = (s3lmwid32_t) BAD_LMWID(lm);
			}
		}
	}
	else {
		for (n = 3; n <= lm->max_ng; n++) {
			lm->ngcache[n-1] =
			(lm_ngcache_entry_t *) ckd_calloc(LM_NGCACHE_SIZE,
											  sizeof(lm_ngcache_entry_t));
			for (i = 0; i < LM_NGCACHE_SIZE; i++) {
				lm->ngcache[n-1][i].lwid =
					(s3lmwid_t *) ckd_calloc(n, sizeof(s3lmwid_t));
				lm->ngcache[n-1][i].lwid[0] = (s3lmwid_t) BAD_LMWID(lm);
			}
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
    for (i = 0; i < lm->n_ng[0]; i++)
        lm->ug[i].dictwid = BAD_S3WID;
	
    return lm;
}

/* LIUM */
#ifndef _READ_DUMP_ONLY_
int32
lm_write_advance(lm_t * lmp, const char *outputdir, const char *outputfn, 
				 const char *filename, const char *fmt, 
				 const char *inputenc, char *outputenc)
{
	int32 return_value = 0;
    char *outputpath;
    char separator[2];
    
    /* Outputpath = outputdir . "/" (or "\" in windows)
	 . outputfn (or filename . outputext); */
    /* Length = strlen(outputdir) + 1 + strlen(outputfn) [or strlen(filename)
	 + strlen(outputext)] + 1 (\0) */
	
#if WIN32
    strcpy(separator, "\\");
#else
    strcpy(separator, "/");
#endif
	
    if (outputfn == NULL) {
        outputpath = (char *)
		ckd_calloc(1, strlen(outputdir) + 1 + strlen(filename) + 6 + 1);
    }
    else {
        /* outputfn is define by the user */
        outputpath = (char *)
		ckd_calloc(1, strlen(outputdir) + 1 + strlen(outputfn) + 1);
        sprintf(outputpath, "%s%s%s", outputdir, separator, outputfn);
    }
	
    /* This might be duplicated with the caller checking but was done for extra safety. */
    assert(encoding_resolve(inputenc, outputenc));
	
    lmp->inputenc = encoding_str2ind(inputenc);
    lmp->outputenc = encoding_str2ind(outputenc);
	
    if (lmp->inputenc != lmp->outputenc) {
        E_INFO("Did I come here?\n");
        lm_convert_encoding(lmp);
    }
	
    if (!strcmp(fmt, "TXT")) {
		if (outputfn == NULL) /* no output file name given */
							  /* use input file name with TXT extension */
            sprintf(outputpath, "%s%s%s.TXT", outputdir, separator, filename);
		
        return_value = lm_write_arpa_text(lmp, outputpath, inputenc, outputenc);
    }
    else if (!strcmp(fmt, "DMP")) {
		
        /* set mtime to be zero because sphinx3 has no mechanism to check
		 whether the file is generated earlier (at least for now.) */
		
        if (lmp->is32bits) {
            E_INFO
			("16 bit DMP format is specified but LM is decided to be 32 bit mode. (May be it has segment size which is large than 64k or programmer forced it).\n",
			 LM_LEGACY_CONSTANT);
            E_INFO("Now use 32 bits format.\n");
			
			if (outputfn == NULL) /* no output file name given */
								  /* use input file name with DMP32 extension */
                sprintf (outputpath, "%s%s%s.DMP32",
                         outputdir, separator, filename);
			
            return_value = lmng_dump(outputpath, lmp, filename, 0, 32);
        }
        else {
			if (outputfn == NULL) /* no output file name given */
								  /* use input file name with DMP extension */
                sprintf (outputpath, "%s%s%s.DMP",
                         outputdir, separator, filename);
			
            return_value = lmng_dump(outputpath, lmp, filename, 0, 16);
        }
    }
    else if (!strcmp(fmt, "DMP32")) {
		
        /* set mtime to be zero because sphinx3 has no mechanism to check
		 whether the file is generated earlier (at least for now.) */
		
		if (outputfn == NULL) /* no output file name given */
							  /* use input file name with DMP32 extension */
            sprintf(outputpath, "%s%s%s.DMP32", outputdir, separator, filename);
		
        return_value = lmng_dump(outputpath, lmp, filename, 0, 32);
		
    }
    else if (!strcmp(fmt, "FST")) {
		
        E_WARN("Invoke un-tested ATT-FSM writer\n");
		
		if (outputfn == NULL) /* no output file name given */
							  /* use input file name with FST extension */
            sprintf(outputpath, "%s%s%s.FST", outputdir, separator, filename);
		
        return_value = lm_write_att_fsm(lmp, outputpath);
		
    }
    else {
		
        E_INFO("Unknown format (%s) is specified\n", fmt);
        return_value = LM_FAIL;
    }
	
	if (outputpath != NULL)
        ckd_free(outputpath);
    
    return return_value;
}

/* LIUM */
int32
lm_write(lm_t * lmp, const char *outputdir, const char *outputfn,
		 const char *filename, const char *fmt)
{
    return lm_write_advance(lmp, outputdir, outputfn, filename, 
							fmt, "iso8859-1", "iso8859-1");
}
#endif

/* LIUM */
/*
 * Free stale bigrams, trigrams and more informations, 
 * those not used since last reset.
 */
void
lm_cache_reset(lm_t * lm)
{
	uint32 i, n;
    int32 *n_ngfree;
    nginfo_t *nginfo, *next_nginfo, *prev_nginfo;
    nginfo32_t *nginfo32, *next_nginfo32, *prev_nginfo32;
    int32 is32bits, b_comment = FALSE;
	char *st_comment;
	
    n_ngfree = (int32 *) ckd_calloc(lm->max_ng, sizeof(int32));
	
    /* ARCHAN: RAH only short-circult this function only */
    if (lm->isLM_IN_MEMORY)     /* RAH We are going to short circuit this if we are running with the lm in memory */
        return;
	
    is32bits = lm->is32bits;
	
	/* Disk-based; free "stale" bigrams */
    if (lm->n_ng[1] > 0) {
        if (is32bits && ((!lm->ng32) || (!lm->ng32[1]))) {
            for (i = 0; i < lm->n_ng[0]; i++) {
                if (lm->membg32[i].bg32 && (!lm->membg32[i].used)) {
                    lm->n_ng_inmem[1] -=
					lm->ug[i + 1].firstbg - lm->ug[i].firstbg;
					
                    ckd_free(lm->membg32[i].bg32);
                    lm->membg32[i].bg32 = NULL;
                    n_ngfree[1]++;
                }
				
                lm->membg32[i].used = 0;
            }
        }
        else if ((!lm->ng) || (!lm->ng[1])) {
            for (i = 0; i < lm->n_ng[0]; i++) {
                if (lm->membg[i].bg && (!lm->membg[i].used)) {
                    lm->n_ng_inmem[1] -=
					lm->ug[i + 1].firstbg - lm->ug[i].firstbg;
					
                    ckd_free(lm->membg[i].bg);
                    lm->membg[i].bg = NULL;
                    n_ngfree[1]++;
                }
				
                lm->membg[i].used = 0;
            }
        }
    }
	
	/* Trigrams to (max_ng)grams */
	for (n = 3; n <= lm->max_ng; n++) {
		if (lm->n_ng[n-1] > 0) {
			if (is32bits) {
				for (i = 0; i < lm->n_ng[0]; i++) {
					prev_nginfo32 = NULL;
					for (nginfo32 = lm->nginfo32[n-1][i]; nginfo32;
						 nginfo32 = next_nginfo32) {
						next_nginfo32 = nginfo32->next;
					
						if (!nginfo32->used) {
							if (((!lm->ng32) || (!lm->ng32[n-1]))
								&& nginfo32->ng32) {
								lm->n_ng_inmem[n-1] -= nginfo32->n_ng;
								ckd_free(nginfo32->ng32);
								n_ngfree[n-1]++;
							}
						
							if (nginfo32->w != NULL)
								ckd_free(nginfo32->w);
							
							ckd_free(nginfo32);
							if (prev_nginfo32)
								prev_nginfo32->next = next_nginfo32;
							else
								lm->nginfo32[n-1][i] = next_nginfo32;
						}
						else {
							nginfo32->used = 0;
							prev_nginfo32 = nginfo32;
						}
					}
				}
			}
			else {
				for (i = 0; i < lm->n_ng[0]; i++) {
					prev_nginfo = NULL;
					for (nginfo = lm->nginfo[n-1][i]; nginfo; 
						 nginfo = next_nginfo) {
						next_nginfo = nginfo->next;
					
						if (!nginfo->used) {
							if (((!lm->ng) || (!lm->ng[n-1])) && nginfo->ng) {
								lm->n_ng_inmem[n-1] -= nginfo->n_ng;
								ckd_free(nginfo->ng);
								n_ngfree[n-1]++;
							}
						
							if (nginfo->w != NULL)
								 ckd_free(nginfo->w);
								 
							ckd_free(nginfo);
							if (prev_nginfo)
								prev_nginfo->next = next_nginfo;
							else
								lm->nginfo[n-1][i] = next_nginfo;
						}
						else {
							nginfo->used = 0;
							prev_nginfo = nginfo;
						}
					}
				}
			}
		}
	}
	
	/* print numbers in mem and free */
    if (n_ngfree != NULL) {
		st_comment = (char *) ckd_calloc ((lm->max_ng)*53, sizeof(char));
        if (st_comment != NULL) {
            st_comment[0] = '\0';
			
            for (n = lm->max_ng; n >= 2; n--) {
                if (n_ngfree[n-1] > 0)
                    b_comment = TRUE;
				
                sprintf (st_comment, "%s; %d %d-grams freed, %d in memory",
                         st_comment, n_ngfree[n-1], n, lm->n_ng_inmem[n-1]);
            }
            
            if (b_comment == TRUE)
                E_INFO ("%s\n", st_comment + 2);
        }
    }
}

void
lm_cache_stats_dump(lm_t * lm)
{
	uint32 n;
	
	/* Trigrams to (max_ng)grams */
	for (n = lm->max_ng; n >= 3; n--) {
		E_INFO ("%9d %d-grams, %9d %d-grams cached, %8d back-off weights; %5d fills, %8d in memory (%.1f%%)\n"
                , lm->n_ng_score[n-1], n, lm->n_ngcache_hit[n-1], n
                , lm->n_ng_bo[n-1], lm->n_ng_fill[n-1], lm->n_ng_inmem[n-1]
                , (lm->n_ng_inmem[n-1] * 100.0) / (lm->n_ng[n-1] + 1));
		
        lm->n_ngcache_hit[n-1] = 0;
        lm->n_ng_fill[n-1] = 0;
        lm->n_ng_score[n-1] = 0;
        lm->n_ng_bo[n-1] = 0;
	}
	
	/* Bigrams */
    E_INFO (
			"%8d 2-grams, %8d back-off weights; %5d fills, %8d in memory (%.1f%%)\n"
            , lm->n_ng_score[1], lm->n_ng_bo[1], lm->n_ng_fill[1],
            lm->n_ng_inmem[1], (lm->n_ng_inmem[1] * 100.0) / (lm->n_ng[1] + 1));
    
    lm->n_ng_fill[1] = 0;
    lm->n_ng_score[1] = 0;
    lm->n_ng_bo[1] = 0;
}

/* LIUM */
int32
lm_uglist(lm_t * lm, ug_t ** ugptr)
{
	*ugptr = lm->ug;
	return (lm->n_ng[0]);
}

/* LIUM */
#ifndef _READ_DUMP_ONLY_
/* This create a mapping from either the unigram or words in a class*/
int32
lm_ug_wordprob(lm_t * lm, dict_t * dict, int32 th, wordprob_t * wp)
{
	int32 i, j, n, p;
	s3wid_t w, dictid;
	lmclass_t *lmclass;
	lmclass_word_t *lm_cw;
	n = lm->n_ng[0];
 
	for (i = 0, j = 0; i < n; i++) {
		w = lm->ug[i].dictwid;
		if (IS_S3WID(w)) {      /*Is w>0? Then it can be either wid or class id */
			if ((w < LM_CLASSID_BASE) || (lm->lmclass == NULL)) {  /*It is just a word */
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
#endif

/* LIUM */
/* Locate a specific N-gram within a N-gram list */
int32
find_ng(ng_t * ng, int32 l, s3lmwid32_t w)
{
    int32 i, b, e;
	
	/* verify parameter */
    if (ng == NULL)
        return -1;
	
	/* Binary search until segment size < threshold */
    b = 0;
    e = l;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (ng[i].wid < w)
            b = i + 1;
        else if (ng[i].wid > w)
            e = i;
        else
            return i;
    }
	
	/* Linear search within narrowed segment */
    for (i = b; (i < e) && (ng[i].wid != w); i++);
    return ((i < e) ? i : -1);
}

/* LIUM */
/* Locate a specific N-gram within a N-gram list */
int32
find_ng32(ng32_t * ng, int32 l, s3lmwid32_t w)
{
    int32 i, b, e;
	
	/* verify parameter */
    if (ng == NULL)
        return -1;
	
	/* Binary search until segment size < threshold */
    b = 0;
    e = l;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (ng[i].wid < w)
            b = i + 1;
        else if (ng[i].wid > w)
            e = i;
        else
            return i;
    }
	
	/* Linear search within narrowed segment */
    for (i = b; (i < e) && (ng[i].wid != w); i++);
    return ((i < e) ? i : -1);
}

/* LIUM */
void load_ng(lm_t * lm, uint32 N, s3lmwid32_t *lw)
/* N >= 2 */
{
    int32 sni, fnn, fnb;
    int32 snb, snn;
	ng_t nextsng;
	ng_t *sng = NULL;
	ng32_t nextsng32;
	ng32_t *sng32 = NULL;
	nginfo_t *snginfo = NULL;
	nginfo32_t *snginfo32 = NULL;
	ng_t *fng = NULL;
	ng32_t *fng32 = NULL;
	nginfo_t *fnginfo = NULL;
	nginfo32_t *fnginfo32 = NULL;
    int32 mem_sz_ng, mem_sz_nginfo;
    int32 is32bits;
	uint32 i, j, br;
	
	if (N <= 1) {
		E_FATAL("load_ng doesn't work with N <= 1\n");
		return;
	}
	
	if (N == 2)
		return load_bg(lm, lw[0]);
	
    is32bits = lm->is32bits;
    mem_sz_ng = is32bits ? sizeof(ng32_t) : sizeof(ng_t);
    mem_sz_nginfo = is32bits ? sizeof(nginfo32_t) : sizeof(nginfo_t);
	
    /* First allocate space for Ng information for (N-1)g lw1...lwN-1 */
    if (is32bits) {
        fnginfo32 = (nginfo32_t *) ckd_malloc(mem_sz_nginfo);
		fnginfo32->w = (s3lmwid32_t *) ckd_calloc(N-2, sizeof(s3lmwid32_t));
		
		for (i = 0; i < (N-2); i++)
			fnginfo32->w[i] = lw[i];
		
		fnginfo32->n_ng = 0;
		fnginfo32->bowt = 0;
		fnginfo32->ng32 = NULL;
		fnginfo32->next = lm->nginfo32[N-1][lw[N-2]];
		lm->nginfo32[N-1][lw[N-2]] = fnginfo32;
    }
    else {
        fnginfo = (nginfo_t *) ckd_malloc (mem_sz_nginfo);
		fnginfo->w = (s3lmwid_t *) ckd_calloc (N-2, sizeof(s3lmwid_t));
		
        for (i = 0; i < (N-2); i++)
            fnginfo->w[i] = lw[i];
		
		fnginfo->n_ng = 0;
		fnginfo->bowt = 0;
		fnginfo->ng = NULL;
		fnginfo->next = lm->nginfo[N-1][lw[N-2]];
		lm->nginfo[N-1][lw[N-2]] = fnginfo;
    }
	
	if(is32bits) {
    /* search bigram */
	
		snb = lm->ug[lw[0]].firstbg;
		snn = lm->ug[lw[0] + 1].firstbg - snb;
	
		if (! lm->membg32[lw[0]].bg32) {
			load_bg (lm, lw[0]);
			lm->membg32[lw[0]].used = 1;
		}
		
		sng32 = lm->membg32[lw[0]].bg32;
		sni = find_ng32 (sng32, snn, lw[1]);
		
        if (sni>=0)
            sng32 = sng32 + sni;
        else
            return;
	
		snb += sni;
		
        /* snb   : bigram index of bigram w1,w2
		 sng32 : pointer to bigram w1,w2      */
		
        /* search (N-1)grams */
        for (i = 3; i <= (N-1); i++) {
            snb = lm->ng_segbase[i-1][snb >> (lm->log_ng_seg_sz[i-2])];
            snb += sng32->firstnng;
            /* snb : (i)gram absolute index of first (i)gram
			 corresponding to w1...w(i-1) */
			
            /* Make sure (i)grams for lw1...lw(i-1),
			 if any, loaded into memory */
            for (snginfo32 = lm->nginfo32[i-1][lw[i-2]]; snginfo32;
                 snginfo32 = snginfo32->next) {
                br = TRUE;
                for (j = 0; (j < (i-2)) && br; j++)
                    if (snginfo32->w[j] != lw[j])
                        br = FALSE;
                if (br)
                    break;
            }
            if (!snginfo32) {
                load_ng(lm, i, lw);
                snginfo32=lm->nginfo32[i-1][lw[i-2]];
            }
            snginfo32->used = 1;
            if (snginfo32->n_ng == 0)
                return;
            
            /* (i)grams for w1...w(i-1) now in memory; look for w1...w(i) */
            sng32 = snginfo32->ng32;
            assert(sng32);
            sni = find_ng32(sng32, snginfo32->n_ng, lw[i-1]);
            if (sni >= 0) {
                sng32 = sng32 + sni;
                snb = snb + sni ;
            }
            else
                return;
            /* snb   : (i)gram index of (i)gram w1...w(i)
			 sng32 : pointer to (i)gram w1...w(i)       */
        }
		
        if (sni>=0)
            fnginfo32->bowt = lm->ngbowt[N-1][sng32->bowtid].l;
    }
	else {
	/* search bigram */
		
        snb = lm->ug[lw[0]].firstbg;
        snn = lm->ug[lw[0]+1].firstbg - snb;
        
		if (!lm->membg[lw[0]].bg) {
			load_bg(lm, lw[0]);
			lm->membg[lw[0]].used = 1;
		}
		
		sng = lm->membg[lw[0]].bg;
		sni = find_ng(sng, snn, lw[1]);
        if (sni >= 0)
            sng = sng + sni;
        else
            return;
		
        snb += sni;
		
        /* snb   : bigram index of bigram w1,w2
		 sng32 : pointer to bigram w1,w2      */
		
        /* search (N-1)grams, if N >= 4 */
        for (i = 3; i <= (N-1); i++)
        {
            snb = lm->ng_segbase[i-1][snb >> (lm->log_ng_seg_sz[i-2])];
            snb += sng->firstnng;
            /* snb : (i)gram absolute index of first (i)gram
			 corresponding to w1...w(i-1) */
			
            /* Make sure (i)grams for lw1...lw(i-1),
			 if any, loaded into memory */
            for (snginfo = lm->nginfo[i-1][lw[i-2]]; snginfo;
                 snginfo = snginfo->next) {
                br = TRUE;
                for (j = 0; (j < (i-2)) && br; j++)
                    if (snginfo->w[j] != lw[j])
                        br = FALSE;
                if (br)
                    break;
            }
            if (!snginfo) {
                load_ng(lm, i, lw);
                snginfo=lm->nginfo[i-1][lw[i-2]];
            }
            snginfo->used = 1;
            
            if (snginfo->n_ng == 0)
                return;
            
            /* (i)grams for w1...w(i-1) now in memory; look for w1...w(i) */
            sng = snginfo->ng;
            assert(sng);
            sni = find_ng(sng, snginfo->n_ng, lw[i-1]);
            if (sni >= 0) {
                sng = sng + sni;
                snb = snb + sni ;
            }
            else
                return;
            /* snb   : (i)gram index of (i)gram w1...w(i)
			 sng32 : pointer to (i)gram w1...w(i)       */
        }
		
        if (sni >= 0)
            fnginfo->bowt = lm->ngbowt[N-1][sng->bowtid].l;
	}
    /* snb     : (N-1)gram index of (N-1)gram w1...w(N-1)
	 sng(32) : pointer to (N-1)gram w1...w(N-1)         */
	
    if (lm->isLM_IN_MEMORY) {
	/* RAH, already have this in memory */
		
        if (is32bits) {
			fnb = lm->ng_segbase[N-1][(snb) >> (lm->log_ng_seg_sz[N-2])]
			+ sng32->firstnng;
            /* fnb : N-gram absolute index of first N-gram
			 corresponding to w1...w(N-1) */
			
            if ((snb+1) < lm->n_ng[N-2])
                fnn = lm->ng_segbase[N-1][(snb+1) >> (lm->log_ng_seg_sz[N-2])]
				+ lm->ng32[N-2][snb+1].firstnng;
            else
                fnn = lm->n_ng[N-1];
			
			fnn -= fnb;
            /* fnn : number of N-grams that can correspond to w1...w(N-1) */
			
            /* store informations */
            fng32 = fnginfo32->ng32 = lm->ng32[N-1] + fnb;
			fnginfo32->n_ng = fnn ;
		}
		else {
			fnb = lm->ng_segbase[N-1][(snb) >> (lm->log_ng_seg_sz[N-2])]
			+ sng->firstnng;
            /* fnb : N-gram absolute index of first N-gram
			 corresponding to w1...w(N-1) */
			
            if ((snb+1) < lm->n_ng[N-2])
                fnn = lm->ng_segbase[N-1][(snb+1) >> (lm->log_ng_seg_sz[N-2])]
				+ lm->ng[N-2][snb+1].firstnng;
            else
                fnn = lm->n_ng[N-1];
			
			fnn -= fnb;
            /* fnn : number of N-grams that can correspond to w1...w(N-1) */
			
            /* store informations */
			fng = fnginfo->ng = lm->ng[N-1] + fnb;
			fnginfo->n_ng = fnn ;
		}
	} 
	else {
		if (is32bits) {
			fnb = lm->ng_segbase[N-1][(snb) >> (lm->log_ng_seg_sz[N-2])]
			+ sng32->firstnng;
            /* fnb : N-gram absolute index of first N-gram
			 corresponding to w1...w(N-1) */
			
			if ((snb+1) < lm->n_ng[N-2]) {
                /* finding on disk the (N-1)gram of (snb+1) index */
				if (fseeko(lm->fp, lm->ngoff[N-2] +((off_t) (snb+1)) * mem_sz_ng,
							SEEK_SET) < 0)
					E_FATAL_SYSTEM ("fseek failed\n");
				if (fread(&nextsng32, mem_sz_ng, 1, lm->fp) != (size_t)1)
					E_FATAL ("fread(nextsng32, %d at %d) failed\n",
                             1, lm->ngoff[N-2]);
				if (lm->byteswap) {
					SWAP_INT32(&(nextsng32.wid));
					SWAP_INT32(&(nextsng32.probid));
					SWAP_INT32(&(nextsng32.bowtid));
					SWAP_INT32(&(nextsng32.firstnng));
				}
				
				fnn = lm->ng_segbase[N-1][(snb+1) >> (lm->log_ng_seg_sz[N-2])]
				+ nextsng32.firstnng; 
			}
			else
				fnn = lm->n_ng[N-1];
			
			fnn -= fnb;
            /* fnn : number of N-grams that can correspond to w1...w(N-1) */
			
            /* store informations */
            fnginfo32->n_ng = fnn;
			
			if (fnn > 0) {
                /* allocate memory for the N-gram pointers */
				fng32 = fnginfo32->ng32 =
				(ng32_t *) ckd_calloc(fnn, mem_sz_ng);			
				
                /* load fnn N-grams from fnb index */
                if (N < lm->max_ng) {
                    if (fseeko (lm->fp, lm->ngoff[N-1] + ((off_t)(fnb)) * mem_sz_ng,
								SEEK_SET) < 0)
                        E_FATAL_SYSTEM("fseek failed\n");
                }
                else {
                    if (fseeko (lm->fp, lm->ngoff[N-1] + ((off_t)fnb)
								* SIZE_OF_NG32_WITHOUT_NNG, SEEK_SET) < 0)
                        E_FATAL_SYSTEM("fseek failed\n");
                }
                for (i = 0; i < fnn; i++) {
                    if (N < lm->max_ng) {
                        if (fread (&(fng32[i]), mem_sz_ng, 1, lm->fp)
                            != (size_t)1)
                            E_FATAL ("fread(fng32, %d at %d) failed\n",
                                     fnn, lm->ngoff[N-1]);
                    }
                    else {
                        if (fread (&(fng32[i]), SIZE_OF_NG32_WITHOUT_NNG,
                                   1, lm->fp) != (size_t)1)
                            E_FATAL ("fread(fng32, %d at %d) failed\n",
                                     fnn, lm->ngoff[N-1]);
                    }
					
                    if (lm->byteswap) {
						SWAP_INT32(&(fng32[i].wid));
						SWAP_INT32(&(fng32[i].probid));
                        SWAP_INT32(&(fng32[i].bowtid));
                        SWAP_INT32(&(fng32[i].firstnng));
					}
				}
			}
			else
				fnginfo32->ng32=NULL;
		}
		else {
			fnb = lm->ng_segbase[N-1][(snb) >> (lm->log_ng_seg_sz[N-2])]
			+ sng->firstnng;
            /* fnb : N-gram absolute index of first N-gram
			 corresponding to w1...w(N-1) */
			
			if ((snb+1) < lm->n_ng[N-2]) {
                /* finding on disk the (N-1)gram of (snb+1) index */
				if (fseeko (lm->fp, lm->ngoff[N-2] + ((off_t)(snb+1)) * mem_sz_ng,
							SEEK_SET) < 0)
					E_FATAL_SYSTEM ("fseek failed\n");
				if (fread (&nextsng, mem_sz_ng, 1, lm->fp) != (size_t)1)
					E_FATAL ("fread(nextsng, %d at %d) failed\n",
                             1, lm->ngoff[N-2]);
				if (lm->byteswap) {
					SWAP_INT16(&(nextsng.wid));
					SWAP_INT16(&(nextsng.probid));
					SWAP_INT16(&(nextsng.bowtid));
					SWAP_INT16(&(nextsng.firstnng));
				}
				
				fnn = lm->ng_segbase[N-1][(snb+1) >> (lm->log_ng_seg_sz[N-2])]
				+ nextsng.firstnng; 
			}
			else
				fnn = lm->n_ng[N-1];
			
			fnn-=fnb;
            /* fnn : number of N-grams that can correspond to w1...w(N-1) */
			
            /* store informations */
            fnginfo->n_ng=fnn;
			
			if (fnn > 0) {
                /* allocate memory for the N-gram pointers */
				fng = fnginfo->ng = (ng_t *) ckd_calloc(fnn, mem_sz_ng);			
                
                /* load fnn N-grams from fnb index */
                if (N < lm->max_ng) {
                    if (fseeko (lm->fp, lm->ngoff[N-1] + ((off_t)(fnb)) * mem_sz_ng,
								SEEK_SET) < 0)
                        E_FATAL_SYSTEM ("fseek failed\n");
                }
                else {
                    if (fseeko (lm->fp, lm->ngoff[N-1] + ((off_t)(fnb))
								* SIZE_OF_NG16_WITHOUT_NNG, SEEK_SET) < 0)
                        E_FATAL_SYSTEM ("fseek failed\n");
                }
                for (i = 0; i < fnn; i++) {
                    if (N < lm->max_ng) {
                        if (fread (&(fng[i]), mem_sz_ng, 1, lm->fp)
                            != (size_t)1)
                            E_FATAL ("fread(fng, %d at %d) failed\n",
                                     fnn, lm->ngoff[N-1]);
                    }
                    else {
                        if (fread (&(fng[i]), SIZE_OF_NG16_WITHOUT_NNG,
                                   1, lm->fp) != (size_t)1)
                            E_FATAL ("fread(fng, %d at %d) failed\n",
                                     fnn, lm->ngoff[N-1]);
                    }
					
                    if (lm->byteswap) {
						SWAP_INT16(&(fng[i].wid));
						SWAP_INT16(&(fng[i].probid));
                        SWAP_INT16(&(fng[i].bowtid));
                        SWAP_INT16(&(fng[i].firstnng));
					}
				}
			}
			else
				fnginfo->ng=NULL;
		}
	}
	
	lm->n_ng_fill[N-1]++;
	lm->n_ng_inmem[N-1] += fnn;
}

/* LIUM */
int32
lm_nglist(lm_t * lm, uint32 N, s3lmwid32_t *lw, ng_t ** ngptr, int32 * bowt)
/* N >= 2 */
{
    nginfo_t *nginfo, *prev_nginfo;
    uint32 i;
    int32 br;
	
    if ((N <= 1) || (lm->n_ng[N-1] <= 0)) {
        *ngptr = NULL;
        *bowt = 0;
        return 0;
    }
	
	for (i = 0; i < (N-1); i++)
		if (NOT_LMWID(lm, lw[i]) || (lw[i] >= lm->n_ng[0]))
			E_FATAL("Bad lw%d argument (%d) to lm_nglist\n", i+1, lw[i]);
	
	if (N == 2) {
	/* bigram */
		br = lm->ug[lw[0] + 1].firstbg - lm->ug[lw[0]].firstbg;
	
		if (br > 0) {
            if (!lm->membg[lw[0]].bg)
                load_bg(lm, lw[0]);
            lm->membg[lw[0]].used = 1;
			
            *ngptr = lm->membg[lw[0]].bg;
            *bowt = lm->ug[lw[0]].bowt.l;
        }
        else {
            *ngptr = NULL;
            *bowt = 0;
        }
		
        return (br);
    }
	else { 
	/* N >= 3 */
	/* trigram or more */
		
        prev_nginfo = NULL;

        for (nginfo = lm->nginfo[N-1][lw[N-2]]; nginfo; nginfo = nginfo->next) {
            br = TRUE;
            for (i = 0; (i < (N-2)) && br; i++)
                if (nginfo->w[i] != lw[i])
					br = FALSE;
            if (br)
                break;
            
            prev_nginfo = nginfo;
        }
		
        if (!nginfo) {
            load_ng(lm, N, lw);
            nginfo = lm->nginfo[N-1][lw[N-2]];
        }
        else if (prev_nginfo) {
            prev_nginfo->next = nginfo->next;
            nginfo->next = lm->nginfo[N-1][lw[N-2]];
            lm->nginfo[N-1][lw[N-2]] = nginfo;
        }
        nginfo->used = 1;
		
        *ngptr = nginfo->ng;
        *bowt = nginfo->bowt;
		
        return (nginfo->n_ng);
    }
}

/* LIUM */
int32
lm_ng32list(lm_t * lm, uint32 N, s3lmwid32_t *lw, ng32_t ** ngptr, int32 * bowt)
/* N >= 2 */
{
    nginfo32_t *nginfo32, *prev_nginfo32;
    uint32 i;
    int32 br;
	
    if ((N <= 1) || (lm->n_ng[N-1] <= 0)) {
        *ngptr = NULL;
        *bowt = 0;
        return 0;
    }
	
    for (i = 0; i < (N-1); i++)
        if (NOT_LMWID(lm, lw[i]) || (lw[i] >= lm->n_ng[0]))
            E_FATAL("Bad lw%d argument (%d) to lm_ng32list\n", i+1, lw[i]);
	
    if (N == 2) {
        /* bigram */
		
        br = lm->ug[lw[0] + 1].firstbg - lm->ug[lw[0]].firstbg;
		
        if (br > 0) {
            if (!lm->membg32[lw[0]].bg32)
                load_bg(lm, lw[0]);
            lm->membg32[lw[0]].used = 1;
			
            *ngptr = lm->membg32[lw[0]].bg32;
            *bowt = lm->ug[lw[0]].bowt.l;
        }
		else {
            *ngptr = NULL;
            *bowt = 0;
        }
		
        return (br);
    }
	else { 
	/* N >= 3 */
	/* trigram or more */
		
        prev_nginfo32 = NULL;
		
        for (nginfo32 = lm->nginfo32[N-1][lw[N-2]]; nginfo32;
             nginfo32 = nginfo32->next) {
            br = TRUE;
            for (i = 0; (i < (N-2)) && br; i++)
                if (nginfo32->w[i] != lw[i])
					br = FALSE;
            if (br)
                break;
            
            prev_nginfo32 = nginfo32;
        }
		
        if (!nginfo32) {
            load_ng(lm, N, lw);
            nginfo32 = lm->nginfo32[N-1][lw[N-2]];
        }
        else if (prev_nginfo32) {
            prev_nginfo32->next = nginfo32->next;
            nginfo32->next = lm->nginfo32[N-1][lw[N-2]];
            lm->nginfo32[N-1][lw[N-2]] = nginfo32;
        }
        nginfo32->used = 1;
		
        *ngptr = nginfo32->ng32;
        *bowt = nginfo32->bowt;
		
        return (nginfo32->n_ng);
    }
}

/* LIUM */
/*
 *  This function look-ups the N-gram score of p(lwN-1|lwN-2...lw1)
 *  and compute the in-class ug probability of wN.
 *  The information for lwN and wN are repeated because the legacy 
 *  implementation(since s3.2) of vithist used only LM wid rather 
 *  than dictionary wid.  
 */
int32
lm_ng_score(lm_t * lm, uint32 N, s3lmwid32_t *lw, s3wid_t wn)
{
    int32 i, n, score, test, h = 0;
	uint32 p, nd, npw;
    ng_t *ng;
    nginfo_t *nginfo, *prev_nginfo;
    ng32_t *ng32;
    nginfo32_t *nginfo32, *prev_nginfo32;
    int32 is32bits;
	
    ng = NULL;
    nginfo = prev_nginfo = NULL;
	
    ng32 = NULL;
    nginfo32 = prev_nginfo32 = NULL;
	
    is32bits = lm->is32bits;
	
	if (N >= 2) {
	/* bigram or more */
		
        if ((lm->n_ng[N-1] == 0) || (NOT_LMWID(lm, lw[0])))
            return (lm_ng_score(lm, N-1, &(lw[1]), wn));
		
        lm->n_ng_score[N-1]++;
    }
	
    for (p = 0; p < N; p++) {
        if (NOT_LMWID(lm, lw[p]) || (lw[p] >= lm->n_ng[0]))
        {
            if (N == 1) {
                E_WARN (
						"Mauvais word ID lw%d=BAD_LMWID (ou >= nombre d'unigrams), retourne MIN_PROB_F\n"
                        , p+1);
                return MIN_PROB_F;
            }
            if (p == 0) {
                if (N >= 3)
                    E_FATAL ("Bad lw%d argument (%d) to lm_ng_score\n",
                             p+1, lw[p]);
                else
                    E_WARN (
							"Mauvais word ID lw%d=BAD_LMWID (ou >= nombre d'unigrams), continue\n"
                            , p+1);
            }
            else {
                if (p >= (N-2)) {
                    E_WARN (
							"Mauvais word ID lw%d=BAD_LMWID (ou >= nombre d'unigrams), retourne le score de l'unigram lw%d\n"
                            , p+1, N);
                    return lm_ng_score (lm, 1, &(lw[N-1]), wn);
                }
                else {
                    E_WARN (
							"Mauvais word ID lw%d=BAD_LMWID (ou >= nombre d'unigrams), retourne le score du %dgram lw%d...lw%d\n"
                            , p+1, N-1-p, 2+p, N);
                    return lm_ng_score (lm, N-1-p, &(lw[1+p]), wn);
                }
            }
        }
	}
	
	if (N >= 3) {
	/* trigram or more */
		
        /* Lookup ngcache first; compute hash(lw1...lwN) */
		
        npw = (N<31)?31/N:1; /* number of bit per word */
        nd = npw+31%npw; /* added 31%npw for use all bits */
        
        /* compute hash with lwN */
        h = lw[N-1] & ((1<<(npw+1))-1);
		
        /* compute hash with lw1...lwN-1 */
        for (p = (N-2); nd < 31; p--) {
            h += ((lw[p] & ((1<<npw)-1)) << nd);
            nd += npw;
        }
		
        h %= LM_NGCACHE_SIZE;
		
		
        if (is32bits) {
            test = TRUE;
            for (p = 0; (p < N) && test; p++)
                if (lm->ngcache32[N-1][h].lwid[p] != lw[p])
                    test = FALSE;
            if (test) {
                lm->n_ngcache_hit[N-1]++;
                return lm->ngcache32[N-1][h].lscr;
            }
			
            prev_nginfo32 = NULL;
            for (nginfo32 = lm->nginfo32[N-1][lw[N-2]]; nginfo32;
                 nginfo32 = nginfo32->next) {
				
                test = TRUE;
                for (p = 0; (p < (N-2)) && test; p++)
                    if (nginfo32->w[p] != lw[p])
                        test = FALSE;
                if (test)
                    break;
				
                prev_nginfo32 = nginfo32;
            }
        }
        else {
            test = TRUE;
            for (p = 0; (p < N) && test; p++)
                if (lm->ngcache[N-1][h].lwid[p] != lw[p])
                    test = FALSE;
            if (test) {
                lm->n_ngcache_hit[N-1]++;
                return lm->ngcache[N-1][h].lscr;
            }
			
            prev_nginfo = NULL;
            for (nginfo = lm->nginfo[N-1][lw[N-2]]; nginfo;
                 nginfo = nginfo->next) {
				
                test = TRUE;
                for (p = 0; (p < (N-2)) && test; p++)
                    if (nginfo->w[p] != lw[p])
                        test = FALSE;
                if (test)
                    break;
				
                prev_nginfo = nginfo;
            }
        }
		
        if (is32bits) {
            if (!nginfo32) {
                /* w1...wN-1 not found, load it at start of list */
                load_ng(lm, N, lw);
                nginfo32 = lm->nginfo32[N-1][lw[N-2]];
            }
            else if (prev_nginfo32) {
                /* w1...wN-1 is found, put it at start of list */
                prev_nginfo32->next = nginfo32->next;
                nginfo32->next = lm->nginfo32[N-1][lw[N-2]];
                lm->nginfo32[N-1][lw[N-2]] = nginfo32;
            }
            nginfo32->used = 1;
        }
        else {
            if (!nginfo) {
                /* w1...wN-1 not found, load it at start of list */
                load_ng(lm, N, lw);
                nginfo = lm->nginfo[N-1][lw[N-2]];
            }
            else if (prev_nginfo) {
                /* w1...wN-1 is found, put it at start of list */
                prev_nginfo->next = nginfo->next;
                nginfo->next = lm->nginfo[N-1][lw[N-2]];
                lm->nginfo[N-1][lw[N-2]] = nginfo;
            }
            nginfo->used = 1;
        }
		
		
        /* N-grams for w1...wN-1 now in memory; look for w1...wN */
        if (is32bits) {
            n = nginfo32->n_ng;
            ng32 = nginfo32->ng32;
            assert(nginfo32);
        }
        else {
            n = nginfo->n_ng;
            ng = nginfo->ng;
            assert(nginfo);
        }
    }
    else {
        if (N == 2) {
		/* bigram */
			
            n = lm->ug[lw[0] + 1].firstbg - lm->ug[lw[0]].firstbg;
			
            if (n > 0) {
                if (is32bits) {
                    if (!lm->membg32[lw[0]].bg32)
                        load_bg(lm, lw[0]);
                    lm->membg32[lw[0]].used = 1;
                    ng32 = lm->membg32[lw[0]].bg32;
                }
                else {
                    if (!lm->membg[lw[0]].bg)
                        load_bg(lm, lw[0]);
                    lm->membg[lw[0]].used = 1;
                    ng = lm->membg[lw[0]].bg;
                }
            }
        }
        else { 
		/* N == 1 */
		/* unigram */
			
            lm->access_type = 1;
			
            if (lm->inclass_ugscore)
                return (lm->ug[lw[0]].prob.l + lm->inclass_ugscore[lw[0]]);
            else
                return (lm->ug[lw[0]].prob.l);
        }
    }
	
    if (is32bits)
        i = find_ng32(ng32, n, lw[N-1]);
    else
        i = find_ng(ng, n, lw[N-1]);
	
    if (i >= 0) {
        if (is32bits)
            score = lm->ngprob[N-1][ng32[i].probid].l;
        else
            score = lm->ngprob[N-1][ng[i].probid].l;
		
        if (lm->inclass_ugscore) {
            /* Only add within class probability if class information exists.
			 Is actually ok to just add the score because if the word
			 is not within-class. The returning scores will be 0. */
            score += lm->inclass_ugscore[wn];
        }
        lm->access_type = N;
    }
    else {
        lm->n_ng_bo[N-1]++;
        
        if (N >= 3) {
		/* trigram or more */
            score = is32bits ? nginfo32->bowt : nginfo->bowt;
            score += lm_ng_score(lm, N-1, &(lw[1]), wn);
        }
        else {
		/* bigram */
            lm->access_type = 1;
            score = lm->ug[lw[0]].bowt.l + lm->ug[lw[1]].prob.l;
        }
    }
	
    if (N >= 3) {
	/* trigram or more */
        if (is32bits) {
            for (p = 0; p < N; p++)
                lm->ngcache32[N-1][h].lwid[p] = lw[p];
            lm->ngcache32[N-1][h].lscr = score;
        }
        else {
            for (p = 0; p < N; p++)
                lm->ngcache[N-1][h].lwid[p] = lw[p];
            lm->ngcache[N-1][h].lscr = score;
        }
    }
	
    return (score);
}

/* LIUM */
int32
lm_ng_exists(lm_t * lm, uint32 N, s3lmwid32_t *lw)
{
    int32 i;
	uint32 n, test;
    ng_t *ng;
    nginfo_t *nginfo, *prev_nginfo;
    ng32_t *ng32;
    nginfo32_t *nginfo32, *prev_nginfo32;
	
    int32 is32bits;
	
    ng = NULL;
    nginfo = prev_nginfo = NULL;
    ng32 = NULL;
    nginfo32 = prev_nginfo32 = NULL;
    is32bits = lm->is32bits;
	
    if ((N >= 2) && ((lm->n_ng[N-1] == 0) || (NOT_LMWID(lm, lw[0]))))
	/* bigram or more */
        return 0;
	
    for (n = 0; n < N; n++)
        if (NOT_LMWID(lm, lw[n]) || (lw[n] >= lm->n_ng[0]))
            return 0;
	
    if (N >= 3) {
	/* trigram or more */
		
        if (is32bits) {
            prev_nginfo32 = NULL;
            for (nginfo32 = lm->nginfo32[N-1][lw[N-2]]; nginfo32;
                 nginfo32 = nginfo32->next) {
				
                test = TRUE;
                for (n = 0; (n < (N-2)) && test; n++)
                    if (nginfo32->w[n] != lw[n])
                        test = FALSE;
                if (test)
                    break;
				
                prev_nginfo32 = nginfo32;
            }
        }
        else {
            prev_nginfo = NULL;
            for (nginfo = lm->nginfo[N-1][lw[N-2]]; nginfo;
                 nginfo = nginfo->next) {
				
                test = TRUE;
                for (n = 0; (n < (N-2)) && test; n++)
                    if (nginfo->w[n] != lw[n])
                        test = FALSE;
                if (test)
                    break;
				
                prev_nginfo = nginfo;
            }
        }
		
        if (is32bits) {
            if (!nginfo32) {
                load_ng(lm, N, lw);
                nginfo32 = lm->nginfo32[N-1][lw[N-2]];
            }
            else if (prev_nginfo32) {
                prev_nginfo32->next = nginfo32->next;
                nginfo32->next = lm->nginfo32[N-1][lw[N-2]];
                lm->nginfo32[N-1][lw[N-2]] = nginfo32;
            }
            nginfo32->used = 1;
            /* N-grams for w1...wN-1 now in memory; look for w1...wN */
            n = nginfo32->n_ng;
            ng32 = nginfo32->ng32;
            assert(nginfo32);
        }
        else {
            if (!nginfo) {
                load_ng(lm, N, lw);
                nginfo = lm->nginfo[N-1][lw[N-2]];
            }
            else if (prev_nginfo) {
                prev_nginfo->next = nginfo->next;
                nginfo->next = lm->nginfo[N-1][lw[N-2]];
                lm->nginfo[N-1][lw[N-2]] = nginfo;
            }
            nginfo->used = 1;
            /* N-grams for w1...wN-1 now in memory; look for w1...wN */
            n = nginfo->n_ng;
            ng = nginfo->ng;
            assert(nginfo);
        }
    }
    else {
        if (N == 2) {
		/* bigram */
			
            n = lm->ug[lw[0] + 1].firstbg - lm->ug[lw[0]].firstbg;
			
            if (n > 0) {
                if (is32bits) {
                    if (!lm->membg32[lw[0]].bg32)
                        load_bg(lm, lw[0]);
                    lm->membg32[lw[0]].used = 1;
                    ng32 = lm->membg32[lw[0]].bg32;
                }
                else {
                    if (!lm->membg[lw[0]].bg)
                        load_bg(lm, lw[0]);
                    lm->membg[lw[0]].used = 1;
                    ng = lm->membg[lw[0]].bg;
                }
            }
        }
        else 
		/* N == 1 */
		/* unigram */
            return 1;
    }
	
    if (is32bits)
        i = find_ng32(ng32, n, lw[N-1]);
    else
        i = find_ng(ng, n, lw[N-1]);
	
    if (i >= 0)
        return 1;
    else
        return 0;
}

/* LIUM */
s3lmwid32_t
lm_wid(lm_t * lm, const char *word)
{
    int32 i;
	
    for (i = 0; i < lm->n_ng[0]; i++)
        if (strcmp(lm->wordstr[i], word) == 0)
            return ((s3lmwid32_t) i);
	
    return BAD_LMWID(lm);
}

/* LIUM */
void
lm_free(lm_t * lm)
{
    uint32 N, i;
    nginfo_t *nginfo;
    nginfo32_t *nginfo32;
	
    if (lm->fp)
        fclose(lm->fp);
	
	if (lm->ug != NULL)
		ckd_free((void *) lm->ug);
	
	if (lm->wordstr != NULL) {
        if (lm->n_ng != NULL)
			for (i = 0; i < lm->n_ng[0]; i++)
				ckd_free((void *) lm->wordstr[i]);      /*  */
		ckd_free((void *) lm->wordstr);
	}
	
    if ((lm->n_ng != NULL) && (lm->n_ng[1] > 0)) {
        if ((lm->ng && lm->ng[1]) || (lm->ng32 && lm->ng32[1])) { /* Memory-based;
																   free all bigrams */
            if (lm->ng && lm->ng[1])
                ckd_free(lm->ng[1]);
            if (lm->ng32 && lm->ng32[1])
                ckd_free(lm->ng32[1]);
			
            if (lm->membg)
                ckd_free(lm->membg);
            if (lm->membg32)
                ckd_free(lm->membg32);
        }
        else {                  /* Disk-based; free in-memory bg */
            if (lm->membg) {
                for (i = 0; i < lm->n_ng[0]; ++i)
                    ckd_free(lm->membg[i].bg);
                ckd_free(lm->membg);
            }
            if (lm->membg32) {
                for (i = 0; i < lm->n_ng[0]; ++i)
                    ckd_free(lm->membg32[i].bg32);
                ckd_free(lm->membg32);
            }
        }
		
		if ((lm->ngprob != NULL) && (lm->ngprob[1] != NULL))
			ckd_free(lm->ngprob[1]);
    }
	
    if (lm->n_ng != NULL)
        for (N = 3; N <= lm->max_ng; N++)
            if (lm->n_ng[N-1] > 0) {
                if (lm->ng && lm->ng[N-1])
                    ckd_free((void *) lm->ng[N-1]);
                if (lm->ng32 && lm->ng32[N-1])
                    ckd_free((void *) lm->ng32[N-1]);
				
                if (lm->nginfo && lm->nginfo[N-1]) {
                    for (i = 0; i < lm->n_ng[0]; i++) {
                        if (lm->nginfo[N-1][i] != NULL) {
                            /* Free the whole linked list of nginfo */
                            while (lm->nginfo[N-1][i]) {
                                nginfo = lm->nginfo[N-1][i];
                                lm->nginfo[N-1][i] = nginfo->next;
                                if (    (!lm->isLM_IN_MEMORY)
									&& (nginfo->ng != NULL) )
                                    ckd_free(nginfo->ng);
                                if (nginfo->w != NULL)
                                    ckd_free(nginfo->w);
                                ckd_free((void *) nginfo);
                            }
                        }
                    }
                    ckd_free((void *) lm->nginfo[N-1]);
                }
                if (lm->nginfo32 && lm->nginfo32[N-1]) {
                    for (i = 0; i < lm->n_ng[0]; i++) {
                        if (lm->nginfo32[N-1][i] != NULL) {
                            while (lm->nginfo32[N-1][i]) {
                                nginfo32 = lm->nginfo32[N-1][i];
                                lm->nginfo32[N-1][i] = nginfo32->next;
                                if (    (!lm->isLM_IN_MEMORY)
									&& (nginfo32->ng32 != NULL) )
                                    ckd_free(nginfo32->ng32);
                                if (nginfo32->w != NULL)
                                    ckd_free(nginfo32->w);
                                ckd_free((void *) nginfo32);
                            }
                        }
                    }
                    ckd_free((void *) lm->nginfo32[N-1]);
                }
				
				
                if (lm->ngcache && lm->ngcache[N-1]) {
                    for (i = 0; i < LM_NGCACHE_SIZE; i++)
                        if (lm->ngcache[N-1][i].lwid != NULL)
                            ckd_free((void *) lm->ngcache[N-1][i].lwid);
                    ckd_free((void *) lm->ngcache[N-1]);
                }
                if (lm->ngcache32 && lm->ngcache32[N-1]) {
                    for (i = 0; i < LM_NGCACHE_SIZE; i++)
                        if (lm->ngcache32[N-1][i].lwid != NULL)
                            ckd_free((void *) lm->ngcache32[N-1][i].lwid);
                    ckd_free((void *) lm->ngcache32[N-1]);
                }
				
                if ( (lm->ng_segbase != NULL) && (lm->ng_segbase[N-1] != NULL) )
                    ckd_free((void *) lm->ng_segbase[N-1]);
                if ( (lm->ngprob != NULL) && (lm->ngprob[N-1] != NULL) )
                    ckd_free((void *) lm->ngprob[N-1]);
                if ( (lm->ngbowt != NULL) && (lm->ngbowt[N-1] != NULL) )
                    ckd_free((void *) lm->ngbowt[N-1]);
            }
    
    if(lm->n_ng) ckd_free((void *) lm->n_ng);
    if(lm->ng) ckd_free((void *) lm->ng);
    if(lm->ng32) ckd_free((void *) lm->ng32);
    if(lm->nginfo) ckd_free((void *) lm->nginfo);
    if(lm->nginfo32) ckd_free((void *) lm->nginfo32);
    if(lm->ngcache) ckd_free((void *) lm->ngcache);
    if(lm->ngcache32) ckd_free((void *) lm->ngcache32);
    if(lm->ng_segbase) ckd_free((void *) lm->ng_segbase);
    if(lm->ngprob) ckd_free((void *) lm->ngprob);
    if(lm->ngbowt) ckd_free((void *) lm->ngbowt);
    if(lm->ngoff) ckd_free((void *) lm->ngoff);
    if(lm->n_ng_fill) ckd_free((void *) lm->n_ng_fill);
    if(lm->n_ng_inmem) ckd_free((void *) lm->n_ng_inmem);
    if(lm->n_ng_score) ckd_free((void *) lm->n_ng_score);
    if(lm->n_ng_bo) ckd_free((void *) lm->n_ng_bo);
    if(lm->n_ngcache_hit) ckd_free((void *) lm->n_ngcache_hit);
    if(lm->sorted_probn) ckd_free((void *) lm->sorted_probn);
    if(lm->sorted_bowtn) ckd_free((void *) lm->sorted_bowtn);
	
    if (lm->dict2lmwid) {
        ckd_free(lm->dict2lmwid);
    }
	
#ifndef _READ_DUMP_ONLY_
    if (lm->HT) {
        hash_table_free(lm->HT);
    }
#endif
	
    ckd_free((void *) lm);
}

/* LIUM */
void
copy_ng_to_ng32(lm_t * lm, uint32 N)
{
    int i;
    assert(lm->ng32[N-1] == NULL);
    lm->ng32[N-1] = (ng32_t *) ckd_calloc(lm->n_ng[N-1] + 1, sizeof(ng32_t));
	
    for (i = 0; i <= lm->n_ng[N-1]; i++)
        copy_ngt_to_ng32t(&(lm->ng[N-1][i]), &(lm->ng32[N-1][i]));
}

/* LIUM */
void
copy_ng32_to_ng(lm_t * lm, uint32 N)
{
    int i;
    assert(lm->ng[N-1] == NULL);
    lm->ng[N-1] = (ng_t *) ckd_calloc(lm->n_ng[N-1] + 1, sizeof(ng_t));
	
    for (i = 0; i <= lm->n_ng[N-1]; i++)
        copy_ng32t_to_ngt(&(lm->ng32[N-1][i]), &(lm->ng[N-1][i]));
}

/* LIUM */
void
swap_ng(ng_t * n16)
{
    SWAP_INT16(&(n16->wid));
    SWAP_INT16(&(n16->probid));
    SWAP_INT16(&(n16->bowtid));
    SWAP_INT16(&(n16->firstnng));
}

/* LIUM */
void
swap_ng32(ng32_t * n32)
{
    SWAP_INT32(&(n32->wid));
    SWAP_INT32(&(n32->probid));
    SWAP_INT32(&(n32->bowtid));
    SWAP_INT32(&(n32->firstnng));
}

/* LIUM */
int32
lm_rawscore(lm_t * lm, int32 score)
{
    score -= lm->wip;
    score = (int32) (score/ ((float) lm->lw));
	
    return score;
}

/* LIUM */
void
lm_convert_structure(lm_t * model, int32 is32bits)
{
	uint32 N;
	
    /* Convert the data structure */
    if (is32bits) {             /* Convert from 16 bits to 32 bits */
        if (model->ng32 == NULL)
            model->ng32 =
			(ng32_t **) ckd_calloc (model->max_ng, sizeof(ng32_t *));
		
        if (model->ng32 != NULL) {
            for (N = 2; N <= model->max_ng; N++) {
                if (model->n_ng[N-1] > 0) {
                    if (model->ng32[N-1] == NULL) {
                        assert(model->ng[N-1] != NULL);
                        copy_ng_to_ng32(model, N);
                    }
                }
			}
		}
    }
    else {                      /* Convert from 32 bits to 16 bits */
        if (model->ng == NULL)
            model->ng = (ng_t **) ckd_calloc (model->max_ng, sizeof(ng_t *));
		
        if (model->ng != NULL) {
            for (N = 2; N <= model->max_ng; N++) {
                if (model->n_ng[N-1] > 0) {
                    if (model->ng[N-1] == NULL) {
                        assert(model->ng32[N-1] != NULL);
                        copy_ng32_to_ng(model, N);
                    }
                }
			}
		}
    }
	
    if (is32bits) {
        assert(model->ng32 != NULL);
        for (N = 2; N <= model->max_ng; N++)
            if (model->n_ng[N-1] > 0)
                assert(model->ng32[N-1] != NULL);
    }
    else {
        assert(model->ng != NULL);
        for (N = 2; N <= model->max_ng; N++)
            if (model->n_ng[N-1] > 0)
                assert(model->ng[N-1] != NULL);
    }
}

/* LIUM */
#ifndef _READ_DUMP_ONLY_

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

/* LIUM */
/*
  INCOMPLETE
 */
int32
lm_add_word_to_ug(lm_t * lm,      /**<In/Out: a modified LM structure */
                  dict_t * dict,      /**< In: an initialized dictionary structure */
                  const char *newword       /**< In: a new word */
    )
{
	uint32 n;
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
      Update the value lm->n_ng[0], lm->max_ug;
   */

    if (hash_table_lookup(lm->HT, newword, &id) == 0) {
        E_WARN("The word %s already exists in the language model \n",
               newword);
        return LM_FAIL;
    }

    lm->n_ng[0] = lm->n_ng[0] + 1;
    lm->max_ug = lm->n_ng[0];

    E_INFO("lm->n_ng[0] %d\n", lm->n_ng[0]);
    lm->ug = (ug_t *) ckd_realloc(lm->ug, (lm->n_ng[0] + 1) * sizeof(ug_t));       /* Yes, +2 look at NewUnigramModel(n_ug+1) */
    lm->wordstr =
        (char **) ckd_realloc(lm->wordstr, (lm->n_ng[0]) * sizeof(char *));

  /** Reallocate the size of lm->membg 
      and lm->tginfo
  */

    if (!lm->is32bits) {
        lm->membg =
            (membg_t *) ckd_realloc(lm->membg,
                                    (lm->n_ng[0]) * sizeof(membg_t));
		
		for (n = 3; n <= lm->max_ng; n++) {
			lm->nginfo[n-1] =
				(nginfo_t **) ckd_realloc(lm->nginfo[n-1],
                                      (lm->n_ng[0]) * sizeof(nginfo_t *));
			lm->nginfo[n-1][lm->n_ng[0] - 1] = NULL;
		}
    }
    else {
        lm->membg32 =
            (membg32_t *) ckd_realloc(lm->membg32,
                                      (lm->n_ng[0]) * sizeof(membg32_t));
		
		for (n = 3; n <= lm->max_ng; n++) {
			lm->nginfo32[n-1] =
				(nginfo32_t **) ckd_realloc(lm->nginfo32[n-1],
											(lm->n_ng[0]) * sizeof(nginfo32_t *));
			lm->nginfo32[n-1][lm->n_ng[0] - 1] = NULL;
		}
    }


    E_WARN("Invoke incomplete lm_add_word_to_ug\n");

  /** Insert the entry into lm->ug and lm->wordstr */

    /* 
       This part is not compeleted, prob.f should be the second best
       unigram probability.  This is a fairly standard that was used by
       Dragon and also recommended by Roni. 
     */

    lm->ug[lm->n_ng[0]].prob.f = -99.0;
    lm->ug[lm->n_ng[0]].bowt.f = -99.0;
    lm->ug[lm->n_ng[0]].dictwid = lm->n_ng[0];/* See the comment in ug_t, this is not exactly correct
												externally application needs to set it again. */

    /* Supposingly, the bigram should follow the unigram order.
       Because, we have no bigram inserted in this case, the
       unigram.firstbg will just follow the previous one.  */

    lm->ug[lm->n_ng[0]].firstbg = lm->ug[lm->n_ng[0] - 1].firstbg;

    lm->wordstr[lm->n_ng[0] - 1] = (char *) ckd_salloc(newword);

    hash_table_enter(lm->HT, lm->wordstr[lm->n_ng[0] - 1], (void *)(long)(lm->n_ng[0] - 1));

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
                        lm_wordstr(lm, lm->n_ng[0] - 1));
                return LM_FAIL;
            }
            else {
                if (dict_filler_word(dict, w))
                    E_ERROR("Filler dictionary word '%s' found in LM\n",
                            lm_wordstr(lm, lm->n_ng[0] - 1));

                if (w != dict_basewid(dict, w)) {
                    E_ERROR
                        ("LM word '%s' is an alternative pronunciation in dictionary\n",
                         lm_wordstr(lm, lm->n_ng[0] - 1));

                    w = dict_basewid(dict, w);
                    lm_lmwid2dictwid(lm, lm->n_ng[0] - 1) = w;
                }

                for (; IS_S3WID(w); w = dict_nextalt(dict, w))
                    lm->dict2lmwid[w] = (s3lmwid32_t) (lm->n_ng[0] - 1);
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
#endif

#if (_LM_TEST_)
main(int32 argc, char *argv[])
{
    char line[4096];
    int32 score, k;
    lm_t *lm;

    if (argc < 2)
        E_FATAL("Usage: %s <LMdumpfile>\n", argv[0]);

    //logs3_init(1.0001, 1, 1);
    lm = lm_read(argv[1], 9.5, 0.2, 0.7);

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
