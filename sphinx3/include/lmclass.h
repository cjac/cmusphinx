/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
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
 * lmclass.h -- Class-of-words objects in language models.
 * 
 * HISTORY
 * $Log$
 * Revision 1.1  2006/04/05  20:27:30  dhdfu
 * A Great Reorganzation of header files and executables
 * 
 * Revision 1.8  2006/02/23 04:22:34  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Fixed  dox-doc.
 *
 * Revision 1.7.4.1  2005/07/13 01:26:23  arthchan2003
 * Fixed dox-doc.
 *
 * Revision 1.7  2005/06/21 22:25:04  arthchan2003
 * Added  keyword.
 *
 * Revision 1.1  2005/05/04 06:08:07  archan
 * Refactor all lm routines except fillpen.c into ./libs3decoder/liblm/ . This will be equivalent to ./lib/liblm in future.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 19-Feb-2004  A Chan (archan@cs.cmu.edu) at Carnegie Mellon University
 * 		copied from sphinx 2 code base. 
 * 24-Mar-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#ifndef _S3_LMCLASS_H_
#define _S3_LMCLASS_H_

#include <stdio.h>

#include <logmath.h>
#include "s3types.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif

/** \file lmclass.h
 * \brief Language model class modules. 
 * This module maintains classes of words and associated probabilities (P(word | class)).
 * Examples of such classes: days of week, months of year, digits, last names, etc.
 * Restrictions:
 *   - Classes cannot be nested; all classes are top-level classes.
 *   - Contents of classes are individual words.  Use "compound words" for phrases.
 * By convention, class names begin and end with [ and ], respectively.  Also, class
 * names are CASE-SENSITIVE.
 */


/** \struct lmclass_word_t
 * \brief A single word in an LM class.
 */
typedef struct lmclass_word_s {
    char *word;		/**< The word string */
    int32 dictwid;	/**< Dictionary word id; NOT filled in by this module, but by
			   the application if desired */
    int32 LOGprob;	/**< Conditional (LOG)probability of this word, given the class */
    struct lmclass_word_s *next;	/**< For linking together words in this LM class,
					   in no particular order. */
} lmclass_word_t;


/** \struct lmclass_t
 * \brief An LM class object.
 */
typedef struct lmclass_s {
    char *name;			/**< Name for this LM class */
    lmclass_word_t *wordlist;	/**< Head of list of words in this class */
    struct lmclass_s *next;	/**< For linking together multiple LM classes in the
				   application, in no particular order. */
} lmclass_t;


/** \struct lmclass_set_t
 * \brief Collection of LM classes.  Most applications would use multiple classes.  This data
 * type is provided as a convenience for maintaining several such classes.
 */
typedef struct lmclass_set_s {
    lmclass_t *lmclass_list;	/**< Head of list of LM classes in this module */
} lmclass_set_t;


/** Initialize and return a new, empty LMclass set */
lmclass_set_t *lmclass_newset ( void );

void lmclass_free(lmclass_t *lmclass);


/**
 * Load LM classes defined in the given file into the given set, and return the new,
 * updated set.  Note that the input file can contain several class definitions.
 * File format:
 * 
 *    LMCLASS <classname1>
 * 	<word1> [<prob1>]
 * 	<word2> [<prob2>]
 * 	... (each word, and its associated probability, in one line)
 *    END <classname1>
 *    LMCLASS <classname2>
 *      ...
 *    END <classname2>
 *    ... (as many classes as desired)
 * 
 * By convention, classname strings begin and end with [ and ] (Roni Rosenfeld).
 * But it is not a REQUIREMENT as far as this module is concerned.
 * 
 * Word probabilities are optional (as indicated by the [] above).  If p = sum of all
 * EXPLICITLY specified probs within a class, (1-p) will be uniformly distributed
 * between the remaining words in that class.
 * 
 * Lines beginning with a # IN THE FIRST COLUMN are comments and are ignored.
 */
lmclass_set_t *lmclass_loadfile (lmclass_set_t *lmclass_set,  /**< An lm class set */
                                 char *file, /**< A class definition file */
                                 logmath_t *logmath
    );


/**
 * Get the LMclass object for the given name from the given set.
 */
lmclass_t *lmclass_get_lmclass (lmclass_set_t *set, char *name);


/**
 * Get the number of LMclass objects in the given set.
 */
int32 lmclass_get_nclass (lmclass_set_t *set);


/** Set the dictwid field of the given LMclass word entry to the given value */
void lmclass_set_dictwid (lmclass_word_t *w, int32 dictwid);


/** Various access functions (macros) */
#define lmclass_getname(class)		((class)->name)
#define lmclass_firstword(class)	((class)->wordlist)
#define lmclass_nextword(class,w)	((w)->next)
#define lmclass_getwid(w)		((w)->dictwid)
#define lmclass_getword(w)		((w)->word)
#define lmclass_getprob(w)		((w)->LOGprob)
#define lmclass_isclass(cl)		((cl) != NULL)
#define lmclass_isword(w)		((w) != NULL)
#define lmclass_firstclass(set)		((set)->lmclass_list)
#define lmclass_nextclass(set,cl)	((cl)->next)


void lmclass_dump (lmclass_t *cl, FILE *fp);
void lmclass_set_dump (lmclass_set_t *set, FILE *fp);

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
