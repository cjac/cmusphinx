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
 * s3mdef_s2map.c - Build mapping from S3 (mdef) to S2 (phone/map) structure.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2004/12/10  16:48:56  rkm
 * Added continuous density acoustic model handling
 * 
 * 
 * 19-Nov-2004	Mosur Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Started.
 */


#include "assert.h"
#include "s2types.h"
#include "err.h"
#include "ckd_alloc.h"
#include "s3mdef_s2map.h"


/*
 * Build an s2 format phone string in s2str from the given mdef/s3pid.
 * Caller must allocate s2str.
 */
static void s3pid2s2str (mdef_t *mdef, int32 pid, char *s2str)
{
  s3cipid_t b, l, r;
  word_posn_t pos;
  char *wpos_s2str;
  
  wpos_s2str = WPOS_NAME;
  
  mdef_phone_components (mdef, pid, &b, &l, &r, &pos);
  if (pos == WORD_POSN_INTERNAL) {
    sprintf (s2str, "%s(%s,%s)",
	     mdef_ciphone_str(mdef, b),
	     mdef_ciphone_str(mdef, l),
	     mdef_ciphone_str(mdef, r));
  } else {
    sprintf (s2str, "%s(%s,%s)%c",
	     mdef_ciphone_str(mdef, b),
	     mdef_ciphone_str(mdef, l),
	     mdef_ciphone_str(mdef, r),
	     wpos_s2str[pos]);
  }
}


int32 s2phonefile_write (s3mdef_s2map_t *s3mdef_s2map, char *phonefile)
{
  FILE *fp;
  mdef_t *mdef;
  int32 p, b;
  char phonestr[4096];
  
  E_INFO("Writing %s\n", phonefile);
  
  mdef = s3mdef_s2map->s3mdef;
  
  if ((fp = fopen(phonefile, "w")) == NULL) {
    E_ERROR("fopen(%s,w) failed\n", phonefile);
    return -1;
  }
  
  for (p = 0; p < mdef_n_ciphone(mdef); p++) {
    fprintf (fp, "%-16s  0  0 %5d %5d\n", mdef_ciphone_str(mdef, p), p, p);
  }
  
  for (; p < mdef_n_phone(mdef); p++) {
    s3pid2s2str(mdef, p, phonestr);
    b = mdef_pid2ci(mdef, p);
    
    fprintf (fp, "%-16s -1  0 %5d %5d\n", phonestr, b, p);
  }
  
  fclose (fp);
  
  return 0;
}


int32 s2mapfile_write (s3mdef_s2map_t *s3mdef_s2map, char *mapfile)
{
  FILE *fp;
  mdef_t *mdef;
  int32 p, ssid, i, s3sen, s2sen, b;
  char phonestr[4096];
  
  E_INFO("Writing %s\n", mapfile);
  
  mdef = s3mdef_s2map->s3mdef;
  
  if ((fp = fopen(mapfile, "w")) == NULL) {
    E_ERROR("fopen(%s,w) failed\n", mapfile);
    return -1;
  }
  
  for (p = mdef_n_ciphone(mdef); p < mdef_n_phone(mdef); p++) {
    ssid = mdef_pid2ssid(mdef, p);
    
    s3pid2s2str(mdef, p, phonestr);
    
    for (i = 0; i < mdef_n_emit_state(mdef); i++) {
      s3sen = mdef_sseq2sen(mdef, ssid, i);
      s2sen = s3mdef_s2map->s2map[s3sen];
      
      b = mdef_pid2ci(mdef, p);
      
      fprintf (fp, "%s<%d>\t%5d\n",
	       phonestr, i,
	       s2sen - s3mdef_s2map->s2senbase[b] + 1);
    }
  }
  
  fclose (fp);
  
  return 0;
}


static void s2map_build (s3mdef_s2map_t *s3mdef_s2map)
{
  int32 *s2map, *s2senbase, *s3map;
  mdef_t *mdef;
  int32 p, i, n;
  int32 s3sen, s2sen;
  
  mdef = s3mdef_s2map->s3mdef;
  
  /* Allocate S3<->S2 senone-ID mapping arrays */
  s2map = (int32 *) ckd_calloc (mdef_n_sen(mdef), sizeof(int32));
  s3map = (int32 *) ckd_calloc (mdef_n_sen(mdef), sizeof(int32));
  s3mdef_s2map->s2map = s2map;
  s3mdef_s2map->s3map = s3map;
  s2senbase = (int32 *) ckd_calloc (mdef_n_ciphone(mdef), sizeof(int32));
  s3mdef_s2map->s2senbase = s2senbase;
  
  /*
   * For each CIphone, build mapping from S3 senid to S2 senid
   */
  
  s3sen = mdef_n_ciphone(mdef) * mdef_n_emit_state(mdef); /* start of s3 CDsen */
  s2sen = 0;	/* start of s2 sen */
  
  for (p = 0; p < mdef_n_ciphone(mdef); p++) {
    s2senbase[p] = s2sen;
    
    n = mdef_ci2n_cdsen(mdef, p);	/* #CD senones for this CI phone */
#if 0
    E_INFO("#CDsen(%s) = %d\n", mdef_ciphone_str(mdef, p), n);
#endif
    /* Add maps for these s3 CD sen for this phone p */
    for (i = 0; i < n; i++) {
      s2map[s3sen] = s2sen;
      s3map[s2sen] = s3sen;
      s2sen++;
      s3sen++;
    }
    
    /* Add maps for s3 CI sens for this phone p */
    for (i = 0; i < mdef_n_emit_state(mdef); i++) {
      s2map[p * mdef_n_emit_state(mdef) + i] = s2sen;
      s3map[s2sen] = p * mdef_n_emit_state(mdef) + i;
      s2sen++;
    }
  }
  
  assert (s3sen = mdef_n_sen(mdef));
  assert (s2sen == s3sen);
  
#if 0
  for (i = 0; i < mdef_n_ciphone(mdef); i++)
    E_INFO("s2senbase(%s) = %d\n", mdef_ciphone_str(mdef, i), s2senbase[i]);
  E_INFO("S3sen -> S2sen, S2 -> S3sen ...\n");
  for (i = 0; i < s3sen; i++)
    E_INFO("[%5d]  %5d  %5d\n", i, s2map[i], s3map[i]);
#endif
}


s3mdef_s2map_t *s3mdef_s2map_init (char *mdeffile)
{
  s3mdef_s2map_t *s3mdef_s2map;
  mdef_t *mdef;
  
  s3mdef_s2map = (s3mdef_s2map_t *) ckd_calloc (1, sizeof(s3mdef_s2map_t));
  
  mdef = mdef_init (mdeffile);
  assert (mdef != NULL);
  assert (mdef_n_emit_state(mdef) == 5);
  
  s3mdef_s2map->s3mdef = mdef;
  
  s2map_build (s3mdef_s2map);
  
  return s3mdef_s2map;
}
