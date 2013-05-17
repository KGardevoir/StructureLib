#ifndef _LINKED_STRUCTURES_GRAPH_H_
#define _LINKED_STRUCTURES_GRAPH_H_
#include "linked_base.h"
#include "dlist.h"

typedef struct graph_vtable {
	const Object_vtable parent;
	const Comparable_vtable comparable;
} graph_vtable;

typedef struct graph {
	const graph_vtable const* method;
	Object* data;
	dlist* edges;//ordered by key value (if possible)
} graph; //adjacencly list

typedef struct graph_adjmat {
	size_t i, j; //the size
	graph ***matrix;//[][]; //the graph
	graph **mappings;//[]; //graph mappings
} graph_adjmat;

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

BOOLEAN graph_map(graph* root, TRAVERSAL_STRATEGY, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func);
#endif
