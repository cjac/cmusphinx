/*
 * cont_ad.h -- Continuous A/D listening and silence filtering module.
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
 * 13-Jul-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added spf and adbufsize to cont_ad_t in order to support variable
 * 		frame sizes depending on audio sampling rate.
 * 
 * 30-Jun-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added FILE* argument to cont_ad_powhist_dump().
 * 
 * 16-Jan-98	Paul Placeway (pwp@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed to use dB instead of the weird power measure.
 * 		Added most system parameters to cont_ad_t instead of hardwiring
 * 		them in cont_ad.c.
 * 		Added cont_ad_set_params() and cont_ad_get_params().
 * 
 * 28-Jul-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added cont_ad_t.siglvl.
 * 
 * 27-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added the option for cont_ad_read to return -1 on EOF.
 * 
 * 21-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added cont_ad_set_thresh().
 * 
 * 20-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Separated thresholds for speech and silence.
 * 
 * 17-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created, based loosely on Steve Reed's original implementation.
 */


#ifndef _CONT_AD_H_
#define _CONT_AD_H_


/*
 * This module is intended to be interposed as a filter between any raw A/D source and the
 * application to remove silence regions.  Its main purpose is to remove regions of
 * silence from the raw input speech.  It is initialized with a raw A/D source function
 * (during the cont_ad_init call).  The application is responsible for setting up the A/D
 * source, turning recording on and off as it desires.  Filtered A/D data can be read by
 * the application using the cont_ad_read function.
 * 
 * In other words, the application calls cont_ad_read instead of the raw A/D source function
 * (e.g., ad_read in libad) to obtain filtered A/D data with silence regions removed.  This
 * module itself does not enforce any other structural changes to the application.
 * 
 * The cont_ad_read function also updates an "absolute" timestamp (see cont_ad_t.read_ts) at
 * the end of each invocation.  The timestamp indicates the total #samples of A/D data read
 * until this point, including data discarded as silence frames.  The application is
 * responsible for using this timestamp to make any policy decisions regarding utterance
 * boundaries or whatever.
 */


#include <stdio.h>

/*
 * Data structure for maintaining speech (non-silence) segments not yet consumed by the
 * application.  FOR THE INTERNAL USE OF THIS MODULE.
 */
typedef struct spseg_s {
    int32 startfrm;	/* Frame-id in adbuf (see below) of start of this segment */ 
    int32 nfrm;		/* #frames in segment (may wrap around adbuf) */
    struct spseg_s *next;	/* Next speech segment (with some intervening silence) */
} spseg_t;


/*
 * Continuous listening module or object.  An application can open and maintain several
 * such objects, if necessary.
 * FYI: Module always in one of two states: SILENCE or SPEECH.  Transitions between the
 * two detected by sliding a window spanning several frames and looking for some minimum
 * number of frames of the other type.
 */
typedef struct {
    /* Function to be called for obtaining A/D data (see prototype for ad_read in ad.h) */
    int32 (*adfunc)(ad_rec_t *ad, int16 *buf, int32 max);
    ad_rec_t *ad;	/* A/D device argument for adfunc.  Also, ad->sps used to
			   determine frame size (spf, see below) */
    int16 *adbuf;	/* Circular buffer for maintaining A/D data read until consumed */
    int32 read_ts;	/* Timestamp (total no. of raw A/D samples read, silence+speech)
			   at the end of the most recent cont_ad_read call */
    int32 siglvl;	/* Max signal level for most recently read data (0-16; #bits) */

    int32 sps;		/* Samples/sec; moved from ad->sps to break dependence on
			   ad by N. Roy.*/

    int32 spf;		/* Samples/frame; audio level is analyzed within frames */
    int32 adbufsize;	/* Buffer size (#samples) */
    int32 prev_sample;	/* For pre-emphasis filter */
    int32 headfrm;	/* Frame # in adbuf with unconsumed A/D data */
    int32 n_frm;	/* #Complete frames of unconsumed A/D data in adbuf */
    int32 n_sample;	/* #Samples of unconsumed data in adbuf */
    int32 tot_frm;	/* Total #frames of A/D data read, including consumed ones */
    int32 noise_level;	/* PWP: what we claim as the "current" noise level */
    
    int32 *pow_hist;	/* Histogram of frame power, moving window, decayed */
    char *frm_pow;	/* Frame power */

    int32 auto_thresh;  /* Do automatic threshold adjustment or not */
    int32 delta_sil;	/* Max silence power/frame ABOVE noise level */
    int32 delta_speech;	/* Min speech power/frame ABOVE noise level */
    int32 min_noise;	/* noise lower than this we ignore */
    int32 max_noise;	/* noise higher than this signals an error */
    int32 winsize;	/* how many frames to look at for speech det */
    int32 speech_onset;	/* start speech on >= these many frames out of winsize, of >= delta_speech */
    int32 sil_onset;	/* end speech on >= these many frames out of winsize, of <= delta_sil */
    int32 leader;	/* pad beggining of speech with this many extra frms */
    int32 trailer;	/* pad end of speech with this many extra frms */

    int32 thresh_speech;/* Frame considered to be speech if power >= thresh_speech
			   (for transitioning from SILENCE to SPEECH state) */
    int32 thresh_sil;	/* Frame considered to be silence if power <= thresh_sil
			   (for transitioning from SPEECH to SILENCE state) */
    int32 thresh_update;/* #Frames before next update to pow_hist/thresholds */

    int32 state;	/* Current state, SILENCE or SPEECH */
    int32 win_startfrm;	/* Where next analysis window begins */
    int32 win_validfrm;	/* #Frames currently available from win_startfrm for analysis */
    int32 n_other;	/* If in SILENCE state, #frames in analysis window considered to
			   be speech; otherwise #frames considered to be silence */
    int32 n_in_a_row;	/* number of frames sequentially other side of thresh */
    spseg_t *spseg_head;/* First of unconsumed speech segments */
    spseg_t *spseg_tail;/* Last of unconsumed speech segments */
} cont_ad_t;


/*
 * One time initialization of a continuous listening/silence filtering object/module.
 * Return value: pointer to a READ-ONLY structure used in other calls to the object.
 * If any error occurs, the return value is NULL.
 */
cont_ad_t *cont_ad_init (ad_rec_t *ad,	/* In: The A/D source object to be filtered */
			 int32 (*adfunc)(ad_rec_t *ad, int16 *buf, int32 max));
					/* In: adfunc = source function to be invoked
					   to obtain raw A/D data.  See ad.h for the
					   required prototype definition. */


/*
 * Calibration to determine an initial silence threshold.  This function can be called
 * any number of times.  It should be called at least once immediately after cont_ad_init.
 * The silence threshold is also updated internally once in a while, so this function
 * only needs to be called in the middle if there is a definite change in the recording
 * environment.
 * The application is responsible for making sure that the raw audio source is turned on
 * before the calibration.
 * Return value: 0 if successful, <0 otherwise.
 */
int32 cont_ad_calib (cont_ad_t *cont);	/* In: object pointer returned by cont_ad_init */

/*
 * If the application has not passed an audio device into the silence filter
 * at initialisation,  this routine can be used to calibrate the filter. The
 * buf (of length max samples) should contain audio data for calibration. This
 * data is assumed to be completely consumed. More than one call may be
 * necessary to fully calibrate. 
 * Return value: 0 if successful, <0 on failure, >0 if calibration not
 * complete.
 */
int32 cont_ad_calib_loop (cont_ad_t *r, int16 *buf, int32 max); 

/*
 * Read A/D data pre-filtered to remove silence segments.
 * Return value: #samples actually read, possibly 0; <0 if EOF on A/D source.
 * The function also updates r->read_ts and r->siglvl (see above).
 */
int32 cont_ad_read (cont_ad_t *r,	/* In: Object pointer returned by cont_ad_init */
		    int16 *buf,		/* Out: On return, buf contains A/D data returned
					   by this function, if any.
					   NOTE: buf must be at least 256 samples long */
		    int32 max);		/* In: Max #samples to be filled into buf */


/*
 * Set silence and speech threshold parameters.  The silence threshold is the max power
 * level, RELATIVE to the peak background noise level, in any silence frame.  Similarly,
 * the speech threshold is the min power level, RELATIVE to the peak background noise
 * level, in any speech frame.  In general, silence threshold <= speech threshold.
 * Increasing the thresholds (say, from the default value of 2 to 3 or 4) reduces the
 * sensitivity to background noise, but may also increase the chances of clipping actual
 * speech.
 * Return value: 0 if successful, <0 otherwise.
 */
int32 cont_ad_set_thresh (cont_ad_t *cont,	/* In: Object ptr from cont_ad_init */
			  int32 sil,	/* In: silence threshold (default 2) */
			  int32 sp);	/* In: speech threshold (default 2) */


/*
 * PWP 1/14/98 -- set the changable params.
 *
 *   delta_sil, delta_speech, min_noise, and max_noise are in dB,
 *   winsize, speech_onset, sil_onset, leader and trailer are in frames of
 *   16 ms length (256 samples @ 16kHz sampling).
 */
int32 cont_ad_set_params (cont_ad_t *r, int32 delta_sil, int32 delta_speech,
			  int32 min_noise, int32 max_noise,
			  int32 winsize, int32 speech_onset, int32 sil_onset,
			  int32 leader, int32 trailer);

/*
 * PWP 1/14/98 -- get the changable params.
 *
 *   delta_sil, delta_speech, min_noise, and max_noise are in dB,
 *   winsize, speech_onset, sil_onset, leader and trailer are in frames of
 *   16 ms length (256 samples @ 16kHz sampling).
 */
int32 cont_ad_get_params (cont_ad_t *r, int32 *delta_sil, int32 *delta_speech,
			  int32 *min_noise, int32 *max_noise,
			  int32 *winsize, int32 *speech_onset, int32 *sil_onset,
			  int32 *leader, int32 *trailer);

/*
 * Reset, discarding any accumulated speech segments.
 * Return value: 0 if successful, <0 otherwise.
 */
int32 cont_ad_reset (cont_ad_t *cont);	/* In: Object pointer from cont_ad_init */


/*
 * Close the continuous listening object.
 */
int32 cont_ad_close (cont_ad_t *cont);	/* In: Object pointer from cont_ad_init */


/*
 * Dump the power histogram.  For debugging...
 */
void cont_ad_powhist_dump (FILE *fp, cont_ad_t *cont);


/*
 * Detach the given continuous listening module from the associated audio device.
 * Return 0 if successful, -1 otherwise.
 */
int32 cont_ad_detach (cont_ad_t *c);


/*
 * Attach the continuous listening module to the given audio device/function.
 * (Like cont_ad_init, but without the calibration.)
 * Return 0 if successful, -1 otherwise.
 */
int32 cont_ad_attach (cont_ad_t *c, ad_rec_t *a, int32 (*func)(ad_rec_t *, int16 *, int32));


void cont_ad_set_logfp (FILE *fp);	/* File containing detailed logs (if non-NULL) */

/*
 * Set the silence and speech thresholds. For this to have any effect, the
 * auto_thresh field of the continuous listening module should be set to
 * FALSE.
 */

int32 cont_set_thresh(cont_ad_t *r, int32 silence, int32 speech);


#endif
