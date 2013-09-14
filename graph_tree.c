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

static BOOLEAN
graph_map_filter_f(node_info *child, splaytree** tree){
	*tree = splay_find(*tree, (Object*)child->node, &child->node->method->comparable);
	if(*tree && child->node->method->comparable.compare(child->node, (*tree)->data) == 0){
		//printf("E");
		return FALSE;
	}
	//printf("N");
	return TRUE;
}

static BOOLEAN
graph_print_children(Object *data){
	printf("%p ", data);
	return TRUE;
}

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
	splaytree *processed = NULL;
	size_t position = 0, size = 0;
	if(more_info) graph_size(root, &size, NULL);
	while(stk){
		while(TRUE){//expand nodes to maximum (left-most)
			node_info *g;
			size_t prev_size = dlist_length(stk);
			stk = dlist_popfront(stk, (Object**)&g, FALSE);
			//stk = dlist_filter_i(stk, &visited, (lMapFunc)graph_map_filter_f, FALSE);
			dlist *new_list = dlist_copy(g->node->edges, FALSE);
			dlist *run;
			DLIST_ITERATE(run, new_list,
				run->data = (void*)node_info_new((graph*)run->data, g->depth+1);
			);
			stk = dlist_concat(dlist_pushback(new_list, (Object*)g, FALSE), stk);
			size_t new_size = dlist_length(stk);
			if(new_size <= prev_size) break;
		}//expand till we can't expand any more.
		node_info *g;
		stk = dlist_popfront(stk, (Object**)&g, FALSE);
	#if 1
		printf("<");
		dlist_map(g->node->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]\n");
	#endif
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = g->depth,
				.position = position,
				.size = size,
				.aux = aux
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

#if 0
	typedef struct aLong {
		const Object_vtable *method;
		long data;
	} aLong;

#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC) do { printf(BSTR); dlist *_run; DLIST_ITERATE(_run, LST, printf(FMT, ACC); ); printf(ESTR); } while(0)
#else
#define DUMPLIST(BSTR, ESTR, LST, FMT, ACC)
#endif


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
				.aux = aux
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

typedef struct
graph_clear_d {
	BOOLEAN destroy_data;
	slist *head;
} graph_clear_d;

static BOOLEAN
graph_clear_f(Object* dat, graph_clear_d* aux, graph *node){
	if(aux->destroy_data) dat->method->destroy(dat);
	aux->head = slist_pushfront(aux->head, (Object*)node, FALSE);
	return TRUE;
}

void
graph_tree_clear(graph *root, BOOLEAN destroy_data){
	graph_clear_d aux = {
		.destroy_data = destroy_data,
		.head = NULL
	};
	graph_tree_map(root, DEPTH_FIRST, FALSE, &aux, (lMapFunc)graph_clear_f);
	slist *iter;
	SLIST_ITERATE(iter, aux.head,
		CALL_VOID(iter->data, destroy,(iter->data));
		LINKED_FREE(iter->data);
	);
	slist_clear(aux.head, FALSE);
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
	graph_map(root, BREADTH_FIRST, FALSE, &mm, (lMapFunc)graph_size_f);
	if(nodes) *nodes = mm.nodes;
	if(edges) *edges = mm.edges;
}

