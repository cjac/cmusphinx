/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * clicore.c -- Basic socket operations for network client.
 * 
 * HISTORY
 * 
 * 26-Oct-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Fixed bug in cli_open that caused client to fail to connect forever
 * 		if the first attempt at connection failed.
 * 
 * 25-Apr-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 * $Log$
 * Revision 1.2  2000/12/05  01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 * 
 * Revision 1.1.1.1  2000/01/28 22:08:41  lenzo
 * Initial import of sphinx2
 *
 *
 */


/*
 * This file tries to hide some of system-specific socket implementation details.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "clicore.h"


#if (! NOERRLOG)
#define ERRLOG(x)	{fprintf x;}
#else
#define ERRLOG(x)	{}
#endif


static SOCKET conn_sd = INVALID_SOCKET;


#if (WIN32)

/*
 * Isn't there a built in perror for socket errors?
 */
static void print_errno (char *hdr)
{
    int32 ec;
    
    printf ("%s: ", __FILE__);

    ec = SOCKET_ERRNO;
    switch (ec) {
    case WSAENETDOWN:		printf ("%s: WSAENETDOWN\n", hdr);
	break;
    case WSANOTINITIALISED:	printf ("%s: WSANOTINITIALISED\n", hdr);
	break;
    case WSAEFAULT:		printf ("%s: WSAEFAULT\n", hdr);
	break;
    case WSAEINTR:		printf ("%s: WSAEINTR\n", hdr);
	break;
    case WSAEINPROGRESS:	printf ("%s: WSAEINPROGRESS\n", hdr);
	break;
    case WSAEINVAL:		printf ("%s: WSAEINVAL\n", hdr);
	break;
    case WSAEMFILE:		printf ("%s: WSAEMFILE\n", hdr);
	break;
    case WSAENOBUFS:		printf ("%s: WSAENOBUFS\n", hdr);
	break;
    case WSAENOTSOCK:		printf ("%s: WSAENOTSOCK\n", hdr);
	break;
    case WSAEOPNOTSUPP:		printf ("%s: WSAEOPNOTSUPP\n", hdr);
	break;
    case WSAEWOULDBLOCK:	printf ("%s: WSAEWOULDBLOCK\n", hdr);
	break;
    case WSAEADDRINUSE:		printf ("%s: WSAEADDRINUSE\n", hdr);
	break;
    case WSAEADDRNOTAVAIL:	printf ("%s: WSAEADDRNOTAVAIL\n", hdr);
	break;
    case WSAEAFNOSUPPORT:	printf ("%s: WSAEAFNOSUPPORT\n", hdr);
	break;
    case WSAECONNREFUSED:	printf ("%s: WSAECONNREFUSED\n", hdr);
	break;
    case WSAEDESTADDRREQ:	printf ("%s: WSAEDESTADDRREQ\n", hdr);
	break;
    case WSAEISCONN:		printf ("%s: WSAEISCONN\n", hdr);
	break;
    case WSAENETUNREACH:	printf ("%s: WSAENETUNREACH\n", hdr);
	break;
    case WSAETIMEDOUT:		printf ("%s: WSAETIMEDOUT\n", hdr);
	break;
    case WSAHOST_NOT_FOUND:	printf ("%s: WSAHOST_NOT_FOUND\n", hdr);
	break;
    case WSATRY_AGAIN:		printf ("%s: WSATRY_AGAIN\n", hdr);
	break;
    case WSANO_RECOVERY:	printf ("%s: WSANO_RECOVERY\n", hdr);
	break;
    case WSANO_DATA:		printf ("%s: WSA_NODATA\n", hdr);
	break;
    default: printf ("%s: ERRNO= %d(%08x)\n", hdr, ec, ec);
	break;
    }
}

#else

static void print_errno (char *hdr)
{
    perror (hdr);
}

#endif


int32 cli_recv_noblock (SOCKET sd, char *buf, int32 len)
{
    int32 k;
    
    if ((k = recv (sd, buf, len, 0)) == SOCKET_ERROR) {
	if (SOCKET_ERRNO != SOCKET_WOULDBLOCK) {
	    print_errno ("cli_recv_noblock");
	    return -2;		/* Error */
	} else
	    return 0;
    } else if (k == 0)
	return -1;		/* EOF */
    else
	return k;
}


int32 cli_recv_block (SOCKET sd, char *buf, int32 len)
{
    fd_set readfds;
    
    FD_ZERO (&readfds);
    FD_SET (sd, &readfds);
    if (select (sd+1, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
	print_errno ("cli_recv_select");
	return -2;		/* Error */
    }
    return (cli_recv_noblock (sd, buf, len));
}


int32 cli_send_noblock (SOCKET sd, char *buf, int32 len)
{
    int32 k;
    
    if ((k = send (sd, buf, len, 0)) == SOCKET_ERROR) {
	if (SOCKET_ERRNO != SOCKET_WOULDBLOCK) {
	    print_errno ("cli_send_noblock");
	    return -2;		/* Error */
	} else
	    return 0;
    } else if (k == 0)
	return -1;		/* EOF/error?? */
    else
	return k;
}


int32 cli_send_block (SOCKET sd, char *buf, int32 len)
{
    int32 k, rem;
    
    rem = len;
    while (rem > 0) {
	if ((k = send (sd, buf, rem, 0)) == SOCKET_ERROR) {
	    if (SOCKET_ERRNO != SOCKET_WOULDBLOCK) {
		print_errno ("cli_send_block");
		return -1;		/* Error */
	    }
	} else if (k == 0)
	    return (len-rem);		/* EOF??  Error */
	else {
	    buf += k;
	    rem -= k;
	}
    }
    return len;
}


#if (WIN32)
/*
 * Initialize Windows sockets DLL.  Return 0 if successful, -1 otherwise.
 */
static int32 win32_init ( void )
{
    WORD ver;
    WSADATA data;
    int32 err;
    
    ver = MAKEWORD(2,0);
    if ((err = WSAStartup (ver, &data)) != 0)
	return -1;
#if 0
    if ((LOBYTE(data.wVersion) != 2) || (HIBYTE(data.wVersion) != 0)) {
	WSACleanup ();
	return -1;
    }
#endif

    return 0;
}


static void win32_end ( void )
{
    WSACleanup();
}
#endif


SOCKET cli_open (char *hostname, int32 port)
{
    struct sockaddr_in addr;
    struct hostent *hp;
    int32 flag;
    
#if (WIN32)
    if (win32_init () != 0) {
	fflush (stdout);
	fprintf (stderr, "%s(%d): win32_init failed\n", __FILE__, __LINE__);
	return INVALID_SOCKET;
    }
#endif
    
    /* Get server host info */
    if ((hp = gethostbyname(hostname)) == NULL) {
	print_errno ("gethostbyname");
#if (WIN32)
	win32_end ();
#endif
	return INVALID_SOCKET;
    }
    
    /* Open a TCP socket */
    for (;;) {
	if ((conn_sd = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	    print_errno ("create_socket");
#if (WIN32)
	    win32_end ();
#endif
	    return INVALID_SOCKET;
	}
	
	/* Connect to server */
	memset ((char *) &addr, sizeof(addr), 0);
	addr.sin_family = AF_INET;
	memcpy (&addr.sin_addr, hp->h_addr, hp->h_length);
	addr.sin_port = htons((u_short) port);
	
	if (connect (conn_sd, &addr, sizeof(addr)) == 0)
	    break;
	print_errno ("connect");
	cli_close (conn_sd);

	ERRLOG((stderr, "%s(%d): Retrying...\n", __FILE__, __LINE__));
	
#if (! WIN32)
	sleep(10);
#else
	Sleep(10000);
#endif
    }
    
    /* Set socket to unbuffered mode (ie, execute sends immediately) */
    flag = 1;
    if (setsockopt(conn_sd, IPPROTO_TCP, TCP_NODELAY, (char *)(&flag), sizeof(flag)) ==
	SOCKET_ERROR) {
	print_errno ("socket_nodelay");
	cli_close (conn_sd);
	return INVALID_SOCKET;
    }
    
    /* Set socket to non-blocking mode */
    flag = 1;
    if (SOCKET_IOCTL (conn_sd, FIONBIO, &flag) == SOCKET_ERROR) {
	print_errno ("socket_noblock");
	cli_close (conn_sd);
	return INVALID_SOCKET;
    }
    
    ERRLOG((stderr, "%s(%d): Connected to server\n", __FILE__, __LINE__));
    
    return (conn_sd);
}


/*
 * Close existing connection to server.
 */
void cli_close (SOCKET sd)
{
    assert (sd == conn_sd);

    closesocket (sd);
    conn_sd = INVALID_SOCKET;

#if (WIN32)
    win32_end ();
#endif

    ERRLOG((stderr, "%s(%d): Connection closed\n", __FILE__, __LINE__));
}
