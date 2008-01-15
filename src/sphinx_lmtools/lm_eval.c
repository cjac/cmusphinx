/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2008 Carnegie Mellon University.  All rights 
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
/**
 * \file lm_eval.c
 * Language model evaluation tool.
 */
#include <logmath.h>
#include <ngram_model.h>
#include <cmd_ln.h>
#include <ckd_alloc.h>
#include <err.h>
#include <strfuncs.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

static const arg_t defn[] = {
  { "-help",
    ARG_BOOLEAN,
    "no",
    "Shows the usage of the tool"},

  { "-logbase",
      ARG_FLOAT64,
      "1.0001",
      "Base in which all log-likelihoods calculated" },

  { "-lm",
    ARG_STRING,
    NULL,
    "Language model file"},

  { "-lsn",
    ARG_STRING,
    NULL,
    "Transcription file to evaluate"},

  { "-text",
    ARG_STRING,
    "Text string to evaluate"},

  { "-mmap",
    ARG_BOOLEAN,
    "no",
    "Use memory-mapped I/O for reading binary LM files"},

  { "-lw",
    ARG_FLOAT32,
    "1.0",
    "Language model weight" },

  { "-wip",
    ARG_FLOAT32,
    "1.0",
    "Word insertion probability" },

  { "-uw",
    ARG_FLOAT32,
    "1.0",
    "Unigram probability weight (interpolated with uniform distribution)"},

  /* FIXME: Support -lmstartsym, -lmendsym, -lmctlfn, -ctl_lm */
  { NULL, 0, NULL, NULL }
};

static int
calc_entropy(ngram_model_t *lm, char **words, int32 n,
	     int32 *out_n_ccs, int32 *out_n_oovs)
{
	int32 *wids;
	int32 startwid;
	int32 i, ch, nccs, noovs;

	/* Reverse this array into an array of word IDs. */
	wids = ckd_calloc(n, sizeof(*wids));
	for (i = 0; i < n; ++i)
		wids[n-i-1] = ngram_wid(lm, words[i]);
	/* Skip <s> as it's a context cue (HACK, this should be configurable). */
	startwid = ngram_wid(lm, "<s>");

	/* Now evaluate the list of words in reverse using the
	 * remainder of the array as the history. */
	ch = noovs = nccs = 0;
	for (i = 0; i < n; ++i) {
		int32 n_used;
		int32 prob;

		/* Skip <s> as it's a context cue (HACK, this should be configurable). */
		if (wids[i] == startwid) {
			++nccs;
			continue;
		}
		/* Skip and count OOVs. */
		if (wids[i] == NGRAM_UNKNOWN_WID) {
			++noovs;
			continue;
		}
		/* Sum up information for each N-gram */
		prob = ngram_ng_score(lm,
				      wids[i], wids + i + 1,
				      n - i - 1, &n_used);
		ch -= prob;
	}

	if (out_n_ccs) *out_n_ccs = nccs;
	if (out_n_oovs) *out_n_oovs = noovs;

	/* Calculate cross-entropy CH = - 1/N sum log P(W|H) */
	return ch / n;
}

static void
evaluate_file(ngram_model_t *lm, logmath_t *lmath, const char *lsnfn)
{
	FILE *fh;
	char line[256];
	int32 nccs, noovs, nwords;
	float64 ch, log_to_log2;;

	if ((fh = fopen(lsnfn, "r")) == NULL)
		E_FATAL_SYSTEM("failed to open transcript file %s", lsnfn);

	/* We have to keep ch in floating-point to avoid overflows, so
	 * we might as well use log2. */
	log_to_log2 = log(logmath_get_base(lmath)) / log(2);
	nccs = noovs = nwords = 0;
	ch = 0.0;
	while (fgets(line, sizeof(line), fh)) {
		char **words;
		int32 n, tmp_ch, tmp_noovs, tmp_nccs;

		n = str2words(line, NULL, 0);
		if (n < 0)
			E_FATAL("str2words(line, NULL, 0) = %d, should not happen\n", n);
		if (n == 0) /* Do nothing! */
			continue;
		words = ckd_calloc(n, sizeof(*words));
		str2words(line, words, n);

		/* Remove any utterance ID (FIXME: has to be a single "word") */
		if (words[n-1][0] == '('
		    && words[n-1][strlen(words[n-1])-1] == ')')
			n = n - 1;

		tmp_ch = calc_entropy(lm, words, n, &tmp_nccs, &tmp_noovs);

		ch += (float64) tmp_ch * n * log_to_log2;
		nccs += tmp_nccs;
		noovs += tmp_noovs;
		nwords += n;
		
		ckd_free(words);
	}

	ch /= nwords;
	printf("cross-entropy: %f bits\n", ch);

	/* Calculate perplexity pplx = exp CH */
	printf("perplexity: %f\n", pow(2.0, ch));

	/* Report OOVs and CCs */
	printf("%d words evaluated\n", nwords);
	printf("%d OOVs, %d context cues removed\n",
	       noovs, nccs);
}

static void
evaluate_string(ngram_model_t *lm, logmath_t *lmath, const char *text)
{
	char *textfoo;
	char **words;
	int32 n, ch, noovs, nccs;

	/* Split it into an array of strings. */
	textfoo = ckd_salloc(text);
	n = str2words(textfoo, NULL, 0);
	if (n < 0)
		E_FATAL("str2words(textfoo, NULL, 0) = %d, should not happen\n", n);
	if (n == 0) /* Do nothing! */
		return;
	words = ckd_calloc(n, sizeof(*words));
	str2words(textfoo, words, n);

	ch = calc_entropy(lm, words, n, &nccs, &noovs);

	printf("input: %s\n", text);
	printf("cross-entropy: %f bits\n",
	       ch * log(logmath_get_base(lmath)) / log(2));

	/* Calculate perplexity pplx = exp CH */
	printf("perplexity: %f\n", logmath_exp(lmath, ch));

	/* Report OOVs and CCs */
	printf("%d words evaluated\n", n);
	printf("%d OOVs, %d context cues removed\n",
	      noovs, nccs);

	ckd_free(textfoo);
	ckd_free(words);
}

int
main(int argc, char *argv[])
{
	cmd_ln_t *config;
	ngram_model_t *lm = NULL;
	logmath_t *lmath;
	const char *lmfn, *lsnfn, *text;

	if ((config = cmd_ln_parse_r(NULL, defn, argc, argv, TRUE)) == NULL)
		return 1;

	/* Create log math object. */
	if ((lmath = logmath_init
	     (cmd_ln_float64_r(config, "-logbase"), 0, 0)) == NULL) {
		E_FATAL("Failed to initialize log math\n");
	}

	/* Load the language model. */
	lmfn = cmd_ln_str_r(config, "-lm");

	if (lmfn == NULL
	    || (lm = ngram_model_read(config, lmfn,
				      NGRAM_AUTO, lmath)) == NULL) {
		E_FATAL("Failed to load language model from %s\n",
			cmd_ln_str_r(config, "-lm"));
	}

	/* Now evaluate some text. */
	lsnfn = cmd_ln_str_r(config, "-lsn");
	text = cmd_ln_str_r(config, "-text");
	if (lsnfn) {
		evaluate_file(lm, lmath, lsnfn);
	}
	else if (text) {
		evaluate_string(lm, lmath, text);
	}

	return 0;
}
