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
 * srvcore.c -- Basic socket operations.
 * 
 * HISTORY
 * 
 * 08-Mar-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added LMFILE_READ and LM_DELETE message types to support client
 * 		functions lm_read and lm_delete.
 * 
 * 20-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Added Alex Hauptmann's code to bind to any port if the preferred
 * 		initial choice is busy.
 * 
 * 20-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Included Sunil's changes to identify remote client when a connection
 * 		is made.
 * 
 * 25-Apr-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created, based on Brian Milnes's earlier version.
 * $Log$
 * Revision 1.9  2001/12/11  00:24:48  lenzo
 * Acknowledgement in License.
 * 
 * Revision 1.8  2001/12/07 17:30:01  lenzo
 * Clean up and remove extra lines.
 *
 * Revision 1.7  2001/12/07 12:21:45  lenzo
 * Move some headers.
 *
 * Revision 1.6  2001/12/07 05:09:30  lenzo
 * License.xsxc
 *
 * Revision 1.5  2001/12/07 04:27:35  lenzo
 * License cleanup.  Remove conditions on the names.  Rationale: These
 * conditions don't belong in the license itself, but in other fora that
 * offer protection for recognizeable names such as "Carnegie Mellon
 * University" and "Sphinx."  These changes also reduce interoperability
 * issues with other licenses such as the Mozilla Public License and the
 * GPL.  This update changes the top-level license files and removes the
 * old license conditions from each of the files that contained it.
 * All files in this collection fall under the copyright of the top-level
 * LICENSE file.
 *
 * Revision 1.4  2001/12/07 00:15:32  lenzo
 * Solaris headers ifdef'd in.
 *
 * Revision 1.3  2001/02/13 18:50:35  lenzo
 * Adding some more comments for a Solaris port.
 *
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:41  lenzo
 * Initial import of sphinx2
 *
 *
 */

/*
 * This file tries to hide much of system-specific socket implementation details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifndef WIN32

#if defined(sun) && defined(__svr4__)
/* H J Fox - added include of sys/ioctl.h and sys/filio.h for solaris */
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <netinet/in.h>
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif

#include "srvcore.h"

#define QUIT(x)		{fflush(stdout); fprintf x; exit(-1);}
#ifdef NOERRLOG
#define ERRLOG(x)	{}
#else
#define ERRLOG(x)	{fprintf x;}
#endif

static SOCKET listen_sd = INVALID_SOCKET;	/* Socket over which server listens for
						   connection req */
static SOCKET conn_sd = INVALID_SOCKET;		/* Socket over which server talks to
						   connected client */
static uint16 bindport;

static FILE *fp = NULL;				/* Recvd pkts logfile */
#define RECVLOGFILE	"RCV.LOG"

#ifdef WIN32

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

static char *prt_ctime ()
{
    time_t cur_time;
    
    time (&cur_time);
    return (ctime(&cur_time));
}

int32 server_recv_noblock (SOCKET sd, char *buf, int32 len)
{
    int32 k;
    
    if ((k = recv (sd, buf, len, 0)) == SOCKET_ERROR) {
	if (SOCKET_ERRNO != SOCKET_WOULDBLOCK) {
	    print_errno ("server_recv_noblock");
	    return -2;		/* Error */
	} else
	    return 0;
    } else if (k == 0)
	return -1;		/* EOF */
    else {
	if (fp) {
	    fwrite (buf, sizeof(char), k, fp);
	    fflush (fp);
	}
	return k;
    }
}

int32 server_recv_block (SOCKET sd, char *buf, int32 len)
{
    fd_set readfds;
    
    FD_ZERO (&readfds);
    FD_SET (sd, &readfds);
    if (select (sd+1, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
	print_errno ("server_recv_select");
	return -2;		/* Error */
    }
    return (server_recv_noblock (sd, buf, len));
}

int32 server_send_block (SOCKET sd, char *buf, int32 len)
{
    int32 k, rem;
    
    rem = len;
    while (rem > 0) {
	if ((k = send (sd, buf, rem, 0)) == SOCKET_ERROR) {
	    if (SOCKET_ERRNO != SOCKET_WOULDBLOCK) {
		print_errno ("server_send_block");
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

#ifdef WIN32
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

/*
 * Initialize server; create socket on which to listen for connection request from
 * client.  This server can only have one client connected at any moment.
 * Return 0 if successful, -1 otherwise.
 */
int32 server_initialize (int32 port)
{
    struct sockaddr_in address;
    int32 length;
    
#ifdef WIN32
    if (win32_init () != 0) {
	fflush (stdout);
	fprintf (stderr, "%s(%d): win32_init failed\n", __FILE__, __LINE__);
	return -1;
    }
#endif

    /* Open a TCP socket */
    if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	print_errno ("create_socket");
#ifdef WIN32
	win32_end ();
#endif
	return -1;
    }
    
    /* Bind socket to input argument port, or to  */
    bindport = (uint16) port;
    memset ((char *) &address, 0, sizeof (address));
    address.sin_family		= AF_INET;
    address.sin_addr.s_addr	= htonl(INADDR_ANY);
    address.sin_port		= htons((u_short)bindport);
    if (bind(listen_sd, (struct sockaddr *)(&address), sizeof(address)) == SOCKET_ERROR) {
	print_errno ("bind failed");
	printf ("%s(%d): Trying any available port\n", __FILE__, __LINE__);
	
	memset ((char *) &address, 0, sizeof (address));
	address.sin_family	= AF_INET;
	address.sin_addr.s_addr	= htonl(INADDR_ANY);
	address.sin_port	= 0;
	
	if (bind (listen_sd, (struct sockaddr *)(&address), sizeof(address)) ==
	    SOCKET_ERROR) {
	    print_errno ("Couldn't bind to any port");
	    closesocket (listen_sd);
	    listen_sd = INVALID_SOCKET;
#ifdef WIN32
	    win32_end ();
#endif
	    return -1;
	}
	
	/* Extract bound port */
	length = sizeof(address);
	if (getsockname (listen_sd, (struct sockaddr *)(&address), &length) ==
	    SOCKET_ERROR) {
	    print_errno ("Couldn't get socket name");
	    closesocket (listen_sd);
	    listen_sd = INVALID_SOCKET;
#ifdef WIN32
	    win32_end ();
#endif
	    return -1;
	}

	bindport = address.sin_port;
    }

    /* Prepare to accept client connection request */
    if (listen(listen_sd, 1) == SOCKET_ERROR) {
	print_errno ("listen_socket");
	closesocket(listen_sd);
	listen_sd = INVALID_SOCKET;
#ifdef WIN32
	win32_end ();
#endif
	return -1;
    }
    
    return 0;
}

/*
 * Close existing connection to client.
 */
void server_close_conn (SOCKET sd)
{
    assert (sd == conn_sd);
    closesocket (sd);
    conn_sd = INVALID_SOCKET;
    ERRLOG((stderr, "%s(%d): Connection closed at %s\n", __FILE__, __LINE__, prt_ctime()));
}

/*
 * Await connection.  Return conn_sd if successful, INVALID_SOCKET otherwise.
 */
SOCKET server_await_conn ( void )
{
    struct sockaddr_in address;
    int32 address_len = sizeof (address);
    int32 flag;

    ERRLOG((stderr, "%s(%d): Listening at port %d\n", __FILE__, __LINE__, bindport));

    if ((conn_sd = accept (listen_sd, &address, &address_len)) == INVALID_SOCKET) {
	print_errno ("conn_accept");
	return INVALID_SOCKET;
    }

    /* Set socket to unbuffered mode (ie, execute sends immediately) */
    flag = 1;
    if (setsockopt(conn_sd, IPPROTO_TCP, TCP_NODELAY, (char *)(&flag), sizeof(flag)) ==
	SOCKET_ERROR) {
	print_errno ("socket_nodelay");
	server_close_conn (conn_sd);
	return INVALID_SOCKET;
    }

    /*
     * Set socket to nonblocking mode.
     */
    flag = 1;
    if (SOCKET_IOCTL (conn_sd, FIONBIO, &flag) == SOCKET_ERROR) {
	print_errno ("socket_noblock");
	server_close_conn (conn_sd);
	return INVALID_SOCKET;
    }

    ERRLOG ((stderr, "%s(%d): Connected >>>>>>>> %s at %s\n",
	     __FILE__, __LINE__, inet_ntoa (address.sin_addr), prt_ctime()));
    
    return conn_sd;
}

/*
 * Cleanup; ready to terminate program.
 */
void server_end ( void )
{
    closesocket (listen_sd);
    listen_sd = INVALID_SOCKET;

#ifdef WIN32
    win32_end ();
#endif

    ERRLOG((stderr, "%s(%d): Sockets closed\n", __FILE__, __LINE__));
}

void server_openlog ( void )
{
    if ((fp = fopen (RECVLOGFILE, "wb")) == NULL)
	fprintf (stderr, "%s(%d): fopen(%s,wb) failed\n",
		 __FILE__, __LINE__, RECVLOGFILE);
}

void server_closelog ( void )
{
    if (fp)
	fclose (fp);
    fp = NULL;
}

void server_writelog (char *buf, int32 len)
{
    if (fp) {
	fwrite (buf, sizeof(char), len, fp);
	fflush (fp);
    }
}
