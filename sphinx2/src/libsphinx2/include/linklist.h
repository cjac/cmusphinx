/* Nasty statically-allocated linked list functions (yes, there are a
   limited number of lists, and they are stored internally to this
   module).  Probably deprecated, but we needed prototypes anyway. */

#ifndef _LINKLIST_H_
#define _LINKLIST_H_

void *listelem_alloc (int32 elem_size);
void listelem_free (void *elem, int32 elem_size);

#endif /* _LINKLIST_H_ */
