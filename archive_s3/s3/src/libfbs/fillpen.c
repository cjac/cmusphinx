/* ====================================================================
 * Copyright (c) 1995-2002 Carnegie Mellon University.  All rights
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
 * fillpen.c -- Filler penalties (penalties for words that do not show up in
 * the main LM.
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
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libutil/libutil.h>
#include <s3.h>

#include "fillpen.h"
#include "dict.h"
#include "logs3.h"


typedef struct fillpen_s {
    int32 *prob;	/* Filler word probability (in logs3 space, after
			   langwt and inspen application) */
    s3wid_t start;	/* Dictionary wid of first and last filler words */
    s3wid_t end;
    float64 lw;		/* Language weight */
    float64 wip;	/* Word insertion penalty */
} fillpen_t;
static fillpen_t _fillpen;	/* Underscore to distinguish it from function */


void fillpen_init (char *file, s3wid_t start, s3wid_t end)
{
    s3wid_t w, bw;
    float32 prob, lw, wip;
    FILE *fp;
    char line[1024], wd[1024];
    int32 k;

    assert (end >= start);

    lw = *((float32 *) cmd_ln_access("-langwt"));
    wip = *((float32 *) cmd_ln_access("-inspen"));
    
    _fillpen.start = start;
    _fillpen.end = end;
    _fillpen.lw = lw;
    _fillpen.wip = wip;
    _fillpen.prob = (int32 *) ckd_calloc (end - start + 1, sizeof(int32));

    /* Initialize all words with filler penalty (HACK!! backward compatibility) */
    prob = *((float32 *) cmd_ln_access("-noisepen"));
    for (w = start; w <= end; w++)
	_fillpen.prob[w - start] = (int32) (logs3(prob) * lw + logs3(wip));

    /* Overwrite silence penalty (HACK!! backward compatibility) */
    w = dict_wordid (SILENCE_WORD);
    assert (IS_WID(w) && (w >= start) && (w <= end));
    prob = *((float32 *) cmd_ln_access("-silpen"));
    _fillpen.prob[w - start] = (int32) (logs3(prob) * lw + logs3(wip));
    
    /* Overwrite with filler prob input file, if specified */
    if (! file)
	return;
    E_INFO("Reading filler penalty file: %s\n", file);
    if ((fp = fopen (file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')	/* Skip comment lines */
	    continue;
	
	k = sscanf (line, "%s %f", wd, &prob);
	if ((k != 0) && (k != 2))
	    E_FATAL("Bad input line: %s\n", line);
	w = dict_wordid(wd);
	if (NOT_WID(w))
	    E_FATAL("Unknown word: %s\n", wd);
	if ((w < start) || (w > end))
	    E_FATAL("%s not in filler dictionary\n", wd);
	
	_fillpen.prob[w - start] = (int32) (logs3(prob) * lw + logs3(wip));
    }
    fclose (fp);

    /* Replicate fillpen values for alternative pronunciations */
    for (w = start; w <= end; w++) {
	bw = dict_basewid (w);
	if (bw != w)
	    _fillpen.prob[w - start] = _fillpen.prob[bw - start];
    }
}


int32 fillpen (s3wid_t w)
{
    assert ((w >= _fillpen.start) && (w <= _fillpen.end));
    return (_fillpen.prob[w - _fillpen.start]);
}
