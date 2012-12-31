#include "linked_structures.h"
#include "tlsf/tlsf.h"
#include "allocator.h"

static graph*
new_graph_node(void* data, BOOLEAN copy, list_tspec* type){
	graph init = {
		.data = copy?type->deep_copy(data):data,
		.edges = NULL
	};
	graph *mem = MALLOC(sizeof(graph));
	memcpy(mem, &init, sizeof(*mem));
	return mem;
}

graph*
graph_insert(graph* root, void* data, BOOLEAN copy, list_tspec* type){
	//add a new node the child of this node.
	return graph_link(root, new_graph_node(data, copy, type), type);
}

graph*
graph_link(graph* root, graph* child, list_tspec* type){
	if(child == NULL) return root;
	if(root == NULL) return child;
	root->edges = dlist_addOrdered(root->edges, child, FALSE, type);
	return root;
}

#if 0
graph*
graph_remove(graph* root, void* key, void** data, BOOLEAN copy, list_tspec* type){
#error "Unimplmented"
}
#endif

static BOOLEAN
graph_dfs_filter(graph *child, splaytree** tree){
	*tree = splay_find(*tree, child, NULL);
	if((*tree)->data == child) return FALSE;
	return TRUE;
}

struct graph_map_internal_dfs_d {
	splaytree *visited;
	dlist *stk;
	lMapFunc func;
	void *aux;
};
static BOOLEAN
graph_map_internal_bfs_f(graph* child, struct graph_map_internal_dfs_d *aux){
	if((aux->visited = splay_find(aux->visited, child, NULL))->data == child){
		aux->visited = splay_insert(aux->visited, child, FALSE, NULL);
		aux->stk = dlist_concat(aux->stk,
				dlist_filter(child->edges, &aux->visited,
					(lMapFunc)graph_dfs_filter, FALSE, NULL));
		if(!aux->func(child->data, aux->aux)) return FALSE;
	}
	return TRUE;
}
static BOOLEAN
graph_map_internal_dfs_f(graph* child, struct graph_map_internal_dfs_d *aux){
	if((aux->visited = splay_find(aux->visited, child, NULL))->data == child){
		aux->visited = splay_insert(aux->visited, child, FALSE, NULL);
		aux->stk = dlist_concat(dlist_filter(child->edges, &aux->visited,
					(lMapFunc)graph_dfs_filter, FALSE, NULL),
				aux->stk);
		if(!aux->func(child->data, aux->aux)) return FALSE;
	}
	return TRUE;
}

static BOOLEAN
graph_map_internal(graph *root, TRAVERSAL_STRATEGY method, void* aux, lMapFunc func){
	struct graph_map_internal_dfs_d df = {
		.visited = splay_insert(NULL, root, FALSE, NULL),
		.stk = dlist_copy(root->edges, FALSE, NULL),
		.func = func,
		.aux = aux
	};
	lMapFunc search = (method==BREADTH_FIRST)?
		(lMapFunc)graph_map_internal_bfs_f:
		(lMapFunc)graph_map_internal_dfs_f;
	while(df.stk){
		graph *g;
		df.stk = dlist_dequeue(df.stk, (void**)&g, FALSE, NULL);
		if(!dlist_map(g->edges, &df, search))
			return FALSE;
	}
	bstree_clear(df.visited, FALSE, NULL);
	return TRUE;
}

struct graph_map_d {
	void* aux;
	lMapFunc func;
};

static BOOLEAN
graph_map_f(graph *root, struct graph_map_d* aux){
	return aux->func(root->data, aux->aux);
}

BOOLEAN
graph_map(graph *root, TRAVERSAL_STRATEGY strat, void* aux, lMapFunc func){
	struct graph_map_d data = {
		.aux = aux,
		.func = func
	};
	return graph_map_internal(root, strat, &data, (lMapFunc)graph_map_f);
}

struct graph_find_d {
	list_tspec *type;
	BOOLEAN key_compar;
	graph *rtn;
	void *goal;
};

static BOOLEAN
graph_find_f(graph* node, struct graph_find_f *aux){
	if((aux->type->key_compar?
			aux->type->compar(aux->goal, node->data):
			aux->type->key_compar(aux->goal, node->data)) == 0){
		aux->rtn = node;
		return FALSE;
	}
	return TRUE;
}

BOOLEAN
graph_find(graph *root, TRAVERSAL_STRATEGY strat, void* dat, list_tspec* type){
	struct graph_find_d mm = {
		.type = type,
		.key_compar = FALSE,
		.rtn = NULL,
		.goal = dat
	};
	if(!graph_map_internal(root, strat, &mm, (lMapFunc)graph_find_f) && mm.rtn)
		return mm.rtn;
	return NULL;
}

BOOLEAN
graph_find_via_key(graph *root, TRAVERSAL_STRATEGY strat, void* dat, list_tspec* type){
	struct graph_find_d mm = {
		.type = type,
		.key_compar = TRUE,
		.rtn = NULL,
		.goal = dat
	};
	if(!graph_map_internal(root, strat, &mm, (lMapFunc)graph_find_f) && mm.rtn)
		return mm.rtn;
	return NULL;
}

dlist*
graph_path(graph *root, TRAVERSAL_STRATEGY strat, void* dat, list_tspec* type){

}
