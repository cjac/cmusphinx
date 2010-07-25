/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.	All rights
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
 /*************************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 2000 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 *************************************************
 *
 *  Aug 6, 2004
 *
 * HISTORY
 *
 * $Log$
 * Revision 1.13  2006/03/18  21:49:54  dhdfu
 * Port this to Unix (sorry about the kind of ugly thread-abstraction macros)
 * 
 * Revision 1.12  2005/10/05 15:42:52  dhdfu
 * Accidentally checked-in a different version, reverting...
 *
 * Revision 1.10  2005/06/22 05:39:56  arthchan2003
 * Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 *
 * Revision 1.2  2005/03/30 00:43:41  archan
 * Add $Log$
 * Revision 1.13  2006/03/18  21:49:54  dhdfu
 * Port this to Unix (sorry about the kind of ugly thread-abstraction macros)
 * 
 * Add Revision 1.12  2005/10/05 15:42:52  dhdfu
 * Add Accidentally checked-in a different version, reverting...
 * Add
 * Add Revision 1.10  2005/06/22 05:39:56  arthchan2003
 * Add Synchronize argument with decode. Removed silwid, startwid and finishwid.  Wrapped up logs3_init, Wrapped up lmset. Refactor with functions in dag.
 * Add into most of the .[ch] files. It is easy to keep track changes.
 *
 */

/** \file main_livedecode.c
 * \brief live-mode decoder demo. 
 *
 *  Created by Yitao Sun (yitao@cs.cmu.edu).  This is a test program written
 *  for the Win32 platform.  The program initializes Sphinx3 live-decode API,
 *  then in a press-to-start and press-to-stop fashion, records and decodes a 
 *  session of user speech.  The threading and synchronization code are Win32-
 *  specific.  Ravi Mosur (rkm@cs.cmu.edu) suggested using select() (and no
 *  threads) on the /dev/tty* device to remove Win32 dependency.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <sphinxbase/ad.h>

#include <s3_decode.h>

#define BUFSIZE 4096
#define TIMEOUT 100

#ifdef WIN32
#include <windows.h>
#define THREAD_START DWORD WINAPI
#define COND_TIMEDOUT WAIT_TIMEOUT
typedef HANDLE condition_t;
typedef HANDLE mythread_t;
#define cond_wait(c) WaitForSingleObject(c, INFINITE)
#define cond_wait_timed(cc,ticks)  WaitForSingleObject(*(cc),ticks)
#define cond_signal(c) SetEvent(c)
#define create_cond(cc) (*(cc) = CreateEvent(NULL, TRUE, FALSE, NULL))
#define create_thread(tt, proc) (*(tt) = CreateThread(NULL, 0, proc, NULL, 0, NULL))
#define join_thread(t) WaitForSingleObject(t, INFINITE)

#else                           /* !WIN32 */

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#define THREAD_START void *
#define COND_TIMEDOUT ETIMEDOUT
typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    int fired;
} condition_t;
typedef pthread_t mythread_t;
#define cond_wait(c) {				\
	pthread_mutex_lock(&(c).mtx);		\
	pthread_cond_wait(&(c).cond, &(c).mtx);	\
	pthread_mutex_unlock(&(c).mtx);		\
}
int
cond_wait_timed(condition_t * c, int ticks)
{
    struct timeval now;
    struct timespec timeout;
    int rv;

    pthread_mutex_lock(&c->mtx);
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + ((ticks) / 1000);
    timeout.tv_nsec = now.tv_usec * 1000 + ((ticks) % 1000) * 1000000;
    if (timeout.tv_nsec > 1000000000) {
        ++timeout.tv_sec;
        timeout.tv_nsec -= 1000000000;
    }
    rv = pthread_cond_timedwait(&c->cond, &c->mtx, &timeout);
    pthread_mutex_unlock(&c->mtx);
    if (c->fired)
        return 0;
    return rv;
}

#define cond_signal(c) {			\
	pthread_mutex_lock(&(c).mtx);		\
	(c).fired = 1;				\
	pthread_cond_signal(&(c).cond);		\
	pthread_mutex_unlock(&(c).mtx);		\
}
#define create_cond(cc) {			\
	pthread_cond_init(&(cc)->cond, NULL);	\
	pthread_mutex_init(&(cc)->mtx, NULL);	\
	(cc)->fired = 0;			\
}
#define create_thread(tt, proc) pthread_create(tt, NULL, proc, NULL)
#define join_thread(t) pthread_join(t, NULL)
#endif                          /* !WIN32 */

condition_t startEvent;
condition_t finishEvent;
fe_t *fe;
s3_decode_t decoder;

FILE *dump = 0;

THREAD_START
process_thread(void *aParam)
{
    ad_rec_t *in_ad = 0;
    int16 samples[BUFSIZE];
    uint32 num_samples;
    float32 **frames;
    int32 num_frames;

    cond_wait(startEvent);

    if ((in_ad = ad_open_sps((int) cmd_ln_float32_r(decoder.kbcore->config, "-samprate"))) == NULL) {
        printf("Failed to open audio input device\n");
        exit(1);
    }
    ad_start_rec(in_ad);

    while (cond_wait_timed(&finishEvent, TIMEOUT) == COND_TIMEDOUT) {
        num_samples = ad_read(in_ad, samples, BUFSIZE);
        if (num_samples > 0) {
	    /** dump the recorded audio to disk */
            if (fwrite(samples, sizeof(int16), num_samples, dump) <
		num_samples) {
                printf("Error writing audio to dump file.\n");
            }

	    fe_process_utt(fe, samples, num_samples, &frames, &num_frames);
	    if (frames != NULL) {
	        char *hypstr;
		s3_decode_process(&decoder, frames, num_frames);
		ckd_free_2d((void **)frames);
		if (s3_decode_hypothesis(&decoder, NULL, &hypstr, NULL)) {
		    printf("Cannot retrieve hypothesis.\n");
		}
		else {
		    printf("Partial hypothesis:\n%s\n", hypstr);
		}
	    }
        }
    }

    ad_stop_rec(in_ad);
    ad_close(in_ad);

    s3_decode_end_utt(&decoder);

    return 0;
}

int
main(int argc, char **argv)
{
    mythread_t thread;
    char buffer[1024];
    char *hypstr;
    cmd_ln_t *config = NULL;

    /*
     * Initializing
     */
    if (argc != 2) {
        printf("Usage:  livedecode config_file \n");
        return -1;
    }

    if ((config = cmd_ln_parse_file_r(config, S3_DECODE_ARG_DEFS, argv[1], TRUE)) == NULL) {
        printf("Bad arguments file (%s).\n", argv[1]);
        return -1;
    }

    if (s3_decode_init(&decoder, config)) {
        printf("Initialization failed.\n");
        return -1;
    }

    fe = fe_init_auto_r(config); 

    if (s3_decode_begin_utt(&decoder, 0)) {
        printf("Cannot start decoding\n");
        return -1;
    }

  /** initializing a file to dump the recorded audio */
    if ((dump = fopen("out.raw", "wb")) == 0) {
        printf("Cannot open dump file out.raw\n");
        return -1;
    }

    create_cond(&startEvent);
    create_cond(&finishEvent);
    create_thread(&thread, &process_thread);

    /*
     * Wait for some user input, then signal the processing thread to start
     * recording/decoding
     */
    printf("press ENTER to start recording\n");
    fgets(buffer, 1024, stdin);
    cond_signal(startEvent);

    /*
     *  Wait for some user input again, then signal the processing thread to end
     *  recording/decoding
     */
    printf("press ENTER to finish recording\n");
    fgets(buffer, 1024, stdin);
    cond_signal(finishEvent);

    /*
     *  Wait for the working thread to join
     */
    join_thread(thread);

    /*
     *  Print the decoding output
     */
    if (s3_decode_hypothesis(&decoder, NULL, &hypstr, NULL)) {
        printf("Cannot retrieve hypothesis.\n");
    }
    else {
        printf("Hypothesis:\n%s\n", hypstr);
    }

    s3_decode_close(&decoder);
    fe_free(fe);

    fclose(dump);

    return 0;
}
