/*
 * cmd_ln.h -- Command line argument parsing.
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
 * 01-Jan-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


/*********************************************************************
 *
 * $Header$
 *
 * Carnegie Mellon ARPA Speech Group
 *
 * Copyright (c) 1994 Carnegie Mellon University.
 * All rights reserved.
 *
 *********************************************************************
 *
 * file: cmd_ln.h
 * 
 * traceability: 
 * 
 * description: 
 *	This module defines, parses and manages the validation
 * of command line arguments.
 * 
 * author: 
 *	Eric H. Thayer (eht@cs.cmu.edu)
 *
 *********************************************************************/

#ifndef _LIBUTIL_CMD_LN_H_
#define _LIBUTIL_CMD_LN_H_

#ifndef NULL
#define NULL	((void *)0)
#endif

typedef int (*validation_fn_t)(char *switch_name, void *arg);

#define CMD_LN_NO_VALIDATION	((validation_fn_t)NULL)
#define CMD_LN_NO_DEFAULT	((char *)NULL)

typedef enum {
    CMD_LN_UNDEF,
    CMD_LN_INT32,
    CMD_LN_FLOAT32,
    CMD_LN_FLOAT64,
    CMD_LN_STRING,
    CMD_LN_STRING_LIST,
    CMD_LN_BOOLEAN
} arg_type_t;

typedef struct {
    char *switch_name;	/* name of the command line switch */
    arg_type_t type;	/* type of the argument */
    validation_fn_t validate_arg; /* a function which validates
				     argument values */
    void *default_value; /* the default value of the argument, if any */
    char *doc;		 /* a documentation string for the argument */
} arg_def_t;

/* Initializes the command line parsing subsystem */
void
cmd_ln_initialize(void);

/* Defines the set of acceptable command line arguments
 * given the list of command line argument defintions
 * contained in the switch_definition_array.  An element
 * of the array contains the following information:
 *
 *	. switch name (e.g. "-dictionary")
 *
 *	. type of the argument following the switch
 *
 *	. an optional validation function for the
 *	  value given on the command line.
 *
 *	. a translation function for the
 *	  value given on the command line.  This
 *	  function is optional except for
 *	  enumerated types.
 *
 * This function allocates memory, if necessary, for 
 * parsed command line arguments.
*/
int
cmd_ln_define(arg_def_t *defn);

/*
 * Reads the command line and converts the arguments,
 * if necessary, according to their definitions.
 */
int
cmd_ln_parse(int argc, char *argv[]);

/*
 * Calls the validation functions for each of the
 * command line arguments.
 */
int
cmd_ln_validate();

/* Returns a pointer to the parsed command line argument.
   (The returned pointer must not be given to the
   function free())
   It is const because the caller should not modify the thing
   referred to by this pointer. */

const void *
cmd_ln_access(char *switch_name);

/* Prints out the command line argument definitions to stderr */

void
cmd_ln_print_definitions(void);

void
cmd_ln_print_configuration( void );

#endif /* CMD_LN_H */ 


/*
 * Log record.  Maintained by CVS.
 *
 * $Log$
 * Revision 1.1  2000/04/24  09:39:42  lenzo
 * s3 import.
 * 
 *
 */
