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
 * cmd_ln.c -- Command line argument parsing.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 10-Sep-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed strcasecmp() call in cmp_name() to strcmp_nocase() call.
 * 
 * 15-Jul-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added required arguments handling.
 * 
 * 07-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created, based on Eric's implementation.  Basically, combined several
 *		functions into one, eliminated validation, and simplified the interface.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cmd_ln.h"
#include "err.h"
#include "ckd_alloc.h"
#include "hash.h"
#include "case.h"


/* Storage for argument values */
typedef struct argval_s {
    anytype_t val;
    const void *ptr;	/* Needed (with NULL value) in case there is no default */
} argval_t;
static argval_t *argval = NULL;
static hash_table_t *ht;	/* Hash table */


#if 0
static const char *arg_type2str (argtype_t t)
{
    switch (t) {
    case ARG_INT32:
    case REQARG_int32:
	return ("int32");
	break;
    case ARG_FLOAT32:
    case REQARG_FLOAT32:
	return ("float32");
	break;
    case ARG_FLOAT64:
    case REQARG_FLOAT64:
	return ("float64");
	break;
    case ARG_STRING:
    case REQARG_STRING:
	return ("string");
	break;
    default: E_FATAL("Unknown argument type: %d\n", t);
    }
}
#endif


/*
 * Find max length of name and default fields in the given defn array.
 * Return #items in defn array.
 */
static int32 arg_strlen (arg_t *defn, int32 *namelen, int32 *deflen)
{
    int32 i, l;
    
    *namelen = *deflen = 0;
    for (i = 0; defn[i].name; i++) {
	l = strlen (defn[i].name);
	if (*namelen < l)
	    *namelen = l;
	
	if (defn[i].deflt) {
	    l = strlen (defn[i].deflt);
	    if (*deflen < l)
		*deflen = l;
	}
    }

    return i;
}


/* For sorting argument definition list by name */
static arg_t *tmp_defn;

static int32 cmp_name (const void *a, const void *b)
{
    return (strcmp_nocase (tmp_defn[*((int32 *)a)].name, tmp_defn[*((int32 *)b)].name));
}

static int32 *arg_sort (arg_t *defn, int32 n)
{
    int32 *pos;
    int32 i;
    
    pos = (int32 *) ckd_calloc (n, sizeof(int32));
    for (i = 0; i < n; i++)
	pos[i] = i;
    tmp_defn = defn;
    qsort (pos, n, sizeof(int32), cmp_name);
    tmp_defn = NULL;

    return pos;
}


static int32 arg_str2val (argval_t *v, argtype_t t, char *str)
{
    if (! str)
	v->ptr = NULL;
    else {
	switch (t) {
	case ARG_INT32:
	case REQARG_INT32:
	  if (sscanf (str, "%d", &(v->val.i_32)) != 1)
		return -1;
	  v->ptr = (void *) &(v->val.i_32);
	    break;
	case ARG_FLOAT32:
	case REQARG_FLOAT32:
	  if (sscanf (str, "%f", &(v->val.fl_32)) != 1)
	    return -1;
	  v->ptr = (void *) &(v->val.fl_32);
	    break;
	case ARG_FLOAT64:
	case REQARG_FLOAT64:
	  if (sscanf (str, "%lf", &(v->val.fl_64)) != 1)
	    return -1;
	  v->ptr = (void *) &(v->val.fl_64);
	    break;
	case ARG_STRING:
	case REQARG_STRING:
	    v->ptr = (void *) str;
	    break;
	default: E_FATAL("Unknown argument type: %d\n", t);
	}
    }
    
    return 0;
}


static void arg_dump (FILE *fp, arg_t *defn, int32 doc)
{
    int32 *pos;
    int32 i, j, l, n;
    int32 namelen, deflen;
    const void *vp;
    
    /* Find max lengths of name and default value fields, and #entries in defn */
    n = arg_strlen (defn, &namelen, &deflen);
    namelen = namelen & 0xfffffff8;	/* Previous tab position */
    deflen = deflen & 0xfffffff8;	/* Previous tab position */

    fprintf (fp, "[NAME]");
    for (l = 6; l < namelen; l += 8)	/* strlen("[NAME]") */
	fprintf (fp, "\t");
    fprintf (fp, "\t[DEFLT]");
    for (l = 6; l < deflen; l += 8)	/* strlen("[DEFLT]") */
	fprintf (fp, "\t");
    fprintf (fp, "\t[VALUE]\n");
    
    /* Print current configuration, sorted by name */
    pos = arg_sort (defn, n);
    for (i = 0; i < n; i++) {
	j = pos[i];
	
	fprintf (fp, "%s", defn[j].name);
	for (l = strlen(defn[j].name); l < namelen; l += 8)
	    fprintf (fp, "\t");

	fprintf (fp, "\t");
	if (defn[j].deflt) {
	    fprintf (fp, "%s", defn[j].deflt);
	    l = strlen (defn[j].deflt);
	} else
	    l = 0;
	for (; l < deflen; l += 8)
	    fprintf (fp, "\t");
	
	fprintf (fp, "\t");
	if (doc) {
	    if (defn[j].doc)
		fprintf (fp, "%s", defn[j].doc);
	} else {
	    vp = cmd_ln_access (defn[j].name);
	    if (vp) {
		switch (defn[j].type) {
		case ARG_INT32:
		case REQARG_INT32:
		    fprintf (fp, "%d", *((int32 *) vp));
		    break;
		case ARG_FLOAT32:
		case REQARG_FLOAT32:
		    fprintf (fp, "%e", *((float32 *) vp));
		    break;
		case ARG_FLOAT64:
		case REQARG_FLOAT64:
		    fprintf (fp, "%e", *((float64 *) vp));
		    break;
		case ARG_STRING:
		case REQARG_STRING:
		    fprintf (fp, "%s", (char *)vp);
		    break;
		default: E_FATAL("Unknown argument type: %d\n", defn[j].type);
		}
	    }
	}
	
	fprintf (fp, "\n");
    }
    ckd_free (pos);
    
    fprintf (fp, "\n");
    fflush (fp);
}


int32 cmd_ln_parse (arg_t *defn, int32 argc, char *argv[])
{
    int32 i, j, n;
    
    if (argval)
	E_FATAL("Multiple sets of argument definitions not supported\n");
    
    /* Echo command line */
    E_INFO("Parsing command line:\n");
    for (i = 0; i < argc; i++) {
	if (argv[i][0] == '-')
	    fprintf (stderr, "\\\n\t");
	fprintf (stderr, "%s ", argv[i]);
    }
    fprintf (stderr, "\n\n");
    fflush (stderr);
    
    /* Find number of argument names in definition */
    for (n = 0; defn[n].name; n++);
    
    /* Allocate memory for argument values */
    ht = hash_new (n, 0 /* argument names are case-sensitive */);
    argval = (argval_t *) ckd_calloc (n, sizeof(argval_t));
    
    /* Enter argument names into hash table */
    for (i = 0; i < n; i++) {
	/* Associate argument name with index i */
	if (hash_enter (ht, defn[i].name, i) != i)
	    E_FATAL("Duplicate argument name: %s\n", defn[i].name);
    }
    
    /* Parse command line arguments (name-value pairs); skip argv[0] if argc is odd */
    for (j = 1; j < argc; j += 2) {
	if (hash_lookup(ht, argv[j], &i) < 0) {
	    E_ERROR("Unknown argument: %s\n", argv[j]);
	    cmd_ln_print_help (stderr, defn);
	    exit(-1);
	}
	
	/* Check if argument has already been parsed before */
	if (argval[i].ptr)
	    E_FATAL("Multiple occurrences of argument %s\n", argv[j]);
	
	if (j+1 >= argc) {
	    E_ERROR("Argument value for '%s' missing\n", argv[j]);
	    cmd_ln_print_help (stderr, defn);
	    exit(-1);
	}
	
	/* Enter argument value */
	if (arg_str2val (argval+i, defn[i].type, argv[j+1]) < 0) {
	    E_ERROR("Bad argument value for %s: %s\n", argv[j], argv[j+1]);
	    cmd_ln_print_help (stderr, defn);
	    exit(-1);
	}

	assert (argval[i].ptr);
    }
    
    /* Fill in default values, if any, for unspecified arguments */
    for (i = 0; i < n; i++) {
	if (! argval[i].ptr) {
	    if (arg_str2val (argval+i, defn[i].type, defn[i].deflt) < 0)
		E_FATAL("Bad default argument value for %s: %s\n",
			defn[i].name, defn[i].deflt);
	}
    }
    
    /* Check for required arguments; exit if any missing */
    j = 0;
    for (i = 0; i < n; i++) {
	if ((defn[i].type & ARG_REQUIRED) && (! argval[i].ptr)) {
	    E_ERROR("Missing required argument %s\n", defn[i].name);
	    j++;
	}
    }
    if (j > 0) {
	cmd_ln_print_help (stderr, defn);
	exit(-1);
    }
    
    /* Print configuration */
    fprintf (stderr, "Configuration in effect:\n");
    arg_dump (stderr, defn, 0);
    
    return 0;
}

int32 cmd_ln_parse_file(arg_t *defn, char *filename)
{
  FILE *file;
  char **tmp_argv;
  char **argv;
  int argc;
  int argv_size;

  char str[ARG_MAX_LENGTH];
  int len = 0;
  int ch;

  int rv = 0;

  if ((file = fopen(filename, "r")) == NULL) {
    return -1;
  }

  /*
   * initialize default argv, argc, and argv_size.  note that argv[0] is set to
   * a null-string.  basically we don't care about argv[0].  typically, that is
   * set as invoked program name.
   */
  argv_size = 10;
  argc = 1;
  argv = ckd_calloc(argv_size, sizeof(char *));
  argv[0] = ckd_calloc(1, sizeof(char *));
  argv[0][0] = '\0';

  do {
    ch = fgetc(file);
    if (ch == EOF || strchr(" \t\r\n", ch)) {
      /* reallocate argv so it is big enough to contain all the arguments */
      if (argc >= argv_size) {
	if (!(tmp_argv = ckd_calloc(argv_size * 2, sizeof(char *)))) {
	  rv = 1;
	  break;
	}
	memcpy(tmp_argv, argv, argv_size * sizeof(char *));
	ckd_free(argv);
	argv = tmp_argv;
	argv_size *= 2;
      }
      /* add the string to the list of arguments */
      argv[argc] = ckd_calloc(len + 1, sizeof(char));
      strncpy(argv[argc], str, len);
      argv[argc][len] = '\0';
      len = 0;
      argc++;

      for (; ch != EOF && strchr(" \t\r\n", ch); ch = fgetc(file));
      if (ch != EOF) {
	str[len++] = ch;
      }
    }
    else if (len < ARG_MAX_LENGTH) {
      /* add the char to the argument string */
      str[len++] = ch;
    }
    else {
      /* hmmm, we've had an argument that exceeded ARG_MAX_LENGTH */
      rv = 1;
      break;
    }
  } while (ch != EOF);
  
  if (rv == 0) {
    rv = cmd_ln_parse(defn, argc, argv);
  }

  ckd_free(argv);

  return rv;
}

void cmd_ln_print_help (FILE *fp, arg_t *defn)
{
    fprintf (fp, "Arguments list definition:\n");
    arg_dump (fp, defn, 1);
}


const void *cmd_ln_access (char *name)
{
    int32 i;
    
    if (! argval)
	E_FATAL("cmd_ln_access invoked before cmd_ln_parse\n");
    
    if (hash_lookup (ht, name, &i) < 0)
	E_FATAL("Unknown argument: %s\n", name);
    
    return (argval[i].ptr);
}

/* RAH, 4.17.01, free memory allocated above  */
void cmd_ln_free ()
{
  hash_free (ht);
  ckd_free ((void *) argval);

}
