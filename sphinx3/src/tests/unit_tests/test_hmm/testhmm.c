#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "err.h"
#include "hmm.h"
#include "mdef.h"
#include "tmat.h"

int
main(int argc, char *argv[])
{
	hmm_context_t *ctx;
	hmm_t h1, h2;
	mdef_t *mdef;
	tmat_t *tmat;
	int32 *senscr;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s MDEF TMAT SENID\n", argv[0]);
		return 1;
	}

	mdef = mdef_init(argv[1], 1);
	tmat = tmat_init(argv[2], 1e-5, 1);
	senscr = ckd_calloc(mdef_n_sen(mdef), sizeof(*senscr));
	ctx = hmm_context_init(mdef_n_emit_state(mdef),
			       0, tmat->tp, senscr,
			       mdef->sseq);

	hmm_clear(ctx, &h1);
	hmm_clear(ctx, &h2);
	return 0;
}
