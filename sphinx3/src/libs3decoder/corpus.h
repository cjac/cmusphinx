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
 * corpus.h -- Corpus-file related misc functions.
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
 * 09-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added ctl_process_utt ().
 * 
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Updated ctl_infile() spec to included check for already existing file extension.
 * 
 * 23-Mar-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added a general purpose data argument to ctl_process() and its function
 * 		argument func.
 * 
 * 22-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added an optional validation function argument and an optional
 *		duplicate-resolution function argument to both corpus_load_headid() and
 * 		corpus_load_tailid().
 * 
 * 25-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Created.
 */


#ifndef _S3_CORPUS_H_
#define _S3_CORPUS_H_


#include <libutil/libutil.h>


/*
 * Structure for a corpus: essentially a set of strings each associated with a
 * unique ID.  (Such as a reference sentence file, hypothesis file, and various
 * control files.)
 * NOTE: IDs are CASE-SENSITIVE.
 */
typedef struct {
    hash_table_t *ht;	/* Hash table for IDs; CASE-SENSITIVE */
    int32 n;		/* #IDs (and corresponding argument strings) in the corpus */
    char **str;		/* The argument strings */
} corpus_t;


/*
 * Load a corpus from the given file and return it.
 * Each line is a separate entry in the corpus.  Blank lines are skipped.
 * The ID is the FIRST word in a line.
 * 
 * Validation:
 * validate is an optional, application-supplied function to determine if each input
 * corpus data entry is eligible (valid) for inclusion in the final corpus.  It should
 * return an integer value signifying the following actions:
 *      0: Not valid, skip the entry;
 *     !0: Valid, include the entry.
 * If validate is NULL, every input entry is included in the corpus.
 * 
 * Duplicate resolution:
 * dup_resolve is an optional, application-supplied function to resolve duplicate keys
 * (IDs).  It may be NULL if none is available.  If present, and a duplicate key is
 * encountered, the function is invoked with the original and the duplicate corpus
 * strings as arguments (s1 and s2, respectively).  It should return an integer value
 * signifying the following actions:
 *      0: Retain the original string, discard the new one;
 *     >0: Replace the original string with the new one;
 *     <0: Error (causes a FATAL_ERROR).
 * If dup_resolve is NULL, any duplicate ID causes a FATAL_ERROR.
 * 
 * Return value: Ptr to corpus if successful.
 */
corpus_t *corpus_load_headid (char *file,	/* Must be seekable and rewindable */
			      int32 (*validate)(char *str),
			      int32 (*dup_resolve)(char *s1, char *s2));

/*
 * Similar to corpus_load_headid, but the ID is at the END of each line, in parentheses.
 */
corpus_t *corpus_load_tailid (char *file,	/* Must be seekable and rewindable */
			      int32 (*validate)(char *str),
			      int32 (*dup_resolve)(char *s1, char *s2));

/*
 * Lookup the given corpus for the given ID and return the associated string.
 * Return NULL if ID not found.
 */
char *corpus_lookup (corpus_t *corp, char *id);

/*
 * Read another entry from a S3 format "control file" and parse its various fields.
 * Blank lines and lines beginning with a hash-character (#) are omitted.
 * Control file entry format:
 *     uttfile(usually cepstrum file) [startframe endframe [uttid]]
 * Any error in control file entry format is FATAL.
 * Return value: 0 if successful, -1 if no more entries left.
 */
int32 ctl_read_entry (FILE *fp,
		      char *uttfile,	/* Out: (Cep)file containing utterance data */
		      int32 *sf,	/* Out: Start frame in uttfile; 0 if omitted */
		      int32 *ef,	/* Out: End frame in uttfile; -1 (signifying
					   until EOF) if omitted */
		      char *uttid);	/* Out: Utterance ID (generated from uttfile/sf/ef
					   if omitted) */

/*
 * Process the given control file (or stdin if NULL):  Skip the first nskip entries, and
 * process the next count entries by calling the given function (*func) for each entry.
 * Any error in reading the control file is FATAL.
 * Return value: ptmr_t structure containing cpu/elapsed time stats for the run.
 */
ptmr_t ctl_process (char *ctlfile,	/* In: Control file to read; use stdin if NULL */
		    int32 nskip,	/* In: No. of entries to skip at the head */
		    int32 count,	/* In: No. of entries to process after nskip */
		    void (*func) (void *kb, char *uttfile, int32 sf, int32 ef, char *uttid),
		    			/* In: Function to be invoked for each of the
					   count entries processed. */
		    void *kb);		/* In: A catch-all data pointer to be passed as
					   the first argument to func above */

/*
 * A small modification of ctl_process.  It changes the LM dynamically according to the utterances. User can use option -ctl_lm to specify which LM should be used in each utterance.  
 */
ptmr_t ctl_process_dyn_lm (char *ctlfile,	/* In: Control file to read; use stdin if NULL */
			   char *ctllmfile,     /* In: Control file that specify the lm used for the corresponding utterance */
		    int32 nskip,	/* In: No. of entries to skip at the head */
		    int32 count,	/* In: No. of entries to process after nskip */
		    void (*func) (void *kb, char *uttfile, int32 sf, int32 ef, char *uttid),
		    			/* In: Function to be invoked for each of the
					   count entries processed. */
		    void *kb);		/* In: A catch-all data pointer to be passed as
					   the first argument to func above */


/*
 * Like ctl_process, but process the single filename given (uttfile), count times.  After each
 * processing, wait for the time of modification on the given file to change.  In this mode,
 * the decoder can be used to process a dynamically generated sequence of utterances.  To avoid
 * race conditions, each new instance of the file should be created "in an instant": by creating
 * it under a temporary name and finally renaming it to the given filename atomically.
 * Return value: ptmr_t structure containing cpu/elapsed time stats for the run.
 */
ptmr_t ctl_process_utt (char *uttfile,	/* In: Filename to be process (in its entirety) */
			int32 count,	/* In: No. of iterations to process uttfile */
			void (*func) (void *kb, char *uttfile, int32 sf, int32 ef, char *uttid),
			void *kb);

/*
 * Build a complete input filename from the given uttname, directory and file-extension:
 *   If utt begins with a / ignore dir, otherwise prefix dir/ to utt;
 *   If a non-empty file extension is provided, and utt doesn't already have that extension,
 * 	append .ext to filename.
 */
void ctl_infile (char *file,	/* Out: Generated filename (allocated by caller) */
		 char *dir,	/* In: Optional directory spec if relative utt specified */
		 char *ext,	/* In: File extension to be appended to utt to generate
				   complete filename */
		 char *utt);	/* In: Utterance file pathname, absolute or relative,
				   with or without file extension.  This is usually the
				   first field in a control file */

/*
 * Build a complete output filename from the given components as follows:
 *     if dir ends with ,CTL and utt does not begin with /, use dir/utt
 *     if dir ends with ,CTL and utt DOES begin with /, filename is utt
 *     if dir does not end with ,CTL, filename is dir/uttid.
 * If a non-empty ext specified append .ext to generated filename.
 */
void ctl_outfile (char *file,	/* Out: Generated filename (allocated by caller) */
		  char *dir,	/* In: Directory for the generated filename; see comment
				   for special handling of ,CTL suffix */
		  char *ext,	/* In: File-extension applied to the generated filename */
		  char *utt,	/* In: Utterance file pathname, absolute or relative,
				   with or without extension.  This is usually the first
				   field in a control file. */
		  char *uttid);	/* In: Utterance ID (derived from the control file */

#endif
