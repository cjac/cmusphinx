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


#include <libutil/libutil.h>
#include <libmisc/libmisc.h>
#include "fillpen.h"


fillpen_t *fillpen_init (dict_t *dict, char *file, float64 silprob, float64 fillprob,
			 float64 lw, float64 wip)
{
    s3wid_t w, bw;
    float32 prob;
    FILE *fp;
    char line[1024], wd[1024];
    int32 k;
    fillpen_t *_fillpen;
    
    _fillpen = (fillpen_t *) ckd_calloc (1, sizeof(fillpen_t));
    
    _fillpen->dict = dict;
    _fillpen->lw = lw;
    _fillpen->wip = wip;
    if (dict->filler_end >= dict->filler_start)
	_fillpen->prob = (int32 *) ckd_calloc (dict->filler_end - dict->filler_start + 1,
					       sizeof(int32));
    else
	_fillpen->prob = NULL;
    
    /* Initialize all words with filler penalty (HACK!! backward compatibility) */
    prob = fillprob;
    for (w = dict->filler_start; w <= dict->filler_end; w++)
	_fillpen->prob[w - dict->filler_start] = (int32) (logs3(prob) * lw + logs3(wip));

    /* Overwrite silence penalty (HACK!! backward compatibility) */
    w = dict_wordid (dict, SILENCE_WORD);
    if (NOT_WID(w) || (w < dict->filler_start) || (w > dict->filler_end))
	E_FATAL("%s not a filler word in the given dictionary\n", SILENCE_WORD);
    prob = silprob;
    _fillpen->prob[w - dict->filler_start] = (int32) (logs3(prob) * lw + logs3(wip));
    
    /* Overwrite with filler prob input file, if specified */
    if (! file)
	return _fillpen;
    
    E_INFO("Reading filler penalty file: %s\n", file);
    if ((fp = fopen (file, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", file);
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')	/* Skip comment lines */
	    continue;
	
	k = sscanf (line, "%s %f", wd, &prob);
	if ((k != 0) && (k != 2))
	    E_FATAL("Bad input line: %s\n", line);
	w = dict_wordid(dict, wd);
	if (NOT_WID(w) || (w < dict->filler_start) || (w > dict->filler_end))
	    E_FATAL("%s not a filler word in the given dictionary\n", SILENCE_WORD);
	
	_fillpen->prob[w - dict->filler_start] = (int32) (logs3(prob) * lw + logs3(wip));
    }
    fclose (fp);
    
    /* Replicate fillpen values for alternative pronunciations */
    for (w = dict->filler_start; w <= dict->filler_end; w++) {
	bw = dict_basewid (dict, w);
	if (bw != w)
	    _fillpen->prob[w-dict->filler_start] = _fillpen->prob[bw-dict->filler_start];
    }
    
    return _fillpen;
}


int32 fillpen (fillpen_t *f, s3wid_t w)
{
    assert ((w >= f->dict->filler_start) && (w <= f->dict->filler_end));
    return (f->prob[w - f->dict->filler_start]);
}
