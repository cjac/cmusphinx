
#include <sys/stat.h>
#include <fcntl.h>
#include "live2.h"
#include "cmd_ln_args.h"

int
main(int argc, char **argv)
{
	live_decoder_t decoder;
	FILE *infile;
	char *hypstr;
	int16 samples[4096];
	int32 num_samples;

	if (argc <= 2) {
		printf("Usage:  livedecoder2 [ARGS] INPUT_FILE\n");
		return -1;
	}

	if (ld_init(&decoder, argc - 1, argv)) {
		printf("ld_init() failed.\n");
		return -1;
	}

	if (ld_utt_begin(&decoder, 0)) {
		printf("ld_utt_begin() failed.\n");
		return -1;
	}

	printf("decoder started.\n");
	infile = fopen(argv[argc - 1], "rb");
	while (!feof(infile)) {
		num_samples = fread(samples, sizeof(int16), 4096, infile);
		if (num_samples > 0) {
			if (ld_utt_proc_raw(&decoder, samples, num_samples) < 0) {
				printf("ld_utt_proc_raw() returned unexpectedly.\n");
				return -1;
			}
		}
	}
	fclose(infile);
	
	if (ld_utt_end(&decoder)) {
		printf("ld_utt_end() failed.\n");
		return -1;
	}

	ld_utt_hypstr(&decoder, &hypstr);
	printf("decoder returned:\n%s\n", &hypstr);

	return 0;
}
