/*
 * clicore.h -- Network client interface.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.3  2001/12/07  04:27:35  lenzo
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
 * Revision 1.2  2000/12/05 01:45:12  lenzo
 * Restructuring, hear rationalization, warning removal, ANSIfy
 *
 * Revision 1.1.1.1  2000/01/28 22:08:41  lenzo
 * Initial import of sphinx2
 *
 *
 * 
 * 21-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#ifndef _CLICORE_H_
#define _CLICORE_H_

#include "s2types.h"


#if (WIN32)
#include "win32sock.h"
#else
#include "posixsock.h"
#endif


SOCKET cli_open (char *hostname, int32 port);
				/* Open socket connection to server on host using port as
				   address, in non-blocking mode.  Return created socket
				   (or INVALID_SOCKET if error). */

void cli_close (SOCKET);	/* Close SOCKET connection to server */

int32 cli_send_block (SOCKET sd, char *buf, int32 len);
				/* Send len bytes from buf over socket until all sent.
				   Return #bytes sent or -1 if error. */

int32 cli_send_noblock (SOCKET sd, char *buf, int32 len);
				/* Send len bytes from buf over socket but do not block.
				   Return #bytes sent (possibly 0 for non-blocking socket),
				   -1 if EOF, -2 if error. */

int32 cli_recv_noblock (SOCKET sd, char *buf, int32 len);
				/* Receive upto len bytes into buf over socket.  Return
				   #bytes read (possibly 0 for non-blocking socket),
				   -1 if EOF, -2 if error. */

int32 cli_recv_block (SOCKET sd, char *buf, int32 len);
				/* Blocking version of cli_recv_noblock */

#endif
