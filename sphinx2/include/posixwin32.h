/*
 * posixwin32.h -- Containing PC win32 specific mappings to Unix names.
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
 * 16-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Copied from Chengxiang Lu and Brian Milnes original.
 */


 #ifdef WIN32
 typedef char * caddr_t;
 typedef unsigned long u_long;
 typedef unsigned int u_int;
 typedef unsigned short u_short;
 typedef unsigned short u_int16;
 typedef unsigned char u_char;
 typedef float float32;
 #define NDEBUG 1
 #define M_PI 3.1415926535897932385E0
 #define popen _popen
 #define pclose _pclose
 #define MAXPATHLEN FILENAME_MAX
 #endif

