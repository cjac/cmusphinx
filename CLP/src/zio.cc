// -*- C++ -*-
//
// Copyright (c) 1995-1998 SRI International and Andreas Stolcke.
// All right reserved.
//
// Permission to use, copy, and modify this software and its documentation
// for any non-commercial purpose and without fee is hereby granted,
// provided that this entire copyright notice is included on all copies of this
// software and applications and derivations thereof.
// This software is provided on an "as is" basis, without warranty of any
// kind, either expressed or implied, as to any matter including, but not
// limited to warranty of fitness of purpose, or merchantability, or
// results obtained from use of this software.

//      $Id: zio.cc,v 1.2 2000/01/31 21:56:50 chelba Exp $      
//      $Log: zio.cc,v $
//      Revision 1.2  2000/01/31 21:56:50  chelba
//      .

//    Author: Andreas Stolcke
//    Date:   Wed Feb 15 15:19:44 PST 1995
   
//    Description:
//                 Compressed file stdio extension


#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

extern int errno;

#include "zio.h"

#ifdef ZIO_HACK
#undef fopen
#undef fclose
#endif

#define STDIO_NAME	  "-"

#define STD_PATH    ":"   /* "PATH=/usr/bin:/usr/ucb:/usr/bsd:/usr/local/bin" */

#define COMPRESS_CMD	  "compress -c"
#define UNCOMPRESS_CMD	  "uncompress -c"

#define GZIP_CMD	  "gzip -c"
#define GUNZIP_CMD	  "gunzip -c"

/*
 * Does the filename refer to stdin/stdout ?
 */
static int
stdio_filename_p (const char *name)
{
    return (strcmp(name, STDIO_NAME) == 0);
}

/*
 * Does the filename refer to a compressed file ?
 */
static int
compressed_filename_p (const char *name)
{
   int len = strlen(name);

   return
     (len > (int)(sizeof(COMPRESS_SUFFIX)-1)) &&
       (strcmp(name + len - (sizeof(COMPRESS_SUFFIX)-1),
	       COMPRESS_SUFFIX) == 0);
}

/*
 * Does the filename refer to a gzipped file ?
 */
static int
gzipped_filename_p (const char *name)
{
    int len = strlen(name);

    return 
	(len > (int)(sizeof(GZIP_SUFFIX)-1)) &&
		(strcmp(name + len - (sizeof(GZIP_SUFFIX)-1),
			GZIP_SUFFIX) == 0) ||
	(len > (int)(sizeof(OLD_GZIP_SUFFIX)-1)) &&
		(strcmp(name + len - (sizeof(OLD_GZIP_SUFFIX)-1),
			OLD_GZIP_SUFFIX) == 0);
}

/*
 * Check file readability
 */
static int
readable_p (const char *name)
{
    int fd = open(name, O_RDONLY);

    if (fd < 0)
        return 0;
    else {
        close(fd);
	return 1;
    }
}

/*
 * Check file writability
 */
static int
writable_p (const char *name)
{
    int fd = open(name, O_WRONLY|O_CREAT, 0666);

    if (fd < 0)
        return 0;
    else {
        close(fd);
	return 1;
    }
}

/*
 * Open a stdio stream, handling special filenames
 */
FILE *zopen(const char *name, const char *mode)
{
    char command[MAXPATHLEN + 100];

    if (stdio_filename_p(name)) {
	/*
	 * Return stream to stdin or stdout
	 */
	if (*mode == 'r') {
		int fd = dup(0);
		return fd < 0 ? NULL : fdopen(fd, mode);
	} else if (*mode == 'w' || *mode == 'a') {
		int fd = dup(1);
		return fd < 0 ? NULL : fdopen(fd, mode);
	} else {
		return NULL;
	}
    } else if (compressed_filename_p(name)) {
	/*
	 * Return stream to compress pipe
	 */
	if (*mode == 'r') {
		if (!readable_p(name))
		    return NULL;
		sprintf(command, "%s;%s %s", STD_PATH, UNCOMPRESS_CMD, name);
		return popen(command, mode);
	} else if (*mode == 'w') {
		if (!writable_p(name))
		    return NULL;
		sprintf(command, "%s;%s >%s", STD_PATH, COMPRESS_CMD, name);
		return popen(command, mode);
	} else {
		return NULL;
	}
    } else if (gzipped_filename_p(name)) {
	/*
	 * Return stream to gzip pipe
	 */
	if (*mode == 'r') {
		if (!readable_p(name))
		    return NULL;
		sprintf(command, "%s;%s %s", STD_PATH, GUNZIP_CMD, name);
		return popen(command, mode);
	} else if (*mode == 'w') {
		if (!writable_p(name))
		    return NULL;
		sprintf(command, "%s;%s >%s", STD_PATH, GZIP_CMD, name);
		return popen(command, mode);
	} else {
		return NULL;
	}
    } else {
	return fopen(name, mode);
    }
}

/*
 * Close a stream created by zopen()
 */
int
zclose(FILE *stream)
{
    int status;
    struct stat statb;

    /*
     * pclose(), according to the man page, should diagnose streams not 
     * created by popen() and return -1.  however, on SGIs, it core dumps
     * in that case.  So we better be careful and try to figure out
     * what type of stream it is.
     */
    if (fstat(fileno(stream), &statb) < 0)
	return -1;

    /*
     * First try pclose().  It will tell us if stream is not a pipe
     */
    if ((statb.st_mode & S_IFMT) != S_IFIFO ||
        fileno(stream) == 0 || fileno(stream) == 1)
    {
        return fclose(stream);
    } else {
	status = pclose(stream);
	if (status == -1) {
	    /*
	     * stream was not created by popen(), but popen() does fclose
	     * for us in thise case.
	     */
	    return ferror(stream);
	} else {
	    /*
	     * The compressor program terminated with an error, and supposed
	     * has printed a message to stderr.
	     * Set errno to a generic error code if is hasn't been set already.
	     */
	    if (errno == 0) {
		errno = EIO;
	    }
	    return status;
	}
    }
}

#ifdef STAND
int
main (argc, argv)
    int argc;
    char **argv;
{
    int dowrite = 0;
    char buffer[BUFSIZ];
    int nread;
    FILE *stream;

    if (argc < 3) {
	printf("usage: %s file {r|w}\n", argv[0]);
 	exit(2);
    }

    if (*argv[2] == 'r') {
	stream = zopen(argv[1], argv[2]);

	if (!stream) {
		perror(argv[1]);
		exit(1);
	}

	while (!ferror(stream) && !feof(stream) &&!ferror(stdout)) {
		nread = fread(buffer, 1, sizeof(buffer), stream);
		(void)fwrite(buffer, 1, nread, stdout);
	}
    } else {
	stream = zopen(argv[1], argv[2]);

	if (!stream) {
		perror(argv[1]);
		exit(1);
	}

	while (!ferror(stdin) && !feof(stdin) && !ferror(stream)) {
		nread = fread(buffer, 1, sizeof(buffer), stdin);
		(void)fwrite(buffer, 1, nread, stream);
	}
   }
   if (ferror(stdin)) {
	perror("stdin");
   } else if (ferror(stdout)) {
	perror("stdout");
   } else if (ferror(stream)) {
	perror(argv[1]);
   }
   zclose(stream);
   
}
#endif /* STAND */
