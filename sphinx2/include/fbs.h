/*
 * fbs.h -- Interface exported by the decoder module
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
 * $Log$
 * Revision 1.4  2001/11/20  21:22:31  lenzo
 * Win32 re-compatibility fixes.
 * 
 * Revision 1.3  2000/12/05 01:45:11  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.2  2000/02/08 20:44:32  lenzo
 * Changed uttproc_allphone_cepfile() to uttproc_allphone_file.
 *
 * Revision 1.1.1.1  2000/01/28 22:09:07  lenzo
 * Initial import of sphinx2
 *
 *
 * 
 * 05-Jan-99	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added cdcn.h and uttproc_get_cdcn_ptr().
 * 
 * 04-Nov-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added conf field to search_hyp_t.
 * 
 * 30-Oct-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added ascr, lscr fields to search_hyp_t.
 * 
 * 19-Oct-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added uttproc_set_logfile().
 * 
 * 10-Sep-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 *		Added uttproc_allphone_cepfile().
 * 
 * 20-Aug-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added functions uttproc_agcemax_get() and set().
 * 
 * 20-Apr-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added uttproc_set_auto_uttid_prefix().
 * 
 * 24-Mar-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added additional phone_perp field to search_hyp_t for confidence measure
 * 		based on phone perplexity.
 * 
 * 08-Mar-98	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added additional latden field to search_hyp_t for confidence measure
 * 		based on lattice density.
 * 
 * 07-Aug-96	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added uttproc_result_seg and uttproc_partial_result_seg.
 * 		Changed search_hyp_t to support linked list and include word string.
 * 
 * 17-Jun-96	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added uttproc_set_context().
 * 
 * 04-Jun-96	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added BLOCKING option to uttproc_rawdata, uttproc_cepdata, uttproc_result.
 * 		Removed uttproc_set_uttid and added id argument to uttproc_begin_utt.
 * 
 * 24-May-96	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Substantially modified to be driven with externally provided data, rather
 * 			than explicitly reading an A/D source.
 * 		Added uttproc_abort_utt() and uttproc_partial_result().
 * 		Added raw and mfc logging function.
 * 
 * 01-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added functions uttproc_cepmean_set, uttproc_cepmean_get,
 * 		uttproc_agcmax_set, uttproc_agcmax_get.
 * 
 * 07-Aug-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added uttproc_rawdata().
 * 
 * 05-Aug-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added uttproc_beginutt(), uttproc_cepdata(), and uttproc_endutt().
 * 
 * 13-Jun-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Simplified the uttproc interface by combining functions and redefining
 * 		others.
 * 
 * 01-Jun-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added uttproc_set_lm() and uttproc_set_startword().
 * 
 * 01-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _FBS_H_
#define _FBS_H_

#include "s2types.h"

/*
 * The decoder is set up to process one finite-duration utterance at a time.  The
 * maximum duration of an utterance is about 60sec, though other resource limits,
 * such as the back pointer table size, could constrain the duration further.
 */


/*
 * Recognition result (hypothesis) with word segmentation information.
 *
 * FIXME: should this be in search.h?
 */
typedef struct search_hyp_s {
    char const *word;	/* READ-ONLY */
    int32 wid;		/* For internal use of decoder */
    int32 sf, ef;	/* Start, end frames within utterance for this word */
    int32 ascr, lscr;	/* Acoustic, LM scores (not always used!) */
    float conf;		/* Confidence measure (roughly prob(correct)) for this word;
			   NOT FILLED IN BY THE RECOGNIZER at the moment!! */
    struct search_hyp_s *next;	/* Next word segment in the hypothesis; NULL if none */
    int32 latden;	/* Average lattice density in segment.  Larger values imply
			   more confusion and less certainty about the result.  To use
			   it for rejection, cutoffs must be found independently */
    double phone_perp;	/* Average phone perplexity in segment.  Larger values imply
			   more confusion and less certainty.  To use it for rejection,
			   cutoffs must be found independently. */
} search_hyp_t;


/*
 * Called once at initialization with the list of arguments to initialize to initialize
 * the decoder.  If the -ctlfn argument is given, it will process the argument file in
 * batch mode and exit the application.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 fbs_init (int32 argc, char **argv);	/* Arguments for initialization */


/*
 * Called before quitting the application to tie up loose ends in the decoder.
 * Return value: 0 if successful, -1 otherwise.
 */
int32 fbs_end ( void );


/*
 * Called at the beginning of each utterance.  uttid is an input string identifying the
 * utterance; utterance data (raw or mfc files, if any) logged under this name.  The
 * recognition result in the "match" file also identified with this id.  If uttid is
 * NULL, an automatically generated running sequence number (of the form %08d) is used
 * instead.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_begin_utt (char const *uttid);


/*
 * Decode the next block of input samples in the current utterance.  The "block" argument
 * specifies whether the decoder should block until all pending data have been processed.
 * If 0, it is "non-blocking".  That is, the decoder steps through only a few pending
 * frames (at least 1), and the remaining input data is queued up internally for later
 * processing.  In particular, this function can be called with 0-length data to simply
 * process internally queued up frames.
 * 
 * NOTE: The decoder will not actually process the input data if any of the processing
 * depends on the entire utterance.  (For example, if CMN/AGC is based on entire current
 * utterance.)  The data are queued up internally for processing after uttproc_end_utt is
 * called.  Also, one cannot combine uttproc_rawdata and uttproc_cepdata within the same
 * utterance.
 * 
 * Return value: #frames internally queued up and remaining to be decoded; -1 if any
 * error occurs.
 */
int32 uttproc_rawdata (int16 *raw,	/* In: Block of int16 samples */
		       int32 nsample,	/* In: #Samples in above block; can be 0!! */
		       int32 block);	/* In: if !0, process all data before returning */


/*
 * Like uttproc_rawdata, but the input data are cepstrum vectors rather than raw samples.
 * One cannot combine uttproc_cepdata and uttproc_rawdata within the same utterance.
 * Return value: #frames internally queued up and remaining to be decoded; -1 if any
 * error occurs.
 */
int32 uttproc_cepdata (float **cep,	/* In: cep[i] = i-th frame of cepstrum data */
		       int32 nfrm,	/* In: #frames of cep data; can be 0!! */
		       int32 block);	/* In: if !0, process all data before returning */


/*
 * For bookkeeping purposes, marking that no more data is forthcoming in the current
 * utterance.  It should be followed by uttproc_result to obtain the final recognition
 * result.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_end_utt ( void );


/*
 * Obtain recognition result for utterance after uttproc_end_utt has been called.  In
 * the blocking form, all queued up data is processed and final result returned.  In the
 * non-blocking version, only a few pending frames (at least 1) are processed.  In the
 * latter case, the function can be called repeatedly to allow the decoding to complete.
 * 
 * Return value: #frames remaining to be processed.  If non-zero (non-blocking mode) the
 * final result is not yet available.  If 0, frm and hyp contain the final recognition
 * result.  If there is any error, the function returns -1.
 */
int32 uttproc_result (int32 *frm,	/* Out: *frm = #frames in current utterance */
		      char **hyp,	/* Out: *hyp = recognition string; READ-ONLY.
					   Contents clobbered by the next uttproc_result
					   or uttproc_partial_result call */
		      int32 block);	/* In: If !0, process all data and return final
					   result */

/*
 * Like uttproc_result, but returns a list of word segmentations instead of the full
 * recognition string.  The list of word segmentations is READ-ONLY, and clobbered by
 * the next call to any of the result functions.
 * Use uttproc_result or uttproc_result_seg to obtain the final result, but not both!
 */
int32 uttproc_result_seg (int32 *frm,		/* Out: *frm = #frames in utterance */
			  search_hyp_t **hyp,	/* Out: *hyp = first element in NULL
						   terminated linked list of word
						   segmentations */
			  int32 block);

/*
 * Obtain a partial recognition result in the middle of an utterance.  This function can
 * be called anytime after uttproc_begin_utt and before the final uttproc_result.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_partial_result (int32 *frm,  /* Out: *frm = #frames processed
					      corresponding to the partial result */
			      char **hyp); /* Out: *hyp = partial recognition string,
					      READ-ONLY.  Contents clobbered by the next
					      uttproc_result or uttproc_partial_result
					      call. */

/*
 * Like uttproc_partial_result, but returns a list of word segmentations instead of
 * the partial recognition string.  The list of word segmentations is READ-ONLY, and
 * clobbered by the next call to any of the result functions.
 */
int32 uttproc_partial_result_seg (int32 *frm,
				  search_hyp_t **hyp);	/* Out: *hyp = first element in
							   NULL terminated linked list
							   of word segmentations */

/*
 * Called instead of uttproc_end_utt to abort the current utterance immediately.  No
 * final recognition result is available.  Note that one cannot abort an utterance after
 * uttproc_end_utt.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_abort_utt ( void );


/*
 * The sequence uttproc_stop_utt()...uttproc_restart_utt() can be used to re-recognize
 * the current utterance.  It is typically used to switch to a new language model in the
 * middle of an utterance, for example, based on a partial recognition result; the
 * switch occurs in the middle of the two calls.  uttproc_stop_utt must eventually be
 * followed by uttproc_restart_utt.  There can be no other intervening calls relating to
 * the current utterance; i.e., no uttproc_begin_utt, uttproc_rawdata, uttproc_cepdata,
 * uttproc_end_utt, uttproc_result, uttproc_partial_result, or uttproc_abort_utt.
 * This operation cannot be performed after uttproc_end_utt.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_stop_utt ( void );
int32 uttproc_restart_utt ( void );


/*
 * Perform allphone recognition on the given cepstrum file and return a linked list of
 * phones and segmentation.  The filename should NOT contain the (.mfc) file extension.
 * Return value: pointer to head of linked list of search_hyp_t entries for the phone
 * segments; it may be NULL.  It is a READ-ONLY list.  It will be clobbered by the next
 * call to this function.
 */
search_hyp_t *uttproc_allphone_file (char const *file);	/* Without filename extension */


/*
 * Obtain the uttid for the most recent utterance (in progress or just finished)
 * Return value: pointer to READ-ONLY string that is the utterance id.
 */
char const *uttproc_get_uttid ( void );


/*
 * For automatically generated uttid's (see uttproc_begin_utt), also use the prefix
 * given below.  (So the uttid is formatted "%s%08d", prefix, sequence_no.)
 */
int32 uttproc_set_auto_uttid_prefix (char const *prefix);


/*
 * Set the currently active LM to the given named LM.  Multiple LMs can be loaded initially
 * (during fbs_init) or at run time using lm_read (see below).
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_set_lm (char const *lmname);


/*
 * Indicate to the decoder that the named LM has been updated (e.g., with the addition of
 * a new unigram).
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_lmupdate (char const *lmname);


/*
 * Set trigram history context for the next utterance.  Instead of the next utterance
 * beginning with a clean slate, it is begun as if the two words wd1 and wd2 have just
 * been recognized.  They are used as the (trigram) language model history for the
 * utterance.  wd1 can be NULL if there is only a one word history wd2, or both wd1 and
 * wd2 can be NULL to clear any history information.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_set_context (char const *wd1, /* In: First word of history (possibly NULL) */
			   char const *wd2);/* In: Last (most recent) history (maybe NULL) */


/*
 * Set the current logging directory for per utterance raw sample files and cepstrum
 * files.  The file names are <uttid>.raw and <uttid>.mfc respectively, where <uttid> is
 * the utterance id associated with the current utterance (see uttproc_begin_utt).
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_set_rawlogdir (char const *dir);
int32 uttproc_set_mfclogdir (char const *dir);

/* Logfile can be changed in between utterances.  Return value: 0 if ok, else -1 */
int32 uttproc_set_logfile (char const *file);


/*
 * Set and get the current cepstral means for CMN.
 * Return value: 0 if successful, else -1.
 */
int32 uttproc_cepmean_set (float *cep);	/* Cepstral mean set to cep[0-12] */
int32 uttproc_cepmean_get (float *cep);	/* Current cepstral mean copied into cep[0-12] */

/* Similarly, AGC-Estimated-Max */
int32 uttproc_agcemax_set (float c0max);
double uttproc_agcemax_get ( void );


/*
 * For LISTEN project use only.  (okay, but other stuff uses it anyway)
 */
int32 uttproc_set_startword (char const *startword);


/*
 * Read in a new LM file (lmfile), and associate it with name lmname.  If there is
 * already an LM with the same name, it is automatically deleted.  The current LM is
 * undefined at this point; use uttproc_set_lm(lmname) immediately afterwards.
 * Return value: 0 if successful, else -1.
 */
int32 lm_read (char const *lmfile,	/* In: LM file name */
	       char const *lmname,	/* In: LM name associated with this model */
	       double lw,		/* In: Language weight; typically 6.5-9.5 */
	       double uw,		/* In: Unigram weight; typically 0.5 */
	       double wip);		/* In: Word insertion penalty; typically 0.65 */


/*
 * Delete the named LM from the LM collection.  The current LM is undefined at this
 * point.  Use uttproc_set_lm(...) immediately afterwards.
 * Return value: 0 if successful, else -1.
 */
int32 lm_delete (char const *lmname);

/* Read utterance data from a file (instead of from an audio device) -
   passed to uttproc for batch-mode processing. */
int32 adc_file_read(int16 *buf, int32 max);

/* Of course you have to know how to open that file (which was
   cheerfully omitted from this header file in the past) */
int uttfile_open(char const *utt);

/* Misc. undocumented functions.  FIXME: These don't belong here! */
char const *get_current_startword(void);
char const *get_ref_sent(void);

char const *query_ctlfile_name ( void );
char const *query_match_file_name (void);
char const *query_matchseg_file_name (void);
char const *query_dumplat_dir (void);
char const *query_cdcn_file (void);
int32 query_lattice_size ( void );
int32 query_topsen_window ( void );
int32 query_topsen_thresh ( void );
int32 query_report_altpron ( void );
int32 query_fwdtree_flag ( void );
int32 query_fwdflat_flag ( void );
int32 query_bestpath_flag ( void );
int32 query_sampling_rate ( void );
int32 query_phone_conf ( void );
int32 query_compute_all_senones (void);

int32 uttproc_init(void);
int32 uttproc_end(void);
int32 uttproc_feat2rawfr (int32 fr);
int32 uttproc_raw2featfr (int32 fr);
void uttproc_align(char *sent); /* Really should be const */
int32 uttproc_nosearch(int32 flag);
int32 uttproc_get_featbuf (float **cep, float **dcep,
			   float **dcep_80ms, float **pcep, float **ddcep);
void uttprocSetcomp2rawfr(int32 num, int32 const *ptr);
int32 uttprocGetcomp2rawfr(int16 **ptr);
void time_align_utterance (char const *utt,
			   FILE *out_sent_fp,
			   char const *left_word,
			   int32 begin_frame,
			   char *pe_words, /* FIXME: should be const */
			   int32 end_frame,
			   char const *right_word);

void utt_seghyp_free(search_hyp_t *h);

void run_ctl_file (char const *ctl_file_name);
void run_time_align_ctl_file (char const *utt_ctl_file_name,
			      char const *pe_ctl_file_name,
			      char const *out_sent_file_name);

void agc_set_threshold (float threshold);
int32 cep_read_bin (float32 **buf, int32 *len, char const *file);
int32 cep_write_bin(char const *file, float32 *buf, int32 len);

/*
 * Obtain N-best list for current utterance:
 * NOTE: Should be preceded by search_save_lattice ().
 * NOTE: Clobbers any previously returned N-best hypotheses in *alt_out.
 * Arguments:
 *     sf, ef: Start and end frame range within utterance for generating N-best list.
 *     w1, w2: Two-word context preceding utterance; w2 is the later one.  w1 may be -1
 *             (i.e., non-existent).  w2 must be valid; it can be the word-id for <s>.
 *     On return, alt_out[i] = i-th hypothesis generated.
 * Return value: #alternative hypotheses returned; -1 if error.
 */
int32 search_get_alt (int32 n,			/* In: No. of alternatives to produce */
		      int32 sf, int32 ef,	/* In: Start/End frame */
		      int32 w1, int32 w2,	/* In: context words */
		      search_hyp_t ***alt_out);	/* Out: array of alternatives */

/* Should be called before search_get_alt */
void search_save_lattice ( void );

/* Function used internally to decode each utt in ctlfile */
search_hyp_t *run_sc_utterance (char *mfcfile, int32 sf, int32 ef, char *idspec);

#endif
