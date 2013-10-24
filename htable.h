#ifndef _LINKED_STRUCTURES_HTABLE_H_
#define _LINKED_STRUCTURES_HTABLE_H_
#include "linked_base.h"
#ifdef SPLAY_PROCESS
#include "splaytree.h"
#else
#include "scapegoat.h"
static const double SCAPEGOAT_ALPHA=0.7;
#endif

typedef struct htable {
	size_t m_max_size;
	size_t size, m_collision;
	const Comparable_vtable* data_method;
#ifdef SPLAY_PROCESS
	splaytree **m_array;//with type htable_data_cluster
#else
	scapegoat **m_array;
#endif
} htable;

typedef struct htable_node_vtable {
	const Object_vtable parent;
	const Comparable_vtable compare;
} htable_node_vtable;

typedef struct htable_node {
	const htable_node_vtable* method;
	const htable* table;
	Object* data;
	uint64_t hash;
} htable_node;

htable *htable_new(size_t size, const Comparable_vtable* key_method) __attribute__((warn_unused_result));
void htable_destroy(htable*);
//Hash Tables
void htable_insert(htable *table, Object* key, BOOLEAN copy);
Object *htable_remove(htable *table, Object* key, const Comparable_vtable *key_method, BOOLEAN destroy);
BOOLEAN htable_map(htable *table, const TRAVERSAL_STRATEGY strat, const BOOLEAN more_info, void* aux, const lMapFunc);
void htable_clear(htable* tbl, BOOLEAN destroy);//Destroy all nodes in htable
Object* htable_find(htable *table, Object* key, const Comparable_vtable *key_method);

#endif
