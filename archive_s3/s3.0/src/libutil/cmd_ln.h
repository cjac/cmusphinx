/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * cmd_ln.h -- Command line argument parsing.
 *
 * 
 * HISTORY
 * 
 * 15-Jul-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added required arguments types.
 * 
 * 07-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created, based on Eric's implementation.  Basically, combined several
 *		functions into one, eliminated validation, and simplified the interface.
 */


#ifndef _LIBUTIL_CMD_LN_H_
#define _LIBUTIL_CMD_LN_H_


#include "prim_type.h"

#define ARG_REQUIRED	1

/* Arguments of these types are OPTIONAL */
#define ARG_INT32	2
#define ARG_FLOAT32	4
#define ARG_FLOAT64	6
#define ARG_STRING	8

/* Arguments of these types are REQUIRED */
#define REQARG_INT32	(ARG_INT32 | ARG_REQUIRED)
#define REQARG_FLOAT32	(ARG_FLOAT32 | ARG_REQUIRED)
#define REQARG_FLOAT64	(ARG_FLOAT64 | ARG_REQUIRED)
#define REQARG_STRING	(ARG_STRING | ARG_REQUIRED)
typedef int32 argtype_t;


typedef struct {
    char *name;		/* Name of the command line switch (case-insensitive) */
    argtype_t type;
    char *deflt;	/* Default value (as a printed string) or NULL if none */
    char *doc;		/* Documentation/description string */
} arg_t;


/*
 * Parse the given list of arguments (name-value pairs) according to the given definitions.
 * Argument values can be retrieved in future using cmd_ln_access().  argv[0] is assumed to be
 * the program name and skipped.  Any unknown argument name causes a fatal error.  The routine
 * also prints the prevailing argument values (to stderr) after parsing.
 * Return value: 0 if successful, -1 if error.
 */
int32 cmd_ln_parse (arg_t *defn,	/* In: Array of argument name definitions */
		    int32 argc,		/* In: #Actual arguments */
		    char *argv[]);	/* In: Actual arguments */


/*
 * Return a pointer to the previously parsed value for the given argument name.
 * The pointer should be cast to the appropriate type before use:
 * e.g., *((float32 *) cmd_ln_access ("-eps") to get the float32 argument named "-eps".
 * And, some convenient wrappers around this function.
 */
const void *cmd_ln_access (char *name);	/* In: Argument name whose value is sought */
#define cmd_ln_str(name)	((char *)cmd_ln_access(name))
#define cmd_ln_int32(name)	(*((int32 *)cmd_ln_access(name)))
#define cmd_ln_float32(name)	(*((float32 *)cmd_ln_access(name)))
#define cmd_ln_float64(name)	(*((float64 *)cmd_ln_access(name)))


/*
 * Print a help message listing the valid argument names, and the associated
 * attributes as given in defn.
 */
void  cmd_ln_print_help (FILE *fp,	/* In: File to which to print */
			 arg_t *defn);	/* In: Array of argument name definitions */

#endif
