#include <pocketsphinx.h>
#include <stdio.h>
#include <string.h>

#include "pocketsphinx_internal.h"
#include "test_macros.h"

static const mfcc_t prior[13] = {
	FLOAT2MFCC(37.03),
	FLOAT2MFCC(-1.01),
	FLOAT2MFCC(0.53),
	FLOAT2MFCC(0.49),
	FLOAT2MFCC(-0.60),
	FLOAT2MFCC(0.14),
	FLOAT2MFCC(-0.05),
	FLOAT2MFCC(0.25),
	FLOAT2MFCC(0.37),
	FLOAT2MFCC(0.58),
	FLOAT2MFCC(0.13),
	FLOAT2MFCC(-0.16),
	FLOAT2MFCC(0.17)
};

static void
ps_backtrace(ps_decoder_t *ps)
{
	ps_seg_t *seg;
	int32 score;

	for (seg = ps_seg_iter(ps, &score); seg;
	     seg = ps_seg_next(seg)) {
		char const *word;
		int sf, ef;
		int32 post, lscr, ascr, lback;

		word = ps_seg_word(seg);
		ps_seg_frames(seg, &sf, &ef);
		post = ps_seg_prob(seg, &ascr, &lscr, &lback);
		printf("%s (%d:%d) P(w|o) = %f ascr = %d lscr = %d lback = %d\n", word, sf, ef,
		       logmath_exp(ps_get_logmath(ps), post), ascr, lscr, lback);
	}
}

static int
ps_decoder_test(cmd_ln_t *config, char const *sname, char const *expected)
{
	ps_decoder_t *ps;
	FILE *rawfh;
	int16 *buf;
	size_t nread;
	size_t nsamps;
	int32 score, prob;
	char const *hyp;
	char const *uttid;
	double n_speech, n_cpu, n_wall;

	TEST_ASSERT(ps = ps_init(config));

	/* Test it first with pocketsphinx_decode_raw() */
	TEST_ASSERT(rawfh = fopen(DATADIR "/goforward.raw", "rb"));

	/* Test it with ps_process_raw() */
	cmn_prior_set(ps->acmod->fcb->cmn_struct, prior);
	fseek(rawfh, 0, SEEK_END);
	nsamps = ftell(rawfh) / sizeof(*buf);
	fseek(rawfh, 0, SEEK_SET);
	TEST_EQUAL(0, ps_start_utt(ps, NULL));
	nsamps = 2048;
	buf = ckd_calloc(nsamps, sizeof(*buf));
	while (!feof(rawfh)) {
		nread = fread(buf, sizeof(*buf), nsamps, rawfh);
		ps_process_raw(ps, buf, nread, FALSE, FALSE);
		hyp = ps_get_hyp(ps, &score, &uttid);
		prob = ps_get_prob(ps, &uttid);
		printf("%s (%s): %s (%d, %d)\n", sname, uttid, hyp, score, prob);
		ps_backtrace(ps);
	}
	TEST_EQUAL(0, ps_end_utt(ps));
	hyp = ps_get_hyp(ps, &score, &uttid);
	prob = ps_get_prob(ps, &uttid);
	printf("%s (%s): %s (%d, %d)\n", sname, uttid, hyp, score, prob);
	TEST_EQUAL(0, strcmp(uttid, "000000000"));
	TEST_EQUAL(0, strcmp(hyp, expected));
	ps_backtrace(ps);
	ps_get_utt_time(ps, &n_speech, &n_cpu, &n_wall);
	printf("%.2f seconds speech, %.2f seconds CPU, %.2f seconds wall\n",
	       n_speech, n_cpu, n_wall);
	printf("%.2f xRT (CPU), %.2f xRT (elapsed)\n",
	       n_cpu / n_speech, n_wall / n_speech);

	fclose(rawfh);
	ps_free(ps);
	ckd_free(buf);

	return 0;
}

int
main(int argc, char *argv[])
{
	cmd_ln_t *config;

	TEST_ASSERT(config =
		    cmd_ln_init(NULL, ps_args(), TRUE,
				"-hmm", MODELDIR "/hmm/wsj1",
				"-lm", MODELDIR "/lm/wsj/wlist5o.3e-7.vp.tg.lm.DMP",
				"-dict", MODELDIR "/lm/cmudict.0.6d",
				"-fwdtree", "yes",
				"-fwdflat", "no",
				"-bestpath", "yes",
				"-input_endian", "little",
				"-samprate", "16000", NULL));
	return ps_decoder_test(config, "BESTPATH", "GO FORWARD TEN YEARS");
}
