#ifndef _LINKED_STRUCTURES_HTABLE_H_
#define _LINKED_STRUCTURES_HTABLE_H_
#include "linked_base.h"
#include "splaytree.h"

#define DEFAULT_HTABLE_SIZE 256

typedef struct htable {
	const size_t size;
	size_t filled, collision;
	splaytree *array[0];//with type htable_data_cluster
} htable;

typedef struct htable_node_vtable {
	const Object_vtable parent;
	const Comparable_vtable compare;
} htable_node_vtable;

typedef struct htable_node {
	const htable_node_vtable const* method;
	Object* key;
	const Comparable_vtable const* key_method;
	void* data;
	uint64_t hash;
} htable_node;

//Hash Tables
htable* htable_insert(htable *table, Object* key, const Comparable_vtable *key_method, void *data, BOOLEAN copy, size_t isize) __attribute__((warn_unused_result));
htable* htable_remove(htable *table, Object* key, const Comparable_vtable *key_method, Object **key_rtn, void **rtn) __attribute__((warn_unused_result));
BOOLEAN htable_map(htable *table, const TRAVERSAL_STRATEGY strat, const BOOLEAN more_info, void* aux, const lMapFunc);
void htable_clear(htable* tbl);//Destroy all nodes in htable
void* htable_element(htable *table, Object* key, const Comparable_vtable *key_method);

#endif
