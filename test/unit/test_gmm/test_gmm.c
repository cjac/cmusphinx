#include <gau_cb.h>
#include <gau_mix.h>
#include <strfuncs.h>

#include <stdio.h>

int
main(int argc, char *argv[])
{
	gau_cb_t *cb;
	gau_mix_t *mix;

#if 0
	cb = gau_cb_read(NULL, HMMDIR "/means", HMMDIR "/variances");
	mix = gau_mix_read(NULL, HMMDIR "/mixture_weights");

	gau_cb_free(cb);
	gau_mix_free(mix);
#endif

	return 0;
}
