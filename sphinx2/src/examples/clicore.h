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
 * clicore.h -- Network client interface.
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.6  2001/12/11  00:24:48  lenzo
 * Acknowledgement in License.
 * 
 * Revision 1.5  2001/12/07 17:30:01  lenzo
 * Clean up and remove extra lines.
 *
 * Revision 1.4  2001/12/07 05:09:30  lenzo
 * License.xsxc
 *
 * Revision 1.3  2001/12/07 04:27:35  lenzo
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
