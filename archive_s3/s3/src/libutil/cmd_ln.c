/*
 * cmd_ln.c -- Command line argument parsing.
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
 * file: cmd_ln.c
 * 
 * Description: 
 *	This library parses command line arguments and provides
 *	an interface for accessing them.
 *
 * Author: 
 *	$Author$
 * 
 *********************************************************************/

static char rcsid[] = "@(#)$Id$";


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "prim_type.h"
#include "ckd_alloc.h"
#include "cmd_ln.h"
#include "err.h"


void
cmd_ln_initialize(void)
{
}


/*********************************************************************
 *
 * Function: alloc_type	(local scope)
 * 
 * Description: 
 * 	Initializes the command line parsing subsystem
 * 
 * Traceability: 
 * 
 * Function Inputs: 
 *	arg_type_t t -
 *		A command line argument type.
 * 
 * Return Values: 
 *	A pointer to an allocated item of the given type.  An
 *	exception to this is the string type.  There is no
 *	allocation needed for this as it is already represented
 *	by a pointer type.
 * 
 * Global Outputs: 
 * 
 * Errors: 
 * 
 * Pre-Conditions: 
 * 
 * Post-Conditions: 
 * 
 * Design: 
 * 
 * Notes: 
 * 
 *********************************************************************/
static void *
alloc_type(arg_type_t t)
{
    void *mem;

    switch (t) {
    case CMD_LN_INT32:
	mem = ckd_malloc(sizeof(int32));
	break;
	
    case CMD_LN_FLOAT32:
	mem = ckd_malloc(sizeof(float32));
	break;
	
    case CMD_LN_FLOAT64:
	mem = ckd_malloc(sizeof(float64));
	break;

    case CMD_LN_STRING:
	mem = NULL;	/* no allocations done for string type */
	break;
	
    case CMD_LN_BOOLEAN:
	mem = ckd_malloc(sizeof(int32));
	break;

    case CMD_LN_UNDEF:
	E_FATAL("undefined argument type\n");
    }
    
    return mem;
}


static void *
arg_to_str(arg_type_t t, void *val)
{
    static char big_str[1024];
    char **strlst;
    int i;

    switch (t) {
    case CMD_LN_INT32:
	sprintf(big_str, "%d", *(int32 *)val);
	break;
	
    case CMD_LN_FLOAT32:
	sprintf(big_str, "%e", *(float32 *)val);
	break;
	
    case CMD_LN_FLOAT64:
	sprintf(big_str, "%le", *(float64 *)val);
	break;
	
    case CMD_LN_STRING:
	strcpy(big_str, val);
	break;
	
    case CMD_LN_STRING_LIST:
	strlst = (char **)val;
	
	strcpy(big_str, strlst[0]);
	for (i = 1; strlst[i] != NULL; i++) {
	    strcat(big_str, " ");
	    strcat(big_str, strlst[i]);
	}
	
	break;
	
    case CMD_LN_BOOLEAN:
	sprintf(big_str, "%s",
		(*(int32 *)val ? "yes" : "no"));
	break;
	
    case CMD_LN_UNDEF:
	E_FATAL("undefined argument type\n");
    }
    
    return big_str;
}


static arg_def_t *defn_list = NULL;
static int defn_list_len = 0;
static void **parsed_arg_list = NULL;

#define SWITCH_HEADER	"[Switch]"
#define DEFAULT_HEADER	"[Default]"
#define VALUE_HEADER	"[Value]"
#define DOC_HEADER	"[Description]"

static uint32
strlst_len(char **lst)
{
    uint32 len;
    uint32 i;

    
    len = strlen(lst[0]);

    for (i = 1; lst[i] != NULL; i++) {
	len += strlen(lst[i]) + 1;
    }

    return len;
}


static void
strlst_print(char **lst)
{
    uint32 i;

    printf("%s", lst[0]);
    for (i = 1; lst[i] != NULL; i++) {
	printf(" %s", lst[i]);
    }
}


void
cmd_ln_print_configuration()
{
    int i;
    int len;
    int mx_sw_len = 0;
    int mx_df_len = 0;
    int mx_vl_len = 0;
    char fmt_str[64];

    if (defn_list == NULL) {
	E_WARN("No switches defined.  None printed\n");

	return;
    }

    for (i = 0; i < defn_list_len; i++) {
	len = strlen(defn_list[i].switch_name);
	if (len > mx_sw_len) mx_sw_len = len;

	if (defn_list[i].default_value) {
	    if (defn_list[i].type != CMD_LN_STRING_LIST)
		len = strlen(defn_list[i].default_value);
	    else
		len = strlst_len(defn_list[i].default_value);

	    if (len > mx_df_len) mx_df_len = len;
	}

	if (parsed_arg_list[i]) {
	    len = strlen(arg_to_str(defn_list[i].type,
				    parsed_arg_list[i]));
	    if (len > mx_vl_len) mx_vl_len = len;
	}
    }

    if (mx_sw_len < strlen(SWITCH_HEADER)) mx_sw_len = strlen(SWITCH_HEADER);
    if (mx_df_len < strlen(DEFAULT_HEADER)) mx_df_len = strlen(DEFAULT_HEADER);
    if (mx_vl_len < strlen(VALUE_HEADER)) mx_df_len = strlen(VALUE_HEADER);
    
    sprintf(fmt_str, "%%-%ds %%-%ds %%-%ds\n",
	    mx_sw_len, mx_df_len, mx_vl_len);
    printf(fmt_str, SWITCH_HEADER, DEFAULT_HEADER, VALUE_HEADER);

    for (i = 0; i < defn_list_len; i++) {
	printf(fmt_str,
	       defn_list[i].switch_name,
	       (defn_list[i].default_value ? defn_list[i].default_value : ""),
	       (parsed_arg_list[i]? arg_to_str(defn_list[i].type,
					       parsed_arg_list[i]) : ""));
    }

    fflush(stdout);
}


void
cmd_ln_print_definitions()
{
    int i;
    int len;
    int mx_sw_len = 0;
    int mx_df_len = 0;
    int mx_ds_len = 0;
    char fmt_str[64];

    if (defn_list == NULL) {
	E_WARN("No switches defined.  None printed\n");

	return;
    }

    for (i = 0; i < defn_list_len; i++) {
	len = strlen(defn_list[i].switch_name);
	if (len > mx_sw_len) mx_sw_len = len;

	if (defn_list[i].default_value) {
	    len = strlen(defn_list[i].default_value);
	    if (len > mx_df_len) mx_df_len = len;
	}

	if (defn_list[i].doc) {
	    len = strlen(defn_list[i].doc);
	    if (len > mx_ds_len) mx_ds_len = len;
	}
    }

    if (mx_sw_len < strlen(SWITCH_HEADER)) mx_sw_len = strlen(SWITCH_HEADER);
    if (mx_df_len < strlen(DEFAULT_HEADER)) mx_df_len = strlen(DEFAULT_HEADER);
    if (mx_ds_len < strlen(DOC_HEADER)) mx_ds_len = strlen(DOC_HEADER);

    /* sprintf(fmt_str, "%%-%ds %%-%ds %%-%ds\n", mx_sw_len, mx_df_len, mx_ds_len); */
    sprintf(fmt_str, "%%-%ds %%-%ds %%s\n", mx_sw_len, mx_df_len);
    printf(fmt_str, SWITCH_HEADER, DEFAULT_HEADER, DOC_HEADER);

    for (i = 0; i < defn_list_len; i++) {
	printf(fmt_str,
	       defn_list[i].switch_name,
	       (defn_list[i].default_value ? defn_list[i].default_value : ""),
	       (defn_list[i].doc ? defn_list[i].doc : ""));
    }

    fflush(stdout);
}


static void
free_and_alloc(void **arg_ptr, arg_type_t t)
{
    if (*arg_ptr)
	ckd_free(*arg_ptr);
    
    *arg_ptr = alloc_type(t);
}


static void
free_and_alloc_strlst(void **arg_ptr, int new_len)
{
    if (*arg_ptr)
	ckd_free(*arg_ptr);
    
    *arg_ptr = ckd_calloc(new_len, sizeof(char *));
}


static int
parse_arg(int defn_idx, int argc, char *argv[], int start)
{
    arg_type_t arg_type;
    int n_parsed;
    int end;
    int len;
    char *cur_arg;
    int i;

    arg_type = defn_list[defn_idx].type;

    cur_arg = argv[start];

    switch (arg_type) {
	case CMD_LN_INT32:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	*((int *)parsed_arg_list[defn_idx]) = atoi(cur_arg);
	n_parsed = 1;
	break;

	case CMD_LN_FLOAT32:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	*((float *)parsed_arg_list[defn_idx]) = (float)atof(cur_arg);
	n_parsed = 1;
	break;

	case CMD_LN_FLOAT64:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	*((double *)parsed_arg_list[defn_idx]) = atof(cur_arg);
	n_parsed = 1;
	break;

	case CMD_LN_STRING:
	parsed_arg_list[defn_idx] = (void *)cur_arg;
	n_parsed = 1;
	break;

	case CMD_LN_STRING_LIST:
	if (start > 0) {
	    /* start > 0 always the case if parsing a command line. */
	    for (i = start+1; i < argc; i++) {
		if (argv[i][0] == '-')
		    break;
	    }
	}
	else {
	    /* start == 0 always the case if parsing a default */

	    assert(argc == 0);	/* caller must ensure this.  Should help
				 * to avoid bogus conditions */

	    for (i = start+1; argv[i] != NULL; i++);
	}
	
	end = i;

	len = end - start;
	len++;	/* need space for terminating NULL */

	free_and_alloc_strlst(&parsed_arg_list[defn_idx], len);

	for (i = start; i < end; i++) {
	    ((char **)parsed_arg_list[defn_idx])[i-start] = argv[i];
	}
	((char **)parsed_arg_list[defn_idx])[i-start] = NULL;

	n_parsed = len-1;
	break;
	
	case CMD_LN_BOOLEAN:
	free_and_alloc(&parsed_arg_list[defn_idx], arg_type);
	if ((cur_arg[0] == 'y') || (cur_arg[0] == 't') ||
	    (cur_arg[0] == 'Y') || (cur_arg[0] == 'T')) {
	    *(int *)parsed_arg_list[defn_idx] = TRUE;
	    n_parsed = 1;
	}
	else if ((cur_arg[0] == 'n') || (cur_arg[0] == 'f') ||
		 (cur_arg[0] == 'N') || (cur_arg[0] == 'F')) {
	    *(int *)parsed_arg_list[defn_idx] = FALSE;
	    n_parsed = 1;
	}
	else {
	    E_ERROR("Unparsed boolean value '%s'\n", cur_arg);
	    n_parsed = -1;
	}
	break;

	case CMD_LN_UNDEF:
	E_FATAL("Definition for argument %s has undefined type\n",
		defn_list[defn_idx].switch_name);

	default:
	E_FATAL("No case in switch() {} for enum value.\n");
    }

    return n_parsed;
}


int
cmd_ln_define(arg_def_t *defn)
{
    int i;

    assert(defn != NULL);

    defn_list = defn;

    for (i = 0; defn_list[i].switch_name != NULL; i++);
    defn_list_len = i;

    parsed_arg_list = ckd_calloc(defn_list_len, sizeof(void *));

    for (i = 0; i < defn_list_len; i++) {
	if (defn_list[i].default_value) {
	    if (defn_list[i].type != CMD_LN_STRING_LIST) {
		parse_arg(i,
			  0,	/* ignored if start == 0 */
			  (char **) &defn_list[i].default_value,
			  0);
	    }
	    else {
		parse_arg(i,
			  0,	/* ignored if start == 0 */
			  defn_list[i].default_value,
			  0);
	    }
	}
    }

    return 0;
}


static int did_parse = FALSE;

int
cmd_ln_parse(int argc, char *argv[])
{
    int i, j, err, n_arg_parsed;

    if (defn_list == NULL) {
	E_WARN("No switches defined.  None parsed\n");

	did_parse = TRUE;

	return 0;
    }

    for (i = 1, err = 0; i < argc; i++) {
	if (argv[i][0] != '-') {
	    E_ERROR("Expecting '%s -switch_1 <arg_1> -switch_2 <arg_2> ...'\n",
		    argv[0]);

	    err = 1;
	    break;
	}

	for (j = 0; j < defn_list_len; j++) {
	    if (strcmp(argv[i], defn_list[j].switch_name) == 0) {

		n_arg_parsed = parse_arg(j, argc, argv, i+1);
		if (n_arg_parsed < 0) {
		    err = 1;
		}

		i += n_arg_parsed;	/* i incremented for each switch as well */

		break;
	    }
	}

	if (j == defn_list_len) {
	    E_ERROR("Unknown switch %s seen\n", argv[i]);

	    err = 1;
	}
    }

    if (err) {
	exit(1);
    }

    did_parse = TRUE;
    
    return 0;
}


int
cmd_ln_validate()
{
    int i;
    int err;

    if (!did_parse) {
	E_FATAL("cmd_ln_parse() must be called before cmd_ln_validate()\n");
    }

    for (i = 0, err = 0; i < defn_list_len; i++) {
	if (defn_list[i].validate_arg) {
	    if (defn_list[i].validate_arg(defn_list[i].switch_name,
					  parsed_arg_list[i]) == FALSE) {
		err = 1;
	    }
	}
    }

    if (err) return FALSE;

    return TRUE;
}


const void *cmd_ln_access(char *switch_name)
{
    int i;

    if (!did_parse) {
	E_FATAL("cmd_ln_parse() must be called before cmd_ln_access()\n");
    }

    for (i = 0; i < defn_list_len; i++) {
	if (strcmp(defn_list[i].switch_name, switch_name) == 0) {
	    break;
	}
    }

    if (i == defn_list_len) {
	E_FATAL("Unknown switch %s\n", switch_name);
    }

    return parsed_arg_list[i];
}


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.1  2000/04/24  09:39:42  lenzo
 * s3 import.
 * 
 * Revision 1.2  1995/06/02  14:52:54  eht
 * Use pwp's error printing package
 *
 * Revision 1.1  1995/02/13  15:47:41  eht
 * Initial revision
 *
 *
 */
