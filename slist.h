#ifndef _LINKED_STRUCTURES_SLIST_H_
#define _LINKED_STRUCTURES_SLIST_H_
#include "linked_base.h"

typedef struct slist {
	struct slist *next;
	Object* data;
} slist;

#include "dlist.h"

//Add
slist* slist_push(slist* he, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_append(slist* he, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_addOrdered(slist* he, Object* buf, const Comparable_vtable* buf_method, BOOLEAN copy, BOOLEAN overwrite) __attribute__((warn_unused_result));
slist* slist_copy(slist* src, BOOLEAN deep_copy) __attribute__((warn_unused_result));
slist* slist_concat(slist *head, slist *tail) __attribute__((warn_unused_result));

//Remove
void slist_clear(slist *he, BOOLEAN destroy_data);
slist* slist_dequeue(slist *head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_pop(slist *head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeAll(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_remove(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));

//Other Operations
BOOLEAN slist_map(slist *head, const BOOLEAN more_info, const void* aux, const lMapFunc) __attribute__((warn_unused_result));
size_t slist_length(slist* head);

//Find
slist* slist_find(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered) __attribute__((warn_unused_result));
#define SLIST_ITERATE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	for(;_ITER;_ITER=_ITER->next, _depth++) {\
		_CODE\
	}\
}

#endif
