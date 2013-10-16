#ifndef _LINKED_STRUCTURES_UDLIST_H_
#define _LINKED_STRUCTURES_UDLIST_H_
#include "linked_base.h"

//Unrolled udlist.
typedef struct udlist_node {
	struct udlist_node *next;
	struct udlist_node *prev;
	size_t n_filled;
	Object *elements[];
} udlist_node;

typedef struct udlist {
	size_t size;
	const size_t n_elements;//per node
	udlist_node *root;
} udlist;

udlist* udlist_new(size_t n_elements) __attribute__((warn_unused_result));
void udlist_destroy(udlist*);
void udlist_pushfront(udlist* head, Object* obj, BOOLEAN copy);
void udlist_pushback(udlist* head, Object* obj, BOOLEAN copy);
void udlist_insert(udlist* head, Object* obj, size_t idx, BOOLEAN copy);
void udlist_addOrdered(udlist*, Object* obj, const Comparable_vtable* buf_method, BOOLEAN copy);
udlist* udlist_copy(udlist* head, BOOLEAN copy) __attribute__((warn_unused_result));
void udlist_compact(udlist *head);

void udlist_clear(udlist *head, BOOLEAN destroy_data);
Object *udlist_popback(udlist* head, BOOLEAN destroy_data);
Object *udlist_popfront(udlist* he, BOOLEAN destroy_data);
Object *udlist_delete(udlist *head, size_t idx, BOOLEAN destroy_data);
void udlist_removeAll(udlist*, void *key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data);
Object *udlist_remove(udlist*, void *key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data);

Object **udlist_at(udlist *head, size_t idx);

#define UDLIST_ITERATE(_ITER, _HEAD, _CODE) do { \
	if(!(_HEAD) && !(_HEAD)->root){ \
		size_t _idx = 0, _depth = 0; \
		do { \
			for(_idx = 0; _idx < (_ITER)->n_filled; _idx++){ \
				Object *_ITER ## _data = (_ITER)->elements[idx]; \;\
			} \
			_ITER = (_ITER)->next; \
		} while(_ITER != (_HEAD)->root);\
	} \
} while(0)

#endif
