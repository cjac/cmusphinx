/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
/* confidence.h: Calculate confidence scores from word lattices using
 * posterior word probabilities and backoff.
 *
 * Copyright (c) 2004 Carnegie Mellon University.
 * 
 * Author: Rong Zhang <rongz@cs.cmu.edu>
 *
 * Arthur Chan has significantly changed these routines when incorporating to Sphinx 3.x
 *
 * $Log$
 * Revision 1.2  2006/02/21  18:31:09  arthchan2003
 * Merge confidence.c confidence.h and Makefile.am into the trunk.
 * 
 * Revision 1.1.2.1  2006/01/16 18:38:25  arthchan2003
 * Adding Rong's confidence routine.  Compare to Rong's routine, this routine used Sphinx3's njmerical routines and make the chances of backward-forward scores inconsistency to be lower.
 *
 */

#ifndef __CONFIDENCE_H__
#define __CONFIDENCE_H__


#include <stdio.h>
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "s3types.h"
#include "logs3.h"
#include "srch_output.h"

#define CONFIDENCE_MAX_INT 2147483640
#define CONFIDENCE_MIN_INT -2147483640
#define CONFIDENCE_FAILURE 0
#define CONFIDENCE_SUCCESS 1
#define MAGIC_CONFIDENCE_CONSTANT 39.5
#define MIN_LOG	-690810000


/**
   \struct structure specific for node of DAG for confidence
   annotation.
   
 */
typedef struct ca_dagnode_type {
  char word[64]; /**< The word string */
  int wid; /**< The dictionary word ID */
  int seqid; /**< The sequence ID */
  int sf; /**< Start frame */
  int fef; /**< First end frame */
  int lef;  /**< Last end frame */
  int reachable; /**< Whether the node is reachable */
  int visited; /**< Whether it is visited already */
  int fanin; /**<  The# Fan-in */
  int fanout; /**< The# Fan-out */
  int hscore; /**< The heuristic score */
  int pscore; 
  int lscore; /**< The language score */
  int cscore; /**< The confidence score */
  struct ca_daglink_type *succlist;
  struct ca_daglink_type *predlist;
  struct ca_dagnode_type *alloc_next;
} ca_dagnode;

typedef struct ca_daglink_type {
  ca_dagnode *from; /**< The from node */
  ca_dagnode *to; /**<  The to node */
  int ascore;  /**< The acoustic score */
  int alpha_beta; /**< The alpha-beta value. (An intermediate value of
		     computing word posterior probability) */
  struct ca_daglink_type *next;
} ca_daglink;

typedef struct ca_dag_type {
	ca_dagnode *nodelist;
	ca_dagnode **seqidtonode;
	ca_daglink entry;
	ca_daglink exit;
	int nfrm;
	int nnode;
	int nedge;
} ca_dag;


/**
   Compute word posterior probability given a hypotheiss and the
   corresponding DAG. 
 */
int32 confidence_word_posterior(char* dagfile,  /**< The file name of the DAG */
				seg_hyp_line_t *seg_hyp_line,  /**< a pointer of a seg_hyp_line_t */
				char* uttid,  /**< The utterance ID */
				lm_t *lm,   /**< An LM */
				dict_t *dict,  /**< Dictionary */
				fillpen_t *fpen /**< Filler penalty structure */
				);


#if 0 /* Not public */
/**
   Load a lattice which is specific for computing confidence score. 
 */
int ca_dag_load_lattice(char *filename, /**<File name for the lattice */
			ca_dag *word_lattice,  /**< The lattice structure */
			lm_t *lm,    /**< The LM */
			dict_t *dict, /**< The dictionary */
			fillpen_t* fillpen /**< The filler penalty structure */
			);

/**
   Given a lattice, compute the word posterior probability using the alpha-beta like algorithm
 */
int alpha_beta(ca_dag *word_lattice, /**< In: a word lattice */
	       lm_t* lm,  /**< In: a LM */
	       dict_t* dict /**< In: a dictionary */
	       );

/*
  Given a seg_hyp_line, i.e. one hypothesis with time
  information. Compute the posterior word probability score using the
  lattice. 
 */
int pwp(seg_hyp_line_t *seg_hyp_line,  /**< In: a input seg_hyp_line, which has a conf_srch_hyp_t */
	ca_dag *word_lattice  /**< In: a word lattice */
	);

/**
   Free the lattice 
 */
int ca_dag_free_lattice(ca_dag *word_lattice);

#endif

/**
   Compute the LM type of (Need more documents) 
 */
int compute_lmtype(seg_hyp_line_t *seg_hyp_line,  /**< A seg_hyp line*/
		   lm_t* lm,
		   dict_t* dict
		   );

int compute_combined_lmtype(seg_hyp_line_t *seg_hyp_line);

#endif /* __CONFIDENCE_H__ */

