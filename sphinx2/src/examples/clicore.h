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
 * clicore.h -- Network client interface.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.2  2000/12/05  01:45:12  lenzo
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
