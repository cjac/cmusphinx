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
 * feat.c -- Feature vector description and cepstra->feature computation.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Adding feat_free() to free allocated memory
 *
 * 02-Jan-2001	Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Modified feat_s2mfc2feat_block() to handle empty buffers at
 *		the end of an utterance
 *
 * 30-Dec-2000	Rita Singh (rsingh@cs.cmu.edu) at Carnegie Mellon University
 *		Added feat_s2mfc2feat_block() to allow feature computation
 *		from sequences of blocks of cepstral vectors
 *
 * 12-Jun-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Major changes to accommodate arbitrary feature input types.  Added
 * 		feat_read(), moved various cep2feat functions from other files into
 *		this one.  Also, made this module object-oriented with the feat_t type.
 * 		Changed definition of s2mfc_read to let the caller manage MFC buffers.
 * 
 * 03-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added unistd.h include.
 * 
 * 02-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added check for sf argument to s2mfc_read being within file size.
 * 
 * 18-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added sf, ef parameters to s2mfc_read().
 * 
 * 10-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added feat_cepsize().
 * 		Added different feature-handling (s2_4x, s3_1x39 at this point).
 * 		Moved feature-dependent functions to feature-dependent files.
 * 
 * 09-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Moved constant declarations from feat.h into here.
 * 
 * 04-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


/*
 * This module encapsulates different feature streams used by the Sphinx group.  New
 * stream types can be added by augmenting feat_init() and providing an accompanying
 * compute_feat function.  It also provides a "generic" feature vector definition for
 * handling "arbitrary" speech input feature types (see the last section in feat_init()).
 * In this case the speech input data should already be feature vectors; no computation,
 * such as MFC->feature conversion, is available or needed.
 */


#include <libutil/libutil.h>
#include "libs3decoder/feat.h"
#include "feat_dump.h"
#include "libs3decoder/bio.h"
#include "libs3decoder/cmn.h"
#include "libs3decoder/agc.h"
#include "s3types.h"

#if (! WIN32)
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/param.h>
#else
#include <fcntl.h>
#endif


#define FEAT_VERSION	"1.0"



/* Feature computation module for live mode decoder. Computes featuers
 * for incoming blocks of data. Maintains a cyclic buffer of size 256
 * for computing cross-block deltas etc. Needs explicit begin-utterance
 * and end-of-utterance flags to be set to indicate the beginning of
 * a new utterance or the end of an utterance in order to function
 * properly
 * The cyclic buffer of size 256 was controlled by using an unsigned
 * char. Replaced it so that the pointers into the buffer cycle, according
 * to the variable LIVEBUFBLOCKSIZE. This was, if one day we decide
 * to change this variable from 256 to something else, the cyclic buffer
 * will still work.  (ebg)
 */
int32 feat_dump_s2mfc2feat_block(feat_t *fcb, float32 **uttcep, int32 nfr,
			      int32 beginutt, int32 endutt, float32 ***ofeat)
{
    static float32 **feat=NULL;
    static float32 **cepbuf=NULL;
    /*    static int32 nfr_allocated = 0; */ /* Variable never used. - EBG */
    /*    static unsigned char   bufpos, curpos;   */
    /*    static unsigned char  jp1, jp2, jp3, jf1, jf2, jf3;	   */
    static int32   bufpos; /*  RAH 4.15.01 upgraded unsigned char variables to int32*/
    static int32   curpos; /*  RAH 4.15.01 upgraded unsigned char variables to int32*/
    static int32  jp1, jp2, jp3, jf1, jf2, jf3; /* RAH 4.15.01 upgraded unsigned char variables to int32 */
    int32  win, cepsize; 
    int32  i, j, nfeatvec, residualvecs;

    float32 *w, *_w, *f;
    float32 *w1, *w_1, *_w1, *_w_1;
    float32 d1, d2;

    /* If this assert fails, you're risking overwriting elements
     * in the buffer. -EBG */
    assert(nfr < LIVEBUFBLOCKSIZE);
    win = feat_window_size(fcb);

    metricsStart("cmn");

    if (fcb->cmn) /* Only cmn_prior in block computation mode */
	cmn_prior (uttcep, fcb->varnorm, nfr, fcb->cepsize, endutt);

    metricsStop("cmn");


    metricsStart("FeatureExtractor");

    if (fcb->cepsize <= 0) 
	E_FATAL("Bad cepsize: %d\n", fcb->cepsize);
    cepsize = feat_cepsize(fcb);
    if (feat == NULL)
	feat = (float32 **)ckd_calloc_2d(LIVEBUFBLOCKSIZE,
					 feat_stream_len(fcb,0),
					 sizeof(float32));
    if (cepbuf == NULL){
	cepbuf = (float32 **)ckd_calloc_2d(LIVEBUFBLOCKSIZE,
					 cepsize,
					 sizeof(float32));
	beginutt = 1; /* If no buffer was present we are beginning an utt */
    if (! feat)
      E_FATAL("Unable to allocate feat ckd_calloc_2d(%ld,%d,%d)\n",LIVEBUFBLOCKSIZE,feat_stream_len(fcb,0),sizeof(float32));
    if (! cepbuf)
      E_FATAL("Unable to allocate cepbuf ckd_calloc_2d(%ld,%d,%d)\n",LIVEBUFBLOCKSIZE,cepsize,sizeof(float32));
	E_INFO("Feature buffers initialized to %d vectors\n",LIVEBUFBLOCKSIZE);
    }



    residualvecs = 0;
    if (beginutt){
	/* Replicate first frame into the first win frames */
	for (i=0;i<win;i++) 
	   memcpy(cepbuf[i],uttcep[0],cepsize*sizeof(float32));
	/* beginutt = 0; */  /* Removed by Rita Singh around 02-Jan-2001 */
                             /* See History at the top of this file */
	bufpos = win;
	bufpos %= LIVEBUFBLOCKSIZE;
        curpos = bufpos;
        jp1 = curpos - 1;
	jp1 %= LIVEBUFBLOCKSIZE;
        jp2 = curpos - 2;
	jp2 %= LIVEBUFBLOCKSIZE;
        jp3 = curpos - 3;
	jp3 %= LIVEBUFBLOCKSIZE;
        jf1 = curpos + 1;
	jf1 %= LIVEBUFBLOCKSIZE;
        jf2 = curpos + 2;
	jf2 %= LIVEBUFBLOCKSIZE;
        jf3 = curpos + 3;
	jf3 %= LIVEBUFBLOCKSIZE;
	residualvecs -= win;
    }

    for (i=0;i<nfr;i++){
      assert(bufpos < LIVEBUFBLOCKSIZE);
	memcpy(cepbuf[bufpos++],uttcep[i],cepsize*sizeof(float32));
	bufpos %= LIVEBUFBLOCKSIZE;
    }

    if (endutt){
	/* Replicate last frame into the last win frames */
	if (nfr > 0) {
	  for (i=0;i<win;i++) {
	    assert(bufpos < LIVEBUFBLOCKSIZE);
	   memcpy(cepbuf[bufpos++],uttcep[nfr-1],cepsize*sizeof(float32));
	   bufpos %= LIVEBUFBLOCKSIZE;
	  }
        }
	else {
	    int16 tpos = bufpos-1;
	    tpos %= LIVEBUFBLOCKSIZE;
	    for (i=0;i<win;i++) {
	      assert(bufpos < LIVEBUFBLOCKSIZE);
	        memcpy(cepbuf[bufpos++],cepbuf[tpos],cepsize*sizeof(float32));
		bufpos %= LIVEBUFBLOCKSIZE;
	    }
	}
        residualvecs += win;
    }

    /* Create feature vectors */
    nfeatvec = 0;
    nfr += residualvecs;

    for (i = 0; i < nfr; i++,nfeatvec++){
        /* CEP; skip C0 */
        memcpy (feat[i], cepbuf[curpos]+1, (cepsize-1) * sizeof(float32));
    
        /*
         * DCEP: mfc[2] - mfc[-2];
         */
        f = feat[i] + cepsize - 1;
        w  = cepbuf[jf2] + 1;	/* +1 to skip C0 */
        _w = cepbuf[jp2] + 1;

        for (j = 0; j < cepsize-1; j++)
	    f[j] = w[j] - _w[j];
    
        /* POW: C0, DC0, D2C0 */
        f += cepsize-1;

        f[0] = cepbuf[curpos][0];
        f[1] = cepbuf[jf2][0] - cepbuf[jp2][0];

        d1 = cepbuf[jf3][0] - cepbuf[jp1][0];
        d2 = cepbuf[jf1][0] - cepbuf[jp3][0];
        f[2] = d1 - d2;

        /* D2CEP: (mfc[3] - mfc[-1]) - (mfc[1] - mfc[-3]) */
        f += 3;
    
        w1   = cepbuf[jf3] + 1;	/* Final +1 to skip C0 */
        _w1  = cepbuf[jp1] + 1;
        w_1  = cepbuf[jf1] + 1;
        _w_1 = cepbuf[jp3] + 1;

        for (j = 0; j < cepsize-1; j++) {
	    d1 =  w1[j] -  _w1[j];
	    d2 = w_1[j] - _w_1[j];

	    f[j] = d1 - d2;
        }
	jf1++; jf2++; jf3++;
	jp1++; jp2++; jp3++;
	curpos++;
	jf1 %= LIVEBUFBLOCKSIZE;
	jf2 %= LIVEBUFBLOCKSIZE;
	jf3 %= LIVEBUFBLOCKSIZE;
	jp1 %= LIVEBUFBLOCKSIZE;
	jp2 %= LIVEBUFBLOCKSIZE;
	jp3 %= LIVEBUFBLOCKSIZE;
	curpos %= LIVEBUFBLOCKSIZE;
    }
    *ofeat = feat;

    metricsStop("FeatureExtractor");

    return(nfeatvec);
}
