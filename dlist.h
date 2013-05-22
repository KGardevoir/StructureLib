#ifndef _LINKED_STRUCTURES_DLIST_H_
#define _LINKED_STRUCTURES_DLIST_H_
#include "linked_base.h"

typedef struct dlist {
	struct dlist *next;
	Object* data;
	struct dlist *prev;
} dlist;


//Doubly linked list functions
//Add
dlist* dlist_push(dlist*, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
dlist* dlist_append(dlist*, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
dlist* dlist_addOrdered(dlist*, void* buf, const Comparable_vtable* buf_method, BOOLEAN copy) __attribute__((warn_unused_result));
dlist* dlist_copy(dlist* src, BOOLEAN deep_copy) __attribute__((warn_unused_result));

//Remove
void dlist_clear(dlist *he, BOOLEAN destroy_data);
dlist* dlist_dequeue(dlist* head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_pop(dlist* he, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_removeElement(dlist *head, dlist *rem, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_removeAll(dlist*, void *key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_remove(dlist*, void *key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));

//Other Operations
dlist* dlist_filter(dlist *head, void* aux, lMapFunc, BOOLEAN deep) __attribute__((warn_unused_result));
dlist* dlist_filter_i(dlist *head, void *aux, lMapFunc, BOOLEAN free_data) __attribute__((warn_unused_result));
dlist* dlist_transform(dlist *head, void* aux, lTransFunc);
BOOLEAN dlist_map(dlist *head, BOOLEAN more_info, void* aux, lMapFunc);
size_t dlist_length(dlist *head);

//Merging
dlist* dlist_sort(dlist* head, void* key, const Comparator_vtable* key_method) __attribute__((warn_unused_result));
dlist* dlist_merge(dlist* dst, dlist* src, void* key, const Comparator_vtable* method) __attribute__((warn_unused_result));
dlist* dlist_concat(dlist* dst, dlist* src) __attribute__((warn_unused_result));
dlist* dlist_split(dlist* h1, dlist* h2) __attribute__((warn_unused_result));

//Find
dlist* dlist_find(dlist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered) __attribute__((warn_unused_result));
BOOLEAN dlist_has(dlist *head, dlist* node);
#define DLIST_ITERATE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	do{\
		{\
			_CODE\
		}\
		_ITER = _ITER->next;\
		_depth++;\
	} while(_ITER != _HEAD);\
}

#define DLIST_ITERATE_REVERSE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	do{\
		{\
			_CODE\
		}\
		_ITER = _ITER->prev;\
		_depth++;\
	} while(_ITER != _HEAD);\
}
#endif
