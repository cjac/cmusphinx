
#include <assert.h>
#include "list.h"
#include "ckd_alloc.h"

static void list_lock_internal(list_t *);
static void list_unlock_internal(list_t *);
static void list_wait_internal(list_t *, int32);
static void list_wake_internal(list_t *);

int
list_insert_head(list_t *_list, void *_element)
{
  node_t *node;

  assert(_list != null);

  /******************************** execution *******************************/

  if ((node = (node_t *)ckd_malloc(sizeof(node_t))) == null) {
    goto list_insert_head_cleanup;
  }
  node->data = _element;

  list_lock_internal(_list);

  if (_list->tail == null) {
    node->next = node;
    node->prev = node;
    _list->tail = node;
  }
  else {
    node->next = _list->tail->next;
    node->prev = _list->tail;
    _list->tail->next->prev = node;
    _list->tail->next = node;
  }
  list_wake_internal(_list);

  list_unlock_internal(_list);

  return 0;

  /********************************** cleanup *******************************/
  list_insert_head_cleanup:

  if (node != null) {
    ckd_free(node);
  }

  return -1;
}

int
list_insert_tail(list_t *_list, void *_element)
{
  node_t *node;

  assert(_list != null);

  /******************************** execution *******************************/

  if ((node = (node_t *)ckd_malloc(sizeof(node_t))) == null) {
    goto list_insert_tail_cleanup;
  }
  node->data = _element;

  list_lock_internal(_list);

  if (_list->tail == null) {
    node->next = node;
    node->prev = node;
  }
  else {
    node->next = _list->tail->next;
    node->prev = _list->tail;
    _list->tail->next->prev = node;
    _list->tail->next = node;
  }
  _list->tail = node;
  list_wake_internal(_list);

  list_unlock_internal(_list);

  return 0;

  /********************************** cleanup *******************************/
  list_insert_tail_cleanup:

  if (node != null) {
    ckd_free(node);
  }

  return -1;
}

int
list_remove_head(list_t *_list, void **_element)
{
  node_t *node = null;

  assert(_list != null);

  /******************************** execution *******************************/

  list_lock_internal(_list);

  if (_list->tail != null) {
    /** save the node to remove */
    node = _list->tail->next;
    /** check whether the list only has one node */
    if (node == node->next) {
      _list->tail = null;
    }
    else {
      node->next->prev = node->prev;
      node->prev->next = node->next;
    }
  }

  list_unlock_internal(_list);
  
  /** return failure */
  if (node == null) {
    if (_element != null) {
      *_element = null;
    }
    return -1;
  }
  
  /** check whether the return value is requested */
  if (_element != null) {
    *_element = node->data;
  }
  else {
    _list->free_data(node->data);
  }
  ckd_free(node);
  
  return 0;
}

int
list_remove_tail(list_t *_list, void **_element)
{
  node_t *node = null;

  assert(_list != null);

  /******************************** execution *******************************/

  list_lock_internal(_list);

  if (_list->tail != null) {
    /** save the node to remove */
    node = _list->tail;
    /** check whether the list has only one node */
    if (node == node->next) {
      _list->tail = null;
    }
    else {
      node->next->prev = node->prev;
      node->prev->next = node->next;
      _list->tail = node->prev;
    }
  }

  list_unlock_internal(_list);
  
  /** return failure */
  if (node == null) {
    if (_element != null) {
      *_element = null;
    }
    return -1;
  }
  
  /** check whether the return value is requested */
  if (_element != null) {
    *_element = node->data;
  }
  else {
    _list->free_data(node->data);
  }
  ckd_free(node);
  
  return 0;
}

int
list_clear(list_t *_list)
{
  node_t *itr;
  node_t *node;

  assert(_list != null);

  /******************************** execution *******************************/

  list_lock_internal(_list);

  for (itr = _list->tail; itr; node = itr = itr->next) {
    _list->free_data(node->data);
    ckd_free(node);
  }

  list_unlock_internal(_list);

  return 0;
}

int
list_wait(list_t *_list, int32 _msec)
{
  assert(_list != null);

  /******************************** execution *******************************/

  list_lock_internal(_list);

  if (_list->tail == null) {
    list_wait_internal(_list);
  }

  list_unlock_internal(_list);

  return 0;
}

static int
list_lock_internal(list_t *_list)
{
  assert(_list != null);

  /******************************** execution *******************************/

  /** Win32 implementation */

  return 0;

  /********************************** cleanup *******************************/

  return -1;
}

static int
list_unlock_internal(list_t *_list)
{
  /******************************** execution *******************************/

  return 0;

  /********************************** cleanup *******************************/

  return -1;
}

static int
list_wait_internal(list_t *_list)
{
  /******************************** execution *******************************/

  return 0;

  /********************************** cleanup *******************************/

  return -1;
}

static int
list_wake_internal(list_t *_list)
{
  /******************************** execution *******************************/

  return 0;

  /********************************** cleanup *******************************/

  return -1;
}
