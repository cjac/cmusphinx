/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * pdf32to8b.c -- Read HMM senone probs dump file, cluster probs and approximate,
 * and create another dump file.
 */

#include <stdio.h>
#include <string.h>

#define QUIT(x)		{fprintf x; exit(-1);}

static FILE *errfp;

int *orig_val;
int *val;	/* orig_val, sorted */
int chktot = 0;
struct cluster_range_s {
    int from;
    int to;
    int cg;
} cluster_range[256];
unsigned char *clid;

dump_cluster ()
{
    double t, v1, v2, sqrt(), err1, err2, max_err, tot_err;
    int i, cg;
    
    max_err = tot_err = 0.0;
    for (i = 0; i < 256; i++) {
	v1 = cluster_range[i].from;
	v2 = cluster_range[i].to;
	cg = 1 - sqrt(1 - (v1+v2 - (v1*v2)));
	err1 = (cg == 0) ? (double)(cg-v1) : (double)(cg-v1)/(-cg);
	err2 = (v2 == 0) ? (double)(v2-cg) : (double)(v2-cg)/(-v2);
#if 0
	fprintf (stderr, "%3d: [%5d] - [%5d] (%4d) %8d - %8d, cg = %8d (%f, %f)\n",
		 cl, start, s, (start-s+1),
		 val[start], val[s], cg,
		 err1, err2);
	fprintf (stderr, "%3d: %8d - %8d, cg = %8d (%f, %f)\n", i,
		 cluster_range[i].from, cluster_range[i].to, cluster_range[i].cg,
		 err1, err2);
#endif
	if (max_err < err1)
	    max_err = err1;
	if (max_err < err2)
	    max_err = err2;
	tot_err += (err1+err2);
    }
    fprintf (errfp, "max err = %f, tot err = %f\n", max_err, tot_err);
}

/* linearly search for maximal clusters fitting within error_limit */
cluster (from, to, error_limit, final)
    int from, to;
    double error_limit;
    int final;
{
    int i, s, start, cl, cg;
    double t, v1, v2, sqrt(), err1, err2, max_err, tot_err;
    
    start = to;
    cl = 0;
    for (;;) {	/* while more data remains */
	/* find range of next cluster */
	t = val[start] - (error_limit * (-val[start]));
	t -= error_limit * (-t);
	/* should be within val[start]..t */
	for (s = start; (s >= from) && (val[s] >= t); --s);
	s++;
	if (final) {
	    v1 = val[start];
	    v2 = val[s];
	    cg = 1 - sqrt(1 - (v1+v2 - (v1*v2)));
	    if (cl >= 256)
		QUIT((errfp, "%s(%d): cl = %d\n", __FILE__, __LINE__, cl));
	    cluster_range[255-cl].from = val[s];
	    cluster_range[255-cl].to = val[start];
	    cluster_range[255-cl].cg = cg;
	}
	cl++;
	chktot += (start-s+1);
	start = --s;
	if (start < from)
	    break;
    }
    if (final) {
	fprintf (errfp, "Cluster-size= %d\n", cl);
	for (i = cl; i < 256; i++) {
	    cluster_range[255-i].from = cluster_range[255-i].to = cluster_range[255-i].cg =
		cluster_range[256-cl].from - 1;
	}
    }
    return (cl);
}

map_orig_val_to_clid (n)
    int n;
{
    int i, j;
    
    for (i = 0; i < n; i++) {
	for (j = 0; (j < 256) && (orig_val[i] > cluster_range[j].to); j++);
	if ((j >= 256) || (orig_val[i] < cluster_range[j].from))
	    QUIT((errfp, "%s(%d): orig_val[%d] (%d) not in any cluster\n", __FILE__, __LINE__, i, orig_val[i]));
	clid[i] = j;
    }
}

#if defined(_HPUX_SOURCE)
#define SWAPW(x)	x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define SWAPL(x)	x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
    			      (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )
#else
#define SWAPW(x)
#define SWAPL(x)
#endif

static int fread_int32(fp, min, max, name)
    FILE *fp;
    int min, max;
    char *name;
{
    int k;
    
    if (fread (&k, sizeof (int), 1, fp) != 1)
	QUIT((errfp, "%s(%d): fread(%s) failed\n", __FILE__, __LINE__, name));
    SWAPL(k);
    if ((min > k) || (max < k))
	QUIT((errfp, "%s(%d): %s outside range [%d,%d]\n", __FILE__, __LINE__, name, min, max));
    return (k);
}

static fwrite_int32 (fp, val)
    FILE *fp;
    int val;
{
    SWAPL(val);
    fwrite (&val, sizeof(int), 1, fp);
}

cmp_pdfval(x, y)
    int *x, *y;
{
    return (*x - *y);
}

char *fmtdesc[] = {
    "BEGIN FILE FORMAT DESCRIPTION",
    "(int32) <length(string)> (including trailing 0)",
    "<string> (including trailing 0)",
    "... preceding 2 items repeated any number of times",
    "(int32) 0 (length(string)=0 terminates the header)",
    "(int32) <#codewords>",
    "(int32) <#pdfs>",
    "256 (int32) cluster-prob values for codebook-0 codeword-0",
    "#pdf (unsigned char) cluster-prob ids for codebook-0 codeword-0",
    "... preceding 2 items repeated for all codewords in codebook-0",
    "preceding 3 items repeated for codebooks 1, 2, 3.",
    "END FILE FORMAT DESCRIPTION",
    NULL,
};

main (argc, argv)
    int argc;
    char *argv[];
{
    int i, c, n, n_cdwd, n_pdf;
    double err, lower_err, upper_err;
    FILE *fp, *fpout;
    char line[4096], *cltitle;

    if (argc < 3)
	QUIT((stderr, "Usage: %s [<input dumpfile> | - ] [<output dumpfile> | - ]\n", argv[0]));

    sprintf (line, "%s.log", argv[2]);
    if ((errfp = fopen (line, "w")) == NULL) {
	fprintf (stderr, "%s: fopen(%s,w) failed\n", argv[0], line);
	errfp = stderr;
    }
    
    if (strcmp (argv[1], "-") == 0)
	fp = stdin;
    else if ((fp = fopen (argv[1], "r")) == NULL)
	QUIT((errfp, "%s: Cannot open input dumpfile %s\n", argv[0], argv[1]));
    if ((fpout = fopen (argv[2], "w")) == NULL)
	QUIT((errfp, "%s: Cannot create output dumpfile %s\n", argv[0], argv[2]));
    
    fprintf (errfp, "%s: %s %s\n\n", argv[0], __DATE__, __TIME__);

    /* Copy header strings to output */
    for (;;) {
	n = fread_int32 (fp, 0, (int)0x7fffffff, "string length");
	if (n == 0)
	    break;
	if (fread (line, sizeof(char), n, fp) != n)
	    QUIT((errfp, "%s(%d): Cannot read header\n", __FILE__, __LINE__));
	fwrite_int32 (fpout, n);
	fwrite (line, sizeof(char), n, fpout);
    }
    
    /* Write format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	n = strlen(fmtdesc[i])+1;
	fwrite_int32 (fpout, n);
	fwrite (fmtdesc[i], sizeof(char), n, fpout);
    }

    /* Terminate header */
    fwrite_int32 (fpout, 0);

    /* Read #codewords, #pdfs */
    n_cdwd = fread_int32 (fp, 256, 256, "#codewords");
    n_pdf = fread_int32 (fp, 0, 0x7fffffff, "#pdfs");
    fwrite_int32 (fpout, n_cdwd);
    fwrite_int32 (fpout, n_pdf);
    
    if ((orig_val = (int *) malloc (n_pdf*2 * sizeof(int))) == NULL)
	QUIT((errfp, "%s(%d): malloc failed\n", __FILE__, __LINE__));
    val = orig_val + n_pdf;
    if ((clid = (unsigned char *) malloc (n_pdf * sizeof(unsigned char))) ==  NULL)
	QUIT((errfp, "%s(%d): malloc failed\n", __FILE__, __LINE__));
    
    for (c = 0; c < n_cdwd*4; c++) {	/* n_cdwd*4 for the 4 codebooks */
	fprintf (errfp, "cdwd %d\n", c);
	for (i = 0; i < n_pdf; i++) {
	    orig_val[i] = fread_int32 (fp, (int)0x80000000, 0, "Senone prob");
	    val[i] = orig_val[i];
	}
	qsort (val, n_pdf, sizeof(int), cmp_pdfval);
	
	lower_err = upper_err = 0.0;
	err = 0.001;
	for (;;) {
	    chktot = 0;
	    n = cluster (0, n_pdf-1, err, 0);
	    fprintf (errfp, "err = %f, #clusters = %d (chktot = %d)\n", err, n, chktot);
	    if (n == 256)
		break;
	    else if (n < 256) {
		if (err < 1e-6)
		    break;
		upper_err = err;
		err = (err + lower_err) * 0.5;
	    } else {
		lower_err = err;
		err = (upper_err > err) ? (err + upper_err) * 0.5 : err * 2.0;
	    }
	}
	cluster (0, n_pdf-1, err, 1);
	dump_cluster ();

	map_orig_val_to_clid (n_pdf);
	for (i = 0; i < 256; i++)
	    fwrite_int32 (fpout, cluster_range[i].cg);
	fwrite (clid, sizeof (unsigned char), n_pdf, fpout);
    }

    fclose (fpout);
}
