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
