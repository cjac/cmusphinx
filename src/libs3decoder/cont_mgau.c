/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * cont_mgau.c -- Mixture Gaussians for continuous HMM models.
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
 * 26-Aug-2004  ARCHAN (archan@cs.cmu.edu)
 *              Include changes that allow full floating point-based computation. Checking of computation type 
 *              is also added to all function of the code. 
 * 24-Jul-2004  ARCHAN (archan@cs.cmu.edu)
 *              Include new changes that allow use of Maximum Likelihood Linear Regression (MLLR) for speaker adaptation. 
 * 26-May-2004  ARCHAN (archan@cs.cmu.edu)
 *              Incorporate code for numerous fixes. 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Added mgau_free to free memory allocated by mgau_init()
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added mgau_var_nzvec_floor().
 * 
 * 28-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Started.
 */


#include "s3types.h"
#include "bio.h"
#include "vector.h"
#include "logs3.h"
#include "cont_mgau.h"


#define MGAU_PARAM_VERSION	"1.0"	/* Sphinx-3 file format version for mean/var */
#define MGAU_MIXW_VERSION	"1.0"	/* Sphinx-3 file format version for mixw */

/*
 * Sphinx-3 model mean and var files have the same format.  Use this routine for reading
 * either one.
 * Warning! You can only read the variance after reading the mean.  
 */
static int32 mgau_file_read(mgau_model_t *g, char *file_name, int32 type)
{
    char tmp;
    FILE *fp;
    int32 i, k, n;
    int32 n_mgau;
    int32 n_feat;
    int32 n_density;
    int32 *veclen;
    int32 blk;
    int32 byteswap, chksum_present;
    float32 *buf, **pbuf;
    char **argname, **argval;
    uint32 chksum;

    if(g->verbose) E_INFO("Reading mixture gaussian file '%s'\n", file_name);
    
    fp = myfopen (file_name, "rb");
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], MGAU_PARAM_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
		       file_name, argval[i], MGAU_PARAM_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;
    
    chksum = 0;
    
    /* #Codebooks */
    if (bio_fread (&n_mgau, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#codebooks) failed\n", file_name);
    if (n_mgau >= MAX_S3MGAUID) {
	E_FATAL("%s: #Mixture Gaussians (%d) exceeds limit(%d) enforced by MGAUID type\n",
		file_name, n_mgau, MAX_S3MGAUID);
    }
    
    /* #Features/codebook */
    if (bio_fread (&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#features) failed\n", file_name);
    
    if(g->gau_type==CONTHMM){
      if (n_feat != 1)
	E_FATAL("#Features streams(%d) != 1 in continuous HMM\n", n_feat);
    }else if (g->gau_type==SEMIHMM){
      if (n_feat != 4)
	E_FATAL("#Features streams(%d) != 1 in semi-continuous HMM\n", n_feat);
    }
    
    /* #Gaussian densities/feature in each codebook */
    if (bio_fread (&n_density, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (#density/codebook) failed\n", file_name);
    
    /* Vector length of feature stream */

    veclen = ckd_calloc(n_feat, sizeof(uint32));

    if (bio_fread (veclen, sizeof(int32), n_feat, fp, byteswap, &chksum) != n_feat)
	E_FATAL("fread(%s) (feature-lengths) failed\n", file_name);

    for (i = 0, blk = 0; i < n_feat; i++)
	blk += veclen[i];

    /*    if (bio_fread (&veclen, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	  E_FATAL("fread(%s) (feature vector-length) failed\n", file_name);*/
    
    /* #Floats to follow; for the ENTIRE SET of CODEBOOKS */
    if (bio_fread (&n, sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	E_FATAL("fread(%s) (total #floats) failed\n", file_name);

    if (n != n_mgau * n_density * blk) {
	E_FATAL("%s: #float32s(%d) doesn't match dimensions: %d x %d x %d\n",
		file_name, n, n_mgau, n_density, blk);
    }
    
    if(g->gau_type==SEMIHMM){
      E_FATAL("Currently S2 semi-continous HMM is not supported\n");
    }

    if (type == MGAU_MEAN) {
	/* Allocate memory for mixture gaussian densities */
	g->n_mgau = n_mgau;
	g->max_comp = n_density;
	g->veclen = blk;

	if(!(g->mgau)) {
	  g->mgau = (mgau_t *) ckd_calloc (n_mgau, sizeof(mgau_t));

	  buf = (float32 *) ckd_calloc (n, sizeof(float));
	  pbuf = (float32 **) ckd_calloc (n_mgau * n_density, sizeof(float32 *));
	  
	  for (i = 0; i < n_mgau; i++) {
	    g->mgau[i].n_comp = n_density;
	    g->mgau[i].mean = pbuf;
	    
	    for (k = 0; k < n_density; k++) {
	      g->mgau[i].mean[k] = buf;
		buf += blk;
		
	    }
	    pbuf += n_density;
	  }
	}
	buf = g->mgau[0].mean[0];	/* Restore buf to original value */


    } else {
	assert (type == MGAU_VAR);
	
	if (g->n_mgau != n_mgau)
	    E_FATAL("#Mixtures(%d) doesn't match that of means(%d)\n", n_mgau, g->n_mgau);
	if (g->max_comp != n_density)
	    E_FATAL("#Components(%d) doesn't match that of means(%d)\n", n_density, g->max_comp);
	if (g->veclen != blk)
	    E_FATAL("#Vector length(%d) doesn't match that of means(%d)\n", blk, g->veclen);

	if(!(g->mgau[0].var)){
	  buf = (float32 *) ckd_calloc (n, sizeof(float32));
	  pbuf = (float32 **) ckd_calloc (n_mgau * n_density, sizeof(float32 *));
	  
	  for (i = 0; i < n_mgau; i++) {
	    if (g->mgau[i].n_comp != n_density)
	      E_FATAL("Mixture %d: #Components(%d) doesn't match that of means(%d)\n",
		      i, n_density, g->mgau[i].n_comp);
	    
	    g->mgau[i].var = pbuf;
	    
	    for (k = 0; k < n_density; k++) {
	      g->mgau[i].var[k] = buf;
	      buf += blk;
	      
	    }
	    pbuf += n_density;
	  }


	  buf = (float32 *) ckd_calloc (n_mgau * n_density, sizeof(float32));
	  
	  for (i = 0; i < n_mgau; i++) {
	    g->mgau[i].lrd = buf;
	    buf += n_density;
	  }
	}

	buf = g->mgau[0].var[0];	/* Restore buf to original value */
    }
    
    /* Read mixture gaussian densities data */
    if (bio_fread (buf, sizeof(float32), n, fp, byteswap, &chksum) != n)
	E_FATAL("fread(%s) (densitydata) failed\n", file_name);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);

    if (fread (&tmp, 1, 1, fp) == 1)
	E_FATAL("%s: More data than expected\n", file_name);
    
    fclose(fp);
    
    if(g->verbose)
      E_INFO("%d mixture Gaussians, %d components, %d streams, veclen %d\n", n_mgau, n_density, n_feat, blk);
    
    return 0;
}

int32 mgau_mean_reload(mgau_model_t *g, char* mean_file_name)
{
  assert (g->mgau!=NULL);
  mgau_file_read (g, mean_file_name, MGAU_MEAN);
  /* ARCHAN 20040724 should do s3-like verification, need to change interface though. Put it later.  */
  return 0; 
}

int32 mgau_dump(mgau_model_t *g, int32 type)
{
  int32 d, c, i;
  char* tmpstr;
  mgau_t m;
  assert (g!=NULL);
  assert (g->mgau!=NULL);
  assert (g->mgau[0].mean!=NULL);
  assert (g->mgau[0].var!=NULL);
  assert (type == MGAU_VAR || type == MGAU_MEAN);
  tmpstr=ckd_calloc((mgau_veclen(g) * 20),sizeof(char));

  E_INFO("\n");

  if(type==MGAU_MEAN){
    for(d=0;d<mgau_n_mgau(g);d++){
      m=g->mgau[d];
      sprintf(tmpstr,"Mean of %d\n",d);
      E_INFO("%s",tmpstr);
      
      for(c=0;c<mgau_n_comp(g,d);c++){
	sprintf(tmpstr,"Component %d",c);
	for(i=0;i<mgau_veclen(g);i++){
	  sprintf(tmpstr,"%s %f",tmpstr,m.mean[c][i]);
	}
      sprintf(tmpstr, "%s\n",tmpstr);
      E_INFO("%s",tmpstr);
      }
    }
  }
  if(type == MGAU_VAR){
    for(d=0;d<mgau_n_mgau(g);d++){
      m=g->mgau[d];
      sprintf(tmpstr,"Variance of %d",d);
      E_INFO("%s",tmpstr);
      
      for(c=0;c<mgau_n_comp(g,d);c++){
	sprintf(tmpstr,"Component %d\n",c);
	for(i=0;i<mgau_veclen(g);i++){
	  sprintf(tmpstr,"%s %f",tmpstr,m.var[c][i]);
	}
      sprintf(tmpstr, "%s\n",tmpstr);
      }
      E_INFO("%s",tmpstr);
  }
  }

  ckd_free(tmpstr);
  return 0 ;
}

static int32 mgau_mixw_read(mgau_model_t *g, char *file_name, float64 mixwfloor)
{
    char **argname, **argval;
    char eofchk;
    FILE *fp;
    int32 byteswap, chksum_present;
    uint32 chksum;
    int32 *buf;
    float32 *buf_f;
    float32 *pdf;
    int32 i, j, n;
    int32 n_mgau;
    int32 n_feat;
    int32 n_comp;
    int32 n_err;
    
    if(g->verbose) E_INFO("Reading mixture weights file '%s'\n", file_name);
    
    fp = myfopen (file_name, "rb");
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], MGAU_MIXW_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
			file_name, argval[i], MGAU_MIXW_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #senones, #features, #codewords, arraysize */
    if ((bio_fread (&n_mgau, sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&n_feat, sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&n_comp, sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&n, sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
	E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if(g->gau_type==CONTHMM){
      if (n_feat != 1)
	E_FATAL("#Features streams(%d) != 1 in continuous HMM\n", n_feat);
    }else if (g->gau_type==SEMIHMM){
      if (n_feat != 4)
	E_FATAL("#Features streams(%d) != 4 in semi-continuous HMM\n", n_feat);
    }else{
	E_FATAL("How can this happen? Someone must have moved this part of the code somewhere! Not my fault! ARCHAN at 20040504 :-)\n");
    }

    if (n != n_mgau * n_comp) {
	E_FATAL("%s: #float32s(%d) doesn't match header dimensions: %d x %d\n",
		file_name, i, n_mgau, n_comp);
    }

    if (n_mgau != g->n_mgau){
      E_FATAL("%s: #Mixture Gaussians(%d) doesn't match mean/var parameters(%d)\n",
	      n_mgau, g->n_mgau);
    }
    
    if(g->comp_type==MIX_INT_FLOAT_COMP){
      /*Memory allocation of the integer version of mixture weight*/
      buf = (int32 *) ckd_calloc (n_mgau * n_comp, sizeof(int32));

      for (i = 0; i < n_mgau; i++) {
	if (n_comp != mgau_n_comp(g,i))
	  E_FATAL("Mixture %d: #Weights(%d) doesn't match #Gaussian components(%d)\n",
		  i, n_comp, mgau_n_comp(g,i));
	g->mgau[i].mixw = buf;
	buf += g->mgau[i].n_comp;
      }

    }else if(g->comp_type==FULL_FLOAT_COMP){
      /*Memory allocation of the floating point version of mixture weight */
      buf_f = (float32 *) ckd_calloc(n_mgau * n_comp, sizeof(float32));
      for (i = 0; i < n_mgau; i++) {
	if (n_comp != mgau_n_comp(g,i))
	  E_FATAL("Mixture %d: #Weights(%d) doesn't match #Gaussian components(%d)\n",
		  i, n_comp, mgau_n_comp(g,i));
	g->mgau[i].mixw_f = buf_f;
	buf_f += g->mgau[i].n_comp;
      }
    }else{
      E_FATAL("Unsupported GMM computation type %d \n",g->comp_type);
    }


    /* Temporary structure to read in floats */
    /* In the case of hybrid computation, logs3 will be used to convert 
       the weight to integer */
    pdf = (float32 *) ckd_calloc (n_comp, sizeof(float32));
    
    /* Read mixw data, normalize, floor. */
    n_err = 0;
    for (i = 0; i < n_mgau; i++) {
      if (bio_fread((void *)pdf, sizeof(float32), n_comp, fp, byteswap, &chksum) != n_comp)
	E_FATAL("bio_fread(%s) (arraydata) failed\n", file_name);
      
      /* Normalize and floor */
	if (vector_is_zero (pdf, n_comp)) {
	  n_err++;
	  for (j = 0; j < n_comp; j++){

	    if(g->comp_type==MIX_INT_FLOAT_COMP)
	      mgau_mixw(g,i,j) = S3_LOGPROB_ZERO;
	    else if (g->comp_type==FULL_FLOAT_COMP)
	      mgau_mixw_f(g,i,j) = S3_LOGPROB_ZERO_F;
	    else
	      E_FATAL("Unsupported computation type %d \n",g->comp_type);

	  }
	} else {
	  vector_nz_floor (pdf, n_comp, mixwfloor);
	  vector_sum_norm (pdf, n_comp);
	  
	  for (j = 0; j < n_comp; j++){

	    if(g->comp_type==MIX_INT_FLOAT_COMP)
	      mgau_mixw(g,i,j) = (pdf[j] != 0.0) ? logs3(pdf[j]) : S3_LOGPROB_ZERO;
	    else if(g->comp_type==FULL_FLOAT_COMP)
	      mgau_mixw_f(g,i,j) = (pdf[j] != 0.0) ? log(pdf[j]) : S3_LOGPROB_ZERO_F;
	    else
	      E_FATAL("Unsupported computation type %d \n",g->comp_type);

	  }
	}
    }
    if (n_err > 0)
	E_ERROR("Weight normalization failed for %d senones\n", n_err);
    ckd_free (pdf);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    if(g->verbose) E_INFO("Read %d x %d mixture weights\n", n_mgau, n_comp);

    
    return 0;
}


/*
 * Compact each mixture Gaussian in the given model by removing any uninitialized components.
 * A component is considered to be uninitialized if its variance is the 0 vector.  Compact by
 * copying the data rather than moving pointers.  Otherwise, malloc pointers could get
 * corrupted.
 */
static void mgau_uninit_compact (mgau_model_t *g)
{
    int32 m, c, c2, n, nm;
    
    if(g->verbose) E_INFO("Removing uninitialized Gaussian densities\n");
    
    n = 0;
    nm = 0;
    for (m = 0; m < mgau_n_mgau(g); m++) {
	for (c = 0, c2 = 0; c < mgau_n_comp(g,m); c++) {
	    if (! vector_is_zero (mgau_var(g,m,c), mgau_veclen(g))) {
		if (c2 != c) {
		    memcpy (mgau_mean(g,m,c2), mgau_mean(g,m,c),
			    mgau_veclen(g) * sizeof(float32));
		    memcpy (mgau_var(g,m,c2), mgau_var(g,m,c),
			    mgau_veclen(g) * sizeof(float32));
		    
		    if(g->comp_type==MIX_INT_FLOAT_COMP)
		      mgau_mixw(g,m,c2) = mgau_mixw(g,m,c);
		    else if(g->comp_type==FULL_FLOAT_COMP)
		      mgau_mixw_f(g,m,c2) = mgau_mixw_f(g,m,c);
		    else
		      E_FATAL("Unsupported computation type %d \n",g->comp_type);
		}
		c2++;
	    } else {
		n++;
	    }
	}
	mgau_n_comp(g,m) = c2;
	if (c2 == 0) {
	    fprintf (stderr, " %d", m);
	    fflush (stderr);
	    nm++;
	}
    }
    if (nm > 0)
	fprintf (stderr, "\n");
    
    if ((nm > 0) || (n > 0))
	E_WARN ("%d densities removed (%d mixtures removed entirely)\n", n, nm);
}


static void mgau_var_floor (mgau_model_t *g, float64 floor)
{
  int32 m, c, i, n;
  
  if(g->verbose) 
    E_INFO("Applying variance floor\n");
  n = 0;
  for (m = 0; m < mgau_n_mgau(g); m++) {
    for (c = 0; c < mgau_n_comp(g,m); c++) {
      for (i = 0; i < mgau_veclen(g); i++) {
	if (g->mgau[m].var[c][i] < floor) {
	  g->mgau[m].var[c][i] = (float32) floor;
	  n++;
	}
      }
    }
  }
  if(g->verbose)
    E_INFO("%d variance values floored\n", n);
}


int32 mgau_var_nzvec_floor (mgau_model_t *g, float64 floor)
{
  int32 m, c, i, n, l;
  float32 *var;
  
  if(g->verbose)
    E_INFO("Applying variance floor to non-zero variance vectors\n");
  
  l = mgau_veclen(g);
  
  n = 0;
  for (m = 0; m < mgau_n_mgau(g); m++) {
    for (c = 0; c < mgau_n_comp(g,m); c++) {
      var = g->mgau[m].var[c];
      
      if (! vector_is_zero (var, l)) {
	for (i = 0; i < l; i++) {
	  if (var[i] < floor) {
	    var[i] = (float32) floor;
	    n++;
	  }
	}
      }
    }
  }

  if(g->verbose)
  E_INFO("%d variance values floored\n", n);
  
  return n;
}


/*
 * Some of the Mahalanobis distance computation (between Gaussian density means and given
 * vectors) can be carried out in advance.  (See comment in .h file.)
 */
static int32 mgau_precomp (mgau_model_t *g)
{
    int32 m, c, i;
    float64 lrd;

    if(g->verbose)
      E_INFO("Precomputing Mahalanobis distance invariants\n");

    for (m = 0; m < mgau_n_mgau(g); m++) {
	for (c = 0; c < mgau_n_comp(g,m); c++) {
	    lrd = 0.0;
	    for (i = 0; i < mgau_veclen(g); i++) {
		lrd += log(g->mgau[m].var[c][i]);
		
		/* Precompute this part of the exponential */
		g->mgau[m].var[c][i] = (float32) (1.0 / (g->mgau[m].var[c][i] * 2.0));
	    }
	    
	    lrd += mgau_veclen(g) * log(2.0 * PI);	/* (2pi)^velen */
	    mgau_lrd(g,m,c) = (float32)(-0.5 * lrd);	/* Reciprocal, sqrt */
	}
    }
    
    return 0;
}

/* Hack! Temporary measuer to make classifier works. */
int32 mgau_precomp_hack_log_to_float(mgau_model_t *g)
{
    int32 m, c;

    if(g->verbose)
      E_INFO("Revert log values back to normal\n");

    for (m = 0; m < mgau_n_mgau(g); m++) {
	for (c = 0; c < mgau_n_comp(g,m); c++) {
	  mgau_lrd(g,m,c)=exp(mgau_lrd(g,m,c));
	  mgau_mixw_f(g,m,c)=exp(mgau_mixw_f(g,m,c));
	}
    }
    
    return 0;

}

/* At the moment, S3 models have the same #means in each codebook and 1 var/mean */
mgau_model_t *mgau_init (char *meanfile, 
			 char *varfile, float64 varfloor,
			 char *mixwfile, float64 mixwfloor,
			 int32 precomp,
			 char* senmgau,
			 int32 comp_type)
{
    mgau_model_t *g;

    assert (meanfile != NULL);
    assert (varfile != NULL);
    assert (varfloor >= 0.0);
    assert (mixwfile != NULL);
    assert (mixwfloor >= 0.0);
    
    g = (mgau_model_t *) ckd_calloc (1, sizeof(mgau_model_t));

    if(strcmp(senmgau,".cont.") == 0) {
      g->gau_type=CONTHMM;
    }else if(strcmp(senmgau,".semi.") == 0){
      g->gau_type=SEMIHMM;
    }else{
      E_FATAL("Feature should be either .semi. or .cont.");
    }

    if(comp_type==FULL_INT_COMP)
      E_INFO("Currently full integer GMM computation is not supported yet.\n");
    assert(comp_type==FULL_FLOAT_COMP||comp_type==MIX_INT_FLOAT_COMP);
    g->comp_type=comp_type;
    /* Hardwire verbose to 1 */
    g->verbose=1;

    /* Read means and (diagonal) variances for all mixture gaussians */
    mgau_file_read (g, meanfile, MGAU_MEAN);
    mgau_file_read (g, varfile, MGAU_VAR);
    mgau_mixw_read (g, mixwfile, mixwfloor);
    
    mgau_uninit_compact (g);		/* Delete uninitialized components */
    
    if (varfloor > 0.0)
	mgau_var_floor (g, varfloor);	/* Variance floor after above compaction */
    
    if (precomp)
	mgau_precomp (g);		/* Precompute Mahalanobis distance invariants */
    
    if(g->comp_type==MIX_INT_FLOAT_COMP)
      g->distfloor = logs3_to_log (S3_LOGPROB_ZERO);	/* Floor for Mahalanobis distance values */
    else if(g->comp_type==FULL_FLOAT_COMP)
      g->distfloor = S3_LOGPROB_ZERO_F;
    
    return g;
}


int32 mgau_comp_eval (mgau_model_t *g, int32 s, float32 *x, int32 *score)
{
    mgau_t *mgau;
    int32 veclen;
    float32 *m, *v;
    float64 dval, diff, f;
    int32 bs;
    int32 i, c;
    
    veclen = mgau_veclen(g);
    mgau = &(g->mgau[s]);
    f = log_to_logs3_factor();

    bs = MAX_NEG_INT32;
    for (c = 0; c < mgau->n_comp; c++) {
	m = mgau->mean[c];
	v = mgau->var[c];
	dval = mgau->lrd[c];
	
	for (i = 0; i < veclen; i++) {
	    diff = x[i] - m[i];
	    dval -= diff * diff * v[i];
	}
	
	if (dval < g->distfloor)
	    dval = g->distfloor;
	
	score[c] = (int32) (f * dval);
	if (score[c] > bs)
	    bs = score[c];
    }
    
    return bs;
}


int32 mgau_eval (mgau_model_t *g, int32 m, int32 *active, float32 *x)
{
    mgau_t *mgau;
    int32 veclen, score;
    /*tmpscore;*/
    float32 *m1, *m2, *v1, *v2;
    float64 dval1, dval2, diff1, diff2, f;


    int32 i, j, c;
    
    veclen = mgau_veclen(g);
    mgau = &(g->mgau[m]);
    assert(g->comp_type==MIX_INT_FLOAT_COMP);
    f = log_to_logs3_factor();
    score = S3_LOGPROB_ZERO;


    if (! active) {	/* No short list; use all */

     
#if 1
	for (c = 0; c < mgau->n_comp-1; c += 2) {	/* Interleave 2 components for speed */
	    m1 = mgau->mean[c];
	    m2 = mgau->mean[c+1];
	    v1 = mgau->var[c];
	    v2 = mgau->var[c+1];
	    dval1 = mgau->lrd[c];
	    dval2 = mgau->lrd[c+1];
	    
	    for (i = 0; i < veclen; i++) {
		diff1 = x[i] - m1[i];
		dval1 -= diff1 * diff1 * v1[i];
		diff2 = x[i] - m2[i];
		dval2 -= diff2 * diff2 * v2[i];
		/*		E_INFO("x %10f m1 %10f m2 %10f v1 %10f, v2 %10f\n",x[i],m1[i],m2[i],v1[i],v2[i]);
				E_INFO("diff1 %10f,dval1 %10f, diff2 %10f, dval2 %10f\n",diff1,dval1,diff2,dval2);*/
	    }
	    
	    if (dval1 < g->distfloor)	/* Floor */
		dval1 = g->distfloor;
	    if (dval2 < g->distfloor)
		dval2 = g->distfloor;
	    
	    score = logs3_add (score, (int32)(f * dval1) + mgau->mixw[c]);
	    score = logs3_add (score, (int32)(f * dval2) + mgau->mixw[c+1]);
	}
	
	/* Remaining iteration if n_mean odd */
	if (c < mgau->n_comp) {
	    m1 = mgau->mean[c];
	    v1 = mgau->var[c];
	    dval1 = mgau->lrd[c];
	    
	    for (i = 0; i < veclen; i++) {
		diff1 = x[i] - m1[i];
		dval1 -= diff1 * diff1 * v1[i];
	    }
	    
	    if (dval1 < g->distfloor)
		dval1 = g->distfloor;
	    
	    score = logs3_add (score, (int32)(f * dval1) + mgau->mixw[c]);
	}

#endif

    } else {
	for (j = 0; active[j] >= 0; j++) {
	    c = active[j];
	    
	    m1 = mgau->mean[c];
	    v1 = mgau->var[c];
	    dval1 = mgau->lrd[c];
	    
	    for (i = 0; i < veclen; i++) {
		diff1 = x[i] - m1[i];
		dval1 -= diff1 * diff1 * v1[i];
	    }
	    
	    if (dval1 < g->distfloor)
		dval1 = g->distfloor;
	    
	    score = logs3_add (score, (int32)(f * dval1) + mgau->mixw[c]);
	}
    }
    
    if(score == S3_LOGPROB_ZERO){
      /*      E_INFO("Warning!! Score is S3_LOGPROB_ZERO\n");*/
    }
    return score;
}

/* RAH, free memory allocated in mgau_init
   I've not verified that this function catches all of the leaks, just most of them.
 */
void mgau_free (mgau_model_t *g)
{
  //  int i,j;
  if (g) {
    /* Free memory allocated for the mean structure*/
    //    for (i=0;i<g->n_mgau;i++) {
    //      for (j=0;j<g->max_comp;j++) {
    //	if (g->mgau[i].mean[j]) 
    //	  ckd_free ((void *) g->mgau[i].mean[j]);
    //      }
    //    }

    if (g->mgau[0].mean) 
      ckd_free ((void *) g->mgau[0].mean);

    /* Free memory allocated for the var structure*/
//    for (i=0;i<g->n_mgau;i++) {
//      for (j=0;j<g->max_comp;j++) {
//	if (g->mgau[i].var[j]) 
//	  ckd_free ((void *) g->mgau[i].var[j]);
//      }
//    }
    if (g->mgau[0].var) 
      ckd_free ((void *) g->mgau[0].var);
    if (g->mgau[0].lrd) 
      ckd_free ((void *) g->mgau[0].lrd);

    /* Free memory allocated for the mixture weights*/
//    for (i=0;i<g->n_mgau;i++) 
//      if (g->mgau[i].mixw[0]) 
//	ckd_free ((void *) g->mgau[i].mixw[0]);
    
    if (g->mgau[0].mixw) 
      ckd_free ((void *) g->mgau[0].mixw);

    if (g->mgau[0].mixw_f)
      ckd_free ((void *) g->mgau[0].mixw_f);
    
    if (g->mgau)
	  ckd_free ((void *) g->mgau);
    ckd_free ((void *) g);
  }
}
