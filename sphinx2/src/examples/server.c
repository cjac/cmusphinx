/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * server.c -- SphinxII speech recognition server.
 *
 * HISTORY
 * 
 * $Log$
 * Revision 1.3  2000/12/05  01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 * 
 * Revision 1.2  2000/01/28 23:42:14  awb
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2000/01/28 22:08:41  lenzo
 * Initial import of sphinx2
 *
 *
 *
 * 18-Aug-1999  Kevin Lenzo (lenzo@cs.cmu.edu).  Added sys/timeval.h and
 *              changed unsigned int16 etc to uint16 etc as in s2types.h.
 * 
 * 03-Jun-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Creating from version of 1995.
 */


/*
 * This server uses nonblocking sockets for communicating with clients.  It serves
 * one client at a time.
 * The server uses a local audio source, continuously listening for phrases separated
 * by some minimal amount of silence, decodes such phrases and sends a number of
 * alternative hypotheses for it to the client.  Each hypothesis of an utterance is
 * terminated by a newline character.  The final hypothesis is terminated by the
 * string END_UTT and a newline.  The client should acknowledge each utterance with
 * the string ACK followed by a newline.
 * 
 * Once a client is connected, repeat the following sequence of steps as desired.
 *   1. hit <CR> to start listening
 *   2. speak any number of phrases separated by MIN_ENDSIL samples of silence
 *   3. speak "STOP LISTENING" to stop listening
 * Exit the above sequence by hitting q<CR> instead of just <CR>.  At this point, the
 * server closes the client connection and is ready for the next client.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <signal.h>

#include <sys/time.h>

#ifdef WIN32
#include <posixwin32.h>
#else
#include <unistd.h>
#endif

#include "s2types.h"
#include "basic_types.h"
#include "CM_macros.h"
#include "err.h"
#include "ad.h"
#include "cont_ad.h"
#include "srvcore.h"
#include "search_const.h"
#include "msd.h"
#include "list.h"
#include "hash.h"
#include "lmclass.h"
#include "lm_3g.h"
#include "dict.h"
#include "kb_exports.h"
#include "fbs.h"


#define DEFAULT_LISTEN_PORT	7027
static int32 listenport;	/* Port on which server should listen for clients */
static SOCKET sd;		/* Socket connection to a particular client */

static ad_rec_t *ad;		/* Audio device */
static cont_ad_t *cont;		/* Continuous listening/silence filtering module */
#define MIN_ENDSIL	5000	/* #samples of silence to declare end of utterance */

static int32 startwid;


/* Sleep for specified #msec */
static void sleep_msec (int32 ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;
    
    tmo.tv_sec = 0;
    tmo.tv_usec = ms*1000;
    
    select(0, NULL, NULL, NULL, &tmo);
#endif
}


static void server_error ( void )
{
}


#define RBUFSIZE	1024

static int32 await_ack ( void )
{
    char rbuf[RBUFSIZE];
    int32 i, len, rlen;

    /* Looking for the string ACK\n */
    for (rlen = 0; rlen < 4;) {
	len = server_recv_block (sd, rbuf+rlen, RBUFSIZE-rlen);

	if (len == -1)
	    return -1;
	else if (len < -1) {
	    E_ERROR("Receive error %d\n", len);
	    return -1;
	}
	
	rlen += len;
    }
    rbuf[rlen] = '\0';
    
    if ((rlen > 4) || (strcmp(rbuf, "ACK\n") != 0)) {
	E_ERROR("Expecting ACK\n, received:\n\t");
	for (i = 0; i < rlen; i++)
	    fprintf (stderr, " %02x", rbuf[i]);
	fprintf (stderr, "\n");
	return -1;
    }
    
    return 0;
}


#define END_UTT_MARKER	"END_UTT\n"

static int32 send_result (char *best, search_hyp_t **alt, int32 n_alt)
{
    char buf[32768], *bufp;
    search_hyp_t *h;
    int32 i, len, stat;
    
    /* Send best result first, appending a newline */
    strcpy (buf, best);
    len = strlen(buf);
    buf[len++] = '\n';
    buf[len] = '\0';

    if ((stat = server_send_block (sd, buf, len)) != len) {
	E_ERROR("server_send(%d) returned %d\n", len, stat);
	return -1;
    }
    
    /* Compose strings for and send each of the additional hypotheses */
    for (i = 0; i < n_alt; i++) {
	bufp = buf;
	for (h = alt[i]; h; h = h->next) {
	    if (h->wid != startwid) {		/* Filter out the startword <s> */
		strcpy (bufp, h->word);
		bufp += strlen(h->word);
		*(bufp++) = ' ';
	    }
	    /* Need a check to prevent overflowing buf somewhere!! */
	}
	*(bufp++) = '\n';
	*bufp = '\0';
	
	len = bufp - buf;
	
	if ((stat = server_send_block (sd, buf, len)) != len) {
	    E_ERROR("server_send(%d) returned %d\n", len, stat);
	    return -1;
	}
    }

    /* Send end of utterance marker */
    strcpy (buf, END_UTT_MARKER);
    len = strlen(buf);
    if ((stat = server_send_block (sd, buf, len)) != len) {
	E_ERROR("server_send(%d) returned %d\n", len, stat);
	return -1;
    }
    
    /* Await acknowledgement (ACK\n) from client */
    if (await_ack() < 0)
	return -1;		/* Error occurred; should close connection */

    return 0;
}


#define MAX_ALT		30

/*
 * Main utterance processing loop:
 *     for (;;) {
 * 	   wait for start of next utterance;
 * 	   decode utterance until silence of at least MIN_ENDSIL frames observed;
 * 	   print utterance result;
 *     }
 */
static int32 listen_loop()
{
    int16 adbuf[4096];
    int32 k, fr, ts, rem;
    int32 n_alt;
    char *hyp;
    char word1[1024], word2[1024], word3[1024];
    search_hyp_t **alt;
    
    for (;;) {
	/* Resume A/D recording for next utterance */
	if (ad_start_rec (ad) < 0)
	    E_FATAL("ad_start_rec failed\n");
    
	/* Indicate listening for next utterance */
	printf("\nREADY....\n"); fflush (stdout);
	
	/* Await beginning of next utterance */
	while ((k = cont_ad_read (cont, adbuf, 4096)) == 0)
	    sleep_msec(100);
	
	if (k < 0)
	    E_FATAL("cont_ad_read failed\n");
	
	/*
	 * Non-zero amount of data received; start recognition of new utterance.
	 * NULL argument to uttproc_begin_utt => automatic generation of utterance-id.
	 */
	if (uttproc_begin_utt (NULL) < 0)
	    E_FATAL("uttproc_begin_utt() failed\n");

	uttproc_rawdata (adbuf, k, 0);
	printf ("Listening...\n"); fflush (stdout);

	/* Note timestamp for this first block of data */
	ts = cont->read_ts;

	/* Decode utterance until end (marked by a "long" silence, >1sec) */
	for (;;) {
	    /* Read non-silence audio data, if any, from continuous listening module */
	    if ((k = cont_ad_read (cont, adbuf, 4096)) < 0)
		E_FATAL("cont_ad_read failed\n");
	    if (k == 0) {
		/*
		 * No speech data available; check current timestamp with most recent
		 * speech to see if more than min_endsil.  If so, end of utterance.
		 */
		if ((cont->read_ts - ts) > MIN_ENDSIL)
		    break;
	    } else {
		/* New speech data received; note current timestamp */
		ts = cont->read_ts;
	    }
	    
	    /*
	     * Decode whatever data was read above.  NOTE: Non-blocking mode!!
	     * rem = #frames remaining to be decoded upon return from the function.
	     */
	    rem = uttproc_rawdata (adbuf, k, 0);

	    /* If no work to be done, sleep a bit */
	    if ((rem == 0) && (k == 0))
		sleep_msec (100);
	}
	
	/*
	 * Utterance ended; flush any accumulated, unprocessed A/D data and stop
	 * listening until current utterance completely decoded
	 */
	ad_stop_rec (ad);
	while (ad_read (ad, adbuf, 4096) >= 0);
	cont_ad_reset (cont);

	printf ("Stopped listening, please wait...\n"); fflush (stdout);

	/* Finish decoding, obtain and print result */
	uttproc_end_utt ();
	if (uttproc_result (&fr, &hyp, 1) < 0)
	    E_FATAL("uttproc_result failed\n");
	printf ("%d: %s\n", fr, hyp); fflush (stdout);

	k = sscanf (hyp, "%s %s %s", word1, word2, word3);
	if (k > 0) {
	    search_save_lattice ();
	    n_alt = search_get_alt (MAX_ALT, 0, fr-1, -1, startwid, &alt);
	    
	    if (send_result (hyp, alt, n_alt) < 0)
		return -1;
	}
	
	/* Exit if the utterance was "STOP LISTENING" */
	if ((k == 2) &&
	    ((strcmp (word1, "stop") == 0) || (strcmp (word1, "STOP") == 0)) &&
	    ((strcmp (word2, "listening") == 0) || (strcmp (word2, "LISTENING") == 0)))
	    return 0;
    }

    return 0;
}


/*
 * Process current client until told to quit.
 */
static int32 process_client ( void )
{
    char line[1024];
    
    for (;;) {
	fprintf (stderr, "Hit <CR> to start listening, q<CR> to quit client connection");
	fflush (stderr);
	
	fgets (line, sizeof(line), stdin);
	if ((line[0] == 'q') || (line[0] == 'Q'))
	    break;
	
	if (listen_loop () < 0)
	    return -1;
    }

    return 0;
}


static void sigint_handler (int arg)
{
    E_INFO("^C...Exiting\n");
    
    server_end ();
    fbs_end ();
    
    exit (0);
}


/*
 * Main server loop.  Await client connections and process them.
 */
static void s2srv_loop ( void )
{
#ifndef WIN32
    signal (SIGPIPE, SIG_IGN);	/* How about WIN32? */
#endif
    signal (SIGINT, sigint_handler);
    
    for (;;) {
	/* Wait for connection from new client */
        E_INFO("Awaiting client connection\n");
	if ((sd = server_await_conn ()) == INVALID_SOCKET) {
	    server_end ();
	    E_INFO("No connection; retrying after a few seconds\n");
#if WIN32
	    Sleep (3000);
#else
	    sleep (30);
#endif
	    if (server_initialize (listenport) < 0)
		E_FATAL("server_initialize() failed\n");
	    
	    continue;
	}
	
	/* Client connected; process it */
        E_INFO("Client connected\n");
	if (process_client () < 0)
	    server_error ();

	/* Close client connection */
        E_INFO("Closing client connection\n");
	server_close_conn (sd);
    }

    /* Never really get here...!! */
    server_end ();
}


int32 s2srv_init (char *portarg)
{
    if ((! portarg) || (sscanf (portarg, "%d", &listenport) != 1)) {
	fflush (stdout);
        listenport = DEFAULT_LISTEN_PORT;
	fprintf (stderr, "%s(%d): Bad or missing port# argument, using %d\n",
		 __FILE__, __LINE__, DEFAULT_LISTEN_PORT);
	fflush (stderr);
    }
    
    if (server_initialize (listenport) < 0)
	E_FATAL("server_initialize() failed\n");
    
    return 0;
}

int
main (int32 argc, char *argv[])
{
    int32 i;
    char *port;
    
    /* Default initial values */
    listenport = 0;
    port = NULL;
    
    /* Parse command line arguments */
    for (i = 1; i < argc-1; i++) {
	if (strcmp (argv[i], "-port") == 0) {
	    port = argv[i+1];
	    i++;
	}
    }

    /* Open audio device and calibrate for background noise level */
    if ((ad = ad_open ()) == NULL)
	E_FATAL("ad_open() failed\n");
    if ((cont = cont_ad_init (ad, ad_read)) == NULL)
	E_FATAL("cont_ad_init failed\n");
    fprintf (stderr, "Calibrating background noise level...");
    fflush (stderr);
    ad_start_rec (ad);
    cont_ad_calib (cont);
    ad_stop_rec (ad);
    fprintf (stderr, "done\n");
    fflush (stderr);
    
    /* Initialize recognition engine */
    fbs_init (argc, argv);

    startwid = kb_get_word_id ("<s>");
    
    /* Initialize server and process clients */
    s2srv_init (port);
    s2srv_loop ();

    /* Close recognition engine */
    fbs_end ();
    return 0;
}
