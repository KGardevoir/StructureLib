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

void graph_tree_size(graph* root, size_t* nodes, size_t* edges);//count of all the nodes in the graph
graph* graph_tree_find(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method);
dlist* graph_tree_path(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method) __attribute__((warn_unused_result));

typedef struct {
	graph *i_root;
	dlist *i_stk;
	graph *r_current;
	size_t r_depth;
} graph_tree_iterator_post;

graph_tree_iterator_post* graph_tree_iterator_post_new(graph *, graph_tree_iterator_post* mem);
graph* graph_tree_iterator_post_next(graph_tree_iterator_post*);
void graph_tree_iterator_post_destroy(graph_tree_iterator_post*);

typedef struct {
	graph *i_root;
	dlist *i_stk;
	graph *r_current;
	size_t r_depth;
	BOOLEAN p_add_children;
} graph_tree_iterator_pre;

graph_tree_iterator_pre* graph_tree_iterator_pre_new(graph *, graph_tree_iterator_pre* mem);
graph* graph_tree_iterator_pre_next(graph_tree_iterator_pre*);
void graph_tree_iterator_pre_destroy(graph_tree_iterator_pre*);

typedef struct {
	graph *i_root;
	dlist *i_stk;
	graph *r_current;
	size_t r_depth;
	BOOLEAN p_add_children;
} graph_tree_iterator_breadth;

graph_tree_iterator_breadth* graph_tree_iterator_breadth_new(graph *, graph_tree_iterator_breadth* mem);
graph* graph_tree_iterator_breadth_next(graph_tree_iterator_breadth*);
void graph_tree_iterator_breadth_destroy(graph_tree_iterator_breadth*);



#endif /* end of include guard: _GRAPH_TREE_H_ */

