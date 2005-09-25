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
 * main_gauvq.c -- Gaussian Selection Routine. 
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
 * $Log$
 * Revision 1.1.2.1  2005/09/25  20:05:05  arthchan2003
 * (Not tested) First New Tool of September: gauvq. It can be used to generate Gaussian Selection map.
 * 
 *
 * 15-Dec-2003	Arthur Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 * 		First created
 */


#include "ms_mgau.h"
#include "logs3.h"
#include "sort.h"
#include "vector.h"
#include "s3types.h"
#include "math.h"
#include "cmdln_macro.h"
#include "cmd_ln.h"

static ms_mgau_model_t *ms_mgau;
static sort_array_t *sa;

static arg_t arg[] = {
  log_table_command_line_macro()
  gmm_command_line_macro() 
  vq_cluster_command_line_macro()
    { "-lothres",
      ARG_FLOAT32,
      "0",
      "Lower Threshold used to determine whether a gaussian is within a code book" },
    { "-hithres",
      ARG_FLOAT32,
      "1000000",
      "Higher Threshold used to determine whether a gaussian is within a code book. Default is very large to allow every gaussians were included." },
    { "-mingau",
      ARG_INT32,
      "1",
      "Minimum number of gaussian to compute in every state" },
    { "-drmaxgau",
      ARG_INT32,
      "512",
      "Maximum number of gaussian to compute in every state in the donut area. Default is very large to allow every gaussians were included." },
    {"-minlothres",
      ARG_FLOAT32,
      "-1",
      "Minimum value of lower threshold. Options for turning on batch-mode generation." },
    {"-maxlothres",
      ARG_FLOAT32,
      "-1",
      "Maximum value of lower threshold. Options for turning on batch-mode generation." },
    {"-intlothres",
      ARG_FLOAT32,
      "-1",
      "Interval value of lower threshold. Options for turning on batch-mode generation." },
    { "-outputvq",
      ARG_STRING,
      "output.vq",
      "The output code vector (default: output.vq) and corresponding Gaussian distribution" },
    { "-ncode",
      ARG_INT32,
      "4096",
      "No. of clusters in the output vector-clusters book" },
    { NULL, ARG_INT32, NULL, NULL }

};

/* \struct gau_al_struct
 */
typedef struct {
  int32 n_code; /**< Number of codeword */
  float32 threshold_low;  /**< Low threshold */
  float32 threshold_high; /**< High threshold */
  int32 min_count;   /**< Min count for the Gaussian */
  int32 dr_max_count;  /**< The max count of the Gaussian in donut region */
} gau_al_struct;

static gau_al_struct* gau_al_init(
				 int32 n_code,  /**< Number of codeword */
				 float32 lowt, /**< The low threshold */
				 float32 hight, /**< The high threshold */
				 int32 min_count, /**< Mininum count of Gaussian */
				 int32 dr_max_count  /**< The max count of the Gaussian in donut region */
				 )
{
  gau_al_struct* gal;
  if(min_count>1 || min_count <0) {
    E_ERROR("-min_count currently only support values 0 and 1\n");
    return NULL;
  }

  gal=ckd_calloc(1,sizeof(gau_al_struct));
  gal->n_code=n_code;
  gal->threshold_low=lowt;
  gal->threshold_high=hight;
  gal->min_count=min_count;
  gal->dr_max_count=dr_max_count;
  return gal;
}

static void gau_al_free(gau_al_struct *gal /**< A gal_al structure */
			)
{
  ckd_free(gal);
}

static void write_vq_file(gauden_t *g,
			  float32 **vqmean, 
			  float32 *smooth_cov, 
			  float32 *w_dists, 
			  int32 *is_computed, 
			  char* outfn, 
			  gau_al_struct *gal
			  )
{
  int32 n_cols;
  int32 m_id,s_id,g_id,c_id;
  int32 i,j;
  bitvec_t bv;
  float32 m,v,c;
  int32 gau_count;
  int32 dr_gau_count;
  FILE *fpout;

  /*Compute the smoothed covariance matrix*/

  n_cols=*(g->featlen);
  
  /*i, Empty the smooth covariance vector */
  for(c_id=0;c_id<n_cols;c_id++){
    smooth_cov[c_id]=0;
  }

  /*ii, Compute smoothed version of covariance*/

  for(m_id=0;m_id<g->n_mgau;m_id++) {
    for(s_id=0;s_id<g->n_feat;s_id++){
      for(g_id=0;g_id<g->n_density;g_id++){
	for(c_id=0;c_id<n_cols;c_id++)
	  smooth_cov[c_id]+=g->var[m_id][s_id][g_id][c_id];
      }
    }
  }

  for(c_id=0;c_id<n_cols;c_id++)
    smooth_cov[c_id]/=(g->n_mgau*g->n_feat*g->n_density);

  /*iii,For each code, Compute the weighted distance from the vector to each gaussian in the mixture-stream, See whether the gaussian is "closed" to it*/
  /*input the threshold*/
  bv=bitvec_alloc(g->n_density);

  fpout = myfopen (outfn, "wb");
    
  E_INFO("Start writing information to a file\n");
  /* Format <size of gaussian><size of feat><size of number of components per mixture><size of output vq><feature length>*/
  /* First 20 bytes */
  fwrite(&(g->n_mgau),sizeof(int32),1,fpout);
  fwrite(&(g->n_feat),sizeof(int32),1,fpout);
  fwrite(&(g->n_density),sizeof(int32),1,fpout);
  fwrite(&(gal->n_code),sizeof(int32),1,fpout);
  fwrite(&n_cols,sizeof(int32),1,fpout);
    
  for(i=0;i<gal->n_code;i++){
    for(c_id=0;c_id<n_cols;c_id++)
      fwrite(&(vqmean[i][c_id]),sizeof(float32),1,fpout);
    
    for(m_id=0;m_id<g->n_mgau;m_id++){
      for(s_id=0;s_id<g->n_feat;s_id++) {
	/*Compute weighted distance for each code word*/
	for(g_id=0;g_id<g->n_density;g_id++) {
	  w_dists[g_id]=0;
	  for(c_id=0;c_id<n_cols;c_id++) {
	    m=g->mean[m_id][s_id][g_id][c_id];
	    c=vqmean[i][c_id];
	    v=smooth_cov[c_id];
	    w_dists[g_id]+=(c-m)*(c-m)/v;
	  }
		    
	  w_dists[g_id]/=n_cols;
	}

	/*Compute the count as well as indicating whether a gaussian should be computed in a state*/

	gau_count=0;
	
	for(g_id=0;g_id<g->n_density;g_id++){
	  is_computed[g_id]=0;
	  if(w_dists[g_id]<=gal->threshold_low){
	      gau_count++;
	      is_computed[g_id]=1;
	  }
	}

	/*constrain the programming such that at least one gaussian were that 
	  per state*/
		
	if(gau_count<gal->min_count) {
	  sa->size=g->n_density;

	  for(g_id=0,j=0;g_id<g->n_density;g_id++,j++) {
	    sa->s_array[j].key=g_id;
	    sa->s_array[j].val=w_dists[g_id];
	  }
	  insertion_sort(sa);
	  
	  for(j=0;j<gal->min_count;j++)
	    is_computed[sa->s_array[j].key]=1;
	}
		
	for(g_id=0,j=0;g_id<g->n_density;g_id++,j++) {
	    sa->s_array[j].key=g_id;
	    sa->s_array[j].val=w_dists[g_id];
	}

	insertion_sort(sa);

	if(gau_count > gal->min_count) {
	  /* Logic of constraining the maximum number of gaussians within the dual rings */
	  j=0;
	  dr_gau_count=0;
	  for(g_id=0;g_id<g->n_density;g_id++) {
	    if(gal->threshold_low<w_dists[g_id]&&w_dists[g_id]<gal->threshold_high)
	      {
		dr_gau_count++;
		sa->s_array[j].key=g_id;
		sa->s_array[j].val=w_dists[g_id];
		j++;
	      }
	  }
		
	  if(dr_gau_count > gal->dr_max_count) {
	    sa->size=dr_gau_count;
	    insertion_sort(sa);
	    for(j=0;j<gal->dr_max_count;j++)
	      is_computed[sa->s_array[j].key]=1;
	  }else{
	    for(j=0;j<dr_gau_count;j++)
	      is_computed[sa->s_array[j].key]=1;
	  }
	}

	bitvec_clear_all(bv,g->n_density);
	
	for(g_id=0;g_id<g->n_density;g_id++){
	  if(is_computed[g_id]){
	    bitvec_set(bv,g_id);
	  }
	}
	
	/*It is a hack! Please look at bitvec.h to understand the size of the vector */
	/*use function bitvec_uint32size(g->n_density)*/
	fwrite(bv,bitvec_uint32size(g->n_density)*sizeof(uint32),1,fpout);
      }
    }
  }
  /* Very small allocation, should not cause fragmentation error ... */
  bitvec_free(bv);
  fclose(fpout);
}


/* Some notes on dual ring constraints */
/* First case, gau_count < min_count */
/* Then, use the the closest min_count gaussians*/
/* Second case, gau_count > min_count */
/* Then, use all the gau_count, and use dr_min_count gaussians inside the dual ring */

int32 main (int32 argc, char *argv[])
{
  int32 m_id,s_id,g_id,c_id; /* the mixture of gaussian, the stream, the gaussian, the component*/
  int32 max_datarows,n_cols;
  float32 **data, **vqmean,  *smooth_cov;
  int32 *vqmap;
  float64 sqerr;

  float32 *w_dists;
  int32 *is_computed;
  gau_al_struct *gal;
  gauden_t *g;
  senone_t *s;

  float32 min_thres_low, max_thres_low, int_thres_low;
  float32 thres;
  int32 row_id;

  char* outputfilename;

  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc,argv,"default.arg",arg);
  unlimit ();
  
  
  /* ARCHAN: What should be done here:
     1, Read in the parameters including mean, variance, mixture weight and a threhold T
     2, Do a clustering and output the resulting codebook
     3, try to output the code to gaussians mapping 
  */

  gal=gau_al_init(cmd_ln_int32("-ncode"),
		  cmd_ln_float32("-lothres"),
		  cmd_ln_float32("-hithres"),
		  cmd_ln_int32("-mingau"),
		  cmd_ln_int32("-drmaxgau"));


  min_thres_low = cmd_ln_int32("-minlothres");
  max_thres_low = cmd_ln_int32("-maxlothres");
  int_thres_low = cmd_ln_int32("-intlothres");  

  if(min_thres_low>0 && max_thres_low > 0 && int_thres_low >0){
    if(max_thres_low < min_thres_low){
      E_ERROR("max of lothres should be larger than min of lothres\n");
      exit(1);
    }
  }
    
  
  logs3_init (cmd_ln_float32("-logbase"),1,cmd_ln_int32("-log3table"));

    /*Steps 1 : Read in the parameters including mean,variance, mixture weight */
    /*1a, gauden_init */
    /*put the variance floor */

  ms_mgau=ms_mgau_init(cmd_ln_str("-mean"),
		       cmd_ln_str("-var"),cmd_ln_float32("-varfloor"),
		       cmd_ln_str("-mixw"),cmd_ln_float32("-mixwfloor"),
		       0, /* No precomputation */
		       ".cont.",
		       NULL, 100000);
  s=ms_mgau->s;
  g=ms_mgau->g;
    
  /*Step 2: put the stuff correctly into vector_vqgen */
  /*calculate the number of "rows" input to the vq generator
    Number of rows = number of mixture * number of stream in one mixture * number of gaussians in a mixture stream*/
  max_datarows=ms_mgau->s->n_gauden;
  max_datarows*=ms_mgau->s->n_cw; 
  max_datarows*=ms_mgau->s->n_feat;

  n_cols=*(ms_mgau->g->featlen);

#if 1
  E_INFO("Total number of data point: %d\n",max_datarows);
  E_INFO("Parameters: %d %d %d\n",ms_mgau->s->n_gauden,ms_mgau->s->n_cw,ms_mgau->s->n_feat);
  E_INFO("Total number of output code word: %d\n",gal->n_code);
  E_INFO("Total number of columns %d\n",n_cols);
#endif

  /*allocate the memory*/
  data =(float32**) ckd_calloc_2d (max_datarows, n_cols, sizeof(float32));
  vqmean = (float32 **) ckd_calloc_2d (gal->n_code, n_cols, sizeof(float32));
  vqmap = (int32 *) ckd_calloc (max_datarows, sizeof(int32));

  /*read them into the code word into data array */
  /*All zero vector are skipped */
  row_id=0;
  for(m_id=0;m_id<ms_mgau->g->n_mgau;m_id++){
    for(s_id=0;s_id<ms_mgau->g->n_feat;s_id++){
      for(g_id=0;g_id<ms_mgau->g->n_density;g_id++)    {
	if(!vector_is_zero(g->mean[m_id][s_id][g_id],n_cols)){
	  row_id++;
	  for(c_id=0;c_id<n_cols;c_id++){
	    data[row_id][c_id]=g->mean[m_id][s_id][g_id][c_id];
	  }
	}
      }
    }
  }

  E_INFO("The final number of rows %d\n",row_id);
  max_datarows=row_id;
  
  /*give the correct input and output for vector_vqgen */

  /*run vector_vqgen */
  E_INFO("Start clustering\n");
  sqerr =vector_vqgen(data,max_datarows,n_cols,
		      cmd_ln_int32("-ncode"),
		      cmd_ln_float32("-eps"),
		      cmd_ln_int32("-iter"),
		      vqmean,vqmap,cmd_ln_int32("-seed"));


  /*Step 3: Compute the code to gaussians mapping (one to many mapping)*/
  /*3a, improvise ways to compute the mapping */
  
  /*For every gaussians in every mixture-stream, compute the
    weighted distance proposed by Bocchieri 1993
    
    weighted distance is defined as (x-m)^t*Sigma*(x-m) where Sigma
    is the covariance matrix of that gaussian. The paper suggested
    that a smoothed version of Sigma using other gaussians in the
    same mixture stream will be more robust. We will try that at
    here*/

    smooth_cov=ckd_calloc(n_cols, sizeof(int32));    
    w_dists = ckd_calloc(g->n_density, sizeof(float32));
    is_computed= ckd_calloc(g->n_density, sizeof(int32));

    sa=(sort_array_t *)malloc(sizeof(sort_array_t));
    sa->size=g->n_density;
    sa->s_array=(sort_t*)ckd_calloc(sa->size,sizeof(sort_t));


    outputfilename=cmd_ln_str("-outputvq");
    if(min_thres_low > 0 && max_thres_low >0 && int_thres_low > 0){
      for(thres=min_thres_low;thres<max_thres_low;thres+=int_thres_low)	{
	char* tmp;
	tmp=NULL;
	sprintf(tmp,"%f",thres);
	strcat(outputfilename,tmp);
	write_vq_file(ms_mgau->g,
		      vqmean,
		      smooth_cov,w_dists,is_computed,
		      outputfilename,
		      gal);

      }
    }else {
      write_vq_file(g,
		    vqmean,
		    smooth_cov,w_dists,is_computed,
		    outputfilename,
		    gal);
    }

    /*free memory for data */
    gau_al_free(gal);
    ckd_free_2d ((void **) data);
    ckd_free_2d ((void **) vqmean);
    ckd_free((void*)vqmap);
    ckd_free((void*)smooth_cov);
    ckd_free((void*)w_dists);
    ckd_free((void*)is_computed);
    
    ckd_free((sort_t*)(sa->s_array));
    free(s);

  cmd_ln_appl_exit();
  exit(0);

}
