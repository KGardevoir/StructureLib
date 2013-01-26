#ifndef _LINKED_STRUCTURES_H_
#define _LINKED_STRUCTURES_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void  (*lDestroy)(void* data);//how to destroy our data
typedef void* (*lDeepCopy)(const void* src); //return a copy of the information (within a newly allocated buffer)
typedef long  (*lCompare)(const void* key, const void*); //how to compare (NULL if undesired)
typedef long  (*lKeyCompare)(const void *key, const void*);

typedef enum TRAVERSAL_STRATEGY {
	BREADTH_FIRST=0,
	DEPTH_FIRST_PRE=1,
	DEPTH_FIRST=2,
	DEPTH_FIRST_IN=2,
	DEPTH_FIRST_POST=3,
} TRAVERSAL_STRATEGY;

typedef struct list_tspec {
	const lDestroy destroy;
	const lDeepCopy deep_copy;
	const lCompare compar;
	const lKeyCompare key_compar;
} list_tspec;

typedef struct slist {
	struct slist *next;
	void* data;
} slist;

typedef struct dlist {
	struct dlist *next;
	void* data;
	struct dlist *prev;
} dlist;

typedef struct tree {
	void* data;
	struct tree *left;
	struct tree *right;
} tree;

typedef struct graph {
	void* data;
	dlist* edges;//ordered by key value (if possible)
} graph; //adjacencly list

typedef struct graph_adjmat {
	size_t i, j; //the size
	graph ***matrix;//[][]; //the graph
	graph **mappings;//[]; //graph mappings
} graph_adjmat;

typedef tree bstree;
typedef bstree splaytree;

typedef enum BOOLEAN { FALSE=0, TRUE=-1 } BOOLEAN;
typedef struct lMapFuncAux {
	BOOLEAN isAux;
	size_t depth;
	size_t position;
	size_t size;
	void* aux;//user data
} lMapFuncAux;
typedef BOOLEAN (*lMapFunc)(void *data, void *aux/*auxilarly data (constant between calls)*/); //a mapping function
typedef BOOLEAN (*lTransFunc)(void **data, void* aux);/*in-place data transformation, should always return TRUE as FALSE means stop*/


//Add
slist* slist_push(slist* he, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
slist* slist_append(slist* he, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
slist* slist_addOrdered(slist* he, void* buf, BOOLEAN copy, BOOLEAN overwrite, list_tspec*) __attribute__((warn_unused_result));
slist* slist_copy(slist* src, BOOLEAN deep_copy, list_tspec*) __attribute__((warn_unused_result));

//Remove
void slist_clear(slist *he, BOOLEAN destroy_data, list_tspec*);
slist* slist_dequeue(slist *head, void** data, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
slist* slist_pop(slist *head, void** data, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
slist* slist_removeElement(slist *head, slist *rem, BOOLEAN destroy_data, list_tspec* type) __attribute__((warn_unused_result));
slist* slist_removeViaAllKey(slist *head, void **data, void* key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
slist* slist_removeViaKey(slist *head, void **data, void* key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));

//Other Operations
BOOLEAN slist_map(slist *head, BOOLEAN more_info, void* aux, lMapFunc) __attribute__((warn_unused_result));
size_t slist_length(slist* head);
//Transform
void** slist_toArray(slist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
void** slist_toArrayReverse(slist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
dlist* slist_to_dlist(slist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));

//Find
void* slist_find(slist *head, void* key, BOOLEAN ordered, list_tspec*) __attribute__((warn_unused_result));
#define SLIST_ITERATE(_ITER, _HEAD, _CODE) {\
	_ITER = _HEAD;\
	size_t _depth = 0;\
	for(;_ITER;_ITER=_ITER->next, _depth++) {\
		_CODE\
	}\
}

//Doubly linked list functions
//Add
dlist* dlist_push(dlist*, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result)) __attribute__((warn_unused_result));
dlist* dlist_append(dlist*, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_addOrdered(dlist*, void* buf, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_copy(dlist* src, BOOLEAN deep_copy, list_tspec*) __attribute__((warn_unused_result));

//Remove
void dlist_clear(dlist *he, BOOLEAN destroy_data, list_tspec*);
dlist* dlist_dequeue(dlist* head, void** data, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_pop(dlist* he, void** data, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_removeViaKey(dlist*, void **data, void *key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_removeAllViaKey(dlist*, void **data, void *key, BOOLEAN ordered, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_removeElement(dlist *head, dlist *rem, BOOLEAN destroy_data, list_tspec* type) __attribute__((warn_unused_result));

//Other Operations
dlist* dlist_filter(dlist *head, void* aux, lMapFunc, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_filter_i(dlist *head, void *aux, lMapFunc, BOOLEAN free_data, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_transform(dlist *head, void* aux, lTransFunc);
BOOLEAN dlist_map(dlist *head, BOOLEAN more_info, void* aux, lMapFunc);
size_t dlist_length(dlist *head);
//Transform
void** dlist_toArray(dlist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
void** dlist_toArrayReverse(dlist *head, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));
slist* dlist_to_slist(dlist *he, BOOLEAN deep, list_tspec*) __attribute__((warn_unused_result));

//Merging
dlist* dlist_sort(dlist* head, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_merge(dlist* dst, dlist* src, list_tspec*) __attribute__((warn_unused_result));
dlist* dlist_concat(dlist* dst, dlist* src) __attribute__((warn_unused_result));
dlist* dlist_split(dlist* h1, dlist* h2) __attribute__((warn_unused_result));

//Find
//Data returned from find is PACKED which means that it returns the node, not the data, where the data resides
dlist* dlist_find(dlist *head, const void* key, BOOLEAN ordered, list_tspec*) __attribute__((warn_unused_result));
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
bstree* bstree_insert(bstree* root, void *data, BOOLEAN copy, list_tspec*);
bstree* bstree_remove(bstree* root, void *data, void** rtn, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
bstree* bstree_remove_via_key(bstree* root, void *data, void** rtn, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));

void    bstree_clear(bstree* root, BOOLEAN destroy_data, list_tspec*);
bstree* bstree_find_via_key(bstree *root, const void const *key, list_tspec*);
bstree* bstree_find(bstree *root, void *data, list_tspec*);
bstree* bstree_parent(bstree *root, void *data, list_tspec*);
dlist*  bstree_path(bstree *root, void *data, list_tspec*) __attribute__((warn_unused_result));

bstree* bstree_predessor(bstree *root, bstree *node, list_tspec *type);
bstree* bstree_successor(bstree *root, bstree *node, list_tspec *type);
bstree* bstree_findmin(bstree *root);
bstree* bstree_findmax(bstree *root);

void   bstree_info(bstree *root, size_t *min, size_t *max, size_t *avg, size_t *leaves, size_t *size, dlist **lleaves, dlist **lnodes);

BOOLEAN bstree_map(bstree *root, const TRAVERSAL_STRATEGY, BOOLEAN more_info, void* aux, lMapFunc func);

//Splay Trees
splaytree* splay_insert(splaytree* root, void *data, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
splaytree* splay_remove(splaytree* root, void *data, void** rtn, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
splaytree* splay_remove_via_key(splaytree* root, void *data, void** rtn, BOOLEAN destroy_data, list_tspec*) __attribute__((warn_unused_result));
splaytree* splay_find_via_key(splaytree* root, const void const *key, list_tspec* type) __attribute__((warn_unused_result));
splaytree* splay_find(splaytree* root, void *data, list_tspec* type) __attribute__((warn_unused_result));

//Graphs
graph* graph_insert(graph* root, void* data, BOOLEAN copy, list_tspec*);
graph* graph_link(graph* root, graph* child, list_tspec*);
graph* graph_remove(graph* root, void* key, void** data, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
graph* graph_remove_via_key(graph* root, void* key, void** data, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
graph_adjmat* graph_matrix(graph* root, void* key, void** data, BOOLEAN copy, list_tspec*) __attribute__((warn_unused_result));
void graph_clear(graph *root, BOOLEAN destroy_data, list_tspec*);

void graph_size(graph* root, size_t* nodes, size_t* edges);//count of all the nodes in the graph
graph* graph_find(graph* root, TRAVERSAL_STRATEGY, void* key, list_tspec*);
graph* graph_find_via_key(graph* root, TRAVERSAL_STRATEGY, void* key, list_tspec*);
//TODO this is significantly more complex than previously anticipated (it requires some advanced
//AI stuff to do efficiently, e.g. A*, B*, Beam, D*).
dlist* graph_path(graph* root, TRAVERSAL_STRATEGY, void* key, list_tspec*) __attribute__((warn_unused_result));
graph* graph_path_key_match(graph *root, dlist *key_path, list_tspec *type);

BOOLEAN graph_map(graph* root, TRAVERSAL_STRATEGY, BOOLEAN more_info, void* aux, lMapFunc func);

#endif //_LINKED_STRUCTURES_H_
