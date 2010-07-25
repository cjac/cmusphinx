#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sphinxbase/ckd_alloc.h>
#include "hmm.h"
#include "mdef.h"
#include "tmat.h"
#include "logs3.h"

int
main(int argc, char *argv[])
{
	hmm_context_t *ctx;
	hmm_t h1, h2;
	mdef_t *mdef;
	tmat_t *tmat;
	int32 *senscr;
	logmath_t *logmath;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s MDEF TMAT\n", argv[0]);
		return 1;
	}

	logmath = logs3_init(1.0001, 1, 1);
	mdef = mdef_init(argv[1], 1);
	tmat = tmat_init(argv[2], 1e-5, 1, logmath);
	senscr = ckd_calloc(mdef_n_sen(mdef), sizeof(*senscr));
	ctx = hmm_context_init(mdef_n_emit_state(mdef),
			       tmat->tp, senscr,
			       mdef->sseq);

	hmm_init(ctx, &h1, 0, 0, 0);
	hmm_init(ctx, &h2, 1, 0, 0);
	hmm_dump(&h1, stdout);
	hmm_dump(&h2, stdout);

	hmm_enter(&h1, 0, 42, 0);
	hmm_vit_eval(&h1);
	hmm_dump(&h1, stdout);

	hmm_enter(&h2, 0, 69, 0);
	hmm_vit_eval(&h2);
	hmm_dump(&h2, stdout);

	hmm_enter(&h2, 0, 69, 0);
	hmm_mpx_ssid(&h2, 0) = 1;
	hmm_vit_eval(&h2);
	hmm_dump(&h2, stdout);

	return 0;
}
