#include <pocketsphinx.h>
#include <stdio.h>
#include <string.h>

#include "pocketsphinx_internal.h"
#include "test_macros.h"
#include "ps_test.c"

int
main(int argc, char *argv[])
{
	ps_decoder_t *ps;
	ps_nbest_t *nbest;
	cmd_ln_t *config;
	FILE *rawfh;
	char const *hyp;
	char const *uttid;
	int32 score, n;

	TEST_ASSERT(config =
		    cmd_ln_init(NULL, ps_args(), TRUE,
				"-hmm", MODELDIR "/hmm/wsj1",
				"-lm", MODELDIR "/lm/wsj/wlist5o.3e-7.vp.tg.lm.DMP",
				"-dict", MODELDIR "/lm/cmudict.0.6d",
				"-fwdtree", "yes",
				"-fwdflat", "yes",
				"-bestpath", "yes",
				"-input_endian", "little",
				"-samprate", "16000", NULL));
	TEST_ASSERT(ps = ps_init(config));
	TEST_ASSERT(rawfh = fopen(DATADIR "/goforward.raw", "rb"));
	ps_decode_raw(ps, rawfh, "goforward", -1);
	fclose(rawfh);
	hyp = ps_get_hyp(ps, &score, &uttid);
	printf("BESTPATH (%s): %s (%d)\n", uttid, hyp, score);
	TEST_ASSERT(nbest = ps_nbest(ps, 0, -1, NULL, NULL));
	n = 1;
	while (ps_nbest_next(nbest)) {
		ps_seg_t *seg;
		hyp = ps_nbest_hyp(nbest, &score);
		printf("NBEST %d: %s (%d)\n", n, hyp, score);
		for (seg = ps_nbest_seg(nbest, &score); seg;
		     seg = ps_seg_next(seg)) {
			char const *word;
			int sf, ef;

			word = ps_seg_word(seg);
			ps_seg_frames(seg, &sf, &ef);
			printf("%s %d %d\n", word, sf, ef);
		}
		if (n == 10)
			break;
		++n;
	}
	ps_nbest_free(nbest);
	ps_free(ps);
	return 0;
}
