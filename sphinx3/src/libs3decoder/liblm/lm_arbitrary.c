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
 * lm_arbitrary.c - Arbitrary length n-gram data structure 
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
 * (Important: Please do not delete) Started by Arthur Chan at
 * 20050714. genlm_fread and genlm_fwrite are inspired by rr_fread and
 * rr_fwrite in **CMU LM Tookkit version 1** . Oral consent from
 * Prof. Roni Rosenfield granted the use of the code and deriving from
 * the code to release under the Sphinx's license.
 *
 * The **binary file format** (or .binlm file) is compatible with the
 * binLM of CMU LM toolkit version 2.  The binlm reader is largely
 * similar to what one could find in CMU/Cambridage LM toolkit because
 * of the file format.  The data structure, though, is significantly 
 * redesigned to adapt the use in Sphinx 3.X. 
 *
 * There are several major features of the modules:
 * 1, slm_t is now a children structure of genlm_t. 
 * 2, several restrictions of slm_t is removed. 
 * 
 * 
 * $Log$
 * Revision 1.1.2.1  2005/07/17  05:24:31  arthchan2003
 * (Incomplete) Added lm_arbitrary.[ch], an arbitrary n-gram data structure.  Far from completed. Don't expect too much.
 * 
 *
 */

#include <lm_arbitrary.h>
#include <bio.h>

int32 genlm_fread(FILE *fp, 
		  char *valptr, 
		  int32 elemsize, 
		  int32 n_elem,
		  char *headername,
		  int32 checking
		  )
{
  int n_read, i;
  char dummychar;

  if (n_elem > 0) {
     n_read = fread(valptr,elemsize,n_elem,fp);
     if (n_read != n_elem){
       E_INFO("genlm_fread: LM read error, reading %s.  Only %d of %d elements could be read\n",
	 headername, n_read, n_elem);
       return  (GEN_LM_READ_FAIL); 
     }

     if (elemsize == sizeof(int32)) {
        for (i=0; i<n_elem; i++)
	  SWAP_INT32(valptr+(elemsize*i));
     }
     else if (elemsize == sizeof(int16)) {
        for (i=0; i<n_elem; i++) 
	   SWAP_INT16(valptr+(elemsize*i));
     }
     else if (elemsize == sizeof(float64)) {
        for (i=0; i<n_elem; i++)
	   SWAP_FLOAT64(valptr+(elemsize*i));
     }
  }

  if (checking) {
    if (fread(&dummychar,1,1,fp) != 0){
      E_INFO("Try to read one more characer to test whether there are more stuffs after the reading. There is. So there must be something wrong. ");
      return  (GEN_LM_READ_FAIL); 
    }
  }

  return(GEN_LM_READ_SUCCEED); 
}

int32 genlm_read_binlm_header(slm_t *lm, FILE* fp)
{
  if(genlm_fread(fp,
		 (char*)&(lm->ng->n),
		 sizeof(int32),
		 1,
		 "n",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->voc_sz),
		 sizeof(int16),
		 1,
		 "vocab_size"
		 ,0)==GEN_LM_READ_FAIL) 
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->no_ctxt_cue),
		 sizeof(int16),
		 1,
		 "no_ctxt_cue",
		 0)==GEN_LM_READ_FAIL) 
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->voc_type),
		 sizeof(int16),
		 1,
		 "vocab_type"
		 ,0)==GEN_LM_READ_FAIL) 
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->cnt_tab_sz),
		 sizeof(cnt_ind_t),
		 1,
		 "count_table_size",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->disc_method),
		 sizeof(int16),
		 1,
		 "discounting_method",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->min_alpha),
		 sizeof(float64),
		 1,
		 "min_alpha",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->max_alpha),
		 sizeof(float64),
		 1,
		 "min_alpha",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

 
  if(genlm_fread(fp,
		 (char*)&(lm->out_of_range_alphas),
		 sizeof(int16),
		 1,
		 "out_of_range_alphas",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->sz_alpha_array),
		 sizeof(int16),
		 1,
		 "size_of_alpha_array",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->n_ug),
		 sizeof(int32),
		 1,
		 "n_ug",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->zeroton_fract),
		 sizeof(float64),
		 1,
		 "zeroton_fraction",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->fourbyte_counts),
		 sizeof(flag),
		 1,
		 "four_byte_counts",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->fourbyte_alpha),
		 sizeof(flag),
		 1,
		 "four_byte_alphas",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  if(genlm_fread(fp,
		 (char*)&(lm->first_id),
		 sizeof(uint16),
		 1,
		 "first_id",
		 0)==GEN_LM_READ_FAIL)
    return GEN_LM_READ_FAIL;

  return GEN_LM_READ_SUCCEED;
}

slm_t* genlm_read_binlm(char* filename)
{
  int32 usingPipe = FALSE;
  slm_t *lm;
  FILE* bin_fp=fopen_comp(filename,"rb",&usingPipe);

  lm=ckd_calloc(1,sizeof(slm_t));
  
  if(genlm_read_binlm_header(lm, bin_fp)==GEN_LM_READ_FAIL){
    E_INFO("Unable to read the .binlm format binary header\n");
    goto parse_error;
  }
  return lm;

 parse_error:
  ckd_free(lm);
  return NULL;
}


void genlm_free_lm(slm_t *lm)
{
  ckd_free((void*)lm);
}


