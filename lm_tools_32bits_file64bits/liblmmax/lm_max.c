/**
 \file lm_max.c
 \brief Implemantation of maximum probabilities computing
	 
 This is the implmentation file for compute and use maximums back-off
 weight for all trigrams ended by any words w2,w3 and maximum
 probabilities for all quadrigrams ended by any words w2,w3,w4.
*/
/*
 HISTORY
 2008/07/08  N. Coetmeur, supervised by Y. Esteve
 Creating
*/


#include <stdlib.h>
#include "lm_max.h"
#include "logs3.h"


/** Search thresh used by find_bg and find_tg functions */
#define BINARY_SEARCH_THRESH 16


/**
 \brief Header of a max dump file
*/
static const char *ng_header = "3-gram Max LM";

/**
 \brief File format description

 Written in max dump files.
*/
static const char *fmtdesc[] = {
 "BEGIN FILE FORMAT DESCRIPTION",
 "Header string length (int32) and string (including trailing 0)",
 "Original LM Max filename string-length (int32) and filename (including trailing 0)",
 "(int32) string-length and string (including trailing 0)",
 "... previous entry continued any number of times",
 "(int32) 0 (terminating sequence of strings)",
 "(int32) lm_t.1count (must be > 0)",
 "(int32) lm_t.2count (must be > 0)",
 "(int32) lm_t.3count (must be > 0)",
 "lm_t.1count+1 1-grams (including sentinel)",
 "lm_t.2count+1 2-grams (including sentinel)",
 "lm_t.3count 3-grams",
 "END FILE FORMAT DESCRIPTION",
 NULL,
};


/**
  \brief Verify the maximum probability of a quadrigram

  Verify the probability of a quadrigram w1,w2,w3,w4 and
  store the maximum in a sorted trigrams w2,w3,w4 list.

  \return Error value
*/
static int
store_max_tg	(lm_t		*	lm,		/**< In: Language model */
				 lm_max_t	*	lm_max,	/**< In/Out: Set of maximums
										   probabilities and back-off weights */
				 lmlog_t		prob,	/**< In: Probability of current
													   quadrigram w1,w2,w3,w4 */
				 s3lmwid32_t	w2,		/**< In: Second word of quadrigram */
				 s3lmwid32_t	w3,		/**< In: Third word of quadrigram */
				 s3lmwid32_t	w4		/**< In: Fourth word of quadrigram */
				 )
{
	bg_max_bowt_node_t *new_bgn;
	bg_max_bowt_node_t *cur_bgn;
	bg_max_bowt_node_t *prec_bgn = NULL;
	tg_max_prob_node_t *new_tgn;
	tg_max_prob_node_t *cur_tgn = NULL;
	tg_max_prob_node_t *prec_tgn = NULL;

	/* verify parameters */
	if (    (lm_max == NULL) || (lm == NULL)
		|| NOT_LMWID(lm, w2) || (w2 >= lm->n_ng[0])
		|| NOT_LMWID(lm, w3) || (w3 >= lm->n_ng[0])
		|| NOT_LMWID(lm, w4) || (w4 >= lm->n_ng[0]) ) {
			E_ERROR ("Bad arguments to store_max_tg\n");
			return -1;
	}

	/* search the bigram w2,w3 */
	cur_bgn = lm_max->ug_list[w2].cur_bgn;
	if ((cur_bgn != NULL) && (cur_bgn->bg.wid > w3))
		cur_bgn = lm_max->ug_list[w2].firstbgn;
	while ((cur_bgn != NULL) && (cur_bgn->bg.wid < w3)) {
		prec_bgn = cur_bgn;
		cur_bgn = cur_bgn->next;
	}

	if ((cur_bgn != NULL) && (cur_bgn->bg.wid == w3)) {
		/* bigram found */
		lm_max->ug_list[w2].cur_bgn = cur_bgn;

		/* search the trigram w2,w3,w4 */
		cur_tgn = cur_bgn->cur_tgn;
		if ((cur_tgn != NULL) && (cur_tgn->tg.wid > w4))
			cur_tgn = cur_bgn->firsttgn;
		while ((cur_tgn != NULL) && (cur_tgn->tg.wid < w4)) {
			prec_tgn = cur_tgn;
			cur_tgn = cur_tgn->next;
		}

		if ((cur_tgn != NULL) && (cur_tgn->tg.wid == w4)) {
			/* trigram found */
			cur_bgn->cur_tgn = cur_tgn;

			/* verify the maximum probability */
			if (cur_tgn->tg.max_prob.f < prob.f)
				cur_tgn->tg.max_prob.l = prob.l;

			return 0;
		}
	}
	else {
		/* no bigram found, allocate a new */
		new_bgn = (bg_max_bowt_node_t *)
			ckd_calloc (1, sizeof(bg_max_bowt_node_t));
		if (new_bgn == NULL) {
			E_ERROR_SYSTEM (
					"Fail to allocate memory for a new bigram node\n");
			return -2;
		}
		new_bgn->bg.wid = w3;

		/* link the new bigram */
		new_bgn->next = cur_bgn;
		if (cur_bgn == lm_max->ug_list[w2].firstbgn)
			lm_max->ug_list[w2].firstbgn = new_bgn;
		if (prec_bgn != NULL)
			prec_bgn->next = new_bgn;

		lm_max->ug_list[w2].cur_bgn = new_bgn;
		lm_max->ug_list[w2].n_bgn++;
		lm_max->n_bg++;
		cur_bgn = new_bgn;
	}

	/* allocate a new trigram */
	new_tgn = (tg_max_prob_node_t *) ckd_calloc (1, sizeof(tg_max_prob_node_t));
	if (new_tgn == NULL) {
		E_ERROR_SYSTEM (
			"Fail to allocate memory for a new trigram node\n");
		return -2;
	}
	new_tgn->tg.wid = w4;
	new_tgn->tg.max_prob.l = prob.l;

	/* link the new trigram */
	new_tgn->next = cur_tgn;
	if (cur_tgn == cur_bgn->firsttgn)
		cur_bgn->firsttgn = new_tgn;
	if (prec_tgn != NULL)
		prec_tgn->next = new_tgn;

	cur_bgn->n_tgn++;
	lm_max->n_tg++;
	cur_bgn->cur_tgn = new_tgn;

	return 0;
}

/**
  \brief Verify the maximum back-off weight of a trigram

  Verify the back-off weight of a trigram w1,w2,w3 and
  store the maximum in a sorted bigrams w2,w3 list.

  \return Error value
*/
static int
store_max_bg	(lm_t		*	lm,		/**< In: Language model */
				 lm_max_t	*	lm_max,	/**< In/Out: Set of maximums
										   probabilities and back-off weights */
				 lmlog_t		bowt,	/**< In: Back-off weight of
													 current trigram w1,w2,w3 */
				 s3lmwid32_t	w2,		/**< In: Second word of trigram */
				 s3lmwid32_t	w3		/**< In: Third word of trigram */
				 )
{
	bg_max_bowt_node_t *new_bgn;
	bg_max_bowt_node_t *cur_bgn;
	bg_max_bowt_node_t *prec_bgn = NULL;

	/* verify parameters */
	if (    (lm_max == NULL) || (lm == NULL)
		|| NOT_LMWID(lm, w2) || (w2 >= lm->n_ng[0])
		|| NOT_LMWID(lm, w3) || (w3 >= lm->n_ng[0]) ) {
			E_ERROR ("Bad arguments to store_max_bg\n");
			return -1;
	}

	/* search the bigram w2,w3 */
	cur_bgn = lm_max->ug_list[w2].cur_bgn;
	if ((cur_bgn != NULL) && (cur_bgn->bg.wid > w3))
		cur_bgn = lm_max->ug_list[w2].firstbgn;
	while ((cur_bgn != NULL) && (cur_bgn->bg.wid < w3)) {
		prec_bgn = cur_bgn;
		cur_bgn = cur_bgn->next;
	}

	if ((cur_bgn != NULL) && (cur_bgn->bg.wid == w3)) {
		/* bigram found */
		lm_max->ug_list[w2].cur_bgn = cur_bgn;

		/* verify the maximum back-off weight */
		if (cur_bgn->bg.max_bowt.f < bowt.f)
			cur_bgn->bg.max_bowt.l = bowt.l;
	}
	else {
		/* no bigram found, allocate a new */
		new_bgn = (bg_max_bowt_node_t *)
			ckd_calloc (1, sizeof(bg_max_bowt_node_t));
		if (new_bgn == NULL) {
			E_ERROR_SYSTEM (
					"Fail to allocate memory for a new bigram node\n");
			return -2;
		}
		new_bgn->bg.wid = w3;
		new_bgn->bg.max_bowt.l = bowt.l;

		/* link the new bigram */
		new_bgn->next = cur_bgn;
		if (cur_bgn == lm_max->ug_list[w2].firstbgn)
			lm_max->ug_list[w2].firstbgn = new_bgn;
		if (prec_bgn != NULL)
			prec_bgn->next = new_bgn;

		lm_max->ug_list[w2].cur_bgn = new_bgn;
		lm_max->ug_list[w2].n_bgn++;
		lm_max->n_bg++;
	}

	return 0;
}

/**
 \brief Convert all probabilities to logs3 values
*/
static void
lmmax2logs3	(lm_max_t *	lm_max /**< In/Out: Maximums model */
				)
{
    uint32 i;

	/* verify parameters */
	if (lm_max == NULL) {
		E_ERROR ("Bad lm_max argument for lmmax2logs3\n");
		return;
	}
	if (lm_max->use_linked_lists) {
		E_ERROR ("lmmax2logs3 must use LM Max model with tables\n");
		return;
	}


    /* bigrams */
    for (i = 0; i < lm_max->n_bg; i++) {
		if (lm_max->bg_table[i].max_bowt.f < MIN_PROB_F)
			lm_max->bg_table[i].max_bowt.f = MIN_PROB_F;

		lm_max->bg_table[i].max_bowt.l
			= log10_to_logs3 (lm_max->bg_table[i].max_bowt.f);
	}

    /* trigrams */
	for (i = 0; i < lm_max->n_tg; i++)
		lm_max->tg_table[i].max_prob.l
			= log10_to_logs3 (lm_max->tg_table[i].max_prob.f);
}

/**
  \brief Set the language-weight and insertion penalty parameters for the LM,
  after revoking any earlier set of such parameters

  \warning  This function doesn't prevent underflow of values.
  Make sure you call safe lm2logs3 before it. 
*/
static void
lm_max_set_param	(lm_max_t *	lm_max,	/**< In/Out: Maximums model */
					 float64	lw,    	/**< In: Langauage weight */
					 float64	wip    	/**< In: Word insertion penalty */
					 )
{
	int32 iwip = logs3(wip);
    uint32 i;

	/* verify parameters */
	if (lm_max == NULL) {
		E_ERROR ("Bad lm_max argument for lm_max_set_param\n");
		return;
	}
	if (lm_max->use_linked_lists) {
		E_ERROR ("lm_max_set_param must use LM Max model with tables\n");
		return;
	}
    if (lw <= 0.0) {
        E_ERROR ("lw = %e\n", lw);
		return;
	}
    if (wip <= 0.0) {
        E_ERROR ("wip = %e\n", wip);
		return;
	}

    /* bigrams */
    for (i = 0; i < lm_max->n_bg; i++)
		lm_max->bg_table[i].max_bowt.l
			= (int32) (lm_max->bg_table[i].max_bowt.l * lw);

    /* trigrams */
	for (i = 0; i < lm_max->n_tg; i++)
		lm_max->tg_table[i].max_prob.l =
			(int32) (lm_max->tg_table[i].max_prob.l * lw) + iwip;
}


int lm_max_compute (lm_t *lm, lm_max_t *lm_max)
{
	uint32 ugi, bgi_cur, bgi_end, tgi_cur, tgi_end, qgi_cur=0, qgi_end;
	uint32 i, p_ug=0, p_bg=0, p_tg=0, p_qg=0;
	ng32_t * bg;
	ng32_t * tg;
	ng32_t * qg;

	/* verify parameters */
	if ((lm == NULL) || (lm_max == NULL)) {
		E_ERROR ("Bad arguments for lm_max_compute\n");
		return -1;
	}
	if (! lm_max->use_linked_lists) {
		E_ERROR ("lm_max_compute must use LM Max model with linked lists\n");
		return -1;
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

				/* compute maximum back-off weight
					   for all trigrams endend by w2,w3 */
				if (store_max_bg (lm, lm_max, lm->ngbowt[3][tg->bowtid],
							  bg->wid, tg->wid))
					return -2;

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
					if (store_max_tg (lm, lm_max, lm->ngprob[3][qg->probid],
								  bg->wid, tg->wid, qg->wid))
						return -3;

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
	return 0;
}

lm_max_t * lm_max_read_dump (const char * filename,
							 int32 applyweight, float64 lw, float64 wip)
{
	uint32 len, n_ug, n_bg, n_tg;
	lm_max_t * lm_max;
	char str[14];
	FILE * fp;

	/* verify parameters */
    if (lw <= 0.0) {
        E_ERROR ("lw = %e\n", lw);
		return NULL;
	}
    if (wip <= 0.0){
        E_ERROR ("wip = %e\n", wip);
		return NULL;
	}

	if ((fp = fopen(filename, "rb")) == NULL)
	{
        E_ERROR_SYSTEM ("fopen(%s,rb) failed\n", filename);
		return NULL;
	}

	/* read header */
	if (fread (&len, sizeof(uint32), 1, fp) != 1) {
        E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}
	if (fread(str, sizeof(char), len, fp) != len)
	{
        E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}
	if ( (len!=(strlen(ng_header)+1))
		|| (strncmp(str, ng_header, len)!=0) ) {
		E_ERROR ("Bad header read\n");
		fclose (fp);
		return NULL;
	}

	/* read file name */
	if (fread (&len, sizeof(uint32), 1, fp) != 1) {
        E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}
	if (fseek(fp, len, SEEK_CUR) != 0)
	{
        E_ERROR_SYSTEM ("fseek(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}

	/* read and skip format description */
	do {
		if (fread (&len, sizeof(uint32), 1, fp) != 1) {
			E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
			fclose (fp);
			return NULL;
		}
		if (len > 0)
			if (fseek(fp, len, SEEK_CUR) != 0)
			{
				E_ERROR_SYSTEM ("fseek(%s) failed\n", filename);
				fclose (fp);
				return NULL;
			}
	} while (len > 0);

	/* read numbers of N-grams */
	if (fread (&n_ug, sizeof(uint32), 1, fp) != 1) {
        E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}
	else
		if (n_ug <= 0) {
			E_ERROR ("Incorrect number of unigrams\n");
			fclose (fp);
			return NULL;
		}
	if (fread (&n_bg, sizeof(uint32), 1, fp) != 1) {
        E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}
	else
		if (n_bg <= 0) {
			E_ERROR ("Incorrect number of bigrams\n");
			fclose (fp);
			return NULL;
		}
	if (fread (&n_tg, sizeof(uint32), 1, fp) != 1) {
        E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
		fclose (fp);
		return NULL;
	}
	else
		if (n_tg <= 0) {
			E_ERROR ("Incorrect number of trigrams\n");
			fclose (fp);
			return NULL;
		}

	/* allocate new model */
	lm_max = lm_max_tables_new (n_ug, n_bg, n_tg);
	if (lm_max != NULL) {
		/* read unigrams (with sentinel) */
		if (fread (lm_max->ug_table, sizeof(uint32), n_ug+1, fp) != (n_ug+1)) {
			E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
			fclose (fp);
			lm_max_free (lm_max);
			return NULL;
		}

		/* read bigrams (with sentinel) */
		if (fread (lm_max->bg_table, sizeof(bg_max_bowt_t), n_bg+1,
				   fp) != (n_bg+1)) {
			E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
			fclose (fp);
			lm_max_free (lm_max);
			return NULL;
		}

		/* read trigrams */
		if (fread (lm_max->tg_table, sizeof(tg_max_prob_t), n_tg,
				   fp) != n_tg) {
			E_ERROR_SYSTEM ("fread(%s) failed\n", filename);
			fclose (fp);
			lm_max_free (lm_max);
			return NULL;
		}

		if (applyweight) {
			/* applying unigram weight */

			/* convert to logs3 values */
			lmmax2logs3 (lm_max);

			/* apply the new lw and wip values */
			lm_max_set_param (lm_max, lw, wip);
		}
	}

	fclose (fp);
	return lm_max;
}

uint32 find_bg (bg_max_bowt_t * table, int32 nsearch, s3lmwid32_t w)
{
    int32 i, b, e;

    /* verify parameter */
    if (table == NULL)
        return nsearch;

    /* Binary search until segment size < threshold */
    b = 0;
    e = nsearch;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (table[i].wid < w)
            b = i + 1;
        else if (table[i].wid > w)
            e = i;
        else
            return i;
    }

    /* Linear search within narrowed segment */
    for (i = b; (i < e) && (table[i].wid != w); i++);
    return ((i < e) ? i : nsearch);
}

uint32 find_tg (tg_max_prob_t * table, int32 nsearch, s3lmwid32_t w)
{
    int32 i, b, e;

    /* verify parameter */
    if (table == NULL)
        return nsearch;

    /* Binary search until segment size < threshold */
    b = 0;
    e = nsearch;
    while (e - b > BINARY_SEARCH_THRESH) {
        i = (b + e) >> 1;
        if (table[i].wid < w)
            b = i + 1;
        else if (table[i].wid > w)
            e = i;
        else
            return i;
    }

    /* Linear search within narrowed segment */
    for (i = b; (i < e) && (table[i].wid != w); i++);
    return ((i < e) ? i : nsearch);
}

int lm_max_get_prob (lm_max_t * lm_max, lmlog_t * prob,
						 s3lmwid32_t w2, s3lmwid32_t w3, s3lmwid32_t w4)
{
	uint32 ng, fbg, ibg, ftg, itg;
	bg_max_bowt_node_t *cur_bgn;
	tg_max_prob_node_t *cur_tgn;

	/* verify parameters */
	if ( (lm_max == NULL) || (prob == NULL) ||
		(w2 >= lm_max->n_ug) || (w3 >= lm_max->n_ug) || (w4 >= lm_max->n_ug) ) {
		E_ERROR ("Bad arguments to lm_max_get_prob\n");
		return -1;
	}

	/* search the bigram w2,w3 */
	if (lm_max->use_linked_lists) {
		/* search in linked lists */

		cur_bgn = lm_max->ug_list[w2].cur_bgn;
		if ((cur_bgn != NULL) && (cur_bgn->bg.wid > w3))
			cur_bgn = lm_max->ug_list[w2].firstbgn;
		while ((cur_bgn != NULL) && (cur_bgn->bg.wid < w3))
			cur_bgn = cur_bgn->next;

		if ((cur_bgn != NULL) && (cur_bgn->bg.wid == w3)) {
			/* bigram found */
			lm_max->ug_list[w2].cur_bgn = cur_bgn;

			/* search the trigram w2,w3,w4 */
			cur_tgn = cur_bgn->cur_tgn;
			if ((cur_tgn != NULL) && (cur_tgn->tg.wid > w4))
				cur_tgn = cur_bgn->firsttgn;
			while ((cur_tgn != NULL) && (cur_tgn->tg.wid < w4))
				cur_tgn = cur_tgn->next;

			if ((cur_tgn != NULL) && (cur_tgn->tg.wid == w4)) {
				/* trigram found */
				cur_bgn->cur_tgn = cur_tgn;

				(*prob) = cur_tgn->tg.max_prob;
			}
			else
				/* no trigram w2,w3,w4 found */
				return -2;
		}
		else
			/* no bigram w2,w3 found */
			return -3;
	}
	else {
		/* search in tables */

		fbg = lm_max->ug_table[w2];
		ibg = ng = lm_max->ug_table[w2+1]-fbg;
		if (fbg < lm_max->n_bg)
			ibg = find_bg (&(lm_max->bg_table[fbg]), ng, w3);

		if (ibg < ng) {
			/* bigram found */

			/* search the trigram w2,w3,w4 */
			ftg = lm_max->bg_table[fbg+ibg].firsttg;
			itg = ng = lm_max->bg_table[fbg+ibg+1].firsttg-ftg;
			if (ftg < lm_max->n_tg)
				itg = find_tg (&(lm_max->tg_table[ftg]), ng, w4);

			if (itg < ng)
				/* trigram found */
				(*prob) = lm_max->tg_table[ftg+itg].max_prob;
			else
				/* no trigram w2,w3,w4 found */
				return -2;
		}
		else
			/* no bigram w2,w3 found */
			return -3;
	}

	return 0;
}

int lm_max_get_bowt (lm_max_t * lm_max, lmlog_t * bowt,
					 s3lmwid32_t w2, s3lmwid32_t w3)
{
	bg_max_bowt_node_t *cur_bgn;
	uint32 ng, fbg, ibg;

	/* verify parameters */
	if ( (lm_max == NULL) || (bowt == NULL)
		|| (w2 >= lm_max->n_ug) || (w3 >= lm_max->n_ug) ) {
		E_ERROR ("Bad arguments to lm_max_get_bowt\n");
		return -1;
	}

	/* search the bigram w2,w3 */
	if (lm_max->use_linked_lists) {
		/* search in linked lists */

		cur_bgn = lm_max->ug_list[w2].cur_bgn;
		if ((cur_bgn != NULL) && (cur_bgn->bg.wid > w3))
			cur_bgn = lm_max->ug_list[w2].firstbgn;
		while ((cur_bgn != NULL) && (cur_bgn->bg.wid < w3))
			cur_bgn = cur_bgn->next;

		if ((cur_bgn != NULL) && (cur_bgn->bg.wid == w3)) {
			/* bigram found */
			lm_max->ug_list[w2].cur_bgn = cur_bgn;

			(*bowt) = cur_bgn->bg.max_bowt;
		}
		else
			/* no bigram w2,w3 found */
			return -2;
	}
	else {
		/* search in tables */

		fbg = lm_max->ug_table[w2];
		ibg = ng = lm_max->ug_table[w2+1]-fbg;
		if (fbg < lm_max->n_bg)
			ibg = find_bg (&(lm_max->bg_table[fbg]), ng, w3);

		if (ibg < ng)
			/* bigram found */
			(*bowt) = lm_max->bg_table[fbg+ibg].max_bowt;
		else
			/* no bigram w2,w3 found */
			return -2;
	}

	return 0;
}

int lm_max_write_dump (const char *dmp_file, lm_max_t *lm_max)
{
	tg_max_prob_node_t * cur_tgn;
	bg_max_bowt_node_t * cur_bgn;
	bg_max_bowt_t bg_sentinel;
	uint32 i, n;
	FILE * fp;

    E_INFO ("Dumping Max LM to %s\n", dmp_file);
	fp = fopen (dmp_file, "wb");
	if (fp == NULL) {
		E_ERROR_SYSTEM ("Cannot create file %s\n", dmp_file);
		return -1;
	}


	/* write header */

    n = strlen (ng_header) + 1;

    fwrite (&n, sizeof(uint32), 1, fp);
    fwrite (ng_header, sizeof(char), n, fp);


	/* write filename */

    n = strlen(dmp_file) + 1;

    fwrite (&n, sizeof(uint32), 1, fp);
    fwrite (dmp_file, sizeof(char), n, fp);


    /* write file format description */

    for (i = 0; fmtdesc[i] != NULL; i++) {
        n = strlen(fmtdesc[i]) + 1;
		fwrite (&n, sizeof(uint32), 1, fp);
        fwrite (fmtdesc[i], sizeof(char), n, fp);
    }

    /* pad it out in order to achieve 32-bit alignment */
    n = 4-(ftell(fp) & 3);
    if (n < 4) {
		fwrite (&n, sizeof(uint32), 1, fp);
        fwrite ("!!!!", 1, n, fp);
    }
	n = 0;
	fwrite (&n, sizeof(uint32), 1, fp);


	/* write numbers of n-grams */
    fwrite (&(lm_max->n_ug), sizeof(uint32), 1, fp);
    fwrite (&(lm_max->n_bg), sizeof(uint32), 1, fp);
    fwrite (&(lm_max->n_tg), sizeof(uint32), 1, fp);


	/* write unigrams (with sentinel) */
	if (lm_max->use_linked_lists) {
		/* compute (and write) first bigram indexes */
		for (n=0, i=0; i<=lm_max->n_ug; n+=lm_max->ug_list[i++].n_bgn)
			fwrite (&n, sizeof(uint32), 1, fp);
	}
	else
		for (i=0; i<=lm_max->n_ug; i++)
			fwrite (&(lm_max->ug_table[i]), sizeof(uint32), 1, fp);

	/* write bigrams (with sentinel) */
	if (lm_max->use_linked_lists) {
		for (n=0, i=0; i<lm_max->n_ug; i++) {
			/* write bigrams in linked lists */
			for (cur_bgn=lm_max->ug_list[i].firstbgn; cur_bgn!=NULL;
				 n+=cur_bgn->n_tgn, cur_bgn=cur_bgn->next) {
				cur_bgn->bg.firsttg = n;
				fwrite (&(cur_bgn->bg), sizeof(bg_max_bowt_t), 1, fp);
			}
		}

		/* write sentinel */
		bg_sentinel.wid = BAD_S3LMWID32;
		bg_sentinel.max_bowt.l = 0;
		bg_sentinel.firsttg = n;
		fwrite (&bg_sentinel, sizeof(bg_max_bowt_t), 1, fp);
	}
	else
		for (i=0; i<=lm_max->n_bg; i++)
			fwrite (&(lm_max->bg_table[i]), sizeof(bg_max_bowt_t), 1, fp);

	/* write trigrams */
	if (lm_max->use_linked_lists) {
		for (i=0; i<lm_max->n_ug; i++)
			for (cur_bgn=lm_max->ug_list[i].firstbgn; cur_bgn!=NULL;
				 cur_bgn=cur_bgn->next)
				/* write trigrams in linked lists */
				for (cur_tgn=cur_bgn->firsttgn; cur_tgn!=NULL;
					 cur_tgn=cur_tgn->next)
					fwrite (&(cur_tgn->tg), sizeof(tg_max_prob_t), 1, fp);
	}
	else
		for (i=0; i<lm_max->n_tg; i++)
			fwrite (&(lm_max->tg_table[i]), sizeof(tg_max_prob_t), 1, fp);

	fclose (fp);
	return 0;
}

int32 lm_best_4g_score (lm_t * lm, lm_max_t * lm_max,
						s3lmwid32_t * lw, s3wid_t wn)
{
    lmlog_t score, score2;
	int tg_exist;
    uint32 p;

	/* for the moment, this function work only
	 for quadrigrams ended by three word */
	uint32 N = 4; /* maximum N-gram level, 4 for quadrigrams */
	uint32 uw = 1; /* unknown words in quadrigram */
	/* in the future, this function could be adapted for work for N-grams
	 with up to (N-1) unknown words (with N and uw in parameters) */

    
    if (N >= 2) {
        /* bigram or more */

        if (lm->n_ng[N-1] == 0)
            return (lm_ng_score(lm, N-1, &(lw[1-uw]), wn));
    }

    for (p = 1; p < N; p++)
        if (NOT_LMWID(lm, lw[p-uw]) || (lw[p-uw] >= lm->n_ng[0]))
        {
            if (N == 1)
                return MIN_PROB_F;
            if (p == 0) {
                if (N >= 3)
                    E_FATAL ("Bad lw%d argument (%d) to lm_ng_score\n",
                             p+1, lw[p-uw]);
            }
            else {
                if (p >= (N-2))
                    return lm_ng_score (lm, 1, &(lw[N-1-uw]), wn);
                else
                    return lm_ng_score (lm, N-1-p, &(lw[1+p-uw]), wn);
            }
        }

	/* search score for existing quadrigram */
	tg_exist = (lm_max_get_prob (lm_max, &score, lw[0], lw[1], lw[2]) == 0);
	if (tg_exist && lm->inclass_ugscore) {
		/* Only add within class probability if class information exists.
		Is actually ok to just add the score because if the word
		is not within-class. The returning scores will be 0. */
		score.l += lm->inclass_ugscore[wn];
	}

	/* compute score of inexisting quadrigram */
	if ( (lm_max_get_bowt (lm_max, &score2, lw[0], lw[1]) != 0)
		|| (score2.l < 0) )
		score2.l = 0;
	score2.l += lm_ng_score(lm, N-1, &(lw[1-uw]), wn);

	/* verify which score is the best */
	if ( tg_exist && (score.l > score2.l) )
		return (score.l);
	else
		return (score2.l);
}

lm_max_t *lm_max_lists_new (uint32 n_ug)
{
	lm_max_t *lm_max;

	lm_max = (lm_max_t *) ckd_calloc (1, sizeof(lm_max_t));
	if (lm_max == NULL) {
		E_ERROR_SYSTEM ("Fail to allocate a lm_max_t structure\n");
		return NULL;
	}

	lm_max->n_ug = n_ug;
	lm_max->use_linked_lists = 1;

	lm_max->ug_list = (ug_max_node_t *)
		ckd_calloc (n_ug, sizeof(ug_max_node_t));
	if (lm_max->ug_list == NULL) {
		E_ERROR_SYSTEM ("Fail to allocate memory for unigram entries list\n");
		ckd_free (lm_max);
		return NULL;
	}

	return lm_max;
}

lm_max_t * lm_max_tables_new (uint32 n_ug, uint32 n_bg, uint32 n_tg)
{
	lm_max_t *lm_max;

	lm_max = (lm_max_t *) ckd_calloc (1, sizeof(lm_max_t));
	if (lm_max == NULL) {
		E_ERROR_SYSTEM ("Fail to allocate a lm_max_t structure\n");
		return NULL;
	}

	lm_max->n_ug = n_ug;
	lm_max->n_bg = n_bg;
	lm_max->n_tg = n_tg;
	lm_max->use_linked_lists = 0;

	lm_max->ug_table = (uint32 *) ckd_calloc (n_ug+1, sizeof(uint32));
	if (lm_max->ug_table == NULL) {
		E_ERROR_SYSTEM ("Fail to allocate memory for unigram entries table\n");
		ckd_free (lm_max);
		return NULL;
	}

	lm_max->bg_table = (bg_max_bowt_t *)
		ckd_calloc (n_bg+1, sizeof(bg_max_bowt_t));
	if (lm_max->bg_table == NULL) {
		E_ERROR_SYSTEM ("Fail to allocate memory for bigram entries table\n");
		ckd_free (lm_max->ug_table);
		ckd_free (lm_max);
		return NULL;
	}

	lm_max->tg_table = (tg_max_prob_t *)
		ckd_calloc (n_tg, sizeof(tg_max_prob_t));
	if (lm_max->tg_table == NULL) {
		E_ERROR_SYSTEM ("Fail to allocate memory for trigram entries table\n");
		ckd_free (lm_max->ug_table);
		ckd_free (lm_max->bg_table);
		ckd_free (lm_max);
		return NULL;
	}

	return lm_max;
}

void lm_max_free (lm_max_t *lm_max)
{
	bg_max_bowt_node_t *cur_bg;
	bg_max_bowt_node_t *next_bg;
	tg_max_prob_node_t *cur_tg;
	tg_max_prob_node_t *next_tg;
	uint32 i;

	if (lm_max == NULL)
		return;

	/* free linked lists */
	if (lm_max->ug_list != NULL) {
		for (i=0; i<(lm_max->n_ug); i++)
			for (cur_bg=lm_max->ug_list[i].firstbgn; cur_bg!=NULL; cur_bg=next_bg) {
#ifdef _LM_MAX_DEBUG_
				printf ("word %d,%d : %f (%d)\n", i, cur_bg->bg.wid,
						cur_bg->bg.max_bowt.f, cur_bg->bg.max_bowt.l);
#endif
				for (cur_tg=cur_bg->firsttgn; cur_tg!=NULL; cur_tg=next_tg) {
					next_tg = cur_tg->next;
#ifdef _LM_MAX_DEBUG_
					printf ("word %d,%d,%d : %f (%d)\n", i,
							cur_bg->bg.wid, cur_tg->tg.wid,
							cur_tg->tg.max_prob.f, cur_tg->tg.max_prob.l);
#endif
					ckd_free (cur_tg);
				}
				next_bg = cur_bg->next;
				ckd_free (cur_bg);
			}

		ckd_free (lm_max->ug_list);
	}

	/* free N-grams tables */
	if (lm_max->ug_table != NULL)
		ckd_free (lm_max->ug_table);
	if (lm_max->bg_table != NULL)
		ckd_free (lm_max->bg_table);
	if (lm_max->tg_table != NULL)
		ckd_free (lm_max->tg_table);

	ckd_free (lm_max);
}
