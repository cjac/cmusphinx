#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <s3.h>
#include <libutil/libutil.h>
#include <libfbs/misc.h>


static void usagemsg (char *pgm)
{
    E_FATAL("Usage: %s ctlfile nbestdir rescoredir matchfile langwt wordinspen\n", pgm);
}


main (int32 argc, char *argv[])
{
    char *ctlfile, *nbestdir, *rescoredir, *matchfile, *lwarg, *wiparg, *lp;
    float32 lw, wip;
    FILE *ctlfp, *rfp, *mfp;
    char ctlspec[1024], uttid[1024], file[1024], line[1024];
    int32 sf, ef, nhyp, ascr, tscr, best, bestid;
    hyp_t **hyplist, *h;
    int32 i;
    
    if (argc != 7)
	usagemsg (argv[0]);

    ctlfile = argv[1];
    nbestdir = argv[2];
    rescoredir = argv[3];
    matchfile = argv[4];
    lwarg = argv[5];
    wiparg = argv[6];
    
    if ((sscanf (lwarg, "%f", &lw) != 1) || (sscanf (wiparg, "%f", &wip) != 1))
	usagemsg (argv[0]);

    ctlfp = ctlfile_open (ctlfile);

    if ((mfp = fopen (matchfile, "w")) == NULL) {
	E_ERROR("fopen(%s,w) failed; using stdout\n", matchfile);
	mfp = stdout;
    }
    
    nhyp = 0;
    rfp = NULL;
    
    while (ctlfile_next (ctlfp, ctlspec, &sf, &ef, uttid) == 0) {
	fflush (mfp);

	if (nhyp > 0)
	    nbestlist_free (hyplist, nhyp);
	if (rfp)
	    fclose (rfp);
	
	if ((nhyp = nbestfile_load (nbestdir, uttid, &hyplist)) <= 0) {
	    E_ERROR ("Nbest load failed; cannot rescore %s\n", uttid);
	    fprintf (mfp, "%s T 0 A 0 L 0 (null)\n", uttid);
	    
	    continue;
	}
	
	sprintf (file, "%s/%s.alpha", rescoredir, uttid);
	if ((rfp = fopen (file, "r")) == NULL) {
	    E_ERROR ("%s: Cannot load alpha score file %s\n", uttid, file);
	    fprintf (mfp, "%s T 0 A 0 L 0 (null)\n", uttid);
	    
	    continue;
	}
	
	/* Skip initial comments */
	while (((lp = fgets (line, sizeof(line), rfp)) != NULL) && (line[0] == '#'));
	
	best = (int32) 0x80000000;

	for (i = 0; i < nhyp; i++) {
	    if (! lp) {
		E_ERROR ("%s: Premature EOF(%s)\n", uttid, file);
		break;
	    }
	    
	    if (sscanf (line, "%d", &ascr) != 1) {
		E_ERROR ("%s: Bad alpha score line: %s\n", uttid, line);
		break;
	    }
	    
	    tscr = ascr + hyplist[i]->lscr * lw;
	    if (tscr > best) {
		best = tscr;
		bestid = i;
	    }
	    
	    lp = fgets (line, sizeof(line), rfp);
	}
	
	if (i < nhyp)
	    fprintf (mfp, "%s T 0 A 0 L 0 (null)\n", uttid);
	else {
	    E_INFO ("%s: best hyp %d\n", uttid, bestid);
	    
	    fprintf (mfp, "%s T %d A %d L %d", uttid, best,
		     (int32) (best - hyplist[bestid]->lscr * lw),
		     (int32) (hyplist[bestid]->lscr * lw));

	    for (h = hyplist[bestid]; h; h = h->next)
		fprintf (mfp, " %d %s", h->sf, h->word);
	    fprintf (mfp, "\n");
	}
    }

    ctlfile_close (ctlfp);
    fclose (mfp);
}
