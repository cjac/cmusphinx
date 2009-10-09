
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "s3_cfg.h"
#include "s3_arraylist.h"
#include "logs3.h"

#define FREE_VARIABLE(v) if (v != NULL) free(v);

#if 0
#define DEBUG_ENTRY(e) s3_cfg_print_entry(_cfg, e, stdout);printf("\n");
#else
#define DEBUG_ENTRY(e)
#endif


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void
eval_state(s3_cfg_t *_cfg, s3_cfg_state_t *_state);

static s3_cfg_entry_t *
add_entry(s3_cfg_state_t *_state, s3_cfg_rule_t *_rule, int _dot,
          s3_cfg_state_t *_origin, int32 _score,
          s3_cfg_entry_t *_back, s3_cfg_entry_t *_cmplt);

static s3_cfg_state_t *
add_state(s3_cfg_t *_cfg, s3_cfg_state_t *_back, s3_cfg_id_t _term);

static void
free_state(s3_cfg_state_t *_state);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static s3_cfg_item_t *
add_item(s3_cfg_t *_cfg, char *_name);

static void
print_parse(s3_cfg_t *_cfg, s3_cfg_entry_t *_parse, FILE *_out,
                        int _depth);

static void
compile_nonterm(s3_cfg_t *_cfg, s3_cfg_item_t *_item, logmath_t *logmath);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
s3_cfg_init(s3_cfg_t *_cfg)
{
  hash_table_t *name2id = NULL;
  s3_cfg_id_t start_products[] =
    { S3_CFG_START_ITEM, S3_CFG_EOI_ITEM, S3_CFG_EOR_ITEM };

  assert(_cfg != NULL);

  /****************************************************************************
   * Create a new CFG
   */

  s3_arraylist_init(&_cfg->rules);
  s3_arraylist_init(&_cfg->item_info);

  name2id = hash_table_new(S3_CFG_NAME_HASH_SIZE, HASH_CASE_YES);

  _cfg->name2id = name2id;
  _cfg->predictions = NULL;

  /****************************************************************************
   * Initialize constant (non-)terminals and rules
   */

  /* adding constant items to the mix
   *   $PSTART - pseudo starting non-terminal (for internal use only)
   *   $START - starting non-terminal
   *   EOR - end-of-rule terminal
   *   EOI - end-of-input terminal
   *   NIL - nil or epsilon terminal
   *
   * !!! NOTE: order here is important.  do NOT mix !!!
   */
  add_item(_cfg, S3_CFG_PSTART_ITEM_STR);
  add_item(_cfg, S3_CFG_START_ITEM_STR);
  add_item(_cfg, S3_CFG_EOR_ITEM_STR);
  add_item(_cfg, S3_CFG_EOI_ITEM_STR);
  add_item(_cfg, S3_CFG_NIL_ITEM_STR);

  /*  adding pseudo start rule to the mix
   *
   *    0.0 $PSTART -> $START #EOR#
   */
  s3_cfg_add_rule(_cfg, S3_CFG_PSTART_ITEM, 1.0f, start_products);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

s3_cfg_t *
s3_cfg_read_simple(const char *_fn)
{
  s3_cfg_t *cfg = NULL;
  FILE *file = NULL;
  s3_cfg_id_t src;
  s3_cfg_id_t products[S3_CFG_MAX_ITEM_COUNT + 1];
  s3_cfg_id_t item;
  char name[S3_CFG_MAX_ITEM_STR_LEN + 1];
  char format[1024];
  float32 score;
  int len;
  int i;

  assert(_fn != NULL);

  cfg = (s3_cfg_t *)ckd_calloc(1, sizeof(s3_cfg_t));
  s3_cfg_init(cfg);

  if ((file = fopen(_fn, "r")) == NULL)
    E_FATAL("Cannot open input plain cfg file");

  sprintf(format, "%%%ds", S3_CFG_MAX_ITEM_STR_LEN);

  while (!feof(file)) {
    /* read the prior */
    if (fscanf(file, "%f", &score) != 1 || score < 0)
      break;

    /* read the source */
    if (fscanf(file, format, name) != 1)
      E_FATAL("Bad CFG production rule\n");

    src = s3_cfg_str2id(cfg, name);
    if (src == S3_CFG_INVALID_ID)
      E_FATAL("Bad CFG production rule\n");

    if (s3_cfg_is_terminal(src))
      E_FATAL("Bad CFG production rule\n");
    
    if (fscanf(file, "%d", &len) != 1)
      E_FATAL("Bad CFG production rule\n");
    
    if (len > S3_CFG_MAX_ITEM_COUNT)
      E_FATAL("CFG Production rule too long\n");

    /* read the products */
    for (i = 0; i < len; i++) {
      if (fscanf(file, format, name) != 1)
        E_FATAL("Bad CFG production rule\n");

      item = s3_cfg_str2id(cfg, name);
      if (item == S3_CFG_INVALID_ID)
        E_FATAL("Bad CFG production term\n");
      products[i] = item;
    }
    products[len] = S3_CFG_EOR_ITEM;

    s3_cfg_add_rule(cfg, src, score, products);
  }

  fclose(file);

  return cfg;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
s3_cfg_write_simple(s3_cfg_t *_cfg, const char *_fn)
{
  FILE *file = NULL;
  s3_arraylist_t *rules = NULL;
  s3_cfg_rule_t *rule = NULL;
  int i, j, count;

  assert(_cfg != NULL);
  assert(_fn != NULL);

  if ((file = fopen(_fn, "w")) == NULL)
    E_FATAL("Failed to open output file for writing");

  rules = &_cfg->rules;
  count = s3_arraylist_count(rules);
  for (i = 1; i < count; i++) {
    rule = (s3_cfg_rule_t *)s3_arraylist_get(rules, i);
    fprintf(file, "%f %s %d",
            rule->score, s3_cfg_id2str(_cfg, rule->src), rule->len);
    for (j = 0; j < rule->len; j++)
      fprintf(file, " %s", s3_cfg_id2str(_cfg, rule->products[j]));
  }
  fprintf(file, "\n");
  
  fclose(file);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
s3_cfg_rescore(s3_cfg_t *_cfg, logmath_t *logmath)
{
  int i;
  s3_arraylist_t *rules = NULL;
  s3_cfg_rule_t *rule = NULL;

  assert(_cfg != NULL);

  rules = &_cfg->rules;
  for (i = s3_arraylist_count(rules) - 1; i >= 0; i--) {
    rule = (s3_cfg_rule_t *)s3_arraylist_get(rules, i);
    rule->log_score = logs3(logmath, rule->prob_score);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

s3_cfg_state_t *
s3_cfg_create_parse(s3_cfg_t *_cfg)
{
  s3_cfg_state_t *state = NULL;
  s3_cfg_rule_t *rule = NULL;
  
  assert(_cfg != NULL);

  add_state(_cfg, NULL, S3_CFG_NIL_ITEM);
  
  /* to initialize the parser, we need to create the root state and add to 
   * it the starting entry using the pseudo start rule
   *
   *   0.0 $PSTART -> $START #EOR#
   */
  rule = s3_arraylist_get(&_cfg->rules, 0);
  add_entry(state, rule, 0, 0, rule->log_score, NULL, NULL);
  
  eval_state(_cfg, state);

  return state;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

s3_cfg_state_t *
s3_cfg_input_term(s3_cfg_t *_cfg, s3_cfg_state_t *_cur, s3_cfg_id_t _term)
{
  int index;
  s3_cfg_state_t *state = NULL;
  
  assert(_cfg != NULL);

  index = s3_cfg_id2index(_term);
  state = (s3_cfg_state_t *)s3_arraylist_get(&_cur->expansions, index);

  if (state == NULL)
    return NULL;

  if (state->num_expanded == -1)
    eval_state(_cfg, state);

  return state;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
s3_cfg_close(s3_cfg_t *_cfg)
{
  int i;
  s3_cfg_rule_t *rule = NULL;
  s3_cfg_item_t *item = NULL;

  for (i = _cfg->rules.count - 1; i >= 0; i--) {
    rule = (s3_cfg_rule_t *)s3_arraylist_get(&_cfg->rules, i);
    free(rule->products);
    free(rule);
  }

  for (i = _cfg->item_info.count - 1; i >= 0; i--) { 
    item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, i);
    free(item->name);
    free(item); 
  } 

  if (_cfg->name2id != NULL)
    hash_table_free(_cfg->name2id); 
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
free_state(s3_cfg_state_t *_state)
{
  int i;
  s3_cfg_entry_t *entry = NULL;
  s3_cfg_state_t *parent = NULL;

  for (i = _state->entries.count - 1; i >= 0; i--) {
    entry = (s3_cfg_entry_t *)s3_arraylist_get(&_state->entries, i);
    free(entry);
  }

  parent = _state->back;
  i = s3_cfg_id2index(_state->input);

  s3_arraylist_close(&_state->entries);
  s3_arraylist_close(&_state->expansions);
  free(_state);

  if (parent != NULL) {
    parent->num_expanded--;
    s3_arraylist_set(&parent->expansions, i, NULL);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
free_parse(s3_cfg_state_t *_parse)
{
  int i;
  s3_cfg_state_t *scan = NULL;

  if (_parse->num_expanded > 0) {
    for (i = s3_arraylist_count(&_parse->expansions) - 1; i >= 0; i--) {
      scan = (s3_cfg_state_t *)s3_arraylist_get(&_parse->expansions, i);
      free_parse(scan);
    }
  }

  free_state(_parse);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
s3_cfg_free_parse(s3_cfg_t *_cfg, s3_cfg_state_t *_parse)
{
  s3_cfg_state_t *root = NULL;

  assert(_cfg != NULL);
  assert(_parse != NULL);

  root = _parse;
  while (root->back != NULL)
    root = root->back;
  free_parse(root);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

s3_cfg_id_t
s3_cfg_str2id(s3_cfg_t *_cfg, char *_name)
{
  void *id;
  char term[S3_CFG_MAX_ITEM_STR_LEN + 1];
  int start, end;

  assert(_cfg != NULL);
  assert(_name != NULL);

  for (start = 0; start < S3_CFG_MAX_ITEM_STR_LEN; start++) 
    if (strchr(" \t\r\n", _name[start]) == NULL)
      break;
  for (end = start; end < S3_CFG_MAX_ITEM_STR_LEN; end++) 
    if (strchr(" \t\r\n", _name[end]) != NULL)
      break;

  if (end - start >= (S3_CFG_MAX_ITEM_STR_LEN))
    return S3_CFG_INVALID_ID;
  strncpy(term, _name + start, end - start + 1);

  /* if hash lookup for item name succeeded, we return the id associated with
   * the name */
  if (hash_table_lookup(_cfg->name2id, term, &id) == 0)
    return (s3_cfg_id_t)id;
  else
    return add_item(_cfg, term)->id;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

const char *
s3_cfg_id2str(s3_cfg_t *_cfg, s3_cfg_id_t _id)
{
  assert(_cfg != NULL);

  return ((s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info,
                                             s3_cfg_id2index(_id)))->name;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static s3_cfg_entry_t *
add_entry(s3_cfg_state_t *_state,
          s3_cfg_rule_t *_rule,
          int _dot,
          s3_cfg_state_t *_origin,
          int32 _score,
          s3_cfg_entry_t *_back,
          s3_cfg_entry_t *_complete)
{
  s3_cfg_entry_t *entry = NULL;

  assert(_state != NULL);
  assert(_rule != NULL);

  entry = (s3_cfg_entry_t *)ckd_calloc(1, sizeof(s3_cfg_entry_t));
  entry->rule = _rule;
  entry->dot = _dot;
  entry->origin = _origin;
  entry->score = _score;
  entry->back = _back;
  entry->complete = _complete;

  s3_arraylist_append(&_state->entries, entry);

  return entry;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static s3_cfg_state_t *
add_state(s3_cfg_t *_cfg, s3_cfg_state_t *_back, s3_cfg_id_t _term)
{
  s3_cfg_state_t *state = NULL;

  assert(_cfg != NULL);

  state = (s3_cfg_state_t *)ckd_calloc(1, sizeof(s3_cfg_state_t));
  s3_arraylist_init(&state->entries);
  s3_arraylist_init(&state->expansions);
  state->input = _term;
  state->back = _back;
  state->best_completed_entry = NULL;
  state->best_overall_entry = NULL;
  state->best_completed_parse = NULL;
  state->best_overall_parse = NULL;
  state->num_expanded = -1;
  if (_back != NULL)
    s3_arraylist_set(&_back->expansions, s3_cfg_id2index(_term), state);

  return state;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void
eval_state(s3_cfg_t *_cfg, s3_cfg_state_t *_state)
{
  s3_cfg_rule_t *rule = NULL;
  s3_cfg_entry_t *entry = NULL;
  s3_cfg_entry_t *cmplt_entry = NULL;
  s3_cfg_state_t *target_state = NULL;
  s3_cfg_state_t *origin_state = NULL;
  s3_cfg_item_t *item = NULL;
  s3_cfg_id_t scan;
  s3_arraylist_t *arraylist = NULL;
  int8 *predictions = NULL;
  int32 score;
  int index;
  int dot;
  int i, j;

  assert(_cfg != NULL);
  assert(_state != NULL);

  if (_state->back != NULL) {
    _state->back->num_expanded++;
  }
  _state->num_expanded = 0;

  predictions = _cfg->predictions;
  memset(predictions, 0, _cfg->item_info.count * sizeof(int8));

  /* iterate thru the entries in the state and perform prediction, scan,
   * and completion steps */
  for (i = 0; i < _state->entries.count; i++) {
    entry = (s3_cfg_entry_t *)s3_arraylist_get(&_state->entries, i);
    rule = entry->rule;
    dot = entry->dot;
    origin_state = entry->origin;
    score = entry->score;
    
    scan = rule->products[dot];
    index = s3_cfg_id2index(scan);

    DEBUG_ENTRY(entry);

    item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);

    /* saving some scores */
    if (_state->best_overall_entry == NULL ||
        score < _state->best_overall_entry->score)
      _state->best_overall_entry = entry;
    
    if (_state->best_overall_parse == NULL ||
        score < _state->best_overall_parse->score)
      _state->best_overall_parse = entry;

    if (s3_cfg_is_terminal(scan)) {
      /************************************************************************
       * NORMAL COMPLETION
       *
       * When we encounter an entry of the form
       *
       *   $X -> (A * #EOR#, s0, i),
       *
       * we look for any entry in state S(i) of the form
       *
       *   $Z -> (A * $X B #EOR#, s1, j)
       *
       * and add the entry
       *
       *   $Z -> (A $X * B #EOR#, s1 + s2, j)
       *
       * to the current state.  We also need to keep a record of which
       * subparses were used to complete entries.  In this case, we need to
       * remember that this particular completed entry of $X is used to
       * advance the parsing of $Z.  In this case, the pointer p1 is added to
       * the entry
       *
       *   $Z -> (A $X(p1) * B #EOR#, s1 + s2, j)
       *
       * for records keeping sake.
       */
      if (scan == S3_CFG_EOR_ITEM) {
        scan = entry->rule->src;
        arraylist = &entry->origin->entries;

        for (j = s3_arraylist_count(arraylist) - 1; j >= 0; j--) {
          cmplt_entry = (s3_cfg_entry_t *)s3_arraylist_get(arraylist, j);

          if (cmplt_entry->rule->products[cmplt_entry->dot] == scan)
            add_entry(_state,
                      cmplt_entry->rule,
                      cmplt_entry->dot + 1,
                      cmplt_entry->origin,
                      cmplt_entry->score + entry->score,
                      cmplt_entry,
                      entry);
        }
      }
      /************************************************************************
       * PARSE COMPLETION
       *
       * We encountered an entry of the form
       *
       *   ($PSTART -> $START * #EOI#, s i).
       *
       * Instead of waiting for an input symbol #EOI# and completing the
       * pseudo-start rule in the next state, we finish the parse here and save
       * us a step.  We do need to check against other completed parses in this
       * state, since only the parse with the highest score is kept.
       */
      else if (scan == S3_CFG_EOI_ITEM) {
        if (_state->best_completed_entry == NULL ||
            score < _state->best_completed_entry->score)
          _state->best_completed_entry = entry;

        if (_state->best_completed_parse == NULL ||
            score < _state->best_completed_parse->score)
          _state->best_completed_parse = entry;

      }
      /************************************************************************
       * NORNAL SCANNING
       *
       * When we encounter an entry of the form
       *
       *   ($X -> A * y B #EOR#, s, i),
       *
       * and the input symbol/terminal is y, we add to the next state the entry
       *
       *   ($X -> A y * B #EOR#, s, i)
       */
      else {
        index = s3_cfg_id2index(scan);
        arraylist = &_state->expansions;
        target_state = (s3_cfg_state_t *)s3_arraylist_get(arraylist, index);
        if (target_state == NULL)
          target_state = add_state(_cfg, _state, scan);
        add_entry(target_state, rule, dot + 1, origin_state, score,
                  entry, NULL);
      }
    }
    else {
      /************************************************************************
       * AUTOMATIC COMPLETION OF EPSILON PRODUCING NON-TERMINALS
       *
       * When we encounter an entry of the form
       *
       *   ($X -> A * $Y B #EOR#, s0, i),
       *
       * we check whether $Y is a epsilon producing non-terminal, i.e.,
       * whether the rule
       * 
       *   $Y -> #EOR#
       * 
       * exists.  If that is the case, we do not add any entry corresponding to
       * such epsilon producing rule.  Instead, we take a short-cut by add the
       * following entry to the current state
       *   
       *   ($X -> A $Y(null) * B  #EOR#, s0 + s1, i).
       *
       * Note in this new entry, the completed non-terminal $Y has a NULL sub-
       * parse pointer.
       */
      if (item->nil_rule != NULL)
        add_entry(_state, rule, dot + 1, origin_state,
                  score + item->nil_rule->log_score, entry, NULL);

      /************************************************************************
       * NORMAL PREDICTION
       * 
       * When we encounter an entry of the form
       * 
       *   ($X -> A * $Y B #EOR#, s0, i),
       *
       * we want to expand the non-terminal $Y.  That is, we add an entry for
       * each rule that has $Y on its left-hand side.  However, we don't want
       * to keep repeated copies of the same entries, so we keep track of which
       * non-terminals we've already expanded in a table.
       */
      if (!predictions[index]) {
        predictions[index] = 1;
        arraylist = &item->rules;
        for (j = s3_arraylist_count(arraylist) - 1; j >= 0; j--) {
          rule = (s3_cfg_rule_t *)s3_arraylist_get(arraylist, j);
          if (rule->products[0] != S3_CFG_EOR_ITEM)
            add_entry(_state, rule, 0, _state, rule->log_score, NULL, NULL);
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static s3_cfg_item_t *
add_item(s3_cfg_t *_cfg, char *_name)
{
  s3_cfg_item_t *item = NULL;
  char *name = NULL;
  int index;

  assert(_cfg != NULL);
  assert(_name != NULL);

  index = s3_arraylist_count(&_cfg->item_info);

  item = (s3_cfg_item_t *)ckd_calloc(1, sizeof(s3_cfg_item_t));
  name = (char *)ckd_salloc(_name);
  
  s3_arraylist_init(&item->rules);
  
  /* create item's new id */
  item->id = (name[0] == S3_CFG_NONTERM_PREFIX ? 0 : S3_CFG_TERM_BIT) | index;
  item->name = name;
  item->nil_rule = NULL;
  
  hash_table_enter(_cfg->name2id, name, (void *)item->id);

  s3_arraylist_set(&_cfg->item_info, index, item);
  
  return item;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

s3_cfg_rule_t *
s3_cfg_add_rule(s3_cfg_t *_cfg, s3_cfg_id_t _src, float32 _score, 
                s3_cfg_id_t *_products)
{
  s3_cfg_rule_t *rule = NULL;
  s3_cfg_id_t *products = NULL;
  s3_cfg_item_t *item = NULL;
  int len = 0;
  int index;

  assert(_cfg != NULL);
  assert(_products != NULL);

  /****************************************************************************
   * Create a new rule
   */
  index = s3_cfg_id2index(_src);
  for (len = 0; len < S3_CFG_MAX_ITEM_COUNT; len++)
    if (_products[len] == S3_CFG_EOR_ITEM)
      break;
  if (_products[len] != S3_CFG_EOR_ITEM)
    E_FATAL("CFG Production rule does not contain EOR item");

  rule = (s3_cfg_rule_t *)ckd_calloc(1, sizeof(s3_cfg_rule_t));
  products = (s3_cfg_id_t *)ckd_calloc(len + 1, sizeof(s3_cfg_id_t));
  memcpy(products, _products, (len + 1) * sizeof(s3_cfg_id_t));
  
  rule->src = _src;
  rule->score = _score;
  rule->products = products;
  rule->len = len;

  /****************************************************************************
   * Add the new rule to the CFG
   */

  s3_arraylist_append(&_cfg->rules, rule);

  item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);

  if (len > 0)
    s3_arraylist_append(&item->rules, rule);
  else if (item->nil_rule == NULL || item->nil_rule->score < _score)
    item->nil_rule = rule;

  return rule;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#if 0
static void
read_1rule(s3_cfg_t *_cfg, FILE *_file, float32 *_score,
           s3_cfg_id_t *_src, s3_cfg_id_t *_products)
{
  char name[S3_CFG_MAX_ITEM_STR_LEN + 1];
  float32 score;
  s3_cfg_id_t src;
  s3_cfg_id_t products[S3_CFG_MAX_ITEM_COUNT + 1];
  s3_cfg_id_t item;
  char format[1024];
  int len;
  int i;

  assert(_cfg != NULL);
  assert(_file != NULL);

  sprintf(format, "%%%ds", S3_CFG_MAX_ITEM_STR_LEN);

  /* read the prior */
  if (fscanf(_file, "%f", &score) != 1 || score < 0)
    E_FATAL("Bad CFG production rule\n");

  /* read the source */
  if (fscanf(_file, format, name) != 1)
    E_FATAL("Bad CFG production rule\n");

  src = s3_cfg_str2id(_cfg, name);
  if (src == S3_CFG_INVALID_ID)
    E_FATAL("Bad CFG production rule\n");

  if (s3_cfg_is_terminal(src))
    E_FATAL("Bad CFG production rule\n");

  if (fscanf(_file, "%d", &len) != 1)
    E_FATAL("Bad CFG production rule\n");

  if (len > S3_CFG_MAX_ITEM_COUNT)
    E_FATAL("CFG Production rule too long\n");

  /* read the products */
  for (i = 0; i < len; i++) {
    if (fscanf(_file, format, name) != 1)
      E_FATAL("Bad CFG production rule\n");

    item = s3_cfg_str2id(_cfg, name);
    if (item == S3_CFG_INVALID_ID)
      E_FATAL("Bad CFG production term\n");
    products[i] = item;
  }
  products[len] = S3_CFG_EOR_ITEM;

  *_src = src;
  *_score = score;
  memcpy(_products, products, (len + 1) * sizeof(s3_cfg_id_t));

}
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


void
s3_cfg_print_rule(s3_cfg_t *_cfg, s3_cfg_rule_t *_rule, FILE *_out)
{
  s3_cfg_item_t *item = NULL;
  int index, len, i;

  assert(_cfg != NULL);
  assert(_rule != NULL);

  index = s3_cfg_id2index(_rule->src);
  item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);
  
  fprintf(_out, "(%s -> ", item->name);

  for (i = 0, len = _rule->len; i < len; i++) {
    index = s3_cfg_id2index(_rule->products[i]);
    item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);

    fprintf(_out, "%s", item->name);
    if (i != len - 1)
      fprintf(_out, " ");
  }
  
  fprintf(_out, ", %.3f)", _rule->prob_score);
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


void
s3_cfg_print_entry(s3_cfg_t *_cfg, s3_cfg_entry_t *_entry, FILE *_out)
{
  s3_cfg_item_t *item = NULL;
  s3_cfg_rule_t *rule = NULL;
  int index;
  int dot;
  int i;

  assert(_cfg != NULL);
  assert(_entry != NULL);

  rule = _entry->rule;
  dot = _entry->dot;

  index = s3_cfg_id2index(rule->src);
  item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);

  fprintf(_out, "(%s -> ", item->name);

  for (i = 0; i < rule->len; i++) {
    if (i == dot)
      fprintf(_out, "* ");
    
    index = s3_cfg_id2index(rule->products[i]);
    item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);

    fprintf(_out, "%s", item->name);
    fprintf(_out, " ");
  }

  if (dot == rule->len)
    fprintf(_out, "*, %d)", _entry->score);
  else
    fprintf(_out, ", %d)", _entry->score);
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


static void
print_parse(s3_cfg_t *_cfg, s3_cfg_entry_t *_parse, FILE *_out,
                        int _depth)
{
  int i;

  assert(_cfg != NULL);
  assert(_parse != NULL);

  if (_parse->back != NULL)
    print_parse(_cfg, _parse->back, _out, _depth);

  if (_parse->dot == 0) {
    for (i = _depth - 1; i >= 0; i--)
      fprintf(_out, "> ");
    s3_cfg_print_rule(_cfg, _parse->rule, _out);
    fprintf(_out, "\n");
  }

  if (_parse->complete != NULL)
    print_parse(_cfg, _parse->complete, _out, _depth + 1);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


void
s3_cfg_print_parse(s3_cfg_t *_cfg, s3_cfg_entry_t *_parse, FILE *_out)
{
  print_parse(_cfg, _parse, _out, 0);
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void
s3_cfg_compile_rules(s3_cfg_t *_cfg, logmath_t *logmath)
{
  s3_cfg_item_t *item = NULL;
  s3_arraylist_t *arraylist = NULL;
  int i, n;

  assert(_cfg != NULL);

  arraylist = &_cfg->item_info;
  n = s3_arraylist_count(arraylist);
  for (i = n - 1; i >= 0; i--) {
    item = s3_arraylist_get(arraylist, i);
    if (!s3_cfg_is_terminal(item->id))
      compile_nonterm(_cfg, item, logmath);
  }

  _cfg->predictions = (int8 *)ckd_calloc(n, sizeof(int8));
}



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void
compile_nonterm(s3_cfg_t *_cfg, s3_cfg_item_t *_item, logmath_t *logmath)
{
  int i, n;
  s3_arraylist_t *arraylist;
  float32 sum = 0;
  s3_cfg_rule_t *rule;

  assert(_cfg != NULL);
  assert(_item != NULL);

  /* calculate fake score sum */
  arraylist = &_item->rules;
  n = s3_arraylist_count(arraylist);
  for (i = n - 1; i >= 0; i--) {
    rule = (s3_cfg_rule_t *)s3_arraylist_get(arraylist, i);
    sum += rule->score;
  }

  if (_item->nil_rule != NULL)
    sum += _item->nil_rule->score;

  if (sum == 0)
    E_FATAL("CFG production rule scores cannot sum to 0\n");

  /* calculate probability and log score */
  for (i = n - 1; i >= 0; i--) {
    rule = (s3_cfg_rule_t *)s3_arraylist_get(arraylist, i);
    rule->prob_score = rule->score / sum;
    rule->log_score = logs3(logmath, rule->prob_score);
  }

  if (_item->nil_rule != NULL) {
    _item->nil_rule->prob_score = _item->nil_rule->score / sum;
    _item->nil_rule->log_score = logs3(logmath, _item->nil_rule->prob_score);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

