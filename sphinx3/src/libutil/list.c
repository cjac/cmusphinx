
#include <assert.h>
#include "list.h"
#include "ckd_alloc.h"

/** Win32 implementation */
#include <windows.h>

static void list_lock_internal(list_t *);
static void list_unlock_internal(list_t *);
static int list_wait_internal(list_t *, int32);
static void list_wake_internal(list_t *);

list_t*
list_init(void (*_free_data)(void *))
{
  list_t *list = null;

  if ((list = (list_t *)ckd_malloc(sizeof(list_t))) == null) {
    goto list_init_cleanup;
  }

  list->mutex = null;
  list->cond = null;
  list->tail = null;
  list->free_data = null;

  if ((list->mutex = CreateMutex()) == NULL) {
    goto list_init_cleanup;
  }

  if ((list->cond = CreateEvent()) == NULL) {
    goto list_init_cleanup;
  }

  list->free_data = _free_data;

  return list;

  list_init_cleanup:

  if (list->mutex != null) {
    CloseHandle(list->mutex);
  }

  if (list->cond != null) {
    CloseHandle(list->cond);
  }

  return null;
}

void
list_free(list_t *_list)
{
  assert(_list != null && _list->mutex != null && _list->cond != null);

  list_clear(_list);

  CloseHandle(_list->mutex);
  CloseHandle(_list->cond);
  ckd_free(_list);
}

int
list_insert_head(list_t *_list, void *_element)
{
  node_t *node;

  assert(_list != null);

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
list_peek_head(list_t *_list, void **_element)
{
  assert(_list != null);

  list_lock_internal(_list);

  if (_list->tail != null) {
    *_element = _list->tail->next->data;
    list_unlock_internal(_list);

    return 0;
    
  }
  else {
    *_element = null;
    list_unlock_internal(_list);

    return -1;
  }
}

int
list_peek_tail(list_t *_list, void **_element)
{
  assert(_list != null);

  list_lock_internal(_list);

  if (_list->tail != null) {
    *_element = _list->tail->data;
    list_unlock_internal(_list);

    return 0;
    
  }
  else {
    *_element = null;
    list_unlock_internal(_list);

    return -1;
  }
}

void
list_clear(list_t *_list)
{
  node_t *itr;
  node_t *node;

  assert(_list != null && _list->free_data != null);

  list_lock_internal(_list);

  for (itr = _list->tail; itr; node = itr = itr->next) {
    _list->free_data(node->data);
    ckd_free(node);
  }

  list_unlock_internal(_list);
}

void
list_wait(list_t *_list, int32 _msec)
{
  assert(_list != null);


  list_lock_internal(_list);

  if (_list->tail == null) {
    list_wait_internal(_list);
  }

  list_unlock_internal(_list);
}

static void
list_lock_internal(list_t *_list)
{
  assert(_list != null);
  assert(_list->mutex != NULL);

  /** Win32 implementation */
  WaitForSingleObject(_list->mutex, INFINITE);
}

static void
list_unlock_internal(list_t *_list)
{
  assert(_list != null);
  assert(_list->mutex != NULL);

  /** Win32 implementation */
  ReleaseMutex(_list->mutex);
}

static void
list_wait_internal(list_t *_list, int32 _millisec)
{
  HANDLE obj[2];

  assert(_list != null);
  assert(_list->mutex != NULL && _list->cond != NULL);

  /** Win32 implementation */
  WaitForSingleObject(_list->mutex, INFINITE);
  obj[0] = _list->mutex;
  obj[1] = _list->cond;
  ResetEvent(_list->cond);
  ReleaseMutex(_list->mutex);
  WaitForMultipleObjects(2, obj, TRUE,
			 _millisec == 0 ? INFINITE : _millisec);
}

static void
list_wake_internal(list_t *_list)
{
  assert(_list != null);
  assert(_list->mutex != NULL && _list->cond != NULL);

  /** Win32 implementation */
  WaitForSingleObject(_list->mutex, INFINITE);
  SetEvent(_list->cond);
}
