#include "graph.h"
#include "slist.c"
#include "splaytree.h"
static void graph_node_destroy(graph* self){ dlist_clear(self->edges, FALSE); }
static char* graph_node_hashable(const graph* self, size_t *size){ *size = self->method->parent.size; return (char*)self; }
static long graph_node_compare(graph* self, graph* oth);
static graph* graph_node_copy(graph* self, void* mem);
static graph_vtable _graph_vtable = {
		.parent = {
			.copy = (Object*(*)(const Object *self, void* mem))graph_node_copy,
			.destroy  = (void(*)(const Object *self))graph_node_destroy,
			.hashable = (char*(*)(const Object *self, size_t* size))graph_node_hashable,
			.equals   = NULL,
			.size = sizeof(graph)
		},
		.comparable = {
			.compare = (long(*)(const void* self, const void *oth))graph_node_compare
		}
	};
static long graph_node_compare(graph* self, graph* oth){ return self>oth?1:(self==oth?0:-1); }
//static long graph_node_compare(graph* self, graph* oth){
//	return ((aComparable*)self->data)->method->compare(self->data, oth->data);
//}

static inline graph*
graph_node_new(graph* self, Object* data, BOOLEAN copy){
	graph init = {
		.method = &_graph_vtable,
		.data = copy?data->method->copy(data, MALLOC(self->method->parent.size)):data,
		.edges = NULL
	};
	//graph *mem = (graph*)MALLOC(sizeof(graph));
	memcpy(self, &init, sizeof(init));
	return self;
}

static graph*
graph_node_copy(graph* self, void* mem){
	graph init = {
		.method = &_graph_vtable,
		.data = self->data,
		.edges = self->edges
	};
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}



graph*
graph_insert(graph* root, Object* data, BOOLEAN copy){
	//add a new node the child of this node.
	return graph_link(root, graph_node_new(MALLOC(_graph_vtable.parent.size), data, copy));
}

graph*
graph_link(graph* root, graph* child){
	if(child == NULL) return root;
	if(root == NULL) return child;
	//printf("%p %p %p\n", child, child->data, child->edges);
	root->edges = dlist_append(root->edges, (Object*)child, FALSE);
	//Rather than adding ordered, order based on arrival time
	return root;
}

#if 0
graph*
graph_remove(graph* root, void* key, void** data, BOOLEAN copy, list_tspec* type){
#error "Unimplmented"
}
#endif

static BOOLEAN
graph_map_filter_f(graph *child, splaytree** tree){
	*tree = splay_find(*tree, (Object*)child, &child->method->comparable);
	if(*tree && child->method->comparable.compare(child, (*tree)->data) == 0){
		//printf("E");
		return FALSE;
	}
	//printf("N");
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
graph_map_internal_dfs_po(graph *root, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){//Post fix
	splaytree *visited = NULL; //splay_insert(NULL, (Object*)root, &root->method->comparable, FALSE);
	splaytree *processed = NULL;
	dlist *stk = dlist_append(NULL, (Object*)root, FALSE);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) graph_size(root, &size, NULL);
	while(stk){
		while(TRUE){//expand nodes to maximium (left-most)
			graph *g;
			size_t prev_size = dlist_length(stk);
			stk = dlist_dequeue(stk, (Object**)&g, FALSE);
			visited = splay_insert(visited, (Object*)g, &g->method->comparable, FALSE);
			//stk = dlist_filter_i(stk, &visited, (lMapFunc)graph_map_filter_f, FALSE);
			stk = dlist_concat(
					dlist_append(dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE), (Object*)g, FALSE), stk);
			size_t new_size = dlist_length(stk);
			if(new_size <= prev_size) break;
		}//expand till we can't expand any more.
		graph *g;
		stk = dlist_dequeue(stk, (Object**)&g, FALSE);
		processed = splay_insert(processed, (Object*)g, &g->method->comparable, FALSE);
		stk = dlist_filter_i(stk, &processed, (lMapFunc)graph_map_filter_f, FALSE);//clear entries we have now processed
		/*
		printf("<");
		dlist_map(g->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]");
		printf("(%ld);", g->data);
		*/
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(pass_data?g->data:(Object*)g, &ax))
				goto cleanup;
			position++;
		} else {
			if(!func(pass_data?g->data:(Object*)g, aux))
				goto cleanup;
		}
	}
	btree_clear(visited, FALSE);
	btree_clear(processed, FALSE);
	return TRUE;
cleanup:
	btree_clear(visited, FALSE);
	btree_clear(processed, FALSE);
	dlist_clear(stk, FALSE);
	return FALSE;
}

static BOOLEAN
graph_map_internal(graph *root, TRAVERSAL_STRATEGY method, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	if(method == DEPTH_FIRST_POST) return graph_map_internal_dfs_po(root, pass_data, more_info, aux, func);
	splaytree *visited = splay_insert(NULL, (Object*)root, &root->method->comparable, FALSE);
	dlist *stk = dlist_append(NULL, (Object*)root, FALSE);
	size_t depth = 0, position = 0, size = 0;
	if(more_info) graph_size(root, &size, NULL);
	while(stk){
		graph *g;
		stk = dlist_dequeue(stk, (Object**)&g, FALSE);
		visited = splay_insert(visited, (Object*)g, &g->method->comparable, FALSE);
		stk = dlist_filter_i(stk, &visited, (lMapFunc)graph_map_filter_f, FALSE);
		if(method == DEPTH_FIRST || method == DEPTH_FIRST_PRE){
			stk = dlist_concat(
					dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE), stk);
		} else {
			stk = dlist_concat(stk,
					dlist_filter(g->edges, &visited,
						(lMapFunc)graph_map_filter_f, FALSE));
		}
		/*
		printf("<");
		dlist_map(g->edges, FALSE, NULL, (lMapFunc)graph_print_children);
		printf(">");
		printf("[");
		dlist_map(stk, FALSE, NULL, (lMapFunc)graph_print_children);
		printf("]");
		printf("(%ld);", g->data);
		*/
		if(more_info){
			lMapFuncAux ax = {
				.isAux = TRUE,
				.depth = depth,
				.position = position,
				.size = size,
				.aux = aux
			};
			if(!func(pass_data?g->data:(Object*)g, &ax))
				goto cleanup;
			position++;
		} else {
			if(!func(pass_data?g->data:(Object*)g, aux))
				goto cleanup;
		}
	}
	btree_clear(visited, FALSE);
	return TRUE;
cleanup:
	btree_clear(visited, FALSE);
	dlist_clear(stk, FALSE);
	return FALSE;
}

BOOLEAN
graph_map(graph *root, TRAVERSAL_STRATEGY strat, BOOLEAN pass_data, BOOLEAN more_info, void* aux, lMapFunc func){
	return graph_map_internal(root, strat, pass_data, more_info, aux, func);
}

typedef struct
graph_clear_d {
	BOOLEAN destroy_data;
	slist *head;
} graph_clear_d;

static BOOLEAN
graph_clear_f(graph *node, graph_clear_d* aux){
	if(aux->destroy_data) node->data->method->destroy(node->data);
	aux->head = slist_push(aux->head, (Object*)node, FALSE);
	return TRUE;
}
void
graph_clear(graph *root, BOOLEAN destroy_data){
	graph_clear_d aux = {
		.destroy_data = destroy_data,
		.head = NULL
	};
	graph_map_internal(root, DEPTH_FIRST, FALSE, FALSE, &aux, (lMapFunc)graph_clear_f);
	slist *iter;
	SLIST_ITERATE(iter, aux.head,
		iter->data->method->destroy((Object*)iter->data);
		FREE(iter->data);
	);
	slist_clear(aux.head, FALSE);
}

struct graph_find_d {
	graph *rtn;
	void* goal;
	const Comparable_vtable *goal_method;
};

static BOOLEAN
graph_find_f(graph* node, struct graph_find_d *aux){
	if(aux->goal_method->compare(aux->goal, node->data) == 0){
		aux->rtn = node;
		return FALSE;
	}
	return TRUE;
}

graph*
graph_find(graph *root, TRAVERSAL_STRATEGY strat, void* key, const Comparable_vtable* key_method){
	struct graph_find_d mm = {
		.rtn = NULL,
		.goal = key,
		.goal_method = key_method
	};
	if(!graph_map_internal(root, strat, FALSE, FALSE, &mm, (lMapFunc)graph_find_f) && mm.rtn)
		return mm.rtn;
	return NULL;
}

struct graph_size_d {
	size_t nodes;
	size_t edges;
};

static BOOLEAN
graph_size_f(graph* root, struct graph_size_d *aux){
	aux->nodes = aux->nodes + 1;
	aux->edges = aux->edges + dlist_length(root->edges);
	return TRUE;
}

void
graph_size(graph* root, size_t *nodes, size_t *edges){
	struct graph_size_d mm = {
		.nodes = 0,
		.edges = 0
	};
	graph_map_internal(root, BREADTH_FIRST, FALSE, FALSE, &mm, (lMapFunc)graph_size_f);
	*nodes = mm.nodes;
	*edges = mm.edges;
}

graph*
graph_path_key_match(graph *root, dlist *key_path){
	size_t size = dlist_length(key_path);
	size_t i = 0;
	dlist *run = key_path;
	graph *troot = root;
	while(i < size){
		dlist *descent = dlist_find(troot->edges, (void*)run->data, &root->method->comparable, FALSE);
		if(descent == NULL) return NULL;
		troot = (graph*)descent->data;
		i++; run = run->next;
	}
	return troot;
}


#if 0
graph*
graph_spanning(graph* root, long(edge_weight)(graph*,graph*)){
	//TODO utilize Kruskal's Algorithm
}
#endif
