/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
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
 * client.c -- client that uses the server in fbs8-6-97/examples/server.
 * 
 * HISTORY
 * 
 * 03-Jun-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Creating.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "s2types.h"

#include "CM_macros.h"
#include "err.h"
#include "ad.h"

#if (! WIN32)
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "clicore.h"


/*
 * NOTE: Each incoming line must be shorter than RBUFSIZE
 */
#define RBUFSIZE	32767	/* Receive buffer size */
#define RBUFLWM		256	/* Low water mark */
static char *rbuf;
static int32 roff;
static int32 rlen;

static SOCKET sd;		/* Socket connection to server */


static char *get_next_line ( void )
{
    int32 i, k, last;
    char *ln;
    
    last = roff;
    for (;;) {
	/* Look for next \n in data already received */
	for (i = last; (i < roff+rlen) && (rbuf[i] != '\n'); i++);
	if (rbuf[i] == '\n') {
	    rbuf[i] = '\0';

	    ln = rbuf+roff;
	    rlen -= i+1 - roff;
	    roff = i+1;
	    
	    return ln;
	}
	last = i;
	
	/* No complete line, receive more data; first reclaim buffer space if necessary */
	if (rlen == RBUFSIZE)
	    E_FATAL("Increase RBUFSIZE\n");
	if ((roff > 0) && (roff+rlen >= RBUFSIZE-RBUFLWM)) {
	    for (i = 0; i < rlen; i++)
		rbuf[i] = rbuf[roff+i];

	    last -= roff;
	    roff = 0;
	}
	
	if ((k = cli_recv_block (sd, rbuf+roff+rlen, RBUFSIZE-roff-rlen)) < 0)
	    E_FATAL("Receive failed\n");
	
	rlen += k;
    }
}


static int32 send_line (char *str)
{
    int32 len;
    
    len = strlen(str);
    if (cli_send_block (sd, str, len) != len)
	return -1;
    
    return 0;
}


int main (int32 argc, char *argv[])
{
    char *host, *ln;
    int32 port;
    
    if (argc != 3)
	E_FATAL("Usage: %s host port\n", argv[0]);
    host = argv[1];
    if (sscanf (argv[2], "%d", &port) != 1)
	E_FATAL("Usage: %s host port\n", argv[0]);
    
    if ((sd = cli_open (host, port)) == INVALID_SOCKET)
	E_FATAL("Failed to connect to %s.%d\n", host, port);
    
    rbuf = (char *) CM_calloc (RBUFSIZE, 1);

    for (;;) {
	ln = get_next_line ();
	printf ("%s\n", ln);

	if (strcmp (ln, "END_UTT") == 0) {
	    if (send_line ("ACK\n") < 0)
		E_FATAL("send failed\n");
	}
    }
}
