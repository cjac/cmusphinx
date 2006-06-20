/* ====================================================================
 * Copyright (c) 1999-2006 Carnegie Mellon University.  All rights
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



/* Each of the integers 0 ... 65535 are associated with a floating
point number. For the lower integers, the floating point numbers with
which they are associated are equally log-linearly spaced across the
range specified by the -min_alpha and -max_alpha values (note that
these values correspond to the log to the base 10 of the backoff
weights). The last N integers (where N is specified by the
-out_of_range_alphas switch) correspond exactly to values of alpha
which are encountered which fall outside this range. Therefore, a
small range with a small value of N will allow the alphas to be
represented most accurately, but if more than N out of range alphas
are encountered, then the program will fail. The default values are
min_alpha = -3.2, max_alpha, max_alpha = 2.5, out_of_range_alphaa =
10000.  This method does, however, introduce slight inaccuracies, and
so it is only recommended when memory is short. By default, the alphas
are stored in four bytes. */

/*
  $Log: two_byte_alphas.c,v
 */


#include <math.h>
#include "ngram.h"

unsigned short short_alpha(double long_alpha,
			   double *alpha_array,
			   unsigned short *size_of_alpha_array,
			   int elements_in_range,
			   double min_range,
			   double max_range) {

  double log_10_alpha;
  unsigned short short_version;

  if (long_alpha > 0.0) {
    log_10_alpha = log10(long_alpha);
  }
  else {
    log_10_alpha = BAD_LOG_PROB;
  }

  if (log_10_alpha >= min_range && log_10_alpha <= max_range) {    
    short_version = (unsigned short) (elements_in_range * 
				      (log_10_alpha - min_range) / 
				      (max_range - min_range));
  }else {
    int i;

    for (i=0;i<=(*size_of_alpha_array)-1;i++) {
      if (alpha_array[i] == log_10_alpha)
	return(elements_in_range + i);
    }

    if ((elements_in_range + *size_of_alpha_array) >= 65535) {
      quit(-1,"Error : Number of out-of-range alphas exceeds %d. Rerun with a bigger \nrange (use the -min_alpha and -max_alpha options), or a higher number of \nallowed out of range alphas (use the -out_of_range_alphas options).\n",65535-elements_in_range);
    }
    
    alpha_array[*size_of_alpha_array] = log_10_alpha;
    (*size_of_alpha_array)++;
    short_version = elements_in_range + (*size_of_alpha_array);
    
  }
  
  return(short_version);

}

double double_alpha(unsigned short short_alpha,
		    double *alpha_array,
		    int size_of_alpha_array,
		    int elements_in_range,
		    double min_range,
		    double max_range) {
  
  /* Returns the actual (ie NOT log) value of the alpha */

  double log_10_alpha;

  if (short_alpha > (elements_in_range + size_of_alpha_array))
    quit(-1,"Error : two-byte alpha value is out of range. short alpha = %d\n",short_alpha);

  if (short_alpha > elements_in_range)
    log_10_alpha = alpha_array[short_alpha-elements_in_range];
  else 
    log_10_alpha = min_range + (short_alpha * (max_range - min_range)/ elements_in_range);

  return(pow(10.0,log_10_alpha));
}
