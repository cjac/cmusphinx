
#include <sys/stat.h>
#include <fcntl.h>
#include "live2.h"
#include "args.h"
#include "ad.h"

#define BUFSIZE 4096

HANDLE startEvent;
HANDLE finishEvent;
live_decoder_t decoder;

DWORD WINAPI
process_thread(LPVOID aParam)
{
  ad_rec_t *in_ad = 0;
  int16 samples[BUFSIZE];
  int32 num_samples;

  WaitForSingleObject(startEvent, INFINITE);

  in_ad = ad_open_sps(cmd_ln_int32 ("-samprate"));
  ad_start_rec(in_ad);

  while (WaitForSingleObject(finishEvent, 0) == WAIT_TIMEOUT) {
    num_samples = ad_read(in_ad, samples, BUFSIZE);
    if (num_samples > 0) {
      if (ld_utt_proc_raw(&decoder, samples, num_samples) < 0) {
	printf(">>>>> ld_utt_proc_raw() returned unexpectedly.\n");
	return -1;
      }
    }
  }

  ad_stop_rec(in_ad);
  ad_close(in_ad);

  if (ld_utt_end(&decoder)) {
    printf(">>>>> ld_utt_end() failed.\n");
    return -1;
  }

  return 0;
}

int
main(int argc, char **argv)
{
  HANDLE thread;
  char buffer[1024];
  char *hypstr;

  /////////////////////////////////////////////////////////////////////////////
  // Initializing
  //
  if (argc != 2) {
    printf("Usage:  livedecoder2 config_file \n");
    return -1;
  }

  if (cmd_ln_parse_file(arg_def, argv[1])) {
    printf(">>>> cmd_ln_parse_file failed.\n");
    return -1;
  }

  if (ld_init(&decoder)) {
    printf(">>>>> ld_init() failed.\n");
    return -1;
  }

  if (ld_utt_begin(&decoder, 0)) {
    printf(">>>>> ld_utt_begin() failed.\n");
    return -1;
  }

  startEvent = CreateEvent(NULL,
			   TRUE,
			   FALSE,
			   "StartEvent");

  finishEvent = CreateEvent(NULL,
			    TRUE,
			    FALSE,
			    "FinishEvent");

  thread = CreateThread(NULL,
			0,
			process_thread,
			NULL,
			0,
			NULL);

  /////////////////////////////////////////////////////////////////////////////
  // Wait for some user input, then signal the processing thread to start
  // recording/decoding
  //
  printf("press ENTER to start recording\n");
  fgets(buffer, 1024, stdin);
  SetEvent(startEvent);

  /////////////////////////////////////////////////////////////////////////////
  // Wait for some user input again, then signal the processing thread to end
  // recording/decoding
  //
  printf("press ENTER to finish recording\n");
  fgets(buffer, 1024, stdin);
  SetEvent(finishEvent);

  /////////////////////////////////////////////////////////////////////////////
  // Wait for the working thread to join
  //
  WaitForSingleObject(thread, INFINITE);

  /////////////////////////////////////////////////////////////////////////////
  // Print the decoding output
  //
  if (ld_utt_hyps(&decoder, &hypstr, 0)) {
    printf(">>>>> ld_utt_hyps() failed\n");
  }
  else {
    printf(">>>>> decoder returned:\n%s\n", hypstr);
  }

  ld_finish(&decoder);

  return 0;
}
