#include "graph_tree.h"
#include "slist.h"
#include "splaytree.h"

typedef struct graph_tree_verify_d {
	graph *root;
	size_t num;
} graph_tree_verify_d;

graph*
graph_tree_insert(graph* root, Object* data, BOOLEAN copy){
	//add a new node the child of this node, only if it will not form a loop
	return graph_tree_link(root, graph_insert(NULL, data, copy));
}

static BOOLEAN
graph_tree_verify_f(Object *data, graph_tree_verify_d *aux, graph *node){
	(void)data;
	if(node == aux->root){
		aux->num++;
	}
	//printf("NUM: %zu\n", aux->num);
	return aux->num <= 1;
}

graph*
graph_tree_link(graph* root, graph* child){
	if(child == NULL) return root;
	if(root == NULL) return child;
	//printf("%p %p %p\n", child, child->data, child->edges);
	root->edges = dlist_pushback(root->edges, (Object*)child, FALSE);
	//verify tree
	graph_tree_verify_d test = {
		.root = root,
		.num = 0
	};
	if(!graph_tree_map(root, DEPTH_FIRST_PRE, FALSE, &test, (lMapFunc)graph_tree_verify_f)){
		//printf("Not adding child\n");
		root->edges = dlist_popback(root->edges, NULL, FALSE);
	}
	//Rather than adding ordered, order based on arrival time
	return root;
}

static const Object_vtable node_info_vtable = {
	.destroy = (void(*)(const Object*))free
};

typedef struct node_info {
	const Object_vtable *method;
	const size_t depth;
	const graph *node;
} node_info;

static node_info*
node_info_new(graph *node, size_t depth){
	node_info init = {
		.method = &node_info_vtable,
		.depth = depth,
		.node = node
	};
	return (node_info*)memcpy(LINKED_MALLOC(sizeof(node_info)), &init, sizeof(node_info));
}

#if 0
	typedef struct aLong {
		const Object_vtable *method;
		long data;
	} aLong;

#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC) do { printf(BSTR); dlist *_run; DLIST_ITERATE(_run, LST, printf(FMT, ACC); ); printf(ESTR); } while(0)
static BOOLEAN
graph_print_children(aLong *data){
	printf("%ld ", data->data);
	return TRUE;
}
#else
#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC)
#endif


/**
 * Iterate through the graph in postfix order, normally this is performed prefix. The space complexity is O(3*E+N) worst
 * case, time complexity is O(N) (approximately).
 *
 * @param root - the root of the graph
 * @param pass_data - pass the data, instead of the node itself.
 * @param more_info - give more info, as defined by the lMapFuncAux datatype.
 * @param aux - auxillarly persistent data
 * @param func - function to do
 */
static inline BOOLEAN
graph_tree_map_internal_dfs_po(graph *root, const BOOLEAN more_info, const void* aux, const lMapFunc func){//Post fix
	dlist *stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	dlist *exec = NULL;
	size_t position = 0, size = 0;
	if(more_info) graph_size(root, &size, NULL);
	while(stk){
		node_info *g;
		stk = dlist_popfront(stk, (Object**)&g, FALSE);
		dlist *new_list = dlist_reverse(dlist_copy(g->node->edges, FALSE));
		dlist *run;
		DLIST_ITERATE(run, new_list,
			run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
		);
		stk = dlist_concat(new_list, stk);
	#if 0
		printf("<");
		dlist_map(g->node->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]\n");
	#endif
		exec = dlist_pushback(exec, (Object*)g, FALSE);
	}
	dlist *run;
	exec = dlist_tail(exec);//execute from tail->head, so make the tail first
	DLIST_ITERATE_REVERSE(run, exec,
		node_info *g = (node_info*)(run->data);
		if(more_info){
			lMapFuncAux ax;
			ax.isAux = TRUE;
			ax.depth = g->depth;
			ax.position = position;
			ax.size = size;
			ax.aux = (void*)aux;
			if(!func(g->node->data, &ax, (graph*)g->node)){
				goto cleanup;
			}
			position++;
		} else {
			if(!func(g->node->data, aux, (graph*)g->node)){
				goto cleanup;
			}
		}
	);
	dlist_clear(exec, TRUE);
	return TRUE;
cleanup:
	dlist_clear(stk, TRUE);
	dlist_clear(exec, TRUE);
	return FALSE;
}


BOOLEAN
graph_tree_map(graph *root, const TRAVERSAL_STRATEGY method, const BOOLEAN more_info, const void* aux, const lMapFunc func){
	if(method == DEPTH_FIRST_POST) return graph_tree_map_internal_dfs_po(root, more_info, aux, func);
	dlist *stk = dlist_pushback(NULL, (Object*)node_info_new(root, 0), FALSE);
	size_t position = 0, size = 0;
	if(more_info) graph_tree_size(root, &size, NULL);
	while(stk){
		node_info *g;
		stk = dlist_popfront(stk, (Object**)&g, FALSE);
		dlist *new_list = dlist_copy(g->node->edges, FALSE);
		dlist *run;
		DLIST_ITERATE(run, new_list,
			run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
		);
		if(method == DEPTH_FIRST || method == DEPTH_FIRST_PRE){
			stk = dlist_concat(new_list, stk);
		} else {
			stk = dlist_concat(stk, new_list);
		}
		/*
		printf("<");
		dlist_map(g->node->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]\n");
		*/
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = g->depth,
				.position = position,
				.size = size,
				.aux = (void*)aux
			};
			if(!func(g->node->data, &ax, (graph*)g->node)){
				LINKED_FREE(g);
				goto cleanup;
			}
			position++;
		} else {
			if(!func(g->node->data, aux, (graph*)g->node)){
				LINKED_FREE(g);
				goto cleanup;
			}
		}
		LINKED_FREE(g);
	}
	return TRUE;
cleanup:
	dlist_clear(stk, TRUE);
	return FALSE;
}

struct graph_find_d {
	graph *rtn;
	void* goal;
	const Comparable_vtable *goal_method;
};

static BOOLEAN
graph_find_f(Object* data, struct graph_find_d *aux, graph* node){
	if(aux->goal_method->compare(aux->goal, data) == 0){
		aux->rtn = node;
		return FALSE;
	}
	return TRUE;
}

graph*
graph_tree_find(graph *root, TRAVERSAL_STRATEGY strat, void* key, const Comparable_vtable* key_method){
	struct graph_find_d mm = {
		.rtn = NULL,
		.goal = key,
		.goal_method = key_method
	};
	if(!graph_tree_map(root, strat, FALSE, &mm, (lMapFunc)graph_find_f) && mm.rtn)
		return mm.rtn;
	return NULL;
}

struct graph_size_d {
	size_t nodes;
	size_t edges;
};

static BOOLEAN
graph_size_f(Object *data, struct graph_size_d *aux, graph *root){
	aux->nodes = aux->nodes + 1;
	aux->edges = aux->edges + dlist_length(root->edges);
	return TRUE;
}

void
graph_tree_size(graph* root, size_t *nodes, size_t *edges){
	struct graph_size_d mm = {
		.nodes = 0,
		.edges = 0
	};
	graph_tree_map(root, BREADTH_FIRST, FALSE, &mm, (lMapFunc)graph_size_f);
	if(nodes) *nodes = mm.nodes;
	if(edges) *edges = mm.edges;
}

