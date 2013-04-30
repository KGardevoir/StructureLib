#ifndef _LINKED_STRUCTURES_H_
#define _LINKED_STRUCTURES_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "Object.h"


typedef enum TRAVERSAL_STRATEGY {
	BREADTH_FIRST=0,
	DEPTH_FIRST_PRE=1,
	DEPTH_FIRST=2,
	DEPTH_FIRST_IN=2,
	DEPTH_FIRST_POST=3,
} TRAVERSAL_STRATEGY;

typedef struct slist {
	struct slist *next;
	Object* data;
} slist;

typedef struct dlist {
	struct dlist *next;
	Object* data;
	struct dlist *prev;
} dlist;

typedef struct tree {
	Object* data;
	struct tree *left;
	struct tree *right;
} tree;

typedef struct graph_vtable {
	Object_vtable parent;
	Comparable_vtable comparable;
} graph_vtable;

typedef struct graph {
	const graph_vtable *method;
	Object* data;
	dlist* edges;//ordered by key value (if possible)
} graph; //adjacencly list

typedef struct graph_adjmat {
	size_t i, j; //the size
	graph ***matrix;//[][]; //the graph
	graph **mappings;//[]; //graph mappings
} graph_adjmat;

typedef tree bstree;
typedef bstree splaytree;

typedef struct htable {
	const size_t size;
	size_t filled, collision;
	splaytree *array[0];//with type htable_data_cluster
} htable;

typedef struct htable_node_vtable {
	Object_vtable parent;
	Comparable_vtable compare;
} htable_node_vtable;

typedef struct htable_node {
	const htable_node_vtable *method;
	Object* key;
	const Comparable_vtable *key_method;
	void* data;
	uint64_t hash;
} htable_node;

typedef struct lMapFuncAux {
	BOOLEAN isAux;
	size_t depth;
	size_t position;
	size_t size;
	void* aux;//user data
} lMapFuncAux;//TODO finish implementing all fields for graphs and htable
typedef BOOLEAN (*lMapFunc)(Object *data, void *aux/*auxilarly data (constant between calls)*/); //a mapping function
typedef BOOLEAN (*lTransFunc)(Object **data, void* aux);/*in-place data transformation, should always return TRUE as FALSE means stop*/


//Add
slist* slist_push(slist* he, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_append(slist* he, Object* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_addOrdered(slist* he, Object* buf, const Comparable_vtable* buf_method, BOOLEAN copy, BOOLEAN overwrite) __attribute__((warn_unused_result));
slist* slist_copy(slist* src, BOOLEAN deep_copy) __attribute__((warn_unused_result));

//Remove
void slist_clear(slist *he, BOOLEAN destroy_data);
slist* slist_dequeue(slist *head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_pop(slist *head, Object** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeAll(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_remove(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));

//Other Operations
BOOLEAN slist_map(slist *head, BOOLEAN more_info, void* aux, lMapFunc) __attribute__((warn_unused_result));
size_t slist_length(slist* head);
//Transform
Object** slist_toArray(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));
Object** slist_toArrayReverse(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));
dlist* slist_to_dlist(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));

//Find
void* slist_find(slist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered) __attribute__((warn_unused_result));
#define SLIST_ITERATE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	for(;_ITER;_ITER=_ITER->next, _depth++) {\
		_CODE\
	}\
}

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
//Transform
Object** dlist_toArray(dlist *head, BOOLEAN deep) __attribute__((warn_unused_result));
Object** dlist_toArrayReverse(dlist *head, BOOLEAN deep) __attribute__((warn_unused_result));
slist* dlist_to_slist(dlist *he, BOOLEAN deep) __attribute__((warn_unused_result));

//Merging
dlist* dlist_sort(dlist* head, void* key, const Comparator_vtable* key_method) __attribute__((warn_unused_result));
dlist* dlist_merge(dlist* dst, dlist* src, void* key, const Comparator_vtable* method) __attribute__((warn_unused_result));
dlist* dlist_concat(dlist* dst, dlist* src) __attribute__((warn_unused_result));
dlist* dlist_split(dlist* h1, dlist* h2) __attribute__((warn_unused_result));

//Find
dlist* dlist_find(dlist *head, void* key, const Comparable_vtable* key_method, BOOLEAN ordered) __attribute__((warn_unused_result));
BOOLEAN dlist_had(dlist *head, dlist* node);
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


//Binary Search Tree, a threaded tree would be nice, but determining leaf nodes is SIGNIFICANTLY more difficult
bstree* bstree_insert(bstree* root, Object* data, const Comparable_vtable *data_method, BOOLEAN copy);
bstree* bstree_remove(bstree* root, Object* key, const Comparable_vtable *key_method, Object** rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));

void    bstree_clear(bstree* root, BOOLEAN destroy_data);
bstree* bstree_find(bstree *root, Object* data, const Comparable_vtable *data_method);
bstree* bstree_parent(bstree *root, Object* data, const Comparable_vtable *data_method);
dlist*  bstree_path(bstree *root, Object* data, const Comparable_vtable *data_method) __attribute__((warn_unused_result));

bstree* bstree_predessor(bstree *root, bstree *node, const Comparable_vtable *data_method);
bstree* bstree_successor(bstree *root, bstree *node, const Comparable_vtable *data_method);
bstree* bstree_findmin(bstree *root);
bstree* bstree_findmax(bstree *root);

void   bstree_info(bstree *root, size_t *min, size_t *max, size_t *avg, size_t *leaves, size_t *size, dlist **lleaves, dlist **lnodes);

BOOLEAN bstree_map(bstree *root, const TRAVERSAL_STRATEGY, BOOLEAN more_info, void* aux, lMapFunc func);

//Splay Trees
splaytree* splay_insert(splaytree* root, Object* data, const Comparable_vtable *data_method, BOOLEAN copy) __attribute__((warn_unused_result));
splaytree* splay_remove(splaytree* root, Object* data, const Comparable_vtable *data_method, Object **rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));
splaytree* splay_find(splaytree* root, Object* key, const Comparable_vtable *key_method) __attribute__((warn_unused_result));

//Graphs
graph* graph_insert(graph* root, Object* data, BOOLEAN copy);
graph* graph_link(graph* root, graph* child);
graph* graph_remove(graph* root, Object* key, Object** data, BOOLEAN copy) __attribute__((warn_unused_result));
graph_adjmat* graph_matrix(graph* root, Object* key, Object** data, BOOLEAN copy) __attribute__((warn_unused_result));
void graph_clear(graph *root, BOOLEAN destroy_data);

void graph_size(graph* root, size_t* nodes, size_t* edges);//count of all the nodes in the graph
graph* graph_find(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method);
//TODO this is significantly more complex than previously anticipated (it requires some advanced
//AI stuff to do efficiently, e.g. A*, B*, Beam, D*).
dlist* graph_path(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method) __attribute__((warn_unused_result));
graph* graph_path_key_match(graph *root, dlist *key_path);

BOOLEAN graph_map(graph* root, TRAVERSAL_STRATEGY, BOOLEAN more_info, void* aux, lMapFunc func);

//Hash Tables
htable* htable_insert(htable *table, Object* key, const Comparable_vtable *key_method, void *data, BOOLEAN copy, size_t isize) __attribute__((warn_unused_result));
htable* htable_remove(htable *table, Object* key, const Comparable_vtable *key_method, Object **key_rtn, void **rtn) __attribute__((warn_unused_result));
BOOLEAN htable_map(htable *table, TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc);
void htable_clear(htable* tbl);//Destroy all nodes in htable
void* htable_element(htable *table, Object* key, const Comparable_vtable *key_method);
#endif //_LINKED_STRUCTURES_H_
