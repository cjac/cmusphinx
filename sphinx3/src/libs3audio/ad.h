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
 * ad.h -- generic live audio interface for recording and playback
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
 * Revision 1.2  2001/12/11  04:40:55  lenzo
 * License cleanup.
 * 
 * Revision 1.1.1.1  2001/12/03 16:01:45  egouvea
 * Initial import of sphinx3
 *
 * Revision 1.1.1.1  2001/01/17 05:17:14  ricky
 * Initial Import of the s3.3 decoder, has working decodeaudiofile, s3.3_live
 *
 * 
 * 19-Jan-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added AD_ return codes.  Added ad_open_sps_bufsize(), and
 * 		ad_rec_t.n_buf.
 * 
 * 17-Apr-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ad_open_play_sps().
 * 
 * 07-Mar-98	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ad_open_sps().
 * 
 * 10-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ad_wbuf_t, ad_rec_t, and ad_play_t types, and augmented all
 * 		recording functions with ad_rec_t, and playback functions with
 * 		ad_play_t.
 * 
 * 06-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 *		Created.
 */


#ifndef _AD_H_
#define _AD_H_

/* 1.2.01 RAH #include <s2types.h> */
/* 1.2.01 RAH added */
/* awb deleted *temporarily* */
/* #include "prim_type.h" */

#if (WIN32)

#include <windows.h>
#include <mmsystem.h>

#elif (__alpha && 0)

#include <AF/AFlib.h>

#elif (_HPUX_SOURCE)

#include <audio/Alib.h>

#endif


#define AD_SAMPLE_SIZE		(sizeof(int16))
#define DEFAULT_SAMPLES_PER_SEC	16000

/* Return codes */
#define AD_OK		0
#define AD_EOF		-1
#define AD_ERR_GEN	-1
#define AD_ERR_NOT_OPEN	-2
#define AD_ERR_WAVE	-3


#if (WIN32)
typedef struct {
    HGLOBAL h_whdr;
    LPWAVEHDR p_whdr;
    HGLOBAL h_buf;
    LPSTR p_buf;
} ad_wbuf_t;
#endif


/* ------------ RECORDING -------------- */

/*
 * NOTE: ad_rec_t and ad_play_t are READ-ONLY structures for the user.
 */

#if (WIN32)

typedef struct {
    HWAVEIN h_wavein;	/* "HANDLE" to the audio input device */
    ad_wbuf_t *wi_buf;	/* Recording buffers provided to system */
    int32 n_buf;	/* #Recording buffers provided to system */
    int32 opened;	/* Flag; A/D opened for recording */
    int32 recording;
    int32 curbuf;	/* Current buffer with data for application */
    int32 curoff;	/* Start of data for application in curbuf */
    int32 curlen;	/* #samples of data from curoff in curbuf */
    int32 lastbuf;	/* Last buffer containing data after recording stopped */
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_rec_t;

#elif (__alpha && 0)

typedef struct {
    AFAudioConn *aud;
    AC ac;
    int32 recording;		/* flag; TRUE iff currently recording */
    ATime last_rec_time;	/* timestamp of last sample recorded in buffer */
    ATime end_rec_time;		/* time at which recording stopped */
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_rec_t;

#elif (sun4)

typedef struct {
    int32 audio_fd;
    int32 recording;
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_rec_t;

#elif (linux)

/* Added by jd5q+@andrew.cmu.edu, 10/3/1997: */
typedef struct {
    int32 dspFD;	/* Audio device descriptor */
    int32 recording;
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_rec_t;

#elif (_HPUX_SOURCE)

typedef struct {
    Audio *audio;	/* The main audio handle */
    ATransID xid;	/* The current transaction ID */
    int32 streamSocket;	/* Connection socket */
    int32 recording;	/* TRUE iff currently recording */
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_rec_t;

#else

typedef struct {
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_rec_t;	/* Dummy definition for systems without A/D stuff */


#endif


/*
 * Open audio device for recording.  Opened in non-blocking mode and placed in idle
 * state.  Return value: pointer to read-only ad_rec_t structure if successful, NULL
 * otherwise.  The return value to be used as the first argument to other recording
 * functions.
 */
ad_rec_t *ad_open_sps (int32 samples_per_sec);


/* Like ad_open_sps but with default samples/sec and bufsize */
ad_rec_t *ad_open ( void );


#if (WIN32)
/*
 * Like ad_open_sps but specifies buffering required within driver.  This function is
 * useful if the default (5000 msec worth) is too small and results in loss of data.
 */
ad_rec_t *ad_open_sps_bufsize (int32 samples_per_sec, int32 bufsize_msec);
#endif


/* Start audio recording.  Return value: 0 if successful, <0 otherwise */
int32 ad_start_rec (ad_rec_t *);


/* Stop audio recording.  Return value: 0 if successful, <0 otherwise */
int32 ad_stop_rec (ad_rec_t *);


/* Close the recording device.  Return value: 0 if successful, <0 otherwise */
int32 ad_close (ad_rec_t *);


/*
 * Read next block of audio samples while recording; read upto max samples into buf.
 * Return value: # samples actually read (could be 0 since non-blocking); -1 if not
 * recording and no more samples remaining to be read from most recent recording.
 */
int32 ad_read (ad_rec_t *, int16 *buf, int32 max);


/* ------ PLAYBACK; SIMILAR TO RECORDING ------- */

#if (WIN32)

typedef struct {
    HWAVEOUT h_waveout;	/* "HANDLE" to the audio output device */
    ad_wbuf_t *wo_buf;	/* Playback buffers given to the system */
    int32 opened;	/* Flag; A/D opened for playback */
    int32 playing;
    char *busy;		/* flags [N_WO_BUF] indicating whether given to system */
    int32 nxtbuf;	/* Next buffer [0..N_WO_BUF-1] to be used for playback data */
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_play_t;

#else

typedef struct {
    int32 sps;		/* Samples/sec */
    int32 bps;		/* Bytes/sample */
} ad_play_t;	/* Dummy definition for systems without A/D stuff */

#endif


ad_play_t *ad_open_play_sps (int32 samples_per_sec);

ad_play_t *ad_open_play ( void );

int32 ad_start_play (ad_play_t *);

int32 ad_stop_play (ad_play_t *);

int32 ad_close_play (ad_play_t *);


/*
 * Write the next block of [len] samples from rawbuf to the A/D device for playback.
 * The device may queue less than len samples, possibly 0, since it is non-blocking.
 * The application should resubmit the remaining data to be played.
 * Return value: # samples accepted for playback; -1 if error.
 */
int32 ad_write (ad_play_t *, int16 *buf, int32 len);


/* ------ MISCELLANEOUS ------- */

/*
 * Convert mu-law data to int16 linear PCM format.
 */
void ad_mu2li (int16 *outbuf,		/* Out: PCM data placed here (allocated by user) */
	       unsigned char *inbuf,	/* In: Input buffer with mulaw data */
	       int32 n_samp);		/* In: #Samples in inbuf */

#endif
