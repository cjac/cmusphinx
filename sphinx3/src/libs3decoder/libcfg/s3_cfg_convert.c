
#include <sphinxbase/libutil.h>

#include "s3_cfg.h"
#include "fsg.h"

static void
prune_states(s2_fsg_t *_fsg);

static void
mark_dead_state(s2_fsg_t *_fsg, int _state, int *_marks, glist_t *_adj);

static void
convert_cfg_rule(s3_cfg_t *_cfg,
		 s2_fsg_t *_fsg,
		 s3_cfg_rule_t *_rule,
		 int _src,
		 int _dest,
		 int *_expansions,
		 int _max_expansion);

s2_fsg_t *
s3_cfg_convert_to_fsg(s3_cfg_t *_cfg, int _max_expansion)
{
  s3_cfg_rule_t *rule;
  s2_fsg_t *fsg;
  int *expansions;
  int i, n;

  assert(_cfg != NULL);

  n = s3_arraylist_count(&_cfg->item_info);
  rule = s3_arraylist_get(&_cfg->rules, 0);

  expansions = (int *)ckd_calloc(n, sizeof(int));
  fsg = (s2_fsg_t *)ckd_calloc(1, sizeof(s2_fsg_t));
  fsg->name = NULL;
  fsg->n_state = 2;
  fsg->start_state = 0;
  fsg->final_state = 1;
  fsg->trans_list = NULL;

  for (i = 0; i < n; i++)
    expansions[i] = 0;
  convert_cfg_rule(_cfg, fsg, rule, 0, 1, expansions, _max_expansion);

  prune_states(fsg);

  return fsg;
}

static void
convert_cfg_rule(s3_cfg_t *_cfg,
		 s2_fsg_t *_fsg,
		 s3_cfg_rule_t *_rule,
		 int _src,
		 int _dest,
		 int *_expansions,
		 int _max_expansion)
{
  int index;
  int i, j, n;
  int cur, u, v;
  s3_cfg_id_t id;
  s3_cfg_item_t *item;
  s3_cfg_rule_t *rule;
  s2_fsg_trans_t *trans;

  cur = _src;

  /* Check whether the target rule has any variables that exceeded the
   * expansion count
   */
  for (i = 0; i < _rule->len; i++) {
    id = _rule->products[i];
    if (_expansions[s3_cfg_id2index(id)] > _max_expansion)
      return;
  }

  /* Iterate through the production variables. */
  for (i = 0; i < _rule->len; i++) {
    id = _rule->products[i];

    /* For each terminal:
     *   1.  Create a new state.
     *   2.  Add a single definite transition from the current state to the
     *       new state that emits the terminal.
     *   3.  Use the new state as the current state.
     */
    if (s3_cfg_is_terminal(id)) {
      if (id != S3_CFG_EOI_ITEM) {
	trans = (s2_fsg_trans_t*)ckd_calloc(1, sizeof(s2_fsg_trans_t));
	trans->from_state = cur;
	trans->to_state = _fsg->n_state;
	trans->prob = 1.0;
	trans->word = (char *)ckd_salloc(s3_cfg_id2str(_cfg, id));
	trans->next = _fsg->trans_list;
	_fsg->trans_list = trans;
	
	cur = _fsg->n_state++;
      }
    }

    /* For each non-terminal X:
     *   1.  Create a new destination state, v.
     *   2.  Increment expansion count for X.
     *   3.  For each (non-epsilon) expansion rule with X as source:
     *      a.  Create a new source state u
     *      b.  Convert the rule with u as src and v as dest.
     *      c.  Create a new epsilong transition from cur to u with the rule's
     *          expansion probability.
     *   4.  Set the current state to v.
     *   5.  Decrement expansion count for X.
     */
    else {
      index = s3_cfg_id2index(id);
      v = _fsg->n_state++;
      _expansions[index]++;
      item = (s3_cfg_item_t *)s3_arraylist_get(&_cfg->item_info, index);
      n = s3_arraylist_count(&item->rules);
      for (j = 0; j < n; j++) {
	rule = (s3_cfg_rule_t *)s3_arraylist_get(&item->rules, j);
	u = _fsg->n_state++;
	convert_cfg_rule(_cfg, _fsg, rule, u, v, _expansions, _max_expansion);
	trans = (s2_fsg_trans_t *)ckd_calloc(1, sizeof(s2_fsg_trans_t));
	trans->from_state = cur;
	trans->to_state = u;
	trans->prob = rule->prob_score;
	trans->word = NULL;
	trans->next = _fsg->trans_list;
	_fsg->trans_list = trans;
      }

      if (item->nil_rule != NULL) {
	trans = (s2_fsg_trans_t *)ckd_calloc(1, sizeof(s2_fsg_trans_t));
	trans->from_state = cur;
	trans->to_state = v;
	trans->prob = item->nil_rule->prob_score;
	trans->word = NULL;
	trans->next = _fsg->trans_list;
	_fsg->trans_list = trans;
      }
	
      cur = v;
      _expansions[index]--;
    }
  }

  /* Make one final transition from our last state to the destination state. */
  trans = (s2_fsg_trans_t*)ckd_calloc(1, sizeof(s2_fsg_trans_t));
  trans->from_state = cur;
  trans->to_state = _dest;
  trans->prob = 1;
  trans->word = NULL;
  trans->next = _fsg->trans_list;
  _fsg->trans_list = trans;
}

static void
prune_states(s2_fsg_t *_fsg)
{
  s2_fsg_trans_t **edges;
  s2_fsg_trans_t *trans, *prev;
  glist_t *adj;
  int *states;
  int i, j, count;

  assert(_fsg != NULL);

  count = 0;
  for (trans = _fsg->trans_list; trans; trans = trans->next, count++);

  states = (int *)ckd_calloc(_fsg->n_state, sizeof(int));
  edges = (s2_fsg_trans_t **)ckd_calloc(_fsg->n_state,
					sizeof(s2_fsg_trans_t *));

  /* Check and remove passable states (states with only one out-going epsilon
   * edge).  The array states[i] keeps track of out-degree and the array
   * edges[i] keeps track of the last out-going edge from state i.
   */

  for (i = _fsg->n_state - 1; i >= 0; i--) {
    edges[i] = NULL;
    states[i] = 0;
  }
  
  for (trans = _fsg->trans_list; trans != NULL; trans = trans->next) {
    edges[trans->from_state] = trans;
    states[trans->from_state]++;
  }

  count = 0;
  for (i = _fsg->n_state - 1; i >= 0; i--) {
    if (states[i] == 1 && edges[i]->word == NULL) {
      j = i;
      while (states[j] == 1 && edges[j]->word == NULL)
	j = edges[j]->to_state;
      states[i] = j;
      count++;
    }
    else
      states[i] = -1;
  }

  trans = _fsg->trans_list;
  prev = NULL;
  while (trans) {
    if (states[trans->from_state] != -1) {
      if (prev == NULL) {
	trans = _fsg->trans_list;
	_fsg->trans_list = trans->next;
	ckd_free(trans->word);
	ckd_free(trans);
	trans = _fsg->trans_list;
      }
      else {
	prev->next = trans->next;
	ckd_free(trans->word);
	ckd_free(trans);
	trans = prev->next;
      }
    }
    else {
      if (states[trans->to_state] != -1)
	trans->to_state = states[trans->to_state];
      prev = trans;
      trans = trans->next;
    }
  }

  /* Check and remove dead states (states that cannot reach the final state */

  adj = (glist_t *)ckd_calloc(_fsg->n_state, sizeof(glist_t));
  for (i = _fsg->n_state - 1; i >= 0; i--) {
    adj[i] = NULL;
    states[i] = -1;
  }

  for (trans = _fsg->trans_list; trans; trans = trans->next)
    adj[trans->to_state] = glist_add_ptr(adj[trans->to_state], trans);
  
  mark_dead_state(_fsg, _fsg->final_state, states, adj);

  count = 0;
  for (i = 0; i < _fsg->n_state; i++) {
    glist_free(adj[i]);
    if (states[i] != -1)
      states[i] = count++;
  }
  

  _fsg->n_state = count;

  trans = _fsg->trans_list;
  prev = NULL;
  while (trans) {
    if (states[trans->from_state] == -1 || states[trans->to_state] == -1) {
      if (prev == NULL) {
	trans = _fsg->trans_list;
	_fsg->trans_list = trans->next;
	ckd_free(trans->word);
	ckd_free(trans);
	trans = _fsg->trans_list;
      }
      else {
	prev->next = trans->next;
	ckd_free(trans->word);
	ckd_free(trans);
	trans = prev->next;
      }
    }
    else {
      trans->from_state = states[trans->from_state];
      trans->to_state = states[trans->to_state];
      prev = trans;
      trans = trans->next;
    }
  }

  ckd_free(states);
  ckd_free(edges);

}

static void
mark_dead_state(s2_fsg_t *_fsg, int _state, int *_marks, glist_t *_adj)
{
  s2_fsg_trans_t *trans;
  glist_t itr;

  assert(_fsg != NULL);
  
  _marks[_state] = 1;

  for (itr = _adj[_state]; itr; itr = gnode_next(itr)) {
    trans = (s2_fsg_trans_t *)gnode_ptr(itr);
    if (trans->to_state == _state && _marks[trans->from_state] == -1)
      mark_dead_state(_fsg, trans->from_state, _marks, _adj);
  }
}

#if 0

/* The following is a lame-duck attempt to convert a CFG to a FSG provided
 * the CFG is really a FSG in disguise.  It is un-tested, un-finished, not
 * really usable, and not really desirable.  Maybe someday it will be revived.
 * Then again, maybe not.
 */

static int
add_trans(s2_fsg_t*,int,int,float32,char*);

/* Attempt to convert a CFG to a FSG.  No heuristic simplifcation is performed
 * here.  The conversion will only take place if all expansion rule in the 
 * CFG takes one of the following form
 * 
 *   X -> w0 w1 ... wN Y
 *   X -> w0 w1 ... wN
 *   X -> Y
 *   X -> w0
 *   X -> nil
 *
 * where X, Y are non-terminals, and w0, ..., wN are terminals.  If the
 * conversion is not possible, _fsg is set to NULL and the function returns -1.
 */
void
s3_cfg_convert_to_fsg(s3_cfg_t *_cfg,s2_fsg_t **_fsg)
{
  hash_table_t *item2state=NULL;
  int num_states=1; /* let 0 be the end state */
  int start_state=0;
  int from_state;
  int to_state;
  int i=0,j=0;
  s3_cfg_id_t id;
  s3u_vector_t *items=NULL;
  s3u_vector_t *rules=NULL;
  s3_cfg_item_t *item=NULL;
  s3_cfg_rule_t *rule=NULL;
  s2_fsg_t *fsg=NULL;
  char *word;

  item2state=hash_table_new(S3_CFG_NAME_HASH_SIZE,HASH_CASE_YES);
  if (item2state==NULL)
    goto cleanup;

  fsg=(s2_fsg_t*)ckd_calloc(1,sizeof(s2_fsg_t));
  fsg->name=NULL;
  fsg->trans_list=NULL;

  items=&_cfg->item_info;
  for (i=s3u_vec_count(items)-1;i>=0;i--) {
    if ((item=s3u_vec_get(items,i))==NULL)
      goto cleanup;

    rules=&item->rules;
    if (!s3_cfg_is_terminal(item->id) &&
	(item->nil_rule!=NULL || (rules!=NULL && s3u_vec_count(rules)>0)))
      hash_enter_bkey(item2state,&item->id,sizeof(s3_cfg_id_t),num_states++);
  }

  /* iterate through the CFG's expansion rules and convert them to FSG
   * transitions.  If at any point the conversion fails, do some cleanup
   * and return.
   */
  rules=&_cfg->rules;
  for (i=s3u_vec_count(rules)-1;i>=0;i--) {
    if ((rule=s3u_vec_get(rules,i))==NULL)
      goto cleanup;

    hash_lookup_bkey(item2state,&rule->src,sizeof(s3_cfg_id_t),&from_state);

    /* a NULL production rule means we transition to the end state */
    if (rule->len==0)
      add_trans(fsg,from_state,0,rule->prob_score,NULL);
    else if (rule->len==1) {
      id=rule->products[0];
      /* a single terminal means we output the terminal and transition to 
       * the end state
       */
      if (s3_cfg_is_terminal(id)) {
	word=((s3_cfg_item_t*)s3u_vec_get(items,s3_cfg_id2index(id)))->name;
	add_trans(fsg,from_state,0,rule->prob_score,word);
      }
      /* a single non-terminal means we take an epsilon transition */
      else {
	hash_lookup_bkey(item2state,&id,sizeof(s3_cfg_id_t),&to_state);
	add_trans(fsg,from_state,to_state,rule->prob_score,NULL);
      }
    }
    else {
      for (j=1;j<rule->len;j++) {

	/* get the output for the transition */
	id=rule->products[j-1];
	if (!s3_cfg_is_terminal(id))
	  goto cleanup;
	word=((s3_cfg_item_t*)s3u_vec_get(items,s3_cfg_id2index(id)))->name;

	/* get the target state for the transition */
	id=rule->products[j];
	if (s3_cfg_is_terminal(id))
	  to_state=num_states++;
	else
	  hash_lookup_bkey(item2state,&id,sizeof(s3_cfg_id_t),&to_state);

	add_trans(fsg,from_state,to_state,j==1?rule->prob_score:1.0,word);

	from_state=to_state;
      }
    }
  }
  
  *_fsg=fsg;
  return 0;

 cleanup:
  if (fsg!=NULL)
    free_fsg(fsg);

  return -1;
}

static int
add_trans(s2_fsg_t *_fsg,int _from,int _to,float32 _prob,char *_word)
{
  s2_fsg_trans_t *trans=NULL;
  char *word=NULL;

  trans=(s2_fsg_trans_t*)ckd_calloc(1,sizeof(s2_fsg_trans_t));
  if (trans==NULL)
    goto cleanup;
  
  if (_word!=NULL) {
    word=(char*)ckd_salloc(_word);
    if (word==NULL)
      goto cleanup;
  }
    
  trans->from_sate=_from;
  trans->to_state=_to;
  trans->prob=_prob;
  trans->word=word;
  trans->next=_fsg->trans_list;
  _fsg->trans_list=trans;      

  return 0;

 cleanup:
  if (trans!=NULL)
    ckd_free(trans);
  if (word!=NULL)
    ckd_free(word);

  return -1;
}

static void
free_fsg(s2_fsg_t *_fsg)
{
  s2_fsg_trans_t *trans;

  trans = _fsg->trans_list;
  while (trans!=NULL) {
    _fsg->trans_list = trans->next;

    if (trans->word!=NULL)
      ckd_free ((void *) trans->word);
    ckd_free ((void *) trans);

    trans = _fsg->trans_list;
  }

  if (_fsg->name!=NULL)
    ckd_free (_fsg->name);
  ckd_free (_fsg);
}

#endif
