/*
 * srvcore.h -- Raw socket functions packaged up a bit better.
 * 
 * HISTORY
 * 
 * 02-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Adapted from Brian Milnes's initial version.
 */


#ifndef _SRVCORE_H_
#define _SRVCORE_H_

#include <s2types.h>

#ifdef WIN32
#include "win32sock.h"
#else
#include "posixsock.h"
#endif


int32 server_initialize (int32 port);	/* Initialize server, use port as binding addr.
					   Return 0 if successful, -1 otherwise. */

SOCKET server_await_conn ( void );	/* Await connection request from client.
					   Configure accepted socket in TCP_NODELAY,
					   nonblocking mode.  Return accepted socket */

void server_close_conn (SOCKET);	/* Close connection to client over socket */

void server_end (void);			/* Before winding up program */

int32 server_send_block (SOCKET sd, char *buf, int32 len);
					/* Send len bytes from buf over socket, until all
					   bytes sent.  Return #bytes sent, -1 if error. */

int32 server_recv_noblock (SOCKET sd, char *buf, int32 len);
					/* Receive upto len bytes into buf over socket.
					   Return #bytes read (possibly 0 for non-blocking
					   socket), -1 if EOF, -2 if error. */

int32 server_recv_block (SOCKET sd, char *buf, int32 len);
					/* Similar to server_recv_noblock but blocks if no data */

void server_openlog ( void );		/* For logging all recvd data for debugging */
void server_closelog ( void );

#endif
