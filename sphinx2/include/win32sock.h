/*
 * win32sock.h -- Hide Windows NT specific socket functions and variables here.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.1  2000/01/28  22:09:07  lenzo
 * Initial revision
 * 
 * 
 * 02-May-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created.
 */


#ifndef _WIN32SOCK_H_
#define _WIN32SOCK_H_

#include <winsock.h>

#define SOCKET_ERRNO		WSAGetLastError()
#define SOCKET_WOULDBLOCK	WSAEWOULDBLOCK
#define SOCKET_IOCTL		ioctlsocket

#endif
