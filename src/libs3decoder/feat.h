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
 * feat.h -- Cepstral features computation.
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
 * 20.Apr.2001  RAH (rhoughton@mediasite.com, ricky.houghton@cs.cmu.edu)
 *              Adding feat_free() to free allocated memory
 * 
 * 04-Jan-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_FEAT_H_
#define _S3_FEAT_H_

#include <libutil/libutil.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIVEBUFBLOCKSIZE        256    /* Blocks of 256 vectors allocated 
					  for livemode decoder */


/*
 * Structure for describing a speech feature type (no. of streams and stream widths),
 * as well as the computation for converting the input speech (e.g., Sphinx-II format
 * MFC cepstra) into this type of feature vectors.
 */
typedef struct feat_s {
    char *name;		/* Printable name for this feature type */
    int32 cepsize;	/* Size of input speech vector (typically, a cepstrum vector) */
    int32 cepsize_used;	/* No. of cepstrum vector dimensions actually used (0 onwards) */
    int32 n_stream;	/* #Feature streams; e.g., 4 in Sphinx-II */
    int32 *stream_len;	/* Vector length of each feature stream */
    int32 window_size;	/* #Extra frames around given input frame needed to compute
			   corresponding output feature (so total = window_size*2 + 1) */
    int32 cmn;		/* Whether CMN is to be performed on each utterance */
    int32 varnorm;	/* Whether variance normalization is to be performed on each utt;
			   Irrelevant if no CMN is performed */
    int32 agc;		/* Whether AGC-Max is to be performed on each utterance */
    void (*compute_feat)(struct feat_s *fcb, float32 **input, float32 **feat);
			/* Function for converting window of input speech vector
			   (input[-window_size..window_size]) to output feature vector
			   (feat[stream][]).  If NULL, no conversion available, the
			   speech input must be feature vector itself.
			   Return value: 0 if successful, -ve otherwise. */
} feat_t;

/* Access macros */
#define feat_name(f)		((f)->name)
#define feat_cepsize(f)		((f)->cepsize)
#define feat_cepsize_used(f)	((f)->cepsize_used)
#define feat_n_stream(f)	((f)->n_stream)
#define feat_stream_len(f,i)	((f)->stream_len[i])
#define feat_window_size(f)	((f)->window_size)


/*
 * Read feature vectors from the given file.  Feature file format:
 *   Line containing the single word: s3
 *   File header including any argument value pairs/line and other text (e.g.,
 * 	'chksum0 yes', 'version 1.0', as in other S3 format binary files)
 *   Header ended by line containing the single word: endhdr
 *   (int32) Byte-order magic number (0x11223344)
 *   (int32) No. of frames in file (N)
 *   (int32) No. of feature streams (S)
 *   (int32 x S) Width or dimensionality of each feature stream (sum = L)
 *   (float32) Feature vector data (NxL float32 items).
 *   (uint32) Checksum (if present).
 * (Note that this routine does NOT verify the checksum.)
 * Return value: # frames read if successful, -1 if error.
 */
int32 feat_readfile (feat_t *fcb,	/* In: Control block from feat_init() */
		     char *file,	/* In: File to read */
		     int32 sf,		/* In: Start/end frames (range) to be read from
					   file; use 0, 0x7ffffff0 to read entire file */
		     int32 ef,
		     float32 ***feat,	/* Out: Data structure to be filled with read
					   data; allocate using feat_array_alloc() */
		     int32 maxfr);	/* In: #Frames allocated for feat above; error if
					   attempt to read more than this amount. */

/*
 * Counterpart to feat_readfile.  Feature data is assumed to be in a contiguous block
 * starting from feat[0][0][0].  (NOTE: No checksum is written.)
 * Return value: # frames read if successful, -1 if error.
 */
int32 feat_writefile (feat_t *fcb,	/* In: Control block from feat_init() */
		     char *file,	/* In: File to write */
		     float32 ***feat,	/* In: Feature data to be written */
		     int32 nfr);	/* In: #Frames to be written */

/*
 * Read Sphinx-II format mfc file (s2mfc = Sphinx-II format MFC data).
 * Return value: #frames read if successful, -1 if error (e.g., mfc array too small).
 */
int32
feat_s2mfc_read (char *file,		/* In: Sphinx-II format MFC file to be read */
		 int32 sf, int32 ef,	/* In: Start/end frames (range) to be read from file;
					   Can use 0,-1 to read entire file */
		 float32 **mfc,		/* Out: 2-D array to be filled with read data;
					   caller must have allocated this array */
		 int32 maxfr);		/* In: #Frames of mfc array allocated; error if
					   attempt to read more than this amount. */

/*
 * Allocate an array to hold several frames worth of feature vectors.  The returned value
 * is the float32 ***data array, organized as follows:
 *   data[0][0] = frame 0 stream 0 vector, data[0][1] = frame 0 stream 1 vector, ...
 *   data[1][0] = frame 1 stream 0 vector, data[0][1] = frame 1 stream 1 vector, ...
 *   data[2][0] = frame 2 stream 0 vector, data[0][1] = frame 2 stream 1 vector, ...
 *   ...
 * NOTE: For I/O convenience, the entire data area is allocated as one contiguous block.
 * Return value: Pointer to the allocated space if successful, NULL if any error.
 */
float32 ***feat_array_alloc (feat_t *fcb,	/* In: Descriptor from feat_init(), used
						   to obtain #streams and stream sizes */
			     int32 nfr);	/* In: #Frames for which to allocate */

/*
 * Like feat_array_alloc except that only a single frame is allocated.  Hence, one
 * dimension less.
 */
float32 **feat_vector_alloc (feat_t *fcb);  /* In: Descriptor from feat_init(),
					       used to obtain #streams and 
					       stream sizes */

/*
 * Initialize feature module to use the selected type of feature stream.  
 * One-time only * initialization at the beginning of the program.  Input type 
 * is a string defining the  kind of input->feature conversion desired:
 *   "s2_4x":   s2mfc->Sphinx-II 4-feature stream,
 *   "s3_1x39": s2mfc->Sphinx-3 single feature stream,
 *   "n1,n2,n3,...": Explicit feature vector layout spec. with comma-separated 
 *   feature stream lengths.  In this case, the input data is already in the 
 *   feature format and there is no conversion necessary.
 * Return value: (feat_t *) descriptor if successful, NULL if error.  Caller 
 * must not directly modify the contents of the returned value.
 */
feat_t *feat_init (char *type,	/* In: Type of feature stream */
		   char *cmn,	/* In: Type of cepstram mean normalization to 
			           be done before feature computation; can be 
		                   NULL (for none) */
		   char *varnorm,  /* In: ("yes" or "no") Whether variance 
				   normalization done on each utt; only 
				   applicable if CMN also done */
		   char *agc);	/* In: Type of automatic gain control to be 
				   done before feature computation; can be 
				   NULL (for none) */

/*
 * Print the given block of feature vectors to the given FILE.
 */
void feat_print (feat_t *fcb,		/* In: Descriptor from feat_init() */
		 float32 ***feat,	/* In: Feature data to be printed */
		 int32 nfr,		/* In: #Frames of feature data above */
		 FILE *fp);		/* In: Output file pointer */

/*
 * Read a specified MFC file (or given segment within it), perform CMN/AGC as 
 * indicated by * fcb, and compute feature vectors.  Feature vectors are 
 * computed for the entire segment specified, by including additional 
 * surrounding or padding frames to accommodate the feature windows.
 * Return value: #Frames of feature vectors computed if successful; -1 if 
 * any error.
 */
int32 feat_s2mfc2feat (feat_t *fcb,	/* In: Descriptor from feat_init() */
		       char *file,	/* In: File to be read */
		       char *dir,	/* In: Directory prefix for file, 
					   if needed; can be NULL */
		       int32 sf, int32 ef,   /* Start/End frames within 
					   file to be read. Use 0,-1 to process 
					   entire file */
		       float32 ***feat,	/* Out: Computed feature vectors; 
					   caller must allocate this space */
		       int32 maxfr);	/* In: Available space (#frames) in 
					   above feat array; it must be 
					   sufficient to hold the result */


/* Feature computation routine for live mode decoder. Computes features
 * for blocks of incoming data. Retains an internal buffer for computing
 * deltas etc */

int32   feat_s2mfc2feat_block(feat_t  *fcb,    /* Descriptor from feat_init() */
			      float32 **uttcep, /* Incoming cepstral buffer */
			      int32   nfr,      /* Size of incoming buffer */
                              int32 beginutt,   /* Begining of utterance flag */
                              int32 endutt,     /* End of utterance flag */
                              float32 ***ofeat  /* Output feature buffer */
			     );


/* Feature computation routine for live mode decoder. Computes features
 * for blocks of incoming data. Retains an internal buffer for computing
 * deltas etc */

/*
 * RAH, remove memory allocated by feat_init
 */
void feat_free (feat_t *f);

#ifdef __cplusplus
}
#endif


#endif
