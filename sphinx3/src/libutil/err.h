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
#define _LIBUTIL_ERR_H_

/* 01.18.01 RAH, allow for C++ compiles */
#ifdef __cplusplus
extern "C" {
#endif



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
/* this macro is never used in the code, and conflicts with MS Visual C
#ifndef E_ABORT
#define E_ABORT  _E__pr_header(__FILE__, __LINE__, "ERROR"),_E__abort_error
#endif
*/

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


