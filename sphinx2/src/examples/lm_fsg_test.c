#include <stdio.h>
#include <string.h>
#include "s2types.h"
#include "err.h"
#include "fbs.h"


#define DATADIR		"/usr2/rkm/sphinxtest/ctl"
#define CTLFILE		"/usr2/rkm/sphinxtest/ctl/h1_et_94-g.ctl"

int
main (int32 argc, char *argv[])
{
  char uttfile[4096];	/* utterance file without filename extension */
  char mfcfile[4096];
  char fsgfile[4096];
  char *fsgname;
  char idspec[4096];
  search_hyp_t *hyp;
  FILE *fp;
  int32 n;
  
  fbs_init (argc, argv);	/* Make sure -ctlfn is not in the args */
  
  if ((fp = fopen(CTLFILE, "r")) == NULL)
    E_FATAL("fopen(%s,r) failed\n", CTLFILE);
  
  for (n = query_ctl_offset(); n > 0; --n)
    if (fscanf (fp, "%s", uttfile) != 1)
      E_FATAL("EOF(%s) while skipping %d utts\n", CTLFILE, query_ctl_offset());
  
  for (n = 0; n < query_ctl_count(); n++) {
    if (fscanf (fp, "%s", uttfile) != 1)
      break;
    
    strcpy (idspec, uttfile);
    sprintf (fsgfile, "%s/%s.fsg", DATADIR, uttfile);
    sprintf (mfcfile, "%s/%s", DATADIR, uttfile);

    /* Decode using N-gram LM */
    uttproc_set_lm("");
    hyp = run_sc_utterance (mfcfile, -1, -1, idspec);

    /* Decode using FSG */
    if ((fsgname = uttproc_load_fsgfile (fsgfile)) == NULL)
      E_FATAL("Error loading FSG file '%s'\n", fsgfile);
    if (uttproc_set_fsg (fsgname) < 0)
      E_FATAL("Error setting current FSG to '%s'\n", fsgname);
    
    hyp = run_sc_utterance (mfcfile, -1, -1, idspec);
    uttproc_del_fsg (fsgname);
  }
  
  fclose (fp);
  
  fbs_end ();
  return 0;
}


#if 0
Compile in this directory using:
gcc  -g -O2 -Wall -I../../include -o sphinx2-lm_fsg lm_fsg_test.c ../../src/libsphinx2/.libs/libsphinx2.a ../../src/libsphinx2fe/.libs/libsphinx2fe.a ../../src/libsphinx2ad/.libs/libsphinx2ad.a -lm
#endif
