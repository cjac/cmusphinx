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
 * s3mdef_s2map.h - Mapping from S3 (mdef) to S2 (phone/map) structure.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2004/12/10  16:48:58  rkm
 * Added continuous density acoustic model handling
 * 
 * 
 * 16-Nov-2004	Mosur Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */


#ifndef __S3MDEF_S2MAP_H__
#define __S3MDEF_S2MAP_H__


#include "mdef.h"


typedef struct s3mdef_s2map_s {
  mdef_t *s3mdef;
  
  /*
   * Order of senones in S3 is different from that in S2.
   * Given phones p1, p2, p3..., in S3 (mdef) the order is:
   *   CI(p1), CI(p2), CI(p3) ... , CD(p1), CD(p2), CD(p3) ...
   * where CI(p) is all CI senones for phone p, and CD(p) is all CD senones
   * for phone p.
   * But in S2 (map), the order is:
   *   CD(p1), CI(p1), CD(p2), CI(p2), CD(p3), CI(p3) ...
   * Hence, we need a permutation mapping from the S3 order to S2 order.
   * 
   * s2map[s], for an S3 senone-ID s, is the corresponding S2 senone-ID;
   * i.e. s2map[s3senid] = s2senid.
   * 
   * s2senbase[p] = first (CD)s2sen for phone p.
   */
  int32 *s2map;
  int32 *s2senbase;
  
  /* Similarly, s3map is the inverse of s2map; s3map[s2senid] = s3senid */
  int32 *s3map;
} s3mdef_s2map_t;


/*
 * Initialize the S3 model definition module with the given S3 model
 * definition file and return the created object.
 */
s3mdef_s2map_t *s3mdef_s2map_init (char *mdeffile);


/*
 * Write an S2 format phone file for the given s3mdef structure.
 * Return 0 if successful, -1 otherwise.
 */
int32 s2phonefile_write (s3mdef_s2map_t *s3mdef_s2map,
			 char *phonefile);

/*
 * Write an S2 format map file for the given s3mdef structure.
 * Return 0 if successful, -1 otherwise.
 */
int32 s2mapfile_write (s3mdef_s2map_t *s3mdef_s2map,
		       char *mapfile);


#endif
