/*
 * err.h -- Package for checking and catching common errors, printing out
 *		errors nicely, etc.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * 6/01/95  Paul Placeway  CMU speech group
 */

#ifndef _LIBUTIL_ERR_H_
#define _LIBUTIL_ERR_H

#include <stdarg.h>
#include <errno.h>

void _E__pr_header( char const *file, long line, char const *msg );
void _E__pr_info_header( char const *file, long line, char const *tag );
void _E__pr_warn( char const *fmt, ... );
void _E__pr_info( char const *fmt, ... );
void _E__die_error( char const *fmt, ... );
void _E__abort_error( char const *fmt, ... );
void _E__sys_error( char const *fmt, ... );
void _E__fatal_sys_error( char const *fmt, ... );

/* These three all abort */

/* core dump after error message */
#define E_ABORT  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__abort_error

/* exit with non-zero status after error message */
#define E_FATAL  _E__pr_header(__FILE__, __LINE__, "FATAL_ERROR"),_E__die_error

/* Print error text; Call perror(""); exit(errno); */
#define E_FATAL_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__fatal_sys_error

/* Print error text; Call perror(""); */
#define E_WARN_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__sys_error

/*
 * Prints error text only.
 *
 * This allows a lower level routine to give information regarding an error condition,
 * but allows higher level routines to give addl information and make the
 * determination whether or not to abort.
 */
#define E_ERROR	  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__pr_warn

#define E_INFO	  _E__pr_info_header(__FILE__, __LINE__, "INFO"),_E__pr_info

#define E_WARN	  _E__pr_header(__FILE__, __LINE__, "WARNING"),_E__pr_warn

#endif /* !_ERR_H */
