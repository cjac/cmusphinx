/*
 * err.h -- Package for checking and catching common errors, printing out
 *		errors nicely, etc.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 *
 * 6/01/95  Paul Placeway  CMU speech group
 */


#ifndef _LIBUTIL_ERR_H_
#define _LIBUTIL_ERR_H

/* 01.18.01 RAH, allow for C++ compiles */
#ifdef __cplusplus
extern "C" {
#endif



#include <stdarg.h>
#include <errno.h>


void _E__pr_header( const char *file, long line, const char *msg );
void _E__pr_info_header( char *file, long line, char *tag );
void _E__pr_warn( char *fmt, ... );
void _E__pr_info( char *fmt, ... );
void _E__die_error( char *fmt, ... );
void _E__abort_error( char *fmt, ... );
void _E__sys_error( char *fmt, ... );
void _E__fatal_sys_error( char *fmt, ... );

/* These three all abort */

/* core dump after error message */
#ifndef E_ABORT
#define E_ABORT  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__abort_error
#endif

/* exit with non-zero status after error message */
#define E_FATAL  _E__pr_header(__FILE__, __LINE__, "FATAL_ERROR"),_E__die_error

/* Print error text; Call perror(""); exit(errno); */
#define E_FATAL_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__fatal_sys_error

/* Print error text; Call perror(""); */
#define E_WARN_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__sys_error

/* Print error text; Call perror(""); */
#define E_ERROR_SYSTEM	_E__pr_header(__FILE__, __LINE__, "SYSTEM_ERROR"),_E__sys_error


/* Print logging information, warnings, or error messages; all to stderr */
#define E_INFO	  _E__pr_info_header(__FILE__, __LINE__, "INFO"),_E__pr_info

#define E_WARN	  _E__pr_header(__FILE__, __LINE__, "WARNING"),_E__pr_warn

#define E_ERROR	  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__pr_warn


#ifdef __cplusplus
}
#endif


#endif /* !_ERR_H */


