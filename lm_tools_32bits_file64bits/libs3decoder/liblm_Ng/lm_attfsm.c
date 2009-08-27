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
  This file is contributed by Laboratoire d'informatique Universite du
  Maine (LIUM) by Prof.  Yannick Esteve. 
 */


/** \file lm_attfsm.c
    \brief Language model dumping in FSM format

	Mainly adapted from LIUM Sphinx's modification.
	Also doesn't work for class-based LM and bigram at this point.
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
 * $Log: lm_attfsm.c,v $
 * 2008/06/27  N. Coetmeur, supervised by Y. Esteve
 * Adjust comments for compatibility with Doxygen 1.5.6
 *
 * 2008/06/20  N. Coetmeur, supervised by Y. Esteve
 * Some changes for compatibility with new lm.h file.
 *
 * Revision 1.2  2006/02/23 04:08:36  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH
 * 1, Added lm_3g.c - a TXT-based LM routines.
 * 2, Added lm_3g_dmp.c - a DMP-based LM routines.
 * 3, (Contributed by LIUM) Added lm_attfsm.c - convert LM to FSM
 * 4, Added lmset.c - a wrapper for the lmset_t structure.
 *
 * Revision 1.1.2.1  2006/01/16 19:57:28  arthchan2003
 * Added LIUM's ATT FSM generation routine.
 *
 *
 */

#include "lm.h"


/**
 \brief Return absolute first trigram index for bigram (i, bgptr[j].wid)
 \return First trigram index
*/
static int32
idx_tg_hist	(	lm_t * lm,		/**< In: language model */
				ng_t * bgptr,	/**< In: bigrams list */
				int32 	i,		/**< In: index of first word from the bigram */
				int32 	j		/**< In: index of the bigram in the list */
				)
{
    int32 b;

    b = lm->ug[i].firstbg + j;
    return (lm->ng_segbase[2][b >> (lm->log_ng_seg_sz[1])] + bgptr[j].firstnng);
}

/**
 \brief Return absolute first trigram index for 32-bits bigram (i, bgptr[j].wid)
 \return First trigram index
*/
static int32
idx_tg32_hist	( lm_t 		* 	lm,		/**< In: language model */
				  ng32_t 	* 	bgptr,	/**< In: 32-bits bigrams list */
				  int32 		i,		/**< In: index of first word
															  from the bigram */
				  int32 		j		/**< In: index of the bigram
																  in the list */
				  )
{
    int32 b;

    b = lm->ug[i].firstbg + j;
    return (lm->ng_segbase[2][b >> (lm->log_ng_seg_sz[1])] + bgptr[j].firstnng);
}

/*
   Write a LM in FST format
*/
int32
lm_write_att_fsm(lm_t * lm, const char *filename)
{
    FILE *file;
    int32 i, j, k, l, nb_bg, nb_bg2, nb_tg, bowt;
    char filesymbolsname[2048];

    ng_t *bgptr, *bgptr2;
    ng_t *tgptr;

    ng32_t *bgptr32, *bgptr32_2;
    ng32_t *tgptr32;
    int32 is32bits;
    s3lmwid32_t wid32_bg, wid32_tg;
    s3lmwid32_t wid32_ng[2];

    short prune_lowprobtg = 0;

    int32 st_ug = lm_n_ng(lm,1) + 1, st_end = lm_n_ng(lm,1) + 2;
		/* unigram state, end state */
    int32 nbwtg = lm_n_ng(lm,1) + 3;
		/* Number of id states used without trigrams */
    int32 id_hist1, id_hist2; /* Indexes of trigram histories */

    is32bits = lm->is32bits;
    sprintf(filesymbolsname, "%s.sym", filename);

    if (!(file = fopen(filesymbolsname, "w")))
        E_FATAL("fopen(%s,w) failed\n", file);
    fprintf(file, "<eps>\t0\n");
    for (i = 0; i < lm_n_ng(lm,1); i++)
        fprintf(file, "%s\t%d\n", lm->wordstr[i], i + 1);
			/* i+1 because id of <eps> HAS TO be EQUAL to 0 */
    fclose(file);

    if (!(file = fopen (filename, "w")))
        E_FATAL ("fopen(%s,w) failed\n", file);

    if (lm_n_ng (lm,1) <= 0)
        E_FATAL("ngram1=%d", lm_n_ng (lm,1));


    /* Start state */

    for (i = 0; i < lm_n_ng(lm,1); i++) {
        if (i % 1000 == 0)
            fprintf (stderr, ".");

        if (i == lm->finishlwid) {
            fprintf (file, "%d\t%d\t%d\t%f\n", st_ug, st_end,
                    lm->finishlwid + 1, -lm->ug[i].prob.f);
        }
        else {
            if (i != lm->startlwid) {
                fprintf (file, "%d\t%d\t%d\t%f\n",
						 st_ug, i, i + 1, -lm->ug[i].prob.f); /* 1g->2g */
            }
            fprintf (file, "%d\t%d\t0\t%f\n",
					 i, st_ug, -lm->ug[i].bowt.f); /* 2g->1g */


            nb_bg = is32bits ?
				      lm_ng32list(lm, 2, (s3lmwid32_t *)&i, &bgptr32, &bowt)
				    : lm_nglist(lm, 2, (s3lmwid32_t *)&i, &bgptr, &bowt);
						/* bowt not used... */

            for (j = 0; j < nb_bg; j++) {

                wid32_bg = is32bits ? bgptr32[j].wid : bgptr[j].wid;
                if (wid32_bg != lm->finishlwid) {

                    /* 32/16 bits code */
                    id_hist1 = is32bits ? idx_tg32_hist(lm, bgptr32, i, j)
                        : idx_tg_hist(lm, bgptr, i, j);

					wid32_ng[0]=i;
					wid32_ng[1]=
						is32bits
						? bgptr32[j].wid
						: (s3lmwid32_t)bgptr[j].wid;
                    nb_tg =
                        is32bits
                        ? lm_ng32list(lm, 3, wid32_ng, &tgptr32, &bowt)
                        : lm_nglist(lm, 3, wid32_ng, &tgptr, &bowt);

                    if (nb_tg > 0) {
                        if (is32bits)
                            fprintf (file, "%d\t%d\t%d\t%f\n", i,
									 id_hist1 + nbwtg, bgptr32[j].wid + 1,
									 -lm->ngprob[1][bgptr32[j].probid].f);
										/* 2g->3g */
                        else
                            fprintf (file, "%d\t%d\t%d\t%f\n", i,
									 id_hist1 + nbwtg, bgptr[j].wid + 1,
									 -lm->ngprob[1][bgptr[j].probid].f);
										/* 2g->3g */
                    }
                    if (is32bits)
                        fprintf (file, "%d\t%d\t0\t%f\n", id_hist1 + nbwtg,
								 bgptr32[j].wid,
								 -lm->ngbowt[2][bgptr32[j].bowtid].f);
									/* 3g->2g */
                    else
                        fprintf (file, "%d\t%d\t0\t%f\n", id_hist1 + nbwtg,
								 bgptr[j].wid,
								 -lm->ngbowt[2][bgptr[j].bowtid].f);
									/* 3g->2g */

                    for (k = 0; k < nb_tg; k++) {       /* 3g->3g */

                        wid32_tg =
                            is32bits ? tgptr32[k].wid : tgptr[k].wid;

                        if (wid32_tg == lm->finishlwid) {
                            if (is32bits)
                                fprintf(file, "%d\t%d\t%d\t%f\n",
                                        id_hist1 + nbwtg, st_end,
                                        tgptr32[k].wid + 1,
                                        -lm->ngprob[2][tgptr32[k].probid].f);
                            else
                                fprintf(file, "%d\t%d\t%d\t%f\n",
                                        id_hist1 + nbwtg, st_end,
                                        tgptr[k].wid + 1,
                                        -lm->ngprob[2][tgptr[k].probid].f);
                        }
                        else {
                            /* bowt not used... */

                            /* 32/16 bits code */
							wid32_bg = is32bits ? bgptr32[j].wid : bgptr[j].wid;
                            nb_bg2 =
                                is32bits ? lm_ng32list(lm, 2, &wid32_bg,
                                                       &bgptr32_2, &bowt) :
                                lm_nglist(lm, 2, &wid32_bg, &bgptr2, &bowt);
                            l = is32bits ? find_ng32(bgptr32_2, nb_bg2,
                                                     tgptr32[k].
                                                     wid) : find_ng(bgptr2,
                                                                    nb_bg2,
                                                                    tgptr
                                                                    [k].
                                                                    wid);

                            if (l > -1) {
                                if (tgptr[k].wid != lm->finishlwid) {

                                    id_hist2 =
                                        is32bits ? idx_tg32_hist(lm,
                                                                 bgptr32_2,
                                                                 bgptr32
                                                                 [j].wid,
                                                                 l) :
                                        idx_tg_hist(lm, bgptr2,
                                                    bgptr[j].wid, l);

                                    if (is32bits) {
										wid32_ng[0]=bgptr32[j].wid;
										wid32_ng[1]=tgptr32[k].wid;

                                        if (prune_lowprobtg) {
                                            if (lm->
                                                ngprob[2][tgptr32[k].probid].
                                                f >
                                                (lm->
                                                 ngbowt[2][bgptr32[j].bowtid].
                                                 f + lm_ng_score(lm,2,wid32_ng,
                                                                 0))) {
                                                fprintf(file,
                                                        "%d\t%d\t%d\t%f\n",
                                                        id_hist1 + nbwtg,
                                                        id_hist2 + nbwtg,
                                                        tgptr32[k].wid + 1,
                                                        -lm->
                                                        ngprob[2][tgptr32[k].
                                                               probid].f);
                                            }
                                        }
                                        else {
                                            fprintf(file,
                                                    "%d\t%d\t%d\t%f\n",
                                                    id_hist1 + nbwtg,
                                                    id_hist2 + nbwtg,
                                                    tgptr32[k].wid + 1,
                                                    -lm->ngprob[2][tgptr32[k].
                                                                probid].f);
                                        }
                                    }
                                    else {
										wid32_ng[0]=bgptr[j].wid;
										wid32_ng[1]=tgptr[k].wid;

                                        if (prune_lowprobtg) {
                                            if (lm->
                                                ngprob[2][tgptr[k].probid].f >
                                                (lm->
                                                 ngbowt[2][bgptr[j].bowtid].
                                                 f + lm_ng_score(lm,2,wid32_ng,
                                                                 0))) {
                                                fprintf(file,
                                                        "%d\t%d\t%d\t%f\n",
                                                        id_hist1 + nbwtg,
                                                        id_hist2 + nbwtg,
                                                        tgptr[k].wid + 1,
                                                        -lm->
                                                        ngprob[2][tgptr[k].
                                                               probid].f);
                                            }
                                        }
                                        else {
                                            fprintf(file,
                                                    "%d\t%d\t%d\t%f\n",
                                                    id_hist1 + nbwtg,
                                                    id_hist2 + nbwtg,
                                                    tgptr[k].wid + 1,
                                                    -lm->ngprob[2][tgptr[k].
                                                                probid].f);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    if (is32bits)
                        fprintf (file, "%d\t%d\t%d\t%f\n",
								 i, st_end, bgptr32[j].wid + 1,
								 -lm->ngprob[1][bgptr32[j].probid].f);
									/* 2g->2g */
                    else
                        fprintf (file, "%d\t%d\t%d\t%f\n",
								 i, st_end, bgptr[j].wid + 1,
								 -lm->ngprob[1][bgptr[j].probid].f);
									/* 2g->2g */
                }
            }
        }
    }

    /* End state */
    fprintf(file, "%d\t0\n", st_end);

    fclose(file);
    fprintf(stderr, "\nFSM written\n\n");
    return LM_SUCCESS;
}
