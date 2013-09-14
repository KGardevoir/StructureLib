/**
 * In graph speak, these are DAG's, the insert functions verify that the graph is a DAG after the node is added,
 * otherwise it is rejected.
 */
#ifndef _GRAPH_TREE_H_
#define _GRAPH_TREE_H_
#include "linked_base.h"
#include "graph.h"

graph* graph_tree_insert(graph* root, Object* data, BOOLEAN copy) __attribute__((warn_unused_result));
graph* graph_tree_link(graph* root, graph* child);

graph* graph_tree_remove(graph* root, Object* key, Object** data, BOOLEAN copy) __attribute__((warn_unused_result));
graph_adjmat* graph_tree_matrix(graph* root, Object* key, Object** data, BOOLEAN copy) __attribute__((warn_unused_result));
void graph_tree_clear(graph *root, BOOLEAN destroy_data);

void graph_tree_size(graph* root, size_t* nodes, size_t* edges);//count of all the nodes in the graph
graph* graph_tree_find(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method);
dlist* graph_tree_path(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method) __attribute__((warn_unused_result));

BOOLEAN graph_tree_map(graph* root, const TRAVERSAL_STRATEGY, const BOOLEAN more_info, const void* aux, const lMapFunc func);


#endif /* end of include guard: _GRAPH_TREE_H_ */

