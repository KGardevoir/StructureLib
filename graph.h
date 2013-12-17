#ifndef _LINKED_STRUCTURES_GRAPH_H_
#define _LINKED_STRUCTURES_GRAPH_H_
#include "linked_base.h"
#include "dlist.h"
#include "splaytree.h"

typedef struct graph_vtable {
	const Object_vtable parent;
	const Comparable_vtable comparable;
} graph_vtable;

typedef struct graph {
	const graph_vtable const* method;
	Object* data;
	dlist* edges;//ordered by key value (if possible)
} graph; //adjacency list

typedef struct graph_adjmat {
	const size_t i, j; //the size
	graph ***matrix;//[][]; //the graph
	graph **mappings;//[]; //graph mappings, e.g. mapping[2] is the graph node "2"
} graph_adjmat;

//Graphs
graph* graph_insert(graph* root, Object* data, BOOLEAN copy) __attribute__((warn_unused_result));
graph* graph_link(graph* root, graph* child);
graph* graph_remove(graph* root, Object* key, Object** data, BOOLEAN copy) __attribute__((warn_unused_result));
graph_adjmat* graph_matrix(graph* root, Object* key, Object** data, BOOLEAN copy) __attribute__((warn_unused_result));
void graph_clear(graph *root, BOOLEAN destroy_data);

void graph_size(graph* root, size_t* nodes, size_t* edges);//count of all the nodes in the graph
graph* graph_find(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method);
dlist* graph_path(graph* root, TRAVERSAL_STRATEGY, void* key, const Comparable_vtable* key_method) __attribute__((warn_unused_result));
graph* graph_path_key_match(graph *root, dlist *key_path);

typedef struct {
	const struct Iterator_vtable *method;
	graph *r_current;
	size_t r_depth;
	BOOLEAN p_add_children;
} graph_iterator;

typedef struct {
	graph_iterator iterator;
	graph *i_root;
	dlist *i_stk;
	splaytree *i_processed;
	splaytree *i_visited;
} graph_iterator_post;

graph_iterator_post* graph_iterator_post_new(graph *, graph_iterator_post* mem);

typedef struct {
	graph_iterator iterator;
	graph *i_root;
	dlist *i_stk, *i_children;
	splaytree *i_visited;
} graph_iterator_pre;

graph_iterator_pre* graph_iterator_pre_new(graph *, graph_iterator_pre* mem);

typedef struct {
	graph_iterator iterator;
	graph *i_root;
	dlist *i_stk, *i_children;
	splaytree *i_visited;
} graph_iterator_breadth;

graph_iterator_breadth* graph_iterator_breadth_new(graph *, graph_iterator_breadth* mem);

//returns a list of nodes topographically sorted (using graph_map DEPTH_FIRST_POST). Your graph should ideally be a DAG,
//otherwise the sort will be invalid as it is impossible to topologically sort these types of graphs, but this will
//still return a result.
dlist* graph_topological_sort(graph* tree) __attribute__((warn_unused_result));
#endif
