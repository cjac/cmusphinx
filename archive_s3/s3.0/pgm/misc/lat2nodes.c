/*
 * lat2nodes.c -- Extract nodes from complete lattices.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 21-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libmain/misc.h>


static void lat2nodes (char *uttid, char *inlatdir, char *outlatdir,
		       char *inlatext, char *outlatext)
{
    char inlatfile[4096], outlatfile[4096], line[16384], wd[4096];
    FILE *infp, *outfp;
    int32 inpipe, outpipe;
    int32 i, k, n, sf, fef, lef;
    
    sprintf (inlatfile, "%s/%s.%s", inlatdir, uttid, inlatext);
    sprintf (outlatfile, "%s/%s.%s", outlatdir, uttid, outlatext);
    E_INFO("%s -> %s\n", inlatfile, outlatfile);

    if ((infp = fopen_compchk (inlatfile, &inpipe)) == NULL)
	E_FATAL("fopen_compchk(%s,r) failed\n", inlatfile);
    if ((outfp = fopen_comp (outlatfile, "w", &outpipe)) == NULL)
	E_FATAL("fopen_comp(%s,w) failed\n", outlatfile);
    
    
    for (;;) {
	if (fgets (line, sizeof(line), infp) == NULL)
	    E_FATAL("Premature EOF(%s)\n", inlatfile);
	
	fprintf (outfp, "%s", line);
	
	if ((sscanf (line, "%s %d", wd, &n) == 2) && (strcmp (wd, "Nodes") == 0))
	    break;
    }
    
    for (i = 0; i < n; i++) {
	if (fgets (line, sizeof(line), infp) == NULL)
	    E_FATAL("Premature EOF(%s)\n", inlatfile);
	
	if ((sscanf (line, "%d %s %d %d %d", &k, wd, &sf, &fef, &lef) != 5) || (k != i))
	    E_FATAL("Bad input line: %s\n", line);
	
	fprintf (outfp, "%d %s %d %d %d\n", k, wd, sf, fef, lef);
    }
    
    for (;;) {
	if (fgets (line, sizeof(line), infp) == NULL)
	    E_FATAL("Premature EOF(%s)\n", inlatfile);
	
	if ((sscanf (line, "%s %d", wd, &n) == 2) && (strcmp (wd, "BestSegAscr") == 0)) {
	    fprintf (outfp, "BestSegAscr 0 (NODEID ENDFRAME ASCORE)\n");
	    break;
	}
	
	fprintf (outfp, "%s", line);
    }
    
    fprintf (outfp, "#\n");
    fprintf (outfp, "Edges (FROM-NODEID TO-NODEID ASCORE)\n");
    fprintf (outfp, "End\n");
    
    fclose_comp (infp, inpipe);
    fclose_comp (outfp, outpipe);
}


main (int32 argc, char *argv[])
{
    char *inlatdir, *outlatdir, *inlatext, *outlatext;
    char uttfile[4096], uttid[4096];
    int32 sf, ef;
    
    if (argc != 5) {
	E_INFO("Usage: %s inlatdir outlatdir inlatext outlatext < ctlfile\n", argv[0]);
	exit(0);
    }
    
    inlatdir = argv[1];
    outlatdir = argv[2];
    inlatext = argv[3];
    outlatext = argv[4];
    
    while (_ctl_read (stdin, uttfile, &sf, &ef, uttid) >= 0)
	lat2nodes (uttid, inlatdir, outlatdir, inlatext, outlatext);
}
