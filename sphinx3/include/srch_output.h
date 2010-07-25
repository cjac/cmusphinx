/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

/* srch_output.h
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.2  2006/02/23 05:13:26  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: A wrapper for the outptu functions of the search routine
 *
 * Revision 1.1.2.1  2006/01/17 01:24:21  arthchan2003
 * The home of log output functions.
 *
 */

#include <stdio.h>

#include <sphinxbase/glist.h>

#include <s3types.h>
#include <dict.h>
#include <lm.h>
#include <search.h>

#ifndef _SRCH_OUTPUT_H_
#define _SRCH_OUTPUT_H_


#ifdef __cplusplus
extern "C" {
#endif
#if 0
/* Fool Emacs. */
}
#endif

#define HYPSEG_SUCCESS 1
#define HYPSEG_FAILURE 0
#define WORST_CONFIDENCE_SCORE ((int)0xE0000000)

int32 compute_scale(int32 sf, int32 ef, int32* scalearray);

/** write a match file 

NOTE: Current match_write has four features which is different with log_hypstr. 
1, match_write allows the use of hdr. 
2, log_hypstr allows matchexact in output. 
3, log_hypstr allows output the score after the match file name. 
4, log_hypstr will dump the pronounciation variation to the code. 

I don't think they are very important in processing so I removed them. 

*/
S3DECODER_EXPORT
void match_write (FILE *fp,  /**< The file pointer */
		  glist_t hyp, /**< A link-list that contains the hypothesis */
		  char* uttid, /**< Utterance id */
		  dict_t* dict, /**< The dictionary */
		  char *hdr    /**< The header */
    );

/** write match segment */
S3DECODER_EXPORT
void matchseg_write (FILE *fp,  /**< The file pointer */
		     glist_t hyp, /**< A link-list that contains the hypotheesis*/
		     char *uttid, /**< Utterance ID of the file */
		     char *hdr, /**< The header */
		     lm_t * lm, /**< Language model */
		     dict_t * dict, /**< Dictionary */
		     int32 num_frm, /**< Number of frames */
		     int32 *scl,     /**< Scale of the decoding. Required, if inverse normalization 
				      */
		     int32 unnorm   /**< Whether unscaled the score back */
    );



/** 
    wrapping up the detail match display. Comparable with log_hyp_detailed . The only
    difference is match_detailed use a link list of srch_hyp_t 
*/
void match_detailed(FILE* fp, /**< The file pointer */
		    glist_t hyp,  /**< A link-list that containt the hypothesis */
		    char* uttid,  /**< The utterance ID */
		    char* LBL,    /**< A header in cap */
		    char* lbl,    /**< A header in small */
		    int32* senscale, /**< Senone scale vector, 
					if specified, normalized score would be displayed, 
					if not, the unormalized score would be displayed. 
				     */
		    dict_t *dict  /**< Dictionary */
    );


/**
   A funtion that reads the s3 hypseg line. 
*/
S3DECODER_EXPORT
int read_s3hypseg_line(char *line,  /**< A line pointer */
		       seg_hyp_line_t *seg_hyp_line,  /**< A hypseg line structure */
		       lm_t* lm,  /**< A LM */
		       dict_t *dict /**< A dictionary */
    );

S3DECODER_EXPORT
int free_seg_hyp_line(seg_hyp_line_t *seg_hyp_line);

/** CODE DUPLICATION!!! Sphinx 3.0 family of logging hyp and hyp segments 
    When hyp_t, srch_hyp_t are united, we could tie it with match_write
    (20051109) ARCHAN: The only consumer of log_hypstr now is main_dag.c

*/
S3DECODER_EXPORT
void log_hypstr (FILE *fp,  /**< A file pointer */
		 srch_hyp_t *hypptr,  /**< A srch_hyp_t */
		 char *uttid,   /**< An utterance ID */
		 int32 exact,   /**< Whether to dump an exact */
		 int32 scr,      /**< The score */
		 dict_t *dict    /**< A dictionary to look up wid */
    );


S3DECODER_EXPORT
void log_hyp_detailed (FILE *fp, /**< A file poointer */
		       srch_hyp_t *hypptr,  /**< A srch_hyp_t */
		       char *uttid,         /**< An utternace ID */
		       char *LBL,           /**< A header in cap */
		       char *lbl,           /**< A header in small */
		       int32* senscale      /**< Senone scale vector, 
					       if specified, normalized score would be displayed, 
					       if not, the unormalized score would be displayed. 
					    */
    );

/** CODE DUPLICATION!!! Sphinx 3.0 family of logging hyp and hyp segments 
    When hyp_t, srch_hyp_t are united, we could tie it with match_write
    (20051109) ARCHAN: The only consumer of log_hypseg now is main_dag.c
*/

void log_hypseg (char *uttid,   /**< Input; uttid */
		 FILE *fp,	/**< Out: output file */
		 srch_hyp_t *hypptr,	/**< In: Hypothesis */
		 int32 nfrm,	/**< In: \#frames in utterance */
		 int32 scl,	/**< In: Acoustic scaling for entire utt */
		 float64 lwf,	/**< In: LM score scale-factor (in dagsearch) */
		 dict_t* dict,  /**< In: dictionary */
		 lm_t *lm,       /**< In: LM */
		 int32 unnorm   /**< Whether unscaled the score back */
    );

#ifdef __cplusplus
}
#endif


#endif /** _SRCH_OUTPUT_H_*/
