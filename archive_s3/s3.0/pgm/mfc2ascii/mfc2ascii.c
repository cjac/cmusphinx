/*
 * mfc2ascii.c -- Print out an mfc cepstrum file in readable ascii format.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1997 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 21-Jul-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added temporal smoothing (-lpf) option.
 * 
 * 14-May-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <libutil/libutil.h>
#include <libio/libio.h>

#include <sys/types.h>
#include <sys/stat.h>
#if (! WIN32)
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/param.h>
#else
#include <fcntl.h>
#endif


static void usagemsg (char *pgm)
{
    printf ("Usage: %s [-lpf(smooth)] [-numbered] [-s<startdim(0..12)>] [-e<enddim(0..12)>] <mfcfile>\n",
	    pgm);
    exit(-1);
}


main (int argc, char *argv[])
{
    struct stat statbuf;
    char *file;
    FILE *fp;
    int32 byterev;
    int32 i, n, n_float32, fr, numbered, sd, ed, smooth;
    float32 cep[13], cep_1[13], cep_2[13], c;	/* Hack!! Hardwired 13 */
    
    if (argc < 2)
	usagemsg (argv[0]);

    numbered = 0;
    file = NULL;
    sd = -1;	/* Start and end dimensions to output */
    ed = -1;
    smooth = 0;
    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'n':
		if (numbered)
		    usagemsg (argv[0]);
		numbered = 1;
		break;
	    case 'l':
		if (smooth)
		    usagemsg (argv[0]);
		smooth = 1;
		break;
	    case 's':
		if ((sd >= 0) || (sscanf (&(argv[i][2]), "%d", &sd) != 1) ||
		    (sd < 0) || (sd > 12))
		    usagemsg (argv[0]);
		break;
	    case 'e':
		if ((ed >= 0) || (sscanf (&(argv[i][2]), "%d", &ed) != 1) ||
		    (ed < 0) || (ed > 12))
		    usagemsg (argv[0]);
		break;
	    default:
		usagemsg (argv[0]);
		break;
	    }
	} else {
	    if (file)
		usagemsg (argv[0]);
	    file = argv[i];
	}
    }
    if (! file)
	usagemsg (argv[0]);
    if (sd < 0)
	sd = 0;
    if (ed < 0)
	ed = 12;
    
    if (stat (file, &statbuf) != 0) {
	E_ERROR("stat(%s) failed\n", file);
	return -1;
    }
    
    if ((fp = fopen(file, "rb")) == NULL) {
	E_ERROR("fopen(%s,rb) failed\n", file);
	return -1;
    }
    
    /* Read #floats in header */
    if (fread_retry (&n_float32, sizeof(int32), 1, fp) != 1) {
	fclose (fp);
	return -1;
    }
    
    /* Check of n_float32 matches file size */
    byterev = FALSE;
    if ((n_float32*sizeof(float32) + 4) != statbuf.st_size) {
	n = n_float32;
	SWAP_INT32(&n);

	if ((n*sizeof(float32) + 4) != statbuf.st_size) {
	    E_ERROR("Header size field: %d(%08x); filesize: %d(%08x)\n",
		    n_float32, n_float32, statbuf.st_size, statbuf.st_size);
	    fclose (fp);
	    return -1;
	}

	n_float32 = n;
	byterev = TRUE;
    }
    if (n_float32 <= 0) {
	E_ERROR("Header size field: %d\n",  n_float32);
	fclose (fp);
	return -1;
    }
    if (byterev)
	E_INFO("Byte-reversing %s\n", file);
    
    E_INFO("Dimensions %d..%d%s\n", sd, ed, (smooth ? " (LPFed)" : ""));

    fr = 0;
    while (fread (cep, sizeof(float32), 13, fp) == 13) {
	if (byterev) {
	    for (i = 0; i < 13; i++) {
		SWAP_FLOAT32(cep+i);
	    }
	}
	
	if (smooth) {
	    /* Smoothing (LPF-ing): 0.25, 0.5, 0.25; except at the ends: 0.5, 0.5 */
	    if ((fr > 0) && numbered)
		printf ("%10d ", fr-1);
	    
	    if (fr == 1) {
		for (i = sd; i <= ed; i++) {
		    c = 0.5 * cep_1[i] + 0.5 * cep[i];
		    printf (" %11.7f", c);
		}
	    } else if (fr > 1) {
		for (i = sd; i <= ed; i++) {
		    c = 0.25 * cep_2[i] + 0.5 * cep_1[i] + 0.25 * cep[i];
		    printf (" %11.7f", c);
		}
	    }

	    memcpy (cep_2, cep_1, sizeof(float32) * 13);
	    memcpy (cep_1, cep, sizeof(float32) * 13);
	} else {
	    if (numbered)
		printf ("%10d ", fr);
	    
	    for (i = sd; i <= ed; i++) {
		printf (" %11.7f", cep[i]);
	    }
	}
	
	if ((! smooth) || (fr > 0))
	    printf ("\n");
	fflush (stdout);
	
	fr++;
    }
    
    if (smooth && (fr > 0)) {
	if (numbered)
	    printf ("%10d ", fr-1);
	
	if (fr > 1) {
	    for (i = sd; i <= ed; i++) {
		c = 0.5 * cep_2[i] + 0.5 * cep_1[i];
		printf (" %11.7f", c);
	    }
	} else if (fr == 1) {
	    for (i = sd; i <= ed; i++)
		printf (" %11.7f", cep_1[i]);
	}

	printf ("\n");
	fflush (stdout);
    }
}
