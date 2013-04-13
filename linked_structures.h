#ifndef _LINKED_STRUCTURES_H_
#define _LINKED_STRUCTURES_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct Object Object;
typedef struct Comparable Comparable;

typedef struct anObject {
	const Object *method;//also can be used as a key for the function type
} anObject;
struct Object {//an interface for objects
	size_t(*getSize)(const anObject *self);
	void  (*destroy)(const anObject *self);//how to destroy our data
	anObject* (*copy)(const anObject *self); //return a copy of the information (within a newly allocated buffer)
};


typedef struct aComparable {
	const Comparable *method;
} aComparable;
struct Comparable {//an interface for comparable objects
	struct Object parent;
	long  (*compare)(const aComparable* self, const anObject* oth); //how to compare (NULL if undesired)
};

/* TODO change hash table to use this instead
typedef struct Hashable Hashable;
typedef struct aHashable {
	const Hashable *method;
} aHashable;
struct Hashable {
	struct Comparable parent;
	long (*hash)(const aHashable* self);
};
*/

typedef enum TRAVERSAL_STRATEGY {
	BREADTH_FIRST=0,
	DEPTH_FIRST_PRE=1,
	DEPTH_FIRST=2,
	DEPTH_FIRST_IN=2,
	DEPTH_FIRST_POST=3,
} TRAVERSAL_STRATEGY;

typedef struct slist {
	struct slist *next;
	anObject* data;
} slist;

typedef struct dlist {
	struct dlist *next;
	anObject* data;
	struct dlist *prev;
} dlist;

typedef struct tree {
	aComparable* data;
	struct tree *left;
	struct tree *right;
} tree;

typedef struct graph {
	const Comparable *method;
	anObject* data;
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
	splaytree *array[1];//with type htable_data_cluster
} htable;

typedef struct htable_cluster {
	const Comparable *method;
	aComparable* key;
	anObject* data;
	uint64_t hash;
} htable_cluster;

typedef enum BOOLEAN { FALSE=0, TRUE=-1 } BOOLEAN;
typedef struct lMapFuncAux {
	BOOLEAN isAux;
	size_t depth;
	size_t position;
	size_t size;
	void* aux;//user data
} lMapFuncAux;//TODO finish implementing all fields for graphs and htable
typedef BOOLEAN (*lMapFunc)(anObject *data, void *aux/*auxilarly data (constant between calls)*/); //a mapping function
typedef BOOLEAN (*lTransFunc)(anObject **data, void* aux);/*in-place data transformation, should always return TRUE as FALSE means stop*/


//Add
slist* slist_push(slist* he, anObject* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_append(slist* he, anObject* buf, BOOLEAN copy) __attribute__((warn_unused_result));
slist* slist_addOrdered(slist* he, aComparable* buf, BOOLEAN copy, BOOLEAN overwrite) __attribute__((warn_unused_result));
slist* slist_copy(slist* src, BOOLEAN deep_copy) __attribute__((warn_unused_result));

//Remove
void slist_clear(slist *he, BOOLEAN destroy_data);
slist* slist_dequeue(slist *head, anObject** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_pop(slist *head, anObject** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_removeAll(slist *head, aComparable* key, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
slist* slist_remove(slist *head, aComparable* key, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));

//Other Operations
BOOLEAN slist_map(slist *head, BOOLEAN more_info, void* aux, lMapFunc) __attribute__((warn_unused_result));
size_t slist_length(slist* head);
//Transform
anObject** slist_toArray(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));
anObject** slist_toArrayReverse(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));
dlist* slist_to_dlist(slist *head, BOOLEAN deep) __attribute__((warn_unused_result));

//Find
void* slist_find(slist *head, aComparable* key, BOOLEAN ordered) __attribute__((warn_unused_result));
#define SLIST_ITERATE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	for(;_ITER;_ITER=_ITER->next, _depth++) {\
		_CODE\
	}\
}

//Doubly linked list functions
//Add
dlist* dlist_push(dlist*, anObject* buf, BOOLEAN copy) __attribute__((warn_unused_result));
dlist* dlist_append(dlist*, anObject* buf, BOOLEAN copy) __attribute__((warn_unused_result));
dlist* dlist_addOrdered(dlist*, aComparable* buf, BOOLEAN copy) __attribute__((warn_unused_result));
dlist* dlist_copy(dlist* src, BOOLEAN deep_copy) __attribute__((warn_unused_result));

//Remove
void dlist_clear(dlist *he, BOOLEAN destroy_data);
dlist* dlist_dequeue(dlist* head, anObject** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_pop(dlist* he, anObject** data, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_remove(dlist*, aComparable *key, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_removeAll(dlist*, aComparable *key, BOOLEAN ordered, BOOLEAN destroy_data) __attribute__((warn_unused_result));
dlist* dlist_removeElement(dlist *head, dlist *rem, BOOLEAN destroy_data) __attribute__((warn_unused_result));

//Other Operations
dlist* dlist_filter(dlist *head, void* aux, lMapFunc, BOOLEAN deep) __attribute__((warn_unused_result));
dlist* dlist_filter_i(dlist *head, void *aux, lMapFunc, BOOLEAN free_data) __attribute__((warn_unused_result));
dlist* dlist_transform(dlist *head, void* aux, lTransFunc);
BOOLEAN dlist_map(dlist *head, BOOLEAN more_info, void* aux, lMapFunc);
size_t dlist_length(dlist *head);
//Transform
anObject** dlist_toArray(dlist *head, BOOLEAN deep) __attribute__((warn_unused_result));
anObject** dlist_toArrayReverse(dlist *head, BOOLEAN deep) __attribute__((warn_unused_result));
slist* dlist_to_slist(dlist *he, BOOLEAN deep) __attribute__((warn_unused_result));

//Merging
dlist* dlist_sort(dlist* head) __attribute__((warn_unused_result));
dlist* dlist_merge(dlist* dst, dlist* src) __attribute__((warn_unused_result));
dlist* dlist_concat(dlist* dst, dlist* src) __attribute__((warn_unused_result));
dlist* dlist_split(dlist* h1, dlist* h2) __attribute__((warn_unused_result));

//Find
//Data returned from find is PACKED which means that it returns the node, not the data, where the data resides
dlist* dlist_find(dlist *head, const aComparable* key, BOOLEAN ordered) __attribute__((warn_unused_result));
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
	} while(_ITER != HEAD);\
}


//Binary Search Tree, a threaded tree would be nice, but determining leaf nodes is SIGNIFICANTLY more difficult
bstree* bstree_insert(bstree* root, aComparable *data, BOOLEAN copy);
bstree* bstree_remove(bstree* root, aComparable *data, aComparable** rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));

void    bstree_clear(bstree* root, BOOLEAN destroy_data);
bstree* bstree_find(bstree *root, aComparable *data);
bstree* bstree_parent(bstree *root, aComparable *data);
dlist*  bstree_path(bstree *root, aComparable *data) __attribute__((warn_unused_result));

bstree* bstree_predessor(bstree *root, bstree *node);
bstree* bstree_successor(bstree *root, bstree *node);
bstree* bstree_findmin(bstree *root);
bstree* bstree_findmax(bstree *root);

void   bstree_info(bstree *root, size_t *min, size_t *max, size_t *avg, size_t *leaves, size_t *size, dlist **lleaves, dlist **lnodes);

BOOLEAN bstree_map(bstree *root, const TRAVERSAL_STRATEGY, BOOLEAN more_info, void* aux, lMapFunc func);

//Splay Trees
splaytree* splay_insert(splaytree* root, aComparable *data, BOOLEAN copy) __attribute__((warn_unused_result));
splaytree* splay_remove(splaytree* root, aComparable *data, aComparable **rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));
splaytree* splay_find(splaytree* root, aComparable *data) __attribute__((warn_unused_result));

//Graphs
graph* graph_insert(graph* root, anObject* data, BOOLEAN copy);
graph* graph_link(graph* root, graph* child);
graph* graph_remove(graph* root, anObject* key, anObject** data, BOOLEAN copy) __attribute__((warn_unused_result));
graph_adjmat* graph_matrix(graph* root, anObject* key, anObject** data, BOOLEAN copy) __attribute__((warn_unused_result));
void graph_clear(graph *root, BOOLEAN destroy_data);

void graph_size(graph* root, size_t* nodes, size_t* edges);//count of all the nodes in the graph
graph* graph_find(graph* root, TRAVERSAL_STRATEGY, aComparable* key);
//TODO this is significantly more complex than previously anticipated (it requires some advanced
//AI stuff to do efficiently, e.g. A*, B*, Beam, D*).
dlist* graph_path(graph* root, TRAVERSAL_STRATEGY, aComparable* key) __attribute__((warn_unused_result));
graph* graph_path_key_match(graph *root, dlist *key_path);

BOOLEAN graph_map(graph* root, TRAVERSAL_STRATEGY, BOOLEAN more_info, void* aux, lMapFunc func);

//Hash Tables
htable* htable_insert(htable *table, aComparable *key, anObject *data, BOOLEAN copy, size_t isize) __attribute__((warn_unused_result));
htable* htable_remove(htable *table, aComparable *key, anObject **rtn, BOOLEAN destroy_data) __attribute__((warn_unused_result));
BOOLEAN htable_map(htable *table, TRAVERSAL_STRATEGY strat, BOOLEAN more_info, void* aux, lMapFunc);
void htable_clear(htable* tbl, BOOLEAN destroy_data);
void* htable_element(htable *table, aComparable *key);
#endif //_LINKED_STRUCTURES_H_
