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
 * gausubvq.c -- Sub-vector cluster Gaussian densities
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Porting initial version to s3.2.
 */


#include "cont_mgau.h"
#include "logs3.h"
#include "vector.h"


/*
 * Parse subvectors specification string.  Format:
 *   - '/' separated list of subvectors
 *   - each subvector is a ',' separated list of subranges
 *   - each subrange is a single <number> or <number>-<number> (inclusive).
 *     (where, <number> is a feature vector dimension specifier).
 * E.g., "24,0-11/25,12-23/26,27-38" has:
 *   - 3 subvectors
 *   - the 1st subvector has feature dims: 24, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, and 11.
 *   - etc.
 * Return value: subvec = 2-D array of subvector specs.  If there are N subvectors specified,
 * subvec[N] = NULL; and each subvec[0]..subvec[N-1] is -1 terminated vector of feature dims.
 */
static int32 **parse_subvecs (char *str)
{
    char *strp;
    int32 n, n2, l;
    glist_t dimlist;	/* List of dimensions in one subvector */
    glist_t veclist;	/* List of dimlists (subvectors) */
    int32 **subvec;
    gnode_t *gn, *gn2;
    
    veclist = NULL;
    
    strp = str;
    for (;;) {
	dimlist = NULL;
	
       	for (;;) {
	    if (sscanf (strp, "%d%n", &n, &l) != 1)
		E_FATAL ("'%s': Couldn't read int32 @pos %d\n", str, strp-str);
	    strp += l;

	    if (*strp == '-') {
		strp++;
		
		if (sscanf (strp, "%d%n", &n2, &l) != 1)
		    E_FATAL ("'%s': Couldn't read int32 @pos %d\n", str, strp-str);
		strp += l;
	    } else
		n2 = n;
	    
	    if ((n < 0) || (n > n2))
		E_FATAL("'%s': Bad subrange spec ending @pos %d\n", str, strp-str);
	    
	    for (; n <= n2; n++) {
		if (glist_chkdup_int32 (dimlist, n))
		    E_FATAL("'%s': Duplicate dimension ending @pos %d\n", str, strp-str);
		
		dimlist = glist_add_int32 (dimlist, n);
	    }
	    
	    if ((*strp == '\0') || (*strp == '/'))
		break;
	    
	    if (*strp != ',')
		E_FATAL("'%s': Bad delimiter @pos %d\n", str, strp-str);
	    
	    strp++;
	}
	
	veclist = glist_add_ptr (veclist, (void *)dimlist);
	
	if (*strp == '\0')
	    break;
	
	assert (*strp == '/');
	strp++;
    }
    
    /* Convert the glists to arrays; remember the glists are in reverse order of the input! */
    n = glist_count (veclist);					/* #Subvectors */
    subvec = (int32 **) ckd_calloc (n+1, sizeof(int32 *));	/* +1 for sentinel */
    subvec[n] = NULL;						/* sentinel */
    
    for (--n, gn = veclist; (n >= 0) && gn; gn = gnode_next(gn), --n) {
	gn2 = (glist_t) gnode_ptr (gn);
	
	n2 = glist_count (gn2);					/* Length of this subvector */
	if (n2 <= 0)
	    E_FATAL("'%s': 0-length subvector\n", str);
	
	subvec[n] = (int32 *) ckd_calloc (n2+1, sizeof(int32));	/* +1 for sentinel */
	subvec[n][n2] = -1;					/* sentinel */
	
	for (--n2; (n2 >= 0) && gn2; gn2 = gnode_next(gn2), --n2)
	    subvec[n][n2] = gnode_int32 (gn2);
	assert ((n2 < 0) && (! gn2));
    }
    assert ((n < 0) && (! gn));
    
    /* Free the glists */
    for (gn = veclist; gn; gn = gnode_next(gn)) {
	gn2 = (glist_t) gnode_ptr(gn);
	glist_free (gn2);
    }
    glist_free (veclist);
    
    return subvec;
}


static arg_t arg[] = {
    { "-mean",
      REQARG_STRING,
      NULL,
      "Means file" },
    { "-var",
      REQARG_STRING,
      NULL,
      "Variances file" },
    { "-mixw",
      REQARG_STRING,
      NULL,
      "Mixture weights file (needed, even though it's not part of the computation)" },
    { "-stdev",
      ARG_INT32,
      "0",
      "Use std.dev. (rather than var) in computing vector distances during clustering" },
    { "-svspec",
      REQARG_STRING,
      NULL,
      "Subvectors specification (e.g., 24,0-11/25,12-23/26-38 or 0-12/13-25/26-38)" },
    { "-svqrows",
      ARG_INT32,
      "4096",
      "No. of codewords in output subvector codebooks" },
    { "-iter",
      ARG_INT32,
      "100",
      "Max no. of k-means iterations for clustering" },
    { "-eps",
      ARG_FLOAT64,
      "0.0001",
      "Stopping criterion: stop iterations if relative decrease in sq(error) < eps" },
    { "-varfloor",
      ARG_FLOAT64,
      "0.0001",
      "Floor for non-zero variance values in input model" },
    { "-mixwfloor",
      ARG_FLOAT64,
      "0.0000001",
      "Floor for non-zero mixture weight values in input model" },
    { "-subvq",
      ARG_STRING,
      NULL,
      "Output subvq file (stdout if not specified)" },
    { "-log3table",
      ARG_INT32,
      "1",
      "Determines whether to use the log3 table or to compute the values at run time."},
    
    { NULL, ARG_INT32, NULL, NULL }
};


int32 main (int32 argc, char *argv[])
{
    FILE *fpout;
    mgau_model_t *mgau;
    int32 **subvec;
    int32 max_datarows, datarows, datacols, svqrows, svqcols;
    float32 **data, **vqmean;
    int32 *datamap, *vqmap;
    float64 sqerr;
    int32 stdev;
    int32 i, j, v, m, c;
    
    cmd_ln_parse (arg, argc, argv);
    
    logs3_init ((float64) 1.0003);
    
    /* Load means/vars but DO NOT precompute variance inverses or determinants */
    mgau = mgau_init (cmd_ln_str("-mean"), cmd_ln_str("-var"), 0.0 /* no varfloor */,
		      cmd_ln_str("-mixw"), cmd_ln_float64 ("-mixwfloor"),
		      FALSE);
    mgau_var_nzvec_floor (mgau, cmd_ln_float64 ("-varfloor"));
    
    /* Parse subvector spec argument; subvec is null terminated; subvec[x] is -1 terminated */
    subvec = parse_subvecs (cmd_ln_str("-svspec"));
    
    if (cmd_ln_str ("-subvq"))
	fpout = myfopen (cmd_ln_str ("-subvq"), "w");
    else
	fpout = stdout;
    
    /* Echo command line to output file */
    for (i = 0; i < argc-1; i++)
	fprintf (fpout, "# %s \\\n", argv[i]);
    fprintf (fpout, "# %s\n#\n", argv[argc-1]);
    
    /* Print input and output configurations to output file */
    for (v = 0; subvec[v]; v++);		/* No. of subvectors */
    svqrows = cmd_ln_int32 ("-svqrows");
    fprintf (fpout, "VQParam %d %d -> %d %d\n",
	     mgau_n_mgau(mgau), mgau_max_comp(mgau), v, svqrows);
    for (v = 0; subvec[v]; v++) {
	for (i = 0; subvec[v][i] >= 0; i++);
	fprintf (fpout, "Subvector %d length %d ", v, i);
	for (i = 0; subvec[v][i] >= 0; i++)
	    fprintf (fpout, " %2d", subvec[v][i]);
	fprintf (fpout, "\n");
    }
    fflush (fpout);
    
    /*
     * datamap[] for identifying non-0 input vectors that take part in the clustering process:
     *     datamap[m*max_mean + c] = row index of data[][] containing the copy.
     * vqmap[] for mapping vq input data to vq output.
     */
    max_datarows = mgau_n_mgau(mgau) * mgau_max_comp(mgau);
    datamap = (int32 *) ckd_calloc (max_datarows, sizeof(int32));
    vqmap = (int32 *) ckd_calloc (max_datarows, sizeof(int32));
    
    stdev = cmd_ln_int32 ("-stdev");
    
    /* Copy and cluster each subvector */
    for (v = 0; subvec[v]; v++) {
	E_INFO("Clustering subvector %d\n", v);
	
	for (datacols = 0; subvec[v][datacols] >= 0; datacols++);	/* Input subvec length */
	svqcols = datacols * 2;		/* subvec length after concatenating mean + var */
	
	/* Allocate input/output data areas */
	data = (float32 **) ckd_calloc_2d (max_datarows, svqcols, sizeof(float32));
	vqmean = (float32 **) ckd_calloc_2d (svqrows, svqcols, sizeof(float32));
	
	/* Make a copy of the subvectors from the input data, and initialize maps */
	for (i = 0; i < max_datarows; i++)
	    datamap[i] = -1;
	datarows = 0;
	for (m = 0; m < mgau_n_mgau(mgau); m++) {		/* For each mixture m */
	    for (c = 0; c < mgau_n_comp(mgau, m); c++) {	/* For each component c in m */
		if (vector_is_zero (mgau_var(mgau, m, c), mgau_veclen(mgau)))
		    continue;
		
		for (i = 0; i < datacols; i++) {	/* Copy specified dimensions, mean+var */
		    data[datarows][i*2]   = mgau->mgau[m].mean[c][subvec[v][i]];
		    data[datarows][i*2+1] = (! stdev) ?
			mgau->mgau[m].var[c][subvec[v][i]] :
			sqrt(mgau->mgau[m].var[c][subvec[v][i]]);
		}
		datamap[m * mgau_max_comp(mgau) + c] = datarows++;
	    }
	}
	
	E_INFO("Sanity check: input data[0]:\n");
	vector_print (stderr, data[0], svqcols);
	
	for (i = 0; i < max_datarows; i++)
	    vqmap[i] = -1;
#if 0
	{
	    int32 **in;
	    
	    printf ("Input data: %d x %d\n", datarows, svqcols);
	    in = (int32 **)data;
	    for (i = 0; i < datarows; i++) {
		printf ("%8d:", i);
		for (j = 0; j < svqcols; j++)
		    printf (" %08x", in[i][j]);
		printf ("\n");
	    }
	    for (i = 0; i < datarows; i++) {
		printf ("%15d:", i);
		for (j = 0; j < svqcols; j++)
		    printf (" %15.7e", data[i][j]);
		printf ("\n");
	    }
	    fflush (stdout);
	}
#endif
	/* VQ the subvector copy built above */
	sqerr = vector_vqgen (data, datarows, svqcols, svqrows,
			      cmd_ln_float64("-eps"), cmd_ln_int32("-iter"),
			      vqmean, vqmap);
	
	/* Output VQ */
	fprintf (fpout, "Codebook %d Sqerr %e\n", v, sqerr);
	for (i = 0; i < svqrows; i++) {
	    if (stdev) {
		/* Convert clustered stdev back to var */
		for (j = 1; j < svqcols; j += 2)
		    vqmean[i][j] *= vqmean[i][j];
	    }
	    vector_print (fpout, vqmean[i], svqcols);
	}
	
	fprintf (fpout, "Map %d\n", v);
	for (i = 0; i < max_datarows; i += mgau_max_comp(mgau)) {
	    for (j = 0; j < mgau_max_comp(mgau); j++) {
		if (datamap[i+j] < 0)
		    fprintf (fpout, " -1");
		else
		    fprintf (fpout, " %d", vqmap[datamap[i+j]]);
	    }
	    fprintf (fpout, "\n");
	}
	fflush (fpout);
	
	/* Cleanup */
	ckd_free_2d ((void **) data);
	ckd_free_2d ((void **) vqmean);
    }
    
    fprintf (fpout, "End\n");
    fclose (fpout);
    
    exit(0);
}
