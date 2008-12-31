
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "s3_cfg.h"
#include "s3u_vector.h"

#define DELIMS		(xmlChar*)" \t\r\n"
#define NODE_GRAMMAR	(xmlChar*)"grammar"
#define NODE_ITEM	(xmlChar*)"item"
#define NODE_ONEOF	(xmlChar*)"oneof"
#define NODE_RULE	(xmlChar*)"rule"
#define NODE_RULEREF	(xmlChar*)"ruleref"
#define ATTR_ROOT	(xmlChar*)"root"
#define ATTR_URI	(xmlChar*)"uri"
#define ATTR_ID		(xmlChar*)"id"

static int
read_srgs_dom(s3_cfg_t *_cfg, xmlDoc *_doc);
static int
read_srgs_expansion(s3_cfg_t *_cfg, xmlNode *_node, s3_cfg_id_t _src);
static void
get_text_term(xmlChar *_text, int _offset, int _len, char *_term);
static void
get_item_term(char *_term);
static int
get_ruleref_term(xmlNode *_node, char *_term);
static int
get_rule_term(xmlNode *_node, char *_term);
static int
get_text_tokens(const xmlChar *_str, int *_offset, char *_buffer, int _max);
static int
get_rootrule_term(xmlNode *_node, char *_term);

int
s3_cfg_read_srgs(s3_cfg_t *_cfg, const char *_fn)
{
  xmlDoc *doc = NULL;

  assert(_cfg != NULL);
  assert(_fn != NULL);

  /* parse a xml doc w/ default encoding and no extra option */
  doc = xmlReadFile(_fn, NULL, 0);
  if (read_srgs_dom(_cfg, doc))
    goto cleanup;
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return 0;

 cleanup:
  if (doc != NULL)
    xmlFreeDoc(doc);
  xmlCleanupParser();

  return -1;
}

static int
read_srgs_dom(s3_cfg_t *_cfg, xmlDoc *_doc)
{
  xmlNode *grammar;
  xmlNode *rule;
  char term[1024];
  s3_cfg_id_t products[2];
  s3_cfg_id_t id;

  grammar = xmlDocGetRootElement(_doc);
  if (grammar == NULL || xmlStrcmp(NODE_GRAMMAR, grammar->name) != 0)
    goto cleanup;

  get_rootrule_term(grammar, term);
  id = s3_cfg_str2id(_cfg, term);
  if (id == S3_CFG_INVALID_ID)
    goto cleanup;
  products[0] = id;
  products[1] = S3_CFG_EOR_ITEM;
  if (s3_cfg_add_rule(_cfg, S3_CFG_START_ITEM, 1.0, products) == NULL)
    goto cleanup;

  for (rule = grammar->children; rule != NULL; rule = rule->next) {
    if (rule->type == XML_TEXT_NODE)
      continue;
    else if (xmlStrcmp(NODE_RULE, rule->name) == 0) {
      get_rule_term(rule, term);
      id = s3_cfg_str2id(_cfg, term);
      if (id == S3_CFG_INVALID_ID)
	goto cleanup;

      if (read_srgs_expansion(_cfg, rule, id))
	goto cleanup;
    }
    else
      goto cleanup;
  }

  return 0;

 cleanup:
  
  return -1;
}

static int
read_srgs_expansion(s3_cfg_t *_cfg, xmlNode *_node, s3_cfg_id_t _src)
{
  xmlNode *child;
  xmlNode *oneof;
  s3u_vector_t stack;
  char term[1024];
  s3_cfg_id_t id;
  s3_cfg_id_t *products;

  xmlChar *text;
  int offset;

  s3u_vec_init(&stack);

  for (child = _node->children; child != NULL; child = child->next) {
    if (child->type == XML_TEXT_NODE) {
      offset = 0;
      text = child->content;
      while (get_text_tokens(text, &offset, term, 1023) > 0) {
	id = s3_cfg_str2id(_cfg, term);
	if (id == S3_CFG_INVALID_ID)
	  goto cleanup;
	if (s3u_vec_append(&stack, (void*)id))
	  goto cleanup;
      }
    }
    else if (xmlStrcmp(NODE_ITEM, child->name) == 0) {
      get_item_term(term);
      id = s3_cfg_str2id(_cfg, term);
      if (id == S3_CFG_INVALID_ID)
	goto cleanup;
      if (read_srgs_expansion(_cfg, child, id))
	goto cleanup;
      if (s3u_vec_append(&stack, (void*)id))
	  goto cleanup;
    }
    else if (xmlStrcmp(NODE_RULEREF, child->name) == 0) {
      get_ruleref_term(child, term);
      id = s3_cfg_str2id(_cfg, term);
      if (id == S3_CFG_INVALID_ID)
	goto cleanup;
      if (s3u_vec_append(&stack, (void*)id))
	goto cleanup;
    }
    else if (xmlStrcmp(NODE_ONEOF, child->name) == 0) {
      get_item_term(term);
      id = s3_cfg_str2id(_cfg, term);
      if (id == S3_CFG_INVALID_ID)
	goto cleanup;
      if (s3u_vec_append(&stack, (void*)id))
	goto cleanup;

      for (oneof = child->children; oneof != NULL; oneof = oneof->next) {
	if (xmlStrcmp(NODE_ITEM, oneof->name) == 0) {
	  if (read_srgs_expansion(_cfg, oneof, id))
	    goto cleanup;
	}
      }
    }
    else {
      /* let's not deal with this yet */
    }
  }

  if (s3u_vec_append(&stack, (void*)S3_CFG_EOR_ITEM))
    goto cleanup;
  if (s3u_vec_to_array(&stack, (void***)&products))
    goto cleanup;
  if (s3_cfg_add_rule(_cfg, _src, 1.0, products) == NULL)
    goto cleanup;

  s3u_vec_close(&stack);

  return 0;

 cleanup:

  s3u_vec_close(&stack);

  return -1;
}

int
get_text_tokens(const xmlChar *_text, int *_offset, char *_buffer, int _max)
{
  int start = *_offset;
  int end;
  int len;

  while (_text[start] != '\0' && strchr(DELIMS, _text[start]) != NULL)
    start++;

  end = start;
  while (_text[end] != '\0'  && strchr(DELIMS, _text[end]) == NULL)
    end++;
  
  len = end - start;
  if (len > _max)
    return -1;
  if (_text[start] == '\0')
    return 0;

  memcpy(_buffer, _text + start, len);
  _buffer[len] = '\0';
  *_offset = end;

  return 1;
}

void
get_item_term(char *_term)
{
  static int item_count = 0;

  sprintf(_term, "$item(%d)", item_count++);
}

int
get_ruleref_term(xmlNode *_node, char *_term)
{
  xmlChar *uri = xmlGetProp(_node, ATTR_URI);
  if (uri == NULL || uri[0] != '#')
    return -1;
  sprintf(_term, "$rule(%s)", uri + 1);
  xmlFree(uri);

  return 0;
}
			       
int
get_rule_term(xmlNode *_node, char *_term)
{
  xmlChar *idXMLStr = xmlGetProp(_node, ATTR_ID);
  if (idXMLStr == NULL)
    return -1;
  sprintf(_term, "$rule(%s)", idXMLStr);
  xmlFree(idXMLStr);

  return 0;
}

int
get_rootrule_term(xmlNode *_node, char *_term)
{
  xmlChar *rootXMLStr = xmlGetProp(_node, ATTR_ROOT);
  if (rootXMLStr == NULL)
    return -1;
  sprintf(_term, "$rule(%s)", rootXMLStr);
  xmlFree(rootXMLStr);

  return 0;
}
		       
#undef DELIMS
#undef NODE_GRAMMAR
#undef NODE_ITEM
#undef NODE_RULE
#undef NODE_RULEREF
#undef ATTR_URI
#undef ATTR_ID
