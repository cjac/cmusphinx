/*
 * v8seg2ascii.c -- Turn a .v8_seg file into ASCII form phone segmentation.
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
 * 20-Jan-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>
#include <libmain/mdef.h>


static mdef_t *mdef;

typedef struct {
    int32 pid;
    int32 sf;
    int32 ef;
    int32 scr;
} seg_t;
static seg_t *pseg;
#define MAX_SEG		10000


/*
 * Load .v8_seg file into a seg_t array and return #segments read.
 */
static int32 load_segfile (char *segfile, seg_t *pseg)
{
    int32 nseg, k, sf, ef, pid;
    uint16 p, prevp;
    FILE *fp;
    
    if ((fp = fopen (segfile, "rb")) == NULL) {
	E_ERROR("fopen(%s,rb) failed\n", segfile);
	return -1;
    }
    
    assert (fread (&k, sizeof(int32), 1, fp) == 1);
    prevp = p;
    SWAP_INT32(&k);
    
    sf = ef = pid = -1;
    nseg = 0;

    for (; k > 0; --k) {
	if (nseg >= MAX_SEG-2) {
	    E_ERROR("Increase MAX_SEG\n");
	    return -1;
	}
	
	if (fread (&p, sizeof(uint16), 1, fp) != 1) {
	    if (k == 1)		/* Special case bug-handling for one missed frame */
		p = prevp;
	    else {
		E_ERROR("fread() failed; %d frames remaining\n", k);
		return -1;
	    }
	}
	prevp = p;
	
	SWAP_INT16(&p);
	
	if (p & 0x8000) {
	    if (pid >= 0) {
		assert (ef > sf);

		pseg[nseg].pid = pid;
		pseg[nseg].sf = sf;
		pseg[nseg].ef = ef;
		nseg++;
	    }
	    
	    p &= 0x7fff;
	    pid = p / 5;
	    assert (IS_CIPID(pid));
	    
	    sf = ef+1;
	}
	ef++;
    }

    if (nseg >= MAX_SEG-2) {
	E_ERROR("Increase MAX_SEG\n");
	return -1;
    }
    
    if (pid >= 0) {
	assert (ef > sf);

	pseg[nseg].pid = pid;
	pseg[nseg].sf = sf;
	pseg[nseg].ef = ef;
	nseg++;
    }
    pseg[nseg].pid = -1;
    
    fclose (fp);
    
    return nseg;
}


static void process_utt (char *segfile, char *outdir, char *uttid)
{
    FILE *fp;
    char file[4096];
    int32 n_pseg;
    int32 i;

    if ((n_pseg = load_segfile (segfile, pseg)) <= 0) {
	E_ERROR("load_segfile(%s) failed\n", segfile);
	return;
    }
    
    sprintf (file, "%s/%s.phseg", outdir, uttid);
    if ((fp = fopen (file, "w")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	fp = stdout;
    }

#if 0
    for (i = 0; i < n_pseg; i++) {
	fprintf (fp, "%5d %3d %s\n",
		 pseg[i].sf,
		 pseg[i].ef - pseg[i].sf + 1,
		 mdef_ciphone_str (mdef, pseg[i].pid));
    }
#else
    for (i = 0; i < n_pseg; i++) {
	fprintf (fp, "%s %d %d ",
		 mdef_ciphone_str (mdef, pseg[i].pid),
		 pseg[i].sf,
		 pseg[i].ef - pseg[i].sf + 1);
    }
    fprintf (fp, "(%s)\n", uttid);
#endif

    fflush (fp);
    if (fp != stdout)
	fclose (fp);
}


static void build_uttid (char *line, char *uttid)
{
    int32 l;
    
    for (l = strlen(line)-1; (l >= 0) && (line[l] != '/'); --l);
    strcpy (uttid, line+l+1);
}


main (int32 argc, char *argv[])
{
    char *mdeffile, *segdir, *outdir;
    char uttid[4096], segfile[4096], outfile[4096], line[16380];
    int32 l;
    
    if (argc != 4)
	E_FATAL("Usage: %s mdeffile segdir outdir < ctlfile\n", argv[0]);
    
    mdeffile = argv[1];
    segdir = argv[2];
    outdir = argv[3];
    
    mdef = mdef_init (mdeffile);

    pseg = (seg_t *) ckd_calloc (MAX_SEG, sizeof(seg_t));
    
    while (scanf ("%s", uttid) == 1) {
	sprintf (segfile, "%s/%s", segdir, uttid);
	build_uttid (segfile, uttid);
	strcat (segfile, ".v8_seg");
	
	E_INFO("Utt %s\n", uttid);
	
	process_utt (segfile, outdir, uttid);
    }
}
