#ifndef _LINKED_STRUCTURES_SLIST_H_
#define _LINKED_STRUCTURES_SLIST_H_
#include "linked_base.h"

typedef struct slist {
	struct slist *next;
	Object* data;
} slist;

//Add
slist* slist_pushfront(slist* he, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_pushback(slist* he, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_addOrdered(slist* he, Object* buf, const Comparable_vtable* buf_method, BOOLEAN copy, BOOLEAN overwrite) __attribute__((warn_unused_result));
slist* slist_copy(slist* src, BOOLEAN deep_copy) __attribute__((warn_unused_result));
slist* slist_concat(slist *head, slist *tail) __attribute__((warn_unused_result));

//Remove
void slist_clear(slist *he, BOOLEAN destroy_data);
slist* slist_popback(slist *head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_popfront(slist *head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeAll(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_remove(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));

//Other Operations
BOOLEAN slist_map(slist *head, const BOOLEAN more_info, const void* aux, const lMapFunc) __attribute__((warn_unused_result));
size_t slist_length(slist* head);
slist* slist_reverse(slist* head) __attribute__((warn_unused_result));

//Find
slist* slist_find(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered) __attribute__((warn_unused_result));
BOOLEAN slist_has(slist *head, slist* node);
slist *slist_head(slist *head);
slist *slist_tail(slist *head);
slist *slist_at(slist *head, size_t idx, BOOLEAN back);
size_t slist_loc(slist *head, slist *node);

//Other

#define SLIST_ITERATE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	for(;_ITER;_ITER=(_ITER)->next, _depth++) {\
		_CODE\
	}\
}

#endif
