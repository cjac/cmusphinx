#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <posixwin32.h>
#include "s2types.h"
#include "err.h"
#include "fbs.h"

int
main (int32 argc, char *argv[])
{
  char uttfile[4096];	/* utterance file without filename extension */
  char mfcfile[4096];
  char fsgfile[4096];
  char ctlfile[4096];
  char datadir[4096];
  char *fsgname;
  char idspec[4096];
  search_hyp_t *hyp;
  FILE *fp;
  int32 n;
  int32 new_argc;
  char **new_argv;

  new_argc = 0;
  new_argv = (char **)calloc(argc, sizeof(char *));
  ctlfile[0] = '\0';
  for (n = 0; n < argc; n++) {
    if (strcasecmp (argv[n], "-ctlfn") == 0) {
      /* Keep the control file name */
      strcpy(ctlfile, argv[n + 1]);
      /* Don't add the ctlfn argument to the new arg list, so that fbs_init
       * doesn't take over full control. */
    } else {
      new_argv[new_argc++] = argv[n]; 
      if(strcasecmp (argv[n], "-datadir") == 0) {
	strcpy(datadir, argv[n + 1]);
      }
    }
  }

  fbs_init (new_argc, new_argv);	/* Make sure -ctlfn is not in the args */
  
  if ((fp = fopen(ctlfile, "r")) == NULL)
    E_FATAL("fopen(%s,r) failed\n", ctlfile);
  
  for (n = query_ctl_offset(); n > 0; --n)
    if (fscanf (fp, "%s", uttfile) != 1)
      E_FATAL("EOF(%s) while skipping %d utts\n", ctlfile, query_ctl_offset());
  
  for (n = 0; n < query_ctl_count(); n++) {
    if (fscanf (fp, "%s", uttfile) != 1)
      break;
    
    strcpy (idspec, uttfile);
    sprintf (fsgfile, "%s/%s.fsg", datadir, uttfile);
    sprintf (mfcfile, "%s/%s", datadir, uttfile);

    /* Decode using N-gram LM */
    /*
      uttproc_set_lm("");
      hyp = run_sc_utterance (mfcfile, -1, -1, idspec);
    */
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
