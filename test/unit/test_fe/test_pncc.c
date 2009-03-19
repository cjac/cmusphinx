#include <stdio.h>
#include <string.h>

#include "fe.h"
#include "cmd_ln.h"
#include "ckd_alloc.h"
#include "err.h"

#include "test_macros.h"

int
main(int argc, char *argv[])
{
	static const arg_t fe_args[] = {
		waveform_to_cepstral_command_line_macro(),
		{ NULL, 0, NULL, NULL }
	};
	FILE *raw;
	cmd_ln_t *config;
	fe_t *fe;
	int16 *buf;
	mfcc_t **cep;
	long nsamp;
	int32 nfr;

	TEST_ASSERT(config = cmd_ln_parse_r(NULL, fe_args, argc, argv, FALSE));
	cmd_ln_set_boolean_r(config, "-pncc", TRUE);
	TEST_ASSERT(fe = fe_init_auto_r(config));

	TEST_ASSERT(raw = fopen(DATADIR "/chan3.raw", "rb"));
	TEST_EQUAL(0, fe_start_utt(fe));

	fseek(raw, 0, SEEK_END);
	nsamp = ftell(raw) / 2;
	fseek(raw, 0, SEEK_SET);
	buf = ckd_calloc(nsamp, 2);
	if (fread(buf, 2, nsamp, raw) != nsamp)
		E_FATAL_SYSTEM("Failed to read audio data");

	TEST_EQUAL(0, fe_process_utt(fe, buf, nsamp, &cep, &nfr));

	ckd_free(buf);
	ckd_free_2d(cep);
	fclose(raw);
	fe_free(fe);

	return 0;
}
