/* ====================================================================
 * Copyright (c) 1994-2001 Carnegie Mellon University.  All rights
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
 * 
 * HISTORY
 * 
 * circa 1994	P J Moreno at Carnegie Mellon
 * 		Created.
 *
 * For history information, please use 'cvs log'
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libutil/libutil.h>
#include "bio.h"

#define IO_ERR  (-1)
#define IO_SUCCESS  (0)

/* Default cepstral vector size */
#define NUM_COEFF  "13"

/* Default display size, less than the vector size so we display one
 * frame per line.
 */
#define DISPLAY_SIZE "8"

static arg_t arg[] = {
  { "-i",
    ARG_INT32,
    NUM_COEFF,
    "Number of coefficients in the feature vector."},
  { "-d",
    ARG_INT32,
    DISPLAY_SIZE,
    "Width of display"},
  { "-input",
    ARG_STRING,
    NULL,
    "Input cepstral file"},
  { "-logfn",
    ARG_STRING,
    NULL,
    "Log file (default stdout/stderr)" },
  { NULL, ARG_INT32,  NULL, NULL }
};

int read_cep(char *file, float ***cep, int *nframes, int numcep);

int main(int argc, char *argv[])
{
  int i, j, offset;
  int len, vsize, dsize, column;
  float *z, **cep;
  char* cepfile;
  
  print_appl_info(argv[0]);
  cmd_ln_appl_enter(argc, argv, "default.arg", arg);
  vsize = cmd_ln_int32("-i");
  dsize = cmd_ln_int32("-d");
  
  cepfile=ckd_salloc(argv[argc-1]);
  if (!cmd_ln_access("-input")){
    E_FATAL("input file is not specified\n");
  }
  if (read_cep((char *)cmd_ln_access("-input"), &cep, &len, vsize) == IO_ERR)
    E_INFO("ERROR opening %s for reading\n", cepfile);
      
  z = cep[0];
  printf("%d frames\n", len);

  offset = 0;
  column = (vsize > dsize) ? dsize : vsize;

  printf("\n%13s%8s%8s%8s%8s\n\n", "frame#", "c[0]", "c[1]", "c[2]", "...");
  for (i = 0; i < len; ++i)
    {
      printf("Frame> %6d ", i);
      for ( j = 0 ; j < column; ++j)
	printf("%7.3f ", z[offset + j]);
      printf("\n");
      offset += vsize;
    }
  fflush(stdout);
  cmd_ln_appl_exit();

  return(0);

}

int read_cep(char *file, float***cep, int *numframes, int cepsize)
{
    FILE *fp;
    int n_float;
    struct stat statbuf;
    int i, n, byterev, sf, ef;
    float32 **mfcbuf;

    if (stat(file, &statbuf) < 0) {
        printf("stat_retry(%s) failed\n", file);
        return IO_ERR;
    }

    if ((fp = fopen(file, "rb")) == NULL) {
	printf("fopen(%s, rb) failed\n", file);
	return IO_ERR;
    }
    
    /* Read #floats in header */
    if (fread(&n_float, sizeof(int), 1, fp) != 1) {
	fclose (fp);
	return IO_ERR;
    }
    
    /* Check if n_float matches file size */
    byterev = FALSE;
    if ((int)(n_float*sizeof(float) + 4) != statbuf.st_size) {
	n = n_float;
	SWAP_INT32(&n);

	if ((int)(n*sizeof(float) + 4) != statbuf.st_size) {
	    printf("Header size field: %d(%08x); filesize: %d(%08x)\n",
		    n_float, n_float, (int)statbuf.st_size, (int)statbuf.st_size);
	    fclose (fp);
	    return IO_ERR;
	}

	n_float = n;
	byterev = TRUE;
    }
    if (n_float <= 0) {
	printf("Header size field: %d\n",  n_float);
	fclose (fp);
	return IO_ERR;
    }
    
    /* n = #frames of input */
    n = n_float/cepsize;
    if (n * cepsize != n_float) {
	printf("Header size field: %d; not multiple of %d\n", n_float, cepsize);
	fclose (fp);
	return IO_ERR;
    }
    sf = 0;
    ef = n;

    mfcbuf = (float **) ckd_calloc_2d (n, cepsize, sizeof(float32));
    
    /* Read mfc data and byteswap if necessary */
    n_float = n * cepsize;
    if ((int)fread (mfcbuf[0], sizeof(float), n_float, fp) != n_float) {
	printf("Error reading mfc data\n");
	fclose (fp);
	return IO_ERR;
    }
    if (byterev) {
      for (i = 0; i < n_float; i++)
	SWAP_FLOAT32(&(mfcbuf[0][i]));
    }
    fclose (fp);

    *numframes = n;
    *cep = mfcbuf;
    return IO_SUCCESS;
}
