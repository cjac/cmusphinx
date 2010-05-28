/* ====================================================================
 * Copyright (c) 1996-2005 Carnegie Mellon University.  All rights 
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
 */

/* Sphinx3 Context Free Grammar Parser
 *
 * The purpose here is to create a parser that can handle multiple input
 * streams at the same time.  The implementation is based on the Earley 
 * algorithm.
 *
 * The legal rules are in the form
 *
 *     0.33 $rule1 N product1 product2 ... productN
 *
 * 0.33 is a float32 indicating the score (or probability) of this rule being
 * applied.  $rule1 is the name of a non-terminal to be expanded.  And
 * product[1...N] is a string of (non-)terminals that $rule1 will expand to.
 * 
 * The rules are read from a file, if that is not obviously indicated by the
 * API.
 */

#ifndef _S3_CONTEXT_FREE_GRAMMAR_H
#define _S3_CONTEXT_FREE_GRAMMAR_H

#include <stdio.h>

#include <logmath.h>
#include "prim_type.h"
#include "hash_table.h"
#include "s3_arraylist.h"
#include "fsg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define S3_CFG_MAX_RULE_STR_LEN		1023
#define S3_CFG_MAX_ITEM_STR_LEN		40
#define S3_CFG_MAX_ITEM_COUNT		20
#define S3_CFG_INITIAL_RULE_COUNT	1
#define S3_CFG_INITIAL_RULE_SET_COUNT	50
#define S3_CFG_INITIAL_PARSE_SET_COUNT	20
#define S3_CFG_PARSE_HASH_SIZE		251
#define S3_CFG_INITIAL_STATE_SET_COUNT	20
#define S3_CFG_INITIAL_TERM_COUNT	50
#define S3_CFG_NAME_HASH_SIZE		4091

#define S3_CFG_INVALID_SCORE		1.0f
#define S3_CFG_INITIAL_SCORE		0.0f

#define S3_CFG_NONTERM_PREFIX		'$'
#define S3_CFG_TERM_BIT			0x80000000
#define S3_CFG_INDEX_MASK		0x7FFFFFFF

#define S3_CFG_INVALID_ID		0x7FFFFFFF

#define S3_CFG_PSTART_ITEM		0x00000000
#define S3_CFG_PSTART_ITEM_STR		"$PSTART"
#define S3_CFG_START_ITEM		0x00000001
#define S3_CFG_START_ITEM_STR		"$START"
#define S3_CFG_EOR_ITEM			(0x00000002 | S3_CFG_TERM_BIT)
#define S3_CFG_EOR_ITEM_STR		"#EOR#"
#define S3_CFG_EOI_ITEM			(0x00000003 | S3_CFG_TERM_BIT)
#define S3_CFG_EOI_ITEM_STR		"#EOI#"
#define S3_CFG_NIL_ITEM			(0x00000004 | S3_CFG_TERM_BIT)
#define S3_CFG_NIL_ITEM_STR		"#NIL#"

#define S3_CFG_START_RULE		\
  { S3_CFG_PSTART_ITEM, 0.0f, { S3_CFG_START_ITEM, S3_CFG_EOR_ITEM }, 1 }

#define S3_CFG_AUTO_PRUNE_SCORE		0x00000001
#define S3_CFG_AUTO_PRUNE_RANK		0x00000002

#define s3_cfg_is_null_parse(x) (x->entries.count == 0)

#define s3_cfg_is_terminal(x) (x & S3_CFG_TERM_BIT)

#define s3_cfg_id2index(x) (x & S3_CFG_INDEX_MASK)

typedef size_t s3_cfg_id_t;

typedef struct s3_cfg_rule_s {
  s3_cfg_id_t src;

  /* arbitrary floating point score */
  float32 score;
  /* normalized probability score */
  float32 prob_score; 
  /* probability fed to logs3 */
  int32 log_score;

  s3_cfg_id_t *products;
  int len;
} s3_cfg_rule_t;

typedef struct {
  s3_cfg_id_t id;
  char *name;
  s3_arraylist_t rules;
  s3_cfg_rule_t *nil_rule;
} s3_cfg_item_t;

struct s3_cfg_state_s;
typedef struct s3_cfg_entry_s {
  s3_cfg_rule_t *rule;
  int dot;
  struct s3_cfg_state_s *origin;
  int32 score;
  struct s3_cfg_entry_s *back;
  struct s3_cfg_entry_s *complete;
} s3_cfg_entry_t;


typedef struct s3_cfg_state_s {
  s3_cfg_id_t input;
  s3_arraylist_t entries;
  s3_arraylist_t expansions;
  struct s3_cfg_state_s *back;

  s3_cfg_entry_t *best_completed_entry;
  s3_cfg_entry_t *best_overall_entry;
  s3_cfg_entry_t *best_completed_parse;
  s3_cfg_entry_t *best_overall_parse;

  int num_expanded;
} s3_cfg_state_t;

typedef struct {
  s3_arraylist_t rules;
  s3_arraylist_t item_info;
  hash_table_t *name2id;

  int8 *predictions;
} s3_cfg_t;

/**
   Initialize a CFG parser.  The parser structure must be allocated outside
   the function.

   @param _cfg A (pre-allocated) CFG parser.
 */
void
s3_cfg_init(s3_cfg_t *_cfg);


/**
   Close a CFG parser and free its contents.  The parser structure is not
   freed (since it was not allocated by init).

   @param _cfg A (pre-allocated) CFG parser.
 */
void
s3_cfg_close(s3_cfg_t *_cfg);


/**
   Read a CFG from a plain-text file.  The parser structure must be freed
   afterwards.

   @param _fn Plain-text file.
   @return A CFG parser.
 */
S3DECODER_EXPORT
s3_cfg_t *
s3_cfg_read_simple(const char *_fn);


/**
   Read a CFG from a XML-based SRGS file.  The parser structure must be freed
   afterwards.  For more information on SRGS, visit
   http://www.w3.org/TR/speech-grammar/.

   @param _fn XML-based SRGS file.
   @return A CFG parser.
 */
s3_cfg_t *
s3_cfg_read_srgs(const char *_fn);


/**
   Write the CFG to a plain-text file.

   @param _cfg A CFG parser.
   @param _fn An output filename.
   @return 0 for success.  -1 for failure.
 */
void
s3_cfg_write_simple(s3_cfg_t *_cfg, const char *_fn);


/**
   Heuristically convert a CFG to a FSG by limiting non-terminal expansions.

   @param _cfg A CFG parser.
   @return A FSG.
 */
S3DECODER_EXPORT
s2_fsg_t *
s3_cfg_convert_to_fsg(s3_cfg_t *_cfg, int _max_expansion);


/*
 *
 */
void
s3_cfg_rescore(s3_cfg_t *_cfg, logmath_t *logmath);


/**
   Fetch information on a term.

   @param _cfg A CFG parser.
   @param _id A term id.
   @return Term information.
 */
s3_cfg_item_t *
s3_cfg_get_term_info(s3_cfg_t *_cfg, s3_cfg_id_t _id);


/**
   Start a CFG parse session.

   @param _cfg A CFG parser.
   @return A parse session.
 */
s3_cfg_state_t *
s3_cfg_create_parse(s3_cfg_t *_cfg);


/**
   Free a CFG parse session.

   @param _cfg A CFG parser.
   @param _parse A parse session.
 */
void
s3_cfg_free_parse(s3_cfg_t *_cfg, s3_cfg_state_t *_parse);


/*
 *
 */
void
s3_cfg_free_parse_tree(s3_cfg_t *_cfg, s3_cfg_state_t *_parse);


/**
   Continue a parse session by feeding it an input term.
 
   @param _cfg A CFG parser.
   @param _cur A parse session.
   @param _term An input term.  Must be a terminal.
 */
s3_cfg_state_t *
s3_cfg_input_term(s3_cfg_t *_cfg, s3_cfg_state_t *_cur, s3_cfg_id_t _term);


/**
   Add an expansion rule to the CFG parser.
   
   @param _cfg A CFG parser.
   @param _src The source of the expansion.  Must be a non-terminal.
   @param _fake_score Un-normalized transition scores.
   @param _products An array of expansion terms.  Terminated by
     S3_CFG_EOR_ITEM.
   @return A CFG expansion rule.
 */
s3_cfg_rule_t *
s3_cfg_add_rule(s3_cfg_t *_cfg, s3_cfg_id_t _src, float32 _fake_score, 
		s3_cfg_id_t *_products);


/**
   The consistency of the rule scores must be compiled before the parser can
   be used.

   @param _cfg A CFG parser.
 */
S3DECODER_EXPORT
void
s3_cfg_compile_rules(s3_cfg_t *_cfg, logmath_t *logmath);


/**
   Print a CFG expansion rule.

   @param _cfg A CFG parser.
   @param _rule A CFG expansion rule.
   @param _out An output stream.
 */
void
s3_cfg_print_rule(s3_cfg_t *_cfg, s3_cfg_rule_t *_rule, FILE *_out);


/**
   Print a CFG parse entry.  It is a single stage of a single rule expansion.
   For reference, lookup chart parsers.

   @param _cfg A CFG parser.
   @param _entry A CFG parse entry.
   @param _out An output stream.
 */
void
s3_cfg_print_entry(s3_cfg_t *_cfg, s3_cfg_entry_t *_entry, FILE *_out);


/**
   Print a parse session.  This will print all active entries in the parse,
   including unfinished parses.
   
   @param _cfg A CFG parser.
   @param _parse A parse session.
   @param _out An output stream.
 */
void
s3_cfg_print_parse(s3_cfg_t *_cfg, s3_cfg_entry_t *_parse, FILE *_out);


/**
   Convert a term string representation to a term id.  If the term does not
   already exist, assign it a new id.

   @param _cfg A CFG parser.
   @param _item A term string representation.
   @return A term id.
 */
s3_cfg_id_t
s3_cfg_str2id(s3_cfg_t *_cfg, char *_item);


/**
   Convert a term id to its original string representation.

   @param _cfg A CFG parser.
   @param _id A term id.
   @return A string representation of the term.
 */
const char *
s3_cfg_id2str(s3_cfg_t *_cfg, s3_cfg_id_t _id);

#ifdef __cplusplus
}
#endif
#endif

