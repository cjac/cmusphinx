/**
 \file lm_max_read.c
 \brief Tool for use LM Max files
*/
/*
 HISTORY
 2008/09/18  N. Coetmeur, supervised by Y. Esteve
 Adjust test_best_scores function for compute score of inexisting quadrigrams

 2008/07/08  N. Coetmeur, supervised by Y. Esteve
 Creating
*/


#include <stdlib.h>
#include "lm_max.h"
#include "logs3.h"


/**
 \struct test_score_t
 \brief Maximum score testing
*/
typedef struct test_score_t
{
	lmlog_t	max;	/**< Maximum score */
	uint8	used;	/**< Whether a score is saved */
} test_score_t;


/**
	\brief Command line arguments
*/
static arg_t defn[] = {
    { "-dmp",
      ARG_STRING,
      NULL,
      "Language model input file (precompiled .DMP file)" },
    { "-max",
      ARG_STRING,
      NULL,
      "Maximums language model input file (precompiled .MAX file)" },
    { "-appweight",
      ARG_INT32,
      "1",
      "Determine whether to apply options -langwt -ugwt -inspen" },
    { "-logbase",
      ARG_FLOAT32,
      "1.0003",
      "Base in which all log values calculated" },
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    { "-langwt",
      ARG_FLOAT32,
      "9.5",
      "Language weight: empirical exponent applied to LM probabilty" },
    { "-ugwt",
      ARG_FLOAT32,
      "0.7",
      "LM unigram weight: unigram probs interpolated with uniform distribution with this weight" },
    { "-inspen",
      ARG_FLOAT32,
      "0.65",
      "Word insertion penalty" },
    
    { NULL, ARG_INT32,  NULL, NULL }
};


/**
	\brief Verify a max file
	\return Error code
*/
int
verify_max_file	(	lm_t		* lm,		/**< In: language model */
					lm_max_t	* lm_max	/**< In: maximum values */
				)
{
	uint32 ugi, bgi_cur, bgi_end, tgi_cur, tgi_end, qgi_cur=0, qgi_end;
	uint32 i, p_ug=0, p_bg=0, p_tg=0, p_qg=0;
	uint32 nbg, fbg, ibg, ntg, ftg, itg;
	short * max_bowt;
	short * max_prob;
	ng32_t * bg;
	ng32_t * tg;
	ng32_t * qg;

	/* verify parameters */
	if ((lm == NULL) || (lm_max == NULL)) {
		E_ERROR ("Bad arguments for verify_max_file\n");
		return -1;
	}
	if (lm_max->use_linked_lists) {
		E_ERROR ("verify_max_file must use LM Max model with tables\n");
		return -1;
	}

	/* allocate memory */
	max_bowt = (short *)ckd_calloc (lm_max->n_bg, sizeof(short));
	if (max_bowt == NULL) {
		E_ERROR_SYSTEM ("Can't allocate memory for max_bowt table\n");
		return -2;
	}
	max_prob = (short *)ckd_calloc (lm_max->n_tg, sizeof(short));
	if (max_prob == NULL) {
		E_ERROR_SYSTEM ("Can't allocate memory for max_prob table\n");
		ckd_free (max_bowt);
		return -2;
	}

	E_INFO ("Reading all trigrams and quadrigrams\n\n");
	E_INFO ("\t1-grams\t2-grams\t3-grams\t4-grams\n");
	E_INFO ("\t   0%%\t   0%%\t   0%%\t   0%%\n");

	for (ugi=0; ugi<lm_max->n_ug; ugi++) {
		/* unigrams */

		/* compute corresponding first bigram and number */
		bgi_cur = lm->ug[ugi].firstbg;
		bgi_end = lm->ug[ugi+1].firstbg;

		for (bg=&(lm->ng32[1][bgi_cur]); bgi_cur<bgi_end; bgi_cur++, bg++) {
			/* bigrams */

			/* compute corresponding first trigram and number */
			tgi_cur = bg->firstnng
				+ lm->ng_segbase[2][bgi_cur>>(lm->log_ng_seg_sz[1])];
			tgi_end = lm->ng32[1][bgi_cur+1].firstnng
				+ lm->ng_segbase[2][(bgi_cur+1)>>(lm->log_ng_seg_sz[1])];

			for (tg=&(lm->ng32[2][tgi_cur]);
				 tgi_cur<tgi_end; tgi_cur++, tg++) {
				/* trigrams */

				/* verify maximum back-off weight
					   for all trigrams endend by w2,w3 */
				fbg = lm_max->ug_table[bg->wid];
				ibg = nbg = lm_max->ug_table[bg->wid+1]-fbg;
				if (fbg < lm_max->n_bg)
					ibg = find_bg (&(lm_max->bg_table[fbg]), nbg, tg->wid);
				if (ibg < nbg) {
					/* max bigram found */
					if (lm->ngbowt[3][tg->bowtid].f >
						lm_max->bg_table[fbg+ibg].max_bowt.f)
						E_ERROR (
						 "Bad maximum back-off weight (%d,%d)=%f(%d) != %f(%d)\n"
						 , bg->wid, tg->wid
						 , lm_max->bg_table[fbg+ibg].max_bowt.f
						 , lm_max->bg_table[fbg+ibg].max_bowt.l
						 , lm->ngbowt[3][tg->bowtid].f
						 , lm->ngbowt[3][tg->bowtid].l);
					if (lm->ngbowt[3][tg->bowtid].l ==
						lm_max->bg_table[fbg+ibg].max_bowt.l)
						max_bowt[fbg+ibg] = 1;
				}
				else
					/* no max bigram w2,w3 found */
					E_ERROR ("No maximum back-off weight (%d,%d)\n",
							 bg->wid, tg->wid);

				/* compute first quadrigram and number
					   corresponding to current trigram */
				qgi_cur = tg->firstnng
					+ lm->ng_segbase[3][tgi_cur>>(lm->log_ng_seg_sz[2])];
				qgi_end = lm->ng32[2][tgi_cur+1].firstnng
					+ lm->ng_segbase[3][(tgi_cur+1)>>(lm->log_ng_seg_sz[2])];

				for (qg=&(lm->ng32[3][qgi_cur]);
					 qgi_cur<qgi_end; qgi_cur++, qg++)
					/* quadrigrams */

					/* compute maximum probability
						   for all quadigrams endend by w2,w3,w4 */
					if (ibg < nbg) {
						/* max bigram found */

						/* search the max trigram w2,w3,w4 */
						ftg = lm_max->bg_table[fbg+ibg].firsttg;
						itg = ntg = lm_max->bg_table[fbg+ibg+1].firsttg-ftg;
						if (ftg < lm_max->n_tg)
							itg = find_tg (&(lm_max->tg_table[ftg]),
										   ntg, qg->wid);

						if (itg < ntg) {
							/* max trigram found */
							if (lm->ngprob[3][qg->probid].f >
								lm_max->tg_table[ftg+itg].max_prob.f)
								E_ERROR (
								 "Bad maximum probability (%d,%d,%d)=%f(%d) != %f(%d)\n"
								 , bg->wid, tg->wid, qg->wid
								 , lm_max->tg_table[ftg+itg].max_prob.f
								 , lm_max->tg_table[ftg+itg].max_prob.l
								 , lm->ngprob[3][qg->probid].f
								 , lm->ngprob[3][qg->probid].l);
							if (lm->ngprob[3][qg->probid].l ==
								lm_max->tg_table[ftg+itg].max_prob.l)
								max_prob[ftg+itg] = 1;
						}
						else
							/* no max trigram w2,w3,w4 found */
							E_ERROR ("No maximum probability (%d,%d,%d)\n",
									 bg->wid, tg->wid, qg->wid);
					}

				/* print working */
				p_ug=100*ugi/(lm->n_ng[0]);
				p_bg=100*bgi_cur/(lm->n_ng[1]);
				p_tg=100*tgi_cur/(lm->n_ng[2]);
				if (((i=100*qgi_cur/(lm->n_ng[3])) >= 10*(p_qg/10+1))
					&& (i < 100)) {
					p_qg = i;
					E_INFO ("\t%4d%%\t%4d%%\t%4d%%\t%4d%%\n",
							p_ug, p_bg, p_tg, p_qg);
				}
			}
		}
	}
	E_INFO ("\t 100%%\t 100%%\t 100%%\t 100%%\n\n");

	E_INFO ("Verify if all max values are reached\n");
	for (ibg=0; ibg<lm_max->n_bg; ibg++)
		if (!(max_bowt[ibg]))
			E_ERROR ("Max bigram n°%d not reached\n", ibg);
	for (itg=0; itg<lm_max->n_tg; itg++)
		if (!(max_prob[itg]))
			E_ERROR ("Max trigram n°%d not reached\n", itg);
	E_INFO ("End\n");

	/* free memory */
	ckd_free (max_bowt);
	ckd_free (max_prob);

	return 0;
}

/**
	\brief Special version of lm_ng_score used by
	test_best_scores function
	\details Compute score for inexisting N-grams
	\see test_best_scores
	\see lm.h:lm_ng_score
*/
int32
lm_ng_score_spec(lm_t * lm, uint32 N, s3lmwid32_t *lw, s3wid_t wn)
{
    int32 n, score, test;
    uint32 p;
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

    for (p = 0; p < N; p++)
        if (NOT_LMWID(lm, lw[p]) || (lw[p] >= lm->n_ng[0]))
            E_FATAL ("Bad lw%d argument (%d) to lm_ng_score\n", p+1, lw[p]);

    if (N >= 3) {
        /* trigram or more */

        if (is32bits) {
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
						load_ng(lm, 2, lw);
                    lm->membg32[lw[0]].used = 1;
                    ng32 = lm->membg32[lw[0]].bg32;
                }
                else {
                    if (!lm->membg[lw[0]].bg)
                        load_ng(lm, 2, lw);
                    lm->membg[lw[0]].used = 1;
                    ng = lm->membg[lw[0]].bg;
                }
            }
        }
        else { /* N == 1 */
            /* unigram */

            lm->access_type = 1;

            if (lm->inclass_ugscore)
                return (lm->ug[lw[0]].prob.l + lm->inclass_ugscore[lw[0]]);
            else
                return (lm->ug[lw[0]].prob.l);
        }
    }

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


    return (score);
}

/**
	\brief Special version of lm_cache_reset used by
	test_best_scores function
	\details Reset cache without comments
	\see test_best_scores
	\see lm.h:lm_cache_reset
*/
void
lm_cache_reset_spec(lm_t * lm)
{
    uint32 i, n;
    int32 *n_ngfree;
    nginfo_t *nginfo, *next_nginfo, *prev_nginfo;
    nginfo32_t *nginfo32, *next_nginfo32, *prev_nginfo32;
    int32 is32bits;

    n_ngfree = (int32 *) ckd_calloc (lm->max_ng, sizeof(int32));


    /* ARCHAN: RAH only short-circult this function only */
    if (lm->isLM_IN_MEMORY) /* RAH We are going to short circuit this
                                if we are running with the LM in memory */
        return;

    is32bits = lm->is32bits;

    /* disk-based; free "stale" Bigrams */
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
                            if (    ((!lm->ng32) || (!lm->ng32[n-1]))
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
}

/**
	\brief Test maximum scores for all possible quadrigrams
	\return Error code
*/
int
test_best_scores	(	lm_t		*	lm,		/**< In: language model */
						lm_max_t	*	lm_max,	/**< In: maximum values */
						short			part,	/**< In: part to test */
						short			n_parts	/**< In: number of parts */
					)
{
	uint32 first_ug, last_ug, num_ug;
	test_score_t ***prob_table;
	s3lmwid32_t w[4];
	int n, p, p2;
	lmlog_t pc;

	/* verify parameters */
	if ((lm == NULL) || (lm_max == NULL)) {
		E_ERROR ("Bad arguments for test_best_scores\n");
		return -1;
	}
	if (lm_max->use_linked_lists) {
		E_ERROR ("test_best_scores must use LM Max model with tables\n");
		return -1;
	}

	num_ug = lm->n_ng[0]/n_parts;
	if ((n_parts <= 1) || (part <= 0) || (num_ug <= 0)) {
		first_ug = 0;
		last_ug = (num_ug = lm->n_ng[0])-1;
	}
	else {
		if (part >= n_parts) {
			part = n_parts;
			first_ug = (part-1)*num_ug;
			last_ug = lm->n_ng[0]-1;
			num_ug = lm->n_ng[0]-((n_parts-1)*num_ug);
		}
		else {
			first_ug = (part-1)*num_ug;
			last_ug = (part*num_ug)-1;
		}
	}

	E_INFO (">>> allocating memory for best scores...\n");
	prob_table = (test_score_t ***)
			ckd_calloc (num_ug, sizeof(test_score_t **));
	if (prob_table == NULL) {
		E_ERROR_SYSTEM ("Can't allocate memory for test_best_scores\n");
		return -1;
	}
	for (p=0, w[0]=first_ug; w[0]<=(last_ug+1); w[0]++) {
		if (w[0] <= last_ug) {
			prob_table[w[0]-first_ug] = (test_score_t **)
				ckd_calloc (lm->n_ng[0], sizeof(test_score_t *));
			for (w[1]=0; (prob_table[w[0]-first_ug]!=NULL) && (w[1]<lm->n_ng[0])
				; w[1]++) {
				prob_table[w[0]-first_ug][w[1]] = (test_score_t *)
					ckd_calloc (lm->n_ng[0], sizeof(test_score_t));
				for (w[2]=0; (prob_table[w[0]-first_ug][w[1]]!=NULL)
					&& (w[2]<lm->n_ng[0]); w[2]++)
					prob_table[w[0]-first_ug][w[1]][w[2]].used = 0;
			}
		}

		/* print working */
		for (n = 100*(w[0]-first_ug)/num_ug; p <= n; p++)
			if (p > 0) {
				if (p%10 == 0)
					printf("%3d%%\n", p);
				else
					printf(" - ");
				fflush(stdout);
			}
	}

	E_INFOCONT ("\n");
	E_INFO (">>> computing ");
	if ((n_parts <= 1) || (part <= 0))
		E_INFOCONT ("all");
	else
		E_INFOCONT ("part %d/%d (%d at %d) of"
					, part, n_parts, first_ug, last_ug);
	E_INFOCONT (" best scores...\n");
	for (p = 0, w[0]=0; w[0]<=lm->n_ng[0]; w[0]++) {
		for (w[1]=first_ug; (w[0] < lm->n_ng[0]) && (w[1]<=last_ug) ; w[1]++)
			for (w[2]=0; (prob_table[w[1]-first_ug]!=NULL) && (w[2]<lm->n_ng[0])
				; w[2]++) {
				for (w[3]=0; (prob_table[w[1]-first_ug][w[2]]!=NULL)
					&& (w[3]<lm->n_ng[0]); w[3]++) {
					/* existing (or not) quadrigram */
					pc.l = lm_ng_score (lm, 4, w, 0);
					if (prob_table[w[1]-first_ug][w[2]][w[3]].used)
					{
						if (pc.l > prob_table[w[1]-first_ug][w[2]][w[3]].max.l)
							prob_table[w[1]-first_ug][w[2]][w[3]].max.l = pc.l;
					}
					else
					{
						prob_table[w[1]-first_ug][w[2]][w[3]].max.l = pc.l;
						prob_table[w[1]-first_ug][w[2]][w[3]].used = 1;
					}

					/* inexisting quadrigram */
					pc.l = lm_ng_score_spec (lm, 4, w, 0);
					if (pc.l > prob_table[w[1]-first_ug][w[2]][w[3]].max.l)
						prob_table[w[1]-first_ug][w[2]][w[3]].max.l = pc.l;
				}
				lm_cache_reset_spec (lm);
			}

		/* print working */
		for (n = 100*w[0]/lm->n_ng[0]; p <= n; p++)
			if (p > 0) {
				if (p%10 == 0)
					printf("%3d%%\n", p);
				else
					printf(" - ");
				fflush(stdout);
			}
	}

	E_INFOCONT ("\n");
	E_INFO (">>> testing ");
	if ((n_parts <= 1) || (part <= 0))
		E_INFOCONT ("all");
	else
		E_INFOCONT ("part %d/%d (%d at %d) of"
					, part, n_parts, first_ug, last_ug);
	E_INFOCONT (" best scores...\n");
	for (p = 0, p2 = 0, w[0]=first_ug; w[0]<=(last_ug+1); w[0]++) {
		if (w[0] <= last_ug) {
			if (prob_table[w[0]-first_ug] == NULL) {
				if (p2 != (p + 1))
					E_INFOCONT ("\n");
				p2 = p + 1;
				E_ERROR_SYSTEM ("Memory allocation problem\n");
			}
			else {
				for (w[1]=0; w[1]<lm->n_ng[0]; w[1]++) {
					if (prob_table[w[0]-first_ug][w[1]] == NULL) {
						if (p2 != (p + 1))
							E_INFOCONT ("\n");
						p2 = p + 1;
						E_ERROR_SYSTEM ("Memory allocation problem\n");
					}
					else {
						for (w[2]=0; w[2]<lm->n_ng[0]; w[2]++) {
							if (prob_table[w[0]-first_ug][w[1]][w[2]].used)
							{
								pc.l = lm_best_4g_score (lm, lm_max, w, 0);
								if (pc.l
									!= prob_table[w[0]-first_ug][w[1]][w[2]].max.l) {
									if (p2 != (p + 1))
										E_INFOCONT ("\n");
									p2 = p + 1;
									E_ERROR (
										"Bad best score (%d,%d,%d)=%d != %d\n"
										, w[0], w[1], w[2], pc
										, prob_table[w[0]-first_ug][w[1]][w[2]].max.l);
								}
							}
							else {
								if (p2 != (p + 1))
									E_INFOCONT ("\n");
								p2 = p + 1;
								E_ERROR ("Best score (%d,%d,%d) not computed\n"
										 , w[0], w[1], w[2]);
							}
						}
						ckd_free (prob_table[w[0]-first_ug][w[1]]);
						lm_cache_reset_spec (lm);
					}
				}
				ckd_free (prob_table[w[0]-first_ug]);
			}
		}

		/* print working */
		for (n = 100*(w[0]-first_ug)/num_ug; p2 <= n; p2++)
			if (p2 > 0) {
				if (p2%10 == 0) {
					p = p2;
					printf("%3d%%\n", p);
				}
				else
					printf(" - ");
				fflush(stdout);
			}
	}

	E_INFO ("<<< end tests\n");
	return 0;
}

/**
	\brief Print the list of commands
*/
void
show_help ()
{
	printf ("--------------------------------------------------------------\n");
	printf ("b <   w2 w3 w4>  best score of a quadrigram ended by w2,w3,w4\n");
	printf ("m <   w2 w3>     maximum back-off weight of trigrams *,w2,w3\n");
	printf ("m <   w2 w3 w4>  maximum probability of quadrigrams *,w2,w3,w4\n");
	printf ("s <w1         >  score of unigram\n");
	printf ("s <w1 w2      >  score of bigram\n");
	printf ("s <w1 w2 w3   >  score of trigram\n");
	printf ("s <w1 w2 w3 w4>  score of quadrigram\n");
	printf ("f <number>       convert a long in float\n");
	printf ("l <number>       convert a float in long\n");
	printf ("vmf              verify the max file\n");
	printf ("tbs [part nb]    test all (or a part of nb) of the best scores\n");
	printf ("--------------------------------------------------------------\n");
	printf ("exit/quit/q      end of program\n\n");
}

/**
	\brief Main function
*/
int main (int argc, char *argv[])
{
	lm_max_t * ngrams_list = NULL;
	int ret_val = 0, n;
	char word[4][256];
	s3lmwid32_t w[4];
    lm_t * lm = NULL;
	char * lmmaxfile;
	char str[1030];
	char * lmfile;
	float64 log;
	lmlog_t bs;
	short dec;

	/* verify parameters */
	if (argc <= 2)
	{
		cmd_ln_print_help(stderr,defn);
		return -1;
	}
	cmd_ln_parse (defn, argc, argv, 0);
	if ((lmfile = (char *) cmd_ln_str("-dmp")) == NULL)
		E_ERROR ("-dmp argument missing\n");
	if ((lmmaxfile = (char *) cmd_ln_str("-max")) == NULL)
		E_ERROR ("-max argument missing\n");

	/* remove memory allocation restrictions */
    unlimit ();

    /* Initialize log(S3-base). All scores (probs...) computed in log domain to
	avoid underflow. At the same time, log base = 1.0001 (1+epsilon) to allow
	log values to be maintained in int32 variables without significant loss of
	precision. */
    if (*(int32 *)cmd_ln_access("-appweight")) {
		log = *((float64 *) cmd_ln_access("-logbase"));
		if (log <= 1.0)
			E_FATAL("Illegal log-base: %e; must be > 1.0\n", log);
		if (log > 1.1)
			E_WARN("Logbase %e perhaps too large?\n", log);

		logs3_init (log, 0, cmd_ln_int32 ("-log3table"));
    }

	
	/* read language model */
	lm = lm_read_advance2(lmfile, "default",
						  *(float64 *)cmd_ln_access("-langwt"),
						  *(float64 *)cmd_ln_access("-inspen"),
						  *(float64 *)cmd_ln_access("-ugwt"), 0, "DMP",
						  *(int32 *)cmd_ln_access("-appweight"), 1);
	if (lm == NULL)
		return -3;
	if (lm->max_ng < 4) {
		E_ERROR ("You must use a quadrigram (or more) DMP file\n");
		lm_free (lm);
		return -3;
	}
	if (! (lm->is32bits)) {
		/* force 32-bits version */
		E_INFOCONT ("\n");
		E_INFO ("Converting LM in 32-bits...\n\n\n");
		lm_convert_structure (lm, 1);
	}
	else
		E_INFOCONT ("\n\n");

	/* read max file */
	E_INFO ("Reading Maximums LM file %s...\n", lmmaxfile);
	ngrams_list = lm_max_read_dump (lmmaxfile,
									*(int32 *)cmd_ln_access("-appweight"),
									*(float64 *)cmd_ln_access("-langwt"),
									*(float64 *)cmd_ln_access("-inspen"));
	if (ngrams_list == NULL) {
		if (lm != NULL)
			lm_free (lm);
		return -3;
	}
	E_INFO ("... End\n", lmmaxfile);


	/* commands loop */
	E_INFOCONT ("\n");
	str[0]='\0';
	do {
		if (str[0] != '\0') {
			n = 0;
			if (sscanf (str, " b %s %s %s", word[0], word[1], word[2]) == 3)
				n = 131;
			if (sscanf (str, " b %u %u %u", &(w[0]), &(w[1]), &(w[2])) == 3)
				n = 132;
			if (sscanf (str, " m %s %s", word[0], word[1]) == 2)
				n = 221;
			if (sscanf (str, " m %u %u", &(w[0]), &(w[1])) == 2)
				n = 222;
			if (sscanf (str, " m %s %s %s", word[0], word[1], word[2]) == 3)
				n = 231;
			if (sscanf (str, " m %u %u %u", &(w[0]), &(w[1]), &(w[2])) == 3)
				n = 232;
			if (sscanf (str, " s %s", word[0]) == 1)
				n = 311;
			if (sscanf (str, " s %u", &(w[0])) == 1)
				n = 312;
			if (sscanf (str, " s %s %s", word[0], word[1]) == 2)
				n = 321;
			if (sscanf (str, " s %u %u", &(w[0]), &(w[1])) == 2)
				n = 322;
			if (sscanf (str, " s %s %s %s", word[0], word[1], word[2]) == 3)
				n = 331;
			if (sscanf (str, " s %u %u %u", &(w[0]), &(w[1]), &(w[2])) == 3)
				n = 332;
			if (sscanf (str, " s %s %s %s %s"
				, word[0], word[1], word[2], word[3]) == 4)
				n = 341;
			if (sscanf (str, " s %u %u %u %u"
				, &(w[0]), &(w[1]), &(w[2]), &(w[3])) == 4)
				n = 342;
			if (sscanf (str, " f %d", &(w[0])) == 1)
				n = 410;
			if (sscanf (str, " l %lf", &log) == 1)
				n = 420;
			if (strncmp (&(str[dec]), "vmf", 3) == 0)
				n = 510;
			if (strncmp (&(str[dec]), "tbs", 3) == 0)
				n = 520;
			if (sscanf (str, " tbs %u %u", &(w[0]), &(w[1])) == 2)
				n = 521;
			if ((strncmp (&(str[dec]), "help", 4) == 0)
				|| (strncmp (&(str[dec]), "?", 1) == 0))
				n = 900;

			switch (n) {
			case 131:
				for (n=0; n<3; n++)	
					w[n] = lm_wid(lm, word[n]);
				bs.l = lm_best_4g_score (lm, ngrams_list, w, 0);
				printf ("best score (%u,%u,%u) = %f(%d)\n", w[0], w[1], w[2]
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 132:
				bs.l = lm_best_4g_score (lm, ngrams_list, w, 0);
				printf ("best score (%s %s %s) = %f(%d)\n"
					, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
					, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
					, (w[2]<lm->n_ng[0])?lm->wordstr[w[2]]:"~"
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 221:
				for (n=0; n<2; n++)	
					w[n] = lm_wid(lm, word[n]);
				if (lm_max_get_bowt (ngrams_list, &bs, w[0], w[1]) == 0)
					printf ("max bowt (%u %u) = %f(%d)\n", w[0], w[1]
						, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
							/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				else
					printf ("max bowt (%u %u) : bigram not found\n"
						, w[0], w[1]);
				break;
			case 222:
				if (lm_max_get_bowt (ngrams_list, &bs, w[0], w[1]) == 0)
					printf ("max bowt (%s %s) = %f(%d)\n"
						, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
						, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
						, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
							/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				else
					printf ("max bowt (%s %s) : bigram not found\n"
						, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
						, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~");
				break;
			case 231:
				for (n=0; n<3; n++)	
					w[n] = lm_wid(lm, word[n]);
				if (lm_max_get_prob (ngrams_list, &bs, w[0], w[1], w[2])
					== 0)
					printf ("max prob (%u %u %u) = %f(%d)\n", w[0], w[1], w[2]
						, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
							/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				else
					printf ("max prob (%u %u %u) : trigram not found\n"
						, w[0], w[1], w[2]);
				break;
			case 232:
				if (lm_max_get_prob (ngrams_list, &bs, w[0], w[1], w[2])
					== 0)
					printf ("max prob (%s %s %s) = %f(%d)\n"
						, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
						, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
						, (w[2]<lm->n_ng[0])?lm->wordstr[w[2]]:"~", bs.l
							* log10(*((float64 *) cmd_ln_access("-logbase")))
							/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				else
					printf ("max prob (%s %s %s) : trigram not found\n"
						, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
						, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
						, (w[2]<lm->n_ng[0])?lm->wordstr[w[2]]:"~");
				break;
			case 311:
				w[0] = lm_wid(lm, word[0]);
				bs.l = lm_ng_score (lm, 1, w, 0);
				printf ("score (%u) = %f(%d)\n", w[0]
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 312:
				bs.l = lm_ng_score (lm, 1, w, 0);
				printf ("score (%s) = %f(%d)\n"
					, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 321:
				for (n=0; n<2; n++)	
					w[n] = lm_wid(lm, word[n]);
				bs.l = lm_ng_score (lm, 2, w, 0);
				printf ("score (%u %u) = %f(%d)\n", w[0], w[1]
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 322:
				bs.l = lm_ng_score (lm, 2, w, 0);
				printf ("score (%s %s) = %f(%d)\n"
					, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
					, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 331:
				for (n=0; n<3; n++)	
					w[n] = lm_wid(lm, word[n]);
				bs.l = lm_ng_score (lm, 3, w, 0);
				printf ("score (%u %u %u) = %f(%d)\n", w[0], w[1], w[2]
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 332:
				bs.l = lm_ng_score (lm, 3, w, 0);
				printf ("score (%s %s %s) = %f(%d)\n"
					, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
					, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
					, (w[2]<lm->n_ng[0])?lm->wordstr[w[2]]:"~"
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 341:
				for (n=0; n<4; n++)	
					w[n] = lm_wid(lm, word[n]);
				bs.l = lm_ng_score (lm, 4, w, 0);
				printf ("score (%u %u %u %u) = %f(%d)\n", w[0], w[1], w[2], w[3]
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 342:
				bs.l = lm_ng_score (lm, 4, w, 0);
				printf ("score (%s %s %s %s) = %f(%d)\n"
					, (w[0]<lm->n_ng[0])?lm->wordstr[w[0]]:"~"
					, (w[1]<lm->n_ng[0])?lm->wordstr[w[1]]:"~"
					, (w[2]<lm->n_ng[0])?lm->wordstr[w[2]]:"~"
					, (w[3]<lm->n_ng[0])?lm->wordstr[w[3]]:"~"
					, bs.l * log10(*((float64 *) cmd_ln_access("-logbase")))
						/ *(float64 *)cmd_ln_access("-langwt"), bs.l);
				break;
			case 410:
				bs.l = w[0];
				printf ("l %d # f %f\n", bs.l, bs.f);
				break;
			case 420:
				bs.f = log;
				printf ("f %f # l %d\n", bs.f, bs.l);
				break;
			case 510:
				verify_max_file (lm, ngrams_list);
				break;
			case 520:
				test_best_scores (lm, ngrams_list, 0, 1);
				break;
			case 521:
				test_best_scores (lm, ngrams_list, w[0], w[1]);
				break;
			case 900:
				printf("list of commands:\n");
				show_help ();
				break;
			default:
				if ((str[0] != '\n') && (str[0] != ' ') && (str[0] != '\t')) {
					printf("unknown command, try:\n");
					show_help ();
				}
			}
		}
		printf ("> ");
		fgets (str, 1030, stdin);
		for (dec = 0; (str[dec] == ' ') || (str[dec] == '\t'); dec++);
	} while (!((strncmp (&(str[dec]), "q", 1) == 0)
		||  (strncmp (&(str[dec]), "exit", 4) == 0)));


	/* free memory */
	if (ngrams_list != NULL)
		lm_max_free (ngrams_list);
	if (lm != NULL)
		lm_free (lm);
	cmd_ln_free ();

	return ret_val;
}
