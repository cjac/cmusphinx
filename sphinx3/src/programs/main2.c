
#include <sys/stat.h>
#include <fcntl.h>
#include "live2.h"
#include "ad.h"

#define BUFSIZE 4096
int
main(int argc, char **argv)
{
  live_decoder_t decoder;
  FILE *infile;
  ad_rec_t *ad;
  char *hypstr;
  int16 samples[BUFSIZE];
  int32 num_samples;
  int32 tot_samples = 0;

  if (argc < 2) {
    printf("Usage:  livedecoder2 [ARGS] INPUT_FILE\n");
    return -1;
  }

  if (ld_init(&decoder, argc - ((argc - 1) % 2), argv)) {
    printf("ld_init() failed.\n");
    return -1;
  }

  if (ld_utt_begin(&decoder, 0)) {
    printf("ld_utt_begin() failed.\n");
    return -1;
  }

  if (argc % 2) {
    /* record for 5 seconds */
    tot_samples = cmd_ln_int32 ("-samprate") * 5;
    ad = ad_open_sps(cmd_ln_int32 ("-samprate"));
    ad_start_rec(ad);
    while (tot_samples > 0) {
      num_samples = ad_read(ad, samples,
			    tot_samples > BUFSIZE ? BUFSIZE : tot_samples);
      if (num_samples > 0) {
	if (ld_utt_proc_raw(&decoder, samples, num_samples) < 0) {
	  printf("ld_utt_proc_raw() returned unexpectedly.\n");
	  return -1;
	}
	tot_samples -= num_samples;
      }
    }
    ad_stop_rec(ad);
    ad_close(ad);
  }
  else {
    infile = fopen(argv[1], "rb");
    while (!feof(infile)) {
      num_samples = fread(samples, sizeof(int16), BUFSIZE, infile);
      if (num_samples > 0) {
	if (ld_utt_proc_raw(&decoder, samples, num_samples) < 0) {
	  printf("ld_utt_proc_raw() returned unexpectedly.\n");
	  return -1;
	}
      }
    }
    fclose(infile);
  }
	
  if (ld_utt_end(&decoder)) {
    printf("ld_utt_end() failed.\n");
    return -1;
  }

  ld_utt_hyps(&decoder, &hypstr, 0);
  printf("decoder returned:\n%s\n", hypstr);

  return 0;
}
